/*
 * $Id: misc.c,v 1.5 2000/11/01 09:46:19 atrn Exp $
 *
 * Code that is common between the save and restore sides of serialisation
 * is in this file (they are separate modules). This includes the code to
 * initialise the various tables used.
 */

#include "serialisation.h"

/*
 * Struct to map a reference to an object
 */
typedef struct
{
    long	name;
    object_t	*obj;
}
ref_t;

/*
 * Simple growable array of ref_t structures to keep track of names during
 * a serialisation pass.
 */
static int	nrefs;
static int	maxrefs;
static ref_t	*refs;

/*
 * Map a reference to an object.
 */
object_t *
ici_serialisation_find_ref(long v)
{
    int	i;

    for (i = 0; i < nrefs; ++i)
	if (refs[i].name == v)
	    return refs[i].obj;
    return 0;
}

/*
 * Modify an existing reference. Used when an atomic object
 * matches an existing atomic object. The reference was previously
 * entered for a new object and we've just atomised it and need
 * to patch the table entry to point to the atomic version.
 */
void
ici_serialisation_mod_ref(object_t *old, object_t *new)
{
    int	i;

    for (i = 0; i < nrefs; ++i)
    {
	if (refs[i].obj == old)
	{
	    refs[i].obj = new;
	    return;
	}
    }
    abort();
}

/*
 * Add a reference to the ref table, growing it as required.
 * Returns non-zero on error (allocation), zero if okay.
 */
int
ici_serialisation_add_ref(long v, object_t *o)
{
    int	i;

    for (i = 0; i < nrefs; ++i)
    {
	if (refs[i].name == v)
	{
	    if ((i = refs[i].obj != o))
		error = "duplicate object name in restore";
	    return i;
	}
    }
    if (i == maxrefs)
    {
	ref_t	*newrefs;

	if (refs == NULL)
	{
	    if ((newrefs = (ref_t *)ici_alloc(sizeof (ref_t) * 8)) == NULL)
		return 1;
	    maxrefs = 8;
	}
	else if ((newrefs = (ref_t *)ici_alloc(sizeof (ref_t) * maxrefs * 2)) == NULL)
	    return 1;
	else
	{
	    memcpy(newrefs, refs, nrefs * sizeof (ref_t));
	    ici_free(refs);
	    maxrefs *= 2;
	}
	refs = newrefs;
    }
    refs[nrefs].name = v;
    refs[nrefs].obj  = o;
    ++nrefs;
    return 0;
}

/*
 * Does nothing but complement ici_archive_end(). Reset's the
 * reference table state which isn't exactly required but is
 * good form. Catches bugs which is good.
 */
void
ici_serialisation_start(void)
{
    nrefs = maxrefs = 0;
    refs = NULL;
}

/*
 * Undo all our add_refs. If unmark is true we also remove any
 * O_SAVED flags on the objects.
 */
void
ici_serialisation_end(int op)
{
    if (op == ICI_SERIALISATION_SAVE)
    {
	while (nrefs-- > 0)
	    refs[nrefs].obj->o_flags &= ~O_SAVED;
    }
    nrefs = maxrefs = 0;
    if (refs != NULL)
    {
	ici_free(refs);
	refs = NULL;
    }
}

ici_serialisation_opfunc_t	ici_serialisation_opfunc[] =
{
    ici_op_offsq,
    ici_op_openptr,
    ici_op_mklvalue,
    ici_op_onerror,
    ici_op_looper,
    ici_op_continue,
    ici_op_mkptr,
    ici_op_if,
    ici_op_ifelse,
    ici_op_pop,
    ici_op_andand,
    ici_op_switch,
    ici_op_switcher,
    ici_op_return,
    ici_op_fetch,
    ici_op_unary,
    ici_op_for,
    ici_op_forall,
#ifdef BINOPFUNC
    ici_op_binop,
#endif
};

/*
 * Number of operator functions in the above table.
 */
int	ici_serialisation_nopfunc =
	    sizeof ici_serialisation_opfunc/sizeof ici_serialisation_opfunc[0];

