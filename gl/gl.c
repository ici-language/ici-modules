/*
 * $Id: gl.c,v 1.1 1999/08/01 12:32:21 andy Exp $
 *
 * An ici opengl interface. Provides a pretty much one for one binding
 * to the OpenGL C API. Types are modified to be more ici friendly in
 * places (ici arrays are used for instance) and simplifications are
 * added where functions exist in multiple forms that differ in only
 * the type or number of parameters (as ici can determine these at
 * run-time).
 *
 * 
 */

#include <gl.h>
#include "ici.h"

/*
 * Glue functions for those GL functions that take between zero and six
 * double, float or integer arguments. The actual number of arguments
 * comes from the cf_arg2 member of the cfunc structure. The function
 * to call comes from the cf_arg1 member. All return (in ici) NULL.
 */

FUNCDEF(glue_d)
{
    double	a, b, c, d, e, f;

    if (ici_typecheck(CF_ARG2(), &a, &b, &c, &d, &e, &f))
	return 1;
    (*CF_ARG1())(a, b, c, d, e, f);
    return null_ret();
}

FUNCDEF(glue_f)
{
    double	a, b, c, d, e, f;

    if (ici_typecheck(CF_ARG2(), &a, &b, &c, &d, &e, &f))
	return 1;
    (*CF_ARG1())((float)a, (float)b, (float)c, (float)d, (float)e, (float)f);
    return null_ret();
}

FUNCDEF(glue_i)
{
    long	a, b, c, d, e, f;

    if (ici_typecheck(CF_ARG2(), &a, &b, &c, &d, &e, &f))
	return 1;
    (*CF_ARG1())((int)a, (int)b, (int)c, (int)d, (int)e, (int)f);
    return null_ret();
}

/*
 * Glue functions for GL functions that take N integer and M float args.
 * N and M are encoded in the function names.
 */

FUNCDEF(glue_1i1f)
{
    long	a;
    double	b;

    if (ici_typecheck("if", &a, &b))
	return 1;
    (*CF_ARG1())((int)a, (float)b);
    return null_ret();
}

FUNCDEF(glue_2i1f)
{
    long	a, b;
    double	c;

    if (ici_typecheck("iif", &a, &b, &c))
	return 1;
    (*CF_ARG1())((int)a, (int)b, (float)c);
    return null_ret();
}

/*
 * Glue function for GL functions that take from zero to six integers
 * and returns an int.
 */
FUNCDEF(glue_i_ri)
{
    long	a, b, c, d, e, f;

    if (ici_typecheck(CF_ARG2(), &a, &b, &c, &d, &e, &f))
	return 1;
    return int_ret((*CF_ARG1())((int)a, (int)b, (int)c, (int)d, (int)e, (int)f));
}

/*
 * Glue functions for GL functions taking vectors as arguments.
 *
 * Each C type gets a separate function. The expected size of the vector
 * is given by the length of the cf_arg2 string in the cfunc. The
 * actual string is not used however it serves as extra documentation.
 * The function to call is given by the cf_arg1 cfunc member.
 *
 * XXX Should merge all into one function that uses CF_ARG2 string to
 * know size/type of destination array.
 */

FUNCDEF(glue_ia)
{
    array_t	*a;
    int		ia[6];
    int		*i;
    object_t	**o;

    if (ici_typecheck("a", &a))
	return 1;
    if ((a->a_top - a->a_base) != strlen(CF_ARG2()))
    {
	error = "incorrect number of array elements";
	return 1;
    }
    for (i = ia, o = a->a_base; o < a->a_top; ++o, ++i)
    {
	if (!isint(*o))
	{
	    error = "non-integer in array";
	    goto fail;
	}
	*i = intof(*o)->i_value;
    }
    (*CF_ARG1())(ia);
    return null_ret();

fail:
    ici_free(ia);
    return 1;
}

FUNCDEF(glue_sa)
{
    array_t	*a;
    short	ia[6];
    short	*i;
    object_t	**o;

    if (ici_typecheck("a", &a))
	return 1;
    if ((a->a_top - a->a_base) != strlen(CF_ARG2()))
    {
	error = "incorrect number of array elements";
	return 1;
    }
    for (i = ia, o = a->a_base; o < a->a_top; ++o, ++i)
    {
	if (!isint(*o))
	{
	    error = "non-integer in array";
	    goto fail;
	}
	*i = intof(*o)->i_value;
    }
    (*CF_ARG1())(ia);
    return null_ret();

fail:
    ici_free(ia);
    return 1;
}

FUNCDEF(glue_da)
{
    array_t	*a;
    double	ia[6];
    double	*i;
    object_t	**o;

    if (ici_typecheck("a", &a))
	return 1;
    if ((a->a_top - a->a_base) != strlen(CF_ARG2()))
    {
	error = "incorrect number of array elements";
	return 1;
    }
    for (i = ia, o = a->a_base; o < a->a_top; ++o, ++i)
    {
	if (!isfloat(*o))
	{
	    error = "non-integer in array";
	    goto fail;
	}
	*i = floatof(*o)->f_value;
    }
    (*CF_ARG1())(ia);
    return null_ret();

fail:
    ici_free(ia);
    return 1;
}

FUNCDEF(glue_fa)
{
    array_t	*a;
    float	ia[6];
    float	*i;
    object_t	**o;

    if (ici_typecheck("a", &a))
	return 1;
    if ((a->a_top - a->a_base) != strlen(CF_ARG2()))
    {
	error = "incorrect number of array elements";
	return 1;
    }
    for (i = ia, o = a->a_base; o < a->a_top; ++o, ++i)
    {
	if (!isfloat(*o))
	{
	    error = "non-integer in array";
	    goto fail;
	}
	*i = floatof(*o)->f_value;
    }
    (*CF_ARG1())(ia);
    return null_ret();

fail:
    ici_free(ia);
    return 1;
}

/*
 * not_implemented - used as a return for functions not implemented yet
 */
static int
not_implemented(void)
{
    error = "function not implemented";
    return 1;
}

/*
 * Start defining the GL functions...
 *
 * Those that can be implemented using glue functions use the
 * various CFUNCx() macros. Otherwise we use FUNC() and define
 * the code.
 */

CFUNC3(ClearIndex, glue_f, glClearIndex, "f")
CFUNC3(ClearColor, glue_f, glClearColor, "ffff")
CFUNC3(Clear,	   glue_i, glClear, "i")
CFUNC3(IndexMask,  glue_i, glIndexMask, "i")
CFUNC3(ColorMask,  glue_i, glColorMask, "iiii")
CFUNC1(AlphaFunc,  glue_1i1f, glAlphaFunc)
CFUNC3(BlendFunc, glue_i, glBlendFunc, "ii")
CFUNC3(LogicOp,	  glue_i, glLogicOp,   "i")
CFUNC3(CullFace,  glue_i, glCullFace,  "i")
CFUNC3(FrontFace, glue_i, glFrontFace, "i")
CFUNC3(PointSize, glue_f, glPointSize, "f")
CFUNC3(LineWidth, glue_f, glLineWidth, "f")
CFUNC3(LineStipple, glue_i, glLineStipple, "ii")
CFUNC3(PolygonMode, glue_i, glPolygonMode, "ii")
CFUNC3(PolygonOffset, glue_f, glPolygonOffset, "ff")

FUNC(PolygonStipple)
{
    return not_implemented();
}

FUNC(GetPolygonStipple)
{
    return not_implemented();
}

CFUNC3(EdgeFlag, glue_i, glEdgeFlag, "i")

FUNC(EdgeFlagv)
{
    array_t		*a;
    unsigned char	v[1];

    if (ici_typecheck("a", &a))
	return 1;
    if (a->a_top - a->a_base != 1)
    {
	error = "incorrect number of array elements";
	return 1;
    }
    if (!isint(a->a_base[0]))
    {
	error = "incorrect type of array element";
	return 1;
    }
    v[0] = intof(a->a_base[0])->i_value;
    glEdgeFlagv(v);
    return null_ret();
}

CFUNC3(Scissor, glue_i, glScissor, "iiii")

FUNC(ClipPlane)
{
    long	plane;
    array_t	*a;
    double	v[4];
    int		i;

    if (ici_typecheck("ia", &plane, &a))
	return 1;
    if (a->a_top - a->a_base != 4)
    {
	error = "incorrect number of array elements";
	return 1;
    }
    for (i = 0; i < 4; ++i)
    {
	if (!isfloat(a->a_base[i]))
	{
	    error = "incorrect type of array element";
	    return 1;
	}
	v[i] = floatof(a->a_base[i])->f_value;
    }
    glClipPlane(plane, v);
    return null_ret();
}

FUNC(GetClipPlane)
{
    return not_implemented();
}

CFUNC3(DrawBuffer, glue_i, glDrawBuffer, "i")
CFUNC3(ReadBuffer, glue_i, glReadBuffer, "i")
CFUNC3(Enable, glue_i, glEnable, "i")
CFUNC3(Disable, glue_i, glDisable, "i")
CFUNC3(IsEnabled, glue_i_ri, glIsEnabled, "i")
CFUNC3(EnableClientState, glue_i, glEnableClientState, "i")
CFUNC3(DisableClientState, glue_i, glDisableClientState, "i")

FUNC(GetBooleanv)
{
    return not_implemented();
}

FUNC(GetDoublev)
{
    return not_implemented();
}

