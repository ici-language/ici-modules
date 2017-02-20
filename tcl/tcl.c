/*
 * tcl.c - interface between ICI and Tk/Tcl.
 *
 * This is a two-sided language extension. On one hand it is a set of
 * extension functions for ICI that allow it to call the Tcl interpreter.
 * And on the other hand it is an extension to Tcl that provides an "ici"
 * command for executing ICI code from Tcl.
 *
 * Andy Newman <andy@research.canon.com.au>
 */

#include <object.h>
#include <func.h>
#include <buf.h>
#include <op.h>
#include <exec.h>
#include <str.h>

#include <tcl.h>
#ifdef ICI_TK
#ifdef ICI_TKSTEP
#include <tkstep.h>
#else
#include <tk.h>
#endif
#endif

/*
 * An "ici" command for Tcl. Lets you execute ICI code from Tcl.
 */
static int
ici_cmd(ClientData data, Tcl_Interp *interp, int argc, char *argv[])
{
    char	*err;

    if (argc < 2)
    {
	interp->result = "wrong # args, should be: ici string";
	return TCL_ERROR;
    }
    if ((err = ici_call("parse", "s", argv[1])) != NULL)
	interp->result = err;
    return err == NULL ? TCL_OK : TCL_ERROR;
}

/*
 * Initialise the ICI extension to Tcl.
 */
static int
ICI_Init(Tcl_Interp *interp)
{
    Tcl_CreateCommand(interp, "ici", ici_cmd, NULL, NULL);
    return TCL_OK;
}

/*
 * Support code for calling Tcl from ICI. We add a "tcl interpreter"
 * type, tcl_interp, and a number of new functions for evaluating Tcl
 * expressions and accessing Tcl variables.
 */

static string_t	*string_result		= NULL;
static string_t	*string_errorLine	= NULL;
static string_t	*string_global		= NULL;
static int	need_strings		= 1;

static int
get_strings(void)
{
    if ((string_result = get_cname("result")) == NULL)
	return 1;
    if ((string_errorLine = get_cname("errorLine")) == NULL)
    {
	decref(string_result);
	return 1;
    }
    if ((string_global = get_cname("global")) == NULL)
    {
	decref(string_errorLine);
	decref(string_result);
	return 1;
    }
    return need_strings = 0;
}

/*
 * A type for Tcl interpreter objects.
 */
typedef struct
{
    object_t	o_head;		/* Standard ICI object header */
    Tcl_Interp	*tcl_interp;	/* Handle to Tcl interpreter instance */
}
tcl_interp_t;

/*
 * Usual cast and type test macros.
 */
#define	tclinterpof(o)	((tcl_interp_t *)(o))
#define	istclinterp(o)	((o)->o_type == &tcl_interp_type)

/*
 * ICI type code.
 */

/*
 * Mark tcl_interp object during garbage collector's sweep
 * and return size of object being marked.
 */
static unsigned long
mark_tcl_interp(object_t *o)
{
    o->o_flags |= O_MARK;
    return sizeof (tcl_interp_t);
}

/*
 * Free a tcl_interp object when it is no longer referenced.
 * We can actually do something here, throw away the Tcl_Interp
 * object.
 */
static void
free_tcl_interp(object_t *o)
{
    if (tclinterpof(o)->tcl_interp != NULL)
	Tcl_DeleteInterp(tclinterpof(o)->tcl_interp);
    ici_free(o);
}

/*
 * Return an object resulting from an index operation on a tcl_interp.
 * We only understand two keys, the string "result", to return the
 * Tcl interpreter's result string and the string "errorLine" to return
 * the line of last error.
 */
