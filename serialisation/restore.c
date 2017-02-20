/*
 * $Id: restore.c,v 1.5 2000/11/01 09:46:19 atrn Exp $
 *
 */

#include "serialisation.h"

/*
 * Generic restore error message.
 */
object_t *
ici_restore_error(void)
{
    error = "I/O error during restore";
    return NULL;
}

/*
 * Generic "restore" routine for types that can't be archived.
 * Raises an error.
 */
object_t *
ici_restore_simple(file_t *f, ici_serialisation_type_t *t)
{
    if (t == NULL)
	fprintf(stderr, "restore simple: unknown type\n");
    else if (t->type != NULL)
	fprintf(stderr, "restore simple: %s\n", t->type->t_name);
    else
	fprintf(stderr, "restore simple: tag == %d\n", t->tag);
    error = "attempt to restore un-serialisable object";
    return NULL;
}

/*
 * Read a 32bit int in network byte order
 */
int
ici_restore_long(long *v, file_t *f)
{
#ifndef _WIN32
#ifndef ntohl
    long ntohl();
#endif
#endif
    if (ici_readf(f, (char *)v, sizeof *v) != sizeof *v)
    {
	(void)ici_restore_error();
	return 1;
    }
    *v = ntohl(*v);
    return 0;
}

/*
 * Read some number of bytes from a stream into a buffer. Returns the
 * number of bytes read.
 */
int
ici_readf(file_t *f, char *buf, int nbytes)
{
    int	nb = 0;
    int	c;

    while (nbytes-- > 0 && (c = (*f->f_type->ft_getch)(f->f_file)) != EOF)
    {
        ++nb;
        *buf++ = c;
    }
    return nb;
}

/*
 * Utility for types that may be the target of references.
 */
int
ici_read_name(file_t *f, object_t *o)
{
    long	v;

    return ici_restore_long(&v, f) ? 1 : ici_serialisation_add_ref(v, o);
}

object_t *
ici_restore_array(file_t *f, ici_serialisation_type_t *t)
{
    array_t	*a;
    long	nels;

    if ((a = new_array()) == NULL)
	return NULL;
    if (ici_read_name(f, objof(a)) || ici_restore_long(&nels, f))
	goto fail;
    if (nels < 0)
    {
	error = "-ve number of array elements in restore";
	goto fail;
    }
    if (pushcheck(a, nels))
	goto fail;
    while (nels-- > 0)
    {
	object_t *o;

	if ((o = ici_restore(f)) == NULL)
	    goto fail;
	*a->a_top++ = o;
	decref(o);
    }
    return objof(a);

fail:
    decref(a);
    return NULL;
}

object_t *
ici_restore_catch(file_t *f, ici_serialisation_type_t *t)
{
    catch_t	*c;
    long	v;

    if ((c = new_catch(NULL, 0, 0)) == NULL)
	return NULL;
    if ((c->c_catcher = ici_restore(f)) == NULL)
	goto fail;
    decref(c->c_catcher);
    if (ici_restore_long(&v, f))
	goto fail;
    c->c_odepth = v;
    if (ici_restore_long(&v, f))
	goto fail;
    c->c_vdepth = v;
    return objof(c);

fail:
    decref(c);
    return NULL;
}

object_t *
ici_restore_file(file_t *f, ici_serialisation_type_t *t)
{
    long	len;
    char	name[8];

    if (ici_restore_long(&len, f))
	return NULL;
    if (len < 5 || len > 6)
    {
	error = "invalid file name in restore file object";
	return NULL;
    }
    if (ici_readf(f, name, len) != len)
	return ici_restore_error();
    name[len] = '\0';
    if (!strcmp(name, "stdin"))
	return objof(need_stdin());
    if (!strcmp(name, "stdout"))
	return objof(need_stdout());
    if (!strcmp(name, "stderr"))
	return objof(ici_need_stderr());
    error = "unknown file name in restore file object";
    return NULL;
}

