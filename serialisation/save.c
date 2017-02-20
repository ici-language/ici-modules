/*
 * $Id: save.c,v 1.5 2000/11/01 09:46:19 atrn Exp $
 *
 * Output side of serialisation.
 */

#include "serialisation.h"

/*
 * Generic save error message.
 */
int
ici_save_error(void)
{
    error = "I/O error during save";
    return 1;
}

/*
 * Generic "save" routine for types that can't be archived. Raises
 * an error.
 */
int
ici_save_simple(object_t *o, file_t *f)
{
    char n[30];

    if (!chkbuf(50))
    {
	sprintf(buf, "attempt to save a %s", objname(n, o));
	error = buf;
    }
    return 1;
}

/*
 * Save an integer along the stream.
 */
int
ici_save_long(long n, file_t *f)
{
#ifndef _WIN32
#ifndef htonl
    long htonl();
#endif
#endif
    n = htonl(n);
    if ((*f->f_type->ft_write)((char *)&n, sizeof n, f->f_file) != sizeof n)
	return ici_save_error();
    return 0;
}

/*
 * Utility for types that could be targets of references.
 */
int
ici_save_name(object_t *p, file_t *f)
{
    if (ici_save_long((long)p, f) || ici_serialisation_add_ref((long)p, p))
	return 1;
    return 0;
}

/*
 * Utility to save "counted" strings
 */
int
ici_save_len_str(long n, char *s, file_t *f)
{
    if (ici_save_long(n, f) || (*f->f_type->ft_write)(s, n, f->f_file) != n)
	return 1;
    return 0;
}

/*
 * Write the object or NULL if NULL object pointer
 */
int
ici_save_or_null(object_t *o, file_t *f)
{
    if (o == NULL)
	return ici_save(objof(&o_null), f);
    return ici_save(o, f);
}

int
ici_save_array(object_t *o, file_t *f)
{
    long nels = arrayof(o)->a_top - arrayof(o)->a_base;
    long i;

    o->o_flags |= O_SAVED;
    if (ici_save_name(o, f) || ici_save_long(nels, f))
	return 1;
    for (i = 0; i < nels; ++i)
	if (ici_save(arrayof(o)->a_base[i], f))
	    return 1;
    return 0;
}

int
ici_save_catch(object_t *o, file_t *f)
{
    return
	ici_save_or_null(catchof(o)->c_catcher, f)
	||
	ici_save_long(catchof(o)->c_odepth, f)
	||
	ici_save_long(catchof(o)->c_vdepth, f);
}

int
ici_save_file(object_t *o, file_t *f)
{
    char	*name;

    if (fileof(o) == need_stdin())
	name = "stdin";
    else if (fileof(o) == need_stdout())
	name = "stdout";
    else if (fileof(o) == ici_need_stderr())
	name = "stderr";
    else
    {
#if NOTDEF
	extern ftype_t	stdio_ftype;
	extern ftype_t	popen_ftype;
	extern ftype_t	string_ftype;
	/* XXX Fix it so string and memory files can be archived? */
#endif
	error = "can't save general file objects";
	return 1;
    }
    return ici_save_len_str(strlen(name), name, f);
}

/*
 * XXX This is totally busted!
 */
int
ici_save_float(object_t *o, file_t *fi)
{
    char	*p;
    int		n = sizeof (double);

#ifdef ICI_SERIALISATION_LITTLE_ENDIAN
    for (p = (char *)&floatof(o)->f_value + sizeof (double) - 1; n-- > 0; --p)
    {
        if ((*fi->f_type->ft_putch)(*p, fi->f_file) == EOF)
	    return ici_save_error();
    }
#else
    for (p = (char *)&floatof(o)->f_value; n-- > 0; ++p)
    {
        if ((*fi->f_type->ft_putch)(*p, fi->f_file) == EOF)
	    return ici_save_error();
    }
#endif
    return 0;
}

int
ici_save_forall(object_t *o, file_t *f)
{
    return
	ici_save_long(forallof(o)->fa_index, f)
	||
	ici_save_or_null(forallof(o)->fa_aggr, f)
	||
	ici_save_or_null(forallof(o)->fa_code, f)
	||
	ici_save_or_null(forallof(o)->fa_vaggr, f)
	||
	ici_save_or_null(forallof(o)->fa_vkey, f)
	||
	ici_save_or_null(forallof(o)->fa_kaggr, f)
	||
	ici_save_or_null(forallof(o)->fa_kkey, f);
}

