#ifndef ICI_X_KEYSYM_H
#define	ICI_X_KEYSYM_H

/*
 * $Id$
 */

#ifndef ICI_X_DEFS_H
#include "defs.h"
#endif

struct ici_X_keysym
{
    object_t		o_head;
    KeySym		k_keysym;
};

extern type_t		ici_X_keysym_type;

#define	keysymof(o)	((ici_X_keysym_t *)(o))
#define	iskeysym(o)	((o)->o_type == &ici_X_keysym_type)

ici_X_keysym_t	*ici_X_new_keysym(KeySym);

#endif
