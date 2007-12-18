/*
 * Interface to the ClearSilver template processing library.
 *
 * ClearSilver is a template processing system.  ClearSilver reads,
 * textual, template files and performs various macro-processing
 * functions defined within the file.
 *
 * This --intro-- and --synopsis-- are part of --ici-clearsilver-- documentation.
 */

#define ICI_NO_OLD_NAMES

#include <ici.h>
#include "icistr.h"
#include <icistr-setup.h>
#include <ClearSilver.h>

static ici_objwsup_t    *ici_hdf_class          = NULL;

/*
 * Set the ICI error from a NEOERR
 */
static int
set_err(NEOERR *err)
{
    NEOERR *e;
    char *desc;

    for (e = err; e != NULL && e->next != NULL; e = e->next)
        ;
    desc = e != NULL ? e->desc : "ClearSilver error";

    if (ici_chkbuf(strlen(desc) + 1))
        ici_error = "ClearSilver error";
    else
    {
        strcpy(ici_buf, desc);
        ici_error = ici_buf;
    }
    nerr_ignore(&err);
    return 1;
}

/*
 * GC-hook for the HDF handle to destroy the HDF object when
 * our handle to it is collected.
 */
static void
free_hdf(ici_handle_t *h)
{
    HDF *hdf = h->h_ptr;
    hdf_destroy(&hdf);
}

/*
 * Create a new handle to an HDF
 */
static ici_handle_t *
new_hdf(HDF *hdf)
{
    ici_handle_t *h;
    if ((h = ici_handle_new(hdf, ICIS(hdf), ici_hdf_class)) != NULL)
        h->h_pre_free = free_hdf;
    return h;
}

/* 
 * Free the HDF and return 1 for use in method error returns.
 */
static int
destroy_hdf(HDF *hdf)
{
    hdf_destroy(&hdf);
    return 1;
}

/*
 * Check that an ICI method on an HDF has been called correctly
 * and return the HDF instance.
 */
static HDF *
ici_hdf_method(ici_obj_t *inst)
{
    ici_handle_t *h;
    void *ptr;

    if (ici_handle_method_check(inst, ICIS(hdf), &h, &ptr))
        ptr = NULL;
    return ptr;
}

/*
 * hdf = clearsilver.hdf.new()
 *
 * This --topic-- forms part of the --ici-clearsilver-- documentation.
 */
static int
ici_hdf_new(ici_objwsup_t *klass)
{
    HDF *hdf;
    NEOERR *err;
    ici_handle_t *h;

    if (ici_method_check(objof(klass), 0))
        return 1;
    if (ici_typecheck(""))
        return 1;
    if ((err = hdf_init(&hdf)) != STATUS_OK)
        return set_err(err);
    if ((h = new_hdf(hdf)) == NULL)
        return destroy_hdf(hdf);
    return ici_ret_with_decref(objof(h));
}

/*
 * hdf:read_file(filename)
 *
 * Read the HDF file from the named file and return an "HDF" instance
 * holding the values.
 *
 * This --topic-- forms part of the --ici-clearsilver-- documentation.
 */
static int
ici_hdf_read_file(ici_obj_t *inst)
{
    HDF *hdf;
    char *filename;
    NEOERR *err;

    if ((hdf = ici_hdf_method(inst)) == NULL)
        return 1;
    if (ici_typecheck("s", &filename))
        return 1;
    if ((err = hdf_read_file(hdf, filename)) != STATUS_OK)
        return set_err(err);
    return ici_null_ret();
}

/*
 * string = hdf:get_value(key [, default])
 *
 * Return the, string, value of the object with the given key from an HDF.
 * If the key is not defined then return the specified default value which,
 * if not passed, defaults to the empty string.
 *
 * This --topic-- forms part of the --ici-clearsilver-- documentation.
 */
static int
ici_hdf_get_value(ici_obj_t *inst)
{
    HDF *hdf;
    char *key;
    char *default_value;

    if ((hdf = ici_hdf_method(inst)) == NULL)
        return 1;
    switch (NARGS())
    {
    case 1:
        if (ici_typecheck("s", &key))
            return 1;
        default_value = "";
        break;

    default:
        if (ici_typecheck("ss", &key, &default_value))
            return 1;
    }
    return ici_str_ret(hdf_get_value(hdf, key, default_value));
}

