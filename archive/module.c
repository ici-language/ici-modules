/**
 *  ICI Object Serialization Module
 *
 *  Copyright (C) 1996, 2010  A.Newman
 *
 */

/*
 * Object Serialization
 *
 * The ICI 'archive' module performs serialization of ICI object graphs.
 * The module provides two functions, save and restore, to write and read
 * objects to and from files.  All builtin object types that do not rely
 * upon external state, everything but the "file" type used with OS files,
 * may be serialized to a file and later restored.  Files may be long
 * lived disk files, network connections or whatever file-like object.
 *
 * Special note needs to be made that function objects may be serialized.
 * ICI functions are written out as the byte code for the ICI VM (an array
 * of code objects and the like).  Native code functions, i.e. functions
 * that are implemented as native code and known as "cfunc" objects are
 * saved by simply saving their name.  When restored the current scope
 * is looked up for a similarly named cfunc object and that is assumed
 * to tbe write one.
 *
 * Object Serialization Protocol
 *
 * The ICI object serialization protocol is a simple, tagged binary
 * protocol for transferring ICI data types between writers and readers.
 * The protocol is platform independent in that it defines the size and
 * format of all types exchanged. E.g. all integer types are represented
 * in network byte order, floating pointed is defined to be IEEE-754
 * doubles, sizes are sent as 32-bit unsigned values, etc...  The exact
 * protocol follows.
 *
 * Integers are mostly 64-bit values with some object state sent as
 * 32-bit quantities.  64-bit values are sent as two 32-bit words,
 * each sent in network byte order.  The first 32-bit word is the
 * most significant (i.e. the words are also in NBO).
 *
 *
 * Basic Protocol Types
 *
 * tcode ::- byte
 *
 *      An object type code. The top-bit set if the object is atomic.
 *
 * int32 ::- byte byte byte byte
 *
 *      A 32-bit integer in NBO.  The first byte is the most significant.
 *
 * int64 ::- byte byte byte byte byte byte byte byte
 *
 *      A 64-bit integer in NBO.  The first byte is the most significant.
 *
 *
 * ICI Types
 *
 * null ::- tcode
 *
 *      The NULL object is just a type code, there is no state.
 *
 * int ::- tcode int64
 *
 *      An ICI integer.
 *
 * real ::- tcode byte byte byte byte byte byte byte byte
 *
 *      An ICI float.  The eight bytes store an IEEE 754 double
 *      precision number with its bytes in NBO.
 *
 * object ::- tcode id
 *
 *      An object header.  Objects other than integers and floats
 *      are sent as an object header followed by object specific
 *      data.  The header specifies the type code and defines a
 *      "name" for the object.  An object's name is an identifier
 *      assigned to each object by the archive protocol.  During
 *      an archive "session" named objects are only sent once
 *      with any other references to them being sent as "ref"
 *      psuedo-objects.
 *
 * id ::- int64
 *
 *      An id (object name) is a 64-bit value.  It needs to be
 *      unique for the duration of an archive operation.  It
 *      is typically the address of the object in memory.
 *
 * ref ::- tcode id
 *
 *      A reference to an already transmitted object.  This
 *      consists of a tcode for a "ref" psuedo-object type
 *      followed by the id (name) of the object being referenced.
 *
 * string ::- object int64 bytes...
 *
 *      A string is sent as an object header followed by a
 *      64-bit count of the number of bytes of string data
 *      followed by that many bytes of UTF-8 encoded UNICODE
 *      character values.
 *
 * regexp ::- object int32 string
 *
 *      A regular expression is an object header followed by
 *      32-bit integer "flags" value for the regular expression.
 *      The regexp itself is sent as text in a string.
 *
 * array ::- object int64 object*N
 *
 *      An array is an object header followed by a 64-bit count
 *      of the number of objects in the array followed by
 *      that many objects.
 *
 * set ::- object int64 object*N
 *
 *      A set is an object header followed by a 64-bit count
 *      of the number of objects in the set followed by that
 *      many objects.
 *
 * struct ::- object object|null int64 object-pair*N
 *
 *      A struct is an object header followed by the struct's
 *      "super" or NULL.  This is followed by a 64-bit count
 *      of the number of key/value pairs in the struct which
 *      is followed by that many key/value pairs.
 *
 * object-pair ::- object object
 *
 *      Two objects, typically key/value.
 *
 * mem ::- object int64 int32 bytes*N
 *
 *      An ICI mem object is an object header followed by a 64-bit
 *      size, a 32-bit access size specification and the size*access
 *      bytes of the mem object.  
 *
 * ptr ::- object-pair
 *
 *      An ICI pointer object is sent as a pair of objects, the pointer's
 *      aggreate and its key.
 *
 * cfunc ::- tcode int32 byte*N
 *
 *      A C function is sent as a type code and the function's name sent
 *      as a 32-bit count of the number of characters in the string
 *      followed by that many ASCII characters (bytes).
 *
 * func ::- object array array struct string int32
 *
 *      An ICI function is an object header followed by an array that
 *      stores the function's code, an array that stores the names of
 *      the function's formal arguments, a struct containing the
 *      names and values of the function's initial automatic scope,
 *      a string giving the name of the function and finally a
 *      32-bit integer guess of the number of auto variables used
 *      by the function.
 *
 * src ::- int32 string
 *
 *      A source file position marker is sent as a 32-bit integer
 *      line number and a string filename.
 *
 * op ::- int32 int32 int32
 *
 *      An ICI virtual machine operator is three 32-bit integers.
 *      The operator function code, operator flags and operator id.
 *
 * This --intro-- and --synopsis-- are part of --ici-archive-- documentation.
 */

