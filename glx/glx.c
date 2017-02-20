#include <glx.h>
#include "ici.h"
#include <display.h>
#include <window.h>
#include <visual.h>

typedef struct
{
    object_t	o_head;
    GLXContext	c_context;
}
ici_glx_context_t;

#define	isglxcontext(o)	((o)->o_type == &ici_glx_context_type)
#define	glxcontextof(o)	((ici_glx_context_t *)(o))

static unsigned long
mark_context(object_t *o)
{
    o->o_flags |= O_MARK;
    return sizeof (ici_glx_context_t);
}

static type_t	ici_glx_context_type =
{
    mark_context,
    free_simple,
    hash_unique,
    cmp_unique,
    copy_simple,
    assign_simple,
    fetch_simple,
    "glxcontext"
};

static ici_glx_context_t *
ici_glx_new_context(GLXContext context)
{
    ici_glx_context_t	*c;

    if ((c = talloc(ici_glx_context_t)) != NULL)
    {
	objof(c)->o_type  = &ici_glx_context_type;
	objof(c)->o_tcode = TC_OTHER;
	objof(c)->o_nrefs = 1;
	objof(c)->o_flags = 0;
	rego(objof(c));
	c->c_context = context;
    }
    return c;
}

/*
 * array = glx.QueryExtension(display)
 *
 * Determine if a given X server, represented by the display object
 * returned by X.OpenDisplay(), supports the GLX extension.
 */
FUNC(QueryExtension)
{
    ici_X_display_t	*display;
    array_t		*a;
    int			v1, v2;
    int_t		*i;

    if (ici_typecheck("o", &display))
	return 1;
    if (!isdisplay(objof(display)))
	return ici_argerror(0);
    if (!glXQueryExtension(display->d_display, &v1, &v2))
    {
	error = "no GLX extension";
	return 1;
    }
    if ((a = new_array()) == NULL)
	return 1;
    if (pushcheck(a, 2))
	goto fail;
    if ((i = new_int(v1)) == NULL)
	goto fail;
    *a->a_top++ = objof(i);
    decref(i);
    if ((i = new_int(v2)) == NULL)
	goto fail;
    *a->a_top++ = objof(i);
    decref(i);
    return ici_ret_with_decref(objof(a));

fail:
    decref(a);
    return 1;
}

FUNC(ChooseVisual)
{
    ici_X_display_t	*display;
    long		screen;
    array_t		*a;
    int			*visinfo;
    XVisualInfo		*vis;
    int			i;
    int			nints;
    struct_t		*s;

    if (ici_typecheck("oia", &display, &screen, &a))
	return 1;
    if (!isdisplay(objof(display)))
	return ici_argerror(0);
    nints = a->a_top - a->a_base;
    if ((visinfo = ici_alloc(sizeof (int) * nints)) == NULL)
	return 1;
    for (i = 0; i < nints; ++i)
    {
	if (!isint(a->a_base[i]))
	{
	    ici_free(a);
	    error = "non-integer in visual information spec.";
	    return 1;
	}
	visinfo[i] = intof(a->a_base[i])->i_value;
    }
    visinfo[i] = None;
    vis = glXChooseVisual(display->d_display, screen, visinfo);
    ici_free(visinfo);
    if (vis == NULL)
    {
	error = "no such visual";
	return 1;
    }
    s = ici_X_visualinfo_to_struct(vis);
    XFree(vis);
    return ici_ret_with_decref(objof(s));
}

FUNC(CreateContext)
{
    ici_X_display_t	*display;
    struct_t		*visinfo;
    XVisualInfo		vi;
    ici_glx_context_t	*sharelist;
    long		direct;
    GLXContext		ctx;
    GLXContext		slist;

    if (ici_typecheck("odoi", &display, &visinfo, &sharelist, &direct))
	return 1;
    if (!isdisplay(objof(display)))
	return ici_argerror(0);
    if (isglxcontext(objof(sharelist)))
	slist = sharelist->c_context;
    else if (isnull(objof(sharelist)))
	slist = NULL;
    else
	return ici_argerror(2);
    if (!ici_X_struct_to_visualinfo(visinfo, &vi))
	return 1;
    ctx = glXCreateContext(display->d_display, &vi, slist, direct);
    if (ctx == NULL)
    {
	error = "unable to create GLX context";
	return 1;
    }
    return ici_ret_with_decref(objof(ici_glx_new_context(ctx)));
}

FUNC(MakeCurrent)
{
    ici_X_display_t	*display;
    ici_X_window_t	*window;
    ici_glx_context_t	*context;

    if (ici_typecheck("ooo", &display, &window, &context))
	return 1;
    if (!isdisplay(objof(display)))
	return ici_argerror(0);
    if (!iswindow(objof(window)))
	return ici_argerror(1);
    if (!isglxcontext(objof(context)))
	return ici_argerror(2);
    glXMakeCurrent(display->d_display, window->w_window, context->c_context);
    return null_ret();
}