object_t *
ici_restore_float(file_t *f, ici_serialisation_type_t *t)
{
    double	value;
    char	*p;
    int		n = sizeof (double);
    int		c;

#ifdef ICI_SERIALISATION_LITTLE_ENDIAN
    for (p = (char *)&value + sizeof (double) - 1; n-- > 0; --p)
	if ((c = (*f->f_type->ft_getch)(f->f_file)) == EOF)
	    return ici_restore_error();
	else
	    *p = c;
#else
    for (p = (char *)&value; n-- > 0; ++p)
	if ((c = (*f->f_type->ft_getch)(f->f_file)) == EOF)
	    return ici_restore_error();
	else
	    *p = c;
#endif
    return objof(new_float(value));
}

object_t *
ici_restore_forall(file_t *f, ici_serialisation_type_t *t)
{
    forall_t	*fa;
    long	v;

    if ((fa = talloc(forall_t)) == NULL)
	return NULL;
    objof(fa)->o_type  = &forall_type;
    objof(fa)->o_tcode = TC_FORALL;
    objof(fa)->o_flags = 0;
    objof(fa)->o_nrefs = 0;
    fa->fa_aggr	       = NULL;
    fa->fa_code	       = NULL;
    fa->fa_vaggr       = NULL;
    fa->fa_vkey	       = NULL;
    fa->fa_kaggr       = NULL;
    fa->fa_kkey	       = NULL;
    rego(fa);
    if (ici_restore_long(&v, f))
	goto fail;
    fa->fa_index = v;
    if
    (
	(fa->fa_aggr = ici_restore(f)) == NULL
	||
	(fa->fa_code = ici_restore(f)) == NULL
	||
	(fa->fa_vaggr = ici_restore(f)) == NULL
	||
	(fa->fa_vkey = ici_restore(f)) == NULL
	||
	(fa->fa_kaggr = ici_restore(f)) == NULL
	||
	(fa->fa_kkey = ici_restore(f)) == NULL
    )
	goto fail;
    decref(fa->fa_aggr);
    decref(fa->fa_code);
    decref(fa->fa_vaggr);
    decref(fa->fa_vkey);
    decref(fa->fa_kaggr);
    decref(fa->fa_kkey);
    return objof(fa);

fail:
    decref(fa);
    if (fa->fa_aggr != NULL)
	decref(fa->fa_aggr);
    if (fa->fa_code != NULL)
	decref(fa->fa_code);
    if (fa->fa_vaggr != NULL)
	decref(fa->fa_vaggr);
    if (fa->fa_vkey != NULL)
	decref(fa->fa_vkey);
    if (fa->fa_kaggr != NULL)
	decref(fa->fa_kaggr);
    if (fa->fa_kkey != NULL)
	decref(fa->fa_kkey);
    return NULL;
}

object_t *
ici_restore_cfunc(file_t *f, ici_serialisation_type_t *t)
{
    string_t	*s;
    long	len;
    object_t	*o;

    if (ici_restore_long(&len, f))
	return NULL;
    if ((s = new_string(len)) != NULL)
    {
	if (ici_readf(f, s->s_chars, len) != len)
	{
	    decref(s);
	    return ici_restore_error();
	}
	s->s_chars[len] = '\0';
	ici_hash_string(objof(s));
    }
    else
    {
	while (len-- > 0)
	    (void)(*f->f_type->ft_getch)(f->f_file);
	return NULL;
    }
    o = fetch(v_top[-1], atom(objof(s), 0));
    decref(s);
    if (o == objof(&o_null))
    {
	if (!chkbuf(40))
	{
	    sprintf(buf, "\"%s\" undefined", s->s_chars);
	    error = buf;
	}
	return NULL;
    }
    return o;
}

#ifdef ici3
object_t *
ici_restore_method(file_t *f, ici_serialisation_type_t *t)
{
    object_t	*subject;
    object_t	*callable;
    object_t	*o;

    if ((subject = ici_restore(f)) == NULL)
	return NULL;
    if
    (
	isstruct(subject)
	&&
	structof(subject)->s_super != NULL
	&&
	structof(v_top[-1])->s_super != NULL
    )
	structof(subject)->s_super->s_super = structof(v_top[-1])->s_super->s_super;
    if ((callable = ici_restore(f)) == NULL)
    {
	decref(subject);
	return NULL;
    }
    o = objof(ici_new_method(subject, callable));
    return o;
}
#endif