FUNC(GetFloatv)
{
    return not_implemented();
}

FUNC(GetIntegerv)
{
    return not_implemented();
}

CFUNC3(PushAttrib, glue_i, glPushAttrib, "i")
CFUNC3(PopAttrib, glue_i, glPopAttrib, "")
CFUNC3(PushClientAttrib, glue_i, glPushClientAttrib, "i")
CFUNC3(PopClientAttrib, glue_i, glPopClientAttrib, "")
CFUNC3(RenderMode, glue_i_ri, glRenderMode, "i")
CFUNC3(GetError, glue_i_ri, glGetError, "")

FUNC(GetString)
{
    long	name;

    if (ici_typecheck("i", &name))
	return 1;
    return str_ret((char *)glGetString(name));
}

CFUNC3(Finish, glue_i, glFinish, "")
CFUNC3(Flush, glue_i, glFlush, "")
CFUNC3(Hint, glue_i, glHint, "ii")
CFUNC3(ClearDepth, glue_f, glClearDepth, "f")
CFUNC3(DepthFunc, glue_i, glDepthFunc, "i")
CFUNC3(DepthMask, glue_i, glDepthMask, "i")
CFUNC3(DepthRange, glue_i, glDepthRange, "ii")
CFUNC3(ClearAccum, glue_f, glClearAccum, "ffff")
CFUNC1(Accum, glue_1i1f, glAccum)
CFUNC3(MatrixMode, glue_i, glMatrixMode, "i")
CFUNC3(Ortho, glue_d, glOrtho, "ffffff")
CFUNC3(Frustum, glue_d, glFrustum, "ffffff")
CFUNC3(Viewport, glue_i, glViewport, "iiii")
CFUNC3(PushMatrix, glue_i, glPushMatrix, "")
CFUNC3(PopMatrix, glue_i, glPopMatrix, "")
CFUNC3(LoadIdentity, glue_i, glLoadIdentity, "")

FUNC(LoadMatrixd)
{
    return not_implemented();
}

FUNC(LoadMatrixf)
{
    return not_implemented();
}

FUNC(MultMatrixd)
{
    return not_implemented();
}

FUNC(MultMatrixf)
{
    return not_implemented();
}

CFUNC3(Rotated, glue_d, glRotated, "ffff")
CFUNC3(Rotatef, glue_f, glRotatef, "ffff")
CFUNC3(Scaled, glue_d, glScaled, "fff")
CFUNC3(Scalef, glue_f, glScalef, "fff")
CFUNC3(Translated, glue_d, glTranslated, "fff")
CFUNC3(Translatef, glue_f, glTranslatef, "fff")
CFUNC3(IsList, glue_i_ri, glIsList, "i")
CFUNC3(DeleteLists, glue_i, glDeleteLists, "ii")
CFUNC3(GenLists, glue_i_ri, glGenLists, "i")
CFUNC3(NewList, glue_i, glNewList, "ii")
CFUNC3(EndList, glue_i, glEndList, "")
CFUNC3(CallList, glue_i, glCallList, "i")

FUNC(CallLists)
{
    return not_implemented();
}

CFUNC3(ListBase, glue_i, glListBase, "i")
CFUNC3(Begin, glue_i, glBegin, "i")
CFUNC3(End, glue_i, glEnd, "")

CFUNC3(Vertex2d, glue_d, glVertex2d, "ff")
CFUNC3(Vertex2f, glue_f, glVertex2f, "ff")
CFUNC3(Vertex2i, glue_i, glVertex2i, "ii")
CFUNC3(Vertex2s, glue_i, glVertex2s, "ii")
CFUNC3(Vertex3d, glue_d, glVertex3d, "fff")
CFUNC3(Vertex3f, glue_f, glVertex3f, "fff")
CFUNC3(Vertex3i, glue_i, glVertex3i, "iii")
CFUNC3(Vertex3s, glue_i, glVertex3s, "iii")
CFUNC3(Vertex4d, glue_d, glVertex4d, "ffff")
CFUNC3(Vertex4f, glue_f, glVertex4f, "ffff")
CFUNC3(Vertex4i, glue_i, glVertex4i, "iiii")
CFUNC3(Vertex4s, glue_i, glVertex4s, "iiii")
CFUNC3(Vertex2dv, glue_da, glVertex2dv, "ff")
CFUNC3(Vertex2fv, glue_fa, glVertex2fv, "ff")
CFUNC3(Vertex2iv, glue_ia, glVertex2iv, "ii")
CFUNC3(Vertex2sv, glue_sa, glVertex2sv, "ii")
CFUNC3(Vertex3dv, glue_da, glVertex3dv, "fff")
CFUNC3(Vertex3fv, glue_fa, glVertex3fv, "fff")
CFUNC3(Vertex3iv, glue_ia, glVertex3iv, "iii")
CFUNC3(Vertex3sv, glue_sa, glVertex3sv, "iii")
CFUNC3(Vertex4dv, glue_da, glVertex4dv, "ffff")
CFUNC3(Vertex4fv, glue_fa, glVertex4fv, "ffff")
CFUNC3(Vertex4iv, glue_ia, glVertex4iv, "iiii")
CFUNC3(Vertex4sv, glue_sa, glVertex4sv, "iiii")

FUNC(Vertex)
{
    long	ai, bi, ci, di;
    double	af, bf, cf, df;
    array_t	*a;
    int		i;
    int		n;
    type_t	*t;
    int		ia[4];
    double	da[4];

    switch (NARGS())
    {
    case 1:
	if (ici_typecheck("a", &a))
	    return 1;
	switch (n = a->a_top - a->a_base)
	{
	case 2:
	case 3:
	case 4:
	    t = a->a_base[0]->o_type;
	    if (!isint(a->a_base[0]) && !isfloat(a->a_base[0]))
	    {
		error = "incorrect element type in vertex array";
		return 1;
	    }
	    for (i = 0; i < n; ++i)
	    {
		if (a->a_base[i]->o_type != t)
		{
		    error = "incorrect element type in vertex array";
		    return 1;
		}
		if (isint(a->a_base[i]))
		    ia[i] = intof(a->a_base[i])->i_value;
		else
		    da[i] = floatof(a->a_base[i])->f_value;
	    }
	    switch (n)
	    {
	    case 2:
		if (t == objof(&o_zero)->o_type)
		    glVertex2iv(ia);
		else
		    glVertex2dv(da);
		break;

	    case 3:
		if (t == objof(&o_zero)->o_type)
		    glVertex3iv(ia);
		else
		    glVertex3dv(da);
		break;

	    case 4:
		if (t == objof(&o_zero)->o_type)
		    glVertex4iv(ia);
		else
		    glVertex4dv(da);
		break;
	    }
	    break;

	default:
	    error = "incorrect number of elements in vertex array";
	    return 1;
	}
	break;

    case 2:
	if (ici_typecheck("ii", &ai, &bi) == 0)
	    glVertex2i(ai, bi);
	else if (ici_typecheck("ff", &af, &bf) == 0)
	    glVertex2d(af, bf);
	else
	    return 1;
	break;

    case 3:
	if (ici_typecheck("iii", &ai, &bi, &ci) == 0)
	    glVertex3i(ai, bi, ci);
	else if (ici_typecheck("fff", &af, &bf, &cf) == 0)
	    glVertex3d(af, bf, cf);
	else
	    return 1;
	break;

    case 4:
	if (ici_typecheck("iiii", &ai, &bi, &ci, &di) == 0)
	    glVertex4i(ai, bi, ci, di);
	else if (ici_typecheck("ffff", &af, &bf, &cf, &df) == 0)
	    glVertex4d(af, bf, cf, df);
	else
	    return 1;
	break;

    default:
	return ici_argcount(4);
    }
    return null_ret();
}

CFUNC3(Normal3b, glue_i, glNormal3b, "iii")
CFUNC3(Normal3d, glue_d, glNormal3d, "fff")
CFUNC3(Normal3f, glue_f, glNormal3f, "fff")
CFUNC3(Normal3i, glue_i, glNormal3i, "iii")
CFUNC3(Normal3s, glue_i, glNormal3s, "iii")

FUNC(Normal3bv)
{
    return not_implemented();
}

FUNC(Normal3dv)
{
    return not_implemented();
}

FUNC(Normal3fv)
{
    return not_implemented();
}

FUNC(Normal3iv)
{
    return not_implemented();
}

FUNC(Normal3sv)
{
    return not_implemented();
}

CFUNC3(Indexd, glue_d, glIndexd, "f")
CFUNC3(Indexf, glue_f, glIndexf, "f")
CFUNC3(Indexi, glue_i, glIndexi, "i")
CFUNC3(Indexs, glue_i, glIndexs, "i")
CFUNC3(Indexub, glue_i, glIndexub, "i")

FUNC(Indexdv)
{
    return not_implemented();
}

FUNC(Indexfv)
{
    return not_implemented();
}

FUNC(Indexiv)
{
    return not_implemented();
}

FUNC(Indexsv)
{
    return not_implemented();
}

FUNC(Indexubv)
{
    return not_implemented();
}