static object_t *
fetch_tcl_interp(object_t *o, object_t *k)
{
    if (need_strings && get_strings())
	return NULL;
    if (k == objof(string_result))
    {
	/*
	 * The result pointer shouldn't be NULL according to the Tcl
	 * book but I don't trust them. Do something sensible just in case.
	 */
	if (tclinterpof(o)->tcl_interp->result == NULL)
	    return objof(&o_null);
	return objof(new_cname(tclinterpof(o)->tcl_interp->result));
    }
    if (k == objof(string_errorLine))
    {
	return objof(new_int(tclinterpof(o)->tcl_interp->errorLine));
    }
    error = "incorrect key for a tcl_interp";
    return NULL;
}

/*
 * ICI type for tcl_interp objects...
 */
static type_t	tcl_interp_type =
{
    mark_tcl_interp,
    free_tcl_interp,
    hash_unique,
    cmp_unique,
    copy_simple,
    assign_simple,
    fetch_tcl_interp,
    "tcl_interp"
};

/*
 * Create a new Tcl interpreter structure and return its handle.
 *
 * This is called from a couple of places, when creating a new
 * tcp_interp object for ICI and when creating the default interpreter
 * object. This is the only place where the various Tcl extension's
 * "Init" routines get called so all interpreter's are alike (i.e.,
 * have Tk and ICI extensions).
 */
static Tcl_Interp *
make_tcl_interp(void)
{
    Tcl_Interp	*interp;

    if ((interp = Tcl_CreateInterp()) == NULL)
        return NULL;
    if (Tcl_Init(interp) == TCL_ERROR)
    {
	strcpy(buf, interp->result);
	error = buf;
	Tcl_DeleteInterp(interp);
	return NULL;
    }
#ifdef ICI_TK
    if (Tk_Init(interp) == TCL_ERROR)
    {
	strcpy(buf, interp->result);
	error = buf;
	Tcl_DeleteInterp(interp);
	return NULL;
    }
#endif
    if (ICI_Init(interp) == TCL_ERROR)
   {
	strcpy(buf, interp->result);
	error = buf;
	Tcl_DeleteInterp(interp);
	return NULL;
    }
    return interp;
}

/*
 * Create a new tcl_interp object.
 */
static tcl_interp_t *
new_tcl_interp(void)
{
    tcl_interp_t	*tcl_interp = talloc(tcl_interp_t);

    if (tcl_interp != NULL)
    {
	objof(tcl_interp)->o_tcode = TC_OTHER;
	objof(tcl_interp)->o_nrefs = 1;
	objof(tcl_interp)->o_flags = 0;
	objof(tcl_interp)->o_type  = &tcl_interp_type;
	if ((tcl_interp->tcl_interp = make_tcl_interp()) == NULL)
	    rego(tcl_interp);
	else
	{
	    ici_free(tcl_interp);
	    tcl_interp = NULL;
	    error = "can't create Tcl interpreter";
	}
    }
    return tcl_interp;
}

/* 
 * Return the default Tcl interpreter object and create it if need be.
 */
static Tcl_Interp *
default_interp(void)
{
    static Tcl_Interp	*interp = NULL;

    if (interp == NULL)
	interp = make_tcl_interp();
    return interp;
}

/*
 * tcl_interp = Interpreter()
 *
 * Create and return a new Tcl interpreter object for use in subsequent
 * calls to Eval, GlobalEval or EvalFile.
 */
static int
f_tcl_interpreter(void)
{
    return ici_ret_with_decref(objof(new_tcl_interp()));
}

/*
 * string = Eval([interpreter,] string)
 * string = GlobalEval([interpreter,] string)
 * string = EvalFile([interpreter,] string)
 *
 * Evaluate Tcl code and return the result. The nature of evaluation depends
 * on the actual function being called and controls how the string parameter
 * is used. Eval and GlobalEval evaluate code directly and the string contains
 * Tcl code. EvalFile evalulates the content of a file and the string is the
 * name of the file containing the Tcl program to be evaluated.
 */