object_t *
ici_restore_func(file_t *f, ici_serialisation_type_t *t)
{
    func_t	*fn;
    struct_t	*autos;

    if ((fn = new_func()) == NULL)
	return NULL;
    if ((fn->f_code = arrayof(ici_restore(f))) == NULL)
	goto fail;
    decref(fn->f_code);
    if ((fn->f_args = arrayof(ici_restore(f))) == NULL)
	goto fail;
    decref(fn->f_args);
    if ((autos = structof(ici_restore(f))) == NULL)
	goto fail;
    decref(autos);
    fn->f_autos = autos;
    autos->s_super = structof(v_top[-1])->s_super;
    if ((fn->f_name = stringof(ici_restore(f))) == NULL)
	goto fail;
    decref(fn->f_name);
    return objof(fn);

fail:
    decref(f);
    return NULL;
}

object_t *
ici_restore_int(file_t *f, ici_serialisation_type_t *t)
{
    long v;

    if (ici_restore_long(&v, f))
	return NULL;
    return objof(new_int(v));
}

object_t *
ici_restore_mem(file_t *f, ici_serialisation_type_t *t)
{
    mem_t	*m;
    void	*b;
    long	len;
    long	accessz;
    long	tmp;

    if
    (
	ici_restore_long(&len, f)
	||
	ici_restore_long(&accessz, f)
	||
	(b = ici_raw_alloc(len * accessz)) == NULL
    )
	return NULL;
    if
    (
	ici_restore_long(&tmp, f)
	||
	ici_readf(f, (char *)b, len * accessz) != tmp
	||
	(m = new_mem(b, len, accessz, ici_raw_free)) == NULL
    )
    {
	ici_free(b);
	return NULL;
    }
    return objof(m);
}

object_t *
ici_restore_null(file_t *f, ici_serialisation_type_t *t)
{
    incref(&o_null);
    return objof(&o_null);
}

object_t *
ici_restore_op(file_t *f, ici_serialisation_type_t *t)
{
    int		(*fn)() = NULL;
    int		ecode = 0;
    int		code = 0;
    long	n;

    if ((ecode = (*f->f_type->ft_getch)(f->f_file)) == EOF)
	return ici_restore_error();
    switch (ecode)
    {
    case OP_OTHER:
	if ((n = (*f->f_type->ft_getch)(f->f_file)) == EOF)
	    return ici_restore_error();
	if (n < 0 || n >= ici_serialisation_nopfunc)
	{
	    error = "invalid operator function code in restore()";
	    return NULL;
	}
	fn = ici_serialisation_opfunc[n];
	/* FALLTHROUGH */

    default:
	if (ici_restore_long(&n, f))
	    return ici_restore_error();
	code = n;
	break;
    }
    return objof(new_op(fn, ecode, code));
}

object_t *
ici_restore_pc(file_t *f, ici_serialisation_type_t *t)
{
    array_t	fake;
    pc_t	*pc;
    long	ofs;

#ifdef ici3
    if ((pc = new_pc(&fake, 0)) == NULL)
#else
    if ((pc = new_pc(&fake)) == NULL)
#endif
	return NULL;
    if ((pc->pc_code = arrayof(ici_restore(f))) == NULL || ici_restore_long(&ofs, f))
    {
	decref(pc);
	return NULL;
    }
    decref(pc->pc_code);
    pc->pc_next = pc->pc_code->a_base + ofs;
    return objof(pc);
}

object_t *
ici_restore_ptr(file_t *f, ici_serialisation_type_t *t)
{
    object_t	*a, *k, *p;

    if ((a = ici_restore(f)) == NULL)
	return NULL;
    if ((k = ici_restore(f)) == NULL)
	goto fail;
    if ((p = objof(new_ptr(a, k))) == NULL)
    {
	decref(k);
	goto fail;
    }
    decref(a);
    decref(k);
    return p;

fail:
    decref(a);
    return NULL;
}

