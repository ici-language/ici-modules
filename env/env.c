/*
 * env.c - access the process environment as a struct
 *
 * This is an ici module that provides access the whole environment
 * vector as an ici struct. Variables can be retrieved from the environment
 * by fetching them from the global variable "env" and the environment may
 * be altered by assigning to fields of this variable. E.g.,
 *
 *	printf("%s\n", env.PATH);
 *	env.LD_LIBRARY_PATH += ":/usr/local/lib/ici";
 *	etc...
 *
 * The env global is a struct object with a modified assign function.
 */

#include "struct.h"
#include "str.h"
#include "buf.h"

static int
assign_env(object_t *o, object_t *k, object_t *v)
{
    object_t	*prev;

    if (!isstring(k))
    {
	error = "attempt to use non-string key with environment";
	return 1;
    }
    if (!isstring(v))
    {
	error = "attempt to assign non-string value to environment";
	return 1;
    }
    prev = fetch(o, k);
    incref(prev);
    if (struct_type.t_assign(o, k, v))
	goto fail;
    if (chkbuf(stringof(k)->s_nchars + stringof(v)->s_nchars + 1))
    {
	if (prev == objof(&o_null))
	{
	    unassign_struct(structof(o), k);
	    NEXT_VSVER;
	}
	else if ((*struct_type.t_assign)(o, k, prev))
	    error = "environment incoherency, out of memory";
	goto fail;
    }
    sprintf(buf, "%s=%s", stringof(k)->s_chars, stringof(v)->s_chars);
    if (putenv(buf))
    {
	if (prev == objof(&o_null))
	{
	    unassign_struct(structof(o), k);
	    NEXT_VSVER;
	}
	else if ((*struct_type.t_assign)(o, k, prev))
	    error = "environment incoherency, out of memory";
	goto fail;
    }
    return 0;

fail:
    decref(prev);
    return 1;
}

object_t *
ici_env_library_init(void)
{
    static type_t 	env_type;
    extern char 	**environ;
    struct_t		*e;
    char		**p;
    string_t		*s;
    char		*t;
    string_t		*k;

    if ((e = new_struct()) == NULL)
	return NULL;
    for (p = environ; *p != NULL; ++p)
    {
	t = strchr(*p, '=');
	assert(t != NULL);
	if ((k = new_name(*p, t - *p)) == NULL)
	    goto fail;
	if ((s = new_cname(t+1)) == NULL)
	{
	    decref(k);
	    goto fail;
	}
	if (assign(e, k, s))
	{
	    decref(k);
	    decref(s);
	    goto fail;
	}
	decref(s);
	decref(k);
    }
    /*
     * Now we've set up the struct trap assignments to it and
     * vector them through our code that sets the real environment.
     */
    env_type = struct_type;
    env_type.t_assign = assign_env;
    e->o_head.o_type = &env_type;
    return objof(e);

fail:
    decref(e);
    return NULL;
}
