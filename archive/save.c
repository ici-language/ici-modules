/**
 *  ICI Object Serialization Module
 *
 *  Copyright (C) 1996, 2010  A.Newman
 *
 */

#include "archive.h"
#include "icistr.h"

#include <netinet/in.h>

static int save(ici_archive_t *ar, ici_obj_t *obj);

/*
 * Functions to write different size datums to the archive stream.
 */

inline
static int
writen(ici_archive_t *ar, void *data, int len)
{
    return (*ar->a_file->f_type->ft_write)(data, len, ar->a_file->f_file) != len;
}

inline
static int
writeb(ici_archive_t *ar, unsigned char abyte)
{
    return writen(ar, &abyte, 1);
}

inline
static int
write32(ici_archive_t *ar, int aword)
{
    int swapped = htonl(aword);
    return writen(ar, &swapped, sizeof swapped);
}

inline
static int
writedbl(ici_archive_t *ar, double adbl)
{
    return writen(ar, &adbl, sizeof adbl);
}

/*
 * Integers are written as 64-bit values in network byte order.
 */
inline
static int
writel(ici_archive_t *ar, long along)
{
    int msw = (int)((along >> 32) & 0xffffffffL);
    int lsw = (int)(along & 0xffffffffL);
    return write32(ar, msw) || write32(ar, lsw);
}

/*
 * Strings are written as a long count of the number of bytes followed
 * by that many bytes representing the character data.  Since ICI uses
 * 8-bit coding this count is also the number of characters.
 */
inline
static int
writestr(ici_archive_t *ar, ici_str_t *s)
{
    return writel(ar, s->s_nchars) || writen(ar, s->s_chars, s->s_nchars);
}

/*
 * Reference to a previously saved object
 */

static int
save_object_ref(ici_archive_t *ar, ici_obj_t *o)
{
    return writeb(ar, ICI_ARCHIVE_TCODE_REF) || writen(ar, &o, sizeof o);
}

static int
save_object_name(ici_archive_t *ar, ici_obj_t *obj)
{
    return ici_archive_insert(ar, obj, obj) || writen(ar, &obj, sizeof obj);
}

/*
 * ici object header
 */

static int
save_obj(ici_archive_t *ar, ici_obj_t *obj)
{
    unsigned char code = obj->o_tcode & 0x1F;
    if (obj->o_flags & ICI_O_ATOM)
        code |= ICI_ARCHIVE_ATOMIC;
    return writeb(ar, code);
}

// NULL

static int
save_null(ici_archive_t *ar, ici_obj_t *obj)
{
    (void)ar;
    (void)obj;
    return 0;
}

// int

static int
save_int(ici_archive_t *ar, ici_obj_t *obj)
{
    return writel(ar, ici_intof(obj)->i_value);
}

// float

static int
save_float(ici_archive_t *ar, ici_obj_t *obj)
{
    double v = ici_floatof(obj)->f_value;
#if ICI_ARCHIVE_LITTLE_ENDIAN_HOST
    ici_archive_byteswap(&v, sizeof v);
#endif
    return writedbl(ar, v);
}

// string

static int
save_string(ici_archive_t *ar, ici_obj_t *obj)
{
    return save_object_name(ar, obj) || writestr(ar, ici_stringof(obj));
}

// regexp

static int
save_regexp(ici_archive_t *ar, ici_obj_t *obj)
{
    ici_regexp_t *re = ici_regexpof(obj);
    int options;

    ici_pcre_info(re->r_re, &options, NULL);
    return save_object_name(ar, obj) || write32(ar, options) || save_string(ar, ici_objof(re->r_pat));
}

// mem

static int
save_mem(ici_archive_t *ar, ici_obj_t *obj)
{
    ici_mem_t *m = ici_memof(obj);
    return save_object_name(ar, obj)
           || writel(ar, m->m_length)
           || write32(ar, m->m_accessz)
           || writen(ar, m->m_base, m->m_length * m->m_accessz);
}

// array

