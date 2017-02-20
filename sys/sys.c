#include "ici.h"
#include "null.h"

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
#define	ICI_SYS_NOFLOCK
#endif

#ifdef IRIX
#include <signal.h>
#include <sys/param.h>
#endif

#if defined(linux)
#include <sys/signal.h>
#endif

#ifdef _WINDOWS
# define	ICI_SYS_NOPASSWD
# define	ICI_SYS_NOFLOCK
# define	ICI_SYS_NORLIMITS
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

#ifdef __linux__
#define SETPGRP_0_ARGS
#endif
#ifdef	BSD4_4
#define	SETPGRP_2_ARGS
#endif
#if defined __linux__ && BSD4_4
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

#define	F_OK	0
#define	W_OK	2
#define	R_OK	4

#endif

/*
 * Create pre-defined variables to replace C's #define's.
 */
int
ici_sys_vars_init(struct_t *scp)
{
    int		i;

#define	VALOF(x) { # x , x }
    static struct
    {
	char	*name;
	long	val;
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
#ifdef O_BINARY		/* WINDOWS has binary mode for open() */
	VALOF(O_BINARY),
#endif
#ifdef O_SYNC		/* WINDOWS doesn't have O_SYNC */
	VALOF(O_SYNC),
#endif
#ifdef O_NDELAY		/* WINDOWS doesn't have O_NDELAY */
	VALOF(O_NDELAY),
#endif
#ifdef O_NONBLOCK	/* NeXT doesn't have O_NONBLOCK */
	VALOF(O_NONBLOCK),
#endif
	VALOF(R_OK),
	VALOF(W_OK),
#ifdef X_OK		/* WINDOWS doesn't have X_OK */
	VALOF(X_OK),
#endif
	VALOF(F_OK),
	VALOF(SEEK_SET),
	VALOF(SEEK_CUR),
	VALOF(SEEK_END),

#ifndef ICI_SYS_NOFLOCK
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

extern int	access(), acct(), chdir(),
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
extern int	clock();
#endif

extern int	system();
#if 0
extern int	lockf();
#endif

#endif

#ifdef sun
int rename();
int lstat();
void setpwent();
void endpwent();
#endif

#ifdef NeXT
int	acct();
int	getppid(void);
int	lockf();
#endif
#endif /* _WINDOWS */

/*
 * Used as a common error return for system calls that fail. Sets the
 * global error string if the call fails otherwise returns the integer
 * result of the system call.
 */
static int
sys_ret(ret)
int	ret;
{
    if (ret < 0)
    {
	error = syserr();
	return 1;
    }
    return int_ret((long)ret);
}

/*
 * Used to call "simple" system calls. Simple calls are those that take
 * up to four integer parameters and return an integer. FUNCDEF() just
 * makes the name unique for this module. We have to use CFUNC3()
 * to use it, see below.
 */
FUNCDEF(simple)
{
    long	av[4];

    if (ici_typecheck(CF_ARG2(), &av[0], &av[1], &av[2], &av[3]))
	return 1;
    return sys_ret((*CF_ARG1())(av[0], av[1], av[2], av[3]));
}

NEED_STRING(dev);
NEED_STRING(ino);
NEED_STRING(mode);
NEED_STRING(nlink);
NEED_STRING(uid);
NEED_STRING(gid);
NEED_STRING(rdev);
NEED_STRING(size);
NEED_STRING(atime);
NEED_STRING(mtime);
NEED_STRING(ctime);
NEED_STRING(blksize);
NEED_STRING(blocks);
#ifndef ICI_SYS_NOPASSWD
NEED_STRING(name);
NEED_STRING(passwd);
/* NEED_STRING(uid) */	/* Already defined for stat() */
/* NEED_STRING(gid) */	/* Already defined for stat() */
NEED_STRING(gecos);
NEED_STRING(dir);
NEED_STRING(shell);
#endif
NEED_STRING(dupfd);
NEED_STRING(getfd);
NEED_STRING(setfd);
NEED_STRING(getfl);
NEED_STRING(setfl);
NEED_STRING(getown);
NEED_STRING(setown);
NEED_STRING(setlk);

NEED_STRING(sec);
NEED_STRING(usec);
NEED_STRING(infinity);
NEED_STRING(real);
NEED_STRING(virtual);
NEED_STRING(prof);
NEED_STRING(interval);
NEED_STRING(value);

/*
 * open(pathname, flags [, mode])
 *
 * Do the open(2) system call with the specified flags and mode (only needed
 * if flags includes O_CREAT).
 *
 * Returns the file descriptor (an int).
 */
FUNC(open)
{
    char	*fname;
    long	omode;
    long	perms = -1; /* -1 means not passed */

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
	error = "permission bits not specified in open()";
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
    sprintf(buf, "%s is not implemented on Win32 platforms", s);
    error = buf;
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
FUNC(fdopen)
{
#ifdef _WINDOWS	/* WINDOWS can't do fdopen() without lots of work */
    return not_on_win32("fdopen");
#else
    long	fd;
    char	*mode;
    FILE	*stream;
    file_t	*f;

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
	error = "can't fdopen";
	return 1;
    }
    setvbuf(stream, NULL, _IOLBF, 0);
    if ((f = new_file((char *)stream, &stdio_ftype, NULL)) == NULL)
    {
	fclose(stream);
	return 1;
    }
    return ici_ret_with_decref(objof(f));
#endif /* _WINDOWS */
}

FUNC(close)
{
    int	rc;

    if (NARGS() != 1)
	return ici_argcount(1);
    if (isint(ARG(0)))
	rc = close(intof(ARG(0))->i_value);
    else if (isarray(ARG(0)))
    {
	array_t	*a = arrayof(ARG(0));

	if
	(
	    a->a_top - a->a_base != 2
	    ||
	    !isint(a->a_base[0])
	    ||
	    !isint(a->a_base[1])
	)
	{
	    error = "invalid fd array passed to sys.close";
	    return 1;
	}
	rc = close(intof(a->a_base[0])->i_value);
	if (rc == 0)
	    rc = close(intof(a->a_base[1])->i_value);
    }
    else
	return ici_argerror(0);
    return sys_ret(rc);
}


#ifndef _WINDOWS

/* Convert a struct to a struct flock for fcntl's F_SETLK */

NEED_STRING(start);
NEED_STRING(len);
NEED_STRING(pid);
NEED_STRING(type);
NEED_STRING(whence);
NEED_STRING(rdlck);
NEED_STRING(wrlck);
NEED_STRING(unlck);

static int
struct_to_flock(struct_t *d, struct flock *flock)
{
    object_t	*o;

    if ((o = fetch(d, STRING(start))) == objof(&o_null))
	flock->l_start = 0;
    else
	flock->l_start = intof(o)->i_value;
    if ((o = fetch(d, STRING(len))) == objof(&o_null))
	flock->l_len = 0;
    else
	flock->l_len = intof(o)->i_value;
    if ((o = fetch(d, STRING(type))) == objof(&o_null))
	flock->l_type = F_RDLCK;
    else if (isstring(o))
    {
	if (stringof(o) == STRING(rdlck))
	    flock->l_type = F_RDLCK;
	else if (stringof(o) == STRING(wrlck))
	    flock->l_type = F_WRLCK;
	else if (stringof(o) == STRING(unlck))
	    flock->l_type = F_UNLCK;
	else
	    goto bad_lock_type;
    }
    else if (isint(o))
	flock->l_type = intof(o)->i_value;
    else
    {
bad_lock_type:
	error = "invalid lock type";
	return 1;
    }
    if ((o = fetch(d, STRING(whence))) == objof(&o_null))
	flock->l_whence = SEEK_SET;
    else
	flock->l_whence = intof(o)->i_value;
    return 0;
}
#endif	/* _WINDOWS */

/*
 * fcntl(int, what)
 */
FUNC(fcntl)
{
#ifdef _WINDOWS
    return not_on_win32("fdopen");
#else
    long	fd;
    string_t	*what;
    object_t	*arg;
    int		iarg;
    int		iwhat;
    int		r;

    NEED_STRINGS(1);
    switch (NARGS())
    {
    case 2:
	iarg = 1;
	arg = objof(&o_null);
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
    if (what == STRING(dupfd))
	iwhat = F_DUPFD;
    else if (what == STRING(getfd))
	iwhat = F_GETFD;
    else if (what == STRING(setfd))
	iwhat = F_SETFD;
    else if (what == STRING(getfl))
	iwhat = F_GETFL;
    else if (what == STRING(setfl))
	iwhat = F_SETFL;
    else if (what == STRING(getown))
	iwhat = F_GETOWN;
    else if (what == STRING(setown))
	iwhat = F_SETOWN;
    else if (what == STRING(setlk))
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

FUNC(fileno)
{
    file_t	*f;

    if (ici_typecheck("u", &f))
	return 1;
    if (f->f_type != &stdio_ftype && f->f_type != &ici_popen_ftype)
    {
	error = "attempt to obtain file descriptor of non-stdio file";
	return 1;
    }
    return int_ret(fileno((FILE *)f->f_file));
}

FUNC(setlinebuf)
{
    file_t	*file;
    
    if (ici_typecheck("u", &file))
	return 1;
    if (file->f_type->ft_getch == fgetc)
	setlinebuf((FILE *)file->f_file);
    return null_ret();
}


FUNC(mkdir)
{
    char	*path;

#ifdef _WINDOWS
    if (ici_typecheck("s", &path))
	return 1;
    if (mkdir(path) == -1)
	return sys_ret(-1);
#else
    long	mode = 0777;

    if (NARGS() == 1)
    {
	if (ici_typecheck("s", &path))
	    return 1;
    }
    else if (ici_typecheck("si", &path, &mode))
	return 1;
    if (mkdir(path, mode) == -1)
	return sys_ret(-1);
#endif
    return ici_ret_no_decref(objof(&o_null));
}

FUNC(mkfifo)
{
#ifdef _WINDOWS	/* WINDOWS can't do fdopen() without lots of work */
    return not_on_win32("fdopen");
#else
    char	*path;
    long	mode;

    if (ici_typecheck("si", &path, &mode))
	return 1;
    if (mkfifo(path, mode) == -1)
	return sys_ret(-1);
    return ici_ret_no_decref(objof(&o_null));
#endif /* _WINDOWS */
}

FUNC(read)
{
    long	fd;
    long	len;
    string_t	*s;
    int		r;
    char	*msg;

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
	return null_ret();
    }
    if ((s = new_string(r)) == NULL)
    {
	ici_free(msg);
	return 1;
    }
    memcpy(s->s_chars, msg, r);
    ici_hash_string(objof(s));
    ici_free(msg);
    return ici_ret_with_decref(objof(s));
}

FUNC(write)
{
    long	fd;
    object_t	*o;
    char	*addr;
    long	sz;
    int		havesz = 0;

    if (ici_typecheck("io", &fd, &o))
    {
	if (ici_typecheck("ioi", &fd, &o, &sz))
	    return 1;
	havesz = 1;
    }
    if (isstring(o))
    {
	addr = (char *)stringof(o)->s_chars;
	if (!havesz)
	    sz = stringof(o)->s_nchars;
    }
    else if (ismem(o))
    {
	addr = (char *)memof(o)->m_base;
	if (!havesz)
	    sz = memof(o)->m_length * memof(o)->m_accessz;
    }
    else
    {
	return ici_argerror(1);
    }
    return sys_ret(write((int)fd, addr, sz));
}

FUNC(symlink)
{
#ifdef _WINDOWS	/* WINDOWS can't do fdopen() without lots of work */
    return not_on_win32("fdopen");
#else
    char	*a, *b;

    if (ici_typecheck("ss", &a, &b))
	return 1;
    if (symlink(a, b) == -1)
	return sys_ret(-1);
    return ici_ret_no_decref(objof(&o_null));
#endif /* _WINDOWS */
}

FUNC(readlink)
{
#ifdef _WINDOWS	/* WINDOWS can't do fdopen() without lots of work */
    return not_on_win32("fdopen");
#else
    char	*path;
    char	pbuf[MAXPATHLEN+1];

    if (ici_typecheck("s", &path))
	return 1;
    if (readlink(path, pbuf, sizeof pbuf) == -1)
	return sys_ret(-1);
    return ici_ret_with_decref(objof(new_cname(pbuf)));
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
 *	atime		blksize		blocks		ctime
 *	dev		gid		ino		mode
 *	mtime		nlink		rdev		size
 *	uid
 *
 * These correspond to the fields of the same names (with a st_ prefix)
 * in the struct stat returned by the system call.
 */
FUNC(stat)
{
    object_t	*o;
    struct stat	statb;
    int		rc;
    struct_t	*s;

    NEED_STRINGS(1);
    if (NARGS() != 1)
	return ici_argcount(1);
    o = ARG(0);
    if (isint(o))
	rc = fstat(intof(o)->i_value, &statb);
    else if (isstring(o))
	rc = stat(stringof(o)->s_chars, &statb);
    else if (isfile(o) && fileof(o)->f_type == &stdio_ftype)
	rc = fstat(fileno((FILE *)fileof(o)->f_file), &statb);
    else
	return ici_argerror(0);
    if (rc == -1)
	return sys_ret(rc);
    if ((s = new_struct()) == NULL)
	return 1;
#define	SETFIELD(x)						\
    if ((o = objof(new_int(statb.st_ ## x))) == NULL)		\
	goto fail;						\
    else if (assign(s, STRING(x), o))				\
    {								\
	decref(o);						\
	goto fail;						\
    }								\
    else							\
	decref(o)

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
    decref(s);
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
 *	atime		blksize		blocks		ctime
 *	dev		gid		ino		mode
 *	mtime		nlink		rdev		size
 *	uid
 *
 * These correspond to the fields of the same names (with a st_ prefix)
 * in the struct stat returned by the system call.
 */
FUNC(lstat)
{
    object_t	*o;
    struct stat	statb;
    int		rc;
    struct_t	*s;

    NEED_STRINGS(1);
    if (NARGS() != 1)
	return ici_argcount(1);
    o = ARG(0);
    if (isstring(o))
	rc = lstat(stringof(o)->s_chars, &statb);
    else
	return ici_argerror(0);
    if (rc == -1)
	return sys_ret(rc);
    if ((s = new_struct()) == NULL)
	return 1;
#define	SETFIELD(x)						\
    if ((o = objof(new_int(statb.st_ ## x))) == NULL)		\
	goto fail;						\
    else if (assign(s, STRING(x), o))				\
    {								\
	decref(o);						\
	goto fail;						\
    }								\
    else							\
	decref(o)

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
    decref(s);
    return 1;
}
#endif

/*
 * ctime(int)
 *
 * Convert a time value to a string. Takes a count of the number of
 * seconds past the epoch and returns a string (see ctime(3)).
 */
FUNC(ctime)
{
    long	timev;
    string_t	*s;

    if (ici_typecheck("i", &timev) || (s = new_cname(ctime(&timev))) == NULL)
	return 1;
    return ici_ret_with_decref(objof(s));
}

/*
 * time()
 *
 * time() returns the time since 00:00:00 GMT,  Jan.  1,  1970 (the epoch)
 * measured in seconds.
 */
FUNC(time)
{
    return int_ret(time(NULL));
}

static int
assign_timeval(struct_t *s, string_t *k, struct timeval *tv)
{
    struct_t	*ss;
    int_t	*i;

    if (k == NULL)
	ss = s;
    else if ((ss = new_struct()) == NULL)
	return 1;
    if ((i = new_int(tv->tv_usec)) == NULL)
	goto fail;
    if (assign(ss, STRING(usec), i))
    {
	decref(i);
	goto fail;
    }
    decref(i);
    if ((i = new_int(tv->tv_sec)) == NULL)
	goto fail;
    if (assign(ss, STRING(sec), i))
    {
	decref(i);
	goto fail;
    }
    decref(i);
    if (k != NULL && assign(s, k, ss))
	goto fail;
    return 0;

fail:
    if (k != NULL)
	decref(ss);
    return 1;
}

/*
 * struct = getitimer(string)
 */
FUNC(getitimer)
{
    long		which = ITIMER_VIRTUAL;
    struct_t		*s;
    struct itimerval	value;
    object_t		*o;

    NEED_STRINGS(1);
    if (NARGS() != 0)
    {
	if (ici_typecheck("o", &o))
	    return 1;
	if (!isstring(o))
	    return ici_argerror(0);
	if (o == objof(STRING(real)))
	    which = ITIMER_REAL;
	else if (o == objof(STRING(virtual)))
	    which = ITIMER_VIRTUAL;
	else if (o == objof(STRING(prof)))
	    which = ITIMER_PROF;
	else
	    return ici_argerror(0);
    }
    if (getitimer(which, &value) == -1)
	return sys_ret(-1);
    if ((s = new_struct()) == NULL)
	return 1;
    if
    (
	assign_timeval(s, STRING(interval), &value.it_interval)
	||
	assign_timeval(s, STRING(value), &value.it_value)
    )
    {
	decref(s);
	return 1;
    }
    return ici_ret_with_decref(objof(s));
}

static int
fetch_timeval(object_t *s, struct timeval *tv)
{
    object_t	*o;

    if (!isstruct(s))
	return 1;
    if ((o = fetch(s, STRING(usec))) == objof(&o_null))
	tv->tv_usec = 0;
    else if (isint(o))
	tv->tv_usec = intof(o)->i_value;
    else
	return 1;
    if ((o = fetch(s, STRING(sec))) == objof(&o_null))
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
FUNC(setitimer)
{
    long		which = ITIMER_VIRTUAL;
    struct_t		*s;
    struct itimerval	value;
    struct itimerval	ovalue;
    object_t		*o;

    NEED_STRINGS(1);
    if (NARGS() == 1)
    {
	if (ici_typecheck("d", &s))
	    return 1;
    }
    else
    {
	if (ici_typecheck("od", &o, &s))
	    return 1;
	if (o == objof(STRING(real)))
	    which = ITIMER_REAL;
	else if (o == objof(STRING(virtual)))
	    which = ITIMER_VIRTUAL;
	else if (o == objof(STRING(prof)))
	    which = ITIMER_PROF;
	else
	    return ici_argerror(0);
    }
    if ((o = fetch(s, STRING(interval))) == objof(&o_null))
	value.it_interval.tv_sec = value.it_interval.tv_usec = 0;
    else if (fetch_timeval(o, &value.it_interval))
	goto invalid_itimerval;
    if ((o = fetch(s, STRING(value))) == objof(&o_null))
	value.it_value.tv_sec = value.it_value.tv_usec = 0;
    else if (fetch_timeval(o, &value.it_value))
	goto invalid_itimerval;
    if (setitimer(which, &value, &ovalue) == -1)
	return sys_ret(-1);
    if ((s = new_struct()) == NULL)
	return 1;
    if
    (
	assign_timeval(s, STRING(interval), &ovalue.it_interval)
	||
	assign_timeval(s, STRING(value), &ovalue.it_value)
    )
    {
	decref(s);
	return 1;
    }
    return ici_ret_with_decref(objof(s));

invalid_itimerval:
    error = "invalid itimer struct";
    return 1;
}

FUNC(gettimeofday)
{
    struct_t		*s;
    struct timeval	tv;

    NEED_STRINGS(1);
    if (gettimeofday(&tv, NULL) == -1)
	return sys_ret(-1);
    if ((s = new_struct()) == NULL)
	return 1;
    if (assign_timeval(s, NULL, &tv))
    {
	decref(s);
	return 1;
    }
    return ici_ret_with_decref(objof(s));
}


/*
 * access(pathname [,flags])
 *
 * Determine accessibility of a file. If flags is not passed it defaults
 * to F_OK.
 *
 */
FUNC(access)
{
    char	*fname;
    int		bits = F_OK;

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
FUNC(pipe)
{
#ifdef _WINDOWS
    return not_on_win32("pipe");
#else
    int		pfd[2];
    array_t	*a;
    int_t	*fd;

    if ((a = new_array()) == NULL)
	return 1;
    if (pipe(pfd) == -1)
    {
	decref(a);
	return sys_ret(-1);
    }
    if ((fd = new_int(pfd[0])) == NULL)
	goto fail;
    decref(fd);
    if (assign(a, o_zero, fd))
	goto fail;
    if ((fd = new_int(pfd[1])) == NULL)
	goto fail;
    decref(fd);
    if (assign(a, o_one, fd))
	goto fail;
    return ici_ret_with_decref(objof(a));

fail:
    decref(a);
    close(pfd[0]);
    close(pfd[1]);
    return 1;
#endif	/* #ifndef _WINDOWS */
}

/*
 * creat(pathname, mode)
 *
 * Create the named file with the given permissions.
 */
FUNC(creat)
{
    char	*fname;
    long	perms;
    int		fd;

    if (ici_typecheck("si", &fname, &perms))
	return 1;
    if ((fd = creat(fname, perms)) == -1)
	return sys_ret(-1);
    return int_ret(fd);
}

/*
 * dup(int [, int])
 *
 * Perform a dup(2) or dup2(2) system call depending upon args.
 */
FUNC(dup)
{
    long	fd1;
    long	fd2;

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
	return fd2 < 0 ? sys_ret(fd2) : int_ret(fd2);
    }
    if (dup2(fd1, fd2) < 0)
	return sys_ret(-1);
    return int_ret(fd2);
}

/*
 * exec(pathname, array)
 * exec(pathname, arg0, arg1...)
 *
 * Execute a program. The first form uses an array to store the
 * argyuments passed to the program, the second uses a variable
 * number of arguments.
 *
 */
FUNCDEF(exec)
{
    char	*sargv[16];
    char	**argv;
    int		maxargv;
    int		n;
    object_t	**o;
    char	*path;
    int		argc;

#define	ADDARG(X)							\
    {									\
	if (argc >= 16)							\
	{								\
	    if (argc >= maxargv)					\
	    {								\
		char	**newp;						\
		int	 i;						\
		maxargv += argc;					\
		if ((newp = ici_alloc(maxargv * sizeof (char *))) == NULL)	\
		{							\
		    if (argv != sargv)					\
			ici_free(argv);					\
		    return 1;						\
		}							\
		for (i = 0; i < argc; ++i)				\
		    newp[i] = argv[i];					\
		if (argv != sargv)					\
		    ici_free(argv);					\
		argv = newp;						\
	    }								\
	}								\
	argv[argc++] = (X);						\
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
	for (p = arrayof(*o)->a_base; p < arrayof(*o)->a_top; ++p)
	    if (isstring(*p))
		ADDARG(stringof(*p)->s_chars);
    }
    ADDARG(NULL);
    n = (*CF_ARG1())(path, argv);
    if (argv != sargv)
	ici_free(argv);
    return sys_ret(n);
}

CFUNC1(exec, exec, execv);
CFUNC1(execp, exec, execvp);

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
FUNCDEF(spawn)
{
#ifndef _WINDOWS
    error = "spawn is only implemented on Win32 platforms";
    return 1;
#else
    char	*sargv[16];
    char	**argv;
    int		maxargv;
    int		n;
    object_t	**o;
    char	*path;
    int		argc;
    int		mode = _P_NOWAIT;

#define	ADDARG(X)							\
    {									\
	if (argc >= 16)							\
	{								\
	    if (argc >= maxargv)					\
	    {								\
		char	**newp;						\
		int	 i;						\
		maxargv += argc;					\
		if ((newp = ici_alloc(maxargv * sizeof (char *))) == NULL)	\
		{							\
		    if (argv != sargv)					\
			ici_free(argv);					\
		    return 1;						\
		}							\
		for (i = 0; i < argc; ++i)				\
		    newp[i] = argv[i];					\
		if (argv != sargv)					\
		    ici_free(argv);					\
		argv = newp;						\
	    }								\
	}								\
	argv[argc++] = (X);						\
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
	for (p = arrayof(*o)->a_base; p < arrayof(*o)->a_top; ++p)
	    if (isstring(*p))
		ADDARG(stringof(*p)->s_chars);
    }
    ADDARG(NULL);
    n = (*CF_ARG1())(mode, path, argv);
    if (argv != sargv)
	ici_free(argv);
    if (n == -1)
	return sys_ret(n);
    return int_ret(n);
#endif /* WINDOWS */
}

CFUNC1(spawn, spawn, spawnv);
CFUNC1(spawnp, spawn, spawnvp);

/*
 * lseek(int, int [, int])
 *
 * Set a file descriptor's I/O position. If 'whence' not passed
 * it defaults to SEEK_SET.
 */
FUNC(lseek)
{
    int_t	*fd;
    long	ofs;
    long	whence = SEEK_SET;

    switch (NARGS())
    {
    case 2:
	if (ici_typecheck("ii", &fd, &ofs))
	    return 1;
	break;
    case 3:
	if (ici_typecheck("iii", &fd, &ofs, &whence))
	    return 1;
    default:
	return ici_argcount(3);
    }
    return sys_ret(lseek(fd->i_value, ofs, whence));
}

/*
 * wait()
 *
 * Wait for a child process to exit and return the exit status.
 */
FUNC(wait)
{
#ifdef _WINDOWS
    return not_on_win32("wait");
#else

#if NeXT
    union wait	status;
#else
    int	status;
#endif
    if (wait(&status) < 0)
	return sys_ret(-1);
#if NeXT
    return int_ret(status.w_retcode);
#else
    return int_ret(status);
#endif
#endif /* _WINDOWS */
}

#ifndef ICI_SYS_NODIR

#ifdef _WINDOWS
/*
 * Emulate opendir/readdir/et al under WINDOWS environments via findfirst/
 * findnext. Only what f_dir() needs has been emulated (which is to say,
 * not much).
 */

#define	MAXPATHLEN	_MAX_PATH

struct dirent
{
    char	*d_name;
    int		d_namlen;
};

typedef struct DIR
{
    long		handle;
    struct _finddata_t	finddata;
    int			needfindnext;
    struct dirent	dirent;
}
DIR;

static DIR *
opendir(const char *path)
{
    DIR		*dir;
    char	fspec[_MAX_PATH+1];

    if (strlen(path) > (_MAX_PATH - 4))
	return NULL;
    sprintf(fspec, "%s/*.*", path);
    if ((dir = talloc(DIR)) != NULL)
    {
	if ((dir->handle = _findfirst(fspec, &dir->finddata)) == -1)
	{
	    ici_free(dir);
	    return NULL;
	}
	dir->needfindnext = 0;
    }
    return dir;
}

static struct dirent *
readdir(DIR *dir)
{
    if (dir->needfindnext && _findnext(dir->handle, &dir->finddata) != 0)
	    return NULL;
    dir->dirent.d_name = dir->finddata.name;
    dir->dirent.d_namlen = strlen(dir->dirent.d_name);
    dir->needfindnext = 1;
    return &dir->dirent;
}

static void
closedir(DIR *dir)
{
    _findclose(dir->handle);
    ici_free(dir);
}

#ifndef S_ISREG
#define	S_ISREG(m)	(((m) & _S_IFMT) == _S_IFREG)
#endif
#ifndef S_ISDIR
#define	S_ISDIR(m)	(((m) & _S_IFMT) == _S_IFDIR)
#endif

#endif

/*
 * array = dir([path] [, regexp] [, format])
 *
 * Read directory named in path (a string, defaulting to ".", the current
 * working directory) and return the entries that match the pattern (or
 * all names if no pattern passed). The format string identifies what
 * sort of entries should be returned. If the format string is passed
 * then a path MUST be passed (to avoid any ambiguity) but path may be
 * NULL meaning the current working directory (same as "."). The format
 * string uses the following characters,
 *
 *	f	return file names
 *	d	return directory names
 *	a	return all names (which includes things other than
 *		files and directories, e.g., hidden or special files
 *
 * The default format specifier is "f".
 */
FUNC(dir)
{
    char		*path   = ".";
    char		*format = "f";
    regexp_t		*regexp = NULL;
    object_t		*o;
    array_t		*a;
    DIR			*dir;
    struct dirent	*dirent;
    int			fmt;
    string_t		*s;

    switch (NARGS())
    {
    case 0:
	break;

    case 1:
	o = ARG(0);
	if (isstring(o))
	    path = stringof(o)->s_chars;
	else if (isnull(o))
	    ;	/* leave path as is */
	else if (isregexp(o))
	    regexp = regexpof(o);
	else
	    return ici_argerror(0);
	break;

    case 2:
	o = ARG(0);
	if (isstring(o))
	    path = stringof(o)->s_chars;
	else if (isnull(o))
	    ;	/* leave path as is */
	else if (isregexp(o))
	    regexp = regexpof(o);
	else
	    return ici_argerror(0);
	o = ARG(1);
	if (isregexp(o))
	{
	    if (regexp != NULL)
		return ici_argerror(1);
	    regexp = regexpof(o);
	}
	else if (isstring(o))
	    format = stringof(o)->s_chars;
	else
	    return ici_argerror(1);
	break;

    case 3:
	o = ARG(0);
	if (isstring(o))
	    path = stringof(o)->s_chars;
	else if (isnull(o))
	    ;	/* leave path as is */
	else
	    return ici_argerror(0);
	o = ARG(1);
	if (!isregexp(o))
	    return ici_argerror(1);
	regexp = regexpof(o);
	o = ARG(2);
	if (!isstring(o))
	    return ici_argerror(2);
	format = stringof(o)->s_chars;
	break;

    default:
	return ici_argcount(3);
    }

    if (*path == '\0')
	path = ".";

#define	FILES	1
#define	DIRS	2
#define	OTHERS	4

    for (fmt = 0; *format != '\0'; ++format)
    {
	switch (*format)
	{
	case 'f':
	    fmt |= FILES;
	    break;

	case 'd':
	    fmt |= DIRS;
	    break;

	case 'a':
	    fmt |= OTHERS | DIRS | FILES;
	    break;

	default:
	    error = "bad directory format specifier";
	    return 1;
	}
    }
    if ((a = new_array()) == NULL)
	return 1;
    if ((dir = opendir(path)) == NULL)
    {
	if (chkbuf(strlen(path) + 32))
	    error = syserr();
	else
	{
	    sprintf(buf, "%s: %s", path, syserr());
	    error = buf;
	}
	goto fail;
    }
    while ((dirent = readdir(dir)) != NULL)
    {
	struct stat	statbuf;
	char		abspath[MAXPATHLEN+1];

	if
        (
	    regexp != NULL
	    &&
	    pcre_exec
	    (
		regexp->r_re,
		regexp->r_rex,
		dirent->d_name,
#if defined(linux) || defined(SUNOS5)
		dirent->d_reclen,
#else
		dirent->d_namlen,
#endif
#if defined(PCRE_MINOR) && (PCRE_MINOR >= 8)
		0,
#endif
		0,
		re_bra,
		nels(re_bra)
	    )
	    < 0
	)
	    continue;
	sprintf(abspath, "%s/%s", path, dirent->d_name);
#ifndef	_WINDOWS
	if (lstat(abspath, &statbuf) == -1)
	{
	    if (chkbuf(strlen(abspath) + 32))
		error = syserr();
	    else
	    {
		sprintf(buf, "%s: %s", abspath, syserr());
		error = buf;
	    }
	    closedir(dir);
	    goto fail;
	}
	if (S_ISLNK(statbuf.st_mode) && stat(abspath, &statbuf) == -1)
	    continue;
#else
	if (stat(abspath, &statbuf) == -1)
	    continue;
#endif
	if
	(
	    (S_ISREG(statbuf.st_mode) && fmt & FILES)
	    ||
	    (S_ISDIR(statbuf.st_mode) && fmt & DIRS)
	    ||
	    fmt & OTHERS
	)
	{
	    if
	    (
		(s = new_cname(dirent->d_name)) == NULL
		||
		pushcheck(a, 1)
	    )
	    {
		if (s)
		    decref(s);
		closedir(dir);
		goto fail;
	    }
	    *a->a_top++ = objof(s);
	    decref(s);
	}
    }
    closedir(dir);
    return ici_ret_with_decref(objof(a));

#undef	FILES
#undef	DIRS
#undef	OTHERS

fail:
    decref(a);
    return 1;
}
#endif	/* ICI_SYS_NODIR */

/*
 * string = getcwd()
 */
FUNC(getcwd)
{
    char	buf[MAXPATHLEN+1];

    if (getcwd(buf, sizeof buf) == NULL)
	return sys_ret(-1);
    return str_ret(buf);
}

#ifndef ICI_SYS_NOPASSWD
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
 *	name
 *	passwd
 *	uid
 *	gid
 *	gecos
 *	dir
 *	shell
 */
static struct_t	*password_struct(struct passwd *);

FUNC(passwd)
{
    struct passwd	*pwent;
    array_t		*a;

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
	    error = "no such user";
	return ici_ret_with_decref(objof(password_struct(pwent)));

    default:
	return ici_argcount(1);
    }

    if ((a = new_array()) == NULL)
	return 1;
    setpwent();
    while ((pwent = getpwent()) != NULL)
    {
	struct_t	*s;

	if (pushcheck(a, 1) || (s = password_struct(pwent)) == NULL)
	{
	    decref(a);
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
    struct_t	*d;
    object_t	*o;

    NEED_STRINGS(NULL);
    if (pwent == NULL)
	return NULL;
    if ((d = new_struct()) != NULL)
    {

#define	SET_INT_FIELD(x)					\
    if ((o = objof(new_int(pwent->pw_ ## x))) == NULL)		\
	goto fail;						\
    else if (assign(d, STRING(x), o))				\
    {								\
	decref(o);						\
	goto fail;						\
    }								\
    else							\
	decref(o)

#define	SET_STR_FIELD(x)					\
    if ((o = objof(new_cname(pwent->pw_ ## x))) == NULL)	\
	goto fail;						\
    else if (assign(d, STRING(x), o))				\
    {								\
	decref(o);						\
	goto fail;						\
    }								\
    else							\
	decref(o)

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
    decref(d);
    return NULL;
}
#endif

/*
 * string = getpass([string])
 */
FUNC(getpass)
{
    char	*prompt = "Password: ";
    string_t	*pass;

    if (NARGS() > 0)
    {
	if (ici_typecheck("s", &prompt))
	    return 1;
    }
    return str_ret(getpass(prompt));
}


#ifndef _WINDOWS
FUNC(setpgrp)
{
#ifdef SETPGRP_2_ARGS
    long	pid, pgrp;
#else
    long	pgrp;
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
#endif

#ifndef ICI_SYS_NOFLOCK
FUNC(flock)
{
    long	fd, operation;

    if (ici_typecheck("ii", &fd, &operation))
	return 1;
    return sys_ret(flock(fd, operation));
}
#endif

#ifndef _WINDOWS
#ifndef ICI_SYS_NOTRUNCATE
FUNC(truncate)
{
    long	fd;
    long	len;
    char	*s;

    if (ici_typecheck("ii", &fd, &len) == 0)
    {
	if (ftruncate(fd, len) == -1)
	    return sys_ret(-1);
	return null_ret();
    }
    if (ici_typecheck("si", &s, &len) == 0)
    {
	if (truncate(s, len) == -1)
	    return sys_ret(-1);
	return null_ret();
    }
    return 1;
}
#endif

#ifndef ICI_SYS_NORLIMITS

NEED_STRING(core);
NEED_STRING(cpu);
NEED_STRING(data);
NEED_STRING(fsize);
NEED_STRING(memlock);
NEED_STRING(nofile);
NEED_STRING(nproc);
NEED_STRING(rss);
NEED_STRING(stack);
NEED_STRING(sbsize);
NEED_STRING(cur);
NEED_STRING(max);

static int
string_to_resource(object_t *what)
{
    if (stringof(what) == STRING(core))
	return RLIMIT_CORE;
    if (stringof(what) == STRING(cpu))
	return RLIMIT_CPU;
    if (stringof(what) == STRING(data))
	return RLIMIT_DATA;
    if (stringof(what) == STRING(fsize))
	return RLIMIT_FSIZE;
    if (stringof(what) == STRING(memlock))
	return RLIMIT_MEMLOCK;
    if (stringof(what) == STRING(nofile))
	return RLIMIT_NOFILE;
    if (stringof(what) == STRING(nproc))
	return RLIMIT_NPROC;
    if (stringof(what) == STRING(rss))
	return RLIMIT_RSS;
    if (stringof(what) == STRING(stack))
	return RLIMIT_STACK;

#if !(defined __MACH__ && defined __APPLE__)
# define NO_RLIMIT_DBSIZE
#endif
#if __FreeBSD__ < 4
# define NO_RLIMIT_SBSIZE
#endif

#ifndef NO_RLIMIT_SBSIZE
    if (stringof(what) == STRING(sbsize))
	return RLIMIT_SBSIZE;
#endif
    return -1;
}

FUNC(getrlimit)
{
    object_t		*what;
    int			resource;
    struct rlimit	rlimit;
    struct_t		*limit;
    int_t		*iv;

    NEED_STRINGS(1);
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

    if ((limit = new_struct()) == NULL)
	return 1;
    if ((iv = new_int(rlimit.rlim_cur)) == NULL)
	goto fail;
    if (assign(limit, STRING(cur), iv))
    {
	decref(iv);
	goto fail;
    }
    decref(iv);
    if ((iv = new_int(rlimit.rlim_max)) == NULL)
	goto fail;
    if (assign(limit, STRING(max), iv))
    {
	decref(iv);
	goto fail;
    }
    decref(iv);
    return ici_ret_with_decref(objof(limit));
    
fail:
    decref(limit);
    return 1;
}

NEED_STRING(infinity);

FUNC(setrlimit)
{
    object_t		*what;
    object_t		*value;
    struct rlimit	rlimit;
    int			resource;
    object_t		*iv;

    NEED_STRINGS(1);
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
    else if (value == objof(STRING(infinity)))
    {
	rlimit.rlim_cur = RLIM_INFINITY;
    }
    else if (isstruct(value))
    {
	if ((iv = fetch(value, STRING(cur))) == objof(&o_null))
	    goto fail;
	if (!isint(iv))
	    goto fail;
	rlimit.rlim_cur = intof(iv)->i_value;
	if ((iv = fetch(value, STRING(max))) == objof(&o_null))
	    goto fail;
	if (!isint(iv))
	    goto fail;
	rlimit.rlim_max = intof(iv)->i_value;
    }
    else
	return ici_argerror(1);

    return sys_ret(setrlimit(resource, &rlimit));

fail:
    error = "invalid rlimit struct";
    return 1;
}

#endif
#endif /* #ifndef _WINDOWS */

CFUNC3(alarm,	simple,	alarm,	"i")
CFUNC3(acct,	simple,	acct,	"s")
CFUNC3(chdir,	simple,	chdir,	"s")
#ifndef _WINDOWS
CFUNC3(chmod,	simple,	chmod,	"si")
CFUNC3(chown,	simple,	chown,	"sii")
CFUNC3(chroot,	simple,	chroot,	"s")
#endif
    /*### fcntl */
CFUNC3(exit,	simple,	_exit,	"i")
#ifndef _WINDOWS
CFUNC3(fork,	simple,	fork,	"")
CFUNC3(getpid,	simple,	getpid,	"")
CFUNC3(getpgrp,	simple,	getpgrp,"")
CFUNC3(getppid,	simple,	getppid,"")
CFUNC3(getuid,	simple,	getuid,	"")
CFUNC3(geteuid,	simple,	geteuid,"")
CFUNC3(getgid,	simple,	getgid,	"")
CFUNC3(getegid,	simple,	getegid,"")
CFUNC3(kill,	simple,	kill,	"ii")
CFUNC3(link,	simple,	link,	"ss")
CFUNC3(mknod,	simple,	mknod,	"sii")
CFUNC3(nice,	simple,	nice,	"i")
CFUNC3(pause,	simple,	pause,	"")
    /* ### poll */
#endif
CFUNC3(rmdir,	simple,	rmdir,	"s")
#ifndef _WINDOWS
CFUNC3(setuid,	simple,	setuid,	"i")
CFUNC3(setgid,	simple,	setgid,	"i")
CFUNC3(signal,	simple,	signal,	"ii")
CFUNC3(sync,	simple,	sync,	"")
    /* ### times */
#if !defined(NeXT) && !defined(BSD4_4)
CFUNC3(ulimit,	simple,	ulimit,	"ii")
#endif
CFUNC3(umask,	simple,	umask,	"i")
    /* ### uname */
#endif
    /* ### unlink() should go as remove() is more portable */
CFUNC3(unlink,	simple,	unlink,	"s")
    /* ### utime */
#ifndef _WINDOWS
    /*
     * Non system calls, but they act so similar.
     */
CFUNC3(clock,	simple,	clock,	"")
#if !defined(linux) && !defined(BSD4_4)
CFUNC3(lockf,	simple,	lockf,	"iii")
#endif /* linux */
CFUNC3(sleep,	simple,	sleep,	"i")
CFUNC3(usleep,	simple,	usleep,	"i")
#endif /* _WINDOWS */
CFUNC3(rename,	simple,	rename,	"ss")

CFUNC3(isatty, simple, isatty, "i")



#if defined __MACH__ && defined __APPLE__
#include "strings.c"
#include "cfuncs.c"
#endif
