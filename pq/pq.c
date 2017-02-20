/* 
 * $Id: pq.c,v 1.10 2001/09/23 05:58:06 atrn Exp $
 *
 * ICI PostgreSQL inferface module.
 */

#include "ici.h"
#include <libpq-fe.h>

typedef struct
{
    object_t	o_head;
    PGconn	*pgconn;
}
connection_t;

static unsigned long
mark_connection(object_t *o)
{
    o->o_flags |= O_MARK;
    return sizeof (connection_t);
}

static type_t	connection_type =
{
    mark_connection,
    free_simple,
    hash_unique,
    cmp_unique,
    copy_simple,
    assign_simple,
    fetch_simple,
    "connection"
};

inline
static connection_t *
connectionof(void *o)
{
    return (connection_t *)o;
}

inline
static int
isconnection(object_t *o)
{
    return o->o_type == &connection_type;
}

static
connection_t *
new_connection(PGconn *con)
{
    connection_t	*c;

    if (con == NULL)
    {
	c = NULL;
	if (error == NULL || *error == '\0')
	    error = "NULL PGconn * passed to new_connection";
    }
    else if ((c = talloc(connection_t)) != NULL)
    {
	objof(c)->o_flags = 0;
	objof(c)->o_nrefs = 1;
	objof(c)->o_tcode = TC_OTHER;
	objof(c)->o_type  = &connection_type;
	c->pgconn = con;
	rego(c);
    }
    return c;
}

typedef struct
{
    object_t	o_head;
    PGresult	*pgresult;
}
result_t;

inline
static result_t *
resultof(void *o)
{
    return (result_t *)o;
}

static unsigned long
mark_result(object_t *o)
{
    o->o_flags |= O_MARK;
    return sizeof (result_t);
}

static void
free_result(object_t *o)
{
    if (resultof(o)->pgresult != NULL)
	PQclear(resultof(o)->pgresult);
    ici_free(o);
}

static type_t	result_type =
{
    mark_result,
    free_result,
    hash_unique,
    cmp_unique,
    copy_simple,
    assign_simple,
    fetch_simple,
    "result"
};

inline
static int
isresult(object_t *o)
{
    return o->o_type == &result_type;
}

static
result_t *
new_result(PGresult *result)
{
    result_t	*c;
    int		rc;

    if (result == NULL)
	c = NULL;
    else if
    (
	(rc = PQresultStatus(result)) == PGRES_BAD_RESPONSE
	||
	rc == PGRES_NONFATAL_ERROR
	||
	rc == PGRES_FATAL_ERROR
    )
    {
	error = PQresultErrorMessage(result);
	c = NULL;
    }
    else if ((c = talloc(result_t)) != NULL)
    {
	objof(c)->o_flags = 0;
	objof(c)->o_nrefs = 1;
	objof(c)->o_tcode = TC_OTHER;
	objof(c)->o_type  = &result_type;
	c->pgresult = result;
	rego(c);
    }
    return c;
}

FUNCDEF(glue_i_c)
{
    connection_t	*conn;

    if (ici_typecheck("o", &conn))
	return 1;
    if (!isconnection(objof(conn)))
	return ici_argerror(0);
    return int_ret((*CF_ARG1())(conn->pgconn));
}

FUNCDEF(glue_n_c)
{
    connection_t	*conn;

    if (ici_typecheck("o", &conn))
	return 1;
    if (!isconnection(objof(conn)))
	return ici_argerror(0);
    (*CF_ARG1())(conn->pgconn);
    return null_ret();
}

FUNC(glue_s_c)
{
    connection_t	*conn;

    if (ici_typecheck("o", &conn))
	return 1;
    if (!isconnection(objof(conn)))
	return ici_argerror(0);
    return str_ret((char *)(*CF_ARG1())(conn->pgconn));
}

NEED_STRING(keyword);
NEED_STRING(envvar);
NEED_STRING(compiled);
NEED_STRING(val);
NEED_STRING(label);
NEED_STRING(dispchar);
NEED_STRING(dispsize);

