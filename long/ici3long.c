#include "long.h"
#include "ici.h"

static unsigned long
mark_long(object_t *o)
{
	o->o_flags |= O_MARK;
	return sizeof (long_t);
}

static unsigned long
hash_long(object_t *o)
{
	return 17 * (longof(o)->l_value & 0xFFFFFFFF) ^ (longof(o)->l_value >> 32);
}

static int
cmp_long(object_t *a, object_t *b)
{
	return longof(a)->l_value != longof(b)->l_value;
}

type_t	ici_long_type =
{
	mark_long,
	free_simple,
	hash_long,
	cmp_long,
	copy_simple,
	assign_simple,
	fetch_simple,
	"long"
};

static long_t *
atom_long(long long int value)
{
	object_t	*o;
	object_t	**po;
	unsigned long	hval;

	for
	(
		po = &atoms[ici_atom_hash_index(hval)];
		(o = *po) != NULL;
		--po < atoms ? po = atoms + atomsz - 1 : NULL
	)
	{
		if (islong(o) && longof(o)->l_value == value)
			return longof(o);
	}
	return NULL;
}

long_t *
ici_long_new_long(long long int value)
{
	long_t	*l;

	if ((l = atom_long(value)) != NULL)
	{
		incref(l);
		return l;
	}
	if ((l = talloc(long_t)) != NULL)
	{
		objof(l)->o_tcode = TC_OTHER;
		objof(l)->o_flags = 0;
		objof(l)->o_nrefs = 1;
		objof(l)->o_type  = &ici_long_type;
		rego(l);
		objof(l)->o_leafz = 0;
		l->l_value = value;
		l = longof(atom(objof(l), 1));
	}
	return l;
}

FUNC(long)
{
	long		a;
	long		b;

	if (NARGS() == 0)
	{
	    a = b = 0;
	}
	else if (NARGS() == 1)
	{
		if (isint(ARG(0)))
			a = intof(ARG(0))->i_value;
		else if (isstring(ARG(0)))
		{
			long long	value;

			if (sscanf(stringof(ARG(0))->s_chars, "%lld", &value) != 1)
			{
				error = "invalid long value";
				return 1;
			}
			return ici_ret_with_decref(objof(ici_long_new_long(value)));
		}
		else
			return ici_argerror(0);
		b = 0;
	}
	else if (NARGS() == 2)
	{
		if (ici_typecheck("ii", &b, &a))
			return 1;
	}
	else
		return ici_argcount(2);

	return ici_ret_with_decref(objof(ici_long_new_long(((long long)b << 32LL) | a)));
}

FUNC(tostring)
{
	char	mybuf[32];

	if (NARGS() != 1)
		return ici_argcount(1);
	if (!islong(ARG(0)))
		return ici_argerror(0);
	sprintf(mybuf, "%lld", longof(ARG(0))->l_value);
	return str_ret(mybuf);
}

FUNC(tohexstring)
{
	char	mybuf[64];

	if (NARGS() != 1)
		return ici_argcount(1);
	if (!islong(ARG(0)))
		return ici_argerror(0);
	sprintf(mybuf, "0x%0llX", longof(ARG(0))->l_value);
	return str_ret(mybuf);
}


FUNC(negate)
{
	if (NARGS() != 1)
		return ici_argcount(1);
	if (!islong(ARG(0)))
		return ici_argerror(0);
	return ici_ret_with_decref(objof(ici_long_new_long(- longof(ARG(0))->l_value)));
}

FUNC(abs)
{
	long long	value;

	if (NARGS() != 1)
		return ici_argcount(1);
	if (!islong(ARG(0)))
		return ici_argerror(0);
	if ((value = longof(ARG(0))->l_value) < 0)
		value = -value;
	return ici_ret_with_decref(objof(ici_long_new_long(value)));
}

FUNC(and)
{
	long_t	*a;
	long_t	*b;


	if (ici_typecheck("oo", &a, &b))
		return 1;
	if (!islong(objof(a)))
		return ici_argerror(0);
	if (!islong(objof(b)))
		return ici_argerror(1);
	return ici_ret_with_decref(objof(ici_long_new_long(a->l_value & b->l_value)));
}

