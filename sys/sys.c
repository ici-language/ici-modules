#include <ici.h>
#include "icistr.h"
#include <icistr-setup.h>

#undef isset

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#ifndef _WINDOWS
#include <unistd.h>
#endif
#if defined(sun) || defined(BSD4_4) || defined(linux)
#include <sys/wait.h>
#endif
#ifdef NeXT
#include <libc.h>
#endif

#ifdef SUNOS5
#include <signal.h>
#define ICI_SYS_NOFLOCK
#endif

#ifdef IRIX
#include <signal.h>
#include <sys/param.h>
#endif

#if defined(linux)
#include <sys/signal.h>
#endif

#ifdef _WINDOWS
# define        ICI_SYS_NOPASSWD
# define        ICI_SYS_NOFLOCK
# define        ICI_SYS_NORLIMITS
#endif

#ifndef ICI_SYS_NOPASSWD
# include <pwd.h>
#endif

#ifndef ICI_SYS_NODIR
#ifndef _WINDOWS
#include <sys/param.h>
#include <dirent.h>
#endif
#endif

#ifndef ICI_SYS_NORLIMITS
#include <sys/resource.h>
#endif

#if defined(linux) && !defined(MAXPATHLEN)
#include <sys/param.h>
#endif

#if defined(__linux__) || defined(__sun__)
#define SETPGRP_0_ARGS
#endif

#ifdef  BSD4_4
#define SETPGRP_2_ARGS
#endif

#if defined(__linux__) && defined(BSD4_4)
#error __linux__ or BSD4_4 for setpgrp(), not both
#endif

#ifndef _WINDOWS
# ifndef ICI_SYS_NOFLOCK
#  include <sys/file.h>
# endif
#endif

#ifdef _WINDOWS
#include <io.h>
#include <process.h>
#include <direct.h>

/*
 * Some macros for compatibility with Unix
 */

#define F_OK    0
#define W_OK    2
#define R_OK    4

#define MAXPATHLEN  _MAX_PATH
#endif

/*
 * Create pre-defined variables to replace C's #define's.
 */
int
ici_sys_vars_init(objwsup_t *scp)
{
    int         i;

#define VALOF(x) { # x , x }
    static struct
    {
        char    *name;
        long    val;
    }
    var[] =
    {
        VALOF(O_RDONLY),
        VALOF(O_WRONLY),
        VALOF(O_RDWR),
        VALOF(O_APPEND),
        VALOF(O_CREAT),
        VALOF(O_TRUNC),
        VALOF(O_EXCL),
#ifdef O_BINARY         /* WINDOWS has binary mode for open() */
        VALOF(O_BINARY),
#endif
#ifdef O_SYNC           /* WINDOWS doesn't have O_SYNC */
        VALOF(O_SYNC),
#endif
#ifdef O_NDELAY         /* WINDOWS doesn't have O_NDELAY */
        VALOF(O_NDELAY),
#endif
#ifdef O_NONBLOCK       /* NeXT doesn't have O_NONBLOCK */
        VALOF(O_NONBLOCK),
#endif
        VALOF(R_OK),
        VALOF(W_OK),
#ifdef X_OK             /* WINDOWS doesn't have X_OK */
        VALOF(X_OK),
#endif
        VALOF(F_OK),
        VALOF(SEEK_SET),
        VALOF(SEEK_CUR),
        VALOF(SEEK_END),

#if !defined(ICI_SYS_NOFLOCK) && defined(LOCK_SH)
        VALOF(LOCK_SH),
        VALOF(LOCK_EX),
        VALOF(LOCK_NB),
        VALOF(LOCK_UN),
#endif

#ifdef _WINDOWS
        VALOF(_P_WAIT),
        VALOF(_P_NOWAIT),
#endif

        VALOF(S_IFMT),
        VALOF(S_IFCHR),
        VALOF(S_IFDIR),
        VALOF(S_IFREG),
        VALOF(S_IREAD),
        VALOF(S_IWRITE),
        VALOF(S_IEXEC),

#ifndef _WINDOWS
        VALOF(S_IFIFO),
        VALOF(S_IFBLK),
        VALOF(S_IFLNK),
        VALOF(S_IFSOCK),
        VALOF(S_ISUID),
        VALOF(S_ISGID),
        VALOF(S_ISVTX),
        VALOF(S_IRUSR),
        VALOF(S_IWUSR),
        VALOF(S_IXUSR),
        VALOF(S_IRGRP),
        VALOF(S_IWGRP),
        VALOF(S_IXGRP),
        VALOF(S_IROTH),
        VALOF(S_IWOTH),
        VALOF(S_IXOTH)
#endif
    };
#undef VALOF

    for (i = 0; i < sizeof var/sizeof var[0]; ++i)
        if (ici_cmkvar(scp, var[i].name, 'i', &var[i].val))
            return 1;
    return 0;
}

#ifndef _WINDOWS
/*
 * System calls.  We do all the easy ones because they cost so little,
 * most of the code being in the kernel.  And who can tell what people
 * will want anyway?
 */
#ifndef NeXT

extern int      access(), acct(), chdir(),
                chmod(), chown(), chroot(), close(),
#ifndef linux
                creat(),
#else
                __open(), /* creat() is a macro! defined in fcntl.h */
#endif
                dup(),
#if !defined(SUNOS5) && !defined(__hppa) && !defined(BSD4_4)
                fork(), getpid(), getpgrp(),
                getppid(),
#endif
#ifndef IRIX
                kill(), link(), mkdir(), mknod(),
                nice(), open(), pause(), rmdir(),
#endif
#if !defined(SUNOS5) && !defined(__hppa) && !defined(IRIX)
                setpgrp(),
#endif
                setuid(), setgid(), setgid(),

#if !defined(SUNOS5) && !defined(__hppa) && !defined(BSD4_4) && !defined(IRIX)
                sync(),
#endif

#if !defined(linux) && !defined(__hppa) && !defined(BSD4_4) && !defined(sun) && !defined(IRIX)
                umask(), alarm(), getuid(), geteuid(), getgid(), getegid(),
                lseek(), sleep(),
#endif

                ulimit(), unlink();

#if !defined(SUNOS5) && !defined(__hppa) && !defined(__USE_FIXED_PROTOTYPES__) && !defined(BSD4_4) && !defined(linux) && !defined(IRIX)
extern int      clock();
#endif

extern int      system();
#if 0
extern int      lockf();
#endif

#endif

#ifdef sun
int rename();
int lstat();
void setpwent();
void endpwent();
#endif

#ifdef NeXT
int     acct();
int     getppid(void);
int     lockf();
#endif
#endif /* _WINDOWS */