CFUNC3(Color3b, glue_i, glColor3b, "iii")
CFUNC3(Color3d, glue_d, glColor3d, "fff")
CFUNC3(Color3f, glue_f, glColor3f, "fff")
CFUNC3(Color3i, glue_i, glColor3i, "iii")
CFUNC3(Color3ub, glue_i, glColor3ub, "iii")
CFUNC3(Color3ui, glue_i, glColor3ui, "iii")
CFUNC3(Color3us, glue_i, glColor3us, "iii")
CFUNC3(Color4b, glue_i, glColor4b, "iiii")
CFUNC3(Color4d, glue_d, glColor4d, "ffff")
CFUNC3(Color4f, glue_f, glColor4f, "ffff")
CFUNC3(Color4i, glue_i, glColor4i, "iiii")
CFUNC3(Color4ub, glue_i, glColor4ub, "iiii")
CFUNC3(Color4ui, glue_i, glColor4ui, "iiii")
CFUNC3(Color4us, glue_i, glColor4us, "iiii")

FUNC(Color3bv)
{
    return not_implemented();
}

FUNC(Color3dv)
{
    return not_implemented();
}

FUNC(Color3fv)
{
    return not_implemented();
}

FUNC(Color3iv)
{
    return not_implemented();
}

FUNC(Color3sv)
{
    return not_implemented();
}

FUNC(Color3ubv)
{
    return not_implemented();
}

FUNC(Color3uiv)
{
    return not_implemented();
}

FUNC(Color3usv)
{
    return not_implemented();
}

FUNC(Color4bv)
{
    return not_implemented();
}

FUNC(Color4dv)
{
    return not_implemented();
}

FUNC(Color4fv)
{
    return not_implemented();
}

FUNC(Color4iv)
{
    return not_implemented();
}

FUNC(Color4sv)
{
    return not_implemented();
}

FUNC(Color4ubv)
{
    return not_implemented();
}

FUNC(Color4uiv)
{
    return not_implemented();
}

FUNC(Color4usv)
{
    return not_implemented();
}

CFUNC3(TexCoord1d, glue_d, glTexCoord1d, "f")
CFUNC3(TexCoord1f, glue_f, glTexCoord1f, "f")
CFUNC3(TexCoord1i, glue_i, glTexCoord1i, "i")
CFUNC3(TexCoord1s, glue_i, glTexCoord1s, "i")
CFUNC3(TexCoord2d, glue_d, glTexCoord2d, "ff")
CFUNC3(TexCoord2f, glue_f, glTexCoord2f, "ff")
CFUNC3(TexCoord2i, glue_i, glTexCoord2i, "ii")
CFUNC3(TexCoord2s, glue_i, glTexCoord2s, "ii")
CFUNC3(TexCoord3d, glue_d, glTexCoord3d, "fff")
CFUNC3(TexCoord3f, glue_f, glTexCoord3f, "fff")
CFUNC3(TexCoord3i, glue_i, glTexCoord3i, "iii")
CFUNC3(TexCoord3s, glue_i, glTexCoord3s, "iii")
CFUNC3(TexCoord4d, glue_d, glTexCoord4d, "ffff")
CFUNC3(TexCoord4f, glue_f, glTexCoord4f, "ffff")
CFUNC3(TexCoord4i, glue_i, glTexCoord4i, "iiii")
CFUNC3(TexCoord4s, glue_i, glTexCoord4s, "iiii")

FUNC(TexCoord1dv)
{
    return not_implemented();
}

FUNC(TexCoord1fv)
{
    return not_implemented();
}

FUNC(TexCoord1iv)
{
    return not_implemented();
}

FUNC(TexCoord1sv)
{
    return not_implemented();
}

FUNC(TexCoord2dv)
{
    return not_implemented();
}

FUNC(TexCoord2fv)
{
    return not_implemented();
}

FUNC(TexCoord2iv)
{
    return not_implemented();
}

FUNC(TexCoord2sv)
{
    return not_implemented();
}

FUNC(TexCoord3dv)
{
    return not_implemented();
}

FUNC(TexCoord3fv)
{
    return not_implemented();
}

FUNC(TexCoord3iv)
{
    return not_implemented();
}

FUNC(TexCoord3sv)
{
    return not_implemented();
}

FUNC(TexCoord4dv)
{
    return not_implemented();
}

FUNC(TexCoord4fv)
{
    return not_implemented();
}

FUNC(TexCoord4iv)
{
    return not_implemented();
}

FUNC(TexCoord4sv)
{
    return not_implemented();
}

CFUNC3(RasterPos2d, glue_d, glRasterPos2d, "ff")
CFUNC3(RasterPos2f, glue_f, glRasterPos2f, "ff")
CFUNC3(RasterPos2i, glue_i, glRasterPos2i, "ii")
CFUNC3(RasterPos2s, glue_i, glRasterPos2s, "ii")
CFUNC3(RasterPos3d, glue_d, glRasterPos3d, "fff")
CFUNC3(RasterPos3f, glue_f, glRasterPos3f, "fff")
CFUNC3(RasterPos3i, glue_i, glRasterPos3i, "iii")
CFUNC3(RasterPos3s, glue_i, glRasterPos3s, "iii")
CFUNC3(RasterPos4d, glue_d, glRasterPos4d, "ffff")
CFUNC3(RasterPos4f, glue_f, glRasterPos4f, "ffff")
CFUNC3(RasterPos4i, glue_i, glRasterPos4i, "iiii")
CFUNC3(RasterPos4s, glue_i, glRasterPos4s, "iiii")

FUNC(RasterPos2dv)
{
    return not_implemented();
}

FUNC(RasterPos2fv)
{
    return not_implemented();
}

FUNC(RasterPos2iv)
{
    return not_implemented();
}

FUNC(RasterPos2sv)
{
    return not_implemented();
}

FUNC(RasterPos3dv)
{
    return not_implemented();
}

FUNC(RasterPos3fv)
{
    return not_implemented();
}

FUNC(RasterPos3iv)
{
    return not_implemented();
}

FUNC(RasterPos3sv)
{
    return not_implemented();
}

FUNC(RasterPos4dv)
{
    return not_implemented();
}

FUNC(RasterPos4fv)
{
    return not_implemented();
}

FUNC(RasterPos4iv)
{
    return not_implemented();
}

FUNC(RasterPos4sv)
{
    return not_implemented();
}

CFUNC3(Rectd, glue_d, glRectd, "ffff")
CFUNC3(Rectf, glue_f, glRectf, "ffff")
CFUNC3(Recti, glue_i, glRecti, "iiii")
CFUNC3(Rects, glue_i, glRects, "iiii")

FUNC(Rectdv)
{
    return not_implemented();
}

FUNC(Rectfv)
{
    return not_implemented();
}

FUNC(Rectiv)
{
    return not_implemented();
}

FUNC(Rectsv)
{
    return not_implemented();
}

FUNC(VertexPointer)
{
    return not_implemented();
}

FUNC(NormalPointer)
{
    return not_implemented();
}

FUNC(ColorPointer)
{
    return not_implemented();
}

FUNC(IndexPointer)
{
    return not_implemented();
}

FUNC(TextCoordPointer)
{
    return not_implemented();
}

FUNC(EdgeFlagPointer)
{
    return not_implemented();
}

FUNC(GetPointerv)
{
    return not_implemented();
}

FUNC(ArrayElement)
{
    return not_implemented();
}

FUNC(DrawArrays)
{
    return not_implemented();
}

FUNC(DrawElements)
{
    return not_implemented();
}

FUNC(InterleavedArrays)
{
    return not_implemented();
}

CFUNC3(ShadeModel, glue_i, glShadeModel, "i")

FUNC(Lightf)
{
    long	light, pname;
    double	param;

    if (ici_typecheck("iif", &light, &pname, &param))
	return 1;
    glLightf(light, pname, param);
    return null_ret();
}

CFUNC3(Lighti, glue_i, glLighti, "iii")

FUNC(Lightfv)
{
    return not_implemented();
}

FUNC(Lightiv)
{
    return not_implemented();
}

FUNC(GetLightfv)
{
    return not_implemented();
}

FUNC(GetLightiv)
{
    return not_implemented();
}

CFUNC1(LightModelf, glue_1i1f, glLightModelf)
CFUNC3(LightModeli, glue_i, glLightModeli, "ii")

FUNC(LightModelfv)
{
    return not_implemented();
}

FUNC(LightModeliv)
{
    return not_implemented();
}

FUNC(Materialf)
{
    long	face, pname;
    double	param;

    if (ici_typecheck("iif", &face, &pname, &param))
	return 1;
    glMaterialf(face, pname, param);
    return null_ret();
}

CFUNC3(Materiali, glue_i, glMateriali, "iii")

FUNC(Materialfv)
{
    return not_implemented();
}

FUNC(Materialiv)
{
    return not_implemented();
}

FUNC(GetMaterialfv)
{
    return not_implemented();
}

FUNC(GetMaterialiv)
{
    return not_implemented();
}

CFUNC3(ColorMaterial, glue_i, glColorMaterial, "ii")
CFUNC3(PixelZoom, glue_f, glPixelZoom, "ff")

CFUNC1(PixelStoref, glue_1i1f, glPixelStoref)
CFUNC3(PixelStorei, glue_i, glPixelStorei, "ii")
CFUNC1(PixelTransferf, glue_1i1f, glPixelTransferf)
CFUNC3(PixelTransferi, glue_i, glPixelTransferi, "ii")

FUNC(PixelMapfv)
{
    return not_implemented();
}

FUNC(PixelMapuiv)
{
    return not_implemented();
}

FUNC(PixelMapusv)
{
    return not_implemented();
}

FUNC(GetPixelMapfv)
{
    return not_implemented();
}

