#include "ici.h"
#include "display.h"
#include "window.h"
#include "event.h"
#include "gc.h"
#include "WINGs.h"
#include "wings.h"

NEED_STRING(argv);

int
ici_WM_vars_init(struct_t *s)
{
    long	val;
#define	_(n)	val = (n); if (ici_cmkvar(s, #n, 'i', &val)) return 1
    _(WMNormalWindowLevel);
    _(WMFloatingWindowLevel);
    _(WMDockWindowLevel);
    _(WMSubmenuWindowLevel);
    _(WMMainMenuWindowLevel);
    _(WMBorderlessWindowMask);
    _(WMTitledWindowMask);
    _(WMClosableWindowMask);
    _(WMMiniaturizableWindowMask);
    _(WMResizableWindowMask);
    _(WBTMomentaryPush);
    _(WBTPushOnPushOff);
    _(WBTToggle);
    _(WBTSwitch);
    _(WBTRadio);
    _(WBTMomentaryChange);
    _(WBTOnOff);
    _(WBTMomentaryLight);
    _(WBBSpringLoadedMask);
    _(WBBPushInMask);
    _(WBBPushChangeMask);
    _(WBBPushLightMask);
    _(WBBStateLightMask);
    _(WBBStateChangeMask);
    _(WBBStatePushMask);
    _(WTPNoTitle);
    _(WTPAboveTop);
    _(WTPAtTop);
    _(WTPBelowTop);
    _(WTPAboveBottom);
    _(WTPAtBottom);
    _(WTPBelowBottom);
    _(WRFlat);
    _(WRSimple);
    _(WRRaised);
    _(WRSunken);
    _(WRGroove);
    _(WRRidge);
    _(WRPushed);
    _(WALeft);
    _(WACenter);
    _(WARight);
    _(WIPNoImage);
    _(WIPImageOnly);
    _(WIPLeft);
    _(WIPRight);
    _(WIPBelow);
    _(WIPAbove);
    _(WIPOverlaps);
    _(WSAMaxEnd);
    _(WSAMinEnd);
    _(WSANone);
    _(WSNoPart);
    _(WSDecrementPage);
    _(WSIncrementPage);
    _(WSDecrementLine);
    _(WSIncrementLine);
    _(WSKnob);
    _(WSKnobSlot);
    _(WSUNoParts);
    _(WSUOnlyArrows);
    _(WSUAllParts);
    _(WMRadioMode);
    _(WMHighlightMode);
    _(WMListMode);
    _(WMTrackMode);
    _(WMIllegalTextMovement);
    _(WMReturnTextMovement);
    _(WMTabTextMovement);
    _(WMBacktabTextMovement);
    _(WMLeftTextMovement);
    _(WMRightTextMovement);
    _(WMUpTextMovement);
    _(WMDownTextMovement);
    _(WSIReturnArrow);
    _(WSIHighlightedReturnArrow);
    _(WSIScrollerDimple);
    _(WSIArrowLeft);
    _(WSIHighlightedArrowLeft);
    _(WSIArrowRight);
    _(WSIHighlightedArrowRight);
    _(WSIArrowUp);
    _(WSIHighlightedArrowUp);
    _(WSIArrowDown);
    _(WSIHighlightedArrowDown);
    _(WSICheckMark);
    _(WLDSSelected);
    _(WLDSDisabled);
    _(WLDSFocused);
    _(WLDSIsBranch);
    _(WAPRDefault);
    _(WAPRAlternate);
    _(WAPROther);
    _(WAPRError);
    _(WIReadMask);
    _(WIWriteMask);
    _(WIExceptMask);
    _(WC_Window);
    _(WC_Frame);
    _(WC_Label);
    _(WC_Button);
    _(WC_TextField);
    _(WC_Scroller);
    _(WC_ScrollView);
    _(WC_List);
    _(WC_Browser);
    _(WC_PopUpButton);
    _(WC_ColorWell);
    _(WC_Slider);
    _(WC_Matrix);
    _(WC_SplitView);
#undef _
  return 0;
}

/*
 * Turn an ICI array of strings into an argv-style array. The memory for
 * the argv-style array is allocated (via zalloc()) and must be freed
 * after use. The memory is allocated as one block so a single zfree()
 * of the function result is all that's required.
 */
static char **
ici_array_to_argv(array_t *a)
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
    if ((argv = ici_alloc(sz)) == NULL)
	return NULL;
    p = (char *)&argv[argc];
    for (n = argc, po = a->a_base; n > 0; --n, ++po)
    {
	argv[argc - n] = p;
	memcpy(p, stringof(*po)->s_chars, stringof(*po)->s_nchars + 1);
	p += stringof(*po)->s_nchars + 1;
    }
    argv[argc] = NULL;
    return argv;
}

/*
 * Types
 */

DEFINE_ATOMIC_TYPE(wmcolor, WMColor *, wm_color);
DEFINE_ATOMIC_TYPE(wmhandlerid, WMHandlerID *, wm_handlerid);
DEFINE_ATOMIC_TYPE(wmfont, WMFont *, wm_font);
DEFINE_ATOMIC_TYPE(wmpixmap, WMPixmap *, wm_pixmap);

DEFINE_ATOMIC_TYPE(wmscreen, WMScreen *, wm_screen);
DEFINE_ATOMIC_TYPE(wmview, WMView *, wm_view);
DEFINE_ATOMIC_TYPE(wmwindow, WMWindow *, wm_window);
DEFINE_ATOMIC_TYPE(wmframe, WMFrame *, wm_frame);
DEFINE_ATOMIC_TYPE(wmbutton, WMButton *, wm_button);
DEFINE_ATOMIC_TYPE(wmlabel, WMLabel *, wm_label);
DEFINE_ATOMIC_TYPE(wmtextfield, WMTextField *, wm_textfield);
DEFINE_ATOMIC_TYPE(wmscroller, WMScroller *, wm_scroller);
DEFINE_ATOMIC_TYPE(wmscrollview, WMScrollView *, wm_scrollview);
DEFINE_ATOMIC_TYPE(wmlist, WMList *, wm_list);
DEFINE_ATOMIC_TYPE(wmbrowser, WMBrowser *, wm_browser);
DEFINE_ATOMIC_TYPE(wmpopupbutton, WMPopUpButton *, wm_popupbutton);
DEFINE_ATOMIC_TYPE(wmcolorwell, WMColorWell *, wm_colorwell);
DEFINE_ATOMIC_TYPE(wmslider, WMSlider *, wm_slider);
DEFINE_ATOMIC_TYPE(wmsplitview, WMSplitView *, wm_splitview);

DEFINE_TYPE(rcontext, RContext *, r_context);
DEFINE_TYPE(wmlistitem, WMListItem *, wm_listitem);
DEFINE_TYPE(wmopenpanel, WMOpenPanel *, wm_openpanel);
DEFINE_TYPE(wmsavepanel, WMSavePanel *, wm_savepanel);
DEFINE_TYPE(wmalertpanel, WMAlertPanel *, wm_alertpanel);
DEFINE_TYPE(wminputpanel, WMInputPanel *, wm_inputpanel);
DEFINE_TYPE(wmfontpanel, WMFontPanel *, wm_fontpanel);

static int
iswmfilepanel(object_t *o)
{
    return iswmopenpanel(o) || iswmsavepanel(o);
}

static WMFilePanel *
wmfilepanelof(object_t *o)
{
    if (iswmopenpanel(o))
	return (WMFilePanel *)wmopenpanelof(o)->wm_openpanel;
    if (iswmsavepanel(o))
	return (WMFilePanel *)wmsavepanelof(o)->wm_savepanel;
    error = "object is not a FilePanel";
    return NULL;
}

static WMWidget *
wmwidgetof(object_t *o)
{
#define	HANDLE(TYPE)\
    if (iswm ## TYPE (o))\
	return (WMWidget *)(wm ## TYPE ## of(o)->wm_ ## TYPE)

    HANDLE(screen);
    HANDLE(view);
    HANDLE(window);
    HANDLE(frame);
    HANDLE(button);
    HANDLE(label);
    HANDLE(textfield);
    HANDLE(scroller);
    HANDLE(scrollview);
    HANDLE(list);
    HANDLE(browser);
    HANDLE(popupbutton);
    HANDLE(colorwell);
    HANDLE(slider);
    HANDLE(splitview);

    return NULL;

#undef HANDLE
}

static int
iswmwidget(object_t *o)
{
#define	HANDLE(TYPE)\
    if (iswm ## TYPE (o))\
	return 1

    HANDLE(screen);
    HANDLE(view);
    HANDLE(window);
    HANDLE(frame);
    HANDLE(button);
    HANDLE(label);
    HANDLE(textfield);
    HANDLE(scroller);
    HANDLE(scrollview);
    HANDLE(list);
    HANDLE(browser);
    HANDLE(popupbutton);
    HANDLE(colorwell);
    HANDLE(slider);
    HANDLE(splitview);

    return 0;

#undef HANDLE
}

static object_t *
new_wmwidget(WMWidget *widget)
{
    object_t	**po, *o;

    for
    (
	po = &atoms[ici_atom_hash_index((unsigned long)widget * UNIQUE_PRIME)];
	(o = *po) != NULL;
	--po < atoms ? po = atoms + atomsz - 1 : NULL
    )
    {
	if (iswmwidget(o) && wmwidgetof(o) == widget)
	{
	    incref(o);
	    return o;
	}
    }
    return objof(&o_null);
}

/*
 * Functions
 */

FUNC(WidgetClass)
{
    object_t	*wmwidget;

    if (ici_typecheck("o", &wmwidget))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    return int_ret(WMWidgetClass(wmwidgetof(wmwidget)));
}

FUNC(WidgetView)
{
    object_t	*wmwidget;
    WMView	*view;

    if (ici_typecheck("o", &wmwidget))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    view = WMWidgetView(wmwidgetof(wmwidget));
    return ici_ret_with_decref(objof(new_wmview(view)));
}


FUNC(InitializeApplication)
{
    char	*name;
    char	**argv;
    int		argc;
    array_t	*iciargv;

    NEED_STRINGS(1);

    switch (NARGS())
    {
    case 1:
	if (ici_typecheck("s", &name))
	    return 1;
	iciargv = arrayof(fetch(v_top[-1], STRING(argv)));
	if (objof(iciargv) == objof(&o_null))
	{
	    error = "no argv passed and no argv in scope";
	    return 1;
	}
	break;

    case 2:
	if (ici_typecheck("sa", &name, &iciargv))
	    return 1;

    default:
	return ici_argcount(2);
    }
    argv = ici_array_to_argv(iciargv);
    argc = iciargv->a_top - iciargv->a_base;
    WMInitializeApplication(name, &argc, argv);
    return null_ret();
}

/* FUNC(SetApplicationDataPath) */
/* { */
/*    char	*path; */
/**/
/*    if (ici_typecheck("s", &path)) */
/*	return 1; */
/*    WMSetApplicationDataPath(path); */
/*    return null_ret(); */
/*} */

FUNC(GetApplicationName)
{
    return str_ret(WMGetApplicationName());
}

FUNC(PathForResourceOfType)
{
    char	*resource;
    char	*ext;

    switch (NARGS())
    {
    case 1:
	ext = NULL;
	if (ici_typecheck("s", &resource))
	    return 1;
	break;

    default:
	if (ici_typecheck("ss", &resource, &ext))
	    return 1;
    }
    return str_ret(WMPathForResourceOfType(resource, ext));
}

FUNC(CreateScreenWithRContext)
{
    ici_X_display_t	*display;
    long		screen;
    rcontext_t		*rcontext;
    WMScreen		*wmscreen;

    if (ici_typecheck("oio", &display, &screen, &rcontext))
	return 1;
    if (!isdisplay(objof(display)))
	return ici_argerror(0);
    if (!isrcontext(objof(rcontext)))
	return ici_argerror(2);
    wmscreen = WMCreateScreenWithRContext(display->d_display, screen, rcontext->r_context);
    if (wmscreen == NULL)
    {
	error = "failed to CreateScreenWithRContext";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wmscreen(wmscreen)));
}

FUNC(CreateScreen)
{
    ici_X_display_t	*display;
    long		screen;
    WMScreen		*wmscreen;

    if (ici_typecheck("oi", &display, &screen))
	return 1;
    if (!isdisplay(objof(display)))
	return ici_argerror(0);
    wmscreen = WMCreateScreen(display->d_display, screen);
    if (wmscreen == NULL)
    {
	error = "failed to CreateScreen";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wmscreen(wmscreen)));
}

FUNC(CreateSimpleApplicationScreen)
{
    ici_X_display_t	*display;
    WMScreen		*wmscreen;

    if (ici_typecheck("o", &display))
	return 1;
    if (!isdisplay(objof(display)))
	return ici_argerror(0);
    wmscreen = WMCreateSimpleApplicationScreen(display->d_display);
    if (wmscreen == NULL)
    {
	error = "failed to CreateSimpleApplicationScreen";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wmscreen(wmscreen)));
}

FUNC(ScreenMainLoop)
{
    wmscreen_t	*wmscreen;

    if (ici_typecheck("o", &wmscreen))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    WMScreenMainLoop(wmscreen->wm_screen);
    return null_ret();
}

FUNC(ScreenRContext)
{
    wmscreen_t	*wmscreen;
    RContext	*rcontext;

    if (ici_typecheck("o", &wmscreen))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    rcontext = WMScreenRContext(wmscreen->wm_screen);
    return ici_ret_with_decref(objof(new_rcontext(rcontext)));
}

FUNC(ScreenDisplay)
{
    wmscreen_t	*wmscreen;
    Display	*display;

    if (ici_typecheck("o", &wmscreen))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    display = WMScreenDisplay(wmscreen->wm_screen);
    return ici_ret_with_decref(objof(ici_X_new_display(display)));
}

FUNC(ScreenDepth)
{
    wmscreen_t	*wmscreen;

    if (ici_typecheck("o", &wmscreen))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    return int_ret(WMScreenDepth(wmscreen->wm_screen));
}

FUNC(SetApplicationIconImage)
{
    wmscreen_t	*wmscreen;
    wmpixmap_t	*wmpixmap;

    if (ici_typecheck("oo", &wmscreen, &wmpixmap))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    if (!iswmpixmap(objof(wmpixmap)))
	return ici_argerror(1);
    WMSetApplicationIconImage(wmscreen->wm_screen, wmpixmap->wm_pixmap);
    return null_ret();
}

FUNC(GetApplicationIconImage)
{
    wmscreen_t	*wmscreen;
    WMPixmap	*pixmap;

    if (ici_typecheck("o", &wmscreen))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    pixmap = WMGetApplicationIconImage(wmscreen->wm_screen);
    return ici_ret_with_decref(objof(new_wmpixmap(pixmap)));
}