/*
 * Used as a common error return for system calls that fail. Sets the
 * global error string if the call fails otherwise returns the integer
 * result of the system call.
 */
static int
sys_ret(ret)
int     ret;
{
    char        n1[ICI_OBJNAMEZ];

    if (ret < 0)
        return ici_get_last_errno(ici_objname(n1, ici_os.a_top[-1]), NULL);
    return ici_int_ret((long)ret);
}

/*
 * Used to call "simple" system calls. Simple calls are those that take
 * up to four integer parameters and return an integer. FUNCDEF() just
 * makes the name unique for this module. We have to use CFUNC3()
 * to use it, see below.
 */
static int ici_sys_simple()
{
    long        av[4];

    if (ici_typecheck(CF_ARG2(), &av[0], &av[1], &av[2], &av[3]))
        return 1;
    return sys_ret((*(int (*)())CF_ARG1())(av[0], av[1], av[2], av[3]));
}

/*
 * open(pathname, flags [, mode])
 *
 * Do the open(2) system call with the specified flags and mode (only needed
 * if flags includes O_CREAT).
 *
 * Returns the file descriptor (an int).
 */
static int ici_sys_open()
{
    char        *fname;
    long        omode;
    long        perms = -1; /* -1 means not passed */

    switch (NARGS())
    {
    case 2:
        if (ici_typecheck("si", &fname, &omode))
            return 1;
        break;

    case 3:
        if (ici_typecheck("sii", &fname, &omode, &perms))
            return 1;
        break;

    default:
        return ici_argcount(3);
    }
    if (omode & O_CREAT && perms == -1)
    {
        ici_error = "permission bits not specified in open() with O_CREAT";
        return 1;
    }
    return sys_ret(open(fname, omode, perms));
}

#ifdef _WINDOWS
/*
 * General return for things not implemented on (that suckful) Win32.
 */
static int
not_on_win32(char *s)
{
    sprintf(ici_buf, "%s is not implemented on Win32 platforms", s);
    ici_error = ici_buf;
    return 1;
}
#endif

/*
 * fdopen(int [, openmode])
 *
 * Open a file descriptor as an (ICI) file object. This is analogous
 * to fopen, sopen, mopen, popen, sktopen, etc... The openmode is a
 * string as accepted by fopen and defaults to "r" (read only).
 *
 * Returns a file object.
 */
static int ici_sys_fdopen()
{
#ifdef _WINDOWS /* WINDOWS can't do fdopen() without lots of work */
    return not_on_win32("fdopen");
#else
    long        fd;
    char        *mode;
    FILE        *stream;
    file_t      *f;

    switch (NARGS())
    {
    case 1:
        if (ici_typecheck("i", &fd))
            return 1;
        mode = "r";
        break;
    case 2:
        if (ici_typecheck("is", &fd, &mode))
            return 1;
        break;
    default:
        return ici_argcount(2);
    }
    if ((stream = fdopen(fd, mode)) == NULL)
    {
        ici_error = "can't fdopen";
        return 1;
    }
    setvbuf(stream, NULL, _IOLBF, 0);
    if ((f = ici_file_new((char *)stream, &ici_stdio_ftype, NULL, NULL)) == NULL)
    {
        fclose(stream);
        return 1;
    }
    return ici_ret_with_decref(objof(f));
#endif /* _WINDOWS */
}

static int ici_sys_close()
{
    int                 rc;
    object_t            *fd0;
    object_t            *fd1;

    if (NARGS() != 1)
        return ici_argcount(1);
    if (isint(ARG(0)))
        rc = close(intof(ARG(0))->i_value);
    else if (isarray(ARG(0)))
    {
        array_t *a = arrayof(ARG(0));

        if
        (
            ici_array_nels(a) != 2
            ||
            !isint(fd0 = ici_array_get(a, 0))
            ||
            !isint(fd1 = ici_array_get(a, 1))
        )
        {
            ici_error = "invalid fd array passed to sys.close";
            return 1;
        }
        rc = close(intof(fd0)->i_value);
        if (rc == 0)
            rc = close(intof(fd1)->i_value);
    }
    else
        return ici_argerror(0);
    return sys_ret(rc);
}


#ifndef _WINDOWS

/* Convert a struct to a struct flock for fcntl's F_SETLK */

static int
struct_to_flock(struct_t *d, struct flock *flock)
{
    object_t    *o;

    if ((o = ici_fetch(d, ICISO(start))) == ici_null)
        flock->l_start = 0;
    else
        flock->l_start = intof(o)->i_value;
    if ((o = ici_fetch(d, ICISO(len))) == ici_null)
        flock->l_len = 0;
    else
        flock->l_len = intof(o)->i_value;
    if ((o = ici_fetch(d, ICISO(type))) == ici_null)
        flock->l_type = F_RDLCK;
    else if (isstring(o))
    {
        if (o == ICISO(rdlck))
            flock->l_type = F_RDLCK;
        else if (o == ICISO(wrlck))
            flock->l_type = F_WRLCK;
        else if (o == ICISO(unlck))
            flock->l_type = F_UNLCK;
        else
            goto bad_lock_type;
    }
    else if (isint(o))
        flock->l_type = intof(o)->i_value;
    else
    {
bad_lock_type:
        ici_error = "invalid lock type";
        return 1;
    }
    if ((o = ici_fetch(d, ICISO(whence))) == ici_null)
        flock->l_whence = SEEK_SET;
    else
        flock->l_whence = intof(o)->i_value;
    return 0;
}
#endif  /* _WINDOWS */

/*
 * fcntl(int, what)
 */
static int ici_sys_fcntl()
{
#ifdef _WINDOWS
    return not_on_win32("fcntl");
#else
    long        fd;
    string_t    *what;
    object_t    *arg;
    int         iarg;
    int         iwhat;
    int         r;

    switch (NARGS())
    {
    case 2:
        iarg = 1;
        arg = ici_null;
        if (ici_typecheck("io", &fd, &what))
            return 1;
        break;

    case 3:
        if (ici_typecheck("ioo", &fd, &what, &arg))
            return 1;
        if (isint(arg))
            iarg = intof(arg)->i_value;
        else
            iarg = 0;
        break;

    default:
        return ici_argcount(3);
    }
    if (!isstring(objof(what)))
        return ici_argerror(1);
    if (what == ICIS(dupfd))
        iwhat = F_DUPFD;
    else if (what == ICIS(getfd))
        iwhat = F_GETFD;
    else if (what == ICIS(setfd))
        iwhat = F_SETFD;
    else if (what == ICIS(getfl))
        iwhat = F_GETFL;
    else if (what == ICIS(setfl))
        iwhat = F_SETFL;
    else if (what == ICIS(getown))
        iwhat = F_GETOWN;
    else if (what == ICIS(setown))
        iwhat = F_SETOWN;
    else if (what == ICIS(setlk))
    {
        struct flock myflock;

        memset(&myflock, 0, sizeof myflock);
        myflock.l_type = F_RDLCK;
        myflock.l_whence = SEEK_SET;
        iwhat = F_SETLK;
        if (isstruct(arg) && struct_to_flock(structof(arg), &myflock))
            return 1;
        r = fcntl(fd, iwhat, &myflock);
        goto ret;
    }
    else
        return ici_argerror(1);
    r = fcntl(fd, iwhat, iarg);
ret:
    return sys_ret(r);
#endif /* _WINDOWS */
}

