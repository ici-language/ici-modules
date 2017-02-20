#ifndef ICI_WMAKER_H
#define	ICI_WMAKER_H

#ifndef ICI_OBJECT_H
#include "object.h"
#endif

typedef struct ici_wm_appcontext
{
    object_t		o_head;
    WMAppContext	*app_context;
}
ici_wm_appcontext_t;

extern type_t		ici_wm_appcontext_type;

#define	appcontextof(o)	((ici_wm_appcontext_t *)(o))
#define	isappcontext(o)	((o)->o_type == &ici_wm_appcontext_type)

ici_wm_appcontext_t	*ici_WM_new_appcontext(WMAppContext *);

typedef struct ici_wm_menu
{
    object_t	o_head;
    WMMenu	*wm_menu;
}
ici_wm_menu_t;

extern type_t		ici_wm_menu_type;

#define	menuof(o)	((ici_wm_menu_t *)(o))
#define	ismenu(o)	((o)->o_type == &ici_wm_menu_type)

ici_wm_menu_t		*ici_WM_new_menu(WMMenu *);

#endif