FUNC(SetFocusToWidget)
{
    object_t	*wmwidget;

    if (ici_typecheck("o", &wmwidget))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    WMSetFocusToWidget(wmwidgetof(wmwidget));
    return null_ret();
}

FUNC(HandleEvent)
{
    ici_X_event_t	*event;
    
    if (ici_typecheck("o", &event))
	return 1;
    if (!isevent(objof(event)))
	return ici_argerror(0);
    return int_ret(WMHandleEvent(event->e_event));
}

static func_t	*event_handler;

FUNC(HookEventHandler)
{
    object_t	*o;
    func_t	*handler;
 
    if (ici_typecheck("o", &handler))
	return 1;
    if (!isfunc(objof(handler)))
	return ici_argerror(0);
    if ((o = objof(event_handler)) != NULL)
	decref(o);
    else
	o = objof(&o_null);
    event_handler = handler;
    incref(event_handler);
    return ici_ret_no_decref(o);
}

FUNC(ScreenPending)
{
    wmscreen_t	*wmscreen;

    if (ici_typecheck("o", &wmscreen))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    return int_ret(WMScreenPending(wmscreen->wm_screen));
}

static void
event_handler_callback(XEvent *event, void *data)
{
    func_t		*fn;
    object_t		*cdata;
    ici_X_event_t	*e;

    e = ici_X_new_event(event);
    fn = funcof(fetch(structof(data), STRING(fn)));
    cdata = fetch(structof(data), STRING(cdata));
    ici_func(objof(fn), "oo", e, cdata);
    decref(e);
}

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

FUNC(CreateEventHandler)
{
    wmview_t	*wmview;
    long	mask;
    func_t	*callback;
    object_t	*cdata;
    struct_t	*o;

    switch (NARGS())
    {
    case 3:
	cdata = objof(&o_null);
	if (ici_typecheck("oio", &wmview, &mask, &callback))
	    return 1;
	break;

    default:
	if (ici_typecheck("oioo", &wmview, &mask, &callback, &cdata))
	    return 1;
    }
    if (!iswmview(objof(wmview)))
	return ici_argerror(0);
    if (!isfunc(objof(callback)))
	return ici_argerror(2);
    if ((o = makecallback(callback, cdata)) == NULL)
	return 1;
    WMCreateEventHandler(wmview->wm_view, mask, event_handler_callback, o);
    return null_ret();
}

FUNC(DeleteEventHandler)
{
    error = "DeleteEventHandler not implemented";
    return 1;
}

FUNC(IsDoubleClick)
{
    ici_X_event_t	*event;

    if (ici_typecheck("o", &event))
	return 1;
    if (!isevent(objof(event)))
	return ici_argerror(0);
    return int_ret(WMIsDoubleClick(event->e_event));
}

FUNC(NextEvent)
{
    ici_X_display_t	*display;
    XEvent		*xevent;

    if (ici_typecheck("o", &display))
	return 1;
    if (!isdisplay(objof(display)))
	return ici_argerror(0);
    if ((xevent = talloc(XEvent)) == NULL)
	return 1;
    WMNextEvent(display->d_display, xevent);
    return ici_ret_with_decref(objof(ici_X_new_event(xevent)));
}

FUNC(MaskEvent)
{
    ici_X_display_t	*display;
    long		mask;
    XEvent		*xevent;

    if (ici_typecheck("oi", &display, &mask))
	return 1;
    if (!isdisplay(objof(display)))
	return ici_argerror(0);
    if ((xevent = talloc(XEvent)) == NULL)
	return 1;
    WMMaskEvent(display->d_display, mask, xevent);
    return ici_ret_with_decref(objof(ici_X_new_event(xevent)));
}

NEED_STRING(fn);
NEED_STRING(cdata);

static struct_t	*timer_handler_map;

static void
timer_handler_callback(void *data)
{
    func_t	*fn;
    object_t	*cdata;

    fn = funcof(fetch(structof(data), STRING(fn)));
    cdata = fetch(structof(data), STRING(cdata));
    ici_func(objof(fn), "o", cdata);
}

static int
register_timer_handler(wmhandlerid_t *id, object_t *o)
{
    if (timer_handler_map == NULL)
    {
	if ((timer_handler_map = new_struct()) == NULL)
	    return 1;
    }
    return assign(timer_handler_map, id, o);
}

FUNC(AddTimerHandler)
{
    long		milliseconds;
    func_t		*callback;
    object_t		*cdata;
    WMHandlerID		id;
    wmhandlerid_t	*wmid;
    struct_t		*o;

    switch (NARGS())
    {
    case 2:
	cdata = objof(&o_null);
	if (ici_typecheck("io", &milliseconds, &callback))
	    return 1;
	break;

    default:
	if (ici_typecheck("ioo", &milliseconds, &callback, &cdata))
	    return 1;
    }
    if (!isfunc(objof(callback)))
	return ici_argerror(1);
    if ((o = makecallback(callback, cdata)) == NULL)
	return 1;
    id = WMAddTimerHandler(milliseconds, timer_handler_callback, o);
    wmid = new_wmhandlerid(id);
    if (register_timer_handler(wmid, objof(o)))
    {
	decref(o);
	return 1;
    }
    return ici_ret_with_decref(objof(wmid));
}

FUNC(DeleteTimerWithClientData)
{
    object_t	*o;
    slot_t	*sl;

    if (ici_typecheck("o", &o))
	return 1;
again:
    for
    (
	sl = timer_handler_map->s_slots;
	sl < timer_handler_map->s_slots + timer_handler_map->s_nslots;
	++sl
    )
    {
	if (sl->sl_key != NULL && fetch(sl->sl_value, STRING(cdata)) == o)
	{
	    WMDeleteTimerWithClientData(sl->sl_value);
	    unassign_struct(timer_handler_map, sl->sl_key);
	    NEXT_VSVER;
	    goto again;
	}
    }
    return null_ret();
}

FUNC(DeleteTimerHandler)
{
    wmhandlerid_t	*wmid;

    if (ici_typecheck("o", &wmid))
	return 1;
    if (!iswmhandlerid(objof(wmid)))
	return ici_argerror(0);
    WMDeleteTimerHandler(wmid->wm_handlerid);
    return null_ret();
}

static struct_t	*idle_handler_map;

static void
idle_handler_callback(void *data)
{
    func_t	*fn;
    object_t	*cdata;

    fn = funcof(fetch(structof(data), STRING(fn)));
    cdata = fetch(structof(data), STRING(cdata));
    ici_func(objof(fn), "o", cdata);
}

static int
register_idle_handler(wmhandlerid_t *id, object_t *o)
{
    if (idle_handler_map == NULL)
    {
	if ((idle_handler_map = new_struct()) == NULL)
	    return 1;
    }
    return assign(idle_handler_map, id, o);
}

static void
delete_idle_handler(wmhandlerid_t *id)
{
    unassign_struct(idle_handler_map, objof(id));
    NEXT_VSVER;
}

FUNC(AddIdleHandler)
{
    func_t		*callback;
    object_t		*cdata;
    WMHandlerID		id;
    wmhandlerid_t	*wmid;
    struct_t		*o;

    switch (NARGS())
    {
    case 1:
	cdata = objof(&o_null);
	if (ici_typecheck("o", &callback))
	    return 1;
	break;

    default:
	if (ici_typecheck("oo", &callback, &cdata))
	    return 1;
    }
    if (!isfunc(objof(callback)))
	return ici_argerror(0);
    if ((o = makecallback(callback, cdata)) == NULL)
	return 1;
    id = WMAddIdleHandler(idle_handler_callback, o);
    wmid = new_wmhandlerid(id);
    if (register_idle_handler(wmid, objof(o)))
    {
	decref(o);
	return 1;
    }
    return ici_ret_with_decref(objof(wmid));
}

FUNC(DeleteIdleHandler)
{
    wmhandlerid_t	*wmid;

    if (ici_typecheck("o", &wmid))
	return 1;
    if (!iswmhandlerid(objof(wmid)))
	return ici_argerror(0);
    WMDeleteIdleHandler(wmid->wm_handlerid);
    delete_idle_handler(wmid);
    return null_ret();
}

static struct_t	*input_handler_map;

static void
input_handler_callback(int fd, int mask, void *data)
{
    func_t	*fn;
    object_t	*cdata;

    fn = funcof(fetch(structof(data), STRING(fn)));
    cdata = fetch(structof(data), STRING(cdata));
    ici_func(objof(fn), "o", cdata);
}

static int
register_input_handler(wmhandlerid_t *id, object_t *o)
{
    if (input_handler_map == NULL)
    {
	if ((input_handler_map = new_struct()) == NULL)
	    return 1;
    }
    return assign(input_handler_map, id, o);
}

static void
delete_input_handler(wmhandlerid_t *id)
{
    unassign_struct(input_handler_map, objof(id));
    NEXT_VSVER;
}

FUNC(AddInputHandler)
{
    long		fd;
    long		condition;
    func_t		*callback;
    object_t		*cdata;
    WMHandlerID		id;
    wmhandlerid_t	*wmid;
    struct_t		*o;

    switch (NARGS())
    {
    case 3:
	cdata = objof(&o_null);
	if (ici_typecheck("iio", &fd, &condition, &callback))
	    return 1;
	break;

    default:
	if (ici_typecheck("iioo", &fd, &condition, &callback, &cdata))
	    return 1;
    }
    if (!isfunc(objof(callback)))
	return ici_argerror(2);
    if ((o = makecallback(callback, cdata)) == NULL)
	return 1;
    id = WMAddInputHandler(fd, condition, input_handler_callback, o);
    wmid = new_wmhandlerid(id);
    if (register_input_handler(wmid, objof(o)))
    {
	decref(o);
	return 1;
    }
    return ici_ret_with_decref(objof(wmid));
}

FUNC(DeleteInputHandler)
{
    wmhandlerid_t	*wmid;

    if (ici_typecheck("o", &wmid))
	return 1;
    if (!iswmhandlerid(objof(wmid)))
	return ici_argerror(0);
    WMDeleteInputHandler(wmid->wm_handlerid);
    delete_input_handler(wmid);
    return null_ret();
}

FUNC(CreateFont)
{
    wmscreen_t	*wmscreen;
    char	*fontname;
    WMFont	*font;

    if (ici_typecheck("os", &wmscreen, &fontname))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    font = WMCreateFont(wmscreen->wm_screen, fontname);
    if (font == NULL)
    {
	error = "failed to CreateFont";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wmfont(font)));
}

#if 0
xFUNC(CreateFontInDefaultEncoding)
{
    wmscreen_t	*wmscreen;
    char	*fontname;
    WMFont	*font;

    if (ici_typecheck("os", &wmscreen, &fontname))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    font = WMCreateFontInDefaultEncoding(wmscreen->wm_screen, fontname);
    if (font == NULL)
    {
	error = "failed to CreateFontInDefaultEncoding";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wmfont(font)));
}
#endif

FUNC(RetainFont)
{
    wmfont_t	*wmfont;

    if (ici_typecheck("o", &wmfont))
	return 1;
    if (!iswmfont(objof(wmfont)))
	return ici_argerror(0);
    WMRetainFont(wmfont->wm_font);
    return ici_ret_no_decref(objof(wmfont));
}

FUNC(ReleaseFont)
{
    wmfont_t	*wmfont;

    if (ici_typecheck("o", &wmfont))
	return 1;
    if (!iswmfont(objof(wmfont)))
	return ici_argerror(0);
    WMReleaseFont(wmfont->wm_font);
    return null_ret();
}

FUNC(FontHeight)
{
    wmfont_t	*wmfont;

    if (ici_typecheck("o", &wmfont))
	return 1;
    if (!iswmfont(objof(wmfont)))
	return ici_argerror(0);
    return int_ret(WMFontHeight(wmfont->wm_font));
}

FUNC(SystemFontOfSize)
{
    wmscreen_t	*wmscreen;
    long	size;
    WMFont	*font;

    if (ici_typecheck("oi", &wmscreen, &size))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    font = WMSystemFontOfSize(wmscreen->wm_screen, size);
    return ici_ret_with_decref(objof(new_wmfont(font)));
}

FUNC(BoldSystemFontOfSize)
{
    wmscreen_t	*wmscreen;
    long	size;
    WMFont	*font;

    if (ici_typecheck("oi", &wmscreen, &size))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    font = WMBoldSystemFontOfSize(wmscreen->wm_screen, size);
    return ici_ret_with_decref(objof(new_wmfont(font)));
}

FUNC(GetFontFontSet)
{
    error = "GetFontFontSet not implemented";
    return 1;
}

FUNC(RetainPixmap)
{
    wmpixmap_t	*wmpixmap;

    if (ici_typecheck("o", &wmpixmap))
	return 1;
    if (!iswmpixmap(objof(wmpixmap)))
	return ici_argerror(0);
    WMRetainPixmap(wmpixmap->wm_pixmap);
    return ici_ret_no_decref(objof(wmpixmap));
}

FUNC(ReleasePixmap)
{
    wmpixmap_t	*wmpixmap;

    if (ici_typecheck("o", &wmpixmap))
	return 1;
    if (!iswmpixmap(objof(wmpixmap)))
	return ici_argerror(0);
    WMReleasePixmap(wmpixmap->wm_pixmap);
    return null_ret();
}

FUNC(CreatePixmapFromRImage)
{
    error = "CreatePixmapFromRImage not implemented";
    return 1;
}

FUNC(CreatePixmapFromXPMData)
{
    error = "CreatePixmapFromXPMData not implemented";
    return 1;
}

NEED_STRING(width);
NEED_STRING(height);

FUNC(GetPixmapSize)
{
    wmpixmap_t	*wmpixmap;
    struct_t	*s;
    int_t	*n;
    WMSize	size;

    if (ici_typecheck("o", &wmpixmap))
	return 1;
    if (!iswmpixmap(objof(wmpixmap)))
	return ici_argerror(0);
    size = WMGetPixmapSize(wmpixmap->wm_pixmap);
    if ((s = new_struct()) == NULL)
	return 1;
    if ((n = new_int(size.width)) == NULL)
    {
	decref(s);
	return 1;
    }
    if (assign(s, STRING(width), n))
    {
	decref(n);
	decref(s);
	return 1;
    }
    decref(n);
    if ((n = new_int(size.height)) == NULL)
    {
	decref(s);
	return 1;
    }
    if (assign(s, STRING(height), n))
    {
	decref(n);
	decref(s);
	return 1;
    }
    decref(n);
    return ici_ret_with_decref(objof(s));
}