/*
 * hdf:set_value(key, value)
 *
 * Set the value of the object named by the given key to the given,
 * string, value.
 *
 * This --topic-- forms part of the --ici-clearsilver-- documentation.
 */
static int
ici_hdf_set_value(ici_obj_t *inst)
{
    HDF *hdf;
    NEOERR *err;
    char *key;
    char *value;

    if ((hdf = ici_hdf_method(inst)) == NULL)
        return 1;
    if (ici_typecheck("ss", &key, &value))
        return 1;
    if ((err = hdf_set_value(hdf, key, value)) != STATUS_OK)
        return set_err(err);
    return ici_null_ret();
}

/*
 * hdf = hdf:get_obj(key)
 *
 * Return the object with the given key.  If no object with that
 * key exists in the HDF NULL is returned.
 *
 * This --topic-- forms part of the --ici-clearsilver-- documentation.
 */
static int
ici_hdf_get_obj(ici_obj_t *inst)
{
    HDF *hdf;
    char *key;
    ici_handle_t *h2;
    HDF *hdf2;

    if ((hdf = ici_hdf_method(inst)) == NULL)
        return 1;
    if (ici_typecheck("s", &key))
        return 1;
    if ((hdf2 = hdf_get_obj(hdf, key)) == NULL)
        return ici_null_ret();
    if ((h2 = new_hdf(hdf2)) == NULL)
        return destroy_hdf(hdf2);
    return ici_ret_with_decref(objof(hdf2));
}

/*
 * string = hdf:obj_name()
 *
 * Return the name (key) assigned to this HDF object.
 *
 * This --topic-- forms part of the --ici-clearsilver-- documentation.
 */
static int
ici_hdf_obj_name(ici_obj_t *inst)
{
    HDF *hdf;

    if ((hdf = ici_hdf_method(inst)) == NULL)
        return 1;
    if (ici_typecheck(""))
        return 1;
    return ici_str_ret(hdf_obj_name(hdf));
}

/*
 * string = hdr:obj_value()
 *
 * Return the value of this HDF object.
 *
 * This --topic-- forms part of the --ici-clearsilver-- documentation.
 */
static int
ici_hdf_obj_value(ici_obj_t *inst)
{
    HDF *hdf;

    if ((hdf = ici_hdf_method(inst)) == NULL)
        return 1;
    if (ici_typecheck(""))
        return 1;
    return ici_str_ret(hdf_obj_value(hdf));
}

/*
 * hdf = hdf:obj_child()
 *
 * This --topic-- forms part of the --ici-clearsilver-- documentation.
 */
static int
ici_hdf_obj_child(ici_obj_t *inst)
{
    HDF *hdf;
    HDF *child;
    ici_handle_t *h2;

    if ((hdf = ici_hdf_method(inst)) == NULL)
        return 1;
    if (ici_typecheck(""))
        return 1;
    if ((child = hdf_obj_child(hdf)) == NULL)
        return ici_null_ret();
    if ((h2 = new_hdf(child)) == NULL)
        return destroy_hdf(child);
    return ici_ret_with_decref(objof(h2));
}

/*
 * hdf = hdf:obj_next()
 *
 * This --topic-- forms part of the --ici-clearsilver-- documentation.
 */
static int
ici_hdf_obj_next(ici_obj_t *inst)
{
    HDF *hdf;
    HDF *child;
    ici_handle_t *h2;

    if ((hdf = ici_hdf_method(inst)) == NULL)
        return 1;
    if (ici_typecheck(""))
        return 1;
    if ((child = hdf_obj_next(hdf)) == NULL)
        return ici_null_ret();
    if ((h2 = new_hdf(child)) == NULL)
        return destroy_hdf(child);
    return ici_ret_with_decref(objof(h2));
}

/*
 * hdf = hdf:get_child()
 *
 * This --topic-- forms part of the --ici-clearsilver-- documentation.
 */
