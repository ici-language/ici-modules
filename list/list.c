#include "list.h"
#include "ici.h"

NEED_STRING(nels);
NEED_STRING(head);
NEED_STRING(tail);
NEED_STRING(next);
NEED_STRING(prev);
NEED_STRING(obj);

static unsigned long
mark_listnode(object_t *o)
{
    o->o_flags |= O_MARK;
    return sizeof (listnode_t) + mark(listnodeof(o)->ln_obj);
}

static object_t *
fetch_listnode(object_t *o, object_t *k)
{
    listnode_t	*ln = listnodeof(o);

    NEED_STRINGS(NULL);
    if (k == objof(STRING(obj)))
	return ln->ln_obj;
    if (k == objof(STRING(next)))
	return ln->ln_next ?  objof(ln->ln_next) : objof(&o_null);
    if (k == objof(STRING(prev)))
	return ln->ln_prev ? objof(ln->ln_prev) : objof(&o_null);
    return fetch_simple(o, k);
}

type_t	ici_listnode_type =
{
    mark_listnode,
    free_simple,
    hash_unique,
    cmp_unique,
    copy_simple,
    assign_simple,
    fetch_listnode,
    "listnode"
};

listnode_t *
ici_new_listnode(object_t *o)
{
    listnode_t	*ls;

    if ((ls = talloc(listnode_t)) != NULL)
    {
	objof(ls)->o_tcode = TC_OTHER;
	objof(ls)->o_nrefs = 1;
	objof(ls)->o_flags = 0;
	objof(ls)->o_type  = &ici_listnode_type;
	ls->ln_next = NULL;
	ls->ln_prev = NULL;
	ls->ln_obj  = o;
	rego(ls);
    }
    return ls;
}

static unsigned long
mark_list(object_t *o)
{
    listnode_t		*ls;
    unsigned long	sz;

    o->o_flags |= O_MARK;
    sz = sizeof (list_t);
    for (ls = listof(o)->l_head; ls != NULL; ls = ls->ln_next)
	sz += mark(ls);
    return sz;
}

static object_t *
copy_list(object_t *o)
{
    list_t	*l;
    listnode_t	*ls;
    listnode_t	*nls;

    if ((l = talloc(list_t)) == NULL)
	return NULL;
    objof(l)->o_tcode = TC_OTHER;
    objof(l)->o_nrefs = 1;
    objof(l)->o_flags = 0;
    objof(l)->o_type  = &ici_list_type;
    rego(l);
    l->l_head = NULL;
    l->l_tail = NULL;
    l->l_nels = listof(o)->l_nels;
    l->l_ptr  = NULL;
    l->l_idx  = ICI_LIST_INDEX_INVALID;
    for (ls = listof(o)->l_head; ls != NULL; ls = ls->ln_next)
    {
	if ((nls = ici_new_listnode(ls->ln_obj)) == NULL)
	    goto fail;
	if (l->l_tail == NULL)
	{
	    l->l_head = nls;
	    l->l_tail = nls;
	}
	else
	{
	    nls->ln_prev = l->l_tail;
	    l->l_tail = nls;
	}
	decref(nls);
    }
    return objof(l);

fail:
    decref(l);
    return NULL;
}

static object_t *
fetch_list(object_t *o, object_t *k)
{
    list_t	*l = listof(o);
    listnode_t	*ls;
    object_t	*v;
    long	idx;
    long	i;

    NEED_STRINGS(NULL);

    /* integer index means return k'th element of list */
    if (isint(k))
    {
	idx = intof(k)->i_value;
	if (idx < 0 || idx >= l->l_nels)
	    return fetch_simple(objof(l), k);
	if (idx == l->l_idx)
	    return l->l_ptr->ln_obj;
	if (idx == l->l_idx + 1)
	{
	    l->l_ptr = l->l_ptr->ln_next;
	    l->l_idx = idx;
	    return l->l_ptr->ln_obj;
	}
	if (idx == l->l_idx - 1)
	{
	    l->l_ptr = l->l_ptr->ln_prev;
	    l->l_idx = idx;
	    return l->l_ptr->ln_obj;
	}
	if (idx < l->l_idx)
	{
	    if (l->l_ptr == NULL || l->l_idx - idx > idx)
		for (ls = l->l_head, i = 0; i < idx; ++i)
		    ls = ls->ln_next;
	    else
		for (ls = l->l_ptr, i = l->l_idx; i > 0; --i)
		    ls = ls->ln_prev;
	}
	else
	{
	    if (l->l_ptr && l->l_nels - idx > idx)
		for (ls = l->l_ptr, i = l->l_idx; i < idx; ++i)
		    ls = ls->ln_next;
	    else
		for (ls = l->l_tail, i = l->l_nels-1; i > idx; --i)
		    ls = ls->ln_prev;
	}
	assert(ls != NULL);
	if (ls == NULL)
	{
	    error = "internal inconsistency in list code, nels is wrong!";
	    return NULL;
	}
	l->l_ptr = ls;
	l->l_idx = idx;
	return ls->ln_obj;
    }

    /* list.nels returns number of elements in list */
    if (k == objof(STRING(nels)))
    {
	if ((v = objof(new_int(l->l_nels))) != NULL)
	    decref(v);
	return v;
    }
    /* list.head returns head element of list */
    if (k == objof(STRING(head)))
    {
	if (l->l_head == NULL)
	    return objof(&o_null);
	return objof(l->l_head);
    }
    /* list.tail returns tail element of list */
    if (k == objof(STRING(tail)))
    {
	if (l->l_tail == NULL)
	    return objof(&o_null);
	return objof(l->l_tail);
    }

    /* all others fail */
    return fetch_simple(objof(l), k);
}