FUNC(CreatePixmapFromFile)
{
    wmscreen_t	*wmscreen;
    char	*filename;
    WMPixmap	*pixmap;

    if (ici_typecheck("os", &wmscreen, &filename))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    pixmap = WMCreatePixmapFromFile(wmscreen->wm_screen, filename);
    if (pixmap == NULL)
    {
	error = "failed to CreatePixmapFromFile";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wmpixmap(pixmap)));
}

FUNC(CreateBlendedPixmapFromFile)
{
    error = "CreateBlendedPixmapFromFile not implemented";
    return 1;
}

FUNC(DrawPixmap)
{
    wmpixmap_t	*wmpixmap;
    object_t	*drawable;
    Drawable	d;
    long	x, y;

    if (ici_typecheck("ooii", &wmpixmap, &drawable, &x, &y))
	return 1;
    if (!iswmpixmap(objof(wmpixmap)))
	return ici_argerror(0);
    if (iswindow(drawable))
    {
	d = windowof(drawable)->w_window;
    }
    /*    else */
    else
    {
	return ici_argerror(1);
    }
    WMDrawPixmap(wmpixmap->wm_pixmap, d, x, y);
    return null_ret();
}

FUNC(GetPixmapXID)
{
    error = "GetPixmapXID not implemented";
    return 1;
}

FUNC(GetPixmapMaskXID)
{
    error = "GetPixmapMaskXID not implemented";
    return 1;
}

FUNC(GetSystemPixmap)
{
    wmscreen_t	*wmscreen;
    long	image;
    WMPixmap	*pixmap;

    if (ici_typecheck("oi", &wmscreen, &image))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    pixmap = WMGetSystemPixmap(wmscreen->wm_screen, image);
    return ici_ret_with_decref(objof(new_wmpixmap(pixmap)));
}

FUNC(DarkGrayColor)
{
    wmscreen_t	*wmscreen;
    if (ici_typecheck("o", &wmscreen))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    return ici_ret_with_decref(objof(new_wmcolor(WMDarkGrayColor(wmscreen->wm_screen))));
}

FUNC(GrayColor)
{
    wmscreen_t	*wmscreen;
    if (ici_typecheck("o", &wmscreen))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    return ici_ret_with_decref(objof(new_wmcolor(WMGrayColor(wmscreen->wm_screen))));
}

FUNC(BlackColor)
{
    wmscreen_t	*wmscreen;
    if (ici_typecheck("o", &wmscreen))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    return ici_ret_with_decref(objof(new_wmcolor(WMBlackColor(wmscreen->wm_screen))));
}

FUNC(WhiteColor)
{
    wmscreen_t	*wmscreen;

    if (ici_typecheck("o", &wmscreen))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    return ici_ret_with_decref(objof(new_wmcolor(WMWhiteColor(wmscreen->wm_screen))));
}

FUNC(SetColorInGC)
{
    wmcolor_t	*wmcolor;
    ici_X_gc_t	*gc;

    if (ici_typecheck("oo", &wmcolor, &gc))
	return 1;
    if (!iswmcolor(objof(wmcolor)))
	return ici_argerror(0);
    if (!isgc(objof(gc)))
	return ici_argerror(1);
    WMSetColorInGC(wmcolor->wm_color, gc->g_gc);
    return null_ret();
}

FUNC(ColorGC)
{
    ici_X_display_t	*display;
    wmcolor_t		*wmcolor;
    GC			gc;

    if (ici_typecheck("oo", &display, &wmcolor))
	return 1;
    if (!isdisplay(objof(display)))
	return ici_argerror(0);
    if (!iswmcolor(objof(wmcolor)))
	return ici_argerror(1);
    gc = WMColorGC(wmcolor->wm_color);
    return ici_ret_with_decref(objof(ici_X_new_gc(gc)));
}

FUNC(ColorPixel)
{
    wmcolor_t	*wmcolor;

    if (ici_typecheck("o", &wmcolor))
	return 1;
    if (!iswmcolor(objof(wmcolor)))
	return ici_argerror(0);
    return int_ret(WMColorPixel(wmcolor->wm_color));
}

FUNC(PaintColorSwatch)
{
    wmcolor_t	*wmcolor;
    object_t	*drawable;
    Drawable	d;
    long	x, y;
    long	w, h;

    if (ici_typecheck("ooiiii", &wmcolor, &drawable, &x, &y, &w, &h))
	return 1;
    if (!iswmcolor(objof(wmcolor)))
	return ici_argerror(0);
    if (iswindow(drawable))
    {
	d = windowof(drawable)->w_window;
    }
    /*    else */
    else
    {
	return ici_argerror(1);
    }
    WMPaintColorSwatch(wmcolor->wm_color, d, x, y, w, h);
    return null_ret();
}

FUNC(ReleaseColor)
{
    wmcolor_t	*wmcolor;

    if (ici_typecheck("o", &wmcolor))
	return 1;
    if (!iswmcolor(objof(wmcolor)))
	return ici_argerror(0);
    WMReleaseColor(wmcolor->wm_color);
    return null_ret();
}

FUNC(RetainColor)
{
    wmcolor_t	*wmcolor;

    if (ici_typecheck("o", &wmcolor))
	return 1;
    if (!iswmcolor(objof(wmcolor)))
	return ici_argerror(0);
    WMRetainColor(wmcolor->wm_color);
    return ici_ret_no_decref(objof(wmcolor));
}

FUNC(CreateRGBColor)
{
    wmscreen_t	*wmscreen;
    WMColor	*color;
    long	r, g, b;
    long	exact;

    switch (NARGS())
    {
    case 4:
	exact = 0;
	if (ici_typecheck("oiii", &wmscreen, &r, &g, &b))
	    return 1;
	break;
    default:
	if (ici_typecheck("oiiii", &wmscreen, &r, &g, &b, &exact))
	    return 1;
    }
    color = WMCreateRGBColor(wmscreen->wm_screen, r, g, b, exact);
    if (color == NULL)
    {
	error = "failed to CreateRGBColor";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wmcolor(color)));
}

FUNC(CreateNamedColor)
{
    wmscreen_t	*wmscreen;
    char	*name;
    long	exact;
    WMColor	*color;

    switch (NARGS())
    {
    case 2:
	exact = 0;
	if (ici_typecheck("os", &wmscreen, &name))
	    return 1;
	break;
    default:
	if (ici_typecheck("osi", *wmscreen, &name, &exact))
	    return 1;
    }
    color = WMCreateNamedColor(wmscreen->wm_screen, name, exact);
    if (color == NULL)
    {
	error = "failed to CreateNamedColor";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wmcolor(color)));
}

FUNC(RedComponentOfColor)
{
    wmcolor_t	*wmcolor;

    if (ici_typecheck("o", &wmcolor))
	return 1;
    if (!iswmcolor(objof(wmcolor)))
	return ici_argerror(0);
    return int_ret(WMRedComponentOfColor(wmcolor->wm_color));
}

FUNC(GreenComponentOfColor)
{
    wmcolor_t	*wmcolor;

    if (ici_typecheck("o", &wmcolor))
	return 1;
    if (!iswmcolor(objof(wmcolor)))
	return ici_argerror(0);
    return int_ret(WMGreenComponentOfColor(wmcolor->wm_color));
}

FUNC(BlueComponentOfColor)
{
    wmcolor_t	*wmcolor;

    if (ici_typecheck("o", &wmcolor))
	return 1;
    if (!iswmcolor(objof(wmcolor)))
	return ici_argerror(0);
    return int_ret(WMBlueComponentOfColor(wmcolor->wm_color));
}

FUNC(GetColorRGBDescription)
{
    wmcolor_t	*wmcolor;

    if (ici_typecheck("o", &wmcolor))
	return 1;
    if (!iswmcolor(objof(wmcolor)))
	return ici_argerror(0);
    return str_ret(WMGetColorRGBDescription(wmcolor->wm_color));
}

FUNC(DrawString)
{
    wmscreen_t	*wmscreen;
    object_t	*drawable;
    Drawable	d;
    ici_X_gc_t	*gc;
    wmfont_t	*wmfont;
    long	x, y;
    string_t	*text;

    if (ici_typecheck("ooooiio", &wmscreen, &drawable, &gc, &wmfont, &x, &y, &text))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    if (iswindow(drawable))
    {
	d = windowof(drawable)->w_window;
    }
    /*    else */
    else
    {
	return ici_argerror(1);
    }
    if (!isgc(objof(gc)))
	return ici_argerror(2);
    if (!iswmfont(objof(wmfont)))
	return ici_argerror(3);
    if (!isstring(objof(text)))
	return ici_argerror(6);
    WMDrawString(wmscreen->wm_screen, d, gc->g_gc, wmfont->wm_font, x, y, text->s_chars, text->s_nchars);
    return null_ret();
}

FUNC(DrawImageString)
{
    wmscreen_t	*wmscreen;
    object_t	*drawable;
    Drawable	d;
    ici_X_gc_t	*gc;
    wmfont_t	*wmfont;
    long	x, y;
    string_t	*text;

    if (ici_typecheck("ooooiio", &wmscreen, &drawable, &gc, &wmfont, &x, &y, &text))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    if (iswindow(drawable))
    {
	d = windowof(drawable)->w_window;
    }
    /*    else */
    else
    {
	return ici_argerror(1);
    }
    if (!isgc(objof(gc)))
	return ici_argerror(2);
    if (!iswmfont(objof(wmfont)))
	return ici_argerror(3);
    if (!isstring(objof(text)))
	return ici_argerror(6);
    WMDrawImageString(wmscreen->wm_screen, d, gc->g_gc, wmfont->wm_font, x, y, text->s_chars, text->s_nchars);
    return null_ret();
}

FUNC(WidthOfString)
{
    wmfont_t	*wmfont;
    string_t	*text;

    if (ici_typecheck("oo", &wmfont, &text))
	return 1;
    if (!iswmfont(objof(wmfont)))
	return ici_argerror(0);
    if (!isstring(objof(text)))
	return ici_argerror(1);
    return int_ret(WMWidthOfString(wmfont->wm_font, text->s_chars, text->s_nchars));
}

FUNC(WidgetScreen)
{
    object_t	*wmwidget;
    WMScreen	*screen;

    if (ici_typecheck("o", &wmwidget))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    screen = WMWidgetScreen(wmwidgetof(wmwidget));
    return ici_ret_with_decref(objof(new_wmscreen(screen)));
}

FUNC(UnmapWidget)
{
    object_t	*wmwidget;

    if (ici_typecheck("o", &wmwidget))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    WMUnmapWidget(wmwidgetof(wmwidget));
    return null_ret();
}

FUNC(MapWidget)
{
    object_t	*wmwidget;

    if (ici_typecheck("o", &wmwidget))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    WMMapWidget(wmwidgetof(wmwidget));
    return null_ret();
}

FUNC(MoveWidget)
{
    object_t	*wmwidget;
    long	x, y;

    if (ici_typecheck("oii", &wmwidget, &x, &y))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    WMMoveWidget(wmwidgetof(wmwidget), x, y);
    return null_ret();
}

FUNC(ResizeWidget)
{
    object_t	*wmwidget;
    long	w, h;

    if (ici_typecheck("oii", &wmwidget, &w, &h))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    WMResizeWidget(wmwidgetof(wmwidget), w, h);
    return null_ret();
}

FUNC(SetWidgetBackgroundColor)
{
    object_t	*wmwidget;
    wmcolor_t	*wmcolor;

    if (ici_typecheck("oo", &wmwidget, &wmcolor))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    if (!iswmcolor(objof(wmcolor)))
	return ici_argerror(1);
    WMSetWidgetBackgroundColor(wmwidgetof(wmwidget), wmcolor->wm_color);
    return null_ret();
}

FUNC(MapSubwidgets)
{
    object_t	*wmwidget;

    if (ici_typecheck("o", &wmwidget))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    WMMapSubwidgets(wmwidgetof(wmwidget));
    return null_ret();
}

FUNC(UnmapSubwidgets)
{
    object_t	*wmwidget;

    if (ici_typecheck("o", &wmwidget))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    WMUnmapSubwidgets(wmwidgetof(wmwidget));
    return null_ret();
}

FUNC(RealizeWidget)
{
    object_t	*wmwidget;

    if (ici_typecheck("o", &wmwidget))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    WMRealizeWidget(wmwidgetof(wmwidget));
    return null_ret();
}

FUNC(DestroyWidget)
{
    object_t	*wmwidget;

    if (ici_typecheck("o", &wmwidget))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    WMDestroyWidget(wmwidgetof(wmwidget));
    return null_ret();
}

FUNC(WidgetWidth)
{
    object_t	*wmwidget;

    if (ici_typecheck("o", &wmwidget))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    return int_ret(WMWidgetWidth(wmwidgetof(wmwidget)));
}

FUNC(WidgetHeight)
{
    object_t	*wmwidget;

    if (ici_typecheck("o", &wmwidget))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    return int_ret(WMWidgetHeight(wmwidgetof(wmwidget)));
}

FUNC(WidgetXID)
{
    ici_X_display_t	*display;
    object_t		*wmwidget;
    Window		window;

    if (ici_typecheck("oo", &display, &wmwidget))
	return 1;
    if (!isdisplay(objof(display)))
	return ici_argerror(0);
    if (!iswmwidget(wmwidget))
	return ici_argerror(1);
    window = WMWidgetXID(wmwidgetof(wmwidget));
    return ici_ret_with_decref(objof(ici_X_new_window(window)));
}

FUNC(RedisplayWidget)
{
    object_t	*wmwidget;

    if (ici_typecheck("o", &wmwidget))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    WMRedisplayWidget(wmwidgetof(wmwidget));
    return null_ret();
}

FUNC(CreateWindow)
{
    wmscreen_t	*wmscreen;
    char	*name;
    WMWindow	*window;

    if (ici_typecheck("os", &wmscreen, &name))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    window = WMCreateWindow(wmscreen->wm_screen, name);
    if (window == NULL)
    {
	error = "failed to CreateWindow";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wmwindow(window)));
}

FUNC(CreateWindowWithStyle)
{
    wmscreen_t	*wmscreen;
    char	*name;
    long	style;
    WMWindow	*window;

    if (ici_typecheck("osi", &wmscreen, &name, &style))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    window = WMCreateWindowWithStyle(wmscreen->wm_screen, name, style);
    if (window == NULL)
    {
	error = "failed to CreateWindowWithStyle";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wmwindow(window)));
}

FUNC(CreatePanelWithStyleForWindow)
{
    wmwindow_t	*wmwindow;
    char	*name;
    long	style;
    WMWindow	*window;

    if (ici_typecheck("osi", &wmwindow, &name, &style))
	return 1;
    if (!iswmwindow(objof(wmwindow)))
	return ici_argerror(0);
    window = WMCreatePanelWithStyleForWindow(wmwindow->wm_window, name, style);
    if (window == NULL)
    {
	error = "failed to CreatePanelWithStyleForWindow";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wmwindow(window)));
}

