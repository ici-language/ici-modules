/*
 * $Id$
 *
 *
 * This module gives access to interpreter internals by patching various
 * object fetch functions and providing a few new types for constructing
 * code arrays at run-time.
 */

#include "ici.h"
#include <parse.h>
#include "strs.h"
#include <src.h>

NEED_STRING(name);
NEED_STRING(code);
NEED_STRING(ecode);
NEED_STRING(call);
NEED_STRING(namelvalue);
NEED_STRING(dot);
NEED_STRING(dotkeep);
NEED_STRING(dotrkeep);
NEED_STRING(assign);
NEED_STRING(assignlocal);
NEED_STRING(exec);
NEED_STRING(loop);
NEED_STRING(ifnotbreak);
NEED_STRING(break);
NEED_STRING(quote);
NEED_STRING(binop);
NEED_STRING(at);
NEED_STRING(swap);
NEED_STRING(binop_for_temp);
NEED_STRING(aggr_key_call);
NEED_STRING(colon);
NEED_STRING(coloncaret);
NEED_STRING(method_call);
NEED_STRING(super_call);
NEED_STRING(assignlocalvar);
NEED_STRING(offsq);
NEED_STRING(openptr);
NEED_STRING(mklvalue);
NEED_STRING(onerror);
NEED_STRING(looper);
NEED_STRING(continue);
NEED_STRING(mkptr);
NEED_STRING(if);
NEED_STRING(ifelse);
NEED_STRING(pop);
NEED_STRING(andand);
NEED_STRING(switch);
NEED_STRING(switcher);
NEED_STRING(return);
NEED_STRING(fetch);
NEED_STRING(unary);
NEED_STRING(for);
NEED_STRING(forall);


/*
 * The "operator" type provides user level access to ICI's "op" type.
 * and is used for constructing op's at run-time.
 */

typedef struct
{
    object_t	o_head;
    op_t	*o_op;
}
operator_t;

static __inline operator_t *
operatorof(void *p)
{
    return (operator_t *)p;
}

static unsigned long
mark_operator(object_t *o)
{
    o->o_flags |= O_MARK;
    return sizeof (operator_t) + mark(operatorof(o)->o_op);
}

static int
assign_operator(object_t *o, object_t *k, object_t *v)
{
    NEED_STRINGS(1);

    if (k == objof(STRING(code)))
    {
	if (!isint(v))
	{
	    error = "attempt to set operator code to a non-integer value";
	    return 1;
	}
	operatorof(o)->o_op->op_code = intof(v)->i_value;
	return 0;
    }
    return assign_simple(o, k, v);
}

static object_t *
fetch_operator(object_t *o, object_t *k)
{
    NEED_STRINGS(NULL);
    if (k == objof(STRING(code)))
	return objof(new_int(operatorof(o)->o_op->op_code));
    if (k == objof(STRING(ecode)))
	return objof(new_int(operatorof(o)->o_op->op_ecode));
    return fetch_simple(o, k);
}

static type_t
operator_type =
{
    mark_operator,
    free_simple,
    hash_unique,
    cmp_unique,
    copy_simple,
    assign_operator,
    fetch_operator,
    "operator"
};

static __inline int
isoperator(object_t *o)
{
    return o->o_type == &operator_type;
}

static operator_t *
new_operator(op_t *op)
{
    operator_t	*opr;

    if (op == NULL)
	return NULL;
    if ((opr = talloc(operator_t)) != NULL)
    {
	objof(opr)->o_tcode = TC_OTHER;
	objof(opr)->o_nrefs = 1;
	objof(opr)->o_flags = 0;
	objof(opr)->o_type  = &operator_type;
	opr->o_op = op;
	rego(opr);
    }
    return opr;
}

/*
 * A "sourcemarker" type for constructing src_t's
 */

typedef struct
{
    object_t	o_head;
    src_t	*s_src;
}
sourcemarker_t;

static __inline sourcemarker_t *
sourcemarkerof(void *p)
{
    return (sourcemarker_t *)p;
}

static unsigned long
mark_sourcemarker(object_t *o)
{
    o->o_flags |= O_MARK;
    return sizeof (sourcemarker_t) + mark(sourcemarkerof(o)->s_src);
}

NEED_STRING(filename);
NEED_STRING(lineno);

static object_t *
fetch_sourcemarker(object_t *o, object_t *k)
{
    NEED_STRINGS(NULL);
    if (k == objof(STRING(filename)))
	return objof(sourcemarkerof(o)->s_src->s_filename);
    if (k == objof(STRING(lineno)))
    {
	int_t	*rc;

	if ((rc = new_int(sourcemarkerof(o)->s_src->s_lineno)) != NULL)
	    decref(rc);
	return objof(rc);
    }
    return fetch_simple(o, k);
}

