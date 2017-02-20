#ifndef ICI_X_COLOR_H
#define	ICI_X_COLOR_H

/*
 * $Id: color.h,v 1.1 1999/05/23 04:42:21 andy Exp $
 */

#ifndef ICI_X_DEFS_H
#include "defs.h"
#endif

struct ici_X_color
{
    object_t		o_head;
    XColor		c_color;
};

extern type_t		ici_X_color_type;
#define	colorof(o)	((ici_X_color_t *)(o))
#define	iscolor(o)	((o)->o_type == &ici_X_color_type)
ici_X_color_t		*ici_X_new_color(XColor *);

#endif