object_t *
ici_restore_regexp(file_t *f, ici_serialisation_type_t *t)
{
    string_t	*s;
    long	v;

    if (ici_restore_long(&v, f))
	return NULL;
    if ((s = stringof(ici_restore_string(f, NULL))) == NULL)
	return NULL;
    return objof(new_regexp(s, v & 0x03FF));
}

object_t *
ici_restore_set(file_t *f, ici_serialisation_type_t *t)
{
    set_t	*s;
    long	nels;
    object_t	*o;

    if ((s = new_set()) == NULL || ici_read_name(f, objof(s)))
	goto fail;
    if (ici_restore_long(&nels, f))
	goto fail;
    if (nels < 0)
    {
	error = "-ve number of set elements in restore";
	goto fail;
    }
    while (nels-- > 0)
    {
	if ((o = ici_restore(f)) == NULL)
	    goto fail;
	if (assign(s, o, objof(o_one)))
	{
	    decref(o);
	    goto fail;
	}
	decref(o);
    }
    return objof(s);

fail:
    if (s != NULL)
	decref(s);
    return NULL;
}

object_t *
ici_restore_src(file_t *f, ici_serialisation_type_t *t)
{
    src_t	*s;
    long	n;

    if ((s = new_src(0, NULL)) == NULL)
	return NULL;
    if (ici_restore_long(&n, f))
	goto fail;
    s->s_lineno = n;
    if ((s->s_filename = stringof(ici_restore(f))) == NULL)
	goto fail;
    if (objof(s->s_filename) == objof(&o_null))
	s->s_filename = NULL;
    else
	decref(s->s_filename);
    return objof(s);

fail:
    decref(s);
    return NULL;
}

object_t *
ici_restore_string(file_t *f, ici_serialisation_type_t *t)
{
    string_t	*s;
    long	len;

    if (ici_restore_long(&len, f))
	return NULL;
    if ((s = new_string(len)) != NULL)
    {
	if (ici_readf(f, s->s_chars, len) != len)
	{
	    decref(s);
	    return ici_restore_error();
	}
	s->s_chars[len] = '\0';
	ici_hash_string(objof(s));
    }
    else
    {
	while (len-- > 0)
	    (void)(*f->f_type->ft_getch)(f->f_file);
	return NULL;
    }
    return atom(objof(s), 1);
}

object_t *
ici_restore_struct(file_t *f, ici_serialisation_type_t *t)
{
    struct_t	*s;
    long	nels;

    if ((s = new_struct()) == NULL || ici_read_name(f, objof(s)))
	goto fail;
    if ((s->s_super = structof(ici_restore(f))) == NULL)
	goto fail;
    if (objof(s->s_super) == objof(&o_null))
	s->s_super = NULL;
    else
	decref(s->s_super);
    if (ici_restore_long(&nels, f))
	goto fail;
    if (nels < 0)
    {
	error = "-ve number of struct key/pairs in restore";
	goto fail;
    }
    while (nels-- > 0)
    {
	object_t	*k;
	object_t	*v;

	if ((k = ici_restore(f)) == NULL)
	    goto fail;
	if ((v = ici_restore(f)) == NULL)
	{
	    decref(k);
	    goto fail;
	}
	if (assign(s, k, v))
	{
	    decref(v);
	    decref(k);
	    goto fail;
	}
	decref(k);
	decref(v);
    }
    return objof(s);

fail:
    if (s != NULL)
	decref(s);
    return NULL;
}


/*
 * Read a "reference" from a stream
 */
long
ici_restore_ref(file_t *f, ici_serialisation_type_t *t)
{
    long	v;

    if (ici_restore_long(&v, f))
	return 0;
    return v;
}

/*
 * Restore an object from a stream
 */