static type_t	sourcemarker_type =
{
    mark_sourcemarker,
    free_simple,
    hash_unique,
    cmp_unique,
    copy_simple,
    assign_simple,
    fetch_sourcemarker,
    "sourcemarker"
};

static __inline int
issourcemarker(object_t *o)
{
    return o->o_type == &sourcemarker_type;
}

static sourcemarker_t *
new_sourcemarker(src_t *src)
{
    sourcemarker_t	*s;
    
    if ((s = talloc(sourcemarker_t)) != NULL)
    {
	objof(s)->o_tcode = TC_OTHER;
	objof(s)->o_nrefs = 1;
	objof(s)->o_flags = 0;
	objof(s)->o_type  = &sourcemarker_type;
	s->s_src = src;
	rego(s);
    }
    return s;
}

/*
 * Given an operator, return a string giving its name
 */
static object_t *
opname(op_t *op)
{
    NEED_STRINGS(NULL);
    switch (op->op_ecode)
    {
    case OP_OTHER:
	if (op->op_func == ici_op_offsq)
	    return objof(STRING(offsq));
	if (op->op_func == ici_op_openptr)
	    return objof(STRING(openptr));
	if (op->op_func == ici_op_mklvalue)
	    return objof(STRING(mklvalue));
	if (op->op_func == ici_op_onerror)
	    return objof(STRING(onerror));
	if (op->op_func == ici_op_looper)
	    return objof(STRING(looper));
	if (op->op_func == ici_op_continue)
	    return objof(STRING(continue));
	if (op->op_func == ici_op_mkptr)
	    return objof(STRING(mkptr));
	if (op->op_func == ici_op_if)
	    return objof(STRING(if));
	if (op->op_func == ici_op_ifelse)
	    return objof(STRING(ifelse));
	if (op->op_func == ici_op_pop)
	    return objof(STRING(pop));
	if (op->op_func == ici_op_andand)
	    return objof(STRING(andand));
	if (op->op_func == ici_op_switch)
	    return objof(STRING(switch));
	if (op->op_func == ici_op_switcher)
	    return objof(STRING(switcher));
	if (op->op_func == ici_op_return)
	    return objof(STRING(return));
	if (op->op_func == ici_op_fetch)
	    return objof(STRING(fetch));
	if (op->op_func == ici_op_unary)
	    return objof(STRING(unary));
	if (op->op_func == ici_op_for)
	    return objof(STRING(for));
	if (op->op_func == ici_op_forall)
	    return objof(STRING(forall));
	break;

    case OP_CALL:
	return objof(STRING(call));
    case OP_NAMELVALUE:
	return objof(STRING(namelvalue));
    case OP_DOT:
	return objof(STRING(dot));
    case OP_DOTKEEP:
	return objof(STRING(dotkeep));
    case OP_DOTRKEEP:
	return objof(STRING(dotrkeep));
    case OP_ASSIGN:
	return objof(STRING(assign));
    case OP_ASSIGNLOCAL:
	return objof(STRING(assignlocal));
    case OP_EXEC:
	return objof(STRING(exec ));
    case OP_LOOP:
	return objof(STRING(loop));
    case OP_IFNOTBREAK:
	return objof(STRING(ifnotbreak));
    case OP_BREAK:
	return objof(STRING(break));
    case OP_QUOTE:
	return objof(STRING(quote));
    case OP_BINOP:
	return objof(STRING(binop));
    case OP_AT:
	return objof(STRING(at));
    case OP_SWAP:
	return objof(STRING(swap));
    case OP_BINOP_FOR_TEMP:
	return objof(STRING(binop_for_temp));
    case OP_AGGR_KEY_CALL:
	return objof(STRING(aggr_key_call));
    case OP_COLON:
	return objof(STRING(colon));
    case OP_COLONCARET:
	return objof(STRING(coloncaret));
    case OP_METHOD_CALL:
	return objof(STRING(method_call));
    case OP_SUPER_CALL:
	return objof(STRING(super_call));
    case OP_ASSIGNLOCALVAR:
	return objof(STRING(assignlocalvar));
    }

    error = "unknown operator";
    return NULL;
}

/*
 * Fetch function for op_t patched into type via vm.expose()
 */
static object_t *
fetch_op(object_t *o, object_t *k)
{
    NEED_STRINGS(NULL);
    if (k == objof(STRING(code)))
	return objof(new_int(opof(o)->op_code));
    if (k == objof(STRING(ecode)))
	return objof(new_int(opof(o)->op_ecode));
    if (k == objof(STRING(name)))
	return opname(opof(o));
    return fetch_simple(o, k);
}

/*
 * Fetch function for src_t patched into type via vm.expose()
 */
