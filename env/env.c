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
 * The env global is a struct object with a modified ici_assign function.
 */

#include <ici.h>

static int
assign_env(object_t *o, object_t *k, object_t *v)
{
    object_t	*prev;

    if (!isstring(k))
    {
	ici_error = "attempt to use non-string key with environment";
	return 1;
    }
    if (!isstring(v))
    {
	ici_error = "attempt to ici_assign non-string value to environment";
	return 1;
    }
    prev = ici_fetch(o, k);
    ici_incref(prev);
    if (ici_types[TC_STRUCT]->t_assign(o, k, v))
	goto fail;
    if (ici_chkbuf(stringof(k)->s_nchars + stringof(v)->s_nchars + 1))
    {
	if (prev == objof(&o_null))
	{
	    ici_struct_unassign(structof(o), k);
	    ++ici_vsver;
	}
	else if ((*ici_types[TC_STRUCT]->t_assign)(o, k, prev))
	    ici_error = "environment incoherency, out of memory";
	goto fail;
    }
    sprintf(ici_buf, "%s=%s", stringof(k)->s_chars, stringof(v)->s_chars);
    if (putenv(ici_buf))
    {
	if (prev == objof(&o_null))
	{
	    ici_struct_unassign(structof(o), k);
	    ++ici_vsver;
	}
	else if ((*ici_types[TC_STRUCT]->t_assign)(o, k, prev))
	    ici_error = "environment incoherency, out of memory";
	goto fail;
    }
    return 0;

fail:
    ici_decref(prev);
    return 1;
}

object_t *
ici_env_library_init(void)
{
    static ici_type_t 	env_type;
    extern char 	**environ;
    struct_t		*e;
    char		**p;
    string_t		*s;
    char		*t;
    string_t		*k;

    if (ici_interface_check(ICI_VER, ICI_BACK_COMPAT_VER, "env"))
        return NULL;

    if ((e = ici_struct_new()) == NULL)
	return NULL;
    for (p = environ; *p != NULL; ++p)
    {
	t = strchr(*p, '=');
	if (t == NULL)
	{
	    ici_error = "mal-formed environ";
	    return NULL;
	}
	if ((k = ici_str_new(*p, t - *p)) == NULL)
	    goto fail;
	if ((s = ici_str_new_nul_term(t+1)) == NULL)
	{
	    ici_decref(k);
	    goto fail;
	}
	if (ici_assign(e, k, s))
	{
	    ici_decref(k);
	    ici_decref(s);
	    goto fail;
	}
	ici_decref(s);
	ici_decref(k);
    }
    /*
     * Now we've set up the struct trap assignments to it and
     * vector them through our code that sets the real environment.
     */
    env_type = *ici_types[TC_STRUCT];
    env_type.t_assign = assign_env;
    if (!ici_register_type(&env_type))
    {
	return NULL;
    }

    return objof(e);

fail:
    ici_decref(e);
    return NULL;
}