static int
ici_hdf_get_child(ici_obj_t *inst)
{
    HDF *hdf;
    char *key;
    HDF *child;
    ici_handle_t *h2;

    if ((hdf = ici_hdf_method(inst)) == NULL)
        return 1;
    if (ici_typecheck("s", &key))
        return 1;
    if ((child = hdf_get_child(hdf, key)) == NULL)
        return ici_null_ret();
    if ((h2 = new_hdf(child)) == NULL)
        return destroy_hdf(child);
    return ici_ret_with_decref(objof(h2));
}


/*
 * hdf class methods
 */
static ici_cfunc_t hdf_cfuncs[] =
{
    {CF_OBJ, "new",             ici_hdf_new},
    {CF_OBJ, "read_file",       ici_hdf_read_file},
    {CF_OBJ, "get_value",       ici_hdf_get_value},
    {CF_OBJ, "set_value",       ici_hdf_set_value},
    {CF_OBJ, "get_obj",         ici_hdf_get_obj},
    {CF_OBJ, "obj_name",        ici_hdf_obj_name},
    {CF_OBJ, "obj_value",       ici_hdf_obj_value},
    {CF_OBJ, "obj_child",       ici_hdf_obj_child},
    {CF_OBJ, "obj_next",        ici_hdf_obj_next},
    {CF_OBJ, "get_child",       ici_hdf_get_child},
    {CF_OBJ}
};



/*
 * GC hook function to free up a CSPARSE.
 */
static void
ici_template_pre_free(ici_handle_t *h)
{
    CSPARSE *template = h->h_ptr;
    cs_destroy(&template);
}

static int
destroy_cs(CSPARSE *template)
{
    cs_destroy(&template);
    return 1;
}

static CSPARSE *
ici_template_method(ici_obj_t *inst)
{
    ici_handle_t *h;
    void *ptr;

    if (ici_handle_method_check(inst, ICIS(template), &h, &ptr))
        ptr = NULL;
    return ptr;
}

/*
 * template = clearsilver.template.new(hdf)
 *
 * Construct a new template using the given HDF as the source
 * of data values.
 *
 * This --topic-- forms part of the --ici-clearsilver-- documentation.
 */
static int
ici_template_new(ici_objwsup_t *klass)
{
    ici_handle_t *h;
    HDF *hdf;
    CSPARSE *template;
    NEOERR *err;

    if (ici_method_check(objof(klass), 0))
        return 1;
    if (ici_typecheck("o", &h))
        return 1;
    if (!ishandleof(h, ICIS(hdf)))
        return ici_argerror(0);
    hdf = h->h_ptr;
    if ((err = cs_init(&template, hdf)) != STATUS_OK)
        return set_err(err);
    if ((h = ici_handle_new(template, ICIS(template), klass)) == NULL)
        return destroy_cs(template);
    h->h_pre_free = ici_template_pre_free;
    return ici_ret_with_decref(objof(h));
}

/*
 * template:parse_string(string)
 *
 * Parse template text form a string.
 *
 * This --topic-- forms part of the --ici-clearsilver-- documentation.
 */
static int
ici_template_parse_string(ici_obj_t *inst)
{
    CSPARSE *template;
    ici_str_t *str;
    NEOERR *err;

    if ((template = ici_template_method(inst)) == NULL)
        return 1;
    if (ici_typecheck("o", &str))
        return 1;
    if (!isstring(objof(str)))
        return ici_argerror(0);
    if ((err = cs_parse_string(template, str->s_chars, str->s_nchars)) != STATUS_OK)
        return set_err(err);
    return ici_null_ret();
}

/*
 * template:parse_file(string)
 *
 * Parse the template text from the named file.
 *
 * This --topic-- forms part of the --ici-clearsilver-- documentation.
 */
static int
ici_template_parse_file(ici_obj_t *inst)
{
    CSPARSE *template;
    char *filename;
    NEOERR *err;

    if ((template = ici_template_method(inst)) == NULL)
        return 1;
    if (ici_typecheck("s", &filename))
        return 1;
    if ((err = cs_parse_file(template, filename)) != STATUS_OK)
        return set_err(err);
    return ici_null_ret();
}

/*
 * Callback function used by ici_template_render() to append
 * characters to a string.
 */