/*
 * Lets us map type_t or object type code to a function pointer. The function
 * pointer is either the save or restore method according to which side of
 * serialisation we're on. This is initialised when the module is loaded.
 */
static ici_serialisation_type_t	typemap[] =
{
#ifndef _WINDOWS
    {NULL, 		0,		NULL,	NULL},	/* TC_OTHER */
    {&pc_type,		TC_PC,		NULL,	NULL},	/* TC_PC */
    {&src_type,		TC_SRC, 	NULL,	NULL},	/* TC_SRC */
    {&parse_type,	TC_PARSE,	NULL,	NULL},	/* TC_PARSE */
    {&op_type,		TC_OP, 		NULL,	NULL},	/* TC_OP */
    {&string_type,	TC_STRING, 	NULL,	NULL},	/* TC_STRING */
    {&ici_catch_type,	TC_CATCH, 	NULL,	NULL},	/* TC_CATCH */
    {&forall_type,	TC_FORALL, 	NULL,	NULL},	/* TC_FORALL */
    {&int_type,		TC_INT, 	NULL,	NULL},	/* TC_INT */
    {&float_type,	TC_FLOAT, 	NULL,	NULL},	/* TC_FLOAT */
    {&regexp_type,	TC_REGEXP, 	NULL,	NULL},	/* TC_REGEXP */
    {&ptr_type,		TC_PTR, 	NULL,	NULL},	/* TC_PTR */
    {&ici_array_type,	TC_ARRAY, 	NULL,	NULL},	/* TC_ARRAY */
    {&struct_type,	TC_STRUCT, 	NULL,	NULL},	/* TC_STRUCT */
    {&set_type,		TC_SET, 	NULL,	NULL},	/* TC_SET */
    {NULL,		0, 		NULL,	NULL},	/* UNUSED */
    {&mem_type,		TC_MEM, 	NULL,	NULL},	/* TC_MEM */
    {&null_type,	TC_NULL, 	NULL,	NULL},	/* TC_NULL */
#ifdef ici3
    {&ici_func_type,	TC_FUNC, 	NULL,	NULL},	/* TC_FUNC */
    {&ici_cfunc_type,	TC_CFUNC, 	NULL,	NULL},	/* TC_CFUNC */
#else
    {&func_type,	TC_FUNC, 	NULL,	NULL},	/* TC_FUNC */
    {&func_type,	TC_CFUNC, 	NULL,	NULL},	/* TC_CFUNC */
#endif
    {&file_type,	TC_FILE, 	NULL,	NULL},	/* TC_FILE */
    {NULL,		0, 		NULL,	NULL},	/* TC_REF, UNUSED */
#ifdef ici3
    {&ici_method_type,	TC_METHOD, 	NULL,	NULL}	/* TC_METHOD */
#endif
#else
    {NULL, 		0,		NULL,	NULL},	/* TC_OTHER */
    {NULL,		TC_PC,		NULL,	NULL},	/* TC_PC */
    {NULL,		TC_SRC, 	NULL,	NULL},	/* TC_SRC */
    {NULL,		TC_PARSE,	NULL,	NULL},	/* TC_PARSE */
    {NULL,		TC_OP, 		NULL,	NULL},	/* TC_OP */
    {NULL,		TC_STRING, 	NULL,	NULL},	/* TC_STRING */
    {NULL,		TC_CATCH, 	NULL,	NULL},	/* TC_CATCH */
    {NULL,		TC_FORALL, 	NULL,	NULL},	/* TC_FORALL */
    {NULL,		TC_INT, 	NULL,	NULL},	/* TC_INT */
    {NULL,		TC_FLOAT, 	NULL,	NULL},	/* TC_FLOAT */
    {NULL,		TC_REGEXP, 	NULL,	NULL},	/* TC_REGEXP */
    {NULL,		TC_PTR, 	NULL,	NULL},	/* TC_PTR */
    {NULL,		TC_ARRAY, 	NULL,	NULL},	/* TC_ARRAY */
    {NULL,		TC_STRUCT, 	NULL,	NULL},	/* TC_STRUCT */
    {NULL,		TC_SET, 	NULL,	NULL},	/* TC_SET */
    {NULL,		0, 		NULL,	NULL},	/* UNUSED */
    {NULL,		TC_MEM, 	NULL,	NULL},	/* TC_MEM */
    {NULL,		TC_NULL, 	NULL,	NULL},	/* TC_NULL */
    {NULL,		TC_FUNC, 	NULL,	NULL},	/* TC_FUNC */
    {NULL,		TC_CFUNC, 	NULL,	NULL},	/* TC_CFUNC */
    {NULL,		TC_FILE, 	NULL,	NULL},	/* TC_FILE */
    {NULL,		0, 		NULL,	NULL},	/* TC_REF, UNUSED */
#ifdef ici3
    {NULL,		TC_METHOD, 	NULL,	NULL}	/* TC_METHOD */
#endif
#endif
};