FUNC(SwapBuffers)
{
    ici_X_display_t	*display;
    ici_X_window_t	*window;

    if (ici_typecheck("oo", &display, &window))
	return 1;
    if (!isdisplay(objof(display)))
	return ici_argerror(0);
    if (!iswindow(objof(window)))
	return ici_argerror(1);
    glXSwapBuffers(display->d_display, window->w_window);
    return null_ret();
}

static struct var
{
    char	*name;
    long	value;
}
vars[] =
{
    {"USE_GL",				GLX_USE_GL},
    {"BUFFER_SIZE",			GLX_BUFFER_SIZE},
    {"LEVEL",				GLX_LEVEL},
    {"RGBA",				GLX_RGBA},
    {"DOUBLEBUFFER",			GLX_DOUBLEBUFFER},
    {"STEREO",				GLX_STEREO},
    {"AUX_BUFFERS",			GLX_AUX_BUFFERS},
    {"RED_SIZE",			GLX_RED_SIZE},
    {"GREEN_SIZE",			GLX_GREEN_SIZE},
    {"BLUE_SIZE",			GLX_BLUE_SIZE},
    {"ALPHA_SIZE",			GLX_ALPHA_SIZE},
    {"DEPTH_SIZE",			GLX_DEPTH_SIZE},
    {"STENCIL_SIZE",			GLX_STENCIL_SIZE},
    {"ACCUM_RED_SIZE",			GLX_ACCUM_RED_SIZE},
    {"ACCUM_GREEN_SIZE",		GLX_ACCUM_GREEN_SIZE},
    {"ACCUM_BLUE_SIZE",			GLX_ACCUM_BLUE_SIZE},
    {"ACCUM_ALPHA_SIZE",		GLX_ACCUM_ALPHA_SIZE},
    {"X_VISUAL_TYPE_EXT",		GLX_X_VISUAL_TYPE_EXT},
    {"TRANSPARENT_TYPE_EXT",		GLX_TRANSPARENT_TYPE_EXT},
    {"TRANSPARENT_INDEX_VALUE_EXT",	GLX_TRANSPARENT_INDEX_VALUE_EXT},
    {"TRANSPARENT_RED_VALUE_EXT",	GLX_TRANSPARENT_RED_VALUE_EXT},
    {"TRANSPARENT_GREEN_VALUE_EXT",	GLX_TRANSPARENT_GREEN_VALUE_EXT},
    {"TRANSPARENT_BLUE_VALUE_EXT",	GLX_TRANSPARENT_BLUE_VALUE_EXT},
    {"TRANSPARENT_ALPHA_VALUE_EXT",	GLX_TRANSPARENT_ALPHA_VALUE_EXT},
    {"BAD_SCREEN",			GLX_BAD_SCREEN},
    {"BAD_ATTRIBUTE",			GLX_BAD_ATTRIBUTE},
    {"NO_EXTENSION",			GLX_NO_EXTENSION},
    {"BAD_VISUAL",			GLX_BAD_VISUAL},
    {"BAD_CONTEXT",			GLX_BAD_CONTEXT},
    {"BAD_VALUE",			GLX_BAD_VALUE},
    {"BAD_ENUM",			GLX_BAD_ENUM},
    {"VENDOR",				GLX_VENDOR},
    {"VERSION",				GLX_VERSION},
    {"EXTENSIONS",			GLX_EXTENSIONS},
    {"TRUE_COLOR_EXT",			GLX_TRUE_COLOR_EXT},
    {"DIRECT_COLOR_EXT",		GLX_DIRECT_COLOR_EXT},
    {"PSEUDO_COLOR_EXT",		GLX_PSEUDO_COLOR_EXT},
    {"STATIC_COLOR_EXT",		GLX_STATIC_COLOR_EXT},
    {"GRAY_SCALE_EXT",			GLX_GRAY_SCALE_EXT},
    {"STATIC_GRAY_EXT",			GLX_STATIC_GRAY_EXT},
    {"NONE_EXT",			GLX_NONE_EXT},
    {"TRANSPARENT_RGB_EXT",		GLX_TRANSPARENT_RGB_EXT},
    {"TRANSPARENT_INDEX_EXT",		GLX_TRANSPARENT_INDEX_EXT}
};

int
ici_glx_vars_init(struct_t *s)
{
    struct var	*v;

    for (v = vars; v - vars < nels(vars); ++v)
	if (ici_cmkvar(s, v->name, 'i', &v->value))
	    return 1;
    return 0;
}
