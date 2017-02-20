#ifndef ICI_LIST_H
#define	ICI_LIST_H

#ifndef ICI_OBJECT_H
#include <object.h>
#endif

/*
 * A listnode is used to record an element stored in a list.
 * Listnodes are objects in their own right.
 */
typedef struct listnode_s listnode_t;
struct listnode_s
{
    object_t	o_head;
    listnode_t	*ln_next;
    listnode_t	*ln_prev;
    object_t	*ln_obj;
};

#define	islistnode(o)	((o)->o_type == &ici_listnode_type)
#define	listnodeof(o)	((listnode_t *)(o))

extern type_t		ici_listnode_type;
extern listnode_t	*ici_new_listnode(object_t *);

/*
 * Lists store references to listnode along with some other information
 * to speed access to elements.
 */
typedef struct list_s
{
    object_t	 o_head;
    listnode_t	*l_head;
    listnode_t	*l_tail;
    listnode_t	*l_ptr;
    long	 l_idx;
    unsigned int l_nels;
}
list_t;

#define	islist(o)	((o)->o_type == &ici_list_type)
#define	listof(o)	((list_t *)(o))

extern type_t		ici_list_type;
extern list_t		*ici_new_list(void);

enum
{
    ICI_LIST_INDEX_INVALID = -2,
};

#endif /* #ifndef ICI_LIST_H */
