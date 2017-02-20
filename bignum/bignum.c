#include "ici.h"
#include "BigNum.h"
#include "BigZ.h"

static BigZ	zero;

typedef struct
{
    object_t	o_head;
    BigZ	b_num;
}
bignum_t;

inline
static bignum_t *
bignumof(void *p)
{
    return (bignum_t *)p;
}

static unsigned long
mark_bignum(object_t *o)
{
    o->o_flags |= O_MARK;
    return sizeof (bignum_t) + sizeof (struct BigZStruct);
}

static void
free_bignum(object_t *o)
{
    BzFree(bignumof(o)->b_num);
    ici_free(o);
}

static type_t	bignum_type =
{
    mark_bignum,
    free_bignum,
    hash_unique,
    cmp_unique,
    copy_simple,
    assign_simple,
    fetch_simple,
    "bignum"
};

inline
static int
isbignum(object_t *o)
{
    return o->o_type == &bignum_type;
}

/*
 * Called to set error when we fail to allocate a bignum.
 */
inline
static int
allocerr(void)
{
    error = "failed to allocate BigNum";
    return 1;
}

/*
 * new_bignum - create a new ICI bignum object.
 */
static bignum_t *
new_bignum(BigZ bigz)
{
    bignum_t	*bn = 0;

    if (bigz == NULL)
	allocerr();
    else if ((bn = talloc(bignum_t)) != NULL)
    {
	objof(bn)->o_tcode = TC_OTHER;
	objof(bn)->o_nrefs = 1;
	objof(bn)->o_flags = 0;
	objof(bn)->o_type  = &bignum_type;
	bn->b_num	   = bigz;
	rego(objof(bn));
    }
    return bn;
}

/*
 * Non-zero if we need to call BzInit().
 */
static	int need_init = 1;

/*
 * Initialise if required.
 */
inline
static void
checkinit(void)
{
    if (need_init)
    {
	BzInit();
	zero = BzFromInteger(0);
	need_init = 0;
    }
}

/*
 * bignum = bignum.bignum(int|string)
 *
 * Convert object to bignum.
 */
FUNC(bignum)
{
    BigZ	z;

    checkinit();
    if (NARGS() > 1)
	return ici_argcount(1);
    if (NARGS() == 0)
	z = BzFromInteger(0);
    else
    {
	if (isstring(ARG(0)))
	    z = BzFromString(stringof(ARG(0))->s_chars, 10);
	else if (isint(ARG(0)))
	    z = BzFromInteger(intof(ARG(0))->i_value);
	else
	{
	    char	n[80];

	    objname(n, ARG(0));
	    sprintf(buf, "unable to convert a %s to a bignum", n);
	    error = buf;
	    return 1;
	}
    }
    return z == NULL ? allocerr() : ici_ret_with_decref(objof(new_bignum(z)));
}

/* 
 * bignum = bignum.create(ndigits)
 *
 * Create a BigNum with room to store ndigits digits.
 */
FUNC(create)
{
    long	ndigits;
    BigZ	z;

    checkinit();
    if (ici_typecheck("i", &ndigits))
	return 1;
    if (ndigits < 1)
    {
	error = "attempt to create bignum with zero (or fewer) digits";
	return 1;
    }
    z = BzCreate(ndigits);
    return z == NULL ? allocerr() : ici_ret_with_decref(objof(new_bignum(z)));
}

/* 
 * string = bignum.tostring(bignum)
 */
FUNC(tostring)
{
    bignum_t	*bn;
    char	*s;
    string_t	*str;

    checkinit();
    if (ici_typecheck("o", &bn))
	return 1;
    if (!isbignum(objof(bn)))
	return ici_argerror(0);
    s = BzToString(bn->b_num, 10);
    str = new_cname(s);
    BzFreeString(s);
    return ici_ret_with_decref(objof(str));
}

FUNCDEF(glue_N_N)
{
    bignum_t	*a;

    if (ici_typecheck("o", &a))
	return 1;
    if (!isbignum(objof(a)))
	return ici_argerror(0);
    return ici_ret_with_decref
    (
	objof
	(
	    new_bignum((BigZ)(*CF_ARG1())(a->b_num))
	)
    );
}

FUNCDEF(glue_NN_N)
{
    bignum_t	*a;
    bignum_t	*b;

    if (ici_typecheck("oo", &a, &b))
	return 1;
    if (!isbignum(objof(a)))
	return ici_argerror(0);
    if (!isbignum(objof(b)))
	return ici_argerror(1);
    return ici_ret_with_decref
    (
	objof
	(
	    new_bignum((BigZ)(*CF_ARG1())(a->b_num, b->b_num))
	)
    );
}

CFUNC1(negate,	glue_N_N,  BzNegate);
CFUNC1(abs,	glue_N_N,  BzAbs);
CFUNC1(add,	glue_NN_N, BzAdd);
CFUNC1(sub,	glue_NN_N, BzSubtract);
CFUNC1(mult,	glue_NN_N, BzMultiply);
CFUNC1(mod,	glue_NN_N, BzMod);

FUNC(div)
{
    bignum_t	*a;
    bignum_t	*b;

    if (ici_typecheck("oo", &a, &b))
	return 1;
    if (!isbignum(objof(a)))
	return ici_argerror(0);
    if (!isbignum(objof(b)))
	return ici_argerror(1);
    if (BzCompare(b->b_num, zero) == 0)
    {
	error = "division by zero";
	return 1;
    }
    return ici_ret_with_decref
    (
	objof(new_bignum(BzDiv(a->b_num, b->b_num)))
    );
}

FUNC(compare)
{
    bignum_t	*a;
    bignum_t	*b;

    if (ici_typecheck("oo", &a, &b))
	return 1;
    if (!isbignum(objof(a)))
	return ici_argerror(0);
    if (!isbignum(objof(b)))
	return ici_argerror(1);
    return int_ret(BzCompare(a->b_num, b->b_num));
}

#if defined __MACH__ && defined __APPLE__
#include "cfuncs.c"
#include "strings.c"
#endif
