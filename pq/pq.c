/* 
 * $Id: pq.c,v 1.13 2003/01/27 13:06:57 atrn Exp $
 */

/*
 * ICI PostgreSQL inferface module.
 *
 * --intro-- --ici-pq--
 */

#include <libpq-fe.h>
#include <ici.h>
#include "icistr.h"
#include <icistr-setup.h>

static ici_handle_t *
new_conn(PGconn *conn)
{
    return ici_handle_new(conn, ICIS(PGconn), NULL);
}

static void
ici_pq_result_pre_free(ici_handle_t *h)
{
    PQclear((PGresult *)h->h_ptr);
}

static ici_handle_t *
new_result(PGresult *res)
{
    ici_handle_t *h;

    if ((h = ici_handle_new(res, ICIS(PGresult), NULL)) != NULL)
        h->h_pre_free = ici_pq_result_pre_free;
    return h;
}

static int
glue_i_c(void)
{
    ici_handle_t *conn;
    int (*pf)();

    if (ici_typecheck("h", ICIS(PGconn), &conn))
	return 1;
    pf = CF_ARG1();
    return ici_int_ret((*pf)((PGconn *)conn->h_ptr));
}

static int
glue_n_c(void)
{
    ici_handle_t *conn;
    void (*pf)();

    if (ici_typecheck("h", ICIS(PGconn), &conn))
	return 1;
    pf = CF_ARG1();
    (*pf)((PGconn *)conn->h_ptr);
    return ici_null_ret();
}

static int
glue_s_c(void)
{
    ici_handle_t *conn;
    char *(*pf)();

    if (ici_typecheck("h", ICIS(PGconn), &conn))
	return 1;
    pf = CF_ARG1();
    return ici_str_ret((*pf)((PGconn *)conn->h_ptr));
}

static struct_t *
conninfoOption_to_struct(PQconninfoOption *opt)
{
    struct_t		*d;
    string_t		*s;
    int_t		*i;

    if ((d = ici_struct_new()) == NULL)
	return NULL;

#define	ASSIGN(KEY)\
    if (opt->KEY == NULL)\
	s = stringof(&o_null);\
    else if ((s = ici_str_new_nul_term(opt->KEY)) == NULL)\
	goto fail;\
    if (ici_assign(d, ICIS(KEY), s))\
    {\
	ici_decref(s);\
	goto fail;\
    }\
    ici_decref(s)

    ASSIGN(keyword);
    ASSIGN(envvar);
    ASSIGN(compiled);
    ASSIGN(val);
    ASSIGN(label);
    ASSIGN(dispchar);

#undef ASSIGN

    if ((i = ici_int_new(opt->dispsize)) == NULL)
	goto fail;
    if (ici_assign(d, ICIS(dispsize), i))
    {
	ici_decref(i);
	goto fail;
    }
    ici_decref(i);

    return d;

fail:
    ici_decref(d);
    return NULL;
}

static int
ici_pq_conndefaults(void)
{
    array_t		*a;
    struct_t		*d;
    PQconninfoOption	*opt;
    PQconninfoOption	*p;

    if ((a = ici_array_new(7)) == NULL) /* conninfoOption has seven fields */
	return 1;
    opt = PQconndefaults();
    for (p = opt; p->keyword != NULL; ++p)
    {
	if (ici_stk_push_chk(a, 1))
	    goto fail;
	if ((d = conninfoOption_to_struct(p)) == NULL)
	    goto fail;
	*a->a_top++ = objof(d);
	ici_decref(d);
    }
    PQconninfoFree(opt);
    return ici_ret_with_decref(objof(a));

fail:
    PQconninfoFree(opt);
    ici_decref(a);
    return 1;
}

static int
ici_pq_connectdb(void)
{
    char	*conninfo;
    PGconn	*conn;

    if (ici_typecheck("s", &conninfo))
	return 1;
    if ((conn = PQconnectdb(conninfo)) == NULL)
    {
	ici_error = "unable to create PostgreSQL connection object";
	return 1;
    }
    if (PQstatus(conn) != CONNECTION_OK)
    {
	char *err = PQerrorMessage(conn);
	if (ici_chkbuf(strlen(err) + 1))
	    ici_error = "failed to connect to database";
	else
	{
	    strcpy(ici_buf, err);
	    ici_error = ici_buf;
	}
	PQfinish(conn);
	return 1;
    }
    return ici_ret_with_decref(objof(new_conn(conn)));
}

