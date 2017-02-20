/*
 * $Id: xml.c,v 1.1.1.1 1999/09/08 01:27:26 andy Exp $
 *
 * xml.c - Interface to the Expat XML parser.
 *
 * Expat is the XML parser written by J Clark. It is the parser being used to
 * add XML support to Netscape 5 and Perl.
 *
 * http://www.jclark.com/xml/expat.html
 *
 * Originally by Scott Newham <snaf@zeta.org.au>
 * Complete rewrite by Andy Newman <atrn@zeta.org.au>
 *                        ^ note difference in the names, we're not related!
 */

#include "ici.h"
#include "xmlp.h"

/*
 * Create a new xml parser with the given Expat parser.
 */
static xmlp_t *
new_xmlp(XML_Parser p)
{
    xmlp_t	*x;

    if ((x = talloc(xmlp_t)) != NULL)
    {
	objof(x)->o_type = &ici_xmlp_type;
	objof(x)->o_tcode = TC_OTHER;
	objof(x)->o_flags = 0;
	objof(x)->o_nrefs = 1;
	rego(x);
	x->x_parser   = p;
	x->x_startf   = NULL;
	x->x_endf     = NULL;
	x->x_charf    = NULL;
	x->x_defaultf = NULL;
	x->x_pinstf   = NULL;
	x->x_arg      = objof(&o_null);
    }
    return x;
}

static unsigned long
mark_xmlp(object_t *o)
{
    unsigned long	sz;

    o->o_flags |= O_MARK;
    sz  = sizeof(xmlp_t) + mark(xmlpof(o)->x_arg);
    sz += xmlpof(o)->x_startf	== NULL ? 0L : mark(xmlpof(o)->x_startf);
    sz += xmlpof(o)->x_endf	== NULL ? 0L : mark(xmlpof(o)->x_endf);
    sz += xmlpof(o)->x_charf	== NULL ? 0L : mark(xmlpof(o)->x_charf);
    sz += xmlpof(o)->x_defaultf	== NULL ? 0L : mark(xmlpof(o)->x_defaultf);
    sz += xmlpof(o)->x_pinstf	== NULL ? 0L : mark(xmlpof(o)->x_pinstf);
    return sz;
}

/*
 * Free the parser object and loosen all refs to other objects.
 */
static void
free_xmlp(object_t *o)
{
    XML_ParserFree(xmlpof(o)->x_parser);
    ici_free(o);
}

/*
 * AN: Should XML parser object's be atomic? Atomicity being
 * based on the parser structure's address.
 */
static int
cmp_xmlp(object_t *a, object_t *b)
{
    /*
     * Equivalent if the actual Expat parsers are the same.
     */
    return xmlpof(a)->x_parser != xmlpof(b)->x_parser;
}

static unsigned long
hash_xmlp(object_t *o)
{
    return (unsigned long)xmlpof(o)->x_parser * PTR_PRIME_0;
}

type_t	ici_xmlp_type =
{
    mark_xmlp,
    free_xmlp,
    hash_xmlp,
    cmp_xmlp,
    copy_simple,
    assign_simple,
    fetch_simple,
    "XML_Parser"
};

/*
 * Error utility
 */
static int
xml_error(xmlp_t *x)
{
    char	*msg;

    msg = (char *)XML_ErrorString(XML_GetErrorCode(x->x_parser));
    if (chkbuf(80))
	error = msg;
    else
    {
	sprintf
	(
	    buf,
	    "line %d, column %d, %s",
	    XML_GetCurrentLineNumber(x->x_parser),
	    XML_GetCurrentColumnNumber(x->x_parser),
	    msg
	);
	error = buf;
    }
    return 1;
}

/**
 * ParserCreate - create a new XML parser object
 */
FUNC(ParserCreate)
{
    XML_Parser	p;
    xmlp_t	*x;

    if (ici_typecheck(""))
	return 1;
    if ((p = XML_ParserCreate(NULL)) == NULL)
    {
	error = "XML: failed to create parser";
	return 1;
    }
    if ((x = new_xmlp(p)) == NULL)
    {
	XML_ParserFree(p);
	return 1;
    }
    return ici_ret_with_decref(objof(x));
}

/**
 * SetUserData - set the user data associated with an XML parser object
 */
FUNC(SetUserData)
{
    xmlp_t	*x;
    object_t	*arg;

    if (ici_typecheck("oo", &x, &arg))
	return 1;
    if (!isxmlp(objof(x)))
	return ici_argerror(0);
    x->x_arg = arg;
    return null_ret();
}

