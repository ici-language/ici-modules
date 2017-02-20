/* 
 * $Id: shout.c,v 1.1 2000/09/10 00:01:23 atrn Exp $
 *
 * This code is in the public domain. No copyright applies.
 *
 *
 * Interface to libshout. See it's API documentation for details.
 * The ICI i/f is identical to the C API with the following exceptions,
 *
 *	o all names omit the "shout_" prefix as they are expected to
 *	  to be used via auto-loading of the "shout" module, e.g,
 *	  shout.disconnect(conn) vs. shout_disconnect(&conn).
 *
 *	o connection structures are created and returned by
 *	  shout.init_connection() rather than being allocated
 *	  by the user as passed to the function. The differences
 *	  between the C and ICI data models makes this the more
 *	  sensible approach in ICI.
 *
 *	o when libshout functions return errors, ICI errors are
 *	  raised.
 *
 *	o the shout.send_data() function takes slightly different
 *	  parameters than its C version. In ICI we use strings to
 *	  store the data to be sent and need not pass a length
 *	  parameter as ICI knows the length of strings.
 *
 *
 * Andy Newman <atrn@zeta.org.au>
 *
 */

#include "ici.h"
#include <shout/shout.h>

typedef struct
{
    object_t		o_head;
    shout_conn_t	c_conn;
}
conn_t;

static inline conn_t *
connof(void *o)
{
    return (conn_t *)o;
}

static unsigned long
mark_conn(object_t *o)
{
    o->o_flags |= O_MARK;
    return sizeof (conn_t);
}

static void
free_conn(object_t *o)
{
    if (connof(o)->c_conn.connected)
	shout_disconnect(&connof(o)->c_conn);
    ici_free(o);
}

NEED_STRING(ip);
NEED_STRING(port);
NEED_STRING(mount);
NEED_STRING(password);
NEED_STRING(connected);
NEED_STRING(icy_compat);
NEED_STRING(name);
NEED_STRING(genre);
NEED_STRING(description);
NEED_STRING(url);
NEED_STRING(bitrate);
NEED_STRING(ispublic);
NEED_STRING(dumpfile);
NEED_STRING(error);
NEED_STRING(frames);

static int
assign_conn(object_t *o, object_t *k, object_t *v)
{
    NEED_STRINGS(1);

#define	REQSTR()	if (!isstring(v)) goto fail
#define	REQINT()	if (!isint(v)) goto fail

    if (k == objof(STRING(ip)))
    {
	REQSTR();
	connof(o)->c_conn.ip = stringof(v)->s_chars;
	return 0;
    }
    if (k == objof(STRING(port)))
    {
	REQINT();
	connof(o)->c_conn.port = intof(v)->i_value;
	return 0;
    }
    if (k == objof(STRING(mount)))
    {
	REQSTR();
	connof(o)->c_conn.mount = stringof(v)->s_chars;
	return 0;
    }
    if (k == objof(STRING(password)))
    {
	REQSTR();
	connof(o)->c_conn.password = stringof(v)->s_chars;
	return 0;
    }
    if (k == objof(STRING(icy_compat)))
    {
	REQINT();
	connof(o)->c_conn.icy_compat = intof(v)->i_value;
	return 0;
    }
    if (k == objof(STRING(name)))
    {
	REQSTR();
	connof(o)->c_conn.name = stringof(v)->s_chars;
	return 0;
    }
    if (k == objof(STRING(genre)))
    {
	REQSTR();
	connof(o)->c_conn.genre = stringof(v)->s_chars;
	return 0;
    }
    if (k == objof(STRING(description)))
    {
	REQSTR();
	connof(o)->c_conn.description = stringof(v)->s_chars;
	return 0;
    }
    if (k == objof(STRING(url)))
    {
	REQSTR();
	connof(o)->c_conn.url = stringof(v)->s_chars;
	return 0;
    }
    if (k == objof(STRING(bitrate)))
    {
	REQINT();
	connof(o)->c_conn.bitrate = intof(v)->i_value;
	return 0;
    }
    if (k == objof(STRING(ispublic)))
    {
	REQINT();
	connof(o)->c_conn.ispublic = intof(v)->i_value;
	return 0;
    }
    if (k == objof(STRING(dumpfile)))
    {
	REQSTR();
	connof(o)->c_conn.dumpfile = stringof(v)->s_chars;
	return 0;
    }
fail:
    return assign_simple(o, k, v);

#undef REQSTR
#undef REQINT
}