static int ici_sys_fileno()
{
    file_t      *f;

    if (ici_typecheck("u", &f))
        return 1;
    if
    (
        f->f_type != &ici_stdio_ftype
#       ifndef NOPIPES
            &&
            f->f_type != &ici_popen_ftype
#       endif
    )
    {
        ici_error = "attempt to obtain file descriptor of non-stdio file";
        return 1;
    }
    return ici_int_ret(fileno((FILE *)f->f_file));
}

#ifndef _WINDOWS
static int ici_sys_setlinebuf()
{
    file_t      *file;

    if (ici_typecheck("u", &file))
        return 1;
    if (file->f_type->ft_getch == fgetc)
        setlinebuf((FILE *)file->f_file);
    return ici_null_ret();
}
#endif /* _WINDOWS */


static int ici_sys_mkdir()
{
    char        *path;

#ifdef _WINDOWS
    if (ici_typecheck("s", &path))
        return 1;
    if (mkdir(path) == -1)
        return ici_get_last_errno("mkdir", path);
#else
    long        mode = 0777;

    if (NARGS() == 1)
    {
        if (ici_typecheck("s", &path))
            return 1;
    }
    else if (ici_typecheck("si", &path, &mode))
        return 1;
    if (mkdir(path, mode) == -1)
        return ici_get_last_errno("mkdir", path);
#endif
    return ici_ret_no_decref(ici_null);
}

static int ici_sys_mkfifo()
{
#ifdef _WINDOWS /* WINDOWS can't do mkifo() */
    return not_on_win32("mkfifo");
#else
    char        *path;
    long        mode;

    if (ici_typecheck("si", &path, &mode))
        return 1;
    if (mkfifo(path, mode) == -1)
        return ici_get_last_errno("mkfifo", path);
    return ici_ret_no_decref(ici_null);
#endif /* _WINDOWS */
}

static int ici_sys_read()
{
    long        fd;
    long        len;
    string_t    *s;
    int         r;
    char        *msg;

    if (ici_typecheck("ii", &fd, &len))
        return 1;
    if ((msg = ici_alloc(len+1)) == NULL)
        return 1;
    switch (r = read(fd, msg, len))
    {
    case -1:
        ici_free(msg);
        return sys_ret(-1);

    case 0:
        ici_free(msg);
        return ici_null_ret();
    }
    if ((s = ici_str_alloc(r)) == NULL)
    {
        ici_free(msg);
        return 1;
    }
    memcpy(s->s_chars, msg, r);
    s = stringof(ici_atom(objof(s), 1));
    ici_free(msg);
    return ici_ret_with_decref(objof(s));
}

static int ici_sys_write()
{
    long        fd;
    object_t    *o;
    char        *addr;
    long        sz;
    int         havesz = 0;

    if (ici_typecheck("io", &fd, &o))
    {
        if (ici_typecheck("ioi", &fd, &o, &sz))
            return 1;
        havesz = 1;
    }
    if (isstring(o))
    {
        addr = (char *)stringof(o)->s_chars;
        if (!havesz || sz > stringof(o)->s_nchars)
            sz = stringof(o)->s_nchars;
    }
    else if (ismem(o))
    {
        addr = (char *)memof(o)->m_base;
        if (!havesz || (size_t)sz > memof(o)->m_length * memof(o)->m_accessz)
            sz = memof(o)->m_length * memof(o)->m_accessz;
    }
    else
    {
        return ici_argerror(1);
    }
    return sys_ret(write((int)fd, addr, (size_t)sz));
}

static int ici_sys_symlink()
{
#ifdef _WINDOWS /* WINDOWS can't do symlink() */
    return not_on_win32("symlink");
#else
    char        *a, *b;

    if (ici_typecheck("ss", &a, &b))
        return 1;
    if (symlink(a, b) == -1)
        return ici_get_last_errno("symlink", a);
    return ici_ret_no_decref(ici_null);
#endif /* _WINDOWS */
}

static int ici_sys_readlink()
{
#ifdef _WINDOWS /* WINDOWS can't do fdopen() without lots of work */
    return not_on_win32("fdopen");
#else
    char        *path;
    char        pbuf[MAXPATHLEN+1];

    if (ici_typecheck("s", &path))
        return 1;
    if (readlink(path, pbuf, sizeof pbuf) == -1)
        return ici_get_last_errno("readlink", path);
    return ici_ret_with_decref(objof(ici_str_new_nul_term(pbuf)));
#endif /* _WINDOWS */
}

/*
 * stat(string)
 * stat(int)
 * stat(file)
 *
 * Perform the stat(2) or fstat(2) system call. The file to be stat'd can
 * be passed as a pathname (string), ICI file object (returned by fopen)
 * or file descriptor (int).
 *
 * Returns a struct containing integers with the following keys,
 *      atime           blksize         blocks          ctime
 *      dev             gid             ino             mode
 *      mtime           nlink           rdev            size
 *      uid
 *
 * These correspond to the fields of the same names (with a st_ prefix)
 * in the struct stat returned by the system call.
 */
