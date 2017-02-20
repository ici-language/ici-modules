/* 
 * $Id: gtk.c,v 1.36 2001/01/21 01:29:18 atrn Exp $
 *
 * gtk interface for ici.
 * Andy Newman <atrn@zeta.org.au>
 */

#include "ici.h"

#include <gtk/gtk.h>
#include <gtk/gtkenums.h>

/*
 * Gtk widgets are ici objects.  We keep the pointer to the widget
 * (of course) and a structure recording the widget's signal connections.
 * This is because of the mechanism used to deliver signals to ici
 * functions, explained below.
 *
 * The signals struct is keyed by signal names (strings) and maps each
 * to a struct recording the widget, handler function and user-defined
 * parameter to the handler function, the keys are "widget", "fn" and
 * "arg".  When the signal is emitted by the widget a C function is
 * called with the struct as an argument.  We lookup the function and
 * call it passing the widget, event and user-defined argument.
 */
typedef struct
{
    object_t	o_head;
    GtkWidget	*g_widget;
    struct_t	*g_signals;
}
widget_t;

static unsigned long	mark_widget(object_t *);

static type_t	widget_type =
{
    mark_widget,
    free_simple,
    hash_unique,
    cmp_unique,
    copy_simple,
    assign_simple,
    fetch_simple,
    "widget"
};

/* Be modern, use inline functions, use more newlines :) */

inline
static widget_t *
widgetof(void *o)
{
    return (widget_t *)o;
}

inline
static int
iswidget(object_t *o)
{
    return o->o_type == &widget_type;
}

static unsigned long
mark_widget(object_t *o)
{
    o->o_flags |= O_MARK;
    return sizeof (widget_t) + mark(widgetof(o)->g_signals);
}

static widget_t *
new_widget(GtkWidget *widget)
{
    widget_t	*w;

    if (widget == NULL)
	return NULL;
    if ((w = talloc(widget_t)) != NULL)
    {
	objof(w)->o_flags = 0;
	objof(w)->o_nrefs = 1;
	objof(w)->o_tcode = TC_OTHER;
	objof(w)->o_type  = &widget_type;
	if ((w->g_signals = new_struct()) == NULL)
	{
	    decref(w);
	    return NULL;
	}
	w->g_widget = widget;
	rego(w);
    }
    return w;
}

FUNC(widget_from_pointer)
{
    long	addr;

    if (ici_typecheck("i", &addr))
	return 1;
    return ici_ret_with_decref(objof(new_widget((GtkWidget *)addr)));
}


