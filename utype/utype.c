#include "utype.h"
#include "struct.h"
#include "str.h"
#include "exec.h"
#include "func.h"
#include "op.h"
#include "null.h"

ustruct_t *
new_ustruct(utype_t *utype)
{
    ustruct_t	*u = talloc(ustruct_t);

    if (u != NULL)
    {
	if ((u->u_struct = new_struct()) == NULL)
	{
	    ici_free(u);
	    return NULL;
	}
	objof(u)->o_tcode = TC_OTHER;
	objof(u)->o_type  = &utype->u_type;
	objof(u)->o_nrefs = 1;
	objof(u)->o_flags = 0;
	u->u_type = utype;
	rego(objof(u));
	decref(u->u_struct);
    }
    return u;
}

static unsigned long
mark_ustruct(object_t *o)
{
    o->o_flags |= O_MARK;
    return
	sizeof (ustruct_t)
	+
	mark(ustructof(o)->u_struct)
	+
	mark(ustructof(o)->u_type);
}

static int
assign_ustruct(object_t *o, object_t *k, object_t *v)
{
    func_t	*f;

    if ((f = ustructof(o)->u_type->u_assign) == NULL)
	return assign(ustructof(o)->u_struct, k, v);
    error = ici_func(objof(f), "ooo", ustructof(o)->u_struct, k, v);
    return error != NULL;
}

static object_t *
fetch_ustruct(object_t *o, object_t *k)
{
    func_t	*f;
    object_t	*v;

    if ((f = ustructof(o)->u_type->u_fetch) == NULL)
	return fetch(ustructof(o)->u_struct, k);
    error = ici_func(objof(f), "o=oo", &v, ustructof(o)->u_struct, k);
    if (error != NULL)
	v = NULL;
    else
	decref(v);
    return v;
}

static type_t ustruct_type =
{
    mark_ustruct,
    free_simple,
    hash_unique,
    cmp_unique,
    copy_simple,
    assign_ustruct,
    fetch_ustruct,
    "ustruct"
};

utype_t *
new_utype(string_t *name, func_t *fnew, func_t *fassign, func_t *ffetch)
{
    utype_t *u = talloc(utype_t);

    if (u != NULL)
    {
	objof(u)->o_tcode = TC_OTHER;
	objof(u)->o_type = &utype_type;
	objof(u)->o_nrefs = 1;
	objof(u)->o_flags = 0;
	u->u_name   = name;
	u->u_new    = fnew;
	u->u_assign = fassign;
	u->u_fetch  = ffetch;
	u->u_type   = ustruct_type;
	u->u_type.t_name = name->s_chars;
	rego(objof(u));
    }
    return u;
}

static unsigned long
mark_utype(object_t *o)
{
    long rc;

    o->o_flags |= O_MARK;
    rc = sizeof (utype_t) + mark(utypeof(o)->u_name);
    if (utypeof(o)->u_new != NULL)
	rc += mark(utypeof(o)->u_new);
    if (utypeof(o)->u_assign != NULL)
	rc += mark(utypeof(o)->u_assign);
    if (utypeof(o)->u_fetch != NULL)
	rc += mark(utypeof(o)->u_fetch);
    return rc;
}

static int	need_strings = 1;
static string_t	*string_new;
static string_t	*string_assign;
static string_t	*string_fetch;

static object_t *
fetch_utype(object_t *o, object_t *k)
{
    func_t		*f;

    if (need_strings)
    {
	if
	(
	    need_string(&string_new, "new")
	    ||
	    need_string(&string_assign, "assign")
	    ||
	    need_string(&string_fetch, "fetch")
	)
	    return NULL;
	need_strings = 0;
    }
    if (k == objof(string_new))
	f = utypeof(o)->u_new;
    else if (k == objof(string_assign))
	f = utypeof(o)->u_assign;
    else if (k == objof(string_fetch))
	f = utypeof(o)->u_fetch;
    else
	return fetch_simple(o, k);
    if (f == NULL)
	return objof(&o_null);
    return objof(f);
}

