#include "ici.h"
#include <ctype.h>
#include <time.h>

/**
 * string = str.toupper(string)
 *
 * Convert lowercase alphabetic characters in a string to upper case.
 */
FUNC(toupper)
{
    string_t	*str;
    string_t	*newstr;
    char	*buffer;
    char	*p, *q;

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
	if (islower((int)*p))
	    *q = toupper(*p);
	else
	    *q = *p;
    }
    newstr = new_name(buffer, str->s_nchars);
    ici_free(buffer);
    return ici_ret_with_decref(objof(newstr));
}

/**
 * string = str.tolower(string)
 *
 * Convert uppercase alphabetic characters in a string to lower case.
 */
FUNC(tolower)
{
    string_t	*str;
    string_t	*newstr;
    char	*buffer;
    char	*p, *q;

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
    newstr = new_name(buffer, str->s_nchars);
    ici_free(buffer);
    return ici_ret_with_decref(objof(newstr));
}


/**
 * string = str.error(int)
 *
 * Return the error message associated with the given error code.
 */
FUNC(error)
{
    long	code;
    if (ici_typecheck("i", &code))
	return 1;
    return str_ret(strerror((int)code));
}

NEED_STRING(sec);
NEED_STRING(min);
NEED_STRING(hour);
NEED_STRING(mday);
NEED_STRING(mon);
NEED_STRING(year);
NEED_STRING(wday);
NEED_STRING(yday);
NEED_STRING(isdst);
NEED_STRING(gmtoff);
NEED_STRING(zone);

#if !defined _WIN32
# if defined BSD && !defined __APPLE__
/*
 * struct = str.ptime(string)
 */
FUNC(ptime)
{
    char	*fmt;
    char	*str;
    struct_t	*d;
    int_t	*i;
    struct tm	tm;

    NEED_STRINGS(1);
    if (ici_typecheck("ss", &str, &fmt))
	return 1;
    if (strptime(str, fmt, &tm) == NULL)
    {
	error = "failed to convert string";
	return 1;
    }
    if ((d = new_struct()) == NULL)
	return 1;

#define	ASSIGN(N)\
    if ((i = new_int(tm.tm_ ## N)) == NULL || assign(d, STRING(N), i))\
	goto fail;\
    decref(i)

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
	decref(i);
    decref(d);
    return 1;
}
# endif
#endif	/* #ifndef _WIN32 */

/*
 * string = str.join(array [, sep])
 */
FUNC(join)
{
    array_t             *a;
    int                 i;
    object_t            **o;
    string_t            *s;
    char                *p;
    char		*sep = " ";
    char		seplen = 1;

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
    if ((s = new_string(i)) == NULL)
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
    if ((s = stringof(atom(objof(s), 1))) == NULL)
        return 1;
    return ici_ret_with_decref(objof(s));
}


#if defined __MACH__ && defined __APPLE__
#include "strings.c"
#include "cfuncs.c"
#endif
