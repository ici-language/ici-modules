#include "ici.h"
#include "load.h"
#include "dllmanager.h"
#include "dll.h"

/*
 * Make a new dllmanager object. The dllmanager object holds a shadow struct
 * that is used to hold, by name, any dlls that have been loaded.
 *
 * Typically only one of these is ever created when the first mention of
 * 'dll' is made in a ICI program.
 */
static dllmanager_t *
new_dllmanager(void)
{
    dllmanager_t   *dllm;

    if ((dllm = talloc(dllmanager_t)) == NULL)
        return NULL;
    if ((dllm->dllm_struct = new_struct()) == NULL)
    {
        ici_free(dllm);
        return NULL;
    }
    objof(dllm)->o_tcode = TC_OTHER;
    objof(dllm)->o_type  = &dllmanager_type;
    objof(dllm)->o_nrefs = 1;
    objof(dllm)->o_flags = 0;
    rego(objof(dllm));
    decref(dllm->dllm_struct);
    return dllm;
}

/*
 * Mark this and referenced unmarked objects, return memory costs.
 * See comments on t_mark() in object.h.
 */
static unsigned long
mark_dllmanager(object_t *o)
{
    o->o_flags |= O_MARK;
    return sizeof(dllmanager_t) + mark(dllmanagerof(o)->dllm_struct);
}

/*
 * Assign to key k of the object o the value v. Return 1 on error, else 0.
 * See the comment on t_assign() in object.h.
 *
 * We just pass this through to our backing struct.
 */
static int
assign_dllmanager(object_t *o, object_t *k, object_t *v)
{
    return assign(dllmanagerof(o)->dllm_struct, k, v);
}

/*
 * Return the object at key k of the obejct o, or NULL on error.
 * See the comment on t_fetch in object.h.
 *
 * If the key already exists in our backing struct, we just return
 * its value. Otherwise the key is interpreted as a dll name, the dll
 * is loaded (if possible) and returned, with the side effect of assigning
 * it to our backing struct as well.
 */
static object_t *
fetch_dllmanager(object_t *o, object_t *k)
{
    char                *path;
    char                fname[FILENAME_MAX];
    dll_t               lib;
    int                 gotlib;
    object_t            *v;

    gotlib = 0;
    if ((v = fetch(dllmanagerof(o)->dllm_struct, k)) == NULL)
        return NULL;
    if (v != objof(&o_null))
        return v;
    if (!isstring(k) || stringof(k)->s_nchars > sizeof fname - 1)
        return fetch_simple(o, k);

    /*
     * First try to load it directly. (Windows)
     */
    strcpy(fname, stringof(k)->s_chars);
    lib = dlopen(fname, RTLD_NOW|RTLD_GLOBAL);
    if (!valid_dll(lib))
    {
        /*
         * That didn't work. Try finding the file explicitly and using
         * that.
         */
        path = get_dll_path();
#ifndef WIN32
        sprintf(fname, "lib%s", stringof(k)->s_chars);
#endif
        if
        (
            !ici_find_on_path(path, fname, ICI_DLL_EXT)
            &&
            !ici_find_on_path(path, fname, NULL)
        )
        {
            char            n[30];

            sprintf(ici_buf, "could not locate dynamic-load library %s",
                objname(n, k));
            error = ici_buf;
            return NULL;
        }

        /*
         * We have a file. Attempt to dynamically load it.
         */
        lib = dlopen(fname, RTLD_NOW|RTLD_GLOBAL);
    }
    if (!valid_dll(lib))
    {
        char        *err;
        char const  fmt[] = "failed to load %s, %s";

        if ((err = (char *)dlerror()) == NULL)
            err = "dynamic link error";
        if (chkbuf(strlen(fmt) + strlen(fname) + strlen(err) + 1))
            strcpy(ici_buf, err);
        else
            sprintf(ici_buf, fmt, fname, err);
        error = buf;
        return NULL;
    }
    gotlib = 1;
    if ((v = objof(new_dll(lib))) == NULL)
        goto fail;
    if (assign(dllmanagerof(o)->dllm_struct, k, v))
    {
        decref(v);
        goto fail;
    }
    decref(v);
    return v;

fail:
#ifndef SUNOS5 /* Doing the dlclose results in a crash under Solaris - why? */
    if (gotlib)
        dlclose(lib);
#endif
    return NULL;
}

static type_t dllmanager_type =
{
    mark_dllmanager,
    free_simple,
    hash_unique,
    cmp_unique,
    copy_simple,
    assign_dllmanager,
    fetch_dllmanager,
    "dllmanager"
};

object_t *
ici_dll_library_init(void)
{
    return objof(new_dllmanager());
}