/**
 * GetUserData - get the current user data associated with an XML parser
 */
FUNC(GetUserData)
{
    xmlp_t	*x;

    if (ici_typecheck("o", &x))
	return 1;
    if (!isxmlp(objof(x)))
	return ici_argerror(0);
    return ici_ret_no_decref(x->x_arg);
}

/*
 * Generic callback function called by Expat. This in turn calls the specific
 * user callback on the ICI side.
 */
static void 
start_element(void *udata, const char *name, const char **atts)
{
    xmlp_t	*x = udata;
    object_t	*a;

    if (x->x_startf == NULL || error != NULL)
	return;
    /*
     * The attributes are put into a struct object as string objects. The
     * keys are the attribute names, the values the attribute value. If
     * there are no attributes an empty struct is passed.
     */
    if ((a = objof(new_struct())) == NULL)
	return;
    if (*atts != NULL)
    {
	long		i;
	char		**p;
	string_t	*n;
	string_t	*s;

	for (p = (char **)atts, i = 0; *p != NULL; p += 2, i += 2)
	{
	    assert(p[0] != NULL && p[1] != NULL);
	    if ((n = new_cname(p[0])) == NULL)
	    {
		decref(a);
		return;
	    }
	    if ((s = new_cname(p[1])) == NULL)
	    {
		decref(n);
		decref(a);
		return;
	    }
	    if (assign(a, objof(n), objof(s)))
	    {
		decref(s);
		decref(n);
		decref(a);
		return;
	    }
	    decref(s);
	    decref(n);
	}
    }
    error = ici_func(objof(x->x_startf), "oso", x->x_arg, name, a);
    decref(a);
}

/*
 * Generic callback function called by expat. This in turn calls the specific
 * user callback on the ICI side.
 */
static void
end_element(void *udata, const char *name)
{
    xmlp_t	*x = udata;

    if (x->x_endf != NULL && error == NULL)
	error = ici_func(objof(x->x_endf), "os", x->x_arg, name);
}

/**
 * SetElementHandler - define callback functions for XML element processing
 */
FUNC(SetElementHandler)
{
    xmlp_t	*x;
    func_t	*start;
    func_t	*end;

    switch (NARGS())
    {
    case 1:
	x = xmlpof(ARG(0));
	start = end = NULL;
	break;

    default:
	if (ici_typecheck("ooo", &x, &start, &end))
	    return 1;
	if (!funcof(objof(start)))
	    return ici_argerror(1);
	if (!funcof(objof(end)))
	    return ici_argerror(2);
    }
    if (!isxmlp(objof(x)))
	return ici_argerror(0);
    x->x_startf = start;
    x->x_endf = end;
    XML_SetElementHandler(x->x_parser, start_element, end_element);
    XML_SetUserData(x->x_parser, x);
    return null_ret();
}

static void
character_data(void *udata, const XML_Char *str, int len)
{
    xmlp_t	*x = udata;

    if (x->x_charf != NULL && error == NULL)
    {
	string_t	*s;

	if ((s = new_name((char *)str, len)) != NULL)
	{
	    error = ici_func(objof(x->x_charf), "oo", x->x_arg, objof(s));
	    decref(s);
	}
    }
}

FUNC(SetCharacterDataHandler)
{
    xmlp_t	*x;
    func_t	*fn;

    switch (NARGS())
    {
    case 1:
	x = xmlpof(ARG(0));
	fn = NULL;
	break;

    default:
	if (ici_typecheck("oo", &x, &fn))
	    return 1;
	if (!isfunc(objof(fn)))
	    return ici_argerror(1);
    }
    if (!isxmlp(objof(x)))
	return ici_argerror(0);
    x->x_charf = fn;
    XML_SetCharacterDataHandler(x->x_parser, character_data);
    XML_SetUserData(x->x_parser, x);
    return null_ret();
}

static void
default_handler(void *udata, const XML_Char *str, int len)
{
    xmlp_t	*x = udata;

    if (x->x_defaultf != NULL && error == NULL)
    {
	string_t	*s;

	if ((s = new_name((char *)str, len)) != NULL)
	{
	    error = ici_func(objof(x->x_defaultf), "oo", x->x_arg, objof(s));
	    decref(s);
	}
    }
}

/**
 * SetDefaultHandler - define default callback function
 */