static object_t *
fetch_conn(object_t *o, object_t *k)
{
    NEED_STRINGS(NULL);
    if (k == objof(STRING(ip)))
	return objof(new_cname(connof(o)->c_conn.ip));
    if (k == objof(STRING(port)))
	return objof(new_int(connof(o)->c_conn.port));
    if (k == objof(STRING(mount)))
	return objof(new_cname(connof(o)->c_conn.mount));
    if (k == objof(STRING(password)))
	return objof(new_cname(connof(o)->c_conn.password));
    if (k == objof(STRING(connected)))
	return objof(new_int(connof(o)->c_conn.connected));
    if (k == objof(STRING(icy_compat)))
	return objof(new_int(connof(o)->c_conn.icy_compat));
    if (k == objof(STRING(name)))
	return objof(new_cname(connof(o)->c_conn.name));
    if (k == objof(STRING(genre)))
	return objof(new_cname(connof(o)->c_conn.genre));
    if (k == objof(STRING(description)))
	return objof(new_cname(connof(o)->c_conn.description));
    if (k == objof(STRING(url)))
	return objof(new_cname(connof(o)->c_conn.url));
    if (k == objof(STRING(bitrate)))
	return objof(new_int(connof(o)->c_conn.bitrate));
    if (k == objof(STRING(ispublic)))
	return objof(new_int(connof(o)->c_conn.ispublic));
    if (k == objof(STRING(dumpfile)))
	return objof(new_cname(connof(o)->c_conn.dumpfile));
    if (k == objof(STRING(error)))
	return objof(new_int(connof(o)->c_conn.error));
    if (k == objof(STRING(frames)))
	return objof(new_int(connof(o)->c_conn.frames));
    return fetch_simple(o, k);
}

static type_t
conn_type =
{
    mark_conn,
    free_conn,
    hash_unique,
    cmp_unique,
    copy_simple,
    assign_conn,
    fetch_conn,
    "conn"
};

static inline int
isconn(object_t *o)
{
    return o->o_type == &conn_type;
}

static conn_t *
new_conn(void)
{
    conn_t	*c;

    if ((c = talloc(conn_t)) != NULL)
    {
	objof(c)->o_tcode = TC_OTHER;
	objof(c)->o_nrefs = 1;
	objof(c)->o_flags = 0;
	objof(c)->o_type  = &conn_type;
	rego(c);
	shout_init_connection(&c->c_conn);
    }
    return c;
}

static int
libshout_error(conn_t *c)
{
    char *err = NULL;

    switch (c->c_conn.error)
    {
    case SHOUTERR_INSANE:
	err = "internal error, insane parameters passed to libshout";
	break;
    case SHOUTERR_NOCONNECT:
	err = "unable to connect to server";
	break;
    case SHOUTERR_NOLOGIN:
	err = "unable to login to server";
	break;
    case SHOUTERR_MALLOC:
	err = "libshout memory allocation error";
	break;
    case SHOUTERR_SOCKET:
	err = "I/O error sending data to server";
	break;
    case SHOUTERR_METADATA:
	err = "unable to send meta-data";
	break;
    }
    if (err == NULL)
	err = "error in libshout function";
    error = err;
    return 1;
}

FUNC(init_connection)
{
    return ici_ret_with_decref(objof(new_conn()));
}

FUNC(connect)
{
    conn_t	*c;

    if (ici_typecheck("o", &c))
	return 1;
    if (!isconn(objof(c)))
	return ici_argerror(0);
    if (!shout_connect(&c->c_conn))
	return libshout_error(c);
    return null_ret();
}

FUNC(send_data)
{
    conn_t	*c;
    string_t	*s;

    if (ici_typecheck("oo", &c, &s))
	return 1;
    if (!isconn(objof(c)))
	return ici_argerror(0);
    if (!isstring(objof(s)))
	return ici_argerror(1);
    if (!shout_send_data(&c->c_conn, s->s_chars, s->s_nchars))
	return libshout_error(c);
    return null_ret();
}

FUNC(sleep)
{
    conn_t	*c;

    if (ici_typecheck("o", &c))
	return 1;
    if (!isconn(objof(c)))
	return ici_argerror(0);
    shout_sleep(&c->c_conn);
    return null_ret();
}

FUNC(disconnect)
{
    conn_t	*c;

    if (ici_typecheck("o", &c))
	return 1;
    if (!isconn(objof(c)))
	return ici_argerror(0);
    if (!shout_disconnect(&c->c_conn))
	return libshout_error(c);
    return null_ret();
}

FUNC(update_metadata)
{
    conn_t	*c;
    char	*m;

    if (ici_typecheck("os", &c, &m))
	return 1;
    if (!isconn(objof(c)))
	return ici_argerror(0);
    if (!shout_update_metadata(&c->c_conn, m))
	return libshout_error(c);
    return null_ret();
}
