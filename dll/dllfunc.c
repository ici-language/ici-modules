#include "ici.h"
#include "load.h"
#include "dllmanager.h"
#include "dll.h"

typedef enum
{
    CT_CDECL,           /* Push RH arg first, caller pops. */
    CT_STDCALL,         /* Push RH arg first, called function pops. */
    CT_FASTCALL         /* First 2 args in regs, then as per cdecl. */
}
    calltype_t;

#ifdef _WIN32
#define alloca _alloca
#endif

/*
 * Call the function at addr, which has the function style given by ct,
 * with the nargs 32 bit words of arguments stored in args and return
 * the 32 bit word the function returned.
 */
static long
call_long(calltype_t ct, void *addr, int nargs, long *args)
{
    int     i;
    long    arg;

#ifdef _WIN32

    for (i = nargs; --i >= 0; )
    {
        arg = args[i];
        __asm push arg
    }
    __asm call addr
    __asm mov arg,eax
    if (ct == CT_CDECL)
    {
        /*
         * ###Quick hack, I haven't got a book and I don't known how to
         * pop the stack in one go.
         */
        for (i = 0; i < nargs; ++i)
            __asm pop eax
    }

#elif defined __FreeBSD__ && defined __i386__

    /*
     * Assume the one calling convention: CT_CDECL
     */
    for (i = nargs; --i >= 0; )
	asm("push %0" :: "m" (args[i]));
    asm("call *%0" :: "m" (addr));
    asm("mov %%eax,%0" :: "m" (arg));
    asm("addl %0,%%esp" :: "m" (nargs*4));

#else
#error Cannot build dll module for this architecture/compiler
#endif

    return arg;
}


/*
 * Is an object an array only containing string objects?
 */
static int
is_array_of_strings(object_t *o)
{
    object_t	**po;

    if (!isarray(o))
	return 0;
    for (po = arrayof(o)->a_base; po < arrayof(o)->a_top; ++po)
	if (!isstring(*po))
	    return 0;
    return 1;
}

/*
 * Turn an ICI array of strings into an argv-style array. The memory for
 * the argv-style array is allocated (via zalloc()) and must be freed
 * after use. The memory is allocated as one block so a single zfree()
 * of the function result is all that's required.
 */
static char **
ici_array_to_argv(array_t *a)
{
    char	**argv;
    int		argc;
    size_t	sz;
    int		n;
    char	*p;
    object_t	**po;

    argc = a->a_top - a->a_base;
    sz = (argc + 1) * sizeof (char *);
    for (n = argc, po = a->a_base; n; --n, ++po)
    {
	if (!isstring(*po))
	{
	    error = "non-string in argv-style array";
	    return NULL;
	}
	sz += stringof(*po)->s_nchars + 1;
    }
    if ((argv = ici_alloc(sz)) == NULL)
	return NULL;
    p = (char *)&argv[argc];
    for (n = argc, po = a->a_base; n > 0; --n, ++po)
    {
	argv[argc - n] = p;
	memcpy(p, stringof(*po)->s_chars, stringof(*po)->s_nchars + 1);
	p += stringof(*po)->s_nchars + 1;
    }
    argv[argc] = NULL;
    return argv;
}

/*
 * Implement a function call into some dynamically chosen (by name) function
 * in a dynamically loaded DLL.
 *
 * This function is a bit Windows specific. I've used alloca(), as this is
 * known to be available on Windows.
 */