#include "archive.h"
#include "icistr.h"
#include <icistr-setup.h>

static int ici_archive_tcode = 0;

static ici_archive_t *
archive_of(ici_obj_t *o)
{
    return (ici_archive_t *)(o);
}

static unsigned long
mark_archive(ici_obj_t *o)
{
    ici_archive_t *s = archive_of(o);
    o->o_flags |= ICI_O_MARK;
    return sizeof *s + ici_mark(s->a_file) + ici_mark(s->a_sent) + (s->a_scope ? ici_mark(s->a_scope) : 0);
}

static void
free_archive(ici_obj_t *o)
{
    ici_tfree(o, ici_archive_t);
}

static ici_type_t ici_archive_type =
{
    mark_archive,
    free_archive,
    ici_hash_unique,
    ici_cmp_unique,
    ici_copy_simple,
    ici_assign_fail,
    ici_fetch_fail,
    "archive"
};

static ici_archive_t *
new_archive(ici_file_t *file, ici_objwsup_t *scope)
{
    ici_archive_t *ar = ici_talloc(ici_archive_t);
    if (ar != NULL)
    {
        ICI_OBJ_SET_TFNZ(ar, ici_archive_tcode, 0, 1, 0);
        if ((ar->a_sent = ici_struct_new()) == NULL)
        {
            ici_tfree(ar, ici_archive_t);
            return NULL;
        }
        ar->a_file = file;
	ar->a_scope = scope;
        ici_rego(ar);
    }
    return ar;
}

ici_archive_t *
ici_archive_start(ici_file_t *file, ici_objwsup_t *scope)
{
    ici_archive_t *ar = new_archive(file, scope);
    return ar;
}

int
ici_archive_insert(ici_archive_t *ar, ici_obj_t *key, ici_obj_t *val)
{
    ici_int_t *k;
    int failed = 1;

    if ((k = ici_int_new((long)key)) != NULL)
    {
        failed = ici_assign(ar->a_sent, ici_objof(k), val);
        ici_decref(k);
    }
    return failed;
}

void
ici_archive_uninsert(ici_archive_t *ar, ici_obj_t *key)
{
    ici_struct_unassign(ar->a_sent, key);
}

ici_obj_t *
ici_archive_lookup(ici_archive_t *ar, ici_obj_t *obj)
{
    ici_obj_t *v = ici_null;
    ici_int_t *k;

    if ((k = ici_int_new((long)obj)) != NULL)
    {
        v = ici_fetch(ar->a_sent, ici_objof(k));
        ici_decref(k);
    }
    return v == ici_null ? NULL : v;
}

void
ici_archive_stop(ici_archive_t *ar)
{
    ici_decref(ar);
}

typedef int int_func();

static int_func *op_funcs[7] =
{
    NULL,
    NULL, /* ici_o_mklvalue */
    NULL, /* ici_o_onerror */
    NULL, /* ici_o_return */
    NULL, /* ici_o_mkptr */
    NULL, /* ici_o_openptr */
    NULL  /* ici_o_fetch */
};

#define NOPS ((int)(sizeof op_funcs / sizeof op_funcs[0]))

static void init_op_funcs()
{
    op_funcs[1] = ici_op_mklvalue;
    op_funcs[2] = ici_op_onerror;
    op_funcs[3] = ici_op_return;
    op_funcs[4] = ici_op_mkptr;
    op_funcs[5] = ici_op_openptr;
    op_funcs[6] = ici_op_fetch;
}

int ici_archive_op_func_code(int_func *fn)
{
    int i;

    if (op_funcs[1] == NULL)
        init_op_funcs();

    for (i = 0; i < NOPS; ++i)
    {
        if (fn == op_funcs[i])
            return i;
    }
    return -1;
}

int_func *ici_archive_op_func(int code)
{
    if (op_funcs[1] == NULL)
        init_op_funcs();

    if (code < 0 || code >= NOPS)
    {
        return NULL;
    }
    return op_funcs[code];
}

void ici_archive_byteswap(void *ptr, int sz)
{
    char *s = ptr;
    char *e = ptr + sz - 1;
    int i;

    for (i = 0; i < sz / 2 ; ++i)
    {
        char t = s[i];
        s[i] = e[-i];
        e[-i] = t;
    }
}

static ici_cfunc_t archive_cfuncs[] =
{
    {ICI_CF_OBJ, "save", ici_archive_f_save, 0, 0},
    {ICI_CF_OBJ, "restore", ici_archive_f_restore, 0, 0},
    {ICI_CF_OBJ, 0, 0, 0, 0},
};

ici_obj_t *
ici_archive_library_init(void)
{
    if
    (
        ici_interface_check(ICI_VER, ICI_BACK_COMPAT_VER, "archive")
        ||
        init_ici_str()
        ||
        (ici_archive_tcode = ici_register_type(&ici_archive_type)) == 0
    )
    {
        return NULL;
    }
    return ici_objof(ici_module_new(archive_cfuncs));
}
