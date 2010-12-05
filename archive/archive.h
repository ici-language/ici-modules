/**
 *  ICI Object Serialization Module
 *
 *  Copyright (C) 1996, 2010  A.Newman
 *
 */

#ifndef ICI_MODULE_ARCHIVE_H
#define ICI_MODULE_ARCHIVE_H

#include <ici.h>

#if defined WIN32 || defined _WIN32 || defined WIN64 || defined _WIN64
#define ICI_ARCHIVE_LITTLE_ENDIAN_HOST 1
#endif

#ifdef __i386__
#define ICI_ARCHIVE_LITTLE_ENDIAN_HOST 1
#endif

/*
 * An archive session.
 *
 * An ici_archive_t is an ICI object representing an active archiving
 * session, either a save or restore operation.  The ici_archive_t
 * holds the state used during the archive operation.  For both save
 * and restore operations this is the file being used as the source
 * or sink of data and a struct recording all of the objects used
 * in the operation.  The struct is used to implement the "reference"
 * feature of the archive protocol (so it only sends things once).
 *
 * For restore operations only the ici_archive_t also stores the
 * reference to the scope to use when restoring functions.
 */
struct ici_archive
{
    /*  ICI object header */
    ici_obj_t           o_head;

    /*  The source or sink of data */
    ici_file_t          *a_file;

    /*  Record of which objects have been archived so we know when to send refs */
    ici_struct_t        *a_sent;

    /*  The scope used when restoring functions */
    ici_objwsup_t	*a_scope;
};

typedef struct ici_archive ici_archive_t;

/*
 *  We use the top-bit of the "tcode" byte to indicate if
 *  an object is atomic or not.  This limits us to supporting
 *  2^7 tcodes which could be an issue if we extend the
 *  module to support archiving non-standard types.
 */
#define ICI_ARCHIVE_ATOMIC      (0x80)

/*
 *  The "tcode" of the pseudo-type we use to reference
 *  previously archived objects.
 */
#define ICI_ARCHIVE_TCODE_REF   (ICI_TC_MAX_CORE + 1)

/*
 *  Swap a sz bytes size piece of data at ptr
 */
void ici_archive_byteswap(void *ptr, int sz);


/*
 *  Start an archive session using the given file and scope.
 *
 *  This allocates and returns a new ici_archive_t.
 */
ici_archive_t   *ici_archive_start(ici_file_t *file, ici_objwsup_t *scope);

/*
 *  Add a record of an object sent as part of an archive save operation.
 */
int             ici_archive_insert(ici_archive_t *ar, ici_obj_t *key, ici_obj_t *val);

/*
 *  Remove the record of an object sent as part of archive save operation.
 */
void            ici_archive_uninsert(ici_archive_t *ar, ici_obj_t *key);

/*
 *  Given an object "name", stored in an ici_obj_t *, locate the actual
 *  object in the record of archived objects.
 */
ici_obj_t       *ici_archive_lookup(ici_archive_t *ar, ici_obj_t *obj);

/*
 *  Note the end of an archive session.
 */
void            ici_archive_stop(ici_archive_t *ar);

/*
 *  ICI cfuncs implementing the module's functions.
 */
int             ici_archive_f_save(void);
int             ici_archive_f_restore(void);

/*
 *  Utilities for mapping operator function codes to and from
 *  the functions they represent.
 */
int             ici_archive_op_func_code(int (*fn)());
int             (*ici_archive_op_func())(int code);

/*
 *  Some functions, although exported from the ICI interpreter's library,
 *  are not mentioned in its header files.
 */
extern long             ici_hash_string(ici_obj_t *);
extern ici_src_t *      ici_new_src(int, ici_str_t *);
extern void             ici_pcre_info(pcre *, int *, void *);
extern ici_func_t *     ici_new_func();
extern int              ici_op_mklvalue();
extern int              ici_op_onerror();
extern int              ici_op_return();
extern int              ici_op_mkptr();
extern int              ici_op_openptr();
extern int              ici_op_fetch();

#endif