#define	handle_type_with_fetch_assign(NAME, FETCH, ASSIGN)\
\
typedef struct\
{\
    object_t	o_head;\
    void	*g_ptr;\
}\
NAME ## _t;\
\
static unsigned long	mark_ ## NAME (object_t *);\
\
static type_t	NAME ## _type =\
{\
    mark_ ## NAME,\
    free_simple,\
    hash_unique,\
    cmp_unique,\
    copy_simple,\
    ASSIGN ,\
    FETCH ,\
    # NAME \
};\
\
static unsigned long \
mark_ ## NAME (object_t *o)\
{\
    o->o_flags |= O_MARK;\
    return sizeof (NAME ## _t);\
}\
\
inline \
static int \
is ## NAME (object_t *o)\
{\
    return o->o_type == &NAME ## _type;\
}\
\
inline \
static NAME ## _t *\
NAME ## of(void *o)\
{\
    return (NAME ## _t *)o;\
}\
\
static NAME ##_t *\
new_ ## NAME(void *ptr)\
{\
    NAME ##_t	*a;\
\
    if (ptr == NULL)\
	a = NULL;\
    else if ((a = talloc(NAME ## _t)) != NULL)\
    {\
	objof(a)->o_tcode = TC_OTHER;\
	objof(a)->o_nrefs = 1;\
	objof(a)->o_flags = 0;\
	objof(a)->o_type  = & NAME ## _type;\
	rego(a);\
	a->g_ptr = ptr;\
    }\
    return a;\
}

#define	handle_type_with_fetch(NAME, FETCH)\
	handle_type_with_fetch_assign(NAME, FETCH, assign_simple)

#define	handle_type(NAME)\
	handle_type_with_fetch(NAME, fetch_simple)


/* Adjustment */

static object_t	*fetch_adjustment(object_t *, object_t *);
static int	assign_adjustment(object_t *, object_t *, object_t *);

handle_type_with_fetch_assign(adjustment, fetch_adjustment, assign_adjustment)

NEED_STRING(lower);
NEED_STRING(upper);
NEED_STRING(value);
NEED_STRING(step_increment);
NEED_STRING(page_increment);
NEED_STRING(page_size);

static object_t *
fetch_adjustment(object_t *o, object_t *key)
{
    NEED_STRINGS(NULL);
    if (key == objof(STRING(lower)))
	return objof(new_float(GTK_ADJUSTMENT(adjustmentof(o)->g_ptr)->lower));
    if (key == objof(STRING(upper)))
	return objof(new_float(GTK_ADJUSTMENT(adjustmentof(o)->g_ptr)->upper));
    if (key == objof(STRING(value)))
	return objof(new_float(GTK_ADJUSTMENT(adjustmentof(o)->g_ptr)->value));
    if (key == objof(STRING(step_increment)))
	return objof(new_float(GTK_ADJUSTMENT(adjustmentof(o)->g_ptr)->step_increment));
    if (key == objof(STRING(page_increment)))
	return objof(new_float(GTK_ADJUSTMENT(adjustmentof(o)->g_ptr)->page_increment));
    if (key == objof(STRING(page_size)))
	return objof(new_float(GTK_ADJUSTMENT(adjustmentof(o)->g_ptr)->page_size));
    return fetch_simple(o, key);
}

static int
assign_adjustment(object_t *o, object_t *key, object_t *value)
{
    double	v;

    NEED_STRINGS(0);
    if (isint(value))
	v = intof(value)->i_value;
    else if (isfloat(value))
	v = floatof(value)->f_value;
    else
    {
	error = "attempt to assign non-number to adjustment field";
	return 1;
    }
    if (key == objof(STRING(lower)))
	GTK_ADJUSTMENT(adjustmentof(o)->g_ptr)->lower = v;
    else if (key == objof(STRING(upper)))
	GTK_ADJUSTMENT(adjustmentof(o)->g_ptr)->upper = v;
    else if (key == objof(STRING(value)))
	GTK_ADJUSTMENT(adjustmentof(o)->g_ptr)->value = v;
    else if (key == objof(STRING(step_increment)))
	GTK_ADJUSTMENT(adjustmentof(o)->g_ptr)->step_increment = v;
    else if (key == objof(STRING(page_increment)))
	GTK_ADJUSTMENT(adjustmentof(o)->g_ptr)->page_increment = v;
    else if (key == objof(STRING(page_size)))
	GTK_ADJUSTMENT(adjustmentof(o)->g_ptr)->page_size = v;
    else
	return assign_simple(o, key, value);
    return 0;
}

FUNC(adjustment_new)
{
    double	val, lwr, upr, step, page, pagesz;

    if (ici_typecheck("nnnnnn", &val, &lwr, &upr, &step, &page, &pagesz))
	return 1;
    return ici_ret_with_decref(objof(new_adjustment(gtk_adjustment_new(val, lwr, upr, step, page, pagesz))));
}

FUNC(adjustment_changed)
{
    adjustment_t	*adj;

    if (ici_typecheck("o", &adj))
	return 1;
    if (!isadjustment(objof(adj)))
	return ici_argerror(0);
    gtk_adjustment_changed(GTK_ADJUSTMENT(adj->g_ptr));
    return null_ret();
}

FUNC(adjustment_value_changed)
{
    adjustment_t	*adj;

    if (ici_typecheck("o", &adj))
	return 1;
    if (!isadjustment(objof(adj)))
	return ici_argerror(0);
    gtk_adjustment_value_changed(GTK_ADJUSTMENT(adj->g_ptr));
    return null_ret();
}

FUNC(adjustment_set_value)
{
    adjustment_t	*adj;
    double		val;

    if (ici_typecheck("on", &adj, &val))
	return 1;
    if (!isadjustment(objof(adj)))
	return ici_argerror(0);
    gtk_adjustment_set_value(GTK_ADJUSTMENT(adj->g_ptr), val);
    return null_ret();
}

FUNC(adjustment_clamp_page)
{
    adjustment_t	*adj;
    double		lwr;
    double		upr;

    if (ici_typecheck("onn", &adj, &lwr, &upr))
	return 1;
    if (!isadjustment(objof(adj)))
	return ici_argerror(0);
    gtk_adjustment_clamp_page(GTK_ADJUSTMENT(adj->g_ptr), lwr, upr);
    return null_ret();
}

/* Font */

handle_type(font)

/* Color */
handle_type(color)

/* Style */

static object_t		*fetch_style(object_t *, object_t *);
static int		assign_style(object_t *, object_t *, object_t *);

handle_type_with_fetch_assign(style, fetch_style, assign_style)

NEED_STRING(font);
     
static object_t *
fetch_style(object_t *o, object_t *key)
{
    NEED_STRINGS(NULL);
    if (key == objof(STRING(font)))
	return objof(new_font(((GtkStyle *)styleof(o)->g_ptr)->font));
    return fetch_simple(o, key);
}

static int
assign_style(object_t *o, object_t *key, object_t *value)
{
    NEED_STRINGS(0);
    if (key == objof(STRING(font)))
    {
	if (!isfont(value))
	    goto fail;
	gdk_font_ref(fontof(value)->g_ptr);
	((GtkStyle *)styleof(o)->g_ptr)->font = fontof(value)->g_ptr;
	return 0;
    }

fail:
    return assign_simple(o, key, value);
}

FUNC(style_new)
{
    return ici_ret_with_decref(objof(new_style(gtk_style_new())));
}

FUNC(style_copy)
{
    style_t	*style;

    if (ici_typecheck("o", &style))
	return 1;
    if (!isstyle(objof(style)))
	return ici_argerror(0);
    return ici_ret_with_decref(objof(new_style(gtk_style_copy(style->g_ptr))));
}


/* Misc strings. */

NEED_STRING(row);
NEED_STRING(col);
NEED_STRING(rc);
NEED_STRING(widget);
NEED_STRING(fn);
NEED_STRING(arg);
NEED_STRING(argv);
NEED_STRING(expand);
NEED_STRING(fill);
NEED_STRING(padding);
NEED_STRING(packtype);
NEED_STRING(type);
NEED_STRING(window);
NEED_STRING(send_event);

/*
 * Turn an ICI array of strings into an argv-style array. The memory for
 * the argv-style array is allocated (via ici_raw_alloc()) and must be freed
 * after use. The memory is allocated as one block so a single zfree()
 * of the function result is all that's required.
 */
static char **
ici_array_to_argv(array_t *a, int *pargc)
{
    char	**argv;
    int		argc;
    size_t	sz;
    int		n;
    char	*p;
    object_t	**po;

    argc = a->a_top - a->a_base;
    sz = (argc + 1) * sizeof (char *);
    for (n = argc, po = a->a_base; n; --n, ++po)
    {
	if (!isstring(*po))
	{
	    error = "non-string in argv array";
	    return NULL;
	}
	sz += stringof(*po)->s_nchars + 1;
    }
    if ((argv = (char **)ici_raw_alloc(sz)) == NULL)
	return NULL;
    p = (char *)&argv[argc];
    for (n = argc, po = a->a_base; n > 0; --n, ++po)
    {
	argv[argc - n] = p;
	memcpy(p, stringof(*po)->s_chars, stringof(*po)->s_nchars + 1);
	p += stringof(*po)->s_nchars + 1;
    }
    argv[argc] = NULL;
    *pargc = argc;
    return argv;
}

/*
 * "Glue" functions.
 *
 *	glue__W		No args, return widget.
 *	glue_s_W	String arg, return widget.
 *	glue_i_n	Int arg, return NULL.
 *	glue__i		No args, return int.
 *	glue_s_n	String arg, return NULL.
 *	glue_w_n	Widget arg, return NULL.
 *	glue_w_i	Widget arg, return int.
 *	glue_ww_n	Two widget args, return NULL.
 *	glue_wi_n	Widget & int args, return NULL.
 *	glue_wii_n	Widget & two int args, return NULL.
 *	glue_wff_n	Widget & two float args, return NULL.
 *	glue_ws_n	Widget & string arg, return NULL.
 */

FUNCDEF(glue__W)
{
    return ici_ret_with_decref(objof(new_widget((GtkWidget *)(*CF_ARG1())())));
}

FUNCDEF(glue_s_W)
{
    char	*str;

    if (ici_typecheck("s", &str))
	return 1;
    return ici_ret_with_decref(objof(new_widget((GtkWidget *)(*CF_ARG1())(str))));
}

FUNCDEF(glue_i_n)
{
    long	v;

    if (ici_typecheck("i", &v))
	return 1;
    (*CF_ARG1())(v);
    return null_ret();
}

FUNCDEF(glue__i)
{
    return int_ret((*CF_ARG1())());
}

FUNCDEF(glue_s_n)
{
    char	*str;

    if (ici_typecheck("s", &str))
	return 1;
    (*CF_ARG1())(str);
    return null_ret();
}

FUNCDEF(glue_w_n)
{
    widget_t	*widget;

    if (ici_typecheck("o", &widget))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    (*CF_ARG1())(widget->g_widget);
    return null_ret();
}

FUNCDEF(glue_w_i)
{
    widget_t	*widget;

    if (ici_typecheck("o", &widget))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    return int_ret((*CF_ARG1())(widget->g_widget));
}

FUNCDEF(glue_ww_n)
{
    widget_t	*w1;
    widget_t	*w2;

    if (ici_typecheck("oo", &w1, &w2))
	return 1;
    if (!iswidget(objof(w1)))
	return ici_argerror(0);
    if (!iswidget(objof(w2)))
	return ici_argerror(1);
    (*CF_ARG1())(w1->g_widget, w2->g_widget);
    return null_ret();
}

FUNCDEF(glue_wi_n)
{
    widget_t	*widget;
    long	v;

    if (ici_typecheck("oi", &widget, &v))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    (*CF_ARG1())(widget->g_widget, v);
    return null_ret();
}

FUNCDEF(glue_wii_n)
{
    widget_t	*widget;
    long	v1;
    long	v2;

    if (ici_typecheck("oii", &widget, &v1, &v2))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    (*CF_ARG1())(widget->g_widget, v1, v2);
    return null_ret();
}

FUNCDEF(glue_wff_n)
{
    widget_t	*widget;
    double	v1;
    double	v2;

    if (ici_typecheck("onn", &widget, &v1, &v2))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    (*CF_ARG1())(widget->g_widget, v1, v2);
    return null_ret();
}

FUNCDEF(glue_ws_n)
{
    widget_t	*widget;
    char	*s;

    if (ici_typecheck("os", &widget, &s))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    (*CF_ARG1())(widget->g_widget, s);
    return null_ret();
}

/* Misc. */

FUNC(init)
{
    array_t	*a;
    int		argc;
    char	**argv;
    char	*defargv[] = {"ici", NULL};

    NEED_STRINGS(1);

    if (NARGS() > 0)
    {
	if (ici_typecheck("a", &a))
	    return 1;
    }
    else
    {
	a = arrayof(fetch(v_top[-1], STRING(argv)));
	if (a == NULL)
	    return 1;
	if (objof(a) == objof(&o_null))
	{
	    argv = defargv;
	    argc = 1;
	}
    }
    if (argv != defargv)
	argv = ici_array_to_argv(a, &argc);
    if (argv == NULL)
	return 1;
    gtk_init(&argc, &argv);
    if (argv != defargv)
	ici_raw_free(argv);
    return null_ret();
}

static int	signal_handler_errored = 0;

FUNC(main)
{
    gtk_main();
    if (signal_handler_errored)
    {
	signal_handler_errored = 0;
	return 1;
    }
    return null_ret();
}

FUNC(main_quit)
{
    gtk_main_quit();
    return null_ret();
}

FUNC(rc_init)
{
    gtk_rc_init();
    return null_ret();
}

CFUNC1(rc_parse, glue_s_n, gtk_rc_parse);
CFUNC1(rc_parse_string, glue_s_n, gtk_rc_parse_string);

FUNC(gdk_font_load)
{
    char	*name;

    if (ici_typecheck("s", &name))
	return 1;
    return ici_ret_with_decref(objof(new_font(gdk_font_load(name))));
}

CFUNC1(events_pending, glue__i, gtk_events_pending);
CFUNC1(main_level, glue__i, gtk_main_level);
CFUNC1(main_iteration, glue__i, gtk_main_iteration);

FUNC(main_iteration_do)
{
    long	blocking;

    if (ici_typecheck("i", &blocking))
	return 1;
    return int_ret(gtk_main_iteration_do(blocking));
}

CFUNC1(true, glue__i, gtk_true);
CFUNC1(false, glue__i, gtk_false);
CFUNC1(grab_add, glue_w_n, gtk_grab_add);
CFUNC1(grab_remove, glue_w_n, gtk_grab_remove);
CFUNC1(grab_get_current, glue__W, gtk_grab_get_current);

static int
invoke_callback(void *data)
{
    object_t	*fn;
    object_t	*arg;

    fn = fetch(structof(data), STRING(fn));
    arg = fetch(structof(data), STRING(arg));
    if (ici_func(fn, "o", arg) != NULL)
    {
	signal_handler_errored = 1;
	gtk_main_quit();
	return 0;
    }
    return 1;
}

FUNC(init_add)
{
    func_t	*fn;
    object_t	*arg;
    struct_t	*d;

    NEED_STRINGS(1);
    if (NARGS() > 1)
    {
	if (ici_typecheck("oo", &fn, &arg))
	    return 1;
    }
    else
    {
	arg = objof(&o_null);
	if (ici_typecheck("o", &fn))
	    return 1;
    }
    if (!isfunc(objof(fn)) && !iscfunc(objof(fn)) && !ismethod(objof(fn)))
	return ici_argerror(0);
    if ((d = new_struct()) == NULL)
	return 1;
    if (assign(d, STRING(fn), fn))
    {
	decref(d);
	return 1;
    }
    if (assign(d, STRING(arg), arg))
    {
	decref(d);
	return 1;
    }
    gtk_init_add(invoke_callback, d);
    return null_ret();
}

FUNC(quit_add)
{
    func_t	*fn;
    object_t	*arg;
    long	level;
    struct_t	*d;

    NEED_STRINGS(1);
    if (NARGS() > 2)
    {
	if (ici_typecheck("ioo", &level, &fn, &arg))
	    return 1;
    }
    else
    {
	arg = objof(&o_null);
	if (ici_typecheck("io", &level, &fn))
	    return 1;
    }
    if (!isfunc(objof(fn)) && !iscfunc(objof(fn)) && !ismethod(objof(fn)))
	return ici_argerror(0);
    if ((d = new_struct()) == NULL)
	return 1;
    if (assign(d, STRING(fn), fn))
    {
	decref(d);
	return 1;
    }
    if (assign(d, STRING(arg), arg))
    {
	decref(d);
	return 1;
    }
    return int_ret(gtk_quit_add(level, invoke_callback, d));
}

CFUNC1(quit_remove, glue_i_n, gtk_quit_remove);

FUNC(timeout_add)
{
    func_t	*fn;
    object_t	*arg;
    long	timeout;
    struct_t	*d;

    NEED_STRINGS(1);
    if (NARGS() > 2)
    {
	if (ici_typecheck("ioo", &timeout, &fn, &arg))
	    return 1;
    }
    else
    {
	arg = objof(&o_null);
	if (ici_typecheck("io", &timeout, &fn))
	    return 1;
    }
    if (!isfunc(objof(fn)) && !iscfunc(objof(fn)) && !ismethod(objof(fn)))
	return ici_argerror(0);
    if ((d = new_struct()) == NULL)
	return 1;
    if (assign(d, STRING(fn), fn))
    {
	decref(d);
	return 1;
    }
    if (assign(d, STRING(arg), arg))
    {
	decref(d);
	return 1;
    }
    return int_ret(gtk_timeout_add(timeout, invoke_callback, d));
}

CFUNC1(timeout_remove, glue_i_n, gtk_timeout_remove);

FUNC(idle_add)
{
    func_t	*fn;
    object_t	*arg;
    struct_t	*d;

    NEED_STRINGS(1);
    if (NARGS() > 1)
    {
	if (ici_typecheck("oo", &fn, &arg))
	    return 1;
    }
    else
    {
	arg = objof(&o_null);
	if (ici_typecheck("o", &fn))
	    return 1;
    }
    if (!isfunc(objof(fn)) && !iscfunc(objof(fn)) && !ismethod(objof(fn)))
	return ici_argerror(0);
    if ((d = new_struct()) == NULL)
	return 1;
    if (assign(d, STRING(fn), fn))
    {
	decref(d);
	return 1;
    }
    if (assign(d, STRING(arg), arg))
    {
	decref(d);
	return 1;
    }
    return int_ret(gtk_idle_add(invoke_callback, d));
}

CFUNC1(idle_remove, glue_i_n, gtk_idle_remove);

static void
invoke_input_callback(void *data, gint fd, GdkInputCondition cond)
{
    object_t	*fn;
    object_t	*arg;

    fn = fetch(structof(data), STRING(fn));
    arg = fetch(structof(data), STRING(arg));
    if (ici_func(fn, "oii", arg, (long)fd, (long)cond) != NULL)
    {
	signal_handler_errored = 1;
	gtk_main_quit();
    }
}

FUNC(input_add_full)
{
    long	fd;
    long	cond;
    object_t	*fn;
    object_t	*arg;
    struct_t	*d;

    NEED_STRINGS(1);
    if (NARGS() > 3)
    {
	if (ici_typecheck("iioo", &fd, &cond, &fn, &arg))
	    return 1;
    }
    else if (NARGS() > 2)
    {
	arg = objof(&o_null);
	if (ici_typecheck("iio", &fd, &cond, &fn))
	{
	    cond = GDK_INPUT_READ;
	    if (ici_typecheck("ioo", &fd, &fn, &arg))
		return 1;
	}
    }
    else
    {
	arg = objof(&o_null);
	cond = GDK_INPUT_READ;
	if (ici_typecheck("io", &fd, &fn))
	    return 1;
    }
    if (!isfunc(objof(fn)) && !iscfunc(objof(fn)) && !ismethod(objof(fn)))
	return ici_argerror(0);
    if ((d = new_struct()) == NULL)
	return 1;
    if (assign(d, STRING(fn), fn))
    {
	decref(d);
	return 1;
    }
    if (assign(d, STRING(arg), arg))
    {
	decref(d);
	return 1;
    }
    return int_ret(gtk_input_add_full(fd, cond, invoke_input_callback, NULL, d, NULL));
}

CFUNC1(input_remove, glue_i_n, gtk_input_remove);

/* Widget */

CFUNC1(widget_show, glue_w_n, gtk_widget_show);
CFUNC1(widget_destroy, glue_w_n, gtk_widget_destroy);
CFUNC1(widget_set_name, glue_ws_n, gtk_widget_set_name);
CFUNC1(widget_set_sensitive, glue_wi_n, gtk_widget_set_sensitive);
CFUNC1(widget_show_all, glue_w_n, gtk_widget_show_all);
CFUNC1(widget_hide, glue_w_n, gtk_widget_hide);
CFUNC1(widget_hide_all, glue_w_n, gtk_widget_hide_all);
CFUNC1(widget_realize, glue_w_n, gtk_widget_realize);
CFUNC1(widget_unparent, glue_w_n, gtk_widget_unparent);
CFUNC1(widget_reparent, glue_ww_n, gtk_widget_reparent);
CFUNC1(widget_set_usize, glue_wii_n, gtk_widget_set_usize);

FUNC(widget_get_style)
{
    widget_t	*widget;

    if (ici_typecheck("o", &widget))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    return ici_ret_with_decref(objof(new_style(gtk_widget_get_style(GTK_WIDGET(widget->g_widget)))));
}

FUNC(widget_set_style)
{
    widget_t	*widget;
    style_t	*style;

    if (ici_typecheck("oo", &widget, &style))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    if (!isstyle(objof(style)))
	return ici_argerror(1);
    gtk_widget_set_style(GTK_WIDGET(widget->g_widget), (GtkStyle *)style->g_ptr);
    return null_ret();
}

CFUNC1(widget_set_rc_style, glue_w_n, gtk_widget_set_rc_style);
CFUNC1(widget_ensure_style, glue_w_n, gtk_widget_ensure_style);

/* Alignment */

FUNC(alignment_new)
{
    double	xalign, yalign, xscale, yscale;

    if (ici_typecheck("nnnn", &xalign, &yalign, &xscale, &yscale))
	return 1;
    return ici_ret_with_decref(objof(new_widget(gtk_alignment_new(xalign, yalign, xscale, yscale))));
}

FUNC(alignment_set)
{
    widget_t	*widget;
    double	xalign, yalign, xscale, yscale;

    if (ici_typecheck("onnnn", &widget, &xalign, &yalign, &xscale, &yscale))
	return 1;
    gtk_alignment_set(GTK_ALIGNMENT(widget->g_widget), xalign, yalign, xscale, yscale);
    return null_ret();
}

/* Misc. */

CFUNC1(misc_set_alignment, glue_wff_n, gtk_misc_set_alignment);
CFUNC1(misc_set_padding, glue_wii_n, gtk_misc_set_padding);

/* Label */

CFUNC1(label_new, glue_s_W, gtk_label_new);
CFUNC1(label_set_text, glue_ws_n, gtk_label_set_text);
CFUNC1(label_set_justify, glue_wi_n, gtk_label_set_justify);
CFUNC1(label_set_pattern, glue_ws_n, gtk_label_set_pattern);
CFUNC1(label_set_line_wrap, glue_wi_n, gtk_label_set_line_wrap);

/* Button */

CFUNC1(button_new, glue__W, gtk_button_new);
CFUNC1(button_new_with_label, glue_s_W, gtk_button_new_with_label);
CFUNC1(button_get_type, glue__i, gtk_button_get_type);
CFUNC1(button_pressed, glue_w_n, gtk_button_pressed);
CFUNC1(button_released, glue_w_n, gtk_button_released);
CFUNC1(button_clicked, glue_w_n, gtk_button_clicked);
CFUNC1(button_enter, glue_w_n, gtk_button_enter);
CFUNC1(button_leave, glue_w_n, gtk_button_leave);

FUNC(button_child)
{
    widget_t	*button;

    if (ici_typecheck("o", &button))
	return 1;
    if (!iswidget(objof(button)))
	return ici_argerror(0);
    return ici_ret_with_decref(objof(new_widget(GTK_BIN(button->g_widget)->child)));
}

/* Button box */

CFUNC1(button_box_set_layout, glue_wi_n, gtk_button_box_set_layout);
CFUNC1(button_box_set_child_size, glue_wii_n, gtk_button_box_set_layout);
CFUNC1(button_box_set_child_ipadding, glue_wii_n, gtk_button_box_set_layout);

/* Horizontal button box */

CFUNC1(hbutton_box_new, glue__W, gtk_hbutton_box_new);
CFUNC1(hbutton_box_set_spacing_default, glue_i_n, gtk_hbutton_box_set_spacing_default);
CFUNC1(hbutton_box_set_layout_default, glue_i_n, gtk_hbutton_box_set_spacing_default);
CFUNC1(hbutton_box_get_spacing_default, glue__i, gtk_hbutton_box_get_spacing_default);
CFUNC1(hbutton_box_get_layout_default, glue__i, gtk_hbutton_box_get_layout_default);

/* Horizontally paned widget */

CFUNC1(hpaned_new, glue__W, gtk_hpaned_new);

/* Vertically paned widget */

CFUNC1(vpaned_new, glue__W, gtk_vpaned_new);

/* Paned widget */

CFUNC1(paned_add1, glue_ww_n, gtk_paned_add1);
CFUNC1(paned_add2, glue_ww_n, gtk_paned_add2);

FUNC(paned_pack1)
{
    widget_t	*widget;
    widget_t	*child;
    long	resize;
    long	shrink;

    if (ici_typecheck("ooii", &widget, &child, &resize, &shrink))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    if (!iswidget(objof(child)))
	return ici_argerror(1);
    gtk_paned_pack1(GTK_PANED(widget->g_widget), child->g_widget, resize, shrink);
    return null_ret();
}

FUNC(paned_pack2)
{
    widget_t	*widget;
    widget_t	*child;
    long	resize;
    long	shrink;

    if (ici_typecheck("ooii", &widget, &child, &resize, &shrink))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    if (!iswidget(objof(child)))
	return ici_argerror(1);
    gtk_paned_pack2(GTK_PANED(widget->g_widget), child->g_widget, resize, shrink);
    return null_ret();
}

CFUNC1(paned_set_position, glue_wi_n, gtk_paned_set_position);
CFUNC1(paned_set_handle_size, glue_wi_n, gtk_paned_set_handle_size);
CFUNC1(paned_set_gutter_size, glue_wi_n, gtk_paned_set_gutter_size);

/* Horizontal ruler widget */

CFUNC1(hruler_new, glue__W, gtk_hruler_new);

/* Horizontal scale widget */

FUNC(hscale_new)
{
    adjustment_t	*adj;

    if (NARGS() > 0)
    {
	if (ici_typecheck("o", &adj))
	    return 1;
	if (objof(adj) == objof(&o_null))
	    adj = NULL;
	else if (!isadjustment(objof(adj)))
	    return ici_argerror(0);
    }
    else
	adj = NULL;
    return ici_ret_with_decref
    (
	objof
	(
	    new_widget
	    (
		gtk_hscale_new
		(
		    adj ? GTK_ADJUSTMENT(adj->g_ptr) : NULL
		)
	    )
	)
    );
}

/* Vertical scrollbar */

FUNC(vscrollbar_new)
{
    adjustment_t	*a;

    if (NARGS() > 0)
    {
	if (ici_typecheck("o", &a))
	    return 1;
	if (objof(a) == objof(&o_null))
	    a = NULL;
	else if (!isadjustment(objof(a)))
	    return ici_argerror(0);
    }
    else
	a = NULL;
    return ici_ret_with_decref
    (
	objof
	(
	    new_widget
	    (
		gtk_vscrollbar_new
		(
		    a ? GTK_ADJUSTMENT(a->g_ptr) : NULL
		)
	    )
	)
    );
}

/* Horizontal scrollbar */

FUNC(hscrollbar_new)
{
    adjustment_t	*a;

    if (NARGS() > 0)
    {
	if (ici_typecheck("o", &a))
	    return 1;
	if (objof(a) == objof(&o_null))
	    a = NULL;
	else if (!isadjustment(objof(a)))
	    return ici_argerror(0);
    }
    else
	a = NULL;
    return ici_ret_with_decref
    (
	objof
	(
	    new_widget
	    (
		gtk_hscrollbar_new
		(
		    a ? GTK_ADJUSTMENT(a->g_ptr) : NULL
		)
	    )
	)
    );
}

/* CheckButton */

CFUNC1(check_button_get_type, glue__i, gtk_check_button_get_type);
CFUNC1(check_button_new, glue__W, gtk_check_button_new);
CFUNC1(check_button_new_with_label, glue_s_W, gtk_check_button_new_with_label);

/* Window */

FUNC(window_new)
{
    int		wintype;
    long	val;

    if (NARGS() == 0)
	wintype = GTK_WINDOW_TOPLEVEL;
    else
    {
	if (ici_typecheck("i", &val))
	    return 1;
	switch (val)
	{
	case GTK_WINDOW_TOPLEVEL:
	case GTK_WINDOW_DIALOG:
	case GTK_WINDOW_POPUP:
	    wintype = val;
	    break;

	default:
	    error = "bad window type";
	    return 1;
	}
    }
    return ici_ret_with_decref(objof(new_widget(gtk_window_new((GtkWindowType)wintype))));
}

CFUNC1(window_set_title, glue_ws_n, gtk_window_set_title);
CFUNC1(window_set_focus, glue_ww_n, gtk_window_set_focus);
CFUNC1(window_set_default, glue_ww_n, gtk_window_set_default);
CFUNC1(window_set_modal, glue_wi_n, gtk_window_set_modal);

FUNC(window_set_policy)
{
    widget_t	*window;
    long	shrink, grow, autoshrink;

    if (ici_typecheck("oiii", &window, &shrink, &grow, &autoshrink))
	return 1;
    if (!iswidget(objof(window)))
	return ici_argerror(0);
    gtk_window_set_policy(GTK_WINDOW(window->g_widget), shrink, grow, autoshrink);
    return null_ret();
}

CFUNC1(window_set_position, glue_wi_n, gtk_window_set_position);
CFUNC1(window_set_default_size, glue_wii_n, gtk_window_set_default_size);

/* Containers */

CFUNC1(container_set_border_width, glue_wi_n, gtk_container_set_border_width);
CFUNC1(container_add, glue_ww_n, gtk_container_add);
CFUNC1(container_remove, glue_ww_n, gtk_container_remove);
CFUNC1(container_check_resize, glue_w_n, gtk_container_check_resize);

/* Boxes */

FUNC(hbox_new)
{
    long	homo, spacing;

    if (ici_typecheck("ii", &homo, &spacing))
	return 1;
    return ici_ret_with_decref(objof(new_widget(gtk_hbox_new(homo, spacing))));
}

FUNC(vbox_new)
{
    long	homo, spacing;

    if (ici_typecheck("ii", &homo, &spacing))
	return 1;
    return ici_ret_with_decref(objof(new_widget(gtk_vbox_new(homo, spacing))));
}

FUNC(box_pack_start)
{
    widget_t	*box;
    widget_t	*widget;
    long	expand, fill, pad;

    if (ici_typecheck("ooiii", &box, &widget, &expand, &fill, &pad))
	return 1;
    if (!iswidget(objof(box)))
	return ici_argerror(0);
    gtk_box_pack_start
    (
	GTK_BOX(box->g_widget),
	widget->g_widget,
	expand,
	fill,
	pad
    );
    return null_ret();
}

FUNC(box_pack_end)
{
    widget_t	*box;
    widget_t	*widget;
    long	expand, fill, pad;

    if (ici_typecheck("ooiii", &box, &widget, &expand, &fill, &pad))
	return 1;
    if (!iswidget(objof(box)))
	return ici_argerror(0);
    gtk_box_pack_end
    (
	GTK_BOX(box->g_widget),
	widget->g_widget,
	expand,
	fill,
	pad
    );
    return null_ret();
}

CFUNC1(box_set_homogeneous, glue_wi_n, gtk_box_set_homogeneous);
CFUNC1(box_set_spacing, glue_wi_n, gtk_box_set_spacing);

FUNC(box_reorder_child)
{
    widget_t	*box;
    widget_t	*child;
    long	pos;

    if (ici_typecheck("ooi", &box, &child, &pos))
	return 1;
    if (!iswidget(objof(box)))
	return ici_argerror(0);
    gtk_box_reorder_child(GTK_BOX(box->g_widget), child->g_widget, pos);
    return null_ret();
}

/* TODO: box_query_child_packing */

FUNC(box_set_child_packing)
{
    widget_t	*box;
    widget_t	*child;
    long	expand;
    long	fill;
    long	padding;
    long	packtype;

    if (ici_typecheck("ooiiii", &box, &child, &expand, &fill, &padding, &packtype))
	return 1;
    if (!iswidget(objof(box)))
	return ici_argerror(0);
    if (!iswidget(objof(child)))
	return ici_argerror(1);
    gtk_box_set_child_packing
    (
	GTK_BOX(box->g_widget), child->g_widget,
	expand, fill, padding,
	packtype
    );
    return null_ret();
}


/* Events */

static char *
event_name[] =
{
    "delete",
    "destroy",
    "expose",
    "motion_notify",
    "button_press",
    "2button_press",
    "3button_press",
    "button_release",
    "key_press",
    "key_release",
    "enter_notify",
    "leave_notify",
    "focus_change",
    "configure",
    "map",
    "unmap",
    "property_notify",
    "selection_clear",
    "selection_request",
    "selection_notify",
    "proximity_in",
    "proximity_out",
    "drag_enter",
    "drag_leave",
    "drag_motion",
    "drag_status",
    "drop_start",
    "drop_finished",
    "client_event",
    "visibility_notify",
    "no_expose"
};

static struct_t *
event_to_struct(GdkEvent *event)
{
    struct_t	*e;
    object_t	*o;

    if ((e = new_struct()) == NULL)
	return NULL;
    if (event->type >= GDK_DELETE && event->type <= GDK_NO_EXPOSE)
    {
	if ((o = objof(new_cname(event_name[event->type]))) == NULL)
	    goto fail;
	if (assign(e, STRING(type), o))
	    goto fail1;
	decref(o);
    }
    return e;

fail1:
    decref(o);
fail:
    decref(e);
    return NULL;
}

/* Signals */

static GtkObject	*lasthooked;
static int		lastsigid;
static GtkArg		*lastparam;
static int		lastnparam;

/* 
 * Signal emission hook.  We hook into each signal emission
 * that is connected so we can determine the number and types
 * of arguments passed to the callback function.  There is
 * an assumption, currently true, that the hook function is
 * called immediately prior to the delivery function. This
 * allows the above static variables to be sufficient in
 * recording the state of the signal without introducing
 * data structures attached to objects to record their
 * signal emissions.
 *
 * Note that the hook function may be called several times prior
 * to delivery to our particular slot function.  The hook function
 * is per-signal whilst the slot function is per-widget, i.e, the
 * hook function is applied to all signal deliveries in the widget,
 * not only ours. We record the last thing hooked and use this in
 * the slot function.
 */
static gboolean
ehook
(
    GtkObject	*object,
    guint	signalid,
    guint	nparam,
    GtkArg	*param,
    gpointer data
)
{

    if (object != lasthooked)
    {
#ifdef DEBUG_SIGNALS
	int	i;

	fprintf
	(
	    stderr,
	    "emission hook: %p, %d, %d param\n",
	    (void *)object,
	    signalid,
	    nparam
	);
	for (i = 0; i < nparam; ++i)
	{
	    printf("\targ %s", gtk_type_name(param[i].type));
	    if (param[i].name != NULL)
		printf(": %s", param[i].name);
	    printf("\n");
	}
#endif
	lasthooked = object;
	lastsigid = signalid;
	lastparam = param;
	lastnparam = nparam;
    }
    return 1;
}

static void
deliver_signal(GtkWidget *widget, ...)
{
    va_list	va;
    object_t	*w;
    object_t	*fn;
    object_t	*arg;
    int		i;
    array_t	*iargv;
    struct_t	*spec;
    object_t	*o;

    if (GTK_OBJECT(widget) != lasthooked)
	return;

    lasthooked = NULL;

#ifdef DEBUG_SIGNALS

    fprintf(stderr, "deliver: %p, %d, %d param\n", (void *)widget, lastsigid, lastnparam);
    for (i = 0; i < lastnparam; ++i)
    {
	printf("\targ %s", gtk_type_name(lastparam[i].type));
	if (lastparam[i].name != NULL)
	    printf(": %s", lastparam[i].name);
	printf("\n");
    }

#endif

    /*
     * Build ici argument vector
     */
    if ((iargv = new_array()) == NULL)
    {
	signal_handler_errored = 1;
	gtk_main_quit();
	return;
    }
    if (pushcheck(iargv, lastnparam + 2))
	goto fail;
    va_start(va, widget);
    for (i = 0; i < lastnparam; ++i)
    {
	int	t = lastparam[i].type;

#if 0
	fprintf(stderr, "param type %d (%s)\n", t, gtk_type_name(t));
#endif

	/* Gtk type definitions are actually global unsigned int
	 * variables and not enums or macros despite the naming
	 * convention used. This means we can't use a switch and
	 * must resort to a big-if.
	 */
	if (t == GTK_TYPE_GDK_EVENT_TYPE)
	{
	    if ((o = objof(event_to_struct(va_arg(va, GdkEvent *)))) == NULL)
		goto fail;
	}
	else if
	(
	    t == GTK_TYPE_GDK_EVENT_MASK
	    || t == GTK_TYPE_CALENDAR_DISPLAY_OPTIONS
	    || t == GTK_TYPE_CELL_TYPE
	    || t == GTK_TYPE_CLIST_DRAG_POS
	    || t == GTK_TYPE_BUTTON_ACTION
	    || t == GTK_TYPE_CTREE_POS
	    || t == GTK_TYPE_CTREE_LINE_STYLE
	    || t == GTK_TYPE_CTREE_EXPANDER_STYLE
	    || t == GTK_TYPE_CTREE_EXPANSION_TYPE
	    || t == GTK_TYPE_DEBUG_FLAG
	    || t == GTK_TYPE_DEST_DEFAULTS
	    || t == GTK_TYPE_TARGET_FLAGS
	    || t == GTK_TYPE_ARROW_TYPE
	    || t == GTK_TYPE_ATTACH_OPTIONS
	    || t == GTK_TYPE_BUTTON_BOX_STYLE
	    || t == GTK_TYPE_CURVE_TYPE
	    || t == GTK_TYPE_DIRECTION_TYPE
	    || t == GTK_TYPE_JUSTIFICATION
	    || t == GTK_TYPE_MATCH_TYPE
	    || t == GTK_TYPE_MENU_DIRECTION_TYPE
	    || t == GTK_TYPE_MENU_FACTORY_TYPE
	    || t == GTK_TYPE_METRIC_TYPE
	    || t == GTK_TYPE_ORIENTATION
	    || t == GTK_TYPE_CORNER_TYPE
	    || t == GTK_TYPE_PACK_TYPE
	    || t == GTK_TYPE_PATH_PRIORITY_TYPE
	    || t == GTK_TYPE_PATH_TYPE
	    || t == GTK_TYPE_POLICY_TYPE
	    || t == GTK_TYPE_POSITION_TYPE
	    || t == GTK_TYPE_PREVIEW_TYPE
	    || t == GTK_TYPE_RELIEF_STYLE
	    || t == GTK_TYPE_RESIZE_MODE
	    || t == GTK_TYPE_SIGNAL_RUN_TYPE
	    || t == GTK_TYPE_SCROLL_TYPE
	    || t == GTK_TYPE_SELECTION_MODE
	    || t == GTK_TYPE_SHADOW_TYPE
	    || t == GTK_TYPE_STATE_TYPE
	    || t == GTK_TYPE_SUBMENU_DIRECTION
	    || t == GTK_TYPE_SUBMENU_PLACEMENT
	    || t == GTK_TYPE_TOOLBAR_STYLE
	    || t == GTK_TYPE_TROUGH_TYPE
	    || t == GTK_TYPE_UPDATE_TYPE
	    || t == GTK_TYPE_VISIBILITY
	    || t == GTK_TYPE_WINDOW_POSITION
	    || t == GTK_TYPE_WINDOW_TYPE
	    || t == GTK_TYPE_SORT_TYPE
	    || t == GTK_TYPE_FONT_METRIC_TYPE
	    || t == GTK_TYPE_FONT_TYPE
	    || t == GTK_TYPE_FONT_FILTER_TYPE
	    || t == GTK_TYPE_OBJECT_FLAGS
	    || t == GTK_TYPE_ARG_FLAGS
	    || t == GTK_TYPE_PACKER_OPTIONS
	    || t == GTK_TYPE_SIDE_TYPE
	    || t == GTK_TYPE_ANCHOR_TYPE
	    || t == GTK_TYPE_PRIVATE_FLAGS
	    || t == GTK_TYPE_PROGRESS_BAR_STYLE
	    || t == GTK_TYPE_PROGRESS_BAR_ORIENTATION
	    || t == GTK_TYPE_RC_FLAGS
	    || t == GTK_TYPE_RC_TOKEN_TYPE
	    || t == GTK_TYPE_SPIN_BUTTON_UPDATE_POLICY
	    || t == GTK_TYPE_SPIN_TYPE
	    || t == GTK_TYPE_TOOLBAR_CHILD_TYPE
	    || t == GTK_TYPE_TOOLBAR_SPACE_STYLE
	    || t == GTK_TYPE_TREE_VIEW_MODE
	    || t == GTK_TYPE_FUNDAMENTAL_TYPE
	    || t == GTK_TYPE_WIDGET_FLAGS
	    || t == GTK_TYPE_GDK_WINDOW_TYPE
	    || t == GTK_TYPE_GDK_WINDOW_CLASS
	    || t == GTK_TYPE_GDK_IMAGE_TYPE
	    || t == GTK_TYPE_GDK_VISUAL_TYPE
	    || t == GTK_TYPE_GDK_FONT_TYPE
	    || t == GTK_TYPE_GDK_WINDOW_ATTRIBUTES_TYPE
	    || t == GTK_TYPE_GDK_WINDOW_HINTS
	    || t == GTK_TYPE_GDK_FILL
	    || t == GTK_TYPE_GDK_FILL_RULE
	    || t == GTK_TYPE_GDK_LINE_STYLE
	    || t == GTK_TYPE_GDK_CAP_STYLE
	    || t == GTK_TYPE_GDK_JOIN_STYLE
	    || t == GTK_TYPE_GDK_CURSOR_TYPE
	    || t == GTK_TYPE_GDK_FILTER_RETURN
	    || t == GTK_TYPE_GDK_VISIBILITY_STATE
	    || t == GTK_TYPE_GDK_EVENT_MASK
	    || t == GTK_TYPE_GDK_NOTIFY_TYPE
	    || t == GTK_TYPE_GDK_CROSSING_MODE
	    || t == GTK_TYPE_GDK_MODIFIER_TYPE
	    || t == GTK_TYPE_GDK_SUBWINDOW_MODE
	    || t == GTK_TYPE_GDK_INPUT_CONDITION
	    || t == GTK_TYPE_GDK_STATUS
	    || t == GTK_TYPE_GDK_BYTE_ORDER
	    || t == GTK_TYPE_GDK_GC_VALUES_MASK
	    || t == GTK_TYPE_GDK_SELECTION
	    || t == GTK_TYPE_GDK_PROPERTY_STATE
	    || t == GTK_TYPE_GDK_PROP_MODE
	    || t == GTK_TYPE_GDK_INPUT_SOURCE
	    || t == GTK_TYPE_GDK_INPUT_MODE
	    || t == GTK_TYPE_GDK_AXIS_USE
	    || t == GTK_TYPE_GDK_TARGET
	    || t == GTK_TYPE_GDK_SELECTION_TYPE
	    || t == GTK_TYPE_GDK_EXTENSION_MODE
	    || t == GTK_TYPE_GDK_IM_STYLE
	    || t == GTK_TYPE_GDK_IC_ATTRIBUTES_TYPE
	    || t == GTK_TYPE_GDK_WM_DECORATION
	    || t == GTK_TYPE_GDK_WM_FUNCTION
	    || t == GTK_TYPE_GDK_COLOR_CONTEXT_MODE
	    || t == GTK_TYPE_GDK_OVERLAP_TYPE
	    || t == GTK_TYPE_GDK_DRAG_ACTION
	    || t == GTK_TYPE_GDK_DRAG_PROTOCOL
	    || t == GTK_TYPE_GDK_RGB_DITHER
	    || t == GTK_TYPE_ACCEL_GROUP
	    || t == GTK_TYPE_SELECTION_DATA
	    || t == GTK_TYPE_STYLE
	    || t == GTK_TYPE_CTREE_NODE
	    || t == GTK_TYPE_GDK_COLORMAP
	    || t == GTK_TYPE_GDK_VISUAL
	    || t == GTK_TYPE_GDK_FONT
	    || t == GTK_TYPE_GDK_WINDOW
	    || t == GTK_TYPE_GDK_DRAG_CONTEXT
	    || 1
	)
	{
	    if ((o = objof(new_int(va_arg(va, int)))) == NULL)
		goto fail;
	}
	else if (t == GTK_TYPE_GDK_COLOR)
	{
	    if ((o = objof(new_color(va_arg(va, GdkColor *)))) == NULL)
		goto fail;
	}
	else
	{
	    (void) va_arg(va, int); /* consume argument */
	    o = objof(&o_null); /* and subsitute with NULL */
	    incref(o); /* to complement decref below */
	}
	*iargv->a_top++ = o;
	decref(o);
    }
    spec = va_arg(va, struct_t *);
    va_end(va);
    if (!isstruct(objof(spec)))
    {
	error = "internal error, unexpected 'spec' type in Gtk signal delivery function";
	goto fail;
    }

    w = fetch(structof(spec), STRING(widget));
    fn = fetch(structof(spec), STRING(fn));
    arg = fetch(structof(spec), STRING(arg));

    /*
     * Ensure there's room for two more objects in the argument
     * vector and put the user-supplied arg at the end, shove
     * everything up one slot and insert the widget as the
     * first parameter. Couldn't do it before as we didn't have
     * the spec object to look up the widget and we needed to
     * iterate over the actual args to get the spec object.
     */
    *iargv->a_top++ = arg;
    memmove
    (
	iargv->a_base + 1,
	iargv->a_base,
	sizeof (object_t *) * (iargv->a_top - iargv->a_base)
    );
    ++iargv->a_top;
    iargv->a_base[0] = w;
    if (ici_call("call", "oo", fn, iargv) != NULL)
    {
	signal_handler_errored = 1;
	gtk_main_quit();
    }
    decref(iargv);
    return;

fail:
    signal_handler_errored = 1;
    gtk_main_quit();
    decref(iargv);
}

FUNC(signal_connect)
{
    widget_t		*widget;
    string_t		*sig;
    func_t		*fn;
    object_t		*arg;
    struct_t		*aggr;
    int			sigid;

    NEED_STRINGS(1);
    if (NARGS() == 4)
    {
	if (ici_typecheck("oooo", &widget, &sig, &fn, &arg))
	    return 1;
    }
    else
    {
	if (ici_typecheck("ooo", &widget, &sig, &fn))
	    return 1;
	arg = objof(&o_null);
    }
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    if (!isstring(objof(sig)))
	return ici_argerror(1);
    if (!isfunc(objof(fn)) && !iscfunc(objof(fn)) && !ismethod(objof(fn)))
	return ici_argerror(2);
    if ((aggr = structof(fetch(widget->g_signals, sig))) == NULL)
	return 1;
    if (objof(aggr) == objof(&o_null))
    {
	if ((aggr = new_struct()) == NULL)
	    return 1;
	if (assign(widget->g_signals, sig, aggr))
	{
	    decref(aggr);
	    return 1;
	}
    }
    if
    (
	assign(aggr, STRING(fn), fn)
	||
	assign(aggr, STRING(arg), arg)
	||
	assign(aggr, STRING(widget), widget)
    )
    {
	return 1;
    }
    sigid = gtk_signal_lookup(sig->s_chars, GTK_OBJECT_TYPE(widget->g_widget));
    if (strcmp(gtk_signal_name(sigid), "destroy") != 0)
	gtk_signal_add_emission_hook(sigid, ehook, NULL);
    sigid = gtk_signal_connect
    (
	GTK_OBJECT(widget->g_widget),
	sig->s_chars,
	GTK_SIGNAL_FUNC(deliver_signal),
	aggr
    );
    return int_ret(sigid);
}

/* Statusbar */

CFUNC1(statusbar_new, glue__W, gtk_statusbar_new);

FUNC(statusbar_get_context_id)
{
    widget_t	*statusbar;
    char	*contextdesc;

    if (ici_typecheck("os", &statusbar, &contextdesc))
	return 1;
    if (!iswidget(objof(statusbar)))
	return ici_argerror(0);
    return int_ret
    (
	gtk_statusbar_get_context_id
	(
	    GTK_STATUSBAR(statusbar->g_widget),
	    contextdesc
	)
    );
}

FUNC(statusbar_push)
{
    widget_t	*statusbar;
    char	*contextdesc;
    long	contextid;

    if (ici_typecheck("ois", &statusbar, &contextid, &contextdesc))
	return 1;
    if (!iswidget(objof(statusbar)))
	return ici_argerror(0);
    contextid = gtk_statusbar_push
    (
	GTK_STATUSBAR(statusbar->g_widget),
	contextid,
	contextdesc
    );
    return int_ret(contextid);
}

CFUNC1(statusbar_pop, glue_w_n, gtk_statusbar_pop);
CFUNC1(statusbar_remove, glue_wii_n, gtk_statusbar_remove);

/* Menu */

CFUNC1(menu_get_type, glue__i, gtk_menu_get_type);
CFUNC1(menu_new, glue__W, gtk_menu_new);
CFUNC1(menu_append, glue_ww_n, gtk_menu_append);
CFUNC1(menu_prepend, glue_ww_n, gtk_menu_prepend);

FUNC(menu_insert)
{
    widget_t	*menu;
    widget_t	*child;
    long	pos;
    
    if (ici_typecheck("ooi", &menu, &child, &pos))
	return 1;
    if (!iswidget(objof(menu)))
	return ici_argerror(0);
    if (!iswidget(objof(child)))
	return ici_argerror(1);
    gtk_menu_insert(GTK_MENU(menu->g_widget), child->g_widget, pos);
    return null_ret();
}

/* menu_popup */

CFUNC1(menu_reposition, glue_w_n, gtk_menu_reposition);
CFUNC1(menu_popdown, glue_w_n, gtk_menu_popdown);

/* Menu bar */

CFUNC1(menu_bar_get_type, glue__i, gtk_menu_bar_get_type);
CFUNC1(menu_bar_new, glue__W, gtk_menu_bar_new);
CFUNC1(menu_bar_append, glue_ww_n, gtk_menu_bar_append);
CFUNC1(menu_bar_prepend, glue_ww_n, gtk_menu_bar_prepend);

FUNC(menu_bar_insert)
{
    widget_t	*menubar;
    widget_t	*widget;
    long	pos;

    if (ici_typecheck("ooi", &menubar, &widget, &pos))
	return 1;
    if (!iswidget(objof(menubar)))
	return ici_argerror(0);
    if (!iswidget(objof(widget)))
	return ici_argerror(1);
    gtk_menu_bar_insert(GTK_MENU_BAR(menubar->g_widget), widget->g_widget, pos);
    return null_ret();
}

CFUNC1(menu_bar_set_shadow_type, glue_wi_n, gtk_menu_bar_set_shadow_type);

/* Item */

CFUNC1(item_get_type, glue__i, gtk_item_get_type);
CFUNC1(item_select, glue_w_n, gtk_item_select);
CFUNC1(item_deselect, glue_w_n, gtk_item_deselect);
CFUNC1(item_toggle, glue_w_n, gtk_item_toggle);

/* Menu item */

CFUNC1(menu_item_get_type, glue__i, gtk_menu_item_get_type);
CFUNC1(menu_item_new, glue__W, gtk_menu_item_new);
CFUNC1(menu_item_new_with_label, glue_s_W, gtk_menu_item_new_with_label);
CFUNC1(menu_item_set_submenu, glue_ww_n, gtk_menu_item_set_submenu);
CFUNC1(menu_item_remove_submenu, glue_w_n, gtk_menu_item_remove_submenu);
CFUNC1(menu_item_set_placement, glue_wi_n, gtk_menu_item_set_placement);
CFUNC1(menu_item_configure, glue_wii_n, gtk_menu_item_configure);
CFUNC1(menu_item_select, glue_w_n, gtk_menu_item_select);
CFUNC1(menu_item_deselect, glue_w_n, gtk_menu_item_deselect);
CFUNC1(menu_item_activate, glue_w_n, gtk_menu_item_activate);
CFUNC1(menu_item_right_justify, glue_w_n, gtk_menu_item_right_justify);

/* Tree */

CFUNC1(tree_new, glue__W, gtk_tree_new);
CFUNC1(tree_append, glue_ww_n, gtk_tree_append);
CFUNC1(tree_prepend, glue_ww_n, gtk_tree_prepend);

FUNC(tree_insert)
{
    widget_t	*tree;
    widget_t	*widget;
    long	pos;

    if (ici_typecheck("ooi", &tree, &widget, &pos))
	return 1;
    if (!iswidget(objof(tree)))
	return ici_argerror(0);
    if (!iswidget(objof(widget)))
	return ici_argerror(1);
    gtk_tree_insert(GTK_TREE(tree->g_widget), widget->g_widget, pos);
    return null_ret();
}

/* remote_items */

CFUNC1(tree_clear_items, glue_wii_n, gtk_tree_clear_items);
CFUNC1(tree_select_item, glue_wi_n, gtk_tree_select_item);
CFUNC1(tree_unselect_item, glue_wi_n, gtk_tree_unselect_item);
CFUNC1(tree_select_child, glue_ww_n, gtk_tree_select_child);
CFUNC1(tree_unselect_child, glue_ww_n, gtk_tree_unselect_child);

FUNC(tree_child_position)
{
    widget_t	*tree;
    widget_t	*child;

    if (ici_typecheck("oo", &tree, &child))
	return 1;
    if (!iswidget(objof(tree)))
	return ici_argerror(0);
    if (!iswidget(objof(child)))
	return ici_argerror(1);
    return int_ret(gtk_tree_child_position(GTK_TREE(tree->g_widget), child->g_widget));
}

CFUNC1(tree_set_selection_mode, glue_wi_n, gtk_tree_set_selection_mode);
CFUNC1(tree_set_view_mode, glue_wi_n, gtk_tree_set_view_mode);
CFUNC1(tree_set_view_lines, glue_wi_n, gtk_tree_set_view_lines);

/* Tree item */

CFUNC1(tree_item_new, glue__W, gtk_tree_item_new);
CFUNC1(tree_item_new_with_label, glue_s_W, gtk_tree_item_new_with_label);
CFUNC1(tree_item_set_subtree, glue_ww_n, gtk_tree_item_set_subtree);
CFUNC1(tree_item_remove_subtree, glue_w_n, gtk_tree_item_remove_subtree);
CFUNC1(tree_item_select, glue_w_n, gtk_tree_item_select);
CFUNC1(tree_item_deselect, glue_w_n, gtk_tree_item_deselect);
CFUNC1(tree_item_expand, glue_w_n, gtk_tree_item_expand);
CFUNC1(tree_item_collapse, glue_w_n, gtk_tree_item_collapse);

/* Editable */

CFUNC1(editable_select_region, glue_wii_n, gtk_editable_select_region);

FUNC(editable_insert_text)
{
    widget_t	*widget;
    char	*text;
    long	len;
    long	pos;
    gint	ipos;

    if (NARGS() == 3)
    {
	if (ici_typecheck("osi", &widget, &text, &pos))
	    return 1;
	len = -1;
    }
    else if (ici_typecheck("osii", &widget, &text, &len, &pos))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    ipos = pos;
    gtk_editable_insert_text(GTK_EDITABLE(widget->g_widget), text, len, &ipos);
    return int_ret(ipos);
}

CFUNC1(editable_delete_text, glue_wii_n, gtk_editable_delete_text);

FUNC(editable_get_chars)
{
    widget_t	*widget;
    long	start;
    long	end;

    if (ici_typecheck("oii", &widget, &start, &end))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    return str_ret(gtk_editable_get_chars(GTK_EDITABLE(widget->g_widget), start, end));
}

CFUNC1(editable_cut_clipboard, glue_w_n, gtk_editable_cut_clipboard);
CFUNC1(editable_copy_clipboard, glue_w_n, gtk_editable_copy_clipboard);
CFUNC1(editable_paste_clipboard, glue_w_n, gtk_editable_paste_clipboard);
CFUNC1(editable_claim_selection, glue_wii_n, gtk_editable_claim_selection);
CFUNC1(editable_delete_selection, glue_w_n, gtk_editable_delete_selection);
CFUNC1(editable_changed, glue_w_n, gtk_editable_changed);
CFUNC1(editable_set_position, glue_wi_n, gtk_editable_set_position);
CFUNC1(editable_get_position, glue_w_i, gtk_editable_get_position);
CFUNC1(editable_set_editable, glue_wi_n, gtk_editable_set_editable);

/* Text */

CFUNC1(text_get_type, glue__i, gtk_text_get_type);

FUNC(text_new)
{
    adjustment_t	*hadj;
    adjustment_t	*vadj;

    hadj = vadj = NULL;
    if (NARGS() > 0)
    {
	if (ici_typecheck("oo", &hadj, &vadj))
	    return 1;
	if (objof(hadj) == objof(&o_null))
	    hadj = NULL;
	else if (!isadjustment(objof(hadj)))
	    return ici_argerror(0);
	if (objof(vadj) == objof(&o_null))
	    vadj = NULL;
	else if (!isadjustment(objof(vadj)))
	    return ici_argerror(1);
    }
    return ici_ret_with_decref
    (
	objof
	(
	    new_widget
	    (
		gtk_text_new
		(
		    hadj != NULL ? GTK_ADJUSTMENT(hadj->g_ptr) : NULL,
		    vadj != NULL ? GTK_ADJUSTMENT(vadj->g_ptr) : NULL
		)
	    )
	)
    );
}

FUNC(text_get_vadj)
{
    widget_t	*text;
	
    if (ici_typecheck("o", &text))
	return 1;
    if (!iswidget(objof(text)))
	return ici_argerror(0);
    return ici_ret_with_decref(objof(new_adjustment(GTK_OBJECT(GTK_TEXT(text->g_widget)->vadj))));
}

CFUNC1(text_set_editable, glue_wi_n, gtk_text_set_editable);
CFUNC1(text_set_word_wrap, glue_wi_n, gtk_text_set_word_wrap);
CFUNC1(text_set_line_wrap, glue_wi_n, gtk_text_set_line_wrap);

FUNC(text_set_adjustments)
{
    widget_t		*text;
    adjustment_t	*hadj;
    GtkAdjustment	*h;
    adjustment_t	*vadj;
    GtkAdjustment	*v;

    if (ici_typecheck("ooo", &text, &hadj, &vadj))
	return 1;
    if (!iswidget(objof(text)))
	return  ici_argerror(0);
    if (isadjustment(objof(hadj)))
	h = GTK_ADJUSTMENT(hadj->g_ptr);
    else if (isnull(objof(hadj)))
	h = NULL;
    else
	return ici_argerror(1);
    if (isadjustment(objof(vadj)))
	v = GTK_ADJUSTMENT(vadj->g_ptr);
    else if (isnull(objof(vadj)))
	v = NULL;
    else
	return ici_argerror(2);
    gtk_text_set_adjustments(GTK_TEXT(text->g_widget), h, v);
    return null_ret();
}

CFUNC1(text_set_point, glue_wi_n, gtk_text_set_point);
CFUNC1(text_get_point, glue_w_i, gtk_text_get_point);
CFUNC1(text_get_length, glue_w_i, gtk_text_get_length);
CFUNC1(text_freeze, glue_w_n, gtk_text_freeze);
CFUNC1(text_thaw, glue_w_n, gtk_text_thaw);

FUNC(text_insert)
{
    widget_t	*text;
    char	*chars;
    long	len;
    color_t	*fg;
    color_t	*bg;
    font_t	*fnt;

    len = -1;
    fnt = NULL;
    fg = bg = NULL;
    if (NARGS() == 2)
    {
	if (ici_typecheck("os", &text, &chars))
	    return 1;
    }
    else if (NARGS() == 3)
    {
	if (ici_typecheck("oso", &text, &chars, &fnt))
	    return 1;
	if (isint(objof(fnt)))
	{
	    len = intof(fnt)->i_value;
	    fnt = NULL;
	}
	else if (isnull(objof(fnt)))
	    fnt = NULL;
	else if (!isfont(objof(fnt)))
	    return ici_argerror(2);
    }
    else if (ici_typecheck("osiooo", &text, &chars, &len, &fg, &bg, &fnt))
	return 1;
    if (!iswidget(objof(text)))
	return ici_argerror(0);
    gtk_text_insert
    (
	GTK_TEXT(text->g_widget),
	fnt ? fnt->g_ptr : NULL,
	fg ? fg->g_ptr : NULL,
	bg ? bg->g_ptr : NULL,
	chars,
	len
    );
    return null_ret();
}

CFUNC1(text_backward_delete, glue_wi_n, gtk_text_backward_delete);
CFUNC1(text_forward_delete, glue_wi_n, gtk_text_forward_delete);

/* File Selection */

FUNC(file_selection_new)
{
    char	*title;

    if (ici_typecheck("s", &title))
	return 1;
    return ici_ret_with_decref(objof(new_widget(gtk_file_selection_new(title))));
}

CFUNC1(file_selection_set_filename, glue_ws_n, gtk_file_selection_set_filename);

FUNC(file_selection_get_filename)
{
    widget_t	*filesel;

    if (ici_typecheck("o", &filesel))
	return 1;
    if (!iswidget(objof(filesel)))
	return ici_argerror(0);
    return str_ret(gtk_file_selection_get_filename(GTK_FILE_SELECTION(filesel->g_widget)));
}

CFUNC1(file_selection_complete, glue_ws_n, gtk_file_selection_complete);
CFUNC1(file_selection_show_fileop_buttons, glue_w_n, gtk_file_selection_show_fileop_buttons);
CFUNC1(file_selection_hide_fileop_buttons, glue_w_n, gtk_file_selection_hide_fileop_buttons);

FUNC(file_selection_ok_button)
{
    widget_t	*filesel;

    if (ici_typecheck("o", &filesel))
	return 1;
    if (!iswidget(objof(filesel)))
	return ici_argerror(0);
    return ici_ret_with_decref(objof(new_widget(GTK_FILE_SELECTION(filesel->g_widget)->ok_button)));
}

FUNC(file_selection_cancel_button)
{
    widget_t	*filesel;

    if (ici_typecheck("o", &filesel))
	return 1;
    if (!iswidget(objof(filesel)))
	return ici_argerror(0);
    return ici_ret_with_decref(objof(new_widget(GTK_FILE_SELECTION(filesel->g_widget)->cancel_button)));
}

/* Dialog */

FUNC(dialog_new)
{
    return ici_ret_with_decref(objof(new_widget(gtk_dialog_new())));
}

FUNC(dialog_vbox)
{
    widget_t	*d;

    if (ici_typecheck("o", &d))
	return 1;
    if (!iswidget(objof(d)))
	return ici_argerror(0);
    return ici_ret_with_decref(objof(new_widget(GTK_DIALOG(d->g_widget)->vbox)));
}

FUNC(dialog_action_area)
{
    widget_t	*d;

    if (ici_typecheck("o", &d))
	return 1;
    if (!iswidget(objof(d)))
	return ici_argerror(0);
    return ici_ret_with_decref(objof(new_widget(GTK_DIALOG(d->g_widget)->action_area)));
}

/* Entry */

FUNC(entry_new)
{
    return ici_ret_with_decref(objof(new_widget(gtk_entry_new())));
}

FUNC(entry_new_with_max_length)
{
    long	maxlen;

    if (ici_typecheck("i", &maxlen))
	return 1;
    return ici_ret_with_decref(objof(new_widget(gtk_entry_new_with_max_length(maxlen))));
}

CFUNC1(entry_set_text, glue_ws_n, gtk_entry_set_text);
CFUNC1(entry_append_text, glue_ws_n, gtk_entry_append_text);
CFUNC1(entry_prepend_text, glue_ws_n, gtk_entry_prepend_text);
CFUNC1(entry_set_position, glue_wi_n, gtk_entry_set_position);

FUNC(entry_get_text)
{
    widget_t	*widget;

    if (ici_typecheck("o", &widget))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    return str_ret(gtk_entry_get_text(GTK_ENTRY(widget->g_widget)));
}

CFUNC1(entry_select_region, glue_wii_n, gtk_entry_select_region);
CFUNC1(entry_set_visibility, glue_wi_n, gtk_entry_set_visibility);
CFUNC1(entry_set_editable, glue_wi_n, gtk_entry_set_editable);
CFUNC1(entry_set_max_length, glue_wi_n, gtk_entry_set_max_length);

/* CList */

FUNC(clist_new)
{
    long	cols;

    if (ici_typecheck("i", &cols))
	return 1;
    return ici_ret_with_decref(objof(new_widget(gtk_clist_new(cols))));
}

FUNC(clist_new_with_titles)
{
    long	cols;
    array_t	*titles;
    char	**ctitles;
    int		i;
    object_t	*rc;
    long	sz;
    int		n;

    if (ici_typecheck("ia", &cols, &titles))
	return 1;
    sz = sizeof (char *) * (n = titles->a_top - titles->a_base);
    if ((ctitles = ici_raw_alloc(sz)) == NULL)
	return 1;
    for (i = 0; i < n; ++i)
    {
	if (!isstring(titles->a_base[i]))
	{
	    error = "non-string in titles array";
	    ici_raw_free(ctitles);
	    return 1;
	}
	ctitles[i] = stringof(titles->a_base[i])->s_chars;
    }
    rc = objof(new_widget(gtk_clist_new_with_titles(cols, ctitles)));
    ici_raw_free(ctitles);
    return ici_ret_with_decref(rc);
}

FUNC(clist_set_hadjustment)
{
    widget_t		*widget;
    adjustment_t	*adjustment;

    if (ici_typecheck("oo", &widget, &adjustment))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    if (!isadjustment(objof(adjustment)))
	return ici_argerror(1);
    gtk_clist_set_hadjustment(GTK_CLIST(widget->g_widget), GTK_ADJUSTMENT(adjustment->g_ptr));
    return null_ret();
}

FUNC(clist_set_vadjustment)
{
    widget_t		*widget;
    adjustment_t	*adjustment;

    if (ici_typecheck("oo", &widget, &adjustment))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    if (!isadjustment(objof(adjustment)))
	return ici_argerror(1);
    gtk_clist_set_vadjustment(GTK_CLIST(widget->g_widget), GTK_ADJUSTMENT(adjustment->g_ptr));
    return null_ret();
}

CFUNC1(clist_set_shadow_type, glue_wi_n, gtk_clist_set_shadow_type);
CFUNC1(clist_set_selection_mode, glue_wi_n, gtk_clist_set_selection_mode);
CFUNC1(clist_set_reorderable, glue_wi_n, gtk_clist_set_reorderable);
CFUNC1(clist_set_use_drag_icons, glue_wi_n, gtk_clist_set_use_drag_icons);
CFUNC1(clist_set_button_actions, glue_wii_n, gtk_clist_set_button_actions);
CFUNC1(clist_freeze, glue_w_n, gtk_clist_freeze);
CFUNC1(clist_thaw, glue_w_n, gtk_clist_thaw);
CFUNC1(clist_column_titles_show, glue_w_n, gtk_clist_column_titles_show);
CFUNC1(clist_column_titles_hide, glue_w_n, gtk_clist_column_titles_hide);
CFUNC1(clist_column_titles_active, glue_w_n, gtk_clist_column_titles_active);
CFUNC1(clist_column_titles_passive, glue_w_n, gtk_clist_column_titles_passive);
CFUNC1(clist_column_title_active, glue_wi_n, gtk_clist_column_title_active);
CFUNC1(clist_column_title_passive, glue_wi_n, gtk_clist_column_title_passive);

FUNC(clist_set_column_title)
{
    widget_t	*widget;
    long	col;
    char	*title;

    if (ici_typecheck("ois", &widget, &col, &title))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    gtk_clist_set_column_title(GTK_CLIST(widget->g_widget), col, title);
    return null_ret();
}

FUNC(clist_get_column_title)
{
    widget_t	*widget;
    long	col;

    if (ici_typecheck("oi", &widget, &col))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    return str_ret(gtk_clist_get_column_title(GTK_CLIST(widget->g_widget), col));
}

FUNC(clist_set_column_widget)
{
    widget_t	*widget;
    long	col;
    widget_t	*widget2;

    if (ici_typecheck("oio", &widget, &col, &widget2))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    if (!iswidget(objof(widget2)))
	return ici_argerror(2);
    gtk_clist_set_column_widget(GTK_CLIST(widget->g_widget), col, widget2->g_widget);
    return null_ret();
}

FUNC(clist_get_column_widget)
{
    widget_t	*widget;
    long	col;

    if (ici_typecheck("oi", &widget, &col))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    return ici_ret_with_decref(objof(new_widget(gtk_clist_get_column_widget(GTK_CLIST(widget->g_widget), col))));
}

CFUNC1(clist_set_column_justification, glue_wii_n, gtk_clist_set_column_justification);
CFUNC1(clist_set_column_visibility, glue_wii_n, gtk_clist_set_column_visibility);
CFUNC1(clist_set_column_resizeable, glue_wii_n, gtk_clist_set_column_resizeable);

FUNC(clist_append)
{
    widget_t	*widget;
    array_t	*text;
    char	**ctext;
    int		i, n, rc;

    if (ici_typecheck("oa", &widget, &text))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    if ((ctext = ici_raw_alloc(sizeof (char *) * (n = text->a_top - text->a_base))) == NULL)
	return 1;
    for (i = 0; i < n; ++i)
    {
	if (!isstring(text->a_base[i]))
	{
	    error = "non-string in array";
	    ici_raw_free(ctext);
	    return 1;
	}
	ctext[i] = stringof(text->a_base[i])->s_chars;
    }
    rc = gtk_clist_append(GTK_CLIST(widget->g_widget), ctext);
    ici_raw_free(ctext);
    return int_ret(rc);
}

FUNC(clist_prepend)
{
    widget_t	*widget;
    array_t	*text;
    char	**ctext;
    int		i, n, rc;

    if (ici_typecheck("oa", &widget, &text))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    if ((ctext = ici_raw_alloc(sizeof (char *) * (n = text->a_top - text->a_base))) == NULL)
	return 1;
    for (i = 0; i < n; ++i)
    {
	if (!isstring(text->a_base[i]))
	{
	    error = "non-string in array";
	    ici_raw_free(ctext);
	    return 1;
	}
	ctext[i] = stringof(text->a_base[i])->s_chars;
    }
    rc = gtk_clist_prepend(GTK_CLIST(widget->g_widget), ctext);
    ici_raw_free(ctext);
    return int_ret(rc);
}

CFUNC1(clist_set_column_auto_resize, glue_wii_n, gtk_clist_set_column_auto_resize);
CFUNC1(clist_columns_autosize, glue_w_i, gtk_clist_columns_autosize);

FUNC(clist_optimal_column_width)
{
    widget_t	*widget;
    long	col;

    if (ici_typecheck("oi", &widget, &col))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    return int_ret(gtk_clist_optimal_column_width(GTK_CLIST(widget->g_widget), col));
}

CFUNC1(clist_set_column_width, glue_wii_n, gtk_clist_set_column_width);
CFUNC1(clist_set_column_min_width, glue_wii_n, gtk_clist_set_column_min_width);
CFUNC1(clist_set_column_max_width, glue_wii_n, gtk_clist_set_column_max_width);
CFUNC1(clist_set_row_height, glue_wi_n, gtk_clist_set_row_height);

FUNC(clist_moveto)
{
    widget_t	*widget;
    long	row;
    long	col;
    double	row_align;
    double	col_align;

    if (ici_typecheck("oiinn", &widget, &row, &col, &row_align, &col_align))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    gtk_clist_moveto(GTK_CLIST(widget->g_widget), row, col, row_align, col_align);
    return null_ret();
}

FUNC(clist_row_is_visible)
{
    widget_t	*widget;
    long	row;

    if (ici_typecheck("oi", &widget, &row))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    return int_ret(gtk_clist_row_is_visible(GTK_CLIST(widget->g_widget), row));
}

FUNC(clist_get_cell_type)
{
    widget_t	*widget;
    long	row;
    long	col;
    
    if (ici_typecheck("oii", &widget, &row, &col))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    return int_ret(gtk_clist_get_cell_type(GTK_CLIST(widget->g_widget), row, col));
}

FUNC(clist_set_text)
{
    widget_t	*widget;
    long	row;
    long	col;
    char	*text;

    if (ici_typecheck("oiis", &widget, &row, &col, &text))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    gtk_clist_set_text(GTK_CLIST(widget->g_widget), row, col, text);
    return null_ret();
}

FUNC(clist_get_text)
{
    widget_t	*widget;
    long	row;
    long	col;
    char	*text;

    if (ici_typecheck("oii", &widget, &row, &col))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    gtk_clist_get_text(GTK_CLIST(widget->g_widget), row, col, &text);
    return str_ret(text);
}

FUNC(clist_set_shift)
{
    widget_t	*widget;
    long	row;
    long	col;
    long	vert;
    long	hori;

    if (ici_typecheck("oiiii", &widget, &row, &col, &vert, &hori))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    gtk_clist_set_shift(GTK_CLIST(widget->g_widget), row, col, vert, hori);
    return null_ret();
}

FUNC(clist_insert)
{
    widget_t	*widget;
    long	row;
    array_t	*text;
    char	**ctext;
    int		rc;
    int		i, n;

    if (ici_typecheck("oia", &widget, &row, &text))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    ctext = ici_raw_alloc(sizeof (char *) * (n = text->a_top - text->a_base));
    if (ctext == NULL)
	return 1;
    for (i = 0; i < n; ++i)
    {
	if (!isstring(text->a_base[i]))
	{
	    error = "non-string in titles array";
	    ici_raw_free(ctext);
	    return 1;
	}
	ctext[i] = stringof(text->a_base[i])->s_chars;
    }
    rc = gtk_clist_insert(GTK_CLIST(widget->g_widget), row, ctext);
    ici_raw_free(ctext);
    return null_ret();
}

CFUNC1(clist_remove, glue_wi_n, gtk_clist_remove);
CFUNC1(clist_select_row, glue_wii_n, gtk_clist_select_row);
CFUNC1(clist_unselect_row, glue_wii_n, gtk_clist_unselect_row);
CFUNC1(clist_undo_selection, glue_w_n, gtk_clist_undo_selection);
CFUNC1(clist_clear, glue_w_n, gtk_clist_clear);

FUNC(clist_get_selection_info)
{
    widget_t	*widget;
    long	x, y;
    gint	row, col;
    int		rc;
    struct_t	*d;
    int_t	*i;

    NEED_STRINGS(1);
    if (ici_typecheck("oii", &widget, &x, &y))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    rc = gtk_clist_get_selection_info(GTK_CLIST(widget->g_widget), x, y, &row, &col);
    if ((d = new_struct()) == NULL)
	return 1;
    if ((i = new_int(row)) == NULL)
	goto fail;
    if (assign(d, STRING(row), i))
	goto fail;
    decref(i);
    if ((i = new_int(col)) == NULL)
	goto fail;
    if (assign(d, STRING(col), i))
	goto fail;
    decref(i);
    if ((i = new_int(rc)) == NULL)
	goto fail;
    if (assign(d, STRING(rc), i))
	goto fail;
    decref(i);
    return ici_ret_with_decref(objof(d));

fail:
    decref(d);
    return 1;
}

CFUNC1(clist_select_all, glue_w_n, gtk_clist_select_all);
CFUNC1(clist_unselect_all, glue_w_n, gtk_clist_unselect_all);
CFUNC1(clist_swap_rows, glue_wii_n, gtk_clist_swap_rows);
CFUNC1(clist_row_move, glue_wii_n, gtk_clist_row_move);
CFUNC1(clist_set_sort_column, glue_wi_n, gtk_clist_set_sort_column);
CFUNC1(clist_set_sort_type, glue_wi_n, gtk_clist_set_sort_type);
CFUNC1(clist_sort, glue_w_n, gtk_clist_sort);
CFUNC1(clist_set_auto_sort, glue_wi_n, gtk_clist_set_auto_sort);

/* ScrolledWindow */

FUNC(scrolled_window_new)
{
    adjustment_t	*hadj;
    adjustment_t	*vadj;
    GtkAdjustment	*ha, *va;
    
    if (ici_typecheck("oo", &hadj, &vadj))
	return 1;
    if (isnull(objof(hadj)))
	ha = NULL;
    else if (isadjustment(objof(hadj)))
	ha = GTK_ADJUSTMENT(hadj->g_ptr);
    else
	return ici_argerror(0);
    if (isnull(objof(vadj)))
	va = NULL;
    else if (isadjustment(objof(vadj)))
	va = GTK_ADJUSTMENT(vadj->g_ptr);
    else
	return ici_argerror(1);
    return ici_ret_with_decref(objof(new_widget(gtk_scrolled_window_new(ha, va))));
}

FUNC(scrolled_window_set_hadjustment)
{
    widget_t		*widget;
    adjustment_t	*adj;

    if (ici_typecheck("oo", &widget, &adj))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    if (!isadjustment(objof(adj)))
	return ici_argerror(1);
    gtk_scrolled_window_set_hadjustment(GTK_SCROLLED_WINDOW(widget->g_widget), GTK_ADJUSTMENT(adj->g_ptr));
    return null_ret();
}

FUNC(scrolled_window_set_vadjustment)
{
    widget_t		*widget;
    adjustment_t	*adj;

    if (ici_typecheck("oo", &widget, &adj))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    if (!isadjustment(objof(adj)))
	return ici_argerror(1);
    gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(widget->g_widget), GTK_ADJUSTMENT(adj->g_ptr));
    return null_ret();
}

FUNC(scrolled_window_get_hadjustment)
{
    widget_t		*widget;

    if (ici_typecheck("o", &widget))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    return ici_ret_with_decref(objof(new_adjustment(gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(widget->g_widget)))));
}

FUNC(scrolled_window_get_vadjustment)
{
    widget_t		*widget;

    if (ici_typecheck("o", &widget))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    return ici_ret_with_decref(objof(new_adjustment(gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(widget->g_widget)))));
}

CFUNC1(scrolled_window_set_policy, glue_wii_n, gtk_scrolled_window_set_policy);
CFUNC1(scrolled_window_set_placement, glue_wi_n, gtk_scrolled_window_set_placement);
CFUNC1(scrolled_window_add_with_viewport, glue_ww_n, gtk_scrolled_window_add_with_viewport);

/* ComboBox */

FUNC(combo_new)
{
    return ici_ret_with_decref(objof(new_widget(gtk_combo_new())));
}

FUNC(combo_set_popdown_strings)
{
    widget_t	*widget;
    array_t	*strings;
    GList	*items;
    int		i, n;

    if (ici_typecheck("oa", &widget, &strings))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    n = strings->a_top - strings->a_base;
    items = NULL;
    for (i = 0; i < n; ++i)
    {
	if (!isstring(strings->a_base[i]))
	{
	    error = "non-string in strings array";
	    if (items != NULL)
		g_list_free(items);
	    return 1;
	}
	items = g_list_append(items, stringof(strings->a_base[i])->s_chars);
    }
    gtk_combo_set_popdown_strings(GTK_COMBO(widget->g_widget), items);
    if (items != NULL)
	g_list_free(items);
    return null_ret();
}

CFUNC1(combo_set_use_arrows, glue_wi_n, gtk_combo_set_use_arrows);
CFUNC1(combo_set_use_arrows_always, glue_wi_n, gtk_combo_set_use_arrows_always);
CFUNC1(combo_set_case_sensitive, glue_wi_n, gtk_combo_set_case_sensitive);
CFUNC1(combo_set_value_in_list, glue_wii_n, gtk_combo_set_value_in_list);

FUNC(combo_set_item_string)
{
    widget_t	*widget;
    widget_t	*widget2;
    char	*value;

    if (ici_typecheck("ooi", &widget, &widget2, &value))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    if (isnull(objof(widget2)))
	widget2 = NULL;
    else if (!iswidget(objof(widget2)))
	return ici_argerror(1);
    gtk_combo_set_item_string
    (
	GTK_COMBO(widget->g_widget),
	widget2 == NULL ? NULL : GTK_ITEM(widget2->g_widget),
	value
    );
    return null_ret();
}

CFUNC1(combo_disable_activate, glue_w_n, gtk_combo_disable_activate);

FUNC(combo_entry)
{
    widget_t	*widget;

    if (ici_typecheck("o", &widget))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    return ici_ret_with_decref(objof(new_widget(GTK_COMBO(widget->g_widget)->entry)));
}

FUNC(combo_button)
{
    widget_t	*widget;

    if (ici_typecheck("o", &widget))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    return ici_ret_with_decref(objof(new_widget(GTK_COMBO(widget->g_widget)->button)));
}

FUNC(combo_popup)
{
    widget_t	*widget;

    if (ici_typecheck("o", &widget))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    return ici_ret_with_decref(objof(new_widget(GTK_COMBO(widget->g_widget)->popup)));
}

FUNC(combo_popwin)
{
    widget_t	*widget;

    if (ici_typecheck("o", &widget))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    return ici_ret_with_decref(objof(new_widget(GTK_COMBO(widget->g_widget)->popwin)));
}

FUNC(combo_list)
{
    widget_t	*widget;

    if (ici_typecheck("o", &widget))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    return ici_ret_with_decref(objof(new_widget(GTK_COMBO(widget->g_widget)->list)));
}

/* HSeparator */
CFUNC1(hseparator_new, glue__W, gtk_hseparator_new);

/* VSeparator */
CFUNC1(vseparator_new, glue__W, gtk_vseparator_new);

/* Frame */

CFUNC1(frame_new, glue_s_W, gtk_frame_new);
CFUNC1(frame_set_label, glue_ws_n, gtk_frame_set_label);

FUNC(frame_set_label_align)
{
    widget_t	*widget;
    double	x, y;

    if (ici_typecheck("onn", &widget, &x, &y))
	return 1;
    if (!iswidget(objof(widget)))
	return ici_argerror(0);
    gtk_frame_set_label_align(GTK_FRAME(widget->g_widget), x, y);
    return null_ret();
}

CFUNC1(frame_set_shadow_type, glue_wi_n, gtk_frame_set_shadow_type);


/* ButtonBox */

CFUNC1(button_box_set_spacing, glue_wi_n, gtk_button_box_set_spacing);
CFUNC1(button_box_set_layout, glue_wi_n, gtk_button_box_set_layout);

#ifdef EMBED_MOZILLA

#include "gtkmozembed.h"

FUNC(mozilla_get_type)
{
    return int_ret(gtk_moz_embed_get_type());
}

FUNC(mozilla_new)
{
    return ici_ret_with_decref(objof(new_widget(gtk_moz_embed_new())));
}

FUNC(mozilla_load_url)
{
    widget_t	*moz;
    char	*url;

    if (ici_typecheck("os", &moz, &url))
	return 1;
    if (!iswidget(objof(moz)))
	return ici_argerror(0);
    gtk_moz_embed_load_url(GTK_MOZ_EMBED(moz->g_widget), url);
    return null_ret();
}

FUNC(mozilla_stop_load)
{
    widget_t	*moz;

    if (ici_typecheck("o", &moz))
	return 1;
    if (!iswidget(objof(moz)))
	return ici_argerror(0);
    gtk_moz_embed_stop_load(GTK_MOZ_EMBED(moz->g_widget));
    return null_ret();
}

FUNC(mozilla_can_go_back)
{
    widget_t	*moz;

    if (ici_typecheck("o", &moz))
	return 1;
    if (!iswidget(objof(moz)))
	return ici_argerror(0);
    return int_ret(gtk_moz_embed_can_go_back(GTK_MOZ_EMBED(moz->g_widget)));
}

FUNC(mozilla_can_go_forward)
{
    widget_t	*moz;

    if (ici_typecheck("o", &moz))
	return 1;
    if (!iswidget(objof(moz)))
	return ici_argerror(0);
    return int_ret(gtk_moz_embed_can_go_forward(GTK_MOZ_EMBED(moz->g_widget)));
}

FUNC(mozilla_go_back)
{
    widget_t	*moz;

    if (ici_typecheck("o", &moz))
	return 1;
    if (!iswidget(objof(moz)))
	return ici_argerror(0);
    gtk_moz_embed_go_back(GTK_MOZ_EMBED(moz->g_widget));
    return null_ret();
}

FUNC(mozilla_go_forward)
{
    widget_t	*moz;

    if (ici_typecheck("o", &moz))
	return 1;
    if (!iswidget(objof(moz)))
	return ici_argerror(0);
    gtk_moz_embed_go_forward(GTK_MOZ_EMBED(moz->g_widget));
    return null_ret();
}

FUNC(mozilla_render_data)
{
    widget_t	*moz;
    string_t	*data;
    char	*uri;
    char	*mimetype;

    if (ici_typecheck("ooss", &moz, &data, &uri, &mimetype))
	return 1;
    if (!iswidget(objof(moz)))
	return ici_argerror(0);
    if (!isstring(objof(data)))
	return ici_argerror(1);
    gtk_moz_embed_render_data
    (
	GTK_MOZ_EMBED(moz->g_widget),
	data->s_chars,
	data->s_nchars,
	uri,
	mimetype
    );
    return null_ret();
}

FUNC(mozilla_open_stream)
{
    widget_t	*moz;
    char	*uri;
    char	*mimetype;
    
    if (ici_typecheck("oss", &moz, &uri, &mimetype))
	return 1;
    if (!iswidget(objof(moz)))
	return ici_argerror(0);
    gtk_moz_embed_open_stream(GTK_MOZ_EMBED(moz->g_widget), uri, mimetype);
    return null_ret();
}

FUNC(mozilla_append_data)
{
    widget_t	*moz;
    string_t	*data;

    if (ici_typecheck("oo", &moz, &data))
	return 1;
    if (!iswidget(objof(moz)))
	return ici_argerror(0);
    if (!isstring(objof(data)))
	return ici_argerror(1);
    gtk_moz_embed_append_data(GTK_MOZ_EMBED(moz->g_widget), data->s_chars, data->s_nchars);
    return null_ret();
}

FUNC(mozilla_close_stream)
{
    widget_t	*moz;

    if (ici_typecheck("o", &moz))
	return 1;
    if (!iswidget(objof(moz)))
	return ici_argerror(0);
    gtk_moz_embed_close_stream(GTK_MOZ_EMBED(moz->g_widget));
    return null_ret();
}

FUNC(mozilla_reload)
{
    widget_t	*moz;
    long	flags;

    if (ici_typecheck("oi", &moz, &flags))
	return 1;
    if (!iswidget(objof(moz)))
	return ici_argerror(0);
    gtk_moz_embed_reload(GTK_MOZ_EMBED(moz->g_widget), flags);
    return null_ret();
}

FUNC(mozilla_set_chrome_mask)
{
    widget_t	*moz;
    long	mask;

    if (ici_typecheck("oi", &moz, &mask))
	return 1;
    if (!iswidget(objof(moz)))
	return ici_argerror(0);
    gtk_moz_embed_set_chrome_mask(GTK_MOZ_EMBED(moz->g_widget), mask);
    return null_ret();
}

FUNC(mozilla_get_chrome_mask)
{
    widget_t	*moz;

    if (ici_typecheck("o", &moz))
	return 1;
    if (!iswidget(objof(moz)))
	return ici_argerror(0);
    return int_ret(gtk_moz_embed_get_chrome_mask(GTK_MOZ_EMBED(moz->g_widget)));
}

FUNC(mozilla_get_link_message)
{
    widget_t	*moz;

    if (ici_typecheck("o", &moz))
	return 1;
    if (!iswidget(objof(moz)))
	return ici_argerror(0);
    return str_ret(gtk_moz_embed_get_link_message(GTK_MOZ_EMBED(moz->g_widget)));
}

FUNC(mozilla_get_js_status)
{
    widget_t	*moz;

    if (ici_typecheck("o", &moz))
	return 1;
    if (!iswidget(objof(moz)))
	return ici_argerror(0);
    return str_ret(gtk_moz_embed_get_js_status(GTK_MOZ_EMBED(moz->g_widget)));
}

FUNC(mozilla_get_title)
{
    widget_t	*moz;

    if (ici_typecheck("o", &moz))
	return 1;
    if (!iswidget(objof(moz)))
	return ici_argerror(0);
    return str_ret(gtk_moz_embed_get_title(GTK_MOZ_EMBED(moz->g_widget)));
}

FUNC(mozilla_get_location)
{
    widget_t	*moz;

    if (ici_typecheck("o", &moz))
	return 1;
    if (!iswidget(objof(moz)))
	return ici_argerror(0);
    return str_ret(gtk_moz_embed_get_location(GTK_MOZ_EMBED(moz->g_widget)));
}
 
#endif
