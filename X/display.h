#ifndef ICI_X_DISPLAY_H
#define	ICI_X_DISPLAY_H

/*
 * $Id: display.h,v 1.3 1999/02/04 13:14:00 andy Exp $
 */

#ifndef ICI_X_DEFS_H
#include "defs.h"
#endif

struct ici_X_display
{
    object_t	o_head;
    Display	*d_display;
};

extern type_t		ici_X_display_type;

#define	displayof(o)	((ici_X_display_t *)(o))
#define	isdisplay(o)	((o)->o_type == &ici_X_display_type)

ici_X_display_t	*ici_X_new_display(Display *);

#endif
