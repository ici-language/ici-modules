/*
 * The str module contains functions that perform various operations on
 * strings.
 *
 * The module provides native code implementations of a number
 * of common operations to avoid the interpreter overhead if
 * these operations were written in ici.
 *
 * This --intro-- forms part of the --ici-str-- documentation.
 */
#include <ici.h>
#include "icistr.h"
#include <icistr-setup.h>

#include <ctype.h>
#include <time.h>

/*
 * string = str.toupper(string)
 *
 * Convert lowercase alphabetic characters in a string to upper case.
 *
 * This --topic-- forms part of the --ici-str-- documentation.
 */
static int
ici_str_toupper(void)
{
    string_t    *str;
    string_t    *newstr;
    char    *buffer;
    char    *p, *q;

    if (ici_typecheck("o", &str))
    return 1;
    if (!isstring(objof(str)))
    return ici_argerror(0);
    if ((buffer = ici_nalloc(str->s_nchars)) == NULL)
    return 1;
    for
    (
    p = str->s_chars, q = buffer;
    p - str->s_chars < str->s_nchars;
    ++p, ++q
    )
    {
    if (islower((int)*p))
        *q = toupper(*p);
    else
        *q = *p;
    }
    newstr = ici_str_new(buffer, str->s_nchars);
    ici_nfree(buffer, str->s_nchars);
    return ici_ret_with_decref(objof(newstr));
}

/*
 * string = str.tolower(string)
 *
 * Convert uppercase alphabetic characters in a string to lower case.
 *
 * This --topic-- forms part of the --ici-str-- documentation.
 */
static int
ici_str_tolower(void)
{
    string_t    *str;
    string_t    *newstr;
    char    *buffer;
    char    *p, *q;

    if (ici_typecheck("o", &str))
    return 1;
    if (!isstring(objof(str)))
    return ici_argerror(0);
    if ((buffer = ici_alloc(str->s_nchars)) == NULL)
    return 1;
    for
    (
    p = str->s_chars, q = buffer;
    p - str->s_chars < str->s_nchars;
    ++p, ++q
    )
    {
    if (isupper((int)*p))
        *q = tolower(*p);
    else
        *q = *p;
    }
    newstr = ici_str_new(buffer, str->s_nchars);
    ici_free(buffer);
    return ici_ret_with_decref(objof(newstr));
}


/*
 * string = str.error(int)
 *
 * Return the error message associated with the given error code.
 *
 * This --topic-- forms part of the --ici-str-- documentation.
 */
static int
ici_str_error(void)
{
    long    code;
    if (ici_typecheck("i", &code))
    return 1;
    return ici_str_ret(strerror((int)code));
}


#if !defined _WIN32
# if defined BSD_4 && !defined __APPLE__
/*
 * struct = str.ptime(string, format)
 *
 * Parse a date/time in a string according to the given format
 * specification. See ptime(3) for details of the format string
 * syntax.
 *
 * This --topic-- forms part of the --ici-str-- documentation.
 */
static int
ici_str_ptime(void)
{
    char    *fmt;
    char    *str;
    struct_t    *d;
    int_t   *i;
    struct tm   tm;

    if (ici_typecheck("ss", &str, &fmt))
    return 1;
    if (strptime(str, fmt, &tm) == NULL)
    {
    ici_error = "failed to convert string";
    return 1;
    }
    if ((d = ici_struct_new()) == NULL)
    return 1;

#define ASSIGN(N)\
    if ((i = ici_int_new(tm.tm_ ## N)) == NULL || ici_assign(d, ICIS(N), i))\
    goto fail;\
    ici_decref(i)

    ASSIGN(sec);
    ASSIGN(min);
    ASSIGN(hour);
    ASSIGN(mday);
    ASSIGN(mon);
    ASSIGN(year);
    ASSIGN(wday);

#undef ASSIGN

    return ici_ret_with_decref(objof(d));

fail:
    if (i != NULL)
    ici_decref(i);
    ici_decref(d);
    return 1;
}
# endif
#endif  /* #ifndef _WIN32 */

/*
 * string = str.join(array [, sep])
 *
 * Join strings and character codes together as in the same
 * manner as the builtin implode() function but includes a
 * separator string between the elements. If not given the
 * separator defaults to a single space.
 *
 * This --topic-- forms part of the --ici-str-- documentation.
 */
static int
ici_str_join(void)
{
    array_t             *a;
    int                 i;
    object_t            **o;
    string_t            *s;
    char                *p;
    char        *sep = " ";
    char        seplen = 1;

    if (ici_typecheck("a", &a))
    {
    if (ici_typecheck("ao", &a, &s))
        return 1;
    if (!isstring(objof(s)))
        return ici_argerror(1);
    sep = s->s_chars;
    seplen = s->s_nchars;
    }
    i = 0;
    for (o = a->a_base; o < a->a_top; ++o)
    {
        switch ((*o)->o_tcode)
        {
        case TC_INT:
            ++i;
            break;

        case TC_STRING:
            i += stringof(*o)->s_nchars;
            break;
        }
    if (o < (a->a_top-1))
        i += seplen;
    }
    if ((s = ici_str_alloc(i)) == NULL)
        return 1;
    p = s->s_chars;
    for (o = a->a_base; o < a->a_top; ++o)
    {
        switch ((*o)->o_tcode)
        {
        case TC_INT:
            *p++ = (char)intof(*o)->i_value;
            break;

        case TC_STRING:
            memcpy(p, stringof(*o)->s_chars, stringof(*o)->s_nchars);
            p += stringof(*o)->s_nchars;
            break;
        }
    if (o < (a->a_top-1))
    {
        memcpy(p, sep, seplen);
        p += seplen;
    }
    }
    if ((s = stringof(ici_atom(objof(s), 1))) == NULL)
        return 1;
    return ici_ret_with_decref(objof(s));
}

static cfunc_t ici_str_cfuncs[] =
{
    {CF_OBJ, "toupper", ici_str_toupper},
    {CF_OBJ, "tolower", ici_str_tolower},
    {CF_OBJ, "error", ici_str_error},
    {CF_OBJ, "join", ici_str_join},
#if !defined _WIN32
# if defined BSD_4 && !defined __APPLE__
    {CF_OBJ, "ptime", ici_str_ptime},
# endif
#endif  /* #ifndef _WIN32 */
    {CF_OBJ}
};

object_t *
ici_str_library_init(void)
{
    if (ici_interface_check(ICI_VER, ICI_BACK_COMPAT_VER, "str"))
        return NULL;
    if (init_ici_str())
        return NULL;
    return objof(ici_module_new(ici_str_cfuncs));
}
