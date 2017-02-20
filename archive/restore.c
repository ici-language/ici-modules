/**
 *  ICI Object Serialization Module
 *
 *  Copyright (C) 1996, 2010  A.Newman
 *
 */

#include "archive.h"
#include "icistr.h"
#include <assert.h>

#include <netinet/in.h>

static ici_obj_t *restore(ici_archive_t *);

static int
readn(ici_archive_t *ar, void *buf, int len)
{
    int (*get)(void *) = ar->a_file->f_type->ft_getch;
    void *file = ar->a_file->f_file;
    char *p = buf;
    int ch;

    for (p = buf; len-- > 0; *p++ = ch)
        if ((ch = get(file)) == -1)
            return ici_set_error("end-of-file");
    return 0;
}

inline
static int
read8(ici_archive_t *ar, char *abyte)
{
    return readn(ar, abyte, 1);
}

inline
static int
read32(ici_archive_t *ar, int *aword)
{
    int tmp;
    if (readn(ar, &tmp, sizeof tmp))
    	return 1;
    *aword = ntohl(tmp);
    return 0;
}

inline
static int
readl(ici_archive_t *ar, long *along)
{
    int lsw;
    int msw;

    if (read32(ar, &msw))
        return 1;
    if (read32(ar, &lsw))
        return 1;
    *along = ((long)(msw) << 32L) | (long)lsw;
    return 0;
}

inline
static int
readdbl(ici_archive_t *ar, double *dbl)
{
    return readn(ar, dbl, sizeof *dbl);
}

static int
restore_obj(ici_archive_t *ar, char *flags)
{
    char tcode;

    if (read8(ar, &tcode))
    	return -1;
    *flags = tcode & ICI_ARCHIVE_ATOMIC ? ICI_O_ATOM : 0;
    tcode &= ~ICI_ARCHIVE_ATOMIC;
    return tcode;
}

inline
static int
restore_object_name(ici_archive_t *ar, ici_obj_t **name)
{
    return readn(ar, name, sizeof *name);
}

static ici_obj_t *
restore_error(ici_archive_t *ar)
{
    (void)ar;
    ici_error = "unable to restore object";
    return NULL;
}

// null

static ici_obj_t *
restore_null(ici_archive_t *ar)
{
    (void)ar;
    return ici_null;
}

// int

static ici_obj_t *
restore_int(ici_archive_t *ar)
{
    long        value;

    if (readl(ar, &value))
        return NULL;
    return ici_objof(ici_int_new(value));
}

// float

static ici_obj_t *
restore_float(ici_archive_t *ar)
{
    double val;

    if (readdbl(ar, &val))
        return NULL;
#if ICI_ARCHIVE_LITTLE_ENDIAN_HOST
    ici_archive_byteswap(&val, sizeof val);
#endif
    return ici_objof(ici_float_new(val));
}

// string

static ici_obj_t *
restore_string(ici_archive_t *ar)
{
    ici_str_t *s;
    long len;
    ici_obj_t *name;
    ici_obj_t *obj;

    if (restore_object_name(ar, &name))
        return NULL;
    if (readl(ar, &len))
        return NULL;
    if ((s = ici_str_alloc(len)) == NULL)
        return NULL;
    if (readn(ar, s->s_chars, len))
        goto fail;
    ici_hash_string(ici_objof(s));
    if ((obj = ici_atom(ici_objof(s), 1)) == NULL)
        goto fail;
    if (ici_archive_insert(ar, name, obj))
    {
        ici_decref(obj);
        goto fail;
    }
    return obj;

fail:
    ici_decref(s);
    return NULL;
}


// regexp

static ici_obj_t *
restore_regexp(ici_archive_t *ar)
{
    ici_obj_t *r;
    int options;
    ici_str_t *s;
    ici_obj_t *name;

    if (restore_object_name(ar, &name))
        return NULL;
    if (read32(ar, &options))
        return NULL;
    if ((s = ici_stringof(restore_string(ar))) == NULL)
        return NULL;
    r = ici_objof(ici_regexp_new(s, options));
    if (r == NULL)
    {
        ici_decref(s);
        return NULL;
    }
    ici_decref(s);
    if (ici_archive_insert(ar, name, r))
    {
        ici_decref(r);
        return NULL;
    }
    return r;
}

// mem