static int
save_array(ici_archive_t *ar, ici_obj_t *obj)
{
    ici_array_t *a = ici_arrayof(obj);
    ici_obj_t **e;

    if (save_object_name(ar, obj) || writel(ar, ici_array_nels(a)))
        return 1;
    for (e = ici_astart(a); e != ici_alimit(a); e = ici_anext(a, e))
    {
        if (save(ar, *e))
            return 1;
    }
    return 0;
}

// set

static int
save_set(ici_archive_t *ar, ici_obj_t *obj)
{
    ici_set_t *s = ici_setof(obj);
    ici_obj_t **e = s->s_slots;

    if (save_object_name(ar, obj) || writel(ar, s->s_nels))
        return 1;
    for (; e - s->s_slots < s->s_nslots; ++e)
    {
        if (*e && save(ar, *e))
            return 1;
    }
    return 0;
}

// struct

static int
save_struct(ici_archive_t *ar, ici_obj_t *obj)
{
    ici_struct_t *s = ici_structof(obj);
    ici_objwsup_t *super = ici_objwsupof(s)->o_super;
    struct ici_sslot *sl;

    if (save_object_name(ar, obj) || save(ar, super ? ici_objof(super) : ici_null) || writel(ar, s->s_nels))
        return 1;
    for (sl = s->s_slots; sl - s->s_slots < s->s_nslots; ++sl)
    {
        if (sl->sl_key && sl->sl_value)
        {
            if (save(ar, sl->sl_key) || save(ar, sl->sl_value))
                return 1;
        }
    }
    return 0;
}

// ptr

static int
save_ptr(ici_archive_t *ar, ici_obj_t *obj)
{
    return save(ar, ici_ptrof(obj)->p_aggr) || save(ar, ici_ptrof(obj)->p_key);
}

// func

static int
save_func(ici_archive_t *ar, ici_obj_t *obj)
{
    ici_func_t *f = ici_funcof(obj);
    ici_struct_t *autos;

    if (obj->o_tcode == ICI_TC_CFUNC)
    {
        ici_cfunc_t *cf = ici_cfuncof(obj);
        int nchars = strlen(cf->cf_name);
        /* fixme: send name as an ICI string */
        return write32(ar, nchars) || writen(ar, cf->cf_name, nchars);
    }

    if (save_object_name(ar, obj) || save(ar, ici_objof(f->f_code)) || save(ar, ici_objof(f->f_args)))
        return 1;

    if ((autos = ici_structof(ici_typeof(f->f_autos)->t_copy(ici_objof(f->f_autos)))) == NULL)
        return 1;
    autos->o_head.o_super = NULL;
    ici_struct_unassign(autos, ICISO(_func_));
    if (save(ar, ici_objof(autos)))
    {
        ici_decref(autos);
        return 1;
    }
    ici_decref(autos);

    return save(ar, ici_objof(f->f_name)) || write32(ar, f->f_nautos);
}

// src

static int
save_src(ici_archive_t *ar, ici_obj_t *obj)
{
    return write32(ar, ici_srcof(obj)->s_lineno) || save(ar, ici_objof(ici_srcof(obj)->s_filename));
}

// op

static int
save_op(ici_archive_t *ar, ici_obj_t *obj)
{
    return
        write32(ar, ici_archive_op_func_code(ici_opof(obj)->op_func))
        ||
        write32(ar, ici_opof(obj)->op_ecode)
        ||
        write32(ar, ici_opof(obj)->op_code);
}

//
// saver
//

typedef struct
{
    ici_obj_t o_head;
    int (*s_fn)(ici_archive_t *, ici_obj_t *);
}
saver_t;

static saver_t *
saverof(ici_obj_t *obj)
{
    return (saver_t *)obj;
}

static unsigned long
mark_saver(ici_obj_t *o)
{
    o->o_flags |= ICI_O_MARK;
    return sizeof (saver_t);
}

static void
free_saver(ici_obj_t *o)
{
    ici_tfree(o, saver_t);
}

static type_t
saver_type =
{
    mark_saver,
    free_saver,
    ici_hash_unique,
    ici_cmp_unique,
    ici_copy_simple,
    ici_assign_fail,
    ici_fetch_fail,
    "saver"
};

