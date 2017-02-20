#ifndef ICI_SERIALISATION_H
#define	ICI_SERIALISATION_H

/*
 * $Id: serialisation.h,v 1.5 2001/04/19 13:34:26 atrn Exp $
 */

#include <buf.h>
#include <array.h>
#include <struct.h>
#include <str.h>
#include <int.h>
#include <float.h>
#include <re.h>
#include <pcre/internal.h>
#include <parse.h>
#include <op.h>
#include <func.h>
#include <forall.h>
#include <file.h>
#include <exec.h>
#include <catch.h>
#include <mark.h>
#include <ptr.h>
#include <set.h>
#include <mem.h>
#include <pc.h>
#include <src.h>
#ifdef ici3
#include <method.h>
#endif

#ifdef _WIN32
#include <winsock.h>	/* for htonl etc... */
#endif

#if defined(__i386__) || defined(_WINDOWS)
#define	ICI_SERIALISATION_LITTLE_ENDIAN
#endif

#if defined(__alpha__)
#define	ICI_SERIALISATION_LITTLE_ENDIAN
#endif

#if defined(__ppc__)
#define ICI_SERIALISATION_BIG_ENDIAN
#undef ICI_SERIALISATION_LITTLE_ENDIAN
#endif

#if !defined ICI_SERIALISATION_LITTLE_ENDIAN && !defined ICI_SERIALISATION_BIG_ENDIAN
#warning Assuming big endian host
#endif

/*
 * Various constants we use.
 */
enum
{
    /*
     * The second parameter to ici_serialisation_end() indicates
     * if it is being called from a save() or restore() operation.
     * These enumerators are used.
     */
    ICI_SERIALISATION_SAVE,
    ICI_SERIALISATION_RESTORE,

    /*
     * During a save() we flag objects that have already been serialised
     * (and send a reference instead). This is set in the object header
     * flags field.
     */
    O_SAVED		= 0x08,

    /*
     * Extra type codes that are used solely as tags in the object
     * archiving protocol. These tags are greater than 15 as they
     * are not used in the PAIR macro.
     */
    TC_MEM		= 16,
    TC_NULL		= 17,
    TC_FUNC		= 18,
    TC_CFUNC		= 19,
    TC_FILE		= 20,
    TC_METHOD		= 22,	/* Don't clash with existing tags */

    /*
     * TC_REF is a special tag used in the archiving protocol to flatten
     * the type graph. A type that may include itself in its value (e.g.,
     * an array can contain itself) is sent as a reference (TC_REF) iff
     * the object has already been output in this save operation.
     */
    TC_REF		= 21,

    /*
     * Atomic objects have the top-bit of the tag set.
     */
    TC_ATOMIC		= 0x80

};

typedef struct ici_serialisation_type_s	ici_serialisation_type_t;

/*
 * Prototypes for our functions.
 */
extern void	ici_serialisation_init(int, int (**)());
extern void	ici_serialisation_start(void);
extern void	ici_serialisation_end(int);
extern object_t	*ici_serialisation_find_ref(long);
extern void	ici_serialisation_mod_ref(object_t *, object_t *);
extern int	ici_serialisation_add_ref(long, object_t *);

extern file_t	*ici_need_stderr(void);

extern int	ici_save_error(void);
extern int	ici_save_long(long, file_t *);
extern int	ici_save_len_str(long, char *, file_t *);
extern int	ici_save_name(object_t *, file_t *);
extern int	ici_save_simple(object_t *, file_t *);
extern int	ici_save_or_null(object_t *, file_t *);
extern int	ici_save(object_t *, file_t *f);