int
ici_save_cfunc(object_t *o, file_t *f)
{
    return ici_save_len_str(strlen(cfuncof(o)->cf_name),
				cfuncof(o)->cf_name,
				f);
}

#ifdef ici3
int
ici_save_method(object_t *o, file_t *f)
{
    int		rc;
    struct_t	*supsup = NULL;

    if (isstruct(methodof(o)->m_subject) && structof(methodof(o)->m_subject)->s_super != NULL)
    {
	supsup = structof(methodof(o)->m_subject)->s_super->s_super;
	if (supsup != NULL)
	{
	    incref(supsup);
	    structof(methodof(o)->m_subject)->s_super->s_super = NULL;
	}
    }
    rc = ici_save(objof(methodof(o)->m_subject), f);
    if (supsup != NULL)
    {
	structof(methodof(o)->m_subject)->s_super->s_super = supsup;
	decref(supsup);
    }
    if (rc == 0)
        rc = ici_save(objof(methodof(o)->m_callable), f);
    return rc;
}
#endif

int
ici_save_func(object_t *o, file_t *f)
{
    int		rc;
    struct_t	*statics;

    if
    (
	ici_save(objof(funcof(o)->f_code), f)
	||
	ici_save(objof(funcof(o)->f_args), f)
    )
	return 1;
    statics = funcof(o)->f_autos->s_super;
    incref(statics);
    funcof(o)->f_autos->s_super = NULL;
    rc = ici_save(objof(funcof(o)->f_autos), f);
    funcof(o)->f_autos->s_super = statics;
    decref(statics);
    if (rc || ici_save(objof(funcof(o)->f_name), f))
	return 1;
    return 0;
}

int
ici_save_int(object_t *o, file_t *f)
{
    return ici_save_long(intof(o)->i_value, f);
}

int
ici_save_mem(object_t *o, file_t *f)
{
    return ici_save_long(memof(o)->m_length, f)
	   ||
	   ici_save_long(memof(o)->m_accessz, f)
	   ||
	   ici_save_len_str
	   (
	       memof(o)->m_length * memof(o)->m_accessz,
	       memof(o)->m_base,
	       f
	   );
}

int
ici_save_null(object_t *o, file_t *f)
{
    return 0;
}

int
ici_save_op(object_t *o, file_t *f)
{
    int	i;

    if ((*f->f_type->ft_putch)(opof(o)->op_ecode & 255, f->f_file) == EOF)
	return ici_save_error();
    switch (opof(o)->op_ecode)
    {
    case OP_OTHER:
	for (i = 0; i < ici_serialisation_nopfunc; ++i)
	    if (ici_serialisation_opfunc[i] == opof(o)->op_func)
		break;
	if (i == ici_serialisation_nopfunc)
	{
	    error = "unknown operator in save operation";
	    return 1;
	}
	if ((*f->f_type->ft_putch)(i, f->f_file) == EOF)
	    return ici_save_error();
	/* FALLTHROUGH */

    default:
	if (ici_save_long(opof(o)->op_code, f))
	    return 1;
	break;
    }
    return 0;
}

int
ici_save_pc(object_t *o, file_t *f)
{
    return
	ici_save(objof(pcof(o)->pc_code), f)
	||
	ici_save_long(pcof(o)->pc_next - pcof(o)->pc_code->a_base, f);
}

int
ici_save_ptr(object_t *o, file_t *f)
{
    return ici_save(ptrof(o)->p_aggr, f) || ici_save(ptrof(o)->p_key, f);
}

int
ici_save_regexp(object_t *o, file_t *f)
{
    real_pcre	*pre = (real_pcre *)regexpof(o)->r_re;

    if (ici_save_long(pre->options, f))
	return 1;
    if (ici_save_len_str(regexpof(o)->r_pat->s_nchars, regexpof(o)->r_pat->s_chars, f))
	return 1;
    return 0;
}

int
ici_save_set(object_t *o, file_t *f)
{
    object_t	**po;

    o->o_flags |= O_SAVED;
    if (ici_save_name(o, f))
	return 1;
    if (ici_save_long(setof(o)->s_nels, f))
	return 1;
    for
    (
	po = &setof(o)->s_slots[setof(o)->s_nslots - 1];
	po >= setof(o)->s_slots;
	--po
    )
    {
	if (*po != NULL && ici_save(*po, f))
	    return 1;
    }
    return 0;
}