FUNC(or)
{
	long_t	*a;
	long_t	*b;


	if (ici_typecheck("oo", &a, &b))
		return 1;
	if (!islong(objof(a)))
		return ici_argerror(0);
	if (!islong(objof(b)))
		return ici_argerror(1);
	return ici_ret_with_decref(objof(ici_long_new_long(a->l_value | b->l_value)));
}

FUNC(xor)
{
	long_t	*a;
	long_t	*b;


	if (ici_typecheck("oo", &a, &b))
		return 1;
	if (!islong(objof(a)))
		return ici_argerror(0);
	if (!islong(objof(b)))
		return ici_argerror(1);
	return ici_ret_with_decref(objof(ici_long_new_long(a->l_value ^ b->l_value)));
}

FUNC(shiftleft)
{
	long_t	*a;
	long	b;

	if (ici_typecheck("oi", &a, &b))
		return 1;
	if (!islong(objof(a)))
		return ici_argerror(0);
	return ici_ret_with_decref(objof(ici_long_new_long(a->l_value << b)));
}

FUNC(shiftright)
{
	long_t	*a;
	long	b;

	if (ici_typecheck("oi", &a, &b))
		return 1;
	if (!islong(objof(a)))
		return ici_argerror(0);
	return ici_ret_with_decref(objof(ici_long_new_long(a->l_value >> b)));
}

FUNC(add)
{
	long_t	*a;
	long_t	*b;


	if (ici_typecheck("oo", &a, &b))
		return 1;
	if (!islong(objof(a)))
		return ici_argerror(0);
	if (!islong(objof(b)))
		return ici_argerror(1);
	return ici_ret_with_decref(objof(ici_long_new_long(a->l_value + b->l_value)));
}

FUNC(sub)
{
	long_t	*a;
	long_t	*b;


	if (ici_typecheck("oo", &a, &b))
		return 1;
	if (!islong(objof(a)))
		return ici_argerror(0);
	if (!islong(objof(b)))
		return ici_argerror(1);
	return ici_ret_with_decref(objof(ici_long_new_long(a->l_value - b->l_value)));
}

FUNC(mult)
{
	long_t	*a;
	long_t	*b;


	if (ici_typecheck("oo", &a, &b))
		return 1;
	if (!islong(objof(a)))
		return ici_argerror(0);
	if (!islong(objof(b)))
		return ici_argerror(1);
	return ici_ret_with_decref(objof(ici_long_new_long(a->l_value * b->l_value)));
}

FUNC(div)
{
	long_t	*a;
	long_t	*b;


	if (ici_typecheck("oo", &a, &b))
		return 1;
	if (!islong(objof(a)))
		return ici_argerror(0);
	if (!islong(objof(b)))
		return ici_argerror(1);
	if (b->l_value == 0)
	{
		error = "division by zero";
		return 1;
	}
	return ici_ret_with_decref(objof(ici_long_new_long(a->l_value / b->l_value)));
}

FUNC(mod)
{
	long_t	*a;
	long_t	*b;

	if (ici_typecheck("oo", &a, &b))
		return 1;
	if (!islong(objof(a)))
		return ici_argerror(0);
	if (!islong(objof(b)))
		return ici_argerror(1);
	if (b->l_value == 0)
	{
		error = "division by zero";
		return 1;
	}
	return ici_ret_with_decref(objof(ici_long_new_long(a->l_value % b->l_value)));
}

FUNC(compare)
{
	long_t	*a;
	long_t	*b;

	if (ici_typecheck("oo", &a, &b))
		return 1;
	if (!islong(objof(a)))
		return ici_argerror(0);
	if (!islong(objof(b)))
		return ici_argerror(1);
	if (a->l_value == b->l_value)
		return int_ret(0);
	if (a->l_value > b->l_value)
		return int_ret(1);
	return int_ret(-1);
}

FUNC(top)
{
	if (NARGS() != 1)
		return ici_argcount(1);
	if (!islong(ARG(0)))
		return ici_argerror(0);
	return int_ret(longof(ARG(0))->l_value >> 32);
}

FUNC(bot)
{
	if (NARGS() != 1)
		return ici_argcount(1);
	if (!islong(ARG(0)))
		return ici_argerror(0);
	return int_ret(longof(ARG(0))->l_value & 0xFFFFFFFF);
}