static ici_obj_t *
new_saver(int (*fn)(ici_archive_t *, ici_obj_t *))
{
    saver_t *saver;
    static int saver_tcode = 0;

    if (saver_tcode == 0)
    {
        if ((saver_tcode = ici_register_type(&saver_type)) == 0)
            return NULL;
    }

    if ((saver = ici_talloc(saver_t)) != NULL)
    {
        ICI_OBJ_SET_TFNZ(saver, saver_tcode, 0, 1, sizeof (saver_t));
        saver->s_fn = fn;
        ici_rego(saver);
    }

    return ici_objof(saver);
}

static ici_struct_t *
get_archive_map(void)
{
    static ici_struct_t *archive_map = NULL;

    if (archive_map == NULL)
    {
        size_t i;
        struct
        {
            ici_obj_t *name;
            int (*fn)(ici_archive_t *, ici_obj_t *);
        }
        fns[] =
        {
            {ICISO(NULLx), save_null},
            {ICISO(mark), save_null},
            {ICISO(intx), save_int},
            {ICISO(floatx), save_float},
            {ICISO(string), save_string},
            {ICISO(regexp), save_regexp},
            {ICISO(array), save_array},
            {ICISO(set), save_set},
            {ICISO(structx), save_struct},
            {ICISO(mem), save_mem},
            {ICISO(ptr), save_ptr},
            {ICISO(func), save_func},
            {ICISO(src), save_src},
            {ICISO(op), save_op},
        };

        if ((archive_map = ici_struct_new()) == NULL)
            return NULL;

        for (i = 0; i < nels(fns); ++i)
        {
            ici_obj_t *saver;

            if ((saver = new_saver(fns[i].fn)) == NULL)
                goto fail;
            if (ici_assign(archive_map, fns[i].name, saver))
            {
                ici_decref(saver);
                goto fail;
            }
            ici_decref(saver);
        }
    }
    return archive_map;

fail:
    ici_decref(archive_map);
    archive_map = NULL;
    return NULL;
}

static ici_str_t *
tname(ici_obj_t *o)
{
    if (ici_typeof(o)->t_ici_name == NULL)
        ici_typeof(o)->t_ici_name = ici_str_new_nul_term(ici_typeof(o)->t_name);
    return ici_typeof(o)->t_ici_name;
}

static int
save_error(ici_archive_t *ar, ici_obj_t *obj)
{
    (void)ar;

    if (ici_chkbuf(80) == 0)
    {
        sprintf(ici_buf, "%s: unable to save type", ici_typeof(obj)->t_name);
        ici_error = ici_buf;
    }
    return 1;
}

static int
save(ici_archive_t *ar, ici_obj_t *obj)
{
    ici_obj_t *saver;
    int (*fn)(ici_archive_t *, ici_obj_t *);

    if (ici_archive_lookup(ar, obj) != NULL)
        return save_object_ref(ar, obj);

    saver = ici_fetch(ici_objof(get_archive_map()), ici_objof(tname(obj)));
    fn = saver == ici_null ? save_error : saverof(saver)->s_fn;
    return save_obj(ar, obj) || (*fn)(ar, obj);
}

/*
 * archive.save([file, ] any)
 *
 * Save an object to a file in ICI Archive Protocol format.
 *
 * If no file is specified the object is written to the standard output.
 *
 * The archive protocol format is a binary representation of the
 * object that provides sufficient information for the object to be
 * recreated when subsequently restored.
 *
 * Most standard object types may be archived, including functions
 * and self-referential structures.  File objects may NOT be archived.
 *
 * This --topic-- forms part of the --ici-archive-- documentation.
 */
int
ici_archive_f_save(void)
{
    ici_file_t *file;
    ici_obj_t *obj;
    ici_archive_t *ar;
    int failed = 1;

    switch (ICI_NARGS())
    {
    case 2:
        if (ici_typecheck("uo", &file, &obj))
            return 1;
        break;

    case 1:
        if (ici_typecheck("o", &obj))
            return 1;
        if ((file = ici_need_stdout()) == NULL)
            return 1;
        break;

    default:
        return ici_argerror(2);
    }

    if ((ar = ici_archive_start(file, NULL)) != NULL)
    {
        failed = save(ar, obj);
        ici_archive_stop(ar);
    }

    return failed ? failed : ici_null_ret();
}