static object_t *
fetch_src(object_t *o, object_t *k)
{
    NEED_STRINGS(NULL);
    if (k == objof(STRING(filename)))
	return objof(srcof(o)->s_filename);
    if (k == objof(STRING(lineno)))
    {
	int_t	*rc;

	if ((rc = new_int(srcof(o)->s_lineno)) != NULL)
	    decref(rc);
	return objof(rc);
    }
    return fetch_simple(o, k);
}


/*
 * Place to remember previous func_t fetch function, it isn't exported.
 * Set in vm.expose().
 */
static object_t	*(*fetch_func)(object_t *, object_t *) = NULL;

/*
 * Enhanced fetch function for func_t patched via vm.expose()
 */
static object_t *
fetch_func_ex(object_t *o, object_t *k)
{
    NEED_STRINGS(NULL);
    if (k == objof(STRING(code)))
	return objof(copy(funcof(o)->f_code));
    return (*fetch_func)(o, k);
}

/**
 * vm.expose()
 *
 * Patch the interpreter to allow user level access to internals
 * of various objects. Specifically it allows access to the compiled
 * ICI code and its special objects.
 */
FUNC(expose)
{
    if (fetch_func == NULL)
    {
	fetch_func = ici_func_type.t_fetch;
	op_type.t_fetch = fetch_op;
	src_type.t_fetch = fetch_src;
	ici_func_type.t_fetch = fetch_func_ex;
    }
    return null_ret();
}

/**
 * sourcemarker = sourcemarker(src)
 * sourcemarker = sourcemarker(string, int)
 *
 * Create a sourcemarker object from a src object or a filename/line pair.
 */
FUNC(sourcemarker)
{
    src_t		*src;
    string_t		*filename;
    long		lineno;
    int			decit = 0;
    sourcemarker_t	*sm;
    
    NEED_STRINGS(1);
    if (NARGS() == 1)
	src = srcof(ARG(0));
    else if (ici_typecheck("oi", &filename, &lineno))
	return 1;
    else if (!isstring(objof(filename)))
	return ici_argerror(0);
    else if ((src = new_src(lineno, filename)) == NULL)
	return 1;
    else
	decit = 1;
    if (!issrc(objof(src)))
	return ici_argerror(0);
    sm = new_sourcemarker(src);
    if (decit)
	decref(src);
    return ici_ret_with_decref(objof(sm));
}

/**
 * operator = operator(op)
 * operator = operator(string)
 *
 * Construct an operator.
 */
FUNC(operator)
{
    object_t	*o;

    NEED_STRINGS(1);
    if (NARGS() != 1)
	return ici_argcount(1);
    o = ARG(0);
    if (isop(o))
	return ici_ret_with_decref(objof(new_operator(opof(o))));
    if (!isstring(o))
	return ici_argerror(0);
    if (o == objof(STRING(call)))
	return ici_ret_with_decref(objof(new_operator(new_op(NULL, OP_CALL, 0))));
    if (o == objof(STRING(namelvalue)))
	return ici_ret_with_decref(objof(new_operator(new_op(NULL, OP_NAMELVALUE, 0))));
    if (o == objof(STRING(dot)))
	return ici_ret_with_decref(objof(new_operator(new_op(NULL, OP_DOT, 0))));
    if (o == objof(STRING(dotkeep)))
	return ici_ret_with_decref(objof(new_operator(new_op(NULL, OP_DOTKEEP, 0))));
    if (o == objof(STRING(dotrkeep)))
	return ici_ret_with_decref(objof(new_operator(new_op(NULL, OP_DOTRKEEP, 0))));
    if (o == objof(STRING(assign)))
	return ici_ret_with_decref(objof(new_operator(new_op(NULL, OP_ASSIGN, 0))));
    if (o == objof(STRING(assignlocal)))
	return ici_ret_with_decref(objof(new_operator(new_op(NULL, OP_ASSIGNLOCAL, 0))));
    if (o == objof(STRING(exec)))
	return ici_ret_with_decref(objof(new_operator(new_op(NULL, OP_EXEC, 0))));
    if (o == objof(STRING(loop)))
	return ici_ret_with_decref(objof(new_operator(new_op(NULL, OP_LOOP, 0))));
    if (o == objof(STRING(ifnotbreak)))
	return ici_ret_with_decref(objof(new_operator(new_op(NULL, OP_IFNOTBREAK, 0))));
    if (o == objof(STRING(break)))
	return ici_ret_with_decref(objof(new_operator(new_op(NULL, OP_BREAK, 0))));
    if (o == objof(STRING(quote)))
	return ici_ret_with_decref(objof(new_operator(new_op(NULL, OP_QUOTE, 0))));
    if (o == objof(STRING(binop)))
	return ici_ret_with_decref(objof(new_operator(new_op(NULL, OP_BINOP, 0))));
    if (o == objof(STRING(at)))
	return ici_ret_with_decref(objof(new_operator(new_op(NULL, OP_AT, 0))));
    if (o == objof(STRING(swap)))
	return ici_ret_with_decref(objof(new_operator(new_op(NULL, OP_SWAP, 0))));
    if (o == objof(STRING(binop_for_temp)))
	return ici_ret_with_decref(objof(new_operator(new_op(NULL, OP_BINOP_FOR_TEMP, 0))));
    if (o == objof(STRING(aggr_key_call)))
	return ici_ret_with_decref(objof(new_operator(new_op(NULL, OP_AGGR_KEY_CALL, 0))));
    if (o == objof(STRING(colon)))
	return ici_ret_with_decref(objof(new_operator(new_op(NULL, OP_COLON, 0))));
    if (o == objof(STRING(coloncaret)))
	return ici_ret_with_decref(objof(new_operator(new_op(NULL, OP_COLONCARET, 0))));
    if (o == objof(STRING(method_call)))
	return ici_ret_with_decref(objof(new_operator(new_op(NULL, OP_METHOD_CALL, 0))));
    if (o == objof(STRING(super_call)))
	return ici_ret_with_decref(objof(new_operator(new_op(NULL, OP_SUPER_CALL, 0))));
    if (o == objof(STRING(assignlocalvar)))
	return ici_ret_with_decref(objof(new_operator(new_op(NULL, OP_ASSIGNLOCALVAR, 0))));

    return ici_argerror(0);
}

