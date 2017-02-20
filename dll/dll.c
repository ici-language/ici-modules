#include "ici.h"
#include "load.h"
#include "dllmanager.h"
#include "dll.h"

/*
 * Make a new dllib_t (aka dll) object that holds a reference to the loaded
 * library lib. The dll object has a shadow struct that is used to hold
 * functions, by name, as they are referenced.
 *
 * This object can be collected, but doing so does not cause the lib to be
 * unloaded or any such thing. This is because of some problems as explained
 * in dllfunc.c.
 */
dllib_t *
new_dll(dll_t lib)
{
    dllib_t             *dll;

    if ((dll = talloc(dllib_t)) == NULL)
        return NULL;
    if ((dll->dll_struct = new_struct()) == NULL)
    {
        ici_free(dll);
        return NULL;
    }
    objof(dll)->o_tcode = TC_OTHER;
    objof(dll)->o_type = &dll_type;
    objof(dll)->o_nrefs = 1;
    objof(dll)->o_flags = 0;
    dll->dll_lib = lib;
    rego(objof(dll));
    decref(dll->dll_struct);
    return dll;
}

/*
 * Mark this and referenced unmarked objects, return memory costs.
 * See comments on t_mark() in object.h.
 */
static unsigned long
mark_dll(object_t *o)
{
    o->o_flags |= O_MARK;
    return sizeof(dllib_t) + mark(dllof(o)->dll_struct);
}

/*
 * Assign to key k of the object o the value v. Return 1 on error, else 0.
 * See the comment on t_assign() in object.h.
 *
 * We just pass this through to our backing struct.
 */
static int
assign_dll(object_t *o, object_t *k, object_t *v)
{
    return assign(dllof(o)->dll_struct, k, v);
}

/*
 * Return the object at key k of the obejct o, or NULL on error.
 * See the comment on t_fetch in object.h.
 *
 * If the key already exists in our backing struct, we just return
 * its value. Otherwise the key is interpreted as a function name in the dll.
 * It is found (if possible) and returned, with the side effect of assigning
 * it to our backing struct as well.
 */
static object_t *
fetch_dll(object_t *o, object_t *k)
{
    object_t            *v;

    if ((v = fetch(dllof(o)->dll_struct, k)) == NULL)
        return NULL;
    if (v != objof(&o_null))
        return v;
    if (!isstring(k))
        return fetch_simple(o, k);
    if ((v = objof(new_dllfunc(dllof(o)->dll_lib, stringof(k)))) == NULL)
        return NULL;
    if (assign(dllof(o)->dll_struct, k, v))
    {
        decref(v);
        return NULL;
    }
    decref(v);
    return v;
}

static type_t dll_type =
{
    mark_dll,
    free_simple,
    hash_unique,
    cmp_unique,
    copy_simple,
    assign_dll,
    fetch_dll,
    "dll"
};