FUNC(CreatePanelForWindow)
{
    wmwindow_t	*wmwindow;
    char	*name;
    WMWindow	*window;

    if (ici_typecheck("os", &wmwindow, &name))
	return 1;
    if (!iswmwindow(objof(wmwindow)))
	return ici_argerror(0);
    window = WMCreatePanelForWindow(wmwindow->wm_window, name);
    if (window == NULL)
    {
	error = "failed to CreatePanelForWindow";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wmwindow(window)));
}

FUNC(ChangePanelOwner)
{
    wmwindow_t	*a;
    wmwindow_t	*b;

    if (ici_typecheck("oo", &a, &b))
	return 1;
    if (!iswmwindow(objof(a)))
	return ici_argerror(0);
    if (!iswmwindow(objof(b)))
	return ici_argerror(1);
    WMChangePanelOwner(a->wm_window, b->wm_window);
    return null_ret();
}

FUNC(SetWindowTitle)
{
    wmwindow_t	*wmwindow;
    char	*title;

    if (ici_typecheck("os", &wmwindow, &title))
	return 1;
    if (!iswmwindow(objof(wmwindow)))
	return ici_argerror(0);
    WMSetWindowTitle(wmwindow->wm_window, title);
    return null_ret();
}

FUNC(SetWindowMiniwindowTitle)
{
    wmwindow_t	*wmwindow;
    char	*title;

    if (ici_typecheck("os", &wmwindow, &title))
	return 1;
    if (!iswmwindow(objof(wmwindow)))
	return ici_argerror(0);
    WMSetWindowMiniwindowTitle(wmwindow->wm_window, title);
    return null_ret();
}

FUNC(SetWindowMiniwindowImage)
{
    wmwindow_t	*wmwindow;
    wmpixmap_t	*wmpixmap;

    if (ici_typecheck("oo", &wmwindow, &wmpixmap))
	return 1;
    if (!iswmwindow(objof(wmwindow)))
	return ici_argerror(0);
    if (!iswmpixmap(objof(wmpixmap)))
	return ici_argerror(1);
    WMSetWindowMiniwindowImage(wmwindow->wm_window, wmpixmap->wm_pixmap);
    return null_ret();
}

static void
close_callback(WMWidget *self, void *data)
{
    func_t	*fn;
    object_t	*cdata;

    fn = funcof(fetch(structof(data), STRING(fn)));
    cdata = fetch(structof(data), STRING(cdata));
    ici_func(objof(fn), "oo", objof(new_wmwidget(self)), cdata);
}

FUNC(SetWindowCloseAction)
{
    struct_t	*o;
    wmwindow_t	*wmwindow;
    func_t	*callback;
    object_t	*cdata;

    switch (NARGS())
    {
    case 2:
	cdata = objof(&o_null);
	if (ici_typecheck("oo", &wmwindow, &callback))
	    return 1;
	break;
    default:
	if (ici_typecheck("ooo", &wmwindow, &callback, &cdata))
	    return 1;
    }
    if (!iswmwindow(objof(wmwindow)))
	return ici_argerror(0);
    if (!isfunc(objof(callback)))
	return ici_argerror(1);
    if ((o = makecallback(callback, cdata)) == NULL)
	return 1;
    WMSetWindowCloseAction(wmwindow->wm_window, close_callback, o);
    return null_ret();
}

FUNC(SetWindowMaxSize)
{
    wmwindow_t	*wmwindow;
    long	w, h;

    if (ici_typecheck("oii", &wmwindow, &w, &h))
	return 1;
    if (!iswmwindow(objof(wmwindow)))
	return ici_argerror(0);
    WMSetWindowMaxSize(wmwindow->wm_window, w, h);
    return null_ret();
}

FUNC(SetWindowMinSize)
{
    wmwindow_t	*wmwindow;
    long	w, h;

    if (ici_typecheck("oii", &wmwindow, &w, &h))
	return 1;
    if (!iswmwindow(objof(wmwindow)))
	return ici_argerror(0);
    WMSetWindowMinSize(wmwindow->wm_window, w, h);
    return null_ret();
}

FUNC(SetWindowBaseSize)
{
    wmwindow_t	*wmwindow;
    long	w, h;

    if (ici_typecheck("oii", &wmwindow, &w, &h))
	return 1;
    if (!iswmwindow(objof(wmwindow)))
	return ici_argerror(0);
    WMSetWindowBaseSize(wmwindow->wm_window, w, h);
    return null_ret();
}

FUNC(SetWindowResizeIncrements)
{
    wmwindow_t	*wmwindow;
    long	w, h;

    if (ici_typecheck("oii", &wmwindow, &w, &h))
	return 1;
    if (!iswmwindow(objof(wmwindow)))
	return ici_argerror(0);
    WMSetWindowResizeIncrements(wmwindow->wm_window, w, h);
    return null_ret();
}

FUNC(SetWindowLevel)
{
    wmwindow_t	*wmwindow;
    long	level;

    if (ici_typecheck("oi", &wmwindow, &level))
	return 1;
    if (!iswmwindow(objof(wmwindow)))
	return ici_argerror(0);
    WMSetWindowLevel(wmwindow->wm_window, level);
    return null_ret();
}

FUNC(SetWindowDocumentEdited)
{
    wmwindow_t	*wmwindow;
    long	flag;

    if (ici_typecheck("oi", &wmwindow, &flag))
	return 1;
    if (!iswmwindow(objof(wmwindow)))
	return ici_argerror(0);
    WMSetWindowDocumentEdited(wmwindow->wm_window, flag);
    return null_ret();
}

FUNC(CloseWindow)
{
    wmwindow_t	*wmwindow;

    if (ici_typecheck("o", &wmwindow))
	return 1;
    if (!iswmwindow(objof(wmwindow)))
	return ici_argerror(0);
    WMCloseWindow(wmwindow->wm_window);
    return null_ret();
}

static void
button_action_callback(WMWidget *self, void *data)
{
    func_t	*fn;
    object_t	*cdata;
    object_t	*wmwidget;

    if ((wmwidget = new_wmwidget(self)) != NULL)
    {
	fn = funcof(fetch(structof(data), STRING(fn)));
	cdata = fetch(structof(data), STRING(cdata));
	ici_func(objof(fn), "oo", wmwidget, cdata);
	decref(wmwidget);
    }
}

FUNC(SetButtonAction)
{
    wmbutton_t	*wmbutton;
    func_t	*callback;
    object_t	*cdata;
    struct_t	*o;

    switch (NARGS())
    {
    case 2:
	cdata = objof(&o_null);
	if (ici_typecheck("oo", &wmbutton, &callback))
	    return 1;
	break;

    default:
	if (ici_typecheck("ooo", &wmbutton, &callback, &cdata))
	    return 1;
    }
    if (!iswmbutton(objof(wmbutton)))
	return ici_argerror(0);
    if (!isfunc(objof(callback)))
	return ici_argerror(1);
    if ((o = makecallback(callback, cdata)) == NULL)
	return 1;
    WMSetButtonAction(wmbutton->wm_button, button_action_callback, o);
    return null_ret();
}

FUNC(CreateButton)
{
    object_t	*wmwidget;
    long	type;
    WMButton	*button;

    if (ici_typecheck("oi", &wmwidget, &type))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    button = WMCreateButton(wmwidgetof(wmwidget), type);
    if (button == NULL)
    {
	error = "failed to CreateButton";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wmbutton(button)));
}

FUNC(CreateCustomButton)
{
    object_t	*wmwidget;
    long	behaviourmask;
    WMButton	*button;

    if (ici_typecheck("oi", &wmwidget, &behaviourmask))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    button = WMCreateCustomButton(wmwidgetof(wmwidget), behaviourmask);
    if (button == NULL)
    {
	error = "failed to CreateCustomButton";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wmbutton(button)));
}

FUNC(SetButtonImage)
{
    wmbutton_t	*wmbutton;
    wmpixmap_t	*wmpixmap;

    if (ici_typecheck("oo", &wmbutton, &wmpixmap))
	return 1;
    if (!iswmbutton(objof(wmbutton)))
	return ici_argerror(0);
    if (!iswmpixmap(objof(wmpixmap)))
	return ici_argerror(1);
    WMSetButtonImage(wmbutton->wm_button, wmpixmap->wm_pixmap);
    return null_ret();
}

FUNC(SetButtonAltImage)
{
    wmbutton_t	*wmbutton;
    wmpixmap_t	*wmpixmap;

    if (ici_typecheck("oo", &wmbutton, &wmpixmap))
	return 1;
    if (!iswmbutton(objof(wmbutton)))
	return ici_argerror(0);
    if (!iswmpixmap(objof(wmpixmap)))
	return ici_argerror(1);
    WMSetButtonAltImage(wmbutton->wm_button, wmpixmap->wm_pixmap);
    return null_ret();
}

FUNC(SetButtonImagePosition)
{
    wmbutton_t	*wmbutton;
    long	position;

    if (ici_typecheck("oi", &wmbutton, &position))
	return 1;
    if (!iswmbutton(objof(wmbutton)))
	return ici_argerror(0);
    WMSetButtonImagePosition(wmbutton->wm_button, position);
    return null_ret();
}

FUNC(SetButtonFont)
{
    wmbutton_t	*wmbutton;
    wmfont_t	*wmfont;

    if (ici_typecheck("oo", &wmbutton, &wmfont))
	return 1;
    if (!iswmbutton(objof(wmbutton)))
	return ici_argerror(0);
    if (!iswmfont(objof(wmfont)))
	return ici_argerror(1);
    WMSetButtonFont(wmbutton->wm_button, wmfont->wm_font);
    return null_ret();
}

FUNC(SetButtonTextAlignment)
{
    wmbutton_t	*wmbutton;
    long	alignment;

    if (ici_typecheck("oi", &wmbutton, &alignment))
	return 1;
    if (!iswmbutton(objof(wmbutton)))
	return ici_argerror(0);
    WMSetButtonTextAlignment(wmbutton->wm_button, alignment);
    return null_ret();
}

FUNC(SetButtonText)
{
    wmbutton_t	*wmbutton;
    char	*text;

    if (ici_typecheck("os", &wmbutton, &text))
	return 1;
    if (!iswmbutton(objof(wmbutton)))
	return ici_argerror(0);
    WMSetButtonText(wmbutton->wm_button, text);
    return null_ret();
}

FUNC(SetButtonAltText)
{
    wmbutton_t	*wmbutton;
    char	*text;

    if (ici_typecheck("os", &wmbutton, &text))
	return 1;
    if (!iswmbutton(objof(wmbutton)))
	return ici_argerror(0);
    WMSetButtonAltText(wmbutton->wm_button, text);
    return null_ret();
}

FUNC(SetButtonSelected)
{
    wmbutton_t	*wmbutton;
    long	selected;

    if (ici_typecheck("oi", &wmbutton, &selected))
	return 1;
    if (!iswmbutton(objof(wmbutton)))
	return ici_argerror(0);
    WMSetButtonSelected(wmbutton->wm_button, selected);
    return null_ret();
}

FUNC(GetButtonSelected)
{
    wmbutton_t	*wmbutton;

    if (ici_typecheck("o", &wmbutton))
	return 1;
    if (!iswmbutton(objof(wmbutton)))
	return ici_argerror(0);
    return int_ret(WMGetButtonSelected(wmbutton->wm_button));
}

FUNC(SetButtonBordered)
{
    wmbutton_t	*wmbutton;
    long	bordered;

    if (ici_typecheck("oi", &wmbutton, &bordered))
	return 1;
    if (!iswmbutton(objof(wmbutton)))
	return ici_argerror(0);
    WMSetButtonBordered(wmbutton->wm_button, bordered);
    return null_ret();
}

FUNC(SetButtonEnabled)
{
    wmbutton_t	*wmbutton;
    long	enabled;

    if (ici_typecheck("oi", &wmbutton, &enabled))
	return 1;
    if (!iswmbutton(objof(wmbutton)))
	return ici_argerror(0);
    WMSetButtonEnabled(wmbutton->wm_button, enabled);
    return null_ret();
}

FUNC(SetButtonTag)
{
    wmbutton_t	*wmbutton;
    long	tag;

    if (ici_typecheck("oi", &wmbutton, &tag))
	return 1;
    if (!iswmbutton(objof(wmbutton)))
	return ici_argerror(0);
    WMSetButtonTag(wmbutton->wm_button, tag);
    return null_ret();
}

FUNC(GroupButtons)
{
    wmbutton_t	*wmbutton1;
    wmbutton_t	*wmbutton2;

    if (ici_typecheck("oo", &wmbutton1, &wmbutton2))
	return 1;
    if (!iswmbutton(objof(wmbutton1)))
	return ici_argerror(0);
    if (!iswmbutton(objof(wmbutton2)))
	return ici_argerror(1);
    WMGroupButtons(wmbutton1->wm_button, wmbutton2->wm_button);
    return null_ret();
}

FUNC(PerformButtonClick)
{
    wmbutton_t	*wmbutton;

    if (ici_typecheck("o", &wmbutton))
	return 1;
    if (!iswmbutton(objof(wmbutton)))
	return ici_argerror(0);
    WMPerformButtonClick(wmbutton->wm_button);
    return null_ret();
}

FUNC(SetButtonContinuous)
{
    wmbutton_t	*wmbutton;
    long	flag;

    if (ici_typecheck("oi", &wmbutton, &flag))
	return 1;
    if (!iswmbutton(objof(wmbutton)))
	return ici_argerror(0);
    WMSetButtonContinuous(wmbutton->wm_button, flag);
    return null_ret();
}

FUNC(SetButtonPeriodicDelay)
{
    wmbutton_t	*wmbutton;
    double	delay;
    double	interval;

    if (ici_typecheck("onn", &wmbutton, &delay, &interval))
	return 1;
    if (!iswmbutton(objof(wmbutton)))
	return ici_argerror(0);
    WMSetButtonPeriodicDelay(wmbutton->wm_button, delay, interval);
    return null_ret();
}

FUNC(WMCreateLabel)
{
    object_t	*wmwidget;
    WMLabel	*label;

    if (ici_typecheck("o", &wmwidget))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    label = WMCreateLabel(wmwidgetof(wmwidget));
    return ici_ret_with_decref(objof(new_wmlabel(label)));
}

FUNC(SetLabelWraps)
{
    wmlabel_t	*wmlabel;
    long	flag;

    if (ici_typecheck("oi", &wmlabel, &flag))
	return 1;
    if (!iswmlabel(objof(wmlabel)))
	return ici_argerror(0);
    WMSetLabelWraps(wmlabel->wm_label, flag);
    return null_ret();
}