/*
 * string = vm.binop(op)
 * string = vm.binop(operator)
 *
 * Return the token for a binary operator.
 */
FUNC(binop)
{
    op_t	*op;

    NEED_STRINGS(1);
    if (NARGS() != 1)
	return ici_argcount(0);
    if (isoperator(ARG(0)))
	op = operatorof(ARG(0))->o_op;
    else if (isop(ARG(0)))
	op = opof(ARG(0));
    else
	return ici_argerror(0);
    switch (op->op_code)
    {
    case t_subtype(T_ASTERIX):
	return str_ret("*");
    case t_subtype(T_SLASH):
	return str_ret("/");
    case t_subtype(T_PERCENT):
	return str_ret("%");
    case t_subtype(T_PLUS):
	return str_ret("+");
    case t_subtype(T_MINUS):
	return str_ret("-");
    case t_subtype(T_GRTGRT):
	return str_ret(">>");
    case t_subtype(T_LESSLESS):
	return str_ret("<<");
    case t_subtype(T_LESS):
	return str_ret("<");
    case t_subtype(T_GRT):
	return str_ret(">");
    case t_subtype(T_LESSEQ):
	return str_ret("<=");
    case t_subtype(T_GRTEQ):
	return str_ret(">=");
    case t_subtype(T_EQEQ):
	return str_ret("==");
    case t_subtype(T_EXCLAMEQ):
	return str_ret("!=");
    case t_subtype(T_TILDE):
	return str_ret("~");
    case t_subtype(T_EXCLAMTILDE):
	return str_ret("!~");
    case t_subtype(T_2TILDE):
	return str_ret("~~");
    case t_subtype(T_3TILDE):
	return str_ret("~~~");
    case t_subtype(T_AND):
	return str_ret("&");
    case t_subtype(T_CARET):
	return str_ret("^");
    case t_subtype(T_BAR):
	return str_ret("|");
    case t_subtype(T_ANDAND):
	return str_ret("&&");
    case t_subtype(T_BARBAR):
	return str_ret("||");
    case t_subtype(T_COLON):
	return str_ret(":");
    case t_subtype(T_QUESTION):
	return str_ret("?");
    case t_subtype(T_EQ):
	return str_ret("=");
    case t_subtype(T_COLONEQ):
	return str_ret(":=");
    case t_subtype(T_PLUSEQ):
	return str_ret("+=");
    case t_subtype(T_MINUSEQ):
	return str_ret("-=");
    case t_subtype(T_ASTERIXEQ):
	return str_ret("*=");
    case t_subtype(T_SLASHEQ):
	return str_ret("/=");
    case t_subtype(T_PERCENTEQ):
	return str_ret("%=");
    case t_subtype(T_GRTGRTEQ):
	return str_ret(">>=");
    case t_subtype(T_LESSLESSEQ):
	return str_ret("<<=");
    case t_subtype(T_ANDEQ):
	return str_ret("&=");
    case t_subtype(T_CARETEQ):
	return str_ret("^=");
    case t_subtype(T_BAREQ):
	return str_ret("|=");
    case t_subtype(T_2TILDEEQ):
	return str_ret("~~=");
    case t_subtype(T_LESSEQGRT):
	return str_ret("<=>");
    case t_subtype(T_COMMA):
	return str_ret(",");
    }

    error = "unrecognised binary operator";
    return 1;
}


#if defined __MACH__ && defined __APPLE__
#include "strings.c"
#include "cfuncs.c"
#endif