FUNC(GetPixelMapuiv)
{
    return not_implemented();
}

FUNC(GetPixelMapusv)
{
    return not_implemented();
}

FUNC(Bitmap)
{
    return not_implemented();
}

FUNC(ReadPixles)
{
    return not_implemented();
}

FUNC(DrawPixles)
{
    return not_implemented();
}

CFUNC3(CopyPixels, glue_i, glCopyPixels, "iiiii")
CFUNC3(StencilFunc, glue_i, glStencilFunc, "iii")
CFUNC3(StencilMask, glue_i, glStencilMask, "i")
CFUNC3(StencilOp, glue_i, glStencilOp, "iii")
CFUNC3(ClearStencil, glue_i, glClearStencil, "i")

FUNC(TexGend)
{
    return not_implemented();
}

FUNC(TexGenf)
{
    return not_implemented();
}

CFUNC3(TexGeni, glue_i, glTexGeni, "iii")

FUNC(TexGendv)
{
    return not_implemented();
}

FUNC(TexGenfv)
{
    return not_implemented();
}

FUNC(TexGeniv)
{
    return not_implemented();
}

FUNC(GetTexGendv)
{
    return not_implemented();
}

FUNC(GetTexGenfv)
{
    return not_implemented();
}

FUNC(GetTexGeniv)
{
    return not_implemented();
}

FUNC(TexEnvf)
{
    return not_implemented();
}

CFUNC3(TexEnvi, glue_i, glTexEnvi, "iii")

FUNC(TexEnvfv)
{
    return not_implemented();
}

FUNC(TexEnviv)
{
    return not_implemented();
}

FUNC(GetTexEnvfv)
{
    return not_implemented();
}

FUNC(GetTexEnviv)
{
    return not_implemented();
}

CFUNC1(TexParameterf, glue_2i1f, glTexParameterf)
CFUNC3(TexParameteri, glue_i, glTexParameteri, "iii")

FUNC(TexParameterfv)
{
    return not_implemented();
}

FUNC(TexParameteriv)
{
    return not_implemented();
}

FUNC(GetTexParameterfv)
{
    return not_implemented();
}

FUNC(GetTexParameteriv)
{
    return not_implemented();
}

FUNC(TexImage1D)
{
    return not_implemented();
}

FUNC(TexImage2D)
{
    return not_implemented();
}

FUNC(GetTexImage)
{
    return not_implemented();
}

FUNC(GenTextures)
{
    return not_implemented();
}

FUNC(DeleteTextures)
{
    return not_implemented();
}

CFUNC3(BindTexture, glue_i, glBindTexture, "ii")

FUNC(PrioritizeTextures)
{
    return not_implemented();
}

FUNC(AreTexturesResident)
{
    return not_implemented();
}

FUNC(IsTexture)
{
    long	texture;

    if (ici_typecheck("i", &texture))
	return 1;
    return int_ret(glIsTexture(texture));
}

FUNC(TexSubImage1D)
{
    return not_implemented();
}

FUNC(TexSubImage2D)
{
    return not_implemented();
}

FUNC(CopyTexImage1D)
{
    return not_implemented();
}

FUNC(CopyTexImage2D)
{
    return not_implemented();
}

FUNC(CopyTexSubImage1D)
{
    return not_implemented();
}

FUNC(CopyTexSubImage2D)
{
    return not_implemented();
}

FUNC(Map1d)
{
    return not_implemented();
}

FUNC(Map1f)
{
    return not_implemented();
}

FUNC(Map2d)
{
    return not_implemented();
}

FUNC(Map2f)
{
    return not_implemented();
}

FUNC(GetMapdv)
{
    return not_implemented();
}

FUNC(GetMapfv)
{
    return not_implemented();
}

FUNC(GetMapiv)
{
    return not_implemented();
}

CFUNC3(EvalCoord1d, glue_d, glEvalCoord1d, "f")
CFUNC3(EvalCoord1f, glue_f, glEvalCoord1f, "f")

FUNC(EvalCoord1dv)
{
    return not_implemented();
}

FUNC(EvalCoord1fv)
{
    return not_implemented();
}

CFUNC3(EvalCoord2d, glue_d, glEvalCoord1d, "ff")
CFUNC3(EvalCoord2f, glue_f, glEvalCoord1f, "ff")

FUNC(EvalCoord2dv)
{
    return not_implemented();
}

FUNC(EvalCoord2fv)
{
    return not_implemented();
}

FUNC(MapGrid1d)
{
    return not_implemented();
}

FUNC(MapGrid1f)
{
    return not_implemented();
}

FUNC(MapGrid2d)
{
    return not_implemented();
}

FUNC(MapGrid2f)
{
    return not_implemented();
}

CFUNC3(EvalPoint1, glue_i, glEvalPoint1, "i")
CFUNC3(EvalPoint2, glue_i, glEvalPoint2, "ii")
CFUNC3(EvalMesh1, glue_i, glEvalMesh1, "iii")
CFUNC3(EvalMesh2, glue_i, glEvalMesh2, "iiii")

CFUNC1(Fogf, glue_1i1f, glFogf)
CFUNC3(Fogi, glue_i, glFogi, "ii")

FUNC(Fogfv)
{
    return not_implemented();
}

FUNC(Fogiv)
{
    return not_implemented();
}

FUNC(FeedbackBuffer)
{
    return not_implemented();
}

CFUNC3(PassThrough, glue_f, glPassThrough, "f")

FUNC(SelectBuffer)
{
    return not_implemented();
}

CFUNC3(InitNames, glue_i, glInitNames, "")
CFUNC3(LoadName, glue_i, glLoadName, "i")
CFUNC3(PushName, glue_i, glPushName, "i")
CFUNC3(PopName, glue_i, glPopName, "")

/*
 * And a bucket load of "constants" defined in gl.h
 */