static int
f_dllfunc()
{
    cfunc_t             *cf;
    object_t            *ret_type;
    int                 i;
    long                *args;
    long                *argp;
    int                 nargs; /* In 32 bit words. */
    long                ret;
    calltype_t          ct;
    char                *s;
    object_t            *o;

    ret_type = NULL;
    if (NARGS() > 0)
    {
        ret_type = ARG(0);
        nargs = NARGS() - 1;
    }
    else
    {
        nargs = 0;
    }
    cf = cfuncof(o_top[-1]);
    if (cf->cf_arg1 == NULL)
    {
        /*
         * We don't yet have the address of this function. Try and find
         * it by name in the dll. MS C compilers decorate names in different
         * ways depending on the calling convention and stuff. There is also
         * the issue of ASCII vs UNICODE versions. Try all the possibolities
         * until we find one that exists.
         */
        s = ici_alloc(strlen(cf->cf_name) + 15);
        if (s == NULL)
            return 1;
        sprintf(s, "%s", cf->cf_name);
        if ((cf->cf_arg1 = (int (*)())dlsym((dll_t)cf->cf_arg2, s)) != NULL)
            goto got_addr;
        sprintf(s, "%sA", cf->cf_name);
        if ((cf->cf_arg1 = (int (*)())dlsym((dll_t)cf->cf_arg2, s)) != NULL)
            goto got_addr;
        sprintf(s, "_%s", cf->cf_name);
        if ((cf->cf_arg1 = (int (*)())dlsym((dll_t)cf->cf_arg2, s)) != NULL)
            goto got_addr;
        sprintf(s, "_%s@%d", cf->cf_name, nargs * 4);
        if ((cf->cf_arg1 = (int (*)())dlsym((dll_t)cf->cf_arg2, s)) != NULL)
            goto got_addr;
        sprintf(s, "@%s@%d", cf->cf_name, nargs * 4);
        if ((cf->cf_arg1 = (int (*)())dlsym((dll_t)cf->cf_arg2, s)) != NULL)
            goto got_addr;
        if (chkbuf(3 * strlen(s) + 40))
        {
            ici_free(s);
            return 1;
        }
        sprintf(buf, "could not locate symbol \"%s\"", cf->cf_name);
        error = buf;
        ici_free(s);
        return 1;

    got_addr:
        ici_free(cf->cf_name);
        cf->cf_name = s;
    }
#ifdef _WIN32
    /*
     * We have the address of the function. What sort of calling convention
     * does it use? Guess (err hem, work it out) from the name.
     */
    if (cf->cf_name[0] == '@')
        ct = CT_FASTCALL;
    else if (cf->cf_name[0] == '_')
        ct = CT_CDECL;
    else
        ct = CT_STDCALL;
#elif __FreeBSD__
    ct = CT_CDECL;
#endif
    /*
     * Allocate and build an array of arguments as 32 bit words. Left-most
     * argument is first in array.
     */
    args = (long *)alloca(nargs * sizeof(long));
    argp = &args[0];
    for (i = 1; i < NARGS(); ++i, ++argp)
    {
        o = ARG(i);
        if (isint(o))
        {
            *argp = intof(o)->i_value;
        }
        else if (isstring(o))
        {
            *argp = (long)stringof(o)->s_chars;
        }
        else if (isnull(o))
        {
            *argp = 0;
        }
	else if (is_array_of_strings(o))
	{
	    *argp = (long)ici_array_to_argv(arrayof(o));
	}
        else if (isptr(o))
        {
            object_t    *po;

            po = fetch(ptrof(o)->p_aggr, ptrof(o)->p_key);
            if (isint(po))
            {
                long    *p;

                /*
                 * A pointer to an int. Allocate space for our real pointer
                 * to point to.
                 */
                p = (long *)alloca(sizeof(long));
                *p = intof(po)->i_value;
                *argp = (long)p;
            }
	    else if (is_array_of_strings(po))
	    {
		char ***pargv;
		pargv = alloca(sizeof (char **));
		*pargv = ici_array_to_argv(arrayof(po));
		*argp = (long)pargv;
	    }
            else
            {
                return ici_argerror(i);
            }
        }
        else if (ismem(o))
        {
            *argp = (long)memof(o)->m_base;
        }
    }
    ret = call_long(ct, cf->cf_arg1, nargs, args);
    /*
     * Re-scan our arguments for any pointers (to int) and update the
     * value that they point to from the updated value from the
     * function (if it has changed).
     */
    argp = &args[0];
    for (i = 1; i < NARGS(); ++i, ++argp)
    {
        o = ARG(i);
        if (isptr(o))
        {
            object_t    *po;

            po = fetch(ptrof(o)->p_aggr, ptrof(o)->p_key);
            if (isint(po) && intof(po)->i_value != *(long *)*argp)
            {
                if ((po = objof(new_int(*(long *)*argp))) == NULL)
                    return 1;
                if (assign(ptrof(o)->p_aggr, ptrof(o)->p_key, po))
                    return 1;
            }
        }
	else if (is_array_of_strings(o))
	{
	    ici_free((char *)*argp);
	}
    }
    return int_ret(ret);
}

/*
 * Make a new ICI func object. This is the O_CFUNC varient of an ICI
 * function - the type that just transfers directly through a given
 * function pointer. In this case, that function is f_dllfunc() (above).
 * We record the dynamic link library the targeted function needs to
 * be found in in f_arg2 and the name in f_name. The address of the
 * actual function is not discovered until first call. That is in case
 * we need to know anything about the argument list to find the function
 * name (as is the case in Windows stdcall calling convention). When it
 * is found, it is placed in cf_arg1.
 *
 * This is a bit of a hack because the ICI kernel only allows calls
 * on its internal function object. That is why we are using its internal
 * function object. But as a result we can't make our own type that
 * would have first class object references to the dll object and function
 * name as an ICI string. For this reason dll objects can't really be
 * collected and remain permanently loaded.
 */
cfunc_t *
new_dllfunc(dll_t *lib, string_t *s)
{
    cfunc_t              *cf;

    if ((cf = talloc(cfunc_t)) == NULL)
        return NULL;
    memset((char *)cf, 0, sizeof(cfunc_t));
    objof(cf)->o_flags = 0;
    objof(cf)->o_type = &ici_cfunc_type;
    objof(cf)->o_nrefs = 1;
    rego(cf);
    if ((cf->cf_name = ici_alloc(strlen(s->s_chars) + 1)) == NULL)
    {
        decref(cf);
        return NULL;
    }
    strcpy(cf->cf_name, s->s_chars);
    cf->cf_cfunc = f_dllfunc;
    cf->cf_arg2 = (char *)lib; /* Is this cast possible on all systems? */
    return cf;
}
