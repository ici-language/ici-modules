#ifndef ICI_LONG_LONG_H
#define	ICI_LONG_LONG_H

#ifndef ICI_OBJECT_H
#include <object.h>
#endif

typedef struct
{
	object_t	o_head;
	long long int	l_value;
}
long_t;

#define	longof(o)	((long_t *)(o))
#define	islong(o)	((o)->o_type == &ici_long_type)

extern type_t		ici_long_type;

long_t			*ici_long_new_long(long long);

#endif
