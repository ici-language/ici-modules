/*
 * $Id: xml.c,v 1.5 2003/01/23 10:38:32 timl Exp $
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
 * Another rewrite to make it OO in ICI 4 by Tim Long.
 */

#include <ici.h>
#include "icistr.h"
#include <icistr-setup.h>
#include <xmlparse.h>
#include <assert.h>

objwsup_t    *ici_xml_module;
objwsup_t    *ici_xml_parser_class;

/*
 * Convert the current error in the given XML_Parser object (p) to
 * an ICI error. Returns 1.
 */
static int
ici_xml_error(XML_Parser p)
{
    char        *msg;

    msg = (char *)XML_ErrorString(XML_GetErrorCode(p));
    if (ici_chkbuf(80))
        ici_error = msg;
    else
    {
        sprintf
        (
            ici_buf,
            "line %d, column %d: %s",
            XML_GetCurrentLineNumber(p),
            XML_GetCurrentColumnNumber(p),
            msg
        );
        ici_error = ici_buf;
    }
    return 1;
}

/*
 * Generic callback function called by Expat. This in turn calls the specific
 * user callback on the ICI side.
 */
static void
start_element(void *udata, const char *name, const char **atts)
{
    ici_handle_t    *h = udata;
    object_t    *a;

    if (ici_error != NULL)
        return;
    /*
     * The attributes are put into a struct object as string objects. The
     * keys are the attribute names, the values the attribute value. If
     * there are no attributes an empty struct is passed.
     */
    if ((a = objof(ici_struct_new())) == NULL)
        return;
    if (*atts != NULL)
    {
        long            i;
        char            **p;
        string_t        *n;
        string_t        *s;

        for (p = (char **)atts, i = 0; *p != NULL; p += 2, i += 2)
        {
            assert(p[0] != NULL && p[1] != NULL);
            if ((n = ici_str_new_nul_term(p[0])) == NULL)
            {
                ici_decref(a);
                return;
            }
            if ((s = ici_str_new_nul_term(p[1])) == NULL)
            {
                ici_decref(n);
                ici_decref(a);
                return;
            }
            if (ici_assign(a, objof(n), objof(s)))
            {
                ici_decref(s);
                ici_decref(n);
                ici_decref(a);
                return;
            }
            ici_decref(s);
            ici_decref(n);
        }
    }
    ici_method(objof(h), ICIS(start_element), "so", name, a);
    ici_decref(a);
}

/*
 * Generic callback function called by expat. This in turn calls the specific
 * user callback on the ICI side.
 */
static void
end_element(void *udata, const char *name)
{
    ici_handle_t    *h = udata;

    if (ici_error != NULL)
        return;
    ici_method(objof(h), ICIS(end_element), "s", name);
}

static void
character_data(void *udata, const XML_Char *str, int len)
{
    ici_handle_t        *h = udata;
    string_t        *s;

    if (ici_error != NULL)
        return;
    if ((s = ici_str_new((char *)str, len)) != NULL)
    {
        ici_method(objof(h), ICIS(character_data), "o", s);
        ici_decref(s);
    }
}

static void
default_handler(void *udata, const XML_Char *str, int len)
{
    ici_handle_t            *h = udata;
    string_t            *s;

    if (ici_error != NULL)
        return;
    if ((s = ici_str_new((char *)str, len)) == NULL)
        return;
    ici_method(objof(h), ICIS(default_handler), "o", s);
    ici_decref(s);
}

static void
processing_instruction(void *udata, const XML_Char *targ, const XML_Char *data)
{
    ici_handle_t            *h = udata;
    string_t            *t;
    string_t            *d;

    if (ici_error != NULL)
        return;
    if ((t = ici_str_new_nul_term((char *)targ)) == NULL)
        return;
    if ((d = ici_str_new_nul_term((char *)data)) == NULL)
    {
        ici_decref(t);
        return;
    }
    ici_method(objof(h), ICIS(processing_instruction), "oo", t, d);
    ici_decref(t);
    ici_decref(d);
}

/*
 * The handle is about to be freed. Free the parser object.
 */
static void
ici_xml_pre_free(ici_handle_t *h)
{
    XML_ParserFree((XML_Parser)h->h_ptr);
}

/**
 * ParserCreate - create a new XML parser object.
 *
 * ICI->C parser_class:new -> ici_xml_parser_new
 */
static int
ici_xml_parser_new(objwsup_t *klass)
{
    XML_Parser          p;
    ici_handle_t        *h;

    if (ici_method_check(objof(klass), 0))
        return 1;
    if (ici_typecheck(""))
        return 1;
    if ((p = XML_ParserCreate(NULL)) == NULL)
    {
        ici_error = "failed to create XML parser";
        return 1;
    }
    XML_SetElementHandler(p, start_element, end_element);
    XML_SetCharacterDataHandler(p, character_data);
    XML_SetDefaultHandler(p, default_handler);
    XML_SetProcessingInstructionHandler(p, processing_instruction);
    if ((h = ici_handle_new(p, ICIS(XML_Parser), klass)) == NULL)
        return 1;
    h->h_pre_free = ici_xml_pre_free;
    XML_SetUserData(p, h);
    return ici_ret_with_decref(objof(h));
}