static ici_obj_t *
restore_mem(ici_archive_t *ar)
{
    long len;
    int accessz;
    long sz;
    void *p;
    ici_mem_t *m = 0;
    ici_obj_t *name;

    if (restore_object_name(ar, &name) || readl(ar, &len) || read32(ar, &accessz))
        return NULL;
    sz = len * accessz;
    if ((p = ici_alloc(sz)) != NULL)
    {
        if ((m = ici_mem_new(p, len, accessz, ici_free)) == NULL)
            ici_free(p);
        else if (readn(ar, p, sz) || ici_archive_insert(ar, name, ici_objof(m)))
        {
            ici_decref(m);
            m = NULL;
        }
    }
    return ici_objof(m);
}

// array

static ici_obj_t *
restore_array(ici_archive_t *ar)
{
    long n;
    ici_array_t *a;
    ici_obj_t *name;

    if (restore_object_name(ar, &name))
        return NULL;
    if (readl(ar, &n))
        return NULL;
    if ((a = ici_array_new(n)) == NULL)
        return NULL;
    if (ici_archive_insert(ar, name, ici_objof(a)))
        goto fail;
    for (; n > 0; --n)
    {
        ici_obj_t *o;

        if ((o = restore(ar)) == NULL)
            goto fail1;
        if (ici_array_push(a, o))
        {
            ici_decref(o);
            goto fail1;
        }
    }
    return ici_objof(a);

fail1:
    ici_archive_uninsert(ar, name);

fail:
    ici_decref(a);
    return NULL;
}

// set

static ici_obj_t *
restore_set(ici_archive_t *ar)
{
    ici_set_t *s;
    long n;
    long i;
    ici_obj_t *name;

    if (restore_object_name(ar, &name))
        return NULL;
    if ((s = ici_set_new()) == NULL)
        return NULL;
    if (ici_archive_insert(ar, name, ici_objof(s)))
        goto fail;

    if (readl(ar, &n))
        goto fail1;
    for (i = 0; i < n; ++i)
    {
        ici_obj_t *o;

        if ((o = restore(ar)) == NULL)
            goto fail1;
        if (ici_assign(ici_objof(s), ici_objof(o), ici_objof(ici_one)))
        {
            ici_decref(o);
            goto fail1;
        }
        ici_decref(o);
    }
    return ici_objof(s);

fail1:
    ici_archive_uninsert(ar, name);

fail:
    ici_decref(s);
    return NULL;
}

// struct

static ici_obj_t *
restore_struct(ici_archive_t *ar)
{
    ici_struct_t *s;
    ici_obj_t *super;
    long n;
    long i;
    ici_obj_t *name;

    if (restore_object_name(ar, &name))
        return NULL;
    if ((s = ici_struct_new()) == NULL)
        return NULL;

    if (ici_archive_insert(ar, name, ici_objof(s)))
        goto fail;

    if ((super = restore(ar)) == NULL)
        goto fail1;
    if (super != ici_null)
    {
        ici_objwsupof(s)->o_super = ici_objwsupof(super);
        ici_decref(super);
    }

    if (readl(ar, &n))
        goto fail1;
    for (i = 0; i < n; ++i)
    {
        ici_obj_t *key;
        ici_obj_t *value;
        int failed;

        if ((key = restore(ar)) == NULL)
            goto fail1;
        if ((value = restore(ar)) == NULL)
        {
            ici_decref(key);
            goto fail1;
        }
        failed = ici_assign(ici_objof(s), key, value);
        ici_decref(key);
        ici_decref(value);
        if (failed)
            goto fail1;
    }
    return ici_objof(s);

fail1:
    ici_archive_uninsert(ar, name);

fail:
    ici_decref(s);
    return NULL;
}

// ptr

static ici_obj_t *
restore_ptr(ici_archive_t *ar)
{
    ici_obj_t *aggr;
    ici_obj_t *key;
    ici_ptr_t *ptr;

    if ((aggr = restore(ar)) == NULL)
        return NULL;
    if ((key = restore(ar)) == NULL)
    {
        ici_decref(aggr);
        return NULL;
    }
    ptr = ici_ptr_new(aggr, key);
    ici_decref(aggr);
    ici_decref(key);
    return ptr ? ici_objof(ptr) : NULL;
}

// func