#define	NTYPES	(sizeof typemap/sizeof typemap[0])

/*
 * Initialise the tables at run-time.
 */
void
ici_serialisation_init(int what, int (**func)())
{
    int	i;

#ifdef _WINDOWS
    typemap[0].type  = NULL;
    typemap[1].type  = &pc_type;
    typemap[2].type  = &src_type;
    typemap[3].type  = &parse_type;
    typemap[4].type  = &op_type;
    typemap[5].type  = &string_type;
    typemap[6].type  = &ici_catch_type;
    typemap[7].type  = &forall_type;
    typemap[8].type  = &int_type;
    typemap[9].type  = &float_type;
    typemap[10].type = &regexp_type;
    typemap[11].type = &ptr_type;
    typemap[12].type = &ici_array_type;
    typemap[13].type = &struct_type;
    typemap[14].type = &set_type;
    typemap[16].type = &mem_type;
    typemap[17].type = &null_type;
    typemap[18].type = &ici_func_type;
#ifdef ici3
    typemap[19].type = &ici_cfunc_type;
    typemap[22].type = &ici_method_type;
#endif
    typemap[19].type = &ici_func_type;
#endif

    if (what == ICI_SERIALISATION_SAVE)
	for (i = 0; i < NTYPES; ++i)
	    typemap[i].save = (int (*)(object_t *, file_t *))func[i];
    else
	for (i = 0; i < NTYPES; ++i)
	    typemap[i].restore = (object_t *(*)(file_t *, ici_serialisation_type_t *))func[i];
}

ici_serialisation_type_t *
ici_serialisation_lookup_type(object_t *o)
{
    int		c;
    char	name[40];

    if ((c = (o->o_tcode & 0xf)) != TC_OTHER)
    {
	assert(c > 0);
	assert(c <= NTYPES);
	assert(typemap[c].type != NULL);
	return &typemap[c];
    }
    for (c = 0; c <= NTYPES; ++c)
	if (typemap[c].type == o->o_type)
	    break;
    if (c <= NTYPES)
    {
	if (c == TC_FUNC && iscfunc(o))
	    c = TC_CFUNC;
	return &typemap[c];
    }
    if (chkbuf(40))
	return NULL;
    sprintf(buf, "can't serialise %s object", objname(name, o));
    error = buf;
    return NULL;
}

ici_serialisation_type_t *
ici_serialisation_lookup_type_by_tag(int c)
{
    if (c > 0 && c < NTYPES)
	return &typemap[c];
    if (chkbuf(40))
	return NULL;
    sprintf(buf, "invalid type code in restore, tag = 0x%02X", c);
    error = buf;
    return NULL;
}

file_t *
ici_need_stderr()
{
    file_t		*f;
    static string_t	*string_stderr;

    if (need_string(&string_stderr, "stderr"))
	return NULL;
    f = fileof(fetch(v_top[-1], string_stderr));
    if (!isfile(f))
    {
	error = "stderr is not a file";
	return NULL;
    }
    return f;
}