extern int	ici_save_error(void);
extern int	ici_save_simple(object_t *, file_t *);
extern int	ici_save_long(long, file_t *);
extern int	ici_save_name(object_t *, file_t *);
extern int	ici_save_len_str(long, char *, file_t *);
extern int	ici_save_or_null(object_t *o, file_t *f);
extern int	ici_save_array(object_t *o, file_t *f);
extern int	ici_save_catch(object_t *o, file_t *f);
extern int	ici_save_file(object_t *o, file_t *f);
extern int	ici_save_float(object_t *o, file_t *fi);
extern int	ici_save_forall(object_t *o, file_t *f);
extern int	ici_save_cfunc(object_t *o, file_t *f);
#ifdef ici3
extern int	ici_save_method(object_t *o, file_t *f);
#endif
extern int	ici_save_func(object_t *o, file_t *f);
extern int	ici_save_int(object_t *o, file_t *f);
extern int	ici_save_mem(object_t *o, file_t *f);
extern int	ici_save_null(object_t *o, file_t *f);
extern int	ici_save_op(object_t *o, file_t *f);
extern int	ici_save_pc(object_t *o, file_t *f);
extern int	ici_save_ptr(object_t *o, file_t *f);
extern int	ici_save_regexp(object_t *o, file_t *f);
extern int	ici_save_set(object_t *o, file_t *f);
extern int	ici_save_src(object_t *o, file_t *f);
extern int	ici_save_string(object_t *o, file_t *f);
extern int	ici_save_struct(object_t *o, file_t *f);
extern int	ici_save_ref(void *o, file_t *f);
extern int	ici_save(object_t *o, file_t *f);

extern object_t	*ici_restore_error(void);
extern object_t	*ici_restore_simple(file_t *, ici_serialisation_type_t *);
extern int	ici_restore_long(long *, file_t *);
extern int 	ici_readf(file_t *, char *, int);
extern int	ici_read_name(file_t *, object_t *);
extern object_t	*ici_restore_array(file_t *f, ici_serialisation_type_t *);
extern object_t	*ici_restore_catch(file_t *f, ici_serialisation_type_t *);
extern object_t	*ici_restore_file(file_t *f, ici_serialisation_type_t *);
extern object_t	*ici_restore_int(file_t *f, ici_serialisation_type_t *);
extern object_t	*ici_restore_float(file_t *f, ici_serialisation_type_t *);
extern object_t	*ici_restore_forall(file_t *f, ici_serialisation_type_t *);
extern object_t	*ici_restore_cfunc(file_t *f, ici_serialisation_type_t *);
#ifdef ici3
extern object_t	*ici_restore_method(file_t *f, ici_serialisation_type_t *);
#endif
extern object_t	*ici_restore_func(file_t *f, ici_serialisation_type_t *);
extern object_t	*ici_restore_object_t(file_t *f, ici_serialisation_type_t *);
extern object_t	*ici_restore_mem(file_t *f, ici_serialisation_type_t *);
extern object_t	*ici_restore_null(file_t *f, ici_serialisation_type_t *);
extern object_t	*ici_restore_op(file_t *f, ici_serialisation_type_t *);
extern object_t	*ici_restore_pc(file_t *f, ici_serialisation_type_t *);
extern object_t	*ici_restore_ptr(file_t *f, ici_serialisation_type_t *);
extern object_t	*ici_restore_regexp(file_t *f, ici_serialisation_type_t *);
extern object_t	*ici_restore_set(file_t *f, ici_serialisation_type_t *);
extern object_t	*ici_restore_src(file_t *f, ici_serialisation_type_t *);
extern object_t	*ici_restore_string(file_t *f, ici_serialisation_type_t *);
extern object_t	*ici_restore_struct(file_t *f, ici_serialisation_type_t *);
extern long	ici_restore_ref(file_t *f, ici_serialisation_type_t *);

extern object_t	*ici_restore_cfunc(file_t *, ici_serialisation_type_t *);
extern object_t	*ici_restore(file_t *);

struct ici_serialisation_type_s
{
    type_t	*type;
    int		tag;
    int		(*save)(object_t *, file_t *);
    object_t	*(*restore)(file_t *, ici_serialisation_type_t *);
};

extern ici_serialisation_type_t	*ici_serialisation_lookup_type(object_t *);
extern ici_serialisation_type_t	*ici_serialisation_lookup_type_by_tag(int);

typedef int		(*ici_serialisation_opfunc_t)();

extern ici_serialisation_opfunc_t	ici_serialisation_opfunc[];
extern int				ici_serialisation_nopfunc;

#endif /* #ifndef ICI_SERIALISATION_H */