int
ici_save_src(object_t *o, file_t *f)
{
    if (ici_save_long(srcof(o)->s_lineno, f))
	return 1;
    return ici_save_or_null(objof(srcof(o)->s_filename), f);
}

int
ici_save_string(object_t *o, file_t *f)
{
    return ici_save_len_str(stringof(o)->s_nchars, stringof(o)->s_chars, f);
}

int
ici_save_struct(object_t *o, file_t *f)
{
    slot_t	*sl;

    o->o_flags |= O_SAVED;
    if
    (
	ici_save_name(o, f)
	||
	ici_save_or_null(objof(structof(o)->s_super), f)
	||
	ici_save_long(structof(o)->s_nels, f)
    )
	return 1;
    for
    (
	sl = structof(o)->s_slots;
	sl - structof(o)->s_slots < structof(o)->s_nslots;
	++sl
    )
    {
	if
	(
	    sl->sl_key != NULL
	    &&
	    (ici_save(sl->sl_key, f) || ici_save(sl->sl_value, f))
	)
	    return 1;
    }
    return 0;
}

/*
 * Write a "reference" on the stream
 */
int
ici_save_ref(void *o, file_t *f)
{
    if
    (
	(*f->f_type->ft_putch)(TC_REF, f->f_file) == EOF
	||
	ici_save_long((long)o, f)
    )
	return ici_save_error();
    return 0;
}

/*
 * Save the object to the stream
 */
int
ici_save(object_t *o, file_t *f)
{
    ici_serialisation_type_t	*t;

    if (o->o_flags & O_SAVED)
	return ici_save_ref(o, f);
    if ((t = ici_serialisation_lookup_type(o)) == NULL)
	return 1;
    if ((*f->f_type->ft_putch)((unsigned char )t->tag, f->f_file) == EOF)
	return ici_save_error();
    return (*(int (*)(object_t *, file_t *))t->save)(o, f);
}

int
ici_serialisation_f_save(void)
{
    object_t	*o;
    file_t	*f;
    int		rc;

    switch (NARGS())
    {
    case 1:
	if (ici_typecheck("o", &o))
	    return 1;
	if ((f = need_stdout()) == NULL)
	    return 1;
	break;

    case 2:
	if (ici_typecheck("ou", &o, &f))
	    return 1;
	break;

    default:
	return ici_argcount(2);
    }
    ici_serialisation_start();
    rc = ici_save(o, f);
    ici_serialisation_end(ICI_SERIALISATION_SAVE);
    return rc ? rc : ici_ret_no_decref(o);
}

object_t *
ici_save_library_init(void)
{
    static int	(*funcs[])(object_t *, file_t *) =
    {
	ici_save_simple,/* TC_OTHER */
	ici_save_pc,	/* TC_PC */
	ici_save_src,	/* TC_SRC */
	ici_save_simple,/* TC_PARSE */
	ici_save_op,	/* TC_OP */
	ici_save_string,/* TC_STRING */
	ici_save_catch,	/* TC_CATCH */
	ici_save_forall,/* TC_FORALL */
	ici_save_int,	/* TC_INT */
	ici_save_float,	/* TC_FLOAT */
	ici_save_regexp,/* TC_REGEXP */
	ici_save_ptr,	/* TC_PTR */
	ici_save_array,	/* TC_ARRAY */
	ici_save_struct,/* TC_STRUCT */
	ici_save_set,	/* TC_SET */
	ici_save_simple,/* UNUSED */
	ici_save_mem,	/* TC_MEM */
	ici_save_null,	/* TC_NULL */
	ici_save_func,	/* TC_FUNC */
	ici_save_cfunc,	/* TC_CFUNC */
	ici_save_file,	/* TC_FILE */
	ici_save_simple,/* TC_REF */
#ifdef ici3
	ici_save_method,/* TC_METHOD */
#endif
    };

    static cfunc_t cfunc = {CF_OBJ, "save", ici_serialisation_f_save};

    ici_serialisation_init(ICI_SERIALISATION_SAVE, (int (**)())funcs);
#ifdef ici3
    cfunc.o_head.o_type = &ici_cfunc_type;
#else
    cfunc.o_head.o_type = &func_type;
#endif
    return objof(&cfunc);
}
