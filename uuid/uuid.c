/*
 * $Id: uuid.c,v 1.1 2000/10/13 10:16:45 atrn Exp $
 *
 * ICI UUID library interface. Placed in the PUBLIC DOMAIN, A.Newman, October 2000.
 *
 */

#include "ici.h"
#include <uuid/uuid.h>

typedef struct
{
    object_t	o_head;
    uuid_t	u_uid;
}
ici_uuid_t;

inline static
ici_uuid_t *
uuidof(void *p)
{
    return (ici_uuid_t *)p;
}

static unsigned long
mark_uuid(object_t *o)
{
    o->o_flags |= O_MARK;
    return sizeof (ici_uuid_t);
}

static type_t
ici_uuid_type =
{
    mark_uuid,
    free_simple,
    hash_unique,
    cmp_unique,
    copy_simple,
    assign_simple,
    fetch_simple,
    "UUID"
};

inline static int
isuuid(object_t *o)
{
    return o->o_type == &ici_uuid_type;
}

static ici_uuid_t *
new_uuid(uuid_t uuid)
{
    ici_uuid_t	*u;

    if ((u = talloc(ici_uuid_t)) != NULL)
    {
	objof(u)->o_tcode = TC_OTHER;
	objof(u)->o_nrefs = 1;
	objof(u)->o_flags = 0;
	objof(u)->o_type  = &ici_uuid_type;
	rego(u);
	memcpy(u->u_uid, uuid, 16);
    }
    return u;
}

FUNC(clear)
{
    ici_uuid_t	*u;

    if (ici_typecheck("o", &u))
	return 1;
    if (!isuuid(objof(u)))
	return ici_argerror(0);
    uuid_clear(u->u_uid);
    return null_ret();
}

FUNC(compare)
{
    ici_uuid_t	*u;
    ici_uuid_t	*v;

    if (ici_typecheck("oo", &u, &v))
	return 1;
    if (!isuuid(objof(u)))
	return ici_argerror(0);
    if (!isuuid(objof(v)))
	return ici_argerror(1);
    return int_ret(uuid_compare(u->u_uid, v->u_uid));
}

FUNC(copy)
{
    ici_uuid_t	*u;
    uuid_t	uu2;

    if (ici_typecheck("o", &u))
	return 1;
    if (!isuuid(objof(u)))
	return ici_argerror(0);
    uuid_copy(u->u_uid, uu2);
    return ici_ret_with_decref(objof(new_uuid(uu2)));
}

FUNC(generate)
{
    uuid_t	uu;

    uuid_generate(uu);
    return ici_ret_with_decref(objof(new_uuid(uu)));
}

FUNC(generate_random)
{
    uuid_t	uu;

    uuid_generate_random(uu);
    return ici_ret_with_decref(objof(new_uuid(uu)));
}

FUNC(generate_time)
{
    uuid_t	uu;

    uuid_generate_time(uu);
    return ici_ret_with_decref(objof(new_uuid(uu)));
}

FUNC(is_null)
{
    ici_uuid_t	*u;

    if (ici_typecheck("o", &u))
	return 1;
    if (!isuuid(objof(u)))
	return ici_argerror(0);
    return int_ret(uuid_is_null(u->u_uid));
}

FUNC(parse)
{
    char	*str;
    uuid_t	uu;

    if (ici_typecheck("s", &str))
	return 1;
    if (uuid_parse(str, uu) == -1)
    {
	error = "invalid UUID";
	return 1;
    }
    return ici_ret_with_decref(objof(new_uuid(uu)));
}

FUNC(unparse)
{
    ici_uuid_t	*u;
    char	mybuf[64];

    if (ici_typecheck("o", &u))
	return 1;
    if (!isuuid(objof(u)))
	return ici_argerror(0);
    uuid_unparse(u->u_uid, mybuf);
    return str_ret(mybuf);
}

FUNC(time)
{
    ici_uuid_t		*u;
    time_t		t;

    if (ici_typecheck("o", &u))
	return 1;
    if (!isuuid(objof(u)))
	return ici_argerror(0);
    return int_ret(uuid_time(u->u_uid, NULL));
}

FUNC(type)
{
    ici_uuid_t	*u;

    if (ici_typecheck("o", &u))
	return 1;
    if (!isuuid(objof(u)))
	return ici_argerror(0);
    return int_ret(uuid_type(u->u_uid));
}

FUNC(variant)
{
    ici_uuid_t	*u;

    if (ici_typecheck("o", &u))
	return 1;
    if (!isuuid(objof(u)))
	return ici_argerror(0);
    return int_ret(uuid_variant(u->u_uid));
}