FUNC(SetDefaultHandler)
{
    xmlp_t	*x;
    func_t	*fn;

    switch (NARGS())
    {
    case 1:
	x = xmlpof(ARG(0));
	fn = NULL;
	break;

    default:
	if (ici_typecheck("oo", &x, &fn))
	    return 1;
	if (!isfunc(objof(fn)))
	    return ici_argerror(1);
    }
    if (!isxmlp(objof(x)))
	return ici_argerror(0);
    x->x_defaultf = fn;
    XML_SetDefaultHandler(x->x_parser, default_handler);
    XML_SetUserData(x->x_parser, x);
    return null_ret();
}

static void
processing_instruction(void *udata, const XML_Char *targ, const XML_Char *data)
{
    xmlp_t	*x = udata;

    if (x->x_pinstf != NULL && error == NULL)
    {
	string_t	*t;
	string_t	*d;

	if ((t = new_cname((char *)targ)) == NULL)
	    return;
	if ((d = new_cname((char *)data)) == NULL)
	{
	    decref(t);
	    return;
	}
	error = ici_func(objof(x->x_pinstf), "ooo", x->x_arg, t, d);
	decref(t);
	decref(d);
    }
}

/**
 * SetProcessingInstructionHandler - set callback for processing instructions
 */
FUNC(SetProcessingInstructionHandler)
{
    xmlp_t	*x;
    func_t	*fn;

    switch (NARGS())
    {
    case 1:
	x = xmlpof(ARG(0));
	fn = NULL;
	break;

    default:
	if (ici_typecheck("oo", &x, &fn))
	    return 1;
	if (!isfunc(objof(fn)))
	    return ici_argerror(1);
    }
    if (!isxmlp(objof(x)))
	return ici_argerror(0);
    x->x_pinstf = fn;
    XML_SetProcessingInstructionHandler(x->x_parser, processing_instruction);
    XML_SetUserData(x->x_parser, x);
    return null_ret();
}

/**
 * Parse - parse an XML stream
 */
FUNC(Parse)
{
    xmlp_t	*x;
    string_t	*s;
    long	is_final;

    switch (NARGS())
    {
    case 2:
	if (ici_typecheck("oo", &x, &s))
	    return 1;
	is_final = 1;
	break;

    default:
	if (ici_typecheck("ooi", &x, &s, &is_final))
	    return 1;
    }
    if (!isxmlp(objof(x)))
	return ici_argerror(0);
    if (!stringof(objof(s)))
	return ici_argerror(1);
    error = NULL;
    if (!XML_Parse(x->x_parser, s->s_chars, s->s_nchars, is_final))
	return xml_error(x);
    return error != NULL ? 1 : null_ret();
}

/**
 * GetErrorCode - return the current error code from an XML parser object
 */
FUNC(GetErrorCode)
{
    xmlp_t	*x;

    if (ici_typecheck("o", &x))
	return 1;
    if (!isxmlp(objof(x)))
	return ici_argerror(0);
    return int_ret(XML_GetErrorCode(x->x_parser));
}

/**
 * ErrorString - return the current error string from an XML parser object
 */
FUNC(ErrorString)
{
    xmlp_t	*x;

    if (ici_typecheck("o", &x))
	return 1;
    if (!isxmlp(objof(x)))
	return ici_argerror(0);
    return str_ret((char*)XML_ErrorString(XML_GetErrorCode(x->x_parser)));
}

/**
 * GetCurrentLineNumber - return the current line number
 */
FUNC(GetCurrentLineNumber)
{
    xmlp_t	*x;

    if (ici_typecheck("o", &x))
	return 1;
    if (!isxmlp(objof(x)))
	return ici_argerror(0);
    return int_ret(XML_GetCurrentLineNumber(x->x_parser));
}

/**
 * GetCurrentColumnNumber - return the current column number
 */
FUNC(GetCurrentColumnNumber)
{
    xmlp_t	*x;

    if (ici_typecheck("o", &x))
	return 1;
    if (!isxmlp(objof(x)))
	return ici_argerror(0);
    return int_ret(XML_GetCurrentColumnNumber(x->x_parser));
}

/**
 * GetCurrentByteIndex - return the current byte index
 */
FUNC(GetCurrentByteIndex)
{
    xmlp_t	*x;

    if (ici_typecheck("o", &x))
	return 1;
    if (!isxmlp(objof(x)))
	return ici_argerror(0);
    return int_ret(XML_GetCurrentByteIndex(x->x_parser));
}

/**
 * SetBase
 */
FUNC(SetBase)
{
    xmlp_t	*x;
    char	*s;

    if (ici_typecheck("os", &x, &s))
	return 1;
    if (!isxmlp(objof(x)))
	return ici_argerror(0);
    if (!XML_SetBase(x->x_parser, (XML_Char *)s))
	return xml_error(x);
    return null_ret();
}