static int ici_sys_stat()
{
    object_t    *o;
    struct stat statb;
    int         rc;
    struct_t    *s;

    if (NARGS() != 1)
        return ici_argcount(1);
    o = ARG(0);
    if (isint(o))
        rc = fstat(intof(o)->i_value, &statb);
    else if (isstring(o))
        rc = stat(stringof(o)->s_chars, &statb);
    else if (isfile(o) && fileof(o)->f_type == &ici_stdio_ftype)
        rc = fstat(fileno((FILE *)fileof(o)->f_file), &statb);
    else
        return ici_argerror(0);
    if (rc == -1)
        return sys_ret(rc);
    if ((s = ici_struct_new()) == NULL)
        return 1;
#define SETFIELD(x)                                             \
    if ((o = objof(ici_int_new(statb.st_ ## x))) == NULL)       \
        goto fail;                                              \
    else if (ici_assign(s, ICISO(x), o))                        \
    {                                                           \
        ici_decref(o);                                          \
        goto fail;                                              \
    }                                                           \
    else                                                        \
        ici_decref(o)

    SETFIELD(dev);
    SETFIELD(ino);
    SETFIELD(mode);
    SETFIELD(nlink);
    SETFIELD(uid);
    SETFIELD(gid);
    SETFIELD(rdev);
    SETFIELD(size);
    SETFIELD(atime);
    SETFIELD(mtime);
    SETFIELD(ctime);
#ifndef _WINDOWS
    SETFIELD(blksize);
    SETFIELD(blocks);
#endif

#undef SETFIELD

    return ici_ret_with_decref(objof(s));

fail:
    ici_decref(s);
    return 1;
}

#ifndef _WINDOWS
/*
 * lstat(string)
 *
 * Perform the lstat(2) system call. The file to be lstat'd is passed as
 * a pathname (string).
 *
 * Returns a struct containing integers with the following keys,
 *      atime           blksize         blocks          ctime
 *      dev             gid             ino             mode
 *      mtime           nlink           rdev            size
 *      uid
 *
 * These correspond to the fields of the same names (with a st_ prefix)
 * in the struct stat returned by the system call.
 */
static int ici_sys_lstat()
{
    object_t    *o;
    struct stat statb;
    int         rc;
    struct_t    *s;

    if (NARGS() != 1)
        return ici_argcount(1);
    o = ARG(0);
    if (isstring(o))
        rc = lstat(stringof(o)->s_chars, &statb);
    else
        return ici_argerror(0);
    if (rc == -1)
        return sys_ret(rc);
    if ((s = ici_struct_new()) == NULL)
        return 1;
#define SETFIELD(x)                                             \
    if ((o = objof(ici_int_new(statb.st_ ## x))) == NULL)       \
        goto fail;                                              \
    else if (ici_assign(s, ICISO(x), o))                        \
    {                                                           \
        ici_decref(o);                                          \
        goto fail;                                              \
    }                                                           \
    else                                                        \
        ici_decref(o)

    SETFIELD(dev);
    SETFIELD(ino);
    SETFIELD(mode);
    SETFIELD(nlink);
    SETFIELD(uid);
    SETFIELD(gid);
    SETFIELD(rdev);
    SETFIELD(size);
    SETFIELD(atime);
    SETFIELD(mtime);
    SETFIELD(ctime);
    SETFIELD(blksize);
    SETFIELD(blocks);

#undef SETFIELD

    return ici_ret_with_decref(objof(s));

fail:
    ici_decref(s);
    return 1;
}
#endif

/*
 * ctime(int)
 *
 * Convert a time value to a string. Takes a count of the number of
 * seconds past the epoch and returns a string (see ctime(3)).
 */
static int ici_sys_ctime()
{
    long        timev;
    string_t    *s;

    if (ici_typecheck("i", &timev) || (s = ici_str_new_nul_term(ctime(&timev))) == NULL)
        return 1;
    return ici_ret_with_decref(objof(s));
}

/*
 * time()
 *
 * time() returns the time since 00:00:00 GMT,  Jan.  1,  1970 (the epoch)
 * measured in seconds.
 */
static int ici_sys_time()
{
    return ici_int_ret(time(NULL));
}

#ifndef _WINDOWS

static int
assign_timeval(struct_t *s, string_t *k, struct timeval *tv)
{
    struct_t    *ss;
    int_t       *i;

    if (k == NULL)
        ss = s;
    else if ((ss = ici_struct_new()) == NULL)
        return 1;
    if ((i = ici_int_new(tv->tv_usec)) == NULL)
        goto fail;
    if (ici_assign(ss, ICISO(usec), i))
    {
        ici_decref(i);
        goto fail;
    }
    ici_decref(i);
    if ((i = ici_int_new(tv->tv_sec)) == NULL)
        goto fail;
    if (ici_assign(ss, ICISO(sec), i))
    {
        ici_decref(i);
        goto fail;
    }
    ici_decref(i);
    if (k != NULL && ici_assign(s, k, ss))
        goto fail;
    return 0;

fail:
    if (k != NULL)
        ici_decref(ss);
    return 1;
}

/*
 * struct = getitimer(string)
 */
static int ici_sys_getitimer()
{
    long                which = ITIMER_VIRTUAL;
    struct_t            *s;
    struct itimerval    value;
    object_t            *o;

    if (NARGS() != 0)
    {
        if (ici_typecheck("o", &o))
            return 1;
        if (!isstring(o))
            return ici_argerror(0);
        if (o == ICISO(real))
            which = ITIMER_REAL;
        else if (o == ICISO(virtual))
            which = ITIMER_VIRTUAL;
        else if (o == ICISO(prof))
            which = ITIMER_PROF;
        else
            return ici_argerror(0);
    }
    if (getitimer(which, &value) == -1)
        return sys_ret(-1);
    if ((s = ici_struct_new()) == NULL)
        return 1;
    if
    (
        assign_timeval(s, ICIS(interval), &value.it_interval)
        ||
        assign_timeval(s, ICIS(value), &value.it_value)
    )
    {
        ici_decref(s);
        return 1;
    }
    return ici_ret_with_decref(objof(s));
}

static int
fetch_timeval(object_t *s, struct timeval *tv)
{
    object_t    *o;

    if (!isstruct(s))
        return 1;
    if ((o = ici_fetch(s, ICISO(usec))) == ici_null)
        tv->tv_usec = 0;
    else if (isint(o))
        tv->tv_usec = intof(o)->i_value;
    else
        return 1;
    if ((o = ici_fetch(s, ICISO(sec))) == ici_null)
        tv->tv_sec = 0;
    else if (isint(o))
        tv->tv_sec = intof(o)->i_value;
    else
        return 1;
    return 0;
}

/*
 * struct = setitimer(string)
 */
static int ici_sys_setitimer()
{
    long                which = ITIMER_VIRTUAL;
    struct_t            *s;
    struct itimerval    value;
    struct itimerval    ovalue;
    object_t            *o;

    if (NARGS() == 1)
    {
        if (ici_typecheck("d", &s))
            return 1;
    }
    else
    {
        if (ici_typecheck("od", &o, &s))
            return 1;
        if (o == ICISO(real))
            which = ITIMER_REAL;
        else if (o == ICISO(virtual))
            which = ITIMER_VIRTUAL;
        else if (o == ICISO(prof))
            which = ITIMER_PROF;
        else
            return ici_argerror(0);
    }
    if ((o = ici_fetch(s, ICISO(interval))) == ici_null)
        value.it_interval.tv_sec = value.it_interval.tv_usec = 0;
    else if (fetch_timeval(o, &value.it_interval))
        goto invalid_itimerval;
    if ((o = ici_fetch(s, ICISO(value))) == ici_null)
        value.it_value.tv_sec = value.it_value.tv_usec = 0;
    else if (fetch_timeval(o, &value.it_value))
        goto invalid_itimerval;
    if (setitimer(which, &value, &ovalue) == -1)
        return sys_ret(-1);
    if ((s = ici_struct_new()) == NULL)
        return 1;
    if
    (
        assign_timeval(s, ICIS(interval), &ovalue.it_interval)
        ||
        assign_timeval(s, ICIS(value), &ovalue.it_value)
    )
    {
        ici_decref(s);
        return 1;
    }
    return ici_ret_with_decref(objof(s));

invalid_itimerval:
    ici_error = "invalid itimer struct";
    return 1;
}


static int ici_sys_gettimeofday()
{
    struct_t            *s;
    struct timeval      tv;

    if (gettimeofday(&tv, NULL) == -1)
        return sys_ret(-1);
    if ((s = ici_struct_new()) == NULL)
        return 1;
    if (assign_timeval(s, NULL, &tv))
    {
        ici_decref(s);
        return 1;
    }
    return ici_ret_with_decref(objof(s));
}
#endif /* _WINDOWS */


/*
 * access(pathname [,flags])
 *
 * Determine accessibility of a file. If flags is not passed it defaults
 * to F_OK.
 *
 */
static int ici_sys_access()
{
    char        *fname;
    int         bits = F_OK;

    switch (NARGS())
    {
    case 1:
        if (ici_typecheck("s", &fname))
            return 1;
        break;
    case 2:
        if (ici_typecheck("si", &fname, &bits))
            return 1;
        break;
    default:
        return ici_argcount(2);
    }
    return sys_ret(access(fname, bits));
}

/*
 * pipe()
 *
 * Create a pipe and return an array of two file descriptors.
 */
static int ici_sys_pipe()
{
#ifdef _WINDOWS
    return not_on_win32("pipe");
#else
    int         pfd[2];
    array_t     *a;
    int_t       *fd;

    if ((a = ici_array_new(2)) == NULL)
        return 1;
    if (pipe(pfd) == -1)
    {
        ici_decref(a);
        return sys_ret(-1);
    }
    if ((fd = ici_int_new(pfd[0])) == NULL)
        goto fail;
    *a->a_top++ = objof(fd);
    ici_decref(fd);
    if ((fd = ici_int_new(pfd[1])) == NULL)
        goto fail;
    *a->a_top++ = objof(fd);
    ici_decref(fd);
    return ici_ret_with_decref(objof(a));

fail:
    ici_decref(a);
    close(pfd[0]);
    close(pfd[1]);
    return 1;
#endif  /* #ifndef _WINDOWS */
}

/*
 * creat(pathname, mode)
 *
 * Create the named file with the given permissions.
 */
static int ici_sys_creat()
{
    char        *fname;
    long        perms;
    int         fd;

    if (ici_typecheck("si", &fname, &perms))
        return 1;
    if ((fd = creat(fname, perms)) == -1)
        return ici_get_last_errno("creat", fname);
    return ici_int_ret(fd);
}

/*
 * dup(int [, int])
 *
 * Perform a dup(2) or dup2(2) system call depending upon args.
 */
static int ici_sys_dup()
{
    long        fd1;
    long        fd2;

    switch (NARGS())
    {
    case 1:
        if (ici_typecheck("i", &fd1))
            return 1;
        fd2 = -1;
        break;
    case 2:
        if (ici_typecheck("ii", &fd1, &fd2))
            return 1;
        break;
    default:
        return ici_argcount(2);
    }
    if (fd2 == -1)
    {
        fd2 = dup(fd1);
        return fd2 < 0 ? sys_ret(fd2) : ici_int_ret(fd2);
    }
    if (dup2(fd1, fd2) < 0)
        return sys_ret(-1);
    return ici_int_ret(fd2);
}

/*
 * exec(pathname, array)
 * exec(pathname, arg0, arg1...)
 *
 * Execute a program. The first form uses an array to store the
 * arguments passed to the program, the second uses a variable
 * number of arguments.
 *
 */
static int ici_sys_exec()
{
    char        *sargv[16];
    char        **argv;
    int         maxargv;
    int         n;
    object_t    **o;
    char        *path;
    int         argc;

#define ADDARG(X)                                                       \
    {                                                                   \
        if (argc >= 16)                                                 \
        {                                                               \
            if (argc >= maxargv)                                        \
            {                                                           \
                char    **newp;                                         \
                int      i;                                             \
                maxargv += argc;                                        \
                if ((newp = ici_alloc(maxargv * sizeof (char *))) == NULL)      \
                {                                                       \
                    if (argv != sargv)                                  \
                        ici_free(argv);                                 \
                    return 1;                                           \
                }                                                       \
                for (i = 0; i < argc; ++i)                              \
                    newp[i] = argv[i];                                  \
                if (argv != sargv)                                      \
                    ici_free(argv);                                     \
                argv = newp;                                            \
            }                                                           \
        }                                                               \
        argv[argc++] = (X);                                             \
    }

    if ((n = NARGS()) < 2)
        return ici_argcount(2);
    if (!isstring(*(o = ARGS())))
        return ici_argerror(0);
    path = stringof(*o)->s_chars;
    --o;
    argc = 0;
    argv = sargv;
    maxargv = 16;
    if (n > 2 || isstring(*o))
    {
        while (--n > 0)
        {
            ADDARG(stringof(*o)->s_chars);
            --o;
        }
    }
    else if (!isarray(*o))
        return ici_argerror(0);
    else
    {
        object_t **p;
        array_t *a;

        a = arrayof(*o);
        for (p = ici_astart(a); p < ici_alimit(a); p = ici_anext(a, p))
            if (isstring(*p))
                ADDARG(stringof(*p)->s_chars);
    }
    ADDARG(NULL);
    n = (*(int (*)())CF_ARG1())(path, argv);
    if (argv != sargv)
        ici_free(argv);
    return sys_ret(n);
}

/*
 * spawn([mode,] pathname, array)
 * spawn([mode,] pathname, arg0, arg1...)
 * spawnp([mode,] pathname, array)
 * spawnp([mode,] pathname, arg0, arg1...)
 *
 * Execute a program. The first form uses an array to store the
 * argyuments passed to the program, the second uses a variable
 * number of arguments.
 */
static int ici_sys_spawn()
{
#ifndef _WINDOWS
    ici_error = "spawn is only implemented on Win32 platforms";
    return 1;
#else
    char        *sargv[16];
    char        **argv;
    int         maxargv;
    int         n;
    object_t    **o;
    char        *path;
    int         argc;
    int         mode = _P_NOWAIT;

#define ADDARG(X)                                                       \
    {                                                                   \
        if (argc >= 16)                                                 \
        {                                                               \
            if (argc >= maxargv)                                        \
            {                                                           \
                char    **newp;                                         \
                int      i;                                             \
                maxargv += argc;                                        \
                if ((newp = ici_alloc(maxargv * sizeof (char *))) == NULL)      \
                {                                                       \
                    if (argv != sargv)                                  \
                        ici_free(argv);                                 \
                    return 1;                                           \
                }                                                       \
                for (i = 0; i < argc; ++i)                              \
                    newp[i] = argv[i];                                  \
                if (argv != sargv)                                      \
                    ici_free(argv);                                     \
                argv = newp;                                            \
            }                                                           \
        }                                                               \
        argv[argc++] = (X);                                             \
    }

    if ((n = NARGS()) < 2)
        return ici_argcount(2);
    o = ARGS();
    if (isint(*o))
    {
        mode = intof(*o)->i_value;
        --o;
        if (--n < 2)
            return ici_argcount(2);
        if (!isstring(*o))
            return ici_argerror(1);
    }
    else if (!isstring(*o))
        return ici_argerror(0);
    path = stringof(*o)->s_chars;
    --o;
    argc = 0;
    argv = sargv;
    maxargv = 16;
    if (n > 2 || isstring(*o))
    {
        while (--n > 0)
        {
            ADDARG(stringof(*o)->s_chars);
            --o;
        }
    }
    else if (!isarray(*o))
        return ici_argerror(0);
    else
    {
        object_t **p;
        array_t *a;

        a = arrayof(*o);
        for (p = ici_astart(a); p < ici_alimit(a); p = ici_anext(a, p))
            if (isstring(*p))
                ADDARG(stringof(*p)->s_chars);
    }
    ADDARG(NULL);
    n = (*(int (*)())CF_ARG1())(mode, path, argv);
    if (argv != sargv)
        ici_free(argv);
    if (n == -1)
        return sys_ret(n);
    return ici_int_ret(n);
#endif /* WINDOWS */
}

/*
 * lseek(int, int [, int])
 *
 * Set a file descriptor's I/O position. If 'whence' not passed
 * it defaults to SEEK_SET.
 */
static int ici_sys_lseek()
{
    long        fd;
    long        ofs;
    long        whence = SEEK_SET;

    switch (NARGS())
    {
    case 2:
        if (ici_typecheck("ii", &fd, &ofs))
            return 1;
        break;
    case 3:
        if (ici_typecheck("iii", &fd, &ofs, &whence))
            return 1;
        break;

    default:
        return ici_argcount(3);
    }
    return sys_ret(lseek((int)fd, ofs, whence));
}

/*
 * struct = wait()
 *
 * Wait for a child process to exit and return a struct
 * containing the process id (key of "pid") and the exit
 * status (key of "status").
 */
static int ici_sys_wait()
{
#ifdef _WINDOWS
    return not_on_win32("wait");
#else
    int         pid;
    struct_t    *s;
    int_t       *i;

#if NeXT
    union wait  status;
#else
    int status;
#endif
    if ((pid = wait(&status)) < 0)
	    return sys_ret(-1);
    if ((s = ici_struct_new()) == NULL)
	    return 1;
    if ((i = ici_int_new(pid)) == NULL)
        goto fail;
    if (ici_assign(s, ICISO(pid), i))
    {
        ici_decref(i);
        goto fail;
    }
    ici_decref(i);
#if NeXT
    if ((i = ici_int_new(status.w_retcode)) == NULL)
        goto fail;
#else
    if ((i = ici_int_new(status)) == NULL)
        goto fail;
#endif
    if (ici_assign(s, ICISO(status), i))
    {
        ici_decref(i);
        goto fail;
    }
    ici_decref(i);
    return ici_ret_with_decref(objof(s));

fail:
    ici_decref(s);
    return 1;
#endif /* _WINDOWS */
}

/*
 * string = getcwd()
 */
static int ici_sys_getcwd()
{
    char        buf[MAXPATHLEN+1];

    if (getcwd(buf, sizeof buf) == NULL)
        return sys_ret(-1);
    return ici_str_ret(buf);
}

#ifndef _WINDOWS

/*
 * array = passwd()
 * struct = passwd(string)
 * struct = passwd(int)
 *
 * Return an array of all password file entries or a single entry if given a user name
 * or user id. In the later case return NULL if no entry exists for that name/uid.
 *
 * Each password file entry is returned as a struct with the following keys,
 *
 *      name
 *      passwd
 *      uid
 *      gid
 *      gecos
 *      dir
 *      shell
 */
static struct_t *password_struct(struct passwd *);

static int ici_sys_passwd()
{
    struct passwd       *pwent;
    array_t             *a;

    switch (NARGS())
    {
    case 0:
        break;

    case 1:
        if (isint(ARG(0)))
            pwent = getpwuid((uid_t)intof(ARG(0))->i_value);
        else if (isstring(ARG(0)))
            pwent = getpwnam(stringof(ARG(0))->s_chars);
        else
            return ici_argerror(0);
        if (pwent == NULL)
            ici_error = "no such user";
        return ici_ret_with_decref(objof(password_struct(pwent)));

    default:
        return ici_argcount(1);
    }

    if ((a = ici_array_new(0)) == NULL)
        return 1;
    setpwent();
    while ((pwent = getpwent()) != NULL)
    {
        struct_t        *s;

        if (ici_stk_push_chk(a, 1) || (s = password_struct(pwent)) == NULL)
        {
            ici_decref(a);
            return 1;
        }
        *a->a_top++ = objof(s);
    }
    endpwent();
    return ici_ret_with_decref(objof(a));
}

static struct_t *
password_struct(struct passwd *pwent)
{
    struct_t    *d;
    object_t    *o;

    if (pwent == NULL)
        return NULL;
    if ((d = ici_struct_new()) != NULL)
    {

#define SET_INT_FIELD(x)                                        \
    if ((o = objof(ici_int_new(pwent->pw_ ## x))) == NULL)      \
        goto fail;                                              \
    else if (ici_assign(d, ICISO(x), o))                        \
    {                                                           \
        ici_decref(o);                                          \
        goto fail;                                              \
    }                                                           \
    else                                                        \
        ici_decref(o)

#define SET_STR_FIELD(x)                                        \
    if ((o = objof(ici_str_new_nul_term(pwent->pw_ ## x))) == NULL)\
        goto fail;                                              \
    else if (ici_assign(d, ICISO(x), o))                        \
    {                                                           \
        ici_decref(o);                                          \
        goto fail;                                              \
    }                                                           \
    else                                                        \
        ici_decref(o)

        SET_STR_FIELD(name);
        SET_STR_FIELD(passwd);
        SET_INT_FIELD(uid);
        SET_INT_FIELD(gid);
        SET_STR_FIELD(gecos);
        SET_STR_FIELD(dir);
        SET_STR_FIELD(shell);

#undef SET_STR_FIELD
#undef SET_INT_FIELD

    }
    return d;

fail:
    ici_decref(d);
    return NULL;
}

/*
 * string = getpass([string])
 */
static int ici_sys_getpass()
{
    char        *prompt = "Password: ";
    string_t    *pass;

    if (NARGS() > 0)
    {
        if (ici_typecheck("s", &prompt))
            return 1;
    }
    return ici_str_ret(getpass(prompt));
}


static int ici_sys_setpgrp()
{
#ifdef SETPGRP_2_ARGS
    long        pid, pgrp;
#else
    long        pgrp;
#endif

#ifdef SETPGRP_2_ARGS
    switch (NARGS())
    {
    case 1:
        pid = 0;
#endif
        if (ici_typecheck("i", &pgrp))
            return 1;
#ifdef SETPGRP_2_ARGS
        break;

    default:
        if (ici_typecheck("ii", &pid, &pgrp))
            return 1;
        break;
    }
    return sys_ret(setpgrp((pid_t)pid, (pid_t)pgrp));
#elif defined SETPGRP_0_ARGS
    return sys_ret(setpgrp());
#else
    return sys_ret(setpgrp((pid_t)pgrp));
#endif
}

static int ici_sys_flock()
{
    long        fd, operation;

    if (ici_typecheck("ii", &fd, &operation))
        return 1;
    return sys_ret(flock(fd, operation));
}

static int ici_sys_truncate()
{
    long        fd;
    long        len;
    char        *s;

    if (ici_typecheck("ii", &fd, &len) == 0)
    {
        if (ftruncate(fd, len) == -1)
            return sys_ret(-1L);
        return ici_null_ret();
    }
    if (ici_typecheck("si", &s, &len) == 0)
    {
        if (truncate(s, len) == -1)
            return ici_get_last_errno("truncate", s);
        return ici_null_ret();
    }
    return 1;
}
#endif

#ifndef _WINDOWS

static int
string_to_resource(object_t *what)
{
    if (what == ICISO(core))
        return RLIMIT_CORE;
    if (what == ICISO(cpu))
        return RLIMIT_CPU;
    if (what == ICISO(data))
        return RLIMIT_DATA;
    if (what == ICISO(fsize))
        return RLIMIT_FSIZE;
#   if defined(RLIMIT_MEMLOCK)
	if (what == ICISO(memlock))
	    return RLIMIT_MEMLOCK;
#   endif
    if (what == ICISO(nofile))
        return RLIMIT_NOFILE;
#   if defined(RLIMIT_NPROC)
	if (what == ICISO(nproc))
	    return RLIMIT_NPROC;
#   endif
#   if defined(RLIMIT_RSS)
	if (what == ICISO(rss))
	    return RLIMIT_RSS;
#   endif
    if (what == ICISO(stack))
        return RLIMIT_STACK;

#if !(defined __MACH__ && defined __APPLE__)
# define NO_RLIMIT_DBSIZE
#endif
#if __FreeBSD__ < 4
# define NO_RLIMIT_SBSIZE
#endif

#ifndef NO_RLIMIT_SBSIZE
    if (what == ICISO(sbsize))
        return RLIMIT_SBSIZE;
#endif
    return -1;
}

static int ici_sys_getrlimit()
{
    object_t            *what;
    int                 resource;
    struct rlimit       rlimit;
    struct_t            *limit;
    int_t               *iv;

    if (ici_typecheck("o", &what))
        return 1;
    if (isint(what))
        resource = intof(what)->i_value;
    else if (isstring(what))
    {
        if ((resource = string_to_resource(what)) == -1)
            return ici_argerror(0);
    }
    else
        return ici_argerror(0);

    if (getrlimit(resource, &rlimit) < 0)
        return sys_ret(-1);

    if ((limit = ici_struct_new()) == NULL)
        return 1;
    if ((iv = ici_int_new(rlimit.rlim_cur)) == NULL)
        goto fail;
    if (ici_assign(limit, ICISO(cur), iv))
    {
        ici_decref(iv);
        goto fail;
    }
    ici_decref(iv);
    if ((iv = ici_int_new(rlimit.rlim_max)) == NULL)
        goto fail;
    if (ici_assign(limit, ICISO(max), iv))
    {
        ici_decref(iv);
        goto fail;
    }
    ici_decref(iv);
    return ici_ret_with_decref(objof(limit));

fail:
    ici_decref(limit);
    return 1;
}

static int ici_sys_setrlimit()
{
    object_t            *what;
    object_t            *value;
    struct rlimit       rlimit;
    int                 resource;
    object_t            *iv;

    if (ici_typecheck("oo", &what, &value))
        return 1;
    if (isint(what))
        resource = intof(what)->i_value;
    else if (isstring(what))
    {
        if ((resource = string_to_resource(what)) == -1)
            return ici_argerror(0);
    }
    else
        return ici_argerror(0);

    if (isint(value))
    {
        if (getrlimit(resource, &rlimit) < 0)
            return sys_ret(-1);
        rlimit.rlim_cur = intof(value)->i_value;
    }
    else if (value == ICISO(infinity))
    {
        rlimit.rlim_cur = RLIM_INFINITY;
    }
    else if (isstruct(value))
    {
        if ((iv = ici_fetch(value, ICISO(cur))) == ici_null)
            goto fail;
        if (!isint(iv))
            goto fail;
        rlimit.rlim_cur = intof(iv)->i_value;
        if ((iv = ici_fetch(value, ICISO(max))) == ici_null)
            goto fail;
        if (!isint(iv))
            goto fail;
        rlimit.rlim_max = intof(iv)->i_value;
    }
    else
        return ici_argerror(1);

    return sys_ret(setrlimit(resource, &rlimit));

fail:
    ici_error = "invalid rlimit struct";
    return 1;
}



static int ici_sys_sleep()
{
    long	t;

    if (ici_typecheck("i", &t))
	return 1;
#ifndef NOSIGNALS
    ici_signals_blocking_syscall(1);
#endif
    sleep(t);
#ifndef NOSIGNALS
    ici_signals_blocking_syscall(0);
#endif
    return ici_null_ret();
}

static int ici_sys_usleep()
{
    long	t;

    if (ici_typecheck("i", &t))
	return 1;
#ifndef NOSIGNALS
    ici_signals_blocking_syscall(1);
#endif
    usleep(t);
#ifndef NOSIGNALS
    ici_signals_blocking_syscall(0);
#endif
}

#endif /* #ifndef _WINDOWS */

static cfunc_t ici_sys_cfuncs[] =
{
    /* utime */
    {CF_OBJ, "access",  ici_sys_access},
    {CF_OBJ, "chdir",   ici_sys_simple, chdir,  "s"},
    {CF_OBJ, "close",   ici_sys_close},
    {CF_OBJ, "creat",   ici_sys_creat},
    {CF_OBJ, "ctime",   ici_sys_ctime},
    {CF_OBJ, "dup",     ici_sys_dup},
    {CF_OBJ, "exec",    ici_sys_exec, execv},
    {CF_OBJ, "execp",   ici_sys_exec, execvp},
    {CF_OBJ, "exit",    ici_sys_simple, _exit,  "i"},
    {CF_OBJ, "fcntl",   ici_sys_fcntl},
    {CF_OBJ, "fdopen",  ici_sys_fdopen},
    {CF_OBJ, "fileno",  ici_sys_fileno},
    {CF_OBJ, "getcwd",  ici_sys_getcwd},
    {CF_OBJ, "lseek",   ici_sys_lseek},
    {CF_OBJ, "mkdir",   ici_sys_mkdir},
    {CF_OBJ, "mkfifo",  ici_sys_mkfifo},
    {CF_OBJ, "open",    ici_sys_open},
    {CF_OBJ, "pipe",    ici_sys_pipe},
    {CF_OBJ, "read",    ici_sys_read},
    {CF_OBJ, "readlink",ici_sys_readlink},
    {CF_OBJ, "rmdir",   ici_sys_simple, rmdir,  "s"},
    {CF_OBJ, "stat",    ici_sys_stat},
    {CF_OBJ, "symlink", ici_sys_symlink},
    {CF_OBJ, "time",    ici_sys_time},
    {CF_OBJ, "unlink",  ici_sys_simple, unlink, "s"}, /* should go as remove(}, is more portable */
    {CF_OBJ, "wait",    ici_sys_wait},
    {CF_OBJ, "write",   ici_sys_write},
#   ifdef _WINDOWS
        {CF_OBJ, "spawn",   ici_sys_spawn, spawnv},
        {CF_OBJ, "spawnp",  ici_sys_spawn, spawnvp},
#   endif
#   ifndef _WINDOWS
        /* poll */
        /* times */
        /* uname */
        {CF_OBJ, "acct",    ici_sys_simple, acct,   "s"},
        {CF_OBJ, "alarm",   ici_sys_simple, alarm,  "i"},
        {CF_OBJ, "chmod",   ici_sys_simple, chmod,  "si"},
        {CF_OBJ, "chown",   ici_sys_simple, chown,  "sii"},
        {CF_OBJ, "chroot",  ici_sys_simple, chroot, "s"},
        {CF_OBJ, "clock",   ici_sys_simple, clock,  ""},
        {CF_OBJ, "flock",   ici_sys_flock},
        {CF_OBJ, "fork",    ici_sys_simple, fork,   ""},
        {CF_OBJ, "getegid", ici_sys_simple, getegid,""},
        {CF_OBJ, "geteuid", ici_sys_simple, geteuid,""},
        {CF_OBJ, "getgid",  ici_sys_simple, getgid, ""},
        {CF_OBJ, "getitimer",ici_sys_getitimer},
        {CF_OBJ, "getpass", ici_sys_getpass},
        {CF_OBJ, "getpgrp", ici_sys_simple, getpgrp,""},
        {CF_OBJ, "getpid",  ici_sys_simple, getpid, ""},
        {CF_OBJ, "getppid", ici_sys_simple, getppid,""},
        {CF_OBJ, "getrlimit",ici_sys_getrlimit},
        {CF_OBJ, "gettimeofday",ici_sys_gettimeofday},
        {CF_OBJ, "getuid",  ici_sys_simple, getuid, ""},
        {CF_OBJ, "isatty",  ici_sys_simple, isatty, "i"},
        {CF_OBJ, "kill",    ici_sys_simple, kill,   "ii"},
        {CF_OBJ, "link",    ici_sys_simple, link,   "ss"},
        {CF_OBJ, "lstat",   ici_sys_lstat},
        {CF_OBJ, "mknod",   ici_sys_simple, mknod,  "sii"},
        {CF_OBJ, "nice",    ici_sys_simple, nice,   "i"},
        {CF_OBJ, "passwd",  ici_sys_passwd},
        {CF_OBJ, "pause",   ici_sys_simple, pause,  ""},
        {CF_OBJ, "rename",  ici_sys_simple, rename, "ss"},
        {CF_OBJ, "setgid",  ici_sys_simple, setgid, "i"},
        {CF_OBJ, "setitimer",ici_sys_setitimer},
        {CF_OBJ, "setlinebuf", ici_sys_setlinebuf},
        {CF_OBJ, "setpgrp", ici_sys_setpgrp},
        {CF_OBJ, "setrlimit",ici_sys_setrlimit},
        {CF_OBJ, "setuid",  ici_sys_simple, setuid, "i"},
        {CF_OBJ, "signal",  ici_sys_simple, signal, "ii"},
        {CF_OBJ, "sleep",   ici_sys_sleep},
        {CF_OBJ, "sync",    ici_sys_simple, sync,   ""},
        {CF_OBJ, "truncate",ici_sys_truncate},
        {CF_OBJ, "umask",   ici_sys_simple, umask,  "i"},
        {CF_OBJ, "usleep",  ici_sys_usleep},
#       if !defined(linux) && !defined(BSD4_4)
            {CF_OBJ, "lockf",   ici_sys_simple, lockf,  "iii"},
#       endif /* linux */
#       if !defined(NeXT) && !defined(BSD4_4)
            {CF_OBJ, "ulimit",  ici_sys_simple, ulimit, "ii"},
#       endif
#   endif
    {CF_OBJ}
};

object_t *
ici_sys_library_init(void)
{
    objwsup_t           *sys;

    if (ici_interface_check(ICI_VER, ICI_BACK_COMPAT_VER, "sys"))
        return NULL;
    if (init_ici_str())
        return NULL;
    if ((sys = ici_module_new(ici_sys_cfuncs)) == NULL)
        return NULL;
    if (ici_sys_vars_init(sys))
    {
        ici_decref(sys);
        return NULL;
    }
    return objof(sys);
}