static int
f_tcl_eval(void)
{
    tcl_interp_t	*tcl_interp;
    Tcl_Interp		*interp;
    char		*str;

    switch (NARGS())
    {
    case 2:
	if (ici_typecheck("os", &tcl_interp, &str))
	    return 1;
	if (!istclinterp(objof(tcl_interp)))
	    return ici_argerror(0);
	interp = tcl_interp->tcl_interp;
	break;

    case 1:
	if (ici_typecheck("s", &str))
	    return 1;
	if ((interp = default_interp()) == NULL)
	    return 1;
	break;

    default:
	return ici_argcount(2);
    }
    if ((*CF_ARG1())(interp, str) != TCL_OK)
    {
	if (chkbuf(strlen(interp->result) + 1))
	    error = "Tcl error";
	else
	{
	    strcpy(buf, interp->result);
	    error = buf;
	}
	return 1;
    }
    return str_ret(interp->result);
}

/*
 * string = GetVar([interp,] name [, "global" ])
 */
static int
f_tcl_get_var(void)
{
    Tcl_Interp		*interp;
    tcl_interp_t	*tcl_interp;
    int			flags = 0;
    char		*name;
    string_t		*str;
    char		*s;

    if (need_strings && get_strings())
	return 1;
    switch (NARGS())
    {
    case 1:
	if (!isstring(ARG(0)))
	    return ici_argerror(0);
	name = stringof(ARG(0))->s_chars;
	if ((interp = default_interp()) == NULL)
	    return 1;
	flags = 0;
	break;

    case 2:
	if (istclinterp(ARG(0)))
	{
	    if (ici_typecheck("os", &tcl_interp, &name))
		return 1;
	    interp = tcl_interp->tcl_interp;
	}
	else
	{
	    if (ici_typecheck("so", &name, &str))
		return 1;
	    if (!isstring(objof(str)) || str != string_global)
		return ici_argerror(1);
	    flags = TCL_GLOBAL_ONLY;
	    if ((interp = default_interp()) == NULL)
		return 1;
	}
	break;

    case 3:
	if (ici_typecheck("oso", &tcl_interp, &name, &str))
	    return 1;
	if (!istclinterp(objof(tcl_interp)))
	    return ici_argerror(0);
	if (!isstring(objof(str)) || str != string_global)
	    return ici_argerror(2);
	interp = tcl_interp->tcl_interp;
	flags = TCL_GLOBAL_ONLY;
	break;

    default:
	return ici_argcount(3);
    }
    if ((s = Tcl_GetVar(interp, name, flags | TCL_LEAVE_ERR_MSG)) == NULL)
    {
	error = interp->result;
	return 1;
    }
    return str_ret(s);
}

/*
 * string = GetVar2([interp,] name, name [, "global" ])
 */
static int
f_tcl_get_var2(void)
{
    Tcl_Interp		*interp;
    tcl_interp_t	*tcl_interp;
    int			flags = 0;
    char		*name;
    char		*name2;
    string_t		*str;
    char		*s;

    if (need_strings && get_strings())
	return 1;
    switch (NARGS())
    {
    case 2:
	if (ici_typecheck("ss", &name, &name2))
	    return 1;
	if ((interp = default_interp()) == NULL)
	    return 1;
	flags = 0;
	break;

    case 3:
	if (istclinterp(ARG(0)))
	{
	    if (ici_typecheck("oss", &tcl_interp, &name, &name2))
		return 1;
	    interp = tcl_interp->tcl_interp;
	}
	else
	{
	    if (ici_typecheck("sso", &name, &name2, &str))
		return 1;
	    if (!isstring(objof(str)) || str != string_global)
		return ici_argerror(1);
	    flags = TCL_GLOBAL_ONLY;
	    if ((interp = default_interp()) == NULL)
		return 1;
	}
	break;

    case 4:
	if (ici_typecheck("osso", &tcl_interp, &name, &name2, &str))
	    return 1;
	if (!istclinterp(objof(tcl_interp)))
	    return ici_argerror(0);
	if (!isstring(objof(str)) || str != string_global)
	    return ici_argerror(2);
	interp = tcl_interp->tcl_interp;
	flags = TCL_GLOBAL_ONLY;
	break;

    default:
	return ici_argcount(3);
    }
    if ((s = Tcl_GetVar2(interp, name, name2, flags | TCL_LEAVE_ERR_MSG)) == NULL)
    {
	error = interp->result;
	return 1;
    }
    return str_ret(s);
}