FUNC(SetLabelImage)
{
    wmlabel_t	*wmlabel;
    wmpixmap_t	*wmpixmap;

    if (ici_typecheck("oo", &wmlabel, &wmpixmap))
	return 1;
    if (!iswmlabel(objof(wmlabel)))
	return ici_argerror(0);
    if (!iswmpixmap(objof(wmpixmap)))
	return ici_argerror(1);
    WMSetLabelImage(wmlabel->wm_label, wmpixmap->wm_pixmap);
    return null_ret();
}

FUNC(GetLabelImage)
{
    wmlabel_t	*wmlabel;
    WMPixmap	*pixmap;

    if (ici_typecheck("o", &wmlabel))
	return 1;
    if (!iswmlabel(objof(wmlabel)))
	return ici_argerror(0);
    pixmap = WMGetLabelImage(wmlabel->wm_label);
    return ici_ret_with_decref(objof(new_wmpixmap(pixmap)));
}

FUNC(SetLabelImagePosition)
{
    wmlabel_t	*wmlabel;
    long	position;

    if (ici_typecheck("oi", &wmlabel, &position))
	return 1;
    if (!iswmlabel(objof(wmlabel)))
	return ici_argerror(0);
    WMSetLabelImagePosition(wmlabel->wm_label, position);
    return null_ret();
}

FUNC(SetLabelTextAlignment)
{
    wmlabel_t	*wmlabel;
    long	alignment;

    if (ici_typecheck("oi", &wmlabel, &alignment))
	return 1;
    if (!iswmlabel(objof(wmlabel)))
	return ici_argerror(0);
    WMSetLabelTextAlignment(wmlabel->wm_label, alignment);
    return null_ret();
}

FUNC(SetLabelRelief)
{
    wmlabel_t	*wmlabel;
    long	relief;

    if (ici_typecheck("oi", &wmlabel, &relief))
	return 1;
    if (!iswmlabel(objof(wmlabel)))
	return ici_argerror(0);
    WMSetLabelRelief(wmlabel->wm_label, relief);
    return null_ret();
}

FUNC(SetLabelText)
{
    wmlabel_t	*wmlabel;
    char	*text;

    if (ici_typecheck("os", &wmlabel, &text))
	return 1;
    if (!iswmlabel(objof(wmlabel)))
	return ici_argerror(0);
    WMSetLabelText(wmlabel->wm_label, text);
    return null_ret();
}

FUNC(SetLabelFont)
{
    wmlabel_t	*wmlabel;
    wmfont_t	*wmfont;

    if (ici_typecheck("oo", &wmlabel, &wmfont))
	return 1;
    if (!iswmlabel(objof(wmlabel)))
	return ici_argerror(0);
    if (!iswmfont(objof(wmfont)))
	return ici_argerror(1);
    WMSetLabelFont(wmlabel->wm_label, wmfont->wm_font);
    return null_ret();
}

FUNC(SetLabelTextColor)
{
    wmlabel_t	*wmlabel;
    wmcolor_t	*wmcolor;

    if (ici_typecheck("oo", &wmlabel, &wmcolor))
	return 1;
    if (!iswmlabel(objof(wmlabel)))
	return ici_argerror(0);
    if (!iswmcolor(objof(wmcolor)))
	return ici_argerror(1);
    WMSetLabelTextColor(wmlabel->wm_label, wmcolor->wm_color);
    return null_ret();
}

FUNC(CreateFrame)
{
    object_t	*wmwidget;
    WMFrame	*frame;

    if (ici_typecheck("o", &wmwidget))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    frame = WMCreateFrame(wmwidgetof(wmwidget));
    if (frame == NULL)
    {
	error = "failed to CreateFrame";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wmframe(frame)));
}

FUNC(SetFrameTitlePosition)
{
    wmframe_t	*wmframe;
    long	position;

    if (ici_typecheck("oi", &wmframe, &position))
	return 1;
    if (!iswmframe(objof(wmframe)))
	return ici_argerror(0);
    WMSetFrameTitlePosition(wmframe->wm_frame, position);
    return null_ret();
}

FUNC(SetFrameRelief)
{
    wmframe_t	*wmframe;
    long	relief;

    if (ici_typecheck("oi", &wmframe, &relief))
	return 1;
    if (!iswmframe(objof(wmframe)))
	return ici_argerror(0);
    WMSetFrameRelief(wmframe->wm_frame, relief);
    return null_ret();
}

FUNC(SetFrameTitle)
{
    wmframe_t	*wmframe;
    char	*title;

    if (ici_typecheck("os", &wmframe, &title))
	return 1;
    if (!iswmframe(objof(wmframe)))
	return ici_argerror(0);
    WMSetFrameTitle(wmframe->wm_frame, title);
    return null_ret();
}

FUNC(CreateTextField)
{
    object_t	*wmwidget;
    WMTextField	*textfield;

    if (ici_typecheck("o", &wmwidget))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    textfield = WMCreateTextField(wmwidgetof(wmwidget));
    if (textfield == NULL)
    {
	error = "failed to CreateTextField";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wmtextfield(textfield)));
}

FUNC(InsertTextFieldText)
{
    wmtextfield_t	*wmtextfield;
    char		*text;
    long		position;

    if (ici_typecheck("osi", &wmtextfield, &text, &position))
	return 1;
    if (!iswmtextfield(objof(wmtextfield)))
	return ici_argerror(0);
    WMInsertTextFieldText(wmtextfield->wm_textfield, text, position);
    return null_ret();
}

NEED_STRING(position);
NEED_STRING(count);

FUNC(DeleteTextFieldRange)
{
    wmtextfield_t	*wmtextfield;
    struct_t		*range;
    WMRange		wmrange;
    object_t		*o;

    NEED_STRINGS(1);
    if (ici_typecheck("od", &wmtextfield, &range))
	return 1;
    if (!iswmtextfield(objof(wmtextfield)))
	return ici_argerror(0);
    if (!isint(o = fetch(range, STRING(position))))
	goto fail;
    wmrange.position = intof(o)->i_value;
    if (!isint(o = fetch(range, STRING(count))))
	goto fail;
    wmrange.count = intof(o)->i_value;
    WMDeleteTextFieldRange(wmtextfield->wm_textfield, wmrange);
    return null_ret();

fail:
    error = "invalid range struct";
    return 1;
}

FUNC(GetTextFieldText)
{
    wmtextfield_t	*wmtextfield;

    if (ici_typecheck("o", &wmtextfield))
	return 1;
    if (!iswmtextfield(objof(wmtextfield)))
	return ici_argerror(0);
    return str_ret(WMGetTextFieldText(wmtextfield->wm_textfield));
}

FUNC(SetTextFieldText)
{
    wmtextfield_t	*wmtextfield;
    char		*text;
    
    if (ici_typecheck("os", &wmtextfield, &text))
	return 1;
    if (!iswmtextfield(objof(wmtextfield)))
	return ici_argerror(0);
    WMSetTextFieldText(wmtextfield->wm_textfield, text);
    return null_ret();
}

FUNC(SetTextFieldAlignment)
{
    wmtextfield_t	*wmtextfield;
    long		alignment;
    
    if (ici_typecheck("oi", &wmtextfield, &alignment))
	return 1;
    if (!iswmtextfield(objof(wmtextfield)))
	return ici_argerror(0);
    WMSetTextFieldAlignment(wmtextfield->wm_textfield, alignment);
    return null_ret();
}

FUNC(SetTextFieldBordered)
{
    wmtextfield_t	*wmtextfield;
    long		bordered;
    
    if (ici_typecheck("oi", &wmtextfield, &bordered))
	return 1;
    if (!iswmtextfield(objof(wmtextfield)))
	return ici_argerror(0);
    WMSetTextFieldBordered(wmtextfield->wm_textfield, bordered);
    return null_ret();
}

FUNC(SetTextFieldEditable)
{
    wmtextfield_t	*wmtextfield;
    long		enabled;
    
    if (ici_typecheck("oi", &wmtextfield, &enabled))
	return 1;
    if (!iswmtextfield(objof(wmtextfield)))
	return ici_argerror(0);
    WMSetTextFieldEditable(wmtextfield->wm_textfield, enabled);
    return null_ret();
}

FUNC(SetTextFieldSecure)
{
    wmtextfield_t	*wmtextfield;
    long		secure;
    
    if (ici_typecheck("oi", &wmtextfield, &secure))
	return 1;
    if (!iswmtextfield(objof(wmtextfield)))
	return ici_argerror(0);
    WMSetTextFieldSecure(wmtextfield->wm_textfield, secure);
    return null_ret();
}

FUNC(CreateScroller)
{
    object_t	*wmwidget;
    WMScroller	*scroller;

    if (ici_typecheck("o", &wmwidget))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    scroller = WMCreateScroller(wmwidgetof(wmwidget));
    if (scroller == NULL)
    {
	error = "failed to CreateScroller";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wmscroller(scroller)));
}

FUNC(SetScrollerParameters)
{
    wmscroller_t	*wmscroller;
    double		floatvalue;
    double		knobproportion;

    if (ici_typecheck("onn", &wmscroller, &floatvalue, &knobproportion))
	return 1;
    if (!iswmscroller(objof(wmscroller)))
	return ici_argerror(0);
    WMSetScrollerParameters(wmscroller->wm_scroller, floatvalue, knobproportion);
    return null_ret();
}

FUNC(GetScrollerKnobProportion)
{
    wmscroller_t	*wmscroller;

    if (ici_typecheck("o", &wmscroller))
	return 1;
    if (!iswmscroller(objof(wmscroller)))
	return ici_argerror(0);
    return float_ret(WMGetScrollerKnobProportion(wmscroller->wm_scroller));
}

FUNC(GetScrollerValue)
{
    wmscroller_t	*wmscroller;

    if (ici_typecheck("o", &wmscroller))
	return 1;
    if (!iswmscroller(objof(wmscroller)))
	return ici_argerror(0);
    return float_ret(WMGetScrollerValue(wmscroller->wm_scroller));
}

FUNC(GetScrollerHitPart)
{
    wmscroller_t	*wmscroller;

    if (ici_typecheck("o", &wmscroller))
	return 1;
    if (!iswmscroller(objof(wmscroller)))
	return ici_argerror(0);
    return int_ret(WMGetScrollerHitPart(wmscroller->wm_scroller));
}

static void
scroller_action_callback(WMWidget *self, void *data)
{
    func_t	*fn;
    object_t	*cdata;
    object_t	*wmwidget;

    if ((wmwidget = new_wmwidget(self)) != NULL)
    {
	fn = funcof(fetch(structof(data), STRING(fn)));
	cdata = fetch(structof(data), STRING(cdata));
	ici_func(objof(fn), "oo", wmwidget, cdata);
	decref(wmwidget);
    }
}

FUNC(SetScrollerAction)
{
    wmscroller_t	*wmscroller;
    func_t		*callback;
    object_t		*cdata;
    struct_t		*o;

    switch (NARGS())
    {
    case 2:
	cdata = objof(&o_null);
	if (ici_typecheck("oo", &wmscroller, &callback))
	    return 1;
	break;

    default:
	if (ici_typecheck("ooo", &wmscroller, &callback, &cdata))
	    return 1;
    }
    if (!iswmscroller(objof(wmscroller)))
	return ici_argerror(0);
    if (!isfunc(objof(callback)))
	return ici_argerror(1);
    if ((o = makecallback(callback, cdata)) == NULL)
	return 1;
    WMSetScrollerAction(wmscroller->wm_scroller, scroller_action_callback, o);
    return null_ret();
}

FUNC(SetScrollerArrowsPosition)
{
    wmscroller_t	*wmscroller;
    long		position;

    if (ici_typecheck("oi", &wmscroller, &position))
	return 1;
    if (!iswmscroller(objof(wmscroller)))
	return ici_argerror(0);
    WMSetScrollerArrowsPosition(wmscroller->wm_scroller, position);
    return null_ret();
}

FUNC(CreateList)
{
    object_t	*wmwidget;
    WMList	*list;

    if (ici_typecheck("o", &wmwidget))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    list = WMCreateList(wmwidgetof(wmwidget));
    if (list == NULL)
    {
	error = "failed to CreateList";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wmlist(list)));
}

FUNC(InsertListItem)
{
    wmlist_t	*wmlist;
    long	row;
    char	*text;
    WMListItem	*listitem;

    if (ici_typecheck("ois", &wmlist, &row, &text))
	return 1;
    if (!iswmlist(objof(wmlist)))
	return ici_argerror(0);
    listitem = WMInsertListItem(wmlist->wm_list, row, text);
    return ici_ret_with_decref(objof(new_wmlistitem(listitem)));
}

#if 0
xFUNC(AddSortedListItem)
{
    wmlist_t	*wmlist;
    char	*text;
    WMListItem	*listitem;

    if (ici_typecheck("os", &wmlist, &text))
	return 1;
    if (!iswmlist(objof(wmlist)))
	return ici_argerror(0);
    listitem = WMAddSortedListItem(wmlist->wm_list, text);
    return ici_ret_with_decref(objof(new_wmlistitem(listitem)));
}
#endif

FUNC(FindRowOfListItemWithTitle)
{
    wmlist_t	*wmlist;
    char	*text;

    if (ici_typecheck("os", &wmlist, &text))
	return 1;
    if (!iswmlist(objof(wmlist)))
	return ici_argerror(0);
    return int_ret(WMFindRowOfListItemWithTitle(wmlist->wm_list, text));
}

FUNC(GetListItem)
{
    wmlist_t	*wmlist;
    long	row;
    WMListItem	*listitem;

    if (ici_typecheck("oi", &wmlist, &row))
	return 1;
    if (!iswmlist(objof(wmlist)))
	return ici_argerror(0);
    listitem = WMGetListItem(wmlist->wm_list, row);
    return ici_ret_with_decref(objof(new_wmlistitem(listitem)));
}

FUNC(RemoveListItem)
{
    wmlist_t	*wmlist;
    long	row;

    if (ici_typecheck("oi", &wmlist, &row))
	return 1;
    if (!iswmlist(objof(wmlist)))
	return ici_argerror(0);
    WMRemoveListItem(wmlist->wm_list, row);
    return null_ret();
}

FUNC(SelectListItem)
{
    wmlist_t	*wmlist;
    long	row;

    if (ici_typecheck("oi", &wmlist, &row))
	return 1;
    if (!iswmlist(objof(wmlist)))
	return ici_argerror(0);
    WMSelectListItem(wmlist->wm_list, row);
    return null_ret();
}

FUNC(SetListUserDrawProc)
{
    error = "SetListUserDrawProc not implemented";
    return 1;
}

FUNC(SetListUserDrawItemHeight)
{
    error = "SetListUserDrawItemHeight not implemented";
    return 1;
}

