/*
 * ICI language binding to the DEC/INRIA "bignum" high-precision interger arithmetic package
 *
 *
 * This --intro-- forms part of the --ici-bignum-- documentation.
 */

#include <ici.h>
#include "icistr.h"
#include <icistr-setup.h>

#include "BigNum.h"
#include "BigZ.h"

static BigZ	zero;

static void
ici_bignum_pre_free(ici_handle_t *h)
{
    BzFree((BigZ)h->h_ptr);
}

static ici_handle_t *
new_bignum(BigZ z)
{
    ici_handle_t *h;

    if ((h = ici_handle_new(z, ICIS(bignum), NULL)) != NULL)
    {
        h->h_pre_free = ici_bignum_pre_free;
        objof(h)->o_flags &= ~O_SUPER;
    }
    return h;
}

/*
 * Called to set error when we fail to allocate a bignum.
 */
static int
allocerr(void)
{
    ici_error = "failed to allocate a BigNum";
    return 1;
}

/*
 * bignum = bignum.bignum(int|string)
 *
 * Convert object to bignum.
 *
 * This --topic-- forms part of the --ici-bignum-- documentation.
 */
static int
ici_bignum_bignum(void)
{
    BigZ	z;

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
	    char n[80];

	    ici_objname(n, ARG(0));
	    sprintf(ici_buf, "unable to convert a %s to a bignum", n);
	    ici_error = ici_buf;
	    return 1;
	}
    }
    if (z == NULL)
        return allocerr();
    return ici_ret_with_decref(objof(new_bignum(z)));
}

/* 
 * bignum = bignum.create(ndigits)
 *
 * Create a BigNum with room to store ndigits digits.
 *
 * This --topic-- forms part of the --ici-bignum-- documentation.
 */
static int
ici_bignum_create(void)
{
    long	ndigits;
    BigZ	z;

    if (ici_typecheck("i", &ndigits))
	return 1;
    if (ndigits < 1)
    {
	ici_error = "attempt to create bignum with zero (or fewer) digits";
	return 1;
    }
    if ((z = BzCreate(ndigits)) == NULL)
        return allocerr();
    return ici_ret_with_decref(objof(new_bignum(z)));
}

/* 
 * string = bignum.tostring(bignum)
 *
 * This --topic-- forms part of the --ici-bignum-- documentation.
 */
static int
ici_bignum_tostring(void)
{
    ici_handle_t *bn;
    char	*s;
    string_t	*str;

    if (ici_typecheck("h", ICIS(bignum), &bn))
	return 1;
    s = BzToString((BigZ)bn->h_ptr, 10);
    str = ici_str_new_nul_term(s);
    BzFreeString(s);
    return ici_ret_with_decref(objof(str));
}

static int
glue_N_N(void)
{
    ici_handle_t *a;
    BigZ (*pf)(BigZ);
    BigZ z;

    if (ici_typecheck("h", ICIS(bignum), &a))
	return 1;
    pf = CF_ARG1();
    z = (*pf)((BigZ)a->h_ptr);
    return ici_ret_with_decref(objof(new_bignum(z)));
}

static int
glue_NN_N(void)
{
    ici_handle_t *a;
    ici_handle_t *b;
    BigZ (*pf)(BigZ, BigZ);
    BigZ z;

    if (ici_typecheck("hh", ICIS(bignum), &a, ICIS(bignum), &b))
	return 1;
    pf = CF_ARG1();
    z = (*pf)((BigZ)a->h_ptr, (BigZ)b->h_ptr);
    return ici_ret_with_decref(objof(new_bignum(z)));
}

/*
 * bignum = bignum.div(bignum, bignum)
 *
 * This --topic-- forms part of the --ici-bignum-- documentation.
 */
static int
ici_bignum_div(void)
{
    ici_handle_t *a;
    ici_handle_t *b;
    BigZ z;

    if (ici_typecheck("hh", ICIS(bignum), &a, ICIS(bignum), &b))
	return 1;
    if (BzCompare((BigZ)b->h_ptr, zero) == 0)
    {
	ici_error = "division by zero";
	return 1;
    }
    z = BzDiv((BigZ)a->h_ptr, (BigZ)b->h_ptr);
    return ici_ret_with_decref(objof(new_bignum(z)));
}

/*
 * int = bignum.compare(bignum, bignum)
 *
 * This --topic-- forms part of the --ici-bignum-- documentation.
 */
static int
ici_bignum_compare(void)
{
    ici_handle_t *a;
    ici_handle_t *b;

    if (ici_typecheck("hh", ICIS(bignum), &a, ICIS(bignum), &b))
	return 1;
    return ici_int_ret(BzCompare((BigZ)a->h_ptr, (BigZ)b->h_ptr));
}

static cfunc_t ici_bignum_cfuncs[] =
{
    {CF_OBJ, "bignum",   ici_bignum_bignum},
    {CF_OBJ, "create",   ici_bignum_create},
    {CF_OBJ, "tostring", ici_bignum_tostring},
    {CF_OBJ, "div",      ici_bignum_div},
    {CF_OBJ, "compare",  ici_bignum_compare},

/*
 * bignum = bignum.negate(bignum)
 *
 * This --topic-- forms part of the --ici-bignum-- documentation.
 */
    {CF_OBJ, "negate",	 glue_N_N,  BzNegate},

/*
 * bignum = bignum.abs(bignum)
 *
 * This --topic-- forms part of the --ici-bignum-- documentation.
 */
    {CF_OBJ, "abs",	 glue_N_N,  BzAbs},

/*
 * bignum = bignum.add(bignum, bignum)
 *
 * This --topic-- forms part of the --ici-bignum-- documentation.
 */
    {CF_OBJ, "add",	 glue_NN_N, BzAdd},

/*
 * bignum = bignum.sub(bignum, bignum)
 *
 * This --topic-- forms part of the --ici-bignum-- documentation.
 */
    {CF_OBJ, "sub",	 glue_NN_N, BzSubtract},

/*
 * bignum = bignum.mult(bignum, bignum)
 *
 * This --topic-- forms part of the --ici-bignum-- documentation.
 */
    {CF_OBJ, "mult",	 glue_NN_N, BzMultiply},

/*
 * bignum = bignum.mod(bignum, bignum)
 *
 * This --topic-- forms part of the --ici-bignum-- documentation.
 */
    {CF_OBJ, "mod",	 glue_NN_N, BzMod},
    {CF_OBJ}
};

object_t *
ici_bignum_library_init(void)
{
    if (ici_interface_check(ICI_VER, ICI_BACK_COMPAT_VER, "bignum"))
        return NULL;
    if (init_ici_str())
        return NULL;
    BzInit();
    zero = BzFromInteger(0);
    return objof(ici_module_new(ici_bignum_cfuncs));
}