static struct_t *
conninfoOption_to_struct(PQconninfoOption *opt)
{
    struct_t		*d;
    string_t		*s;
    int_t		*i;

    NEED_STRINGS(NULL);
    if ((d = new_struct()) == NULL)
	return NULL;

#define	ASSIGN(KEY)\
    if (opt->KEY == NULL)\
	s = stringof(&o_null);\
    else if ((s = new_cname(opt->KEY)) == NULL)\
	goto fail;\
    if (assign(d, STRING(KEY), s))\
    {\
	decref(s);\
	goto fail;\
    }\
    decref(s)

    ASSIGN(keyword);
    ASSIGN(envvar);
    ASSIGN(compiled);
    ASSIGN(val);
    ASSIGN(label);
    ASSIGN(dispchar);

#undef ASSIGN

    if ((i = new_int(opt->dispsize)) == NULL)
	goto fail;
    if (assign(d, STRING(dispsize), i))
    {
	decref(i);
	goto fail;
    }
    decref(i);

    return d;

fail:
    decref(d);
    return NULL;
}

FUNC(conndefaults)
{
    array_t		*a;
    struct_t		*d;
    PQconninfoOption	*opt;
    PQconninfoOption	*p;

    if ((a = new_array()) == NULL)
	return 1;
    opt = PQconndefaults();
    for (p = opt; p->keyword != NULL; ++p)
    {
	if (pushcheck(a, 1))
	    goto fail;
	if ((d = conninfoOption_to_struct(p)) == NULL)
	    goto fail;
	*a->a_top++ = objof(d);
	decref(d);
    }
    PQconninfoFree(opt);
    return ici_ret_with_decref(objof(a));

fail:
    PQconninfoFree(opt);
    decref(a);
    return 1;
}

FUNC(connectdb)
{
    char	*conninfo;
    PGconn	*conn;

    if (ici_typecheck("s", &conninfo))
	return 1;
    if ((conn = PQconnectdb(conninfo)) == NULL)
    {
	error = "unable to create PostgreSQL connection object";
	return 1;
    }
    if (PQstatus(conn) != CONNECTION_OK)
    {
	error = PQerrorMessage(conn);
	if (chkbuf(strlen(error)+1))
	    error = "failed to connect to database";
	else
	{
	    strcpy(buf, error);
	    error = buf;
	}
	PQfinish(conn);
	return 1;
    }
    return ici_ret_with_decref(objof(new_connection(conn)));
}

FUNC(setdbLogin)
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
    return ici_ret_with_decref(objof(new_connection(conn)));
}

FUNC(connectStart)
{
    char	*conninfo;

    if (ici_typecheck("s", &conninfo))
	return 1;
    return ici_ret_with_decref(objof(new_connection(PQconnectStart(conninfo))));
}

CFUNC1(connectPoll, glue_i_c, PQconnectPoll);
CFUNC1(status, glue_i_c, PQstatus);
CFUNC1(finish, glue_n_c, PQfinish);
CFUNC1(reset, glue_n_c, PQreset);
CFUNC1(resetStart, glue_i_c, PQresetStart);
CFUNC1(resetPoll, glue_i_c, PQresetPoll);
CFUNC1(db, glue_s_c, PQdb);
CFUNC1(user, glue_s_c, PQuser);
CFUNC1(pass, glue_s_c, PQpass);
CFUNC1(host, glue_s_c, PQhost);
CFUNC1(port, glue_s_c, PQport);
CFUNC1(tty, glue_s_c, PQtty);
CFUNC1(options, glue_s_c, PQoptions);
CFUNC1(errorMessage, glue_s_c, PQerrorMessage);
CFUNC1(socket, glue_i_c, PQsocket);
CFUNC1(clientEncoding, glue_i_c, PQclientEncoding);
CFUNC1(backendPID, glue_i_c, PQbackendPID);

static int
cleared_result()
{
    error = "attempt to use result after clearing";
    return 1;
}

FUNCDEF(glue_i_r)
{
    result_t	*res;

    if (ici_typecheck("o", &res))
	return 1;
    if (!isresult(objof(res)))
	return ici_argerror(0);
    if (res->pgresult == NULL)
	return cleared_result();
    return int_ret((*CF_ARG1())(res->pgresult));
}