/*
 * SetVar([interpreter], name, value)
 * SetVar([interpreter], name, name2, value)
 *
 * Set the Tcl variable name or the Tcl array variable name(name2) to
 * the given value. All objects are strings.
 */
static int
f_tcl_set_var(void)
{
    char		*name;
    char		*name2 = NULL;
    char		*value;
    tcl_interp_t	*tcl_interp;
    Tcl_Interp		*interp;

    if ((interp = default_interp()) == NULL)
	return 1;
    switch (NARGS())
    {
    case 2:
	if (ici_typecheck("ss", &name, &value))
	    return 1;
	break;

    case 3:
	if (istclinterp(ARG(0)))
	{
	    if (ici_typecheck("oss", &tcl_interp, &name, &value))
		return 1;
	    interp = tcl_interp->tcl_interp;
	}
	else
	{
	    if (ici_typecheck("sss", &name, &name2, &value))
		return 1;
	}
	break;

    case 4:
	if (ici_typecheck("osss", &tcl_interp, &name, &name2, &value))
	    return 1;
	if (!istclinterp(objof(tcl_interp)))
	    return ici_argerror(0);
	interp = tcl_interp->tcl_interp;
	break;

    default:
	return ici_argerror(3);
    }
    if (name2 == NULL)
    {
	if (Tcl_SetVar(interp, name, value, TCL_LEAVE_ERR_MSG) == NULL)
	{
	    error = interp->result;
	    return 1;
	}
    }
    else
    {
	if (Tcl_SetVar2(interp, name, name2, value, TCL_LEAVE_ERR_MSG) == NULL)
	{
	    error = interp->result;
	    return 1;
	}
    }
    return null_ret();
}

#ifdef ICI_TK
/*
 * Tk_MainLoop()
 *
 * Enter Tk's event processing loop.
 */
static int
f_tk_main_loop(void)
{
    Tk_MainLoop();
    return null_ret();
}
#endif

/*
 * Table of functions we export. Used either during link by referencing
 * tcl_cfuncs from conf.c or during dynamic loading via icitcl_library_init()
 * (see below).
 */
static cfunc_t
tcl_cfuncs[] =
{
    {CF_OBJ,	"Interpreter",	f_tcl_interpreter},
    {CF_OBJ,	"Eval",		f_tcl_eval,		Tcl_Eval},
    {CF_OBJ,	"GlobalEval",	f_tcl_eval,		Tcl_GlobalEval},
    {CF_OBJ,	"EvalFile",	f_tcl_eval,		Tcl_EvalFile},
    {CF_OBJ,	"GetVar",	f_tcl_get_var},
    {CF_OBJ,	"GetVar2",	f_tcl_get_var2},
    {CF_OBJ,	"SetVar",	f_tcl_set_var},
#ifdef ICI_TK
    {CF_OBJ,	"Tk_MainLoop",	f_tk_main_loop},
#endif
    {CF_OBJ}
};

#ifndef ICI_TK_STATICALLY_LINKED
/*
 * This is called by the ICI interpreter when this module is loaded.
 * We call dload_library_init() to create a struct of the functions
 * in our cfuncs table (above).
 */
object_t *
ici_tcl_library_init(void)
{
    struct_t	*s;
    if ((s = new_struct()) != NULL)
    {
	if (ici_assign_cfuncs(s, tcl_cfuncs))
	{
	    decref(s);
	    s = NULL;
	}
    }
    return objof(s);
}
#endif