FUNC(GetListItemHeight)
{
    wmlist_t	*wmlist;

    if (ici_typecheck("o", &wmlist))
	return 1;
    if (!iswmlist(objof(wmlist)))
	return ici_argerror(0);
    return int_ret(WMGetListItemHeight(wmlist->wm_list));
}

FUNC(GetListSelectedItem)
{
    wmlist_t	*wmlist;
    WMListItem	*listitem;

    if (ici_typecheck("o", &wmlist))
	return 1;
    if (!iswmlist(objof(wmlist)))
	return ici_argerror(0);
    listitem = WMGetListSelectedItem(wmlist->wm_list);
    return ici_ret_with_decref(objof(new_wmlistitem(listitem)));
}

FUNC(GetListSelectedItemRow)
{
    wmlist_t	*wmlist;

    if (ici_typecheck("o", &wmlist))
	return 1;
    if (!iswmlist(objof(wmlist)))
	return ici_argerror(0);
    return int_ret(WMGetListSelectedItemRow(wmlist->wm_list));
}

static void
list_action_callback(WMWidget *self, void *data)
{
    func_t	*fn;
    object_t	*cdata;
    object_t	*wmwidget;

    if ((wmwidget = new_wmwidget(self)) != NULL)
    {
	fn = funcof(fetch(structof(data), STRING(fn)));
	cdata = fetch(structof(data), STRING(cdata));
	ici_func(objof(fn), "oo", wmwidget, cdata);
	decref(wmwidget);
    }
}

FUNC(SetListAction)
{
    wmlist_t	*wmlist;
    func_t	*callback;
    object_t	*cdata;
    struct_t	*o;

    switch (NARGS())
    {
    case 2:
	cdata = objof(&o_null);
	if (ici_typecheck("oo", &wmlist, &callback))
	    return 1;
	break;

    default:
	if (ici_typecheck("ooo", &wmlist, &callback, &cdata))
	    return 1;
    }
    if (!iswmlist(objof(wmlist)))
	return ici_argerror(0);
    if (!isfunc(objof(callback)))
	return ici_argerror(1);
    if ((o = makecallback(callback, cdata)) == NULL)
	return 1;
    WMSetListAction(wmlist->wm_list, list_action_callback, o);
    return null_ret();
}

static void
list_double_action_callback(WMWidget *self, void *data)
{
    func_t	*fn;
    object_t	*cdata;
    object_t	*wmwidget;

    if ((wmwidget = new_wmwidget(self)) != NULL)
    {
	fn = funcof(fetch(structof(data), STRING(fn)));
	cdata = fetch(structof(data), STRING(cdata));
	ici_func(objof(fn), "oo", wmwidget, cdata);
	decref(wmwidget);
    }
}

FUNC(SetListDoubleAction)
{
    wmlist_t	*wmlist;
    func_t	*callback;
    object_t	*cdata;
    struct_t	*o;

    switch (NARGS())
    {
    case 2:
	cdata = objof(&o_null);
	if (ici_typecheck("oo", &wmlist, &callback))
	    return 1;
	break;

    default:
	if (ici_typecheck("ooo", &wmlist, &callback, &cdata))
	    return 1;
    }
    if (!iswmlist(objof(wmlist)))
	return ici_argerror(0);
    if (!isfunc(objof(callback)))
	return ici_argerror(1);
    if ((o = makecallback(callback, cdata)) == NULL)
	return 1;
    WMSetListDoubleAction(wmlist->wm_list, list_double_action_callback, o);
    return null_ret();
}

FUNC(ClearList)
{
    wmlist_t	*wmlist;

    if (ici_typecheck("o", &wmlist))
	return 1;
    if (!iswmlist(objof(wmlist)))
	return ici_argerror(0);
    WMClearList(wmlist->wm_list);
    return null_ret();
}

FUNC(GetListNumberOfRows)
{
    wmlist_t	*wmlist;

    if (ici_typecheck("o", &wmlist))
	return 1;
    if (!iswmlist(objof(wmlist)))
	return ici_argerror(0);
    return int_ret(WMGetListNumberOfRows(wmlist->wm_list));
}

FUNC(SetListPosition)
{
    wmlist_t	*wmlist;
    long	position;
    
    if (ici_typecheck("oi", &wmlist, &position))
	return 1;
    if (!iswmlist(objof(wmlist)))
	return ici_argerror(0);
    WMSetListPosition(wmlist->wm_list, position);
    return null_ret();
}

FUNC(SetListBottomPosition)
{
    wmlist_t	*wmlist;
    long	position;
    
    if (ici_typecheck("oi", &wmlist, &position))
	return 1;
    if (!iswmlist(objof(wmlist)))
	return ici_argerror(0);
    WMSetListBottomPosition(wmlist->wm_list, position);
    return null_ret();
}

FUNC(GetListPosition)
{
    wmlist_t	*wmlist;

    if (ici_typecheck("o", &wmlist))
	return 1;
    if (!iswmlist(objof(wmlist)))
	return ici_argerror(0);
    return int_ret(WMGetListPosition(wmlist->wm_list));
}

FUNC(CreateBrowser)
{
    object_t	*wmwidget;
    WMBrowser	*browser;

    if (ici_typecheck("o", &wmwidget))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    browser = WMCreateBrowser(wmwidgetof(wmwidget));
    if (browser == NULL)
    {
	error = "failed to CreateBrowser";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wmbrowser(browser)));
}

FUNC(SetBrowserPathSeparator)
{
    wmbrowser_t	*wmbrowser;
    char	*separator;

    if (ici_typecheck("os", &wmbrowser, &separator))
	return 1;
    if (!iswmbrowser(objof(wmbrowser)))
	return ici_argerror(0);
    WMSetBrowserPathSeparator(wmbrowser->wm_browser, separator);
    return null_ret();
}

FUNC(SetBrowserTitled)
{
    wmbrowser_t	*wmbrowser;
    long	flag;

    if (ici_typecheck("oi", &wmbrowser, &flag))
	return 1;
    if (!iswmbrowser(objof(wmbrowser)))
	return ici_argerror(0);
    WMSetBrowserTitled(wmbrowser->wm_browser, flag);
    return null_ret();
}

FUNC(LoadBrowserColumnZero)
{
    wmbrowser_t	*wmbrowser;

    if (ici_typecheck("o", &wmbrowser))
	return 1;
    if (!iswmbrowser(objof(wmbrowser)))
	return ici_argerror(0);
    WMLoadBrowserColumnZero(wmbrowser->wm_browser);
    return null_ret();
}

FUNC(AddBrowserColumn)
{
    wmbrowser_t	*wmbrowser;

    if (ici_typecheck("o", &wmbrowser))
	return 1;
    if (!iswmbrowser(objof(wmbrowser)))
	return ici_argerror(0);
    return int_ret(WMAddBrowserColumn(wmbrowser->wm_browser));
}

FUNC(RemoveBrowserItem)
{
    wmbrowser_t	*wmbrowser;
    long	column, row;

    if (ici_typecheck("oii", &wmbrowser, &column, &row))
	return 1;
    if (!iswmbrowser(objof(wmbrowser)))
	return ici_argerror(0);
    WMRemoveBrowserItem(wmbrowser->wm_browser, column, row);
    return null_ret();
}

FUNC(SetBrowserMaxVisibleColumns)
{
    wmbrowser_t	*wmbrowser;
    long	columns;

    if (ici_typecheck("oi", &wmbrowser, &columns))
	return 1;
    if (!iswmbrowser(objof(wmbrowser)))
	return ici_argerror(0);
    WMSetBrowserMaxVisibleColumns(wmbrowser->wm_browser, columns);
    return null_ret();
}


FUNC(SetBrowserColumnTitle)
{
    wmbrowser_t	*wmbrowser;
    long	column;
    char	*text;

    if (ici_typecheck("ois", &wmbrowser, &column, &text))
	return 1;
    if (!iswmbrowser(objof(wmbrowser)))
	return ici_argerror(0);
    WMSetBrowserColumnTitle(wmbrowser->wm_browser, column, text);
    return null_ret();
}

#if 0
xFUNC(AddSortedBrowserItem)
{
    wmbrowser_t	*wmbrowser;
    long	column;
    char	*text;
    long	isbranch;
    WMListItem	*listitem;

    if (ici_typecheck("oisi", &wmbrowser, &column, &text, &isbranch))
	return 1;
    if (!iswmbrowser(objof(wmbrowser)))
	return ici_argerror(0);
    listitem = WMAddSortedBrowserItem(wmbrowser->wm_browser, column, text, isbranch);
    return ici_ret_with_decref(objof(new_wmlistitem(listitem)));
}
#endif

FUNC(InsertBrowserItem)
{
    wmbrowser_t	*wmbrowser;
    long	column;
    long	row;
    char	*text;
    long	isbranch;
    WMListItem	*listitem;

    if (ici_typecheck("oiisi", &wmbrowser, &column, &row, &text, &isbranch))
	return 1;
    if (!iswmbrowser(objof(wmbrowser)))
	return ici_argerror(0);
    listitem = WMInsertBrowserItem(wmbrowser->wm_browser, column, row, text, isbranch);
    return ici_ret_with_decref(objof(new_wmlistitem(listitem)));
}

FUNC(SetBrowserPath)
{
    wmbrowser_t	*wmbrowser;
    char	*path;

    if (ici_typecheck("os", &wmbrowser, &path))
	return 1;
    if (!iswmbrowser(objof(wmbrowser)))
	return ici_argerror(0);
    return str_ret(WMSetBrowserPath(wmbrowser->wm_browser, path));
}


FUNC(GetBrowserPath)
{
    wmbrowser_t	*wmbrowser;

    if (ici_typecheck("o", &wmbrowser))
	return 1;
    if (!iswmbrowser(objof(wmbrowser)))
	return ici_argerror(0);
    return str_ret(WMGetBrowserPath(wmbrowser->wm_browser));
}


FUNC(GetBrowserPathToColumn)
{
    wmbrowser_t	*wmbrowser;
    long	column;

    if (ici_typecheck("oi", &wmbrowser, &column))
	return 1;
    if (!iswmbrowser(objof(wmbrowser)))
	return ici_argerror(0);
    return str_ret(WMGetBrowserPathToColumn(wmbrowser->wm_browser, column));
}

FUNC(SetBrowserFillColumnProc)
{
    error = "SetBrowserFillColumnProc not implemented";
    return 1;
}

static void
browser_action_callback(WMWidget *self, void *data)
{
    func_t	*fn;
    object_t	*cdata;
    object_t	*wmwidget;

    if ((wmwidget = new_wmwidget(self)) != NULL)
    {
	fn = funcof(fetch(structof(data), STRING(fn)));
	cdata = fetch(structof(data), STRING(cdata));
	ici_func(objof(fn), "oo", wmwidget, cdata);
	decref(wmwidget);
    }
}

FUNC(SetBrowserAction)
{
    wmbrowser_t	*wmbrowser;
    func_t	*callback;
    object_t	*cdata;
    struct_t	*o;

    switch (NARGS())
    {
    case 2:
	cdata = objof(&o_null);
	if (ici_typecheck("oo", &wmbrowser, &callback))
	    return 1;
	break;

    default:
	if (ici_typecheck("ooo", &wmbrowser, &callback, &cdata))
	    return 1;
    }
    if (!iswmbrowser(objof(wmbrowser)))
	return ici_argerror(0);
    if (!isfunc(objof(callback)))
	return ici_argerror(1);
    if ((o = makecallback(callback, cdata)) == NULL)
	return 1;
    WMSetBrowserAction(wmbrowser->wm_browser, browser_action_callback, o);
    return null_ret();
}

FUNC(GetBrowserSelectedItemInColumn)
{
    wmbrowser_t	*wmbrowser;
    long	column;
    WMListItem	*listitem;

    if (ici_typecheck("oi", &wmbrowser, &column))
	return 1;
    if (!iswmbrowser(objof(wmbrowser)))
	return ici_argerror(0);
    listitem = WMGetBrowserSelectedItemInColumn(wmbrowser->wm_browser, column);
    return ici_ret_with_decref(objof(new_wmlistitem(listitem)));
}

FUNC(GetBrowserFirstVisibleColumn)
{
    wmbrowser_t	*wmbrowser;

    if (ici_typecheck("o", &wmbrowser))
	return 1;
    if (!iswmbrowser(objof(wmbrowser)))
	return ici_argerror(0);
    return int_ret(WMGetBrowserFirstVisibleColumn(wmbrowser->wm_browser));
}

FUNC(GetBrowserSelectedColumn)
{
    wmbrowser_t	*wmbrowser;

    if (ici_typecheck("o", &wmbrowser))
	return 1;
    if (!iswmbrowser(objof(wmbrowser)))
	return ici_argerror(0);
    return int_ret(WMGetBrowserSelectedColumn(wmbrowser->wm_browser));
}

FUNC(GetBrowserSelectedRowInColumn)
{
    wmbrowser_t	*wmbrowser;
    long	column;

    if (ici_typecheck("oi", &wmbrowser, &column))
	return 1;
    if (!iswmbrowser(objof(wmbrowser)))
	return ici_argerror(0);
    return int_ret(WMGetBrowserSelectedRowInColumn(wmbrowser->wm_browser, column));
}

FUNC(GetBrowserNumberOfColumns)
{
    wmbrowser_t	*wmbrowser;

    if (ici_typecheck("o", &wmbrowser))
	return 1;
    if (!iswmbrowser(objof(wmbrowser)))
	return ici_argerror(0);
    return int_ret(WMGetBrowserNumberOfColumns(wmbrowser->wm_browser));
}

FUNC(GetBrowserListInColumn)
{
    wmbrowser_t	*wmbrowser;
    long	column;
    WMList	*list;

    if (ici_typecheck("oi", &wmbrowser, &column))
	return 1;
    if (!iswmbrowser(objof(wmbrowser)))
	return ici_argerror(0);
    list = WMGetBrowserListInColumn(wmbrowser->wm_browser, column);
    return ici_ret_with_decref(objof(new_wmlist(list)));
}

FUNC(CreatePopUpButton)
{
    object_t		*wmwidget;
    WMPopUpButton	*popupbutton;

    if (ici_typecheck("o", &wmwidget))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    popupbutton = WMCreatePopUpButton(wmwidgetof(wmwidget));
    if (popupbutton == NULL)
    {
	error = "failed to CreatePopUpButton";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wmpopupbutton(popupbutton)));
}

static void
popupbutton_action_callback(WMWidget *self, void *data)
{
    func_t	*fn;
    object_t	*cdata;
    object_t	*wmwidget;

    if ((wmwidget = new_wmwidget(self)) != NULL)
    {
	fn = funcof(fetch(structof(data), STRING(fn)));
	cdata = fetch(structof(data), STRING(cdata));
	ici_func(objof(fn), "oo", wmwidget, cdata);
	decref(wmwidget);
    }
}

