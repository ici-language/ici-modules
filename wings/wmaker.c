#include <WMaker.h>

#include "ici.h"

#include "display.h"
#include "window.h"
#include "event.h"

#include "wmaker.h"

/*
 * A type to represent the wmlib application context
 */
static unsigned long
mark_appcontext(object_t *o)
{
    o->o_flags |= O_MARK;
    return sizeof (ici_wm_appcontext_t);
}

type_t	ici_wm_appcontext_type =
{
    mark_appcontext,
    free_simple,
    hash_unique,
    cmp_unique,
    copy_simple,
    assign_simple,
    fetch_simple,
    "appcontext"
};

ici_wm_appcontext_t *
ici_wm_new_appcontext(WMAppContext *appcontext)
{
    ici_wm_appcontext_t	*ac;

    if ((ac = talloc(ici_wm_appcontext_t)) != NULL)
    {
	objof(ac)->o_type  = &ici_wm_appcontext_type;
	objof(ac)->o_tcode = TC_OTHER;
	objof(ac)->o_flags = 0;
	objof(ac)->o_nrefs = 1;
	rego(objof(ac));
	ac->app_context = appcontext;
    }
    return ac;
}

/*
 * A type to represent the wmlib menu
 */
static unsigned long
mark_menu(object_t *o)
{
    o->o_flags |= O_MARK;
    return sizeof (ici_wm_menu_t);
}

type_t	ici_wm_menu_type =
{
    mark_menu,
    free_simple,
    hash_unique,
    cmp_unique,
    copy_simple,
    assign_simple,
    fetch_simple,
    "menu"
};

ici_wm_menu_t *
ici_wm_new_menu(WMMenu *menu)
{
    ici_wm_menu_t	*m;

    if ((m = talloc(ici_wm_menu_t)) != NULL)
    {
	objof(m)->o_type  = &ici_wm_menu_type;
	objof(m)->o_tcode = TC_OTHER;
	objof(m)->o_flags = 0;
	objof(m)->o_nrefs = 1;
	rego(objof(m));
	m->wm_menu = menu;
    }
    return m;
}

/*
 * Utility to make structure passed as user data argument to C callbacks
 */
static struct_t *
makecallback(func_t *callback, object_t *cdata)
{
    struct_t	*o;

    NEED_STRINGS(NULL);
    if ((o = new_struct()) != NULL)
    {
	if (assign(o, STRING(fn), callback) || assign(o, STRING(cdata), cdata))
	{
	    decref(o);
	    o = NULL;
	}
    }
    return o;
}

FUNC(AppCreateWithMain)
{
    WMAppContext	*ac;
    ici_X_display_t	*display;
    long		screen;
    ici_X_window_t	*window;
    
    if (ici_typecheck("oio", &display, &screen, &window))
	return 1;
    if (!isdisplay(objof(display)))
	return ici_argerror(0);
    if (!iswindow(objof(window)))
	return ici_argerror(2);
    ac = WMAppCreateWithMain(display->d_display, screen, window->w_window);
    return ici_ret_with_decref(objof(ici_wm_new_appcontext(ac)));
}

FUNC(AppAddWindow)
{
    ici_wm_appcontext_t	*app;
    ici_X_window_t	*window;

    if (ici_typecheck("oo", &app, &window))
	return 1;
    if (!isappcontext(objof(app)))
	return ici_argerror(0);
    if (!iswindow(objof(window)))
	return ici_argerror(1);
    WMAppAddWindow(app->app_context, window->w_window);
    return null_ret();
}

FUNC(ProcessEvent)
{
    ici_wm_appcontext_t	*app;
    ici_X_event_t	*event;

    if (ici_typecheck("oo", &app, &event))
	return 1;
    if (!isappcontext(objof(app)))
	return ici_argerror(0);
    if (!isevent(objof(event)))
	return ici_argerror(1);
    WMProcessEvent(app->app_context, event->e_event);
    return null_ret();
}

FUNC(HideApplication)
{
    ici_wm_appcontext_t	*app;

    if (ici_typecheck("o", &app))
	return 1;
    if (!isappcontext(objof(app)))
	return ici_argerror(0);
    WMHideApplication(app->app_context);
    return null_ret();
}

FUNC(HideOthers)
{
    ici_wm_appcontext_t	*app;

    if (ici_typecheck("o", &app))
	return 1;
    if (!isappcontext(objof(app)))
	return ici_argerror(0);
    WMHideOthers(app->app_context);
    return null_ret();
}

FUNC(AppSetMainMenu)
{
    ici_wm_appcontext_t	*app;
    ici_wm_menu_t	*menu;

    if (ici_typecheck("oo", &app, &menu))
	return 1;
    if (!isappcontext(objof(app)))
	return ici_argerror(0);
    if (!ismenu(objof(menu)))
	return ici_argerror(1);
    return int_ret(WMAppSetMainMenu(app->app_context, menu->wm_menu));
}