static int
ici_pq_setdbLogin(void)
{
    char	*host;
    char	*port;
    char	*options;
    char	*tty;
    char	*dbname;
    char	*login;
    char	*pwd;
    PGconn	*conn;

    if (ici_typecheck("sssssss", &host, &port, &options, &tty, &dbname, &login, &pwd))
	return 1;
    conn = PQsetdbLogin(host, port, options, tty, dbname, login, pwd);
    return ici_ret_with_decref(objof(new_conn(conn)));
}

static int
ici_pq_connectStart(void)
{
    char	*conninfo;

    if (ici_typecheck("s", &conninfo))
	return 1;
    return ici_ret_with_decref(objof(new_conn(PQconnectStart(conninfo))));
}

static int
glue_i_r(void)
{
    ici_handle_t *res;
    int (*pf)();

    if (ici_typecheck("h", ICIS(PGresult), &res))
	return 1;
    pf = CF_ARG1();
    return ici_int_ret((*pf)((PGresult *)res->h_ptr));
}

static int
ici_pq_exec(void)
{
    ici_handle_t	*conn;
    char		*query;
    PGresult		*res;
    ici_handle_t	*r;

    if (ici_typecheck("hs", ICIS(PGconn), &conn, &query))
	return 1;
    res = PQexec((PGconn *)conn->h_ptr, query);
    switch (PQresultStatus(res))
    {
    case PGRES_EMPTY_QUERY:
    case PGRES_COMMAND_OK:
    case PGRES_TUPLES_OK:
	break;

    default:
    {
	char *err = PQresStatus(PQresultStatus(res));
	if (ici_chkbuf(strlen(err) + 1))
	    ici_error = "database request failed";
	else
	{
	    strcpy(ici_buf, err);
	    ici_error = ici_buf;
	}
	PQclear(res);
	return 1;
    }

    }
    if ((r = new_result(res)) == NULL)
	return 1;
    return ici_ret_with_decref(objof(r));
}

static int
ici_pq_resStatus(void)
{
    long	code;

    if (ici_typecheck("i", &code))
	return 1;
    return ici_str_ret(PQresStatus(code));
}

static int
glue_s_r(void)
{
    ici_handle_t	*res;
    char *(*pf)();

    if (ici_typecheck("h", ICIS(PGresult), &res))
	return 1;
    pf = CF_ARG1();
    return ici_str_ret((*pf)((PGresult *)res->h_ptr));
}

static int
ici_pq_fname(void)
{
    ici_handle_t *res;
    long	idx;

    if (ici_typecheck("hi", ICIS(PGresult), &res, &idx))
	return 1;
    return ici_str_ret(PQfname((PGresult *)res->h_ptr, idx));
}

static int
ici_pq_fnumber(void)
{
    ici_handle_t *res;
    char	*fname;

    if (ici_typecheck("hs", ICIS(PGresult), &res, &fname))
	return 1;
    return ici_int_ret(PQfnumber((PGresult *)res->h_ptr, fname));
}

static int
glue_i_ri(void)
{
    ici_handle_t *res;
    long	arg;
    int         (*pf)();

    if (ici_typecheck("hi", ICIS(PGresult), &res, &arg))
	return 1;
    pf = CF_ARG1();
    return ici_int_ret((*pf)((PGresult *)res->h_ptr, arg));
}

static int
ici_pq_getvalue(void)
{
    ici_handle_t *res;
    long	tup;
    long	fld;

    if (ici_typecheck("hii", ICIS(PGresult), &res, &tup, &fld))
	return 1;
    return ici_str_ret(PQgetvalue((PGresult *)res->h_ptr, tup, fld));
}

static int
ici_pq_getlength(void)
{
    ici_handle_t *res;
    long	tup;
    long	fld;

    if (ici_typecheck("hii", ICIS(PGresult), &res, &tup, &fld))
	return 1;
    return ici_int_ret(PQgetlength((PGresult *)res->h_ptr, tup, fld));
}

static int
ici_pq_getisnull(void)
{
    ici_handle_t *res;
    long	tup;
    long	fld;

    if (ici_typecheck("hii", ICIS(PGresult), &res, &tup, &fld))
	return 1;
    return ici_int_ret(PQgetisnull((PGresult *)res->h_ptr, tup, fld));
}


