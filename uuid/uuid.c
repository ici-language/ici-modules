/*
 * $Id: uuid.c,v 1.2 2000/10/13 10:23:23 atrn Exp $
 *
 * ICI UUID library interface. Placed in the PUBLIC DOMAIN, A.Newman, October 2000.
 *
 */

#include <ici.h>
#include <uuid/uuid.h>

typedef struct
{
    object_t	o_head;
    uuid_t	u_uid;
}
ici_uuid_t;

static int uuid_tcode = 0;

static unsigned long
mark_uuid(object_t *o)
{
    o->o_flags |= O_MARK;
    return sizeof (ici_uuid_t);
}

static void
free_uuid(object_t *o)
{
    ici_tfree(o, ici_uuid_t);
}

static type_t
ici_uuid_type =
{
    mark_uuid,
    free_uuid,
    ici_hash_unique,
    ici_cmp_unique,
    ici_copy_simple,
    ici_assign_fail,
    ici_fetch_fail,
    "uuid"
};

inline static int
isuuid(object_t *o)
{
    return ici_typeof(o) == &ici_uuid_type;
}

static ici_uuid_t *
new_uuid(uuid_t uuid)
{
    ici_uuid_t	*u;

    if ((u = ici_talloc(ici_uuid_t)) != NULL)
    {
        ICI_OBJ_SET_TFNZ(u, uuid_tcode, 0, 1, sizeof *u);
	ici_rego(u);
	memcpy(u->u_uid, uuid, sizeof uuid);
    }
    return u;
}

static int
ici_uuid_clear(void)
{
    ici_uuid_t	*u;

    if (ici_typecheck("o", &u))
	return 1;
    if (!isuuid(objof(u)))
	return ici_argerror(0);
    uuid_clear(u->u_uid);
    return ici_null_ret();
}

static int
ici_uuid_compare(void)
{
    ici_uuid_t	*u;
    ici_uuid_t	*v;

    if (ici_typecheck("oo", &u, &v))
	return 1;
    if (!isuuid(objof(u)))
	return ici_argerror(0);
    if (!isuuid(objof(v)))
	return ici_argerror(1);
    return ici_int_ret(uuid_compare(u->u_uid, v->u_uid));
}

static int
ici_uuid_copy(void)
{
    ici_uuid_t	*u;
    uuid_t	uu2;

    if (ici_typecheck("o", &u))
	return 1;
    if (!isuuid(objof(u)))
	return ici_argerror(0);
    uuid_copy(u->u_uid, uu2);
    return ici_ret_with_decref(objof(new_uuid(uu2)));
}

static int
ici_uuid_generate(void)
{
    uuid_t	uu;

    uuid_generate(uu);
    return ici_ret_with_decref(objof(new_uuid(uu)));
}

static int
ici_uuid_generate_random(void)
{
    uuid_t	uu;

    uuid_generate_random(uu);
    return ici_ret_with_decref(objof(new_uuid(uu)));
}

static int
ici_uuid_generate_time(void)
{
    uuid_t	uu;

    uuid_generate_time(uu);
    return ici_ret_with_decref(objof(new_uuid(uu)));
}

static int
ici_uuid_is_null(void)
{
    ici_uuid_t	*u;

    if (ici_typecheck("o", &u))
	return 1;
    if (!isuuid(objof(u)))
	return ici_argerror(0);
    return ici_int_ret(uuid_is_null(u->u_uid));
}

static int
ici_uuid_parse(void)
{
    char	*str;
    uuid_t	uu;

    if (ici_typecheck("s", &str))
	return 1;
    if (uuid_parse(str, uu) == -1)
    {
	ici_error = "invalid uuid";
	return 1;
    }
    return ici_ret_with_decref(objof(new_uuid(uu)));
}

static int
ici_uuid_unparse(void)
{
    ici_uuid_t	*u;
    char	mybuf[64];

    if (ici_typecheck("o", &u))
	return 1;
    if (!isuuid(objof(u)))
	return ici_argerror(0);
    uuid_unparse(u->u_uid, mybuf);
    return ici_str_ret(mybuf);
}

static int
ici_uuid_time(void)
{
    ici_uuid_t		*u;
    time_t		t;

    if (ici_typecheck("o", &u))
	return 1;
    if (!isuuid(objof(u)))
	return ici_argerror(0);
    return ici_int_ret(uuid_time(u->u_uid, NULL));
}

static int
ici_uuid_typef(void)
{
    ici_uuid_t	*u;

    if (ici_typecheck("o", &u))
	return 1;
    if (!isuuid(objof(u)))
	return ici_argerror(0);
    return ici_int_ret(uuid_type(u->u_uid));
}

static int
ici_uuid_variant(void)
{
    ici_uuid_t	*u;

    if (ici_typecheck("o", &u))
	return 1;
    if (!isuuid(objof(u)))
	return ici_argerror(0);
    return ici_int_ret(uuid_variant(u->u_uid));
}

static cfunc_t ici_uuid_cfuncs[] =
{
    {CF_OBJ, "clear", ici_uuid_clear},
    {CF_OBJ, "compare", ici_uuid_compare},
    {CF_OBJ, "copy", ici_uuid_copy},
    {CF_OBJ, "generate", ici_uuid_generate},
    {CF_OBJ, "generate_random", ici_uuid_generate_random},
    {CF_OBJ, "generate_time", ici_uuid_generate_time},
    {CF_OBJ, "is_null", ici_uuid_is_null},
    {CF_OBJ, "parse", ici_uuid_parse},
    {CF_OBJ, "unparse", ici_uuid_unparse},
    {CF_OBJ, "time", ici_uuid_time},
    {CF_OBJ, "type", ici_uuid_typef},
    {CF_OBJ, "variant", ici_uuid_variant},
    {CF_OBJ}
};

object_t *
ici_uuid_library_init(void)
{
    if (ici_interface_check(ICI_VER, ICI_BACK_COMPAT_VER, "uuid"))
        return NULL;
    if ((uuid_tcode = ici_register_type(&ici_uuid_type)) == 0)
        return NULL;
    return objof(ici_module_new(ici_uuid_cfuncs));
}