static ici_obj_t *
restore_func(ici_archive_t *ar)
{
    ici_obj_t *code;
    ici_obj_t *args = NULL;
    ici_obj_t *autos = NULL;
    ici_obj_t *name = NULL;
    int nautos;
    ici_func_t *fn;
    ici_obj_t *oname;

    if (restore_object_name(ar, &oname))
        return NULL;
    if ((code = restore(ar)) == NULL)
        return NULL;
    if ((args = restore(ar)) == NULL)
        goto fail;
    if ((autos = restore(ar)) == NULL)
        goto fail;
    if ((name = restore(ar)) == NULL)
        goto fail;
    if (read32(ar, &nautos))
        goto fail;
    if ((fn = ici_new_func()) == NULL)
        goto fail;

    fn->f_code = ici_arrayof(code);
    fn->f_args = ici_arrayof(args);
    fn->f_autos = ici_structof(autos);
    fn->f_autos->o_head.o_super = ar->a_scope; /* structof(ici_vs.a_top[-1])->o_head.o_super; */
    fn->f_name = ici_stringof(name);
    fn->f_nautos = nautos;

    ici_decref(code);
    ici_decref(args);
    ici_decref(autos);
    ici_decref(name);

    if (!ici_archive_insert(ar, oname, ici_objof(fn)))
	return ici_objof(fn);

fail:
    ici_decref(code);
    if (args)
        ici_decref(args);
    if (autos)
        ici_decref(autos);
    if (name)
        ici_decref(name);
    return NULL;
}

static ici_obj_t *
restore_op(ici_archive_t *ar)
{
    int op_func_code;
    int op_ecode;
    int op_code;

    if
    (
        read32(ar, &op_func_code)
        ||
        read32(ar, &op_ecode)
        ||
        read32(ar, &op_code)
    )
    {
        return 0;
    }

    return ici_objof(ici_new_op(ici_archive_op_func(op_func_code), op_ecode, op_code));
}

static ici_obj_t *
restore_src(ici_archive_t *ar)
{
    int line;
    ici_obj_t *result;
    ici_obj_t *filename;

    if (read32(ar, &line))
    {
        return NULL;
    }
    if ((filename = restore(ar)) == NULL)
    {
        return NULL;
    }
    if (!ici_isstring(filename))
    {
        ici_error = "unexpected type of filename";
        ici_decref(filename);
        return NULL;
    }
    if ((result = ici_objof(ici_src_new(line, ici_stringof(filename)))) == NULL)
    {
        ici_decref(filename);
        return NULL;
    }
    ici_decref(filename);
    return result;
}

// cfunc

static ici_obj_t *
restore_cfunc(ici_archive_t *ar)
{
    int namelen;
    char *buf;
    ici_str_t *func_name;
    ici_obj_t *fn;
    int n;

    if (read32(ar, &namelen))
        return 0;
    if ((buf = ici_alloc(namelen)) == NULL)
        return 0;
    if (readn(ar, buf, namelen))
    {
        ici_free(buf);
        return 0;
    }
    if ((func_name = ici_str_new(buf, namelen)) == NULL)
    {
        ici_free(buf);
        return 0;
    }
    ici_free(buf);
    n = ici_array_nels(&ici_vs) - 1;
    fn = ici_array_get(&ici_vs, n);
    return ici_objof(fn);
}

static ici_obj_t *
restore_mark(ici_archive_t *ar)
{
    return ici_objof(&ici_o_mark);
}

// ref

static ici_obj_t *
restore_ref(ici_archive_t *ar)
{
    ici_obj_t *obj;
    ici_obj_t *name;

    if (restore_object_name(ar, &name))
        return NULL;
    if ((obj = ici_archive_lookup(ar, name)) == ici_null)
        obj = NULL;
    return obj;
}

// restorer

typedef struct
{
    ici_obj_t o_head;
    ici_obj_t *(*r_fn)(ici_archive_t *);
}
restorer_t;

static unsigned long
mark_restorer(ici_obj_t *o)
{
    o->o_flags |= ICI_O_MARK;
    return sizeof (restorer_t);
}

static void
free_restorer(ici_obj_t *o)
{
    ici_tfree(o, restorer_t);
}

static ici_type_t restorer_type =
{
    mark_restorer,
    free_restorer,
    ici_hash_unique,
    ici_cmp_unique,
    ici_copy_simple,
    ici_assign_fail,
    ici_fetch_fail,
    "restorer"
};

static restorer_t *
restorer_new(ici_obj_t *(*fn)(ici_archive_t *))
{
    static int restorer_tcode = 0;
    restorer_t *r;

    if (restorer_tcode == 0)
    {
        if ((restorer_tcode = ici_register_type(&restorer_type)) == 0)
            return NULL;
    }

    if ((r = ici_talloc(restorer_t)) != NULL)
    {
        ICI_OBJ_SET_TFNZ(r, restorer_tcode, 0, 1, sizeof (restorer_t));
        r->r_fn = fn;
        ici_rego(r);
    }
    return r;
}

