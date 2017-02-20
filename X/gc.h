#ifndef ICI_X_GC_H
#define	ICI_X_GC_H

/*
 * $Id: gc.h,v 1.2 1999/05/23 04:42:21 andy Exp $
 */

#ifndef ICI_X_DEFS_H
#include "defs.h"
#endif

struct ici_X_gc
{
    object_t		o_head;
    GC			g_gc;
};

extern type_t	ici_X_gc_type;

#define	gcof(o)	((ici_X_gc_t *)(o))
#define	isgc(o)	((o)->o_type == &ici_X_gc_type)

ici_X_gc_t	*ici_X_new_gc(GC);

#endif