object_t *
ici_restore(file_t *f)
{
    unsigned int		 c;
    object_t			*o;
    int				atomic;
    ici_serialisation_type_t	*t;

    if ((c = (*f->f_type->ft_getch)(f->f_file)) == EOF)
    {
	error = "end of file in restore";
	return NULL;
    }
    atomic = (c & TC_ATOMIC);
    c &= ~TC_ATOMIC;
    if (c == TC_REF)
    {
	long	name;

	name = ici_restore_ref(f, NULL);
	if (name == 0)
	    return NULL;
	if ((o = ici_serialisation_find_ref(name)) == NULL)
	{
	    error = "bad object reference in serialisation protocol";
	    return NULL;
	}
	incref(o);
	return o;
    }
    if ((t = ici_serialisation_lookup_type_by_tag(c)) == NULL)
    {
	sprintf(buf, "unknown type code in restore, tag = 0x%02X", c);
	error = buf;
	return NULL;
    }
    o = (*(object_t *(*)(file_t *, void *))t->restore)(f, t);
    if (atomic)
    {
	object_t *p;
	p = atom(o, 1);
	if (p != o)
	{
	    ici_serialisation_mod_ref(o, p);
	    o = p;
	}
    }
    return o;
}

int
ici_serialisation_f_restore(void)
{
    struct_t	*scp;
    struct_t	*oscp;
    file_t	*f;
    object_t	*o;

    switch (NARGS())
    {
    case 2:
	if (ici_typecheck("ud", &f, &scp))
	    return 1;
	break;

    case 1:
	scp = structof(v_top[-1]);
	if (ici_typecheck("u", &f))
	{
	    if ((f = need_stdin()) == NULL)
		return 1;
	    if (ici_typecheck("d", &scp))
		return 1;
	}
	break;

    case 0:
	scp = structof(v_top[-1]);
	if ((f = need_stdin()) == NULL)
	    return 1;
	break;

    default:
	return ici_argcount(1);
    }
    ici_serialisation_start();
    incref(scp);
    oscp = structof(v_top[-1]);
    incref(oscp);
    v_top[-1] = objof(scp);
    o = ici_restore(f);
    v_top[-1] = objof(oscp);
    decref(oscp);
    decref(scp);
    ici_serialisation_end(ICI_SERIALISATION_RESTORE);
    return o == NULL ? 1 : ici_ret_with_decref(o);
}

object_t *
ici_restore_library_init(void)
{
    static object_t	*(*funcs[])(file_t *, ici_serialisation_type_t *) =
    {
	ici_restore_simple,	/* TC_OTHER */
	ici_restore_pc,		/* TC_PC */
	ici_restore_src,	/* TC_SRC */
	ici_restore_simple,	/* TC_PARSE */
	ici_restore_op,		/* TC_OP */
	ici_restore_string,	/* TC_STRING */
	ici_restore_catch,	/* TC_CATCH */
	ici_restore_forall,	/* TC_FORALL */
	ici_restore_int,	/* TC_INT */
	ici_restore_float,	/* TC_FLOAT */
	ici_restore_regexp,	/* TC_REGEXP */
	ici_restore_ptr,	/* TC_PTR */
	ici_restore_array,	/* TC_ARRAY */
	ici_restore_struct,	/* TC_STRUCT */
	ici_restore_set,	/* TC_SET */
	ici_restore_simple,	/* UNUSED */
	ici_restore_mem,	/* TC_MEM */
	ici_restore_null,	/* TC_NULL */
	ici_restore_func,	/* TC_FUNC */
	ici_restore_cfunc,	/* TC_CFUNC */
	ici_restore_file,	/* TC_FILE */
	ici_restore_simple,	/* TC_REF */
#ifdef ici3
	ici_restore_method	/* TC_METHOD */
#endif
    };

    static cfunc_t cfunc = {CF_OBJ, "restore", ici_serialisation_f_restore};

    ici_serialisation_init(ICI_SERIALISATION_RESTORE, (int (**)())funcs);
#ifdef ici3
    cfunc.o_head.o_type = &ici_cfunc_type;
#else
    cfunc.o_head.o_type = &func_type;
#endif
    return objof(&cfunc);
}