FUNC(SetPopUpButtonAction)
{
    wmpopupbutton_t	*wmpopupbutton;
    func_t		*callback;
    object_t		*cdata;
    struct_t		*o;

    switch (NARGS())
    {
    case 2:
	cdata = objof(&o_null);
	if (ici_typecheck("oo", &wmpopupbutton, &callback))
	    return 1;
	break;

    default:
	if (ici_typecheck("ooo", &wmpopupbutton, &callback, &cdata))
	    return 1;
    }
    if (!iswmpopupbutton(objof(wmpopupbutton)))
	return ici_argerror(0);
    if (!isfunc(objof(callback)))
	return ici_argerror(1);
    if ((o = makecallback(callback, cdata)) == NULL)
	return 1;
    WMSetPopUpButtonAction(wmpopupbutton->wm_popupbutton, popupbutton_action_callback, o);
    return null_ret();
}

FUNC(SetPopUpButtonPullsDown)
{
    wmpopupbutton_t	*wmpopupbutton;
    long		flag;

    if (ici_typecheck("oi", &wmpopupbutton, &flag))
	return 1;
    if (!iswmpopupbutton(objof(wmpopupbutton)))
	return ici_argerror(0);
    WMSetPopUpButtonPullsDown(wmpopupbutton->wm_popupbutton, flag);
    return null_ret();
}

FUNC(AddPopUpButtonItem)
{
    wmpopupbutton_t	*wmpopupbutton;
    char		*text;

    if (ici_typecheck("os", &wmpopupbutton, &text))
	return 1;
    if (!iswmpopupbutton(objof(wmpopupbutton)))
	return ici_argerror(0);
    WMAddPopUpButtonItem(wmpopupbutton->wm_popupbutton, text);
    return null_ret();
}

FUNC(InsertPopUpButtonItem)
{
    wmpopupbutton_t	*wmpopupbutton;
    long		index;
    char		*text;

    if (ici_typecheck("ois", &wmpopupbutton, &index, &text))
	return 1;
    if (!iswmpopupbutton(objof(wmpopupbutton)))
	return ici_argerror(0);
    WMInsertPopUpButtonItem(wmpopupbutton->wm_popupbutton, index, text);
    return null_ret();
}

FUNC(RemovePopUpButtonItem)
{
    wmpopupbutton_t	*wmpopupbutton;
    long		index;

    if (ici_typecheck("oi", &wmpopupbutton, &index))
	return 1;
    if (!iswmpopupbutton(objof(wmpopupbutton)))
	return ici_argerror(0);
    WMRemovePopUpButtonItem(wmpopupbutton->wm_popupbutton, index);
    return null_ret();
}

FUNC(SetPopUpButtonItemEnabled)
{
    wmpopupbutton_t	*wmpopupbutton;
    long		index;
    long		flag;
    
    if (ici_typecheck("oii", &wmpopupbutton, &index, &flag))
	return 1;
    if (!iswmpopupbutton(objof(wmpopupbutton)))
	return ici_argerror(0);
    WMSetPopUpButtonItemEnabled(wmpopupbutton->wm_popupbutton, index, flag);
    return null_ret();
}

FUNC(SetPopUpButtonSelectedItem)
{
    wmpopupbutton_t	*wmpopupbutton;
    long		index;

    if (ici_typecheck("oi", &wmpopupbutton, &index))
	return 1;
    if (!iswmpopupbutton(objof(wmpopupbutton)))
	return ici_argerror(0);
    WMSetPopUpButtonSelectedItem(wmpopupbutton->wm_popupbutton, index);
    return null_ret();
}

FUNC(GetPopUpButtonSelectedItem)
{
    wmpopupbutton_t	*wmpopupbutton;

    if (ici_typecheck("o", &wmpopupbutton))
	return 1;
    if (!iswmpopupbutton(objof(wmpopupbutton)))
	return ici_argerror(0);
    return int_ret(WMGetPopUpButtonSelectedItem(wmpopupbutton->wm_popupbutton));
}

FUNC(SetPopUpButtonText)
{
    wmpopupbutton_t	*wmpopupbutton;
    char		*text;

    if (ici_typecheck("os", &wmpopupbutton, &text))
	return 1;
    if (!iswmpopupbutton(objof(wmpopupbutton)))
	return ici_argerror(0);
    WMSetPopUpButtonText(wmpopupbutton->wm_popupbutton, text);
    return null_ret();
}

FUNC(GetPopUpButtonItem)
{
    wmpopupbutton_t	*wmpopupbutton;
    long		index;

    if (ici_typecheck("oi", &wmpopupbutton, &index))
	return 1;
    if (!iswmpopupbutton(objof(wmpopupbutton)))
	return ici_argerror(0);
    return str_ret(WMGetPopUpButtonItem(wmpopupbutton->wm_popupbutton, index));
}

FUNC(GetPopUpButtonNumberOfItems)
{
    wmpopupbutton_t	*wmpopupbutton;

    if (ici_typecheck("o", &wmpopupbutton))
	return 1;
    if (!iswmpopupbutton(objof(wmpopupbutton)))
	return ici_argerror(0);
    return int_ret(WMGetPopUpButtonNumberOfItems(wmpopupbutton->wm_popupbutton));
}

FUNC(SetPopUpButtonEnabled)
{
    wmpopupbutton_t	*wmpopupbutton;
    long		flag;

    if (ici_typecheck("oi", &wmpopupbutton, &flag))
	return 1;
    if (!iswmpopupbutton(objof(wmpopupbutton)))
	return ici_argerror(0);
    WMSetPopUpButtonEnabled(wmpopupbutton->wm_popupbutton, flag);
    return null_ret();
}

FUNC(CreateColorWell)
{
    object_t	*wmwidget;
    WMColorWell	*colorwell;

    if (ici_typecheck("o", &wmwidget))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    colorwell = WMCreateColorWell(wmwidgetof(wmwidget));
    if (colorwell == NULL)
    {
	error = "failed to CreateColorWell";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wmcolorwell(colorwell)));
}

FUNC(SetColorWellColor)
{
    wmcolorwell_t	*wmcolorwell;
    wmcolor_t		*wmcolor;

    if (ici_typecheck("oo", &wmcolorwell, &wmcolor))
	return 1;

    if (!iswmcolorwell(objof(wmcolorwell)))
	return ici_argerror(0);
    if (!iswmcolor(objof(wmcolor)))
	return ici_argerror(1);
    WMSetColorWellColor(wmcolorwell->wm_colorwell, wmcolor->wm_color);
    return null_ret();
}

FUNC(GetColorWellColor)
{
    wmcolorwell_t	*wmcolorwell;
    WMColor		*color;

    if (ici_typecheck("o", &wmcolorwell))
	return 1;
    if (!iswmcolorwell(objof(wmcolorwell)))
	return ici_argerror(0);
    color = WMGetColorWellColor(wmcolorwell->wm_colorwell);
    return ici_ret_with_decref(objof(new_wmcolor(color)));
}

FUNC(SetColorWellBordered)
{
    wmcolorwell_t	*wmcolorwell;
    long		bordered;

    if (ici_typecheck("oi", &wmcolorwell, &bordered))
	return 1;
    if (!iswmcolorwell(objof(wmcolorwell)))
	return ici_argerror(0);
    WSetColorWellBordered(wmcolorwell->wm_colorwell, bordered);
    return null_ret();
}

FUNC(CreateScrollView)
{
    object_t		*wmwidget;
    WMScrollView	*scrollview;

    if (ici_typecheck("o", &wmwidget))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    scrollview = WMCreateScrollView(wmwidgetof(wmwidget));
    if (scrollview == NULL)
    {
	error = "failed to CreateScrollView";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wmscrollview(scrollview)));
}

FUNC(ResizeScrollViewContent)
{
    wmscrollview_t	*wmscrollview;
    long		w, h;

    if (ici_typecheck("oii", &wmscrollview, &w, &h))
	return 1;
    if (!iswmscrollview(objof(wmscrollview)))
	return ici_argerror(0);
    WMResizeScrollViewContent(wmscrollview->wm_scrollview, w, h);
    return null_ret();
}

FUNC(SetScrollViewHasHorizontalScroller)
{
    wmscrollview_t	*wmscrollview;
    long		flag;

    if (ici_typecheck("oi", &wmscrollview, &flag))
	return 1;
    if (!iswmscrollview(objof(wmscrollview)))
	return ici_argerror(0);
    WMSetScrollViewHasHorizontalScroller(wmscrollview->wm_scrollview, flag);
    return null_ret();
}

FUNC(SetScrollViewHasVerticalScroller)
{
    wmscrollview_t	*wmscrollview;
    long		flag;

    if (ici_typecheck("oi", &wmscrollview, &flag))
	return 1;
    if (!iswmscrollview(objof(wmscrollview)))
	return ici_argerror(0);
    WMSetScrollViewHasVerticalScroller(wmscrollview->wm_scrollview, flag);
    return null_ret();
}

FUNC(SetScrollViewContentView)
{
    wmscrollview_t	*wmscrollview;
    wmview_t		*wmview;

    if (ici_typecheck("oo", &wmscrollview, &wmview))
	return 1;
    if (!iswmscrollview(objof(wmscrollview)))
	return ici_argerror(0);
    if (!iswmview(objof(wmview)))
	return ici_argerror(1);
    WMSetScrollViewContentView(wmscrollview->wm_scrollview, wmview->wm_view);
    return null_ret();
}

FUNC(SetScrollViewRelief)
{
    wmscrollview_t	*wmscrollview;
    long		relief;

    if (ici_typecheck("oi", &wmscrollview, &relief))
	return 1;
    if (!iswmscrollview(objof(wmscrollview)))
	return ici_argerror(0);
    WMSetScrollViewRelief(wmscrollview->wm_scrollview, relief);
    return null_ret();
}

FUNC(GetScrollViewHorizontalScroller)
{
    wmscrollview_t	*wmscrollview;
    WMScroller		*scroller;

    if (ici_typecheck("o", &wmscrollview))
	return 1;
    if (!iswmscrollview(objof(wmscrollview)))
	return ici_argerror(0);
    scroller = WMGetScrollViewHorizontalScroller(wmscrollview->wm_scrollview);
    return ici_ret_with_decref(objof(new_wmscroller(scroller)));
}

FUNC(GetScrollViewVerticalScroller)
{
    wmscrollview_t	*wmscrollview;
    WMScroller		*scroller;

    if (ici_typecheck("o", &wmscrollview))
	return 1;
    if (!iswmscrollview(objof(wmscrollview)))
	return ici_argerror(0);
    scroller = WMGetScrollViewVerticalScroller(wmscrollview->wm_scrollview);
    return ici_ret_with_decref(objof(new_wmscroller(scroller)));
}

FUNC(SetScrollViewLineScroll)
{
    wmscrollview_t	*wmscrollview;
    long		amount;

    if (ici_typecheck("oi", &wmscrollview, &amount))
	return 1;
    if (!iswmscrollview(objof(wmscrollview)))
	return ici_argerror(0);
    WMSetScrollViewLineScroll(wmscrollview->wm_scrollview, amount);
    return null_ret();
}

FUNC(SetScrollViewPageScroll)
{
    wmscrollview_t	*wmscrollview;
    long		amount;

    if (ici_typecheck("oi", &wmscrollview, &amount))
	return 1;
    if (!iswmscrollview(objof(wmscrollview)))
	return ici_argerror(0);
    WMSetScrollViewPageScroll(wmscrollview->wm_scrollview, amount);
    return null_ret();
}

FUNC(CreateSlider)
{
    object_t		*wmwidget;
    WMSlider		*slider;

    if (ici_typecheck("o", &wmwidget))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    slider = WMCreateSlider(wmwidgetof(wmwidget));
    if (slider == NULL)
    {
	error = "failed to CreateSlider";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wmslider(slider)));
}

FUNC(GetSliderMinValue)
{
    wmslider_t	*wmslider;

    if (ici_typecheck("o", &wmslider))
	return 1;
    if (!iswmslider(objof(wmslider)))
	return ici_argerror(0);
    return int_ret(WMGetSliderMinValue(wmslider->wm_slider));
}

FUNC(GetSliderMaxValue)
{
    wmslider_t	*wmslider;

    if (ici_typecheck("o", &wmslider))
	return 1;
    if (!iswmslider(objof(wmslider)))
	return ici_argerror(0);
    return int_ret(WMGetSliderMaxValue(wmslider->wm_slider));
}

FUNC(GetSliderValue)
{
    wmslider_t	*wmslider;

    if (ici_typecheck("o", &wmslider))
	return 1;
    if (!iswmslider(objof(wmslider)))
	return ici_argerror(0);
    return int_ret(WMGetSliderValue(wmslider->wm_slider));
}

FUNC(SetSliderMinValue)
{
    wmslider_t	*wmslider;
    long	value;

    if (ici_typecheck("oi", &wmslider, &value))
	return 1;
    if (!iswmslider(objof(wmslider)))
	return ici_argerror(0);
    WMSetSliderMinValue(wmslider->wm_slider, value);
    return null_ret();
}

FUNC(SetSliderMaxValue)
{
    wmslider_t	*wmslider;
    long	value;

    if (ici_typecheck("oi", &wmslider, &value))
	return 1;
    if (!iswmslider(objof(wmslider)))
	return ici_argerror(0);
    WMSetSliderMaxValue(wmslider->wm_slider, value);
    return null_ret();
}

FUNC(SetSliderValue)
{
    wmslider_t	*wmslider;
    long	value;

    if (ici_typecheck("oi", &wmslider, &value))
	return 1;
    if (!iswmslider(objof(wmslider)))
	return ici_argerror(0);
    WMSetSliderValue(wmslider->wm_slider, value);
    return null_ret();
}

FUNC(SetSliderContinuous)
{
    wmslider_t	*wmslider;
    long	value;

    if (ici_typecheck("oi", &wmslider, &value))
	return 1;
    if (!iswmslider(objof(wmslider)))
	return ici_argerror(0);
    WMSetSliderContinuous(wmslider->wm_slider, value);
    return null_ret();
}

static void
slider_action_callback(WMWidget *self, void *data)
{
    func_t	*fn;
    object_t	*cdata;
    object_t	*wmwidget;

    if ((wmwidget = new_wmwidget(self)) != NULL)
    {
	fn = funcof(fetch(structof(data), STRING(fn)));
	cdata = fetch(structof(data), STRING(cdata));
	ici_func(objof(fn), "oo", wmwidget, cdata);
	decref(wmwidget);
    }
}