static int
assign_utype(object_t *o, object_t *k, object_t *v)
{
    if (need_strings)
    {
	if
	(
	    need_string(&string_new, "new")
	    ||
	    need_string(&string_assign, "assign")
	    ||
	    need_string(&string_fetch, "fetch")
	)
	    return 1;
	need_strings = 0;
    }
    if (!isfunc(v))
	return assign_simple(o, k, v);
    if (k == objof(string_new))
	utypeof(o)->u_new = funcof(v);
    else if (k == objof(string_assign))
	utypeof(o)->u_assign = funcof(v);
    else if (k == objof(string_fetch))
	utypeof(o)->u_fetch = funcof(v);
    else
	return assign_simple(o, k, v);
    return 0;
}

type_t	utype_type =
{
    mark_utype,
    free_simple,
    hash_unique,
    cmp_unique,
    copy_simple,
    assign_utype,
    fetch_utype,
    "utype"
};

static int
f_def(void)
{
    string_t	*name;
    func_t	*fnew = NULL;
    func_t	*fassign = NULL;
    func_t	*ffetch = NULL;

    switch (NARGS())
    {
    case 1:
	if (ici_typecheck("o", &name))
	    return 1;
	break;
    case 2:
	if (ici_typecheck("oo", &name, &fnew))
	    return 1;
	break;
    case 3:
	if (ici_typecheck("ooo", &name, &fnew, &fassign))
	    return 1;
	break;
    case 4:
	if (ici_typecheck("oooo", &name, &fnew, &fassign, &ffetch))
	    return 1;
	break;
    default:
	return ici_argcount(4);
    }
    if (!isstring(objof(name)))
	return ici_argerror(0);
    if (fnew != NULL && !isfunc(objof(fnew)) && !isnull(objof(fnew)))
	return ici_argerror(1);
    if (fassign != NULL && !isfunc(objof(fassign)) && !isnull(objof(fassign)))
	return ici_argerror(2);
    if (ffetch != NULL && !isfunc(objof(ffetch)) && !isnull(objof(ffetch)))
	return ici_argerror(3);
    if (isnull(objof(fnew)))
	fnew = NULL;
    if (isnull(objof(fassign)))
	fassign = NULL;
    if (isnull(objof(ffetch)))
	ffetch = NULL;
    return ici_ret_with_decref(objof(new_utype(name, fnew, fassign, ffetch)));
}

static int
f_new(void)
{
    utype_t *utype;
    array_t *vargs;
    object_t **o;
    int i;
    ustruct_t *self;

    o = ARGS();
    utype = utypeof(*o--);
    if (!isutype(objof(utype)))
	return ici_argerror(0);
    if ((self = new_ustruct(utype)) == NULL)
	return 1;
    if ((vargs = new_array()) == NULL)
    {
	decref(self);
	return 1;
    }
    *vargs->a_top++ = objof(self->u_struct);
    if ((i = NARGS() - 1) > 0)
    {
	if (pushcheck(vargs, i+1))
	{
	    decref(self);
	    decref(vargs);
	    return 1;
	}
	while (i-- > 0)
	    *vargs->a_top++ = *o--;
    }
    error = ici_call("call", "oo", utype->u_new, vargs);
    decref(vargs);
    if (error != NULL)
    {
	decref(self);
	return 1;
    }
    return ici_ret_with_decref(objof(self));
}

cfunc_t		utype_cfuncs[] =
{
    {CF_OBJ,	"def",		f_def},
    {CF_OBJ,	"new",		f_new},
    {CF_OBJ}
};

object_t *
ici_utype_library_init(void)
{
    struct_t	*s = new_struct();

    if (s != NULL && ici_assign_cfuncs(s, utype_cfuncs))
    {
	decref(s);
	return NULL;
    }
    return objof(s);
}