FUNC(exec)
{
    connection_t	*conn;
    char		*query;
    PGresult		*res;

    if (ici_typecheck("os", &conn, &query))
	return 1;
    if (!isconnection(objof(conn)))
	return ici_argerror(0);
    res = PQexec(conn->pgconn, query);
    switch (PQresultStatus(res))
    {
    case PGRES_EMPTY_QUERY:
    case PGRES_COMMAND_OK:
    case PGRES_TUPLES_OK:
	break;

    default:
	error = PQresStatus(PQresultStatus(res));
	if (chkbuf(strlen(error)+1))
	    error = "database request failed";
	else
	{
	    strcpy(buf, error);
	    error = buf;
	}
	PQclear(res);
	return 1;
    }
    return ici_ret_with_decref(objof(new_result(res)));
}

CFUNC1(resultStatus, glue_i_r, PQresultStatus);

FUNC(resStatus)
{
    long	code;

    if (ici_typecheck("i", &code))
	return 1;
    return str_ret(PQresStatus(code));
}

FUNCDEF(glue_s_r)
{
    result_t	*res;

    if (ici_typecheck("o", &res))
	return 1;
    if (!isresult(objof(res)))
	return ici_argerror(0);
    if (res->pgresult == NULL)
	return cleared_result();
    return str_ret((char *)(*CF_ARG1())(res->pgresult));
}

CFUNC1(resultErrorMessage, glue_s_r, PQresultErrorMessage);
CFUNC1(cmdStatus, glue_s_r, PQcmdStatus);
CFUNC1(oidStatus, glue_s_r, PQoidStatus);
CFUNC1(oidValue, glue_i_r, PQoidValue);
CFUNC1(cmdTuples, glue_s_r, PQcmdTuples);
CFUNC1(ntuples, glue_i_r, PQntuples);
CFUNC1(nfields, glue_i_r, PQnfields);
CFUNC1(binaryTuples, glue_i_r, PQbinaryTuples);

FUNC(fname)
{
    result_t	*res;
    long	idx;

    if (ici_typecheck("oi", &res, &idx))
	return 1;
    if (!isresult(objof(res)))
	return ici_argerror(0);
    if (res->pgresult == NULL)
	return cleared_result();
    return str_ret(PQfname(res->pgresult, idx));
}

FUNC(fnumber)
{
    result_t	*res;
    char	*fname;

    if (ici_typecheck("os", &res, &fname))
	return 1;
    if (!isresult(objof(res)))
	return ici_argerror(0);
    if (res->pgresult == NULL)
	return cleared_result();
    return int_ret(PQfnumber(res->pgresult, fname));
}

FUNCDEF(glue_i_ri)
{
    result_t	*res;
    long	arg;

    if (ici_typecheck("oi", &res, &arg))
	return 1;
    if (!isresult(objof(res)))
	return ici_argerror(0);
    if (res->pgresult == NULL)
	return cleared_result();
    return int_ret((*CF_ARG1())(res->pgresult, arg));
}

CFUNC1(ftype, glue_i_ri, PQftype);
CFUNC1(fsize, glue_i_ri, PQfsize);
CFUNC1(fmod, glue_i_ri, PQfmod);

FUNC(getvalue)
{
    result_t	*res;
    long	tup;
    long	fld;

    if (ici_typecheck("oii", &res, &tup, &fld))
	return 1;
    if (!isresult(objof(res)))
	return ici_argerror(0);
    if (res->pgresult == NULL)
	return cleared_result();
    return str_ret(PQgetvalue(res->pgresult, tup, fld));
}

FUNC(getlength)
{
    result_t	*res;
    long	tup;
    long	fld;

    if (ici_typecheck("oii", &res, &tup, &fld))
	return 1;
    if (!isresult(objof(res)))
	return ici_argerror(0);
    if (res->pgresult == NULL)
	return cleared_result();
    return int_ret(PQgetlength(res->pgresult, tup, fld));
}

FUNC(getisnull)
{
    result_t	*res;
    long	tup;
    long	fld;

    if (ici_typecheck("oii", &res, &tup, &fld))
	return 1;
    if (!isresult(objof(res)))
	return ici_argerror(0);
    if (res->pgresult == NULL)
	return cleared_result();
    return int_ret(PQgetisnull(res->pgresult, tup, fld));
}

FUNC(clear)
{
    result_t	*res;

    if (ici_typecheck("o", &res))
	return 1;
    if (!isresult(objof(res)))
	return ici_argerror(0);
    if (res->pgresult == NULL)
	return cleared_result();
    PQclear(res->pgresult);
    res->pgresult = NULL;
    return null_ret();
}