/*
 * Turn an ICI array of strings into an argv-style array. The memory for
 * the argv-style array is allocated (via ici_alloc()) and must be freed
 * after use. The memory is allocated as one block so a single zfree()
 * of the function result is all that's required.
 */
static char **
array_to_argv(array_t *a, int *pargc)
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
	    ici_error = "non-string in argv array";
	    return NULL;
	}
	sz += stringof(*po)->s_nchars + 1;
    }
    if ((argv = (char **)ici_alloc(sz)) == NULL)
	return NULL;
    p = (char *)&argv[argc];
    for (n = argc, po = a->a_base; n > 0; --n, ++po)
    {
	argv[argc - n] = p;
	memcpy(p, stringof(*po)->s_chars, stringof(*po)->s_nchars + 1);
	p += stringof(*po)->s_nchars + 1;
    }
    argv[argc] = NULL;
    if (pargc != NULL)
	*pargc = argc;
    return argv;
}

static PQprintOpt *
struct_to_printOpt(struct_t *d)
{
    PQprintOpt          *popt;
    object_t		*o;
    char		**names;

    if ((popt = ici_talloc(PQprintOpt)) == NULL)
        return NULL;

    popt->header = 0;
    popt->align = 0;
    popt->standard = 0;
    popt->html3 = 0;
    popt->expanded = 0;
    popt->pager = 0;
    popt->fieldSep = NULL;
    popt->tableOpt = NULL;
    popt->caption = NULL;
    popt->fieldName = NULL;

#define FETCH(FLD)\
    if ((o = ici_fetch(d, ICIS(FLD))) != objof(&o_null))\
    {\
	if (!isint(o))\
	    goto nonint;\
	popt->FLD = intof(o)->i_value;\
    }

    FETCH(header);
    FETCH(align);
    FETCH(standard);
    FETCH(html3);
    FETCH(expanded);
    FETCH(pager);

#undef FETCH
#define FETCH(FLD)\
    if ((o = ici_fetch(d, ICIS(FLD))) != objof(&o_null))\
    {\
	if (!isstring(o))\
	    goto nonstr;\
	popt->FLD = stringof(o)->s_chars;\
    }

    FETCH(fieldSep);
    FETCH(tableOpt);
    FETCH(caption);

#undef FETCH

    names = NULL;
    if ((o = ici_fetch(d, ICIS(fieldName))) != objof(&o_null))
    {
	if (!isarray(o))
	{
	    ici_error = "print options fieldName field not an array";
	    return NULL;
	}
	names = array_to_argv(arrayof(o), NULL);
    }

    popt->fieldName = names;

    return popt;

nonint:
    ici_nfree(popt, sizeof *popt);
    ici_error = "non-int value for int field in print options struct";
    return NULL;

nonstr:
    ici_error = "non-string value for string field in print options struct";
    return NULL;
}

static void
free_printopt(PQprintOpt *popt)
{
    if (popt->fieldName != NULL)
        ici_free(popt->fieldName);
    ici_nfree(popt, sizeof *popt);
}


static int
ici_pq_print(void)
{
    file_t	*file;
    ici_handle_t *res;
    struct_t	*opt;
    PQprintOpt	*popt;
    PQprintOpt	defopt =
    {
	1,	/* header */
	1,	/* align */
	0,
	0,	/* html3 */
	1,	/* expanded */
	0,	/* pager */
	"\t",	/* field separator */
	"",	/* table options */
	"",	/* caption */
	NULL	/* replacement field name array */
    };

    if ((file = ici_need_stdout()) == NULL)
	return 1;
    popt = &defopt;
    if (NARGS() == 1)
    {
	if (ici_typecheck("h", ICIS(PGresult), res))
	    return 1;
    }
    else if (NARGS() == 2)
    {
	if (ici_typecheck("uh", &file, ICIS(PGresult), &res))
	{
	    if (ici_typecheck("hd", ICIS(PGresult), &res, &opt))
		return 1;
	    if ((popt = struct_to_printOpt(opt)) == NULL)
		return 1;
	}
    }
    else
    {
	if (ici_typecheck("uhd", &file, ICIS(PGresult), &res, &opt))
	    return 1;
	if ((popt = struct_to_printOpt(opt)) == NULL)
	    return 1;
    }
    if (file->f_type != &ici_stdio_ftype)
    {
	ici_error = "cannot print to non-stdio file";
        if (popt != &defopt)
            free_printopt(popt);
    }
    PQprint((FILE *)file->f_file, (PGresult *)res->h_ptr, popt);
    if (popt != &defopt)
        free_printopt(popt);
    return ici_null_ret();
}