static NEOERR *
append_to_string(void *ctx, char *s)
{
    ici_str_t *buf = ctx;
    size_t nchars = strlen(s);

    if (ici_str_need_size(buf, buf->s_nchars + nchars + 1))
        return nerr_raise(NERR_NOMEM, "%s", ici_error);
    memcpy(&buf->s_chars[buf->s_nchars], s, nchars);
    buf->s_nchars += nchars;
    buf->s_chars[buf->s_nchars] = '\0';
    return STATUS_OK;
}

static NEOERR *
write_to_file(void *ctx, char *s)
{
    ici_file_t *file = ctx;
    ici_exec_t *x;

    if (file->f_type->ft_flags & FT_NOMUTEX)
        x = ici_leave();
    (*file->f_type->ft_write)(s, strlen(s), file->f_file);
    if (file->f_type->ft_flags & FT_NOMUTEX)
        ici_enter(x);

    return STATUS_OK;
}

/*
 * string = template:render()
 * template:render(file)
 *
 * Render a template by interpreting the template directives contained within
 * the template with data values coming form the HDF associated with the template
 * when it was created.
 *
 * The first form processes the template and returns a string containing the
 * processed output.
 *
 * The second form writes the text resulting from template processing to
 * the given file.
 *
 * This --topic-- forms part of the --ici-clearsilver-- documentation.
 */
static int
ici_template_render(ici_obj_t *inst)
{
    CSPARSE     *template;
    NEOERR      *err;
    ici_file_t  *file;

    if ((template = ici_template_method(inst)) == NULL)
        return 1;
    if (NARGS() == 0)
    {
        ici_str_t *buf;

        if ((buf = ici_str_buf_new(BUFSIZ)) == NULL)
            return 1;
        if ((err = cs_render(template, buf, append_to_string)) != STATUS_OK)
        {
            ici_decref(buf);
            return set_err(err);
        }
        return ici_ret_with_decref(objof(buf));
    }

    if (ici_typecheck("u", &file))
        return 1;
    if ((err = cs_render(template, file, write_to_file)) != STATUS_OK)
        return set_err(err);
    return ici_null_ret();
}

/*
 * template class methods
 */
static ici_cfunc_t template_cfuncs[] =
{
    {CF_OBJ, "new",             ici_template_new},
    {CF_OBJ, "parse_string",    ici_template_parse_string},
    {CF_OBJ, "parse_file",      ici_template_parse_file},
    {CF_OBJ, "render",          ici_template_render},
    {CF_OBJ}
};

/*
 * We have no functions (yet) but require an empty "cfuncs" table
 * for the call to ici_module_init().  That could be declared locally
 * in the module init function but defining this makes it easier if
 * we do acquire module-level cfuncs.
 */
static ici_cfunc_t cfuncs[] =
{
    {CF_OBJ}
};

/*
 * Create an ICI class object of the given name and using the given cfuncs (methods)
 * and add it to the given module.
 */
static ici_objwsup_t *
make_class(ici_str_t *name, ici_cfunc_t *cfuncs, ici_objwsup_t *module)
{
    ici_objwsup_t *klass;
    int failed = 1;

    if ((klass = ici_class_new(cfuncs, module)) != NULL)
    {
        failed = ici_assign_base(module, name, klass);
        ici_decref(klass);
    }
    return failed ? NULL : klass;
}

/*
 * Module initialization.
 */
ici_obj_t *
ici_clearsilver_library_init(void)
{
    ici_objwsup_t       *module;
    NEOERR              *err;

    if (ici_interface_check(ICI_VER, ICI_BACK_COMPAT_VER, "clearsilver"))
        return NULL;
    if (init_ici_str())
        return NULL;

    if ((err = nerr_init()) != STATUS_OK)
    {
        set_err(err);
        return NULL;
    }

    if ((module = ici_module_new(cfuncs)) == NULL)
        return NULL;

    if ((ici_hdf_class = make_class(ICIS(hdf), hdf_cfuncs, module)) == NULL)
        goto fail;

    if (make_class(ICIS(template), template_cfuncs, module) == NULL)
        goto fail;

    return objof(module);

fail:
    if (ici_hdf_class)
        ici_decref(ici_hdf_class);
    if (module)
        ici_decref(module);
    return NULL;
}