FUNC(SetSliderAction)
{
    wmslider_t	*wmslider;
    func_t	*callback;
    object_t	*cdata;
    struct_t	*o;

    switch (NARGS())
    {
    case 2:
	cdata = objof(&o_null);
	if (ici_typecheck("oo", &wmslider, &callback))
	    return 1;
	break;

    default:
	if (ici_typecheck("ooo", &wmslider, &callback, &cdata))
	    return 1;
    }
    if (!iswmslider(objof(wmslider)))
	return ici_argerror(0);
    if (!isfunc(objof(callback)))
	return ici_argerror(1);
    if ((o = makecallback(callback, cdata)) == NULL)
	return 1;
    WMSetSliderAction(wmslider->wm_slider, slider_action_callback, o);
    return null_ret();
}

FUNC(SetSliderKnobThickness)
{
    wmslider_t	*wmslider;
    long	value;

    if (ici_typecheck("oi", &wmslider, &value))
	return 1;
    if (!iswmslider(objof(wmslider)))
	return ici_argerror(0);
    WMSetSliderKnobThickness(wmslider->wm_slider, value);
    return null_ret();
}

FUNC(SetSliderImage)
{
    wmslider_t	*wmslider;
    wmpixmap_t	*wmpixmap;

    if (ici_typecheck("oo", &wmslider, &wmpixmap))
	return 1;
    if (!iswmslider(objof(wmslider)))
	return ici_argerror(0);
    WMSetSliderImage(wmslider->wm_slider, wmpixmap->wm_pixmap);
    return null_ret();
}

FUNC(CreateSplitView)
{
    object_t	*wmwidget;
    WMSplitView	*splitview;

    if (ici_typecheck("o", &wmwidget))
	return 1;
    if (!iswmwidget(wmwidget))
	return ici_argerror(0);
    splitview = WMCreateSplitView(wmwidgetof(wmwidget));
    if (splitview == NULL)
    {
	error = "failed to CreateSplitView";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wmsplitview(splitview)));
}

FUNC(AddSplitViewSubview)
{
    wmsplitview_t	*wmsplitview;
    wmview_t		*wmview;

    if (ici_typecheck("oo", &wmsplitview, &wmview))
	return 1;
    if (!iswmsplitview(objof(wmsplitview)))
	return ici_argerror(0);
    if (!iswmview(objof(wmview)))
	return ici_argerror(1);
    WMAddSplitViewSubview(wmsplitview->wm_splitview, wmview->wm_view);
    return null_ret();
}

FUNC(AdjustSplitViewSubviews)
{
    wmsplitview_t	*wmsplitview;

    if (ici_typecheck("o", &wmsplitview))
	return 1;
    if (!iswmsplitview(objof(wmsplitview)))
	return ici_argerror(0);
    WMAdjustSplitViewSubviews(wmsplitview->wm_splitview);
    return null_ret();
}

FUNC(SetSplitViewConstrainProc)
{
    error = "SetSplitViewConstrainProc not implemented";
    return 1;
}

FUNC(GetSplitViewDividerThickness)
{
    wmsplitview_t	*wmsplitview;

    if (ici_typecheck("o", &wmsplitview))
	return 1;
    if (!iswmsplitview(objof(wmsplitview)))
	return ici_argerror(0);
    return int_ret(WMGetSplitViewDividerThickness(wmsplitview->wm_splitview));
}

FUNC(RunAlertPanel)
{
    wmscreen_t	*wmscreen;
    wmwindow_t	*wmwindow;
    WMWindow	*w;
    char	*title;
    char	*msg;
    char	*defaultbutton;
    string_t	*alternatebutton;
    char	*a;
    string_t	*otherbutton;
    char	*o;

    if (ici_typecheck("oosssoo", &wmscreen, &wmwindow, &title, &msg, &defaultbutton, &alternatebutton, &otherbutton))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    if (iswmwindow(objof(wmwindow)))
	w = wmwindow->wm_window;
    else if (isnull(objof(wmwindow)))
	w = NULL;
    else
	return ici_argerror(1);
    if (isstring(objof(alternatebutton)))
	a = alternatebutton->s_chars;
    else if (isnull(objof(alternatebutton)))
	a = NULL;
    else
	return ici_argerror(5);
    if (isstring(objof(otherbutton)))
	o = otherbutton->s_chars;
    else if (isnull(objof(otherbutton)))
	o = NULL;
    else
	return ici_argerror(6);
    return int_ret(WMRunAlertPanel(wmscreen->wm_screen, w, title, msg, defaultbutton, a, o));
}

FUNC(RunInputPanel)
{
    wmscreen_t	*wmscreen;
    wmwindow_t	*wmwindow;
    char	*title;
    char	*msg;
    char	*defaulttext;
    char	*okbutton;
    char	*cancelbutton;

    if (ici_typecheck("oosssss", &wmscreen, &wmwindow, &title, &msg, &defaulttext, &okbutton, &cancelbutton))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    if (!iswmwindow(objof(wmwindow)))
	return ici_argerror(1);
    return str_ret(WMRunInputPanel(wmscreen->wm_screen, wmwindow->wm_window, title, msg, defaulttext, okbutton, cancelbutton));
}

FUNC(CreateAlertPanel)
{
    wmscreen_t		*wmscreen;
    wmwindow_t		*wmwindow;
    WMWindow		*w;
    char		*title;
    char		*msg;
    char		*defbutton;
    char		*a, *o;
    string_t		*alternatebutton;
    string_t		*otherbutton;
    WMAlertPanel	*panel;
    
    if (ici_typecheck("oosssoo", &wmscreen, &wmwindow, &title, &msg, &defbutton, &alternatebutton, &otherbutton))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    if (iswmwindow(objof(wmwindow)))
	w = wmwindow->wm_window;
    else if (isnull(objof(wmwindow)))
	w = NULL;
    else
	return ici_argerror(1);
    if (isstring(objof(alternatebutton)))
	a = alternatebutton->s_chars;
    else if (isnull(objof(alternatebutton)))
	a = NULL;
    else
	return ici_argerror(5);
    if (isstring(objof(otherbutton)))
	o = otherbutton->s_chars;
    else if (isnull(objof(otherbutton)))
	o = NULL;
    else
	return ici_argerror(6);
    panel = WMCreateAlertPanel(wmscreen->wm_screen, w, title, msg, defbutton, a, o);
    if (panel == NULL)
    {
	error = "failed to CreateAlertPanel";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wmalertpanel(panel)));
}

FUNC(CreateInputPanel)
{
    wmscreen_t		*wmscreen;
    wmwindow_t		*wmwindow;
    char		*title;
    char		*msg;
    char		*defaulttext;
    char		*okbutton;
    char		*cancelbutton;
    WMInputPanel	*panel;
    
    if (ici_typecheck("oosssss", &wmscreen, &wmwindow, &title, &msg, &defaulttext, &okbutton, &cancelbutton))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    if (!iswmwindow(objof(wmwindow)))
	return ici_argerror(1);
    panel = WMCreateInputPanel(wmscreen->wm_screen, wmwindow->wm_window, title, msg, defaulttext, okbutton, cancelbutton);
    if (panel == NULL)
    {
	error = "failed to CreateInputPanel";
	return 1;
    }
    return ici_ret_with_decref(objof(new_wminputpanel(panel)));
}

FUNC(DestroyAlertPanel)
{
    wmalertpanel_t	*wmalertpanel;

    if (ici_typecheck("o", &wmalertpanel))
	return 1;
    if (!iswmalertpanel(objof(wmalertpanel)))
	return ici_argerror(0);
    WMDestroyAlertPanel(wmalertpanel->wm_alertpanel);
    return null_ret();
}

FUNC(DestroyInputPanel)
{
    wminputpanel_t	*wminputpanel;

    if (ici_typecheck("o", &wminputpanel))
	return 1;
    if (!iswminputpanel(objof(wminputpanel)))
	return ici_argerror(0);
    WMDestroyInputPanel(wminputpanel->wm_inputpanel);
    return null_ret();
}

FUNC(GetOpenPanel)
{
    wmscreen_t	*wmscreen;
    WMOpenPanel	*panel;

    if (ici_typecheck("o", &wmscreen))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    panel = WMGetOpenPanel(wmscreen->wm_screen);
    return ici_ret_with_decref(objof(new_wmopenpanel(panel)));
}

FUNC(GetSavePanel)
{
    wmscreen_t	*wmscreen;
    WMSavePanel	*panel;

    if (ici_typecheck("o", &wmscreen))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    panel = WMGetSavePanel(wmscreen->wm_screen);
    return ici_ret_with_decref(objof(new_wmsavepanel(panel)));
}

FUNC(SetFilePanelCanChooseDirectories)
{
    object_t	*wmfilepanel;
    long	flag;
    
    if (ici_typecheck("oi", &wmfilepanel, &flag))
	return 1;
    if (!iswmfilepanel(wmfilepanel))
	return ici_argerror(0);
    WMSetFilePanelCanChooseDirectories(wmfilepanelof(wmfilepanel), flag);
    return null_ret();
}

FUNC(SetFilePanelCanChooseFiles)
{
    object_t	*wmfilepanel;
    long	flag;
    
    if (ici_typecheck("oi", &wmfilepanel, &flag))
	return 1;
    if (!iswmfilepanel(wmfilepanel))
	return ici_argerror(0);
    WMSetFilePanelCanChooseFiles(wmfilepanelof(wmfilepanel), flag);
    return null_ret();
}

FUNC(SetFilePanelDirectory)
{
    object_t	*wmfilepanel;
    char	*path;
    
    if (ici_typecheck("os", &wmfilepanel, &path))
	return 1;
    if (!iswmfilepanel(wmfilepanel))
	return ici_argerror(0);
    WMSetFilePanelDirectory(wmfilepanelof(wmfilepanel), path);
    return null_ret();
}

FUNC(GetFilePanelFileName)
{
    object_t	*wmfilepanel;
    
    if (ici_typecheck("o", &wmfilepanel))
	return 1;
    if (!iswmfilepanel(wmfilepanel))
	return ici_argerror(0);
    return str_ret(WMGetFilePanelFileName(wmfilepanelof(wmfilepanel)));
}

FUNC(FreeFilePanel)
{
    object_t	*wmfilepanel;
    
    if (ici_typecheck("o", &wmfilepanel))
	return 1;
    if (!iswmfilepanel(wmfilepanel))
	return ici_argerror(0);
    WMFreeFilePanel(wmfilepanelof(wmfilepanel));
    return null_ret();
}

FUNC(RunModalFilePanelForDirectory)
{
    object_t	*wmfilepanel;
    wmwindow_t	*wmwindow;
    WMWindow	*window;
    char	*path;
    string_t	*namearg;
    char	*name;
    int		i;

    if (ici_typecheck("ooso", &wmfilepanel, &wmwindow, &path, &namearg))
	return 1;
    if (!iswmfilepanel(wmfilepanel))
	return ici_argerror(0);
    if (iswmwindow(objof(wmwindow)))
	window = wmwindow->wm_window;
    else if (isnull(objof(wmwindow)))
	window = NULL;
    else
	return ici_argerror(1);
    if (isstring(objof(namearg)))
	name = namearg->s_chars;
    else if (isnull(objof(namearg)))
	name = NULL;
    else
	return ici_argerror(3);
    i = WMRunModalFilePanelForDirectory(wmfilepanelof(wmfilepanel), window, path, name, NULL);
    return int_ret(i);
}

FUNC(SetFilePanelAccessoryView)
{
    object_t	*wmfilepanel;
    wmview_t	*wmview;

    if (ici_typecheck("oo", &wmfilepanel, &wmview))
	return 1;
    if (!iswmfilepanel(wmfilepanel))
	return ici_argerror(0);
    if (!iswmview(objof(wmview)))
	return ici_argerror(1);
    WMSetFilePanelAccessoryView(wmfilepanelof(wmfilepanel), wmview->wm_view);
    return null_ret();
}

FUNC(GetFilePanelAccessoryView)
{
    object_t	*wmfilepanel;
    WMView	*view;

    if (ici_typecheck("o", &wmfilepanel))
	return 1;
    if (!iswmfilepanel(wmfilepanel))
	return ici_argerror(0);
    view = WMGetFilePanelAccessoryView(wmfilepanelof(wmfilepanel));
    return ici_ret_with_decref(objof(new_wmview(view)));
}

FUNC(GetFontPanel)
{
    wmscreen_t	*wmscreen;
    WMFontPanel	*panel;

    if (ici_typecheck("o", &wmscreen))
	return 1;
    if (!iswmscreen(objof(wmscreen)))
	return ici_argerror(0);
    panel = WMGetFontPanel(wmscreen->wm_screen);
    return ici_ret_with_decref(objof(new_wmfontpanel(panel)));
}

FUNC(ShowFontPanel)
{
    wmfontpanel_t	*wmfontpanel;

    if (ici_typecheck("o", &wmfontpanel))
	return 1;
    if (!iswmfontpanel(objof(wmfontpanel)))
	return ici_argerror(0);
    WMShowFontPanel(wmfontpanel->wm_fontpanel);
    return null_ret();
}

FUNC(HideFontPanel)
{
    wmfontpanel_t	*wmfontpanel;

    if (ici_typecheck("o", &wmfontpanel))
	return 1;
    if (!iswmfontpanel(objof(wmfontpanel)))
	return ici_argerror(0);
    WMHideFontPanel(wmfontpanel->wm_fontpanel);
    return null_ret();
}

FUNC(SetFontPanelFont)
{
    wmfontpanel_t	*wmfontpanel;
    wmfont_t		*wmfont;
    
    if (ici_typecheck("oo", &wmfontpanel, &wmfont))
	return 1;
    if (!iswmfontpanel(objof(wmfontpanel)))
	return ici_argerror(0);
    if (!iswmfont(objof(wmfont)))
	return ici_argerror(1);
    WMSetFontPanelFont(wmfontpanel->wm_fontpanel, wmfont->wm_font);
    return null_ret();
}

FUNC(GetFontPanelFontName)
{
    wmfontpanel_t	*wmfontpanel;

    if (ici_typecheck("o", &wmfontpanel))
	return 1;
    if (!iswmfontpanel(objof(wmfontpanel)))
	return ici_argerror(0);
    return str_ret(WMGetFontPanelFontName(wmfontpanel->wm_fontpanel));
}

FUNC(GetFontPanelFont)
{
    wmfontpanel_t	*wmfontpanel;
    WMFont		*font;

    if (ici_typecheck("o", &wmfontpanel))
	return 1;
    if (!iswmfontpanel(objof(wmfontpanel)))
	return ici_argerror(0);
    font = WMGetFontPanelFont(wmfontpanel->wm_fontpanel);
    return ici_ret_with_decref(objof(new_wmfont(font)));
}

void
wAbort(void)
{
    exit(1);
}