FUNC(MenuCreate)
{
    ici_wm_appcontext_t	*app;
    char		*title;
    WMMenu		*menu;

    if (ici_typecheck("os", &app, &title))
	return 1;
    if (!isappcontext(objof(app)))
	return ici_argerror(0);
    menu = WMMenuCreate(app->app_context, title);
    return ici_ret_with_decref(objof(ici_wm_new_menu(menu)));
}

static void
menu_action_callback(void *cdata, int code, Time timestamp)
{
    ici_func
    (
	fetch(objof(cdata), STRING(fn)),
	"oii",
	fetch(objof(cdata), STRING(cdata)),
	code,
	timestamp
    );
}

FUNC(MenuAddItem)
{
    ici_wm_menu_t	*menu;
    char		*text;
    func_t		*action;
    object_t		*cdata;
    char		*rtext;
    struct_t		*o;

    switch (NARGS())
    {
    case 3:
	cdata = objof(&o_null);
	rtext = NULL;
	if (ici_typecheck("oso", &menu, &text, &action))
	    return 1;
	break;

    case 4:
	rtext = NULL;
	if (ici_typecheck("osoo", &menu, &text, &action, &cdata))
	    return 1;
	break;

    default:
	if (ici_typecheck("osoos", &menu, &text, &action, &cdata, &rtext))
	    return 1;
    }
    if (!ismenu(objof(menu)))
	return ici_argerror(0);
    if (!isfunc(objof(action)))
	return ici_argerror(2);
    if ((o = makecallback(action, cdata)) == NULL)
	return 1;
    return int_ret(WMMenuAddItem(menu->wm_menu, text, menu_action_callback, o, free, rtext));
}

FUNC(MenuAddSubmenu)
{
    ici_wm_menu_t	*menu;
    char		*title;
    ici_wm_menu_t	*submenu;

    if (ici_typecheck("oso", &menu, &title, &submenu))
	return 1;
    if (!ismenu(objof(menu)))
	return ici_argerror(0);
    if (!ismenu(objof(submenu)))
	return ici_argerror(2);
    return int_ret(WMMenuAddSubmenu(menu->wm_menu, title, submenu->wm_menu));
}

FUNC(RealizeMenus)
{
    ici_wm_appcontext_t	*app;

    if (ici_typecheck("o", &app))
	return 1;
    if (!isappcontext(objof(app)))
	return ici_argerror(0);
    return int_ret(WMRealizeMenus(app->app_context));
}

/* FUNC(AppCreate) */
/* { */
/*     WMAppContext	*ac; */
/*     ici_X_display_t	*display; */
/*     int			screen; */
/*  */
/*     if (ici_typecheck("oi", &display, &screen)) */
/* 	return 1; */
/*     if (!isdisplay(objof(display))) */
/* 	return ici_argerror(0); */
/*     ac = WMAppCreate(display->d_display, screen); */
/*     return ici_ret_with_decref(objof(ici_wm_new_appcontext(ac))); */
/* } */

/* FUNC(MenuRemoveItem) */
/* { */
/*     ici_wm_menu_t	*menu; */
/*     long		index; */
/*  */
/*     if (ici_typecheck("oi", &menu, &index)) */
/* 	return 1; */
/*     if (!ismenu(objof(menu))) */
/* 	return ici_argerror(0); */
/*     return int_ret(WMMenuRemoveItem(menu->wm_menu, index)); */
/* } */

/* FUNC(MenuSetEnabled) */
/* { */
/*     ici_wm_menu_t	*menu; */
/*     long		index; */
/*     long		enabled; */
/*  */
/*     enabled = 1; */
/*     if */
/*     ( */
/* 	ici_typecheck("oi", &menu, &index) */
/* 	&& */
/* 	ici_typecheck("oii", &menu, &index, &enabled) */
/*     ) */
/* 	return 1; */
/*     if (!ismenu(objof(menu))) */
/* 	return ici_argerror(0); */
/*     WMMenuSetEnabled(menu->wm_menu, index, enabled); */
/*     return null_ret(); */
/* } */

/* FUNC(MenuDestroy) */
/* { */
/*     ici_wm_menu_t	*menu; */
/*     int			submenus; */
/*  */
/*     switch (NARGS()) */
/*     { */
/*     case 1: */
/* 	submenus = 1; */
/* 	if (ici_typecheck("o", &menu)) */
/* 	    return 1; */
/* 	break; */
/*  */
/*     default: */
/* 	if (ici_typecheck("oi", &menu, &submenus)) */
/* 	    return 1; */
/*     } */
/*     if (!ismenu(objof(menu))) */
/* 	return ici_argerror(0); */
/*     WMMenuDestroy(menu->wm_menu, submenus); */
/*     return null_ret(); */
/* } */