/**
 * Parse - parse an XML stream
 *
 * ICI->C parser_class:Parse -> ici_xml_Parse
 */
static int
ici_xml_Parse(object_t *inst)
{
    string_t            *s;
    long                is_final;
    XML_Parser          p;
    ici_handle_t            *h;

    if (ici_handle_method_check(inst, ICIS(XML_Parser), &h, &p))
        return 1;
    switch (NARGS())
    {
    case 1:
        if (ici_typecheck("o", &s))
            return 1;
        is_final = 1;
        break;

    default:
        if (ici_typecheck("oi", &s, &is_final))
            return 1;
    }
    if (!isstring(objof(s)))
        return ici_argerror(1);
    ici_error = NULL;
    if (!XML_Parse(p, s->s_chars, s->s_nchars, is_final))
        return ici_xml_error(p);
    return ici_error != NULL ? 1 : ici_null_ret();
}

/**
 * GetErrorCode - return the current error code from an XML parser object
 *
 * ICI->C parser_class:GetErrorCode -> ici_xml_GetErrorCode
 */
static int
ici_xml_GetErrorCode(object_t *inst)
{
    XML_Parser           p;

    if (ici_handle_method_check(inst, ICIS(XML_Parser), NULL, &p))
        return 1;
    return ici_int_ret(XML_GetErrorCode(p));
}

/**
 * ErrorString - return the current error string from an XML parser object
 *
 * ICI->C parser_class:ErrorString -> ici_xml_ErrorString
 */
static int
ici_xml_ErrorString(object_t *inst)
{
    XML_Parser           p;

    if (ici_handle_method_check(inst, ICIS(XML_Parser), NULL, &p))
        return 1;
    return ici_str_ret((char*)XML_ErrorString(XML_GetErrorCode(p)));
}

/**
 * GetCurrentLineNumber - return the current line number
 *
 * ICI->C parser_class:GetCurrentLineNumber -> ici_xml_GetCurrentLineNumber
 */
static int
ici_xml_GetCurrentLineNumber(object_t *inst)
{
    XML_Parser           p;

    if (ici_handle_method_check(inst, ICIS(XML_Parser), NULL, &p))
        return 1;
    return ici_int_ret(XML_GetCurrentLineNumber(p));
}

/**
 * GetCurrentColumnNumber - return the current column number
 *
 * ICI->C parser_class:GetCurrentColNumber -> ici_xml_GetCurrentColNumber
 */
static int
ici_xml_GetCurrentColumnNumber(object_t *inst)
{
    XML_Parser           p;

    if (ici_handle_method_check(inst, ICIS(XML_Parser), NULL, &p))
        return 1;
    return ici_int_ret(XML_GetCurrentColumnNumber(p));
}

/**
 * GetCurrentByteIndex - return the current byte index
 *
 * ICI->C parser_class:GetCurrentByteIndex -> ici_xml_GetCurrentByteIndex
 */
static int
ici_xml_GetCurrentByteIndex(object_t *inst)
{
    XML_Parser           p;

    if (ici_handle_method_check(inst, ICIS(XML_Parser), NULL, &p))
        return 1;
    return ici_int_ret(XML_GetCurrentByteIndex(p));
}

/**
 * SetBase
 *
 * ICI->C parser_class:SetBase -> ici_xml_SetBase
 */
static int
ici_xml_SetBase(object_t *inst)
{
    char                *s;
    XML_Parser           p;

    if (ici_handle_method_check(inst, ICIS(XML_Parser), NULL, &p))
        return 1;
    if (ici_typecheck("s", &s))
        return 1;
    if (!XML_SetBase(p, (XML_Char *)s))
        return ici_xml_error(p);
    return ici_null_ret();
}

cfunc_t ici_xml_cfuncs[] =
{
    {CF_OBJ}
};

cfunc_t ici_xml_parser_cfuncs[] =
{
    {CF_OBJ, "new", ici_xml_parser_new},
    {CF_OBJ, "SetBase", ici_xml_SetBase},
    {CF_OBJ, "Parse", ici_xml_Parse},
    {CF_OBJ, "ErrorString", ici_xml_ErrorString},
    {CF_OBJ, "GetCurrentByteIndex", ici_xml_GetCurrentByteIndex},
    {CF_OBJ, "GetCurrentColumnNumber", ici_xml_GetCurrentColumnNumber},
    {CF_OBJ, "GetCurrentLineNumber", ici_xml_GetCurrentLineNumber},
    {CF_OBJ, "GetErrorCode", ici_xml_GetErrorCode},
    {CF_OBJ}
};

object_t *
ici_xml_library_init(void)
{
    if (ici_interface_check(ICI_VER, ICI_BACK_COMPAT_VER, "xml"))
        return NULL;
    if (init_ici_str())
        return NULL;
    if ((ici_xml_module = ici_module_new(ici_xml_cfuncs)) == NULL)
        return NULL;
    if ((ici_xml_parser_class = ici_class_new(ici_xml_parser_cfuncs, ici_xml_module)) == NULL)
        goto fail;
    if (ici_assign_base(ici_xml_module, ICIS(parser_class), ici_xml_parser_class))
        goto fail;
    ici_decref(ici_xml_parser_class);
    return objof(ici_xml_module);

fail:
    ici_decref(ici_xml_module);
    return NULL;
}