static int
ici_pq_trace(void)
{
    ici_handle_t	*conn;
    file_t		*file;

    if (ici_typecheck("h", ICIS(PGconn), &conn))
    {
	if (ici_typecheck("hu", ICIS(PGconn), &conn, &file))
	    return 1;
	if (file->f_type != &ici_stdio_ftype)
	{
	    ici_error = "can only trace to stdio-type files";
	    return 1;
	}
    }
    else
    {
	if ((file = ici_need_stdout()) == NULL)
	    return 1;
    }
    PQtrace((PGconn *)conn->h_ptr, (FILE *)file->f_file);
    return ici_null_ret();
}

static cfunc_t ici_pq_cfuncs[] =
{
    {CF_OBJ, "untrace", glue_n_c, PQuntrace},
    {CF_OBJ, "connectPoll", glue_i_c, PQconnectPoll},
    {CF_OBJ, "status", glue_i_c, PQstatus},
    {CF_OBJ, "finish", glue_n_c, PQfinish},
    {CF_OBJ, "reset", glue_n_c, PQreset},
    {CF_OBJ, "resetStart", glue_i_c, PQresetStart},
    {CF_OBJ, "resetPoll", glue_i_c, PQresetPoll},
    {CF_OBJ, "db", glue_s_c, PQdb},
    {CF_OBJ, "user", glue_s_c, PQuser},
    {CF_OBJ, "pass", glue_s_c, PQpass},
    {CF_OBJ, "host", glue_s_c, PQhost},
    {CF_OBJ, "port", glue_s_c, PQport},
    {CF_OBJ, "tty", glue_s_c, PQtty},
    {CF_OBJ, "options", glue_s_c, PQoptions},
    {CF_OBJ, "errorMessage", glue_s_c, PQerrorMessage},
    {CF_OBJ, "socket", glue_i_c, PQsocket},
    {CF_OBJ, "clientEncoding", glue_i_c, PQclientEncoding},
    {CF_OBJ, "backendPID", glue_i_c, PQbackendPID},
    {CF_OBJ, "resultStatus", glue_i_r, PQresultStatus},
    {CF_OBJ, "resultErrorMessage", glue_s_r, PQresultErrorMessage},
    {CF_OBJ, "cmdStatus", glue_s_r, PQcmdStatus},
    {CF_OBJ, "oidStatus", glue_s_r, PQoidStatus},
    {CF_OBJ, "oidValue", glue_i_r, PQoidValue},
    {CF_OBJ, "cmdTuples", glue_s_r, PQcmdTuples},
    {CF_OBJ, "ntuples", glue_i_r, PQntuples},
    {CF_OBJ, "nfields", glue_i_r, PQnfields},
    {CF_OBJ, "binaryTuples", glue_i_r, PQbinaryTuples},
    {CF_OBJ, "ftype", glue_i_ri, PQftype},
    {CF_OBJ, "fsize", glue_i_ri, PQfsize},
    {CF_OBJ, "fmod", glue_i_ri, PQfmod},
    {CF_OBJ, "conndefaults", ici_pq_conndefaults},
    {CF_OBJ, "connectdb", ici_pq_connectdb},
    {CF_OBJ, "setdbLogin", ici_pq_setdbLogin},
    {CF_OBJ, "connectStart", ici_pq_connectStart},
    {CF_OBJ, "exec", ici_pq_exec},
    {CF_OBJ, "resStatus", ici_pq_resStatus},
    {CF_OBJ, "fname", ici_pq_fname},
    {CF_OBJ, "fnumber", ici_pq_fnumber},
    {CF_OBJ, "getvalue", ici_pq_getvalue},
    {CF_OBJ, "getlength", ici_pq_getlength},
    {CF_OBJ, "getisnull", ici_pq_getisnull},
    {CF_OBJ, "print", ici_pq_print},
    {CF_OBJ, "trace", ici_pq_trace},
    {CF_OBJ}
};

object_t *
ici_pq_library_init(void)
{
    if (ici_interface_check(ICI_VER, ICI_BACK_COMPAT_VER, "pq"))
	return NULL;
    if (init_ici_str())
	return NULL;
    return objof(ici_module_new(ici_pq_cfuncs));
}