static struct var
{
    char	*name;
    long	value;
}
vars[] =
{
    /* Boolean values */
    {"FALSE", 0},
    {"TRUE", 1},

    /* Data types */
    {"BYTE", 0x1400},
    {"UNSIGNED_BYTE", 0x1401},
    {"SHORT", 0x1402},
    {"UNSIGNED_SHORT", 0x1403},
    {"INT", 0x1404},
    {"UNSIGNED_INT", 0x1405},
    {"FLOAT", 0x1406},
    {"DOUBLE", 0x140A},
    {"_2_BYTES", 0x1407},
    {"_3_BYTES", 0x1408},
    {"_4_BYTES", 0x1409},

    /* Primitives */
    {"LINES", 0x0001},
    {"POINTS", 0x0000},
    {"LINE_STRIP", 0x0003},
    {"LINE_LOOP", 0x0002},
    {"TRIANGLES", 0x0004},
    {"TRIANGLE_STRIP", 0x0005},
    {"TRIANGLE_FAN", 0x0006},
    {"QUADS", 0x0007},
    {"QUAD_STRIP", 0x0008},
    {"POLYGON", 0x0009},
    {"EDGE_FLAG", 0x0B43},

    /* Vertex Arrays */
    {"VERTEX_ARRAY", 0x8074},
    {"NORMAL_ARRAY", 0x8075},
    {"COLOR_ARRAY", 0x8076},
    {"INDEX_ARRAY", 0x8077},
    {"TEXTURE_COORD_ARRAY", 0x8078},
    {"EDGE_FLAG_ARRAY", 0x8079},
    {"VERTEX_ARRAY_SIZE", 0x807A},
    {"VERTEX_ARRAY_TYPE", 0x807B},
    {"VERTEX_ARRAY_STRIDE", 0x807C},
    {"NORMAL_ARRAY_TYPE", 0x807E},
    {"NORMAL_ARRAY_STRIDE", 0x807F},
    {"COLOR_ARRAY_SIZE", 0x8081},
    {"COLOR_ARRAY_TYPE", 0x8082},
    {"COLOR_ARRAY_STRIDE", 0x8083},
    {"INDEX_ARRAY_TYPE", 0x8085},
    {"INDEX_ARRAY_STRIDE", 0x8086},
    {"TEXTURE_COORD_ARRAY_SIZE", 0x8088},
    {"TEXTURE_COORD_ARRAY_TYPE", 0x8089},
    {"TEXTURE_COORD_ARRAY_STRIDE", 0x808A},
    {"EDGE_FLAG_ARRAY_STRIDE", 0x808C},
    {"VERTEX_ARRAY_POINTER", 0x808E},
    {"NORMAL_ARRAY_POINTER", 0x808F},
    {"COLOR_ARRAY_POINTER", 0x8090},
    {"INDEX_ARRAY_POINTER", 0x8091},
    {"TEXTURE_COORD_ARRAY_POINTER", 0x8092},
    {"EDGE_FLAG_ARRAY_POINTER", 0x8093},
    {"V2F", 0x2A20},
    {"V3F", 0x2A21},
    {"C4UB_V2F", 0x2A22},
    {"C4UB_V3F", 0x2A23},
    {"C3F_V3F", 0x2A24},
    {"N3F_V3F", 0x2A25},
    {"C4F_N3F_V3F", 0x2A26},
    {"T2F_V3F", 0x2A27},
    {"T4F_V4F", 0x2A28},
    {"T2F_C4UB_V3F", 0x2A29},
    {"T2F_C3F_V3F", 0x2A2A},
    {"T2F_N3F_V3F", 0x2A2B},
    {"T2F_C4F_N3F_V3F", 0x2A2C},
    {"T4F_C4F_N3F_V4F", 0x2A2D},

    /* Matrix Mode */
    {"MATRIX_MODE", 0x0BA0},
    {"MODELVIEW", 0x1700},
    {"PROJECTION", 0x1701},
    {"TEXTURE", 0x1702},

    /* Points */
    {"POINT_SMOOTH", 0x0B10},
    {"POINT_SIZE", 0x0B11},
    {"POINT_SIZE_GRANULARITY", 0x0B13},
    {"POINT_SIZE_RANGE", 0x0B12},

    /* Lines */
    {"LINE_SMOOTH", 0x0B20},
    {"LINE_STIPPLE", 0x0B24},
    {"LINE_STIPPLE_PATTERN", 0x0B25},
    {"LINE_STIPPLE_REPEAT", 0x0B26},
    {"LINE_WIDTH", 0x0B21},
    {"LINE_WIDTH_GRANULARITY", 0x0B23},
    {"LINE_WIDTH_RANGE", 0x0B22},

    /* Polygons */
    {"POINT", 0x1B00},
    {"LINE", 0x1B01},
    {"FILL", 0x1B02},
    {"CCW", 0x0901},
    {"CW", 0x0900},
    {"FRONT", 0x0404},
    {"BACK", 0x0405},
    {"CULL_FACE", 0x0B44},
    {"CULL_FACE_MODE", 0x0B45},
    {"POLYGON_SMOOTH", 0x0B41},
    {"POLYGON_STIPPLE", 0x0B42},
    {"FRONT_FACE", 0x0B46},
    {"POLYGON_MODE", 0x0B40},
    {"POLYGON_OFFSET_FACTOR", 0x8038},
    {"POLYGON_OFFSET_UNITS", 0x2A00},
    {"POLYGON_OFFSET_POINT", 0x2A01},
    {"POLYGON_OFFSET_LINE", 0x2A02},
    {"POLYGON_OFFSET_FILL", 0x8037},

    /* Display Lists */
    {"COMPILE", 0x1300},
    {"COMPILE_AND_EXECUTE", 0x1301},
    {"LIST_BASE", 0x0B32},
    {"LIST_INDEX", 0x0B33},
    {"LIST_MODE", 0x0B30},

    /* Depth buffer */
    {"NEVER", 0x0200},
    {"LESS", 0x0201},
    {"GEQUAL", 0x0206},
    {"LEQUAL", 0x0203},
    {"GREATER", 0x0204},
    {"NOTEQUAL", 0x0205},
    {"EQUAL", 0x0202},
    {"ALWAYS", 0x0207},
    {"DEPTH_TEST", 0x0B71},
    {"DEPTH_BITS", 0x0D56},
    {"DEPTH_CLEAR_VALUE", 0x0B73},
    {"DEPTH_FUNC", 0x0B74},
    {"DEPTH_RANGE", 0x0B70},
    {"DEPTH_WRITEMASK", 0x0B72},
    {"DEPTH_COMPONENT", 0x1902},

    /* Lighting */
    {"LIGHTING", 0x0B50},
    {"LIGHT0", 0x4000},
    {"LIGHT1", 0x4001},
    {"LIGHT2", 0x4002},
    {"LIGHT3", 0x4003},
    {"LIGHT4", 0x4004},
    {"LIGHT5", 0x4005},
    {"LIGHT6", 0x4006},
    {"LIGHT7", 0x4007},
    {"SPOT_EXPONENT", 0x1205},
    {"SPOT_CUTOFF", 0x1206},
    {"CONSTANT_ATTENUATION", 0x1207},
    {"LINEAR_ATTENUATION", 0x1208},
    {"QUADRATIC_ATTENUATION", 0x1209},
    {"AMBIENT", 0x1200},
    {"DIFFUSE", 0x1201},
    {"SPECULAR", 0x1202},
    {"SHININESS", 0x1601},
    {"EMISSION", 0x1600},
    {"POSITION", 0x1203},
    {"SPOT_DIRECTION", 0x1204},
    {"AMBIENT_AND_DIFFUSE", 0x1602},
    {"COLOR_INDEXES", 0x1603},
    {"LIGHT_MODEL_TWO_SIDE", 0x0B52},
    {"LIGHT_MODEL_LOCAL_VIEWER", 0x0B51},
    {"LIGHT_MODEL_AMBIENT", 0x0B53},
    {"FRONT_AND_BACK", 0x0408},
    {"SHADE_MODEL", 0x0B54},
    {"FLAT", 0x1D00},
    {"SMOOTH", 0x1D01},
    {"COLOR_MATERIAL", 0x0B57},
    {"COLOR_MATERIAL_FACE", 0x0B55},
    {"COLOR_MATERIAL_PARAMETER", 0x0B56},
    {"NORMALIZE", 0x0BA1},

    /* User clipping planes */
    {"CLIP_PLANE0", 0x3000},
    {"CLIP_PLANE1", 0x3001},
    {"CLIP_PLANE2", 0x3002},
    {"CLIP_PLANE3", 0x3003},
    {"CLIP_PLANE4", 0x3004},
    {"CLIP_PLANE5", 0x3005},

    /* Accumulation buffer */
    {"ACCUM_RED_BITS", 0x0D58},
    {"ACCUM_GREEN_BITS", 0x0D59},
    {"ACCUM_BLUE_BITS", 0x0D5A},
    {"ACCUM_ALPHA_BITS", 0x0D5B},
    {"ACCUM_CLEAR_VALUE", 0x0B80},
    {"ACCUM", 0x0100},
    {"ADD", 0x0104},
    {"LOAD", 0x0101},
    {"MULT", 0x0103},
    {"RETURN", 0x0102},

    /* Alpha testing */
    {"ALPHA_TEST", 0x0BC0},
    {"ALPHA_TEST_REF", 0x0BC2},
    {"ALPHA_TEST_FUNC", 0x0BC1},

    /* Blending */
    {"BLEND", 0x0BE2},
    {"BLEND_SRC", 0x0BE1},
    {"BLEND_DST", 0x0BE0},
    {"ZERO", 0},
    {"ONE", 1},
    {"SRC_COLOR", 0x0300},
    {"ONE_MINUS_SRC_COLOR", 0x0301},
    {"DST_COLOR", 0x0306},
    {"ONE_MINUS_DST_COLOR", 0x0307},
    {"SRC_ALPHA", 0x0302},
    {"ONE_MINUS_SRC_ALPHA", 0x0303},
    {"DST_ALPHA", 0x0304},
    {"ONE_MINUS_DST_ALPHA", 0x0305},
    {"SRC_ALPHA_SATURATE", 0x0308},
    {"CONSTANT_COLOR", 0x8001},
    {"ONE_MINUS_CONSTANT_COLOR", 0x8002},
    {"CONSTANT_ALPHA", 0x8003},
    {"ONE_MINUS_CONSTANT_ALPHA", 0x8004},

    /* Render Mode */
    {"FEEDBACK", 0x1C01},
    {"RENDER", 0x1C00},
    {"SELECT", 0x1C02},

    /* Feedback */
    {"_2D", 0x0600},
    {"_3D", 0x0601},
    {"_3D_COLOR", 0x0602},
    {"_3D_COLOR_TEXTURE", 0x0603},
    {"_4D_COLOR_TEXTURE", 0x0604},
    {"POINT_TOKEN", 0x0701},
    {"LINE_TOKEN", 0x0702},
    {"LINE_RESET_TOKEN", 0x0707},
    {"POLYGON_TOKEN", 0x0703},
    {"BITMAP_TOKEN", 0x0704},
    {"DRAW_PIXEL_TOKEN", 0x0705},
    {"COPY_PIXEL_TOKEN", 0x0706},
    {"PASS_THROUGH_TOKEN", 0x0700},
    {"FEEDBACK_BUFFER_POINTER", 0x0DF0},
    {"FEEDBACK_BUFFER_SIZE", 0x0DF1},
    {"FEEDBACK_BUFFER_TYPE", 0x0DF2},

    /* Selection */
    {"SELECTION_BUFFER_POINTER", 0x0DF3},
    {"SELECTION_BUFFER_SIZE", 0x0DF4},

    /* Fog */
    {"FOG", 0x0B60},
    {"FOG_MODE", 0x0B65},
    {"FOG_DENSITY", 0x0B62},
    {"FOG_COLOR", 0x0B66},
    {"FOG_INDEX", 0x0B61},
    {"FOG_START", 0x0B63},
    {"FOG_END", 0x0B64},
    {"LINEAR", 0x2601},
    {"EXP", 0x0800},
    {"EXP2", 0x0801},

    /* Logic Ops */
    {"LOGIC_OP", 0x0BF1},
    {"INDEX_LOGIC_OP", 0x0BF1},
    {"COLOR_LOGIC_OP", 0x0BF2},
    {"LOGIC_OP_MODE", 0x0BF0},
    {"CLEAR", 0x1500},
    {"SET", 0x150F},
    {"COPY", 0x1503},
    {"COPY_INVERTED", 0x150C},
    {"NOOP", 0x1505},
    {"INVERT", 0x150A},
    {"AND", 0x1501},
    {"NAND", 0x150E},
    {"OR", 0x1507},
    {"NOR", 0x1508},
    {"XOR", 0x1506},
    {"EQUIV", 0x1509},
    {"AND_REVERSE", 0x1502},
    {"AND_INVERTED", 0x1504},
    {"OR_REVERSE", 0x150B},
    {"OR_INVERTED", 0x150D},

    /* Stencil */
    {"STENCIL_TEST", 0x0B90},
    {"STENCIL_WRITEMASK", 0x0B98},
    {"STENCIL_BITS", 0x0D57},
    {"STENCIL_FUNC", 0x0B92},
    {"STENCIL_VALUE_MASK", 0x0B93},
    {"STENCIL_REF", 0x0B97},
    {"STENCIL_FAIL", 0x0B94},
    {"STENCIL_PASS_DEPTH_PASS", 0x0B96},
    {"STENCIL_PASS_DEPTH_FAIL", 0x0B95},
    {"STENCIL_CLEAR_VALUE", 0x0B91},
    {"STENCIL_INDEX", 0x1901},
    {"KEEP", 0x1E00},
    {"REPLACE", 0x1E01},
    {"INCR", 0x1E02},
    {"DECR", 0x1E03},

    /* Buffers, Pixel Drawing/Reading */
    {"NONE", 0},
    {"LEFT", 0x0406},
    {"RIGHT", 0x0407},
    /*GL_FRONT			= 0x0404, */
    /*GL_BACK			= 0x0405, */
    /*GL_FRONT_AND_BACK		= 0x0408, */
    {"FRONT_LEFT", 0x0400},
    {"FRONT_RIGHT", 0x0401},
    {"BACK_LEFT", 0x0402},
    {"BACK_RIGHT", 0x0403},
    {"AUX0", 0x0409},
    {"AUX1", 0x040A},
    {"AUX2", 0x040B},
    {"AUX3", 0x040C},
    {"COLOR_INDEX", 0x1900},
    {"RED", 0x1903},
    {"GREEN", 0x1904},
    {"BLUE", 0x1905},
    {"ALPHA", 0x1906},
    {"LUMINANCE", 0x1909},
    {"LUMINANCE_ALPHA", 0x190A},
    {"ALPHA_BITS", 0x0D55},
    {"RED_BITS", 0x0D52},
    {"GREEN_BITS", 0x0D53},
    {"BLUE_BITS", 0x0D54},
    {"INDEX_BITS", 0x0D51},
    {"SUBPIXEL_BITS", 0x0D50},
    {"AUX_BUFFERS", 0x0C00},
    {"READ_BUFFER", 0x0C02},
    {"DRAW_BUFFER", 0x0C01},
    {"DOUBLEBUFFER", 0x0C32},
    {"STEREO", 0x0C33},
    {"BITMAP", 0x1A00},
    {"COLOR", 0x1800},
    {"DEPTH", 0x1801},
    {"STENCIL", 0x1802},
    {"DITHER", 0x0BD0},
    {"RGB", 0x1907},
    {"RGBA", 0x1908},

    /* Implementation limits */
    {"MAX_LIST_NESTING", 0x0B31},
    {"MAX_ATTRIB_STACK_DEPTH", 0x0D35},
    {"MAX_MODELVIEW_STACK_DEPTH", 0x0D36},
    {"MAX_NAME_STACK_DEPTH", 0x0D37},
    {"MAX_PROJECTION_STACK_DEPTH", 0x0D38},
    {"MAX_TEXTURE_STACK_DEPTH", 0x0D39},
    {"MAX_EVAL_ORDER", 0x0D30},
    {"MAX_LIGHTS", 0x0D31},
    {"MAX_CLIP_PLANES", 0x0D32},
    {"MAX_TEXTURE_SIZE", 0x0D33},
    {"MAX_PIXEL_MAP_TABLE", 0x0D34},
    {"MAX_VIEWPORT_DIMS", 0x0D3A},
    {"MAX_CLIENT_ATTRIB_STACK_DEPTH", 0x0D3B},

    /* Gets */
    {"ATTRIB_STACK_DEPTH", 0x0BB0},
    {"CLIENT_ATTRIB_STACK_DEPTH", 0x0BB1},
    {"COLOR_CLEAR_VALUE", 0x0C22},
    {"COLOR_WRITEMASK", 0x0C23},
    {"CURRENT_INDEX", 0x0B01},
    {"CURRENT_COLOR", 0x0B00},
    {"CURRENT_NORMAL", 0x0B02},
    {"CURRENT_RASTER_COLOR", 0x0B04},
    {"CURRENT_RASTER_DISTANCE", 0x0B09},
    {"CURRENT_RASTER_INDEX", 0x0B05},
    {"CURRENT_RASTER_POSITION", 0x0B07},
    {"CURRENT_RASTER_TEXTURE_COORDS", 0x0B06},
    {"CURRENT_RASTER_POSITION_VALID", 0x0B08},
    {"CURRENT_TEXTURE_COORDS", 0x0B03},
    {"INDEX_CLEAR_VALUE", 0x0C20},
    {"INDEX_MODE", 0x0C30},
    {"INDEX_WRITEMASK", 0x0C21},
    {"MODELVIEW_MATRIX", 0x0BA6},
    {"MODELVIEW_STACK_DEPTH", 0x0BA3},
    {"NAME_STACK_DEPTH", 0x0D70},
    {"PROJECTION_MATRIX", 0x0BA7},
    {"PROJECTION_STACK_DEPTH", 0x0BA4},
    {"RENDER_MODE", 0x0C40},
    {"RGBA_MODE", 0x0C31},
    {"TEXTURE_MATRIX", 0x0BA8},
    {"TEXTURE_STACK_DEPTH", 0x0BA5},
    {"VIEWPORT", 0x0BA2},


    /* Evaluators */
    {"AUTO_NORMAL", 0x0D80},
    {"MAP1_COLOR_4", 0x0D90},
    {"MAP1_GRID_DOMAIN", 0x0DD0},
    {"MAP1_GRID_SEGMENTS", 0x0DD1},
    {"MAP1_INDEX", 0x0D91},
    {"MAP1_NORMAL", 0x0D92},
    {"MAP1_TEXTURE_COORD_1", 0x0D93},
    {"MAP1_TEXTURE_COORD_2", 0x0D94},
    {"MAP1_TEXTURE_COORD_3", 0x0D95},
    {"MAP1_TEXTURE_COORD_4", 0x0D96},
    {"MAP1_VERTEX_3", 0x0D97},
    {"MAP1_VERTEX_4", 0x0D98},
    {"MAP2_COLOR_4", 0x0DB0},
    {"MAP2_GRID_DOMAIN", 0x0DD2},
    {"MAP2_GRID_SEGMENTS", 0x0DD3},
    {"MAP2_INDEX", 0x0DB1},
    {"MAP2_NORMAL", 0x0DB2},
    {"MAP2_TEXTURE_COORD_1", 0x0DB3},
    {"MAP2_TEXTURE_COORD_2", 0x0DB4},
    {"MAP2_TEXTURE_COORD_3", 0x0DB5},
    {"MAP2_TEXTURE_COORD_4", 0x0DB6},
    {"MAP2_VERTEX_3", 0x0DB7},
    {"MAP2_VERTEX_4", 0x0DB8},
    {"COEFF", 0x0A00},
    {"DOMAIN", 0x0A02},
    {"ORDER", 0x0A01},

    /* Hints */
    {"FOG_HINT", 0x0C54},
    {"LINE_SMOOTH_HINT", 0x0C52},
    {"PERSPECTIVE_CORRECTION_HINT", 0x0C50},
    {"POINT_SMOOTH_HINT", 0x0C51},
    {"POLYGON_SMOOTH_HINT", 0x0C53},
    {"DONT_CARE", 0x1100},
    {"FASTEST", 0x1101},
    {"NICEST", 0x1102},

    /* Scissor box */
    {"SCISSOR_TEST", 0x0C11},
    {"SCISSOR_BOX", 0x0C10},

    /* Pixel Mode / Transfer */
    {"MAP_COLOR", 0x0D10},
    {"MAP_STENCIL", 0x0D11},
    {"INDEX_SHIFT", 0x0D12},
    {"INDEX_OFFSET", 0x0D13},
    {"RED_SCALE", 0x0D14},
    {"RED_BIAS", 0x0D15},
    {"GREEN_SCALE", 0x0D18},
    {"GREEN_BIAS", 0x0D19},
    {"BLUE_SCALE", 0x0D1A},
    {"BLUE_BIAS", 0x0D1B},
    {"ALPHA_SCALE", 0x0D1C},
    {"ALPHA_BIAS", 0x0D1D},
    {"DEPTH_SCALE", 0x0D1E},
    {"DEPTH_BIAS", 0x0D1F},
    {"PIXEL_MAP_S_TO_S_SIZE", 0x0CB1},
    {"PIXEL_MAP_I_TO_I_SIZE", 0x0CB0},
    {"PIXEL_MAP_I_TO_R_SIZE", 0x0CB2},
    {"PIXEL_MAP_I_TO_G_SIZE", 0x0CB3},
    {"PIXEL_MAP_I_TO_B_SIZE", 0x0CB4},
    {"PIXEL_MAP_I_TO_A_SIZE", 0x0CB5},
    {"PIXEL_MAP_R_TO_R_SIZE", 0x0CB6},
    {"PIXEL_MAP_G_TO_G_SIZE", 0x0CB7},
    {"PIXEL_MAP_B_TO_B_SIZE", 0x0CB8},
    {"PIXEL_MAP_A_TO_A_SIZE", 0x0CB9},
    {"PIXEL_MAP_S_TO_S", 0x0C71},
    {"PIXEL_MAP_I_TO_I", 0x0C70},
    {"PIXEL_MAP_I_TO_R", 0x0C72},
    {"PIXEL_MAP_I_TO_G", 0x0C73},
    {"PIXEL_MAP_I_TO_B", 0x0C74},
    {"PIXEL_MAP_I_TO_A", 0x0C75},
    {"PIXEL_MAP_R_TO_R", 0x0C76},
    {"PIXEL_MAP_G_TO_G", 0x0C77},
    {"PIXEL_MAP_B_TO_B", 0x0C78},
    {"PIXEL_MAP_A_TO_A", 0x0C79},
    {"PACK_ALIGNMENT", 0x0D05},
    {"PACK_LSB_FIRST", 0x0D01},
    {"PACK_ROW_LENGTH", 0x0D02},
    {"PACK_SKIP_PIXELS", 0x0D04},
    {"PACK_SKIP_ROWS", 0x0D03},
    {"PACK_SWAP_BYTES", 0x0D00},
    {"UNPACK_ALIGNMENT", 0x0CF5},
    {"UNPACK_LSB_FIRST", 0x0CF1},
    {"UNPACK_ROW_LENGTH", 0x0CF2},
    {"UNPACK_SKIP_PIXELS", 0x0CF4},
    {"UNPACK_SKIP_ROWS", 0x0CF3},
    {"UNPACK_SWAP_BYTES", 0x0CF0},
    {"ZOOM_X", 0x0D16},
    {"ZOOM_Y", 0x0D17},

    /* Texture mapping */
    {"TEXTURE_ENV", 0x2300},
    {"TEXTURE_ENV_MODE", 0x2200},
    {"TEXTURE_1D", 0x0DE0},
    {"TEXTURE_2D", 0x0DE1},
    {"TEXTURE_WRAP_S", 0x2802},
    {"TEXTURE_WRAP_T", 0x2803},
    {"TEXTURE_MAG_FILTER", 0x2800},
    {"TEXTURE_MIN_FILTER", 0x2801},
    {"TEXTURE_ENV_COLOR", 0x2201},
    {"TEXTURE_GEN_S", 0x0C60},
    {"TEXTURE_GEN_T", 0x0C61},
    {"TEXTURE_GEN_MODE", 0x2500},
    {"TEXTURE_BORDER_COLOR", 0x1004},
    {"TEXTURE_WIDTH", 0x1000},
    {"TEXTURE_HEIGHT", 0x1001},
    {"TEXTURE_BORDER", 0x1005},
    {"TEXTURE_COMPONENTS", 0x1003},
    {"TEXTURE_RED_SIZE", 0x805C},
    {"TEXTURE_GREEN_SIZE", 0x805D},
    {"TEXTURE_BLUE_SIZE", 0x805E},
    {"TEXTURE_ALPHA_SIZE", 0x805F},
    {"TEXTURE_LUMINANCE_SIZE", 0x8060},
    {"TEXTURE_INTENSITY_SIZE", 0x8061},
    {"NEAREST_MIPMAP_NEAREST", 0x2700},
    {"NEAREST_MIPMAP_LINEAR", 0x2702},
    {"LINEAR_MIPMAP_NEAREST", 0x2701},
    {"LINEAR_MIPMAP_LINEAR", 0x2703},
    {"OBJECT_LINEAR", 0x2401},
    {"OBJECT_PLANE", 0x2501},
    {"EYE_LINEAR", 0x2400},
    {"EYE_PLANE", 0x2502},
    {"SPHERE_MAP", 0x2402},
    {"DECAL", 0x2101},
    {"MODULATE", 0x2100},
    {"NEAREST", 0x2600},
    {"REPEAT", 0x2901},
    {"CLAMP", 0x2900},
    {"S", 0x2000},
    {"T", 0x2001},
    {"R", 0x2002},
    {"Q", 0x2003},
    {"TEXTURE_GEN_R", 0x0C62},
    {"TEXTURE_GEN_Q", 0x0C63},

    /* GL 1.1 texturing */
    {"PROXY_TEXTURE_1D", 0x8063},
    {"PROXY_TEXTURE_2D", 0x8064},
    {"TEXTURE_PRIORITY", 0x8066},
    {"TEXTURE_RESIDENT", 0x8067},
    {"TEXTURE_BINDING_1D", 0x8068},
    {"TEXTURE_BINDING_2D", 0x8069},
    {"TEXTURE_INTERNAL_FORMAT", 0x1003},

    /* GL 1.2 texturing */
    {"PACK_SKIP_IMAGES", 0x806B},
    {"PACK_IMAGE_HEIGHT", 0x806C},
    {"UNPACK_SKIP_IMAGES", 0x806D},
    {"UNPACK_IMAGE_HEIGHT", 0x806E},
    {"TEXTURE_3D", 0x806F},
    {"PROXY_TEXTURE_3D", 0x8070},
    {"TEXTURE_DEPTH", 0x8071},
    {"TEXTURE_WRAP_R", 0x8072},
    {"MAX_3D_TEXTURE_SIZE", 0x8073},
    {"TEXTURE_BINDING_3D", 0x806A},

    /* Internal texture formats (GL 1.1) */
    {"ALPHA4", 0x803B},
    {"ALPHA8", 0x803C},
    {"ALPHA12", 0x803D},
    {"ALPHA16", 0x803E},
    {"LUMINANCE4", 0x803F},
    {"LUMINANCE8", 0x8040},
    {"LUMINANCE12", 0x8041},
    {"LUMINANCE16", 0x8042},
    {"LUMINANCE4_ALPHA4", 0x8043},
    {"LUMINANCE6_ALPHA2", 0x8044},
    {"LUMINANCE8_ALPHA8", 0x8045},
    {"LUMINANCE12_ALPHA4", 0x8046},
    {"LUMINANCE12_ALPHA12", 0x8047},
    {"LUMINANCE16_ALPHA16", 0x8048},
    {"INTENSITY", 0x8049},
    {"INTENSITY4", 0x804A},
    {"INTENSITY8", 0x804B},
    {"INTENSITY12", 0x804C},
    {"INTENSITY16", 0x804D},
    {"R3_G3_B2", 0x2A10},
    {"RGB4", 0x804F},
    {"RGB5", 0x8050},
    {"RGB8", 0x8051},
    {"RGB10", 0x8052},
    {"RGB12", 0x8053},
    {"RGB16", 0x8054},
    {"RGBA2", 0x8055},
    {"RGBA4", 0x8056},
    {"RGB5_A1", 0x8057},
    {"RGBA8", 0x8058},
    {"RGB10_A2", 0x8059},
    {"RGBA12", 0x805A},
    {"RGBA16", 0x805B},

    /* Utility */
    {"VENDOR", 0x1F00},
    {"RENDERER", 0x1F01},
    {"VERSION", 0x1F02},
    {"EXTENSIONS", 0x1F03},

    /* Errors */
    {"INVALID_VALUE", 0x0501},
    {"INVALID_ENUM", 0x0500},
    {"INVALID_OPERATION", 0x0502},
    {"STACK_OVERFLOW", 0x0503},
    {"STACK_UNDERFLOW", 0x0504},
    {"OUT_OF_MEMORY", 0x0505},

    /*
     * Extensions
     */

    /* GL_EXT_blend_minmax and GL_EXT_blend_color */
    {"CONSTANT_COLOR_EXT", 0x8001},
    {"ONE_MINUS_CONSTANT_COLOR_EXT", 0x8002},
    {"CONSTANT_ALPHA_EXT", 0x8003},
    {"ONE_MINUS_CONSTANT_ALPHA_EXT", 0x8004},
    {"BLEND_EQUATION_EXT", 0x8009},
    {"MIN_EXT", 0x8007},
    {"MAX_EXT", 0x8008},
    {"FUNC_ADD_EXT", 0x8006},
    {"FUNC_SUBTRACT_EXT", 0x800A},
    {"FUNC_REVERSE_SUBTRACT_EXT", 0x800B},
    {"BLEND_COLOR_EXT", 0x8005},

    /* GL_EXT_polygon_offset */
    {"POLYGON_OFFSET_EXT", 0x8037},
    {"POLYGON_OFFSET_FACTOR_EXT", 0x8038},
    {"POLYGON_OFFSET_BIAS_EXT", 0x8039},

    /* GL_EXT_vertex_array */
    {"VERTEX_ARRAY_EXT", 0x8074},
    {"NORMAL_ARRAY_EXT", 0x8075},
    {"COLOR_ARRAY_EXT", 0x8076},
    {"INDEX_ARRAY_EXT", 0x8077},
    {"TEXTURE_COORD_ARRAY_EXT", 0x8078},
    {"EDGE_FLAG_ARRAY_EXT", 0x8079},
    {"VERTEX_ARRAY_SIZE_EXT", 0x807A},
    {"VERTEX_ARRAY_TYPE_EXT", 0x807B},
    {"VERTEX_ARRAY_STRIDE_EXT", 0x807C},
    {"VERTEX_ARRAY_COUNT_EXT", 0x807D},
    {"NORMAL_ARRAY_TYPE_EXT", 0x807E},
    {"NORMAL_ARRAY_STRIDE_EXT", 0x807F},
    {"NORMAL_ARRAY_COUNT_EXT", 0x8080},
    {"COLOR_ARRAY_SIZE_EXT", 0x8081},
    {"COLOR_ARRAY_TYPE_EXT", 0x8082},
    {"COLOR_ARRAY_STRIDE_EXT", 0x8083},
    {"COLOR_ARRAY_COUNT_EXT", 0x8084},
    {"INDEX_ARRAY_TYPE_EXT", 0x8085},
    {"INDEX_ARRAY_STRIDE_EXT", 0x8086},
    {"INDEX_ARRAY_COUNT_EXT", 0x8087},
    {"TEXTURE_COORD_ARRAY_SIZE_EXT", 0x8088},
    {"TEXTURE_COORD_ARRAY_TYPE_EXT", 0x8089},
    {"TEXTURE_COORD_ARRAY_STRIDE_EXT", 0x808A},
    {"TEXTURE_COORD_ARRAY_COUNT_EXT", 0x808B},
    {"EDGE_FLAG_ARRAY_STRIDE_EXT", 0x808C},
    {"EDGE_FLAG_ARRAY_COUNT_EXT", 0x808D},
    {"VERTEX_ARRAY_POINTER_EXT", 0x808E},
    {"NORMAL_ARRAY_POINTER_EXT", 0x808F},
    {"COLOR_ARRAY_POINTER_EXT", 0x8090},
    {"INDEX_ARRAY_POINTER_EXT", 0x8091},
    {"TEXTURE_COORD_ARRAY_POINTER_EXT", 0x8092},
    {"EDGE_FLAG_ARRAY_POINTER_EXT", 0x8093},

    /* GL_EXT_texture_object */
    {"TEXTURE_PRIORITY_EXT", 0x8066},
    {"TEXTURE_RESIDENT_EXT", 0x8067},
    {"TEXTURE_1D_BINDING_EXT", 0x8068},
    {"TEXTURE_2D_BINDING_EXT", 0x8069},

    /* GL_EXT_texture3D */
    {"PACK_SKIP_IMAGES_EXT", 0x806B},
    {"PACK_IMAGE_HEIGHT_EXT", 0x806C},
    {"UNPACK_SKIP_IMAGES_EXT", 0x806D},
    {"UNPACK_IMAGE_HEIGHT_EXT", 0x806E},
    {"TEXTURE_3D_EXT", 0x806F},
    {"PROXY_TEXTURE_3D_EXT", 0x8070},
    {"TEXTURE_DEPTH_EXT", 0x8071},
    {"TEXTURE_WRAP_R_EXT", 0x8072},
    {"MAX_3D_TEXTURE_SIZE_EXT", 0x8073},
    {"TEXTURE_3D_BINDING_EXT", 0x806A},

    /* GL_EXT_paletted_texture */
    {"TABLE_TOO_LARGE_EXT", 0x8031},
    {"COLOR_TABLE_FORMAT_EXT", 0x80D8},
    {"COLOR_TABLE_WIDTH_EXT", 0x80D9},
    {"COLOR_TABLE_RED_SIZE_EXT", 0x80DA},
    {"COLOR_TABLE_GREEN_SIZE_EXT", 0x80DB},
    {"COLOR_TABLE_BLUE_SIZE_EXT", 0x80DC},
    {"COLOR_TABLE_ALPHA_SIZE_EXT", 0x80DD},
    {"COLOR_TABLE_LUMINANCE_SIZE_EXT", 0x80DE},
    {"COLOR_TABLE_INTENSITY_SIZE_EXT", 0x80DF},
    {"TEXTURE_INDEX_SIZE_EXT", 0x80ED},
    {"COLOR_INDEX1_EXT", 0x80E2},
    {"COLOR_INDEX2_EXT", 0x80E3},
    {"COLOR_INDEX4_EXT", 0x80E4},
    {"COLOR_INDEX8_EXT", 0x80E5},
    {"COLOR_INDEX12_EXT", 0x80E6},
    {"COLOR_INDEX16_EXT", 0x80E7},

    /* GL_EXT_shared_texture_palette */
    {"SHARED_TEXTURE_PALETTE_EXT", 0x81FB},

    /* GL_EXT_point_parameters */
    {"POINT_SIZE_MIN_EXT", 0x8126},
    {"POINT_SIZE_MAX_EXT", 0x8127},
    {"POINT_FADE_THRESHOLD_SIZE_EXT", 0x8128},
    {"DISTANCE_ATTENUATION_EXT", 0x8129},

    /* GL_EXT_rescale_normal */
    {"RESCALE_NORMAL_EXT", 0x803A},

    /* GL_EXT_abgr */
    {"ABGR_EXT", 0x8000},

    /* GL_SGIS_multitexture */
    {"SELECTED_TEXTURE_SGIS", 0x835C},
    {"SELECTED_TEXTURE_COORD_SET_SGIS", 0x835D},
    {"MAX_TEXTURES_SGIS", 0x835E},
    {"TEXTURE0_SGIS", 0x835F},
    {"TEXTURE1_SGIS", 0x8360},
    {"TEXTURE2_SGIS", 0x8361},
    {"TEXTURE3_SGIS", 0x8362},
    {"TEXTURE_COORD_SET_SOURCE_SGIS", 0x8363},

    /* GL_EXT_multitexture */
    {"SELECTED_TEXTURE_EXT", 0x83C0},
    {"SELECTED_TEXTURE_COORD_SET_EXT", 0x83C1},
    {"SELECTED_TEXTURE_TRANSFORM_EXT", 0x83C2},
    {"MAX_TEXTURES_EXT", 0x83C3},
    {"MAX_TEXTURE_COORD_SETS_EXT", 0x83C4},
    {"TEXTURE_ENV_COORD_SET_EXT", 0x83C5},
    {"TEXTURE0_EXT", 0x83C6},
    {"TEXTURE1_EXT", 0x83C7},
    {"TEXTURE2_EXT", 0x83C8},
    {"TEXTURE3_EXT", 0x83C9},

    /* GL_SGIS_texture_edge_clamp */
    {"CLAMP_TO_EDGE_SGIS", 0x812F},

    /* OpenGL 1.2 */
    {"RESCALE_NORMAL", 0x803A},
    {"CLAMP_TO_EDGE", 0x812F},
    {"MAX_ELEMENTS_VERTICES", 0xF0E8},
    {"MAX_ELEMENTS_INDICES", 0xF0E9},
    {"BGR", 0x80E0},
    {"BGRA", 0x80E1},
    {"UNSIGNED_BYTE_3_3_2", 0x8032},
    {"UNSIGNED_BYTE_2_3_3_REV", 0x8362},
    {"UNSIGNED_SHORT_5_6_5", 0x8363},
    {"UNSIGNED_SHORT_5_6_5_REV", 0x8364},
    {"UNSIGNED_SHORT_4_4_4_4", 0x8033},
    {"UNSIGNED_SHORT_4_4_4_4_REV", 0x8365},
    {"UNSIGNED_SHORT_5_5_5_1", 0x8034},
    {"UNSIGNED_SHORT_1_5_5_5_REV", 0x8366},
    {"UNSIGNED_INT_8_8_8_8", 0x8035},
    {"UNSIGNED_INT_8_8_8_8_REV", 0x8367},
    {"UNSIGNED_INT_10_10_10_2", 0x8036},
    {"UNSIGNED_INT_2_10_10_10_REV", 0x8368},
    {"LIGHT_MODEL_COLOR_CONTROL", 0x81F8},
    {"SINGLE_COLOR", 0x81F9},
    {"SEPARATE_SPECULAR_COLOR", 0x81FA},
    {"TEXTURE_MIN_LOD", 0x813A},
    {"TEXTURE_MAX_LOD", 0x813B},
    {"TEXTURE_BASE_LEVEL", 0x813C},
    {"TEXTURE_MAX_LEVEL", 0x813D},

    {"CURRENT_BIT", 0x00000001},
    {"POINT_BIT", 0x00000002},
    {"LINE_BIT", 0x00000004},
    {"POLYGON_BIT", 0x00000008},
    {"POLYGON_STIPPLE_BIT", 0x00000010},
    {"PIXEL_MODE_BIT", 0x00000020},
    {"LIGHTING_BIT", 0x00000040},
    {"FOG_BIT", 0x00000080},
    {"DEPTH_BUFFER_BIT", 0x00000100},
    {"ACCUM_BUFFER_BIT", 0x00000200},
    {"STENCIL_BUFFER_BIT", 0x00000400},
    {"VIEWPORT_BIT", 0x00000800},
    {"TRANSFORM_BIT", 0x00001000},
    {"ENABLE_BIT", 0x00002000},
    {"COLOR_BUFFER_BIT", 0x00004000},
    {"HINT_BIT", 0x00008000},
    {"EVAL_BIT", 0x00010000},
    {"LIST_BIT", 0x00020000},
    {"TEXTURE_BIT", 0x00040000},
    {"SCISSOR_BIT", 0x00080000},
    {"ALL_ATTRIB_BITS", 0x000fffff},
    {"CLIENT_PIXEL_STORE_BIT", 0x00000001},
    {"CLIENT_VERTEX_ARRAY_BIT", 0x00000002},
    {"CLIENT_ALL_ATTRIB_BITS", 0x0000FFFF}
};

int
ici_gl_vars_init(struct_t *s)
{
    struct var	*v;

    for (v = vars; v - vars < nels(vars); ++v)
	if (ici_cmkvar(s, v->name, 'i', &v->value))
	    return 1;
    return 0;
}