NEED_STRING(header);
NEED_STRING(align);
NEED_STRING(standard);
NEED_STRING(html3);
NEED_STRING(expanded);
NEED_STRING(pager);
NEED_STRING(fieldSep);
NEED_STRING(tableOpt);
NEED_STRING(caption);
NEED_STRING(fieldName);

/*
 * Turn an ICI array of strings into an argv-style array. The memory for
 * the argv-style array is allocated (via ici_raw_alloc()) and must be freed
 * after use. The memory is allocated as one block so a single zfree()
 * of the function result is all that's required.
 */
static char **
ici_array_to_argv(array_t *a, int *pargc)
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
	    error = "non-string in argv array";
	    return NULL;
	}
	sz += stringof(*po)->s_nchars + 1;
    }
    if ((argv = (char **)ici_raw_alloc(sz)) == NULL)
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
    static PQprintOpt	popt;
    object_t		*o;
    char		**names;

    NEED_STRINGS(NULL);

    popt.header = 0;
    popt.align = 0;
    popt.standard = 0;
    popt.html3 = 0;
    popt.expanded = 0;
    popt.pager = 0;
    popt.fieldSep = NULL;
    popt.tableOpt = NULL;
    popt.caption = NULL;
    popt.fieldName = NULL;

#define FETCH(FLD)\
    if ((o = fetch(d, STRING(FLD))) != objof(&o_null))\
    {\
	if (!isint(o))\
	    goto nonint;\
	popt. FLD = intof(o)->i_value;\
    }

    FETCH(header);
    FETCH(align);
    FETCH(standard);
    FETCH(html3);
    FETCH(expanded);
    FETCH(pager);

#undef FETCH
#define FETCH(FLD)\
    if ((o = fetch(d, STRING(FLD))) != objof(&o_null))\
    {\
	if (!isstring(o))\
	    goto nonstr;\
	popt. FLD = stringof(o)->s_chars;\
    }

    FETCH(fieldSep);
    FETCH(tableOpt);
    FETCH(caption);

#undef FETCH

    names = NULL;
    if ((o = fetch(d, STRING(fieldName))) != objof(&o_null))
    {
	if (!isarray(o))
	{
	    error = "print options fieldName field not an array";
	    return NULL;
	}
	names = ici_array_to_argv(arrayof(o), NULL);
    }

    popt.fieldName = names;

    return &popt;

nonint:
    error = "non-int value for int field in print options struct";
    return NULL;

nonstr:
    error = "non-string value for string field in print options struct";
    return NULL;
}

FUNC(print)
{
    file_t	*file;
    result_t	*res;
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

    if ((file = need_stdout()) == NULL)
	return 1;
    popt = &defopt;
    if (NARGS() == 1)
    {
	if (ici_typecheck("o", res))
	    return 1;
    }
    else if (NARGS() == 2)
    {
	if (ici_typecheck("uo", &file, &res))
	{
	    if (ici_typecheck("od", &res, &opt))
		return 1;
	    if ((popt = struct_to_printOpt(opt)) == NULL)
		return 1;
	}
    }
    else
    {
	if (ici_typecheck("uod", &file, &res, &opt))
	    return 1;
	if ((popt = struct_to_printOpt(opt)) == NULL)
	    return 1;
    }
    if (file->f_type != &stdio_ftype)
    {
	error = "cannot print to non-stdio file";
	return 1;
    }
    PQprint((FILE *)file->f_file, res->pgresult, popt);
    return null_ret();
}

FUNC(trace)
{
    connection_t	*conn;
    file_t		*file;

    if (ici_typecheck("o", &conn))
    {
	if (ici_typecheck("ou", &conn, &file))
	    return 1;
	if (file->f_type != &stdio_ftype)
	{
	    error = "can only trace to stdio-type files";
	    return 1;
	}
    }
    else
    {
	if ((file = need_stdout()) == NULL)
	    return 1;
    }
    if (!isconnection(objof(conn)))
	return ici_argerror(0);
    PQtrace(conn->pgconn, (FILE *)file->f_file);
    return null_ret();
}

CFUNC1(untrace, glue_n_c, PQuntrace);

#if defined __APPLE__ && defined __MACH__
#include "cfuncs.c"
#include "strings.c"
#endif