static int
add(ici_struct_t *map, int tcode, ici_obj_t *(*fn)(ici_archive_t *))
{
    restorer_t *r;
    int_t *t = 0;

    if ((r = restorer_new(fn)) == NULL)
        return 1;
    if ((t = ici_int_new(tcode)) == NULL)
        goto fail;
    if (ici_assign(ici_objof(map), ici_objof(t), ici_objof(r)))
    {
        ici_decref(t);
        goto fail;
    }
    ici_decref(t);
    ici_decref(r);
    return 0;

fail:
    ici_decref(r);
    return 1;
}

static ici_obj_t *
fetch_by_int(ici_struct_t *s, int key)
{
    ici_obj_t   *k;
    ici_obj_t   *v = NULL;

    if ((k = ici_objof(ici_int_new(key))) != NULL)
    {
        v = ici_fetch(ici_objof(s), k);
        ici_decref(k);
    }
    return v;
}

static restorer_t *
get_restorer(int tcode)
{
    static ici_struct_t *map = 0;
    restorer_t          *r;

    if (!map)
    {
        struct
        {
            int tcode;
            ici_obj_t *(*fn)(ici_archive_t *);
        }
        fns[] =
        {
            {-1, restore_error},
            {ICI_TC_NULL, restore_null},
            {ICI_TC_INT, restore_int},
            {ICI_TC_FLOAT, restore_float},
            {ICI_TC_STRING, restore_string},
            {ICI_TC_REGEXP, restore_regexp},
            {ICI_TC_MEM, restore_mem},
            {ICI_TC_ARRAY, restore_array},
            {ICI_TC_SET, restore_set},
            {ICI_TC_STRUCT, restore_struct},
            {ICI_TC_PTR, restore_ptr},
            {ICI_TC_FUNC, restore_func},
            {ICI_TC_OP, restore_op},
            {ICI_TC_SRC, restore_src},
            {ICI_TC_CFUNC, restore_cfunc},
            {ICI_TC_MARK, restore_mark},
            {ICI_ARCHIVE_TCODE_REF, restore_ref}
        };
        size_t i;

        if ((map = ici_struct_new()) == NULL)
            return NULL;

        for (i = 0; i < nels(fns); ++i)
        {
            if (add(map, fns[i].tcode, fns[i].fn))
                goto fail;
        }
    }

    r = (restorer_t *)fetch_by_int(map, tcode);
    if (ici_objof(r) == ici_null)
    {
        r = (restorer_t *)fetch_by_int(map, -1);
        if (ici_objof(r) == ici_null)
        {
            ici_error = "archive module internal error";
            r = NULL;
        }
    }
    return r;

fail:
    ici_decref(map);
    return NULL;
}

static ici_obj_t *
restore(ici_archive_t *ar)
{
    ici_obj_t *obj = NULL;
    char flags;
    int tcode;

    if ((tcode = restore_obj(ar, &flags)) != -1)
    {
        restorer_t *restorer;

        if ((restorer = get_restorer(tcode)) != NULL && (obj = (*restorer->r_fn)(ar)) != NULL)
        {
            if (flags & ICI_O_ATOM)
                obj = ici_atom(obj, 1);
        }
    }
    return obj;
}

/**
 * any = archive.restore([file] [, struct])
 *
 * Read an object from a file using the ICI Archive Protocol format.
 *
 * If no file is specified the object is read from the standard input.
 *
 * If a struct is passed this is used as the scope assigned to any
 * functions restored from the stream.  This permits callers to
 * control the objects such code can see and therefore that code's
 * ability to interact with the rest of the system.
 *
 * If end of file is seen this fails with an "end-of-file" error rather
 * than returning NULL since NULL is a legitimite object to be restored.
 *
 * This --topic-- forms part of the --ici-archive-- documentation.
 */
int
ici_archive_f_restore(void)
{
    ici_file_t *file;
    ici_archive_t *ar;
    ici_objwsup_t *scp;
    ici_obj_t *obj = NULL;

    scp = ici_structof(ici_vs.a_top[-1])->o_head.o_super;
    switch (ICI_NARGS())
    {
    case 0:
        if ((file = ici_need_stdin()) == NULL)
            return 1;
	break;

    case 1:
        if (ici_typecheck("u", &file))
	{
            if (ici_typecheck("d", &scp))
		return 1;
	    if ((file = ici_need_stdin()) == NULL)
		return 1;
	}
	break;

    default:
	if (ici_typecheck("ud", &file, &scp))
	    return 1;
	break;
    }

    if ((ar = ici_archive_start(file, scp)) != NULL)
    {
        obj = restore(ar);
        ici_archive_stop(ar);
    }

    return obj == NULL ? 1 : ici_ret_with_decref(obj);
}