type_t
ici_list_type =
{
    mark_list,
    free_simple,
    hash_unique,
    cmp_unique,
    copy_list,
    assign_simple,
    fetch_list,
    "list"
};

list_t *
ici_new_list(void)
{
    list_t	*l;

    if ((l = talloc(list_t)) != NULL)
    {
	objof(l)->o_tcode = TC_OTHER;
	objof(l)->o_nrefs = 1;
	objof(l)->o_flags = 0;
	objof(l)->o_type  = &ici_list_type;
	l->l_head = NULL;
	l->l_tail = NULL;
	l->l_nels = 0;
	l->l_ptr  = NULL;
	l->l_idx  = ICI_LIST_INDEX_INVALID;
    }
    return l;
}

/*
 * list = list.list([any...])
 */
FUNC(list)
{
    list_t	*l;
    listnode_t	*ln;
    int		i;

    if ((l = ici_new_list()) == NULL)
	return 1;
    for (i = 0; i < NARGS(); ++i)
    {
	if ((ln = ici_new_listnode(ARG(i))) == NULL)
	{
	    decref(l);
	    return 1;
	}
	if (l->l_tail == NULL)
	{
	    l->l_head = ln;
	    l->l_tail = ln;
	}
	else
	{
	    if ((ln->ln_prev = l->l_tail) != NULL)
		ln->ln_prev->ln_next = ln;
	    l->l_tail = ln;
	}
	decref(ln);
	++l->l_nels;
    }
    return ici_ret_with_decref(objof(l));
}

/*
 * object = list.addhead(list, object)
 */
FUNC(addhead)
{
    list_t	*l;
    object_t	*o;
    listnode_t	*ln;

    if (ici_typecheck("oo", &l, &o))
	return 1;
    if (!islist(objof(l)))
	return ici_argerror(0);
    if ((ln = ici_new_listnode(o)) == NULL)
	return 1;
    if (l->l_head == NULL)
    {
	l->l_head = l->l_tail = l->l_ptr = ln;
	l->l_idx  = 0;
	l->l_nels = 1;
    }
    else
    {
	ln->ln_next = l->l_head;
	l->l_head->ln_prev = ln;
	l->l_head = ln;
	++l->l_nels;
	if (l->l_idx >= 0)
	    ++l->l_idx;
    }
    decref(ln);
    return ici_ret_no_decref(o);
}

/*
 * object = list.addtail(list, object)
 */
FUNC(addtail)
{
    list_t	*l;
    listnode_t	*ln;
    object_t	*o;

    if (ici_typecheck("oo", &l, &o))
	return 1;
    if (!islist(objof(l)))
	return ici_argerror(0);
    if ((ln = ici_new_listnode(o)) == NULL)
	return 1;
    if (l->l_head == NULL)
    {
	l->l_head = l->l_tail = l->l_ptr = ln;
	l->l_idx  = 0;
	l->l_nels = 1;
    }
    else
    {
	l->l_tail->ln_next = ln;
	ln->ln_prev = l->l_tail;
	l->l_tail = ln;
	++l->l_nels;
    }
    decref(ln);
    return ici_ret_no_decref(o);
}

/*
 * listnode = list.head(list)
 */
FUNC(head)
{
    list_t	*l;

    if (ici_typecheck("o", &l))
	return 1;
    if (!islist(objof(l)))
	return ici_argerror(0);
    if (l->l_head == NULL)
	return null_ret();
    return ici_ret_no_decref(objof(l->l_head));
}

/*
 * listnode = list.tail(list)
 */
FUNC(tail)
{
    list_t	*l;

    if (ici_typecheck("o", &l))
	return 1;
    if (!islist(objof(l)))
	return ici_argerror(0);
    if (l->l_tail == NULL)
	return null_ret();
    return ici_ret_no_decref(objof(l->l_tail));
}

/*
 * object = list.remhead(list)
 */
FUNC(remhead)
{
    list_t	*l;
    listnode_t	*ln;

    if (ici_typecheck("o", &l))
	return 1;
    if (!islist(objof(l)))
	return ici_argerror(0);
    if (l->l_nels == 0)
	return null_ret();
    ln = l->l_head;
    if ((l->l_head = ln->ln_next) != NULL)
	l->l_head->ln_prev = NULL;
    else
	l->l_tail = NULL;
    --l->l_nels;
    if (l->l_idx > 0)
	--l->l_idx;
    else
	l->l_idx = ICI_LIST_INDEX_INVALID;
    return ici_ret_no_decref(ln->ln_obj);
}

/*
 * object = list.remtail(list)
 */
FUNC(remtail)
{
    list_t	*l;
    listnode_t	*ln;

    if (ici_typecheck("o", &l))
	return 1;
    if (!islist(objof(l)))
	return ici_argerror(0);
    if (l->l_nels == 0)
	return null_ret();
    ln = l->l_tail;
    if ((l->l_tail = ln->ln_prev) != NULL)
	l->l_tail->ln_next = NULL;
    else
	l->l_head = NULL;
    if (l->l_idx == --l->l_nels)
	l->l_idx = ICI_LIST_INDEX_INVALID;
    return ici_ret_no_decref(ln->ln_obj);
}
