#include "ici.h"
#include "imlib.h"
#include "display.h"
#include "window.h"
#include "pixmap.h"

static unsigned long
mark_ImlibData(object_t *o)
{
    o->o_flags |= O_MARK;
    return sizeof (ici_Imlib_Data_t);
}

static unsigned long
hash_ImlibData(object_t *o)
{
    return (unsigned long)imlibdataof(o)->i_data * UNIQUE_PRIME;
}

static int
cmp_ImlibData(object_t *a, object_t *b)
{
    if (a == b)
	return 0;
    return imlibdataof(a)->i_data != imlibdataof(b)->i_data;
}

INLINE
static ici_Imlib_Data_t *
atom_imlibdata(ImlibData *data)
{
    object_t	**po, *o;

    for
    (
	po = &atoms[ici_atom_hash_index((unsigned long)data * UNIQUE_PRIME)];
	(o = *po) != NULL;
	--po < atoms ? po = atoms + atomsz - 1 : NULL
    )
    {
	if (isimlibdata(o) && imlibdataof(o)->i_data == data)
	    return imlibdataof(o);
    }
    return NULL;
}

type_t	ici_Imlib_Data_type =
{
    mark_ImlibData,
    free_simple,
    hash_ImlibData,
    cmp_ImlibData,
    copy_simple,
    assign_simple,
    fetch_simple,
    "ImlibData"
};

ici_Imlib_Data_t *
ici_Imlib_new_ImlibData(ImlibData *data)
{
    ici_Imlib_Data_t	*i;

    if ((i = atom_imlibdata(data)) != NULL)
	incref(i);
    else if ((i = talloc(ici_Imlib_Data_t)) != NULL)
    {
	objof(i)->o_tcode = TC_OTHER;
	objof(i)->o_nrefs = 1;
	objof(i)->o_flags = 0;
	objof(i)->o_type  = &ici_Imlib_Data_type;
	rego(i);
	i->i_data = data;
	i = imlibdataof(atom(objof(i), 1));
    }
    return i;
}

static unsigned long
mark_ImlibImage(object_t *o)
{
    o->o_flags |= O_MARK;
    return sizeof (ici_Imlib_Image_t);
}

static unsigned long
hash_ImlibImage(object_t *o)
{
    return (unsigned long)imlibimageof(o)->i_image * UNIQUE_PRIME;
}

static int
cmp_ImlibImage(object_t *a, object_t *b)
{
    if (a == b)
	return 0;
    return imlibimageof(a)->i_image != imlibimageof(b)->i_image;
}

INLINE
static ici_Imlib_Image_t *
atom_imlibimage(ImlibImage *image)
{
    object_t	**po, *o;

    for
    (
	po = &atoms[ici_atom_hash_index((unsigned long)image * UNIQUE_PRIME)];
	(o = *po) != NULL;
	--po < atoms ? po = atoms + atomsz - 1 : NULL
    )
    {
	if (isimlibimage(o) && imlibimageof(o)->i_image == image)
	    return imlibimageof(o);
    }
    return NULL;
}

NEED_STRING(rgb_width);
NEED_STRING(rgb_height);
NEED_STRING(filename);
NEED_STRING(width);
NEED_STRING(height);

static object_t *
fetch_ImlibImage(object_t *o, object_t *k)
{
    NEED_STRINGS(NULL);
    if (k == objof(STRING(rgb_width)))
	return objof(new_int(imlibimageof(o)->i_image->rgb_width));
    if (k == objof(STRING(rgb_height)))
	return objof(new_int(imlibimageof(o)->i_image->rgb_height));
    if (k == objof(STRING(filename)))
	return objof(new_cname(imlibimageof(o)->i_image->filename));
    if (k == objof(STRING(width)))
	return objof(new_int(imlibimageof(o)->i_image->width));
    if (k == objof(STRING(height)))
	return objof(new_int(imlibimageof(o)->i_image->height));
    return objof(&o_null);
}

type_t	ici_Imlib_Image_type =
{
    mark_ImlibImage,
    free_simple,
    hash_ImlibImage,
    cmp_ImlibImage,
    copy_simple,
    assign_simple,
    fetch_ImlibImage,
    "ImlibImage"
};

ici_Imlib_Image_t *
ici_Imlib_new_ImlibImage(ImlibImage *image)
{
    ici_Imlib_Image_t	*i;

    if ((i = atom_imlibimage(image)) != NULL)
	incref(i);
    else if ((i = talloc(ici_Imlib_Image_t)) != NULL)
    {
	objof(i)->o_tcode = TC_OTHER;
	objof(i)->o_nrefs = 1;
	objof(i)->o_flags = 0;
	objof(i)->o_type  = &ici_Imlib_Image_type;
	rego(i);
	i->i_image = image;
	i->i_destroyed = 0;
	i = imlibimageof(atom(objof(i), 1));
    }
    return i;
}

FUNC(init)
{
    ici_X_display_t	*display;
    ImlibData		*imlibdata;

    if (ici_typecheck("o", &display))
	return 1;
    if (!isdisplay(objof(display)))
	return ici_argerror(0);
    if ((imlibdata = Imlib_init(display->d_display)) == NULL)
    {
	error = "unable to initialize Imlib";
	return 1;
    }
    return ici_ret_with_decref(objof(ici_Imlib_new_ImlibData(imlibdata)));
}

FUNC(load_image)
{
    ici_Imlib_Data_t	*data;
    char		*name;
    ImlibImage		*image;

    if (ici_typecheck("os", &data, &name))
	return 1;
    if (!isimlibdata(objof(data)))
	return ici_argerror(0);
    image = Imlib_load_image(data->i_data, name);
    if (image == NULL)
    {
	error = "unable to load image";
	return 1;
    }
    return ici_ret_with_decref(objof(ici_Imlib_new_ImlibImage(image)));
}

FUNC(apply_image)
{
    ici_Imlib_Data_t	*data;
    ici_Imlib_Image_t	*image;
    ici_X_window_t	*window;

    if (ici_typecheck("ooo", &data, &image, &window))
	return 1;
    if (!isimlibdata(objof(data)))
	return ici_argerror(0);
    if (!isimlibimage(objof(image)))
	return ici_argerror(1);
    if (!iswindow(objof(window)))
	return ici_argerror(2);
    CHECK_IMAGE(image);
    Imlib_apply_image(data->i_data, image->i_image, window->w_window);
    return null_ret();
}

FUNC(render)
{
    ici_Imlib_Data_t	*data;
    ici_Imlib_Image_t	*image;
    long		width;
    long		height;

    if (ici_typecheck("ooii", &data, &image, &width, &height))
	return 1;
    if (!isimlibdata(objof(data)))
	return ici_argerror(0);
    if (!isimlibimage(objof(image)))
	return ici_argerror(1);
    CHECK_IMAGE(image);
    if (!Imlib_render(data->i_data, image->i_image, width, height))
    {
	error = "failed to render image";
	return 1;
    }
    return null_ret();
}

FUNC(copy_image)
{
    ici_Imlib_Data_t	*data;
    ici_Imlib_Image_t	*image;
    ici_X_pixmap_t	*pixmap;

    if (ici_typecheck("oo", &data, &image))
	return 1;
    if (!isimlibdata(objof(data)))
	return ici_argerror(0);
    if (!isimlibimage(objof(image)))
	return ici_argerror(1);
    CHECK_IMAGE(image);
    pixmap = ici_X_new_pixmap(Imlib_copy_image(data->i_data, image->i_image));
    if (pixmap == NULL)
	return 1;
    return ici_ret_with_decref(objof(pixmap));
}

FUNC(copy_mask)
{
    ici_Imlib_Data_t	*data;
    ici_Imlib_Image_t	*image;
    ici_X_pixmap_t	*pixmap;

    if (ici_typecheck("oo", &data, &image))
	return 1;
    if (!isimlibdata(objof(data)))
	return ici_argerror(0);
    if (!isimlibimage(objof(image)))
	return ici_argerror(1);
    CHECK_IMAGE(image);
    pixmap = ici_X_new_pixmap(Imlib_copy_mask(data->i_data, image->i_image));
    if (pixmap == NULL)
	return 1;
    return ici_ret_with_decref(objof(pixmap));
}

FUNC(move_image)
{
    ici_Imlib_Data_t	*data;
    ici_Imlib_Image_t	*image;
    ici_X_pixmap_t	*pixmap;

    if (ici_typecheck("oo", &data, &image))
	return 1;
    if (!isimlibdata(objof(data)))
	return ici_argerror(0);
    if (!isimlibimage(objof(image)))
	return ici_argerror(1);
    CHECK_IMAGE(image);
    pixmap = ici_X_new_pixmap(Imlib_move_image(data->i_data, image->i_image));
    if (pixmap == NULL)
	return 1;
    return ici_ret_with_decref(objof(pixmap));
}

FUNC(move_mask)
{
    ici_Imlib_Data_t	*data;
    ici_Imlib_Image_t	*image;
    ici_X_pixmap_t	*pixmap;

    if (ici_typecheck("oo", &data, &image))
	return 1;
    if (!isimlibdata(objof(data)))
	return ici_argerror(0);
    if (!isimlibimage(objof(image)))
	return ici_argerror(1);
    CHECK_IMAGE(image);
    pixmap = ici_X_new_pixmap(Imlib_move_mask(data->i_data, image->i_image));
    if (pixmap == NULL)
	return 1;
    return ici_ret_with_decref(objof(pixmap));
}

FUNC(destroy_image)
{
    ici_Imlib_Data_t	*data;
    ici_Imlib_Image_t	*image;

    if (ici_typecheck("oo", &data, &image))
	return 1;
    if (!isimlibdata(objof(data)))
	return ici_argerror(0);
    if (!isimlibimage(objof(image)))
	return ici_argerror(1);
    CHECK_IMAGE(image);
    Imlib_destroy_image(data->i_data, image->i_image);
    image->i_destroyed = 1;
    return null_ret();
}

FUNC(kill_image)
{
    ici_Imlib_Data_t	*data;
    ici_Imlib_Image_t	*image;

    if (ici_typecheck("oo", &data, &image))
	return 1;
    if (!isimlibdata(objof(data)))
	return ici_argerror(0);
    if (!isimlibimage(objof(image)))
	return ici_argerror(1);
    CHECK_IMAGE(image);
    Imlib_kill_image(data->i_data, image->i_image);
    image->i_destroyed = 1;
    return null_ret();
}

FUNC(changed_image)
{
    ici_Imlib_Data_t	*data;
    ici_Imlib_Image_t	*image;

    if (ici_typecheck("oo", &data, &image))
	return 1;
    if (!isimlibdata(objof(data)))
	return ici_argerror(0);
    if (!isimlibimage(objof(image)))
	return ici_argerror(1);
    CHECK_IMAGE(image);
    Imlib_changed_image(data->i_data, image->i_image);
    return null_ret();
}

FUNC(paste_image)
{
    ici_Imlib_Data_t	*data;
    ici_Imlib_Image_t	*image;
    object_t		*drawable;
    unsigned long	d;
    long		x, y, w, h;

    if (ici_typecheck("oooiiii", &data, &image, &drawable, &x, &y, &w, &h))
	return 1;
    if (!isimlibdata(objof(data)))
	return ici_argerror(0);
    if (!isimlibimage(objof(image)))
	return ici_argerror(1);
    if (iswindow(drawable))
	d = windowof(drawable)->w_window;
    else if (ispixmap(drawable))
	d = pixmapof(drawable)->p_pixmap;
    else
	return ici_argerror(2);
    CHECK_IMAGE(image);
    Imlib_paste_image(data->i_data, image->i_image, d, x, y, w, h);
    return null_ret();
}

FUNC(load_file_to_pixmap)
{
    ici_Imlib_Data_t	*data;
    ici_Imlib_Image_t	*image;

    if (ici_typecheck("oo", &data, &image))
	return 1;
    if (!isimlibdata(objof(data)))
	return ici_argerror(0);
    if (!isimlibimage(objof(image)))
	return ici_argerror(1);

    return null_ret();
}

FUNC(free_pixmap)
{
    ici_Imlib_Data_t	*data;
    ici_X_pixmap_t	*pixmap;

    if (ici_typecheck("oo", &data, &pixmap))
	return 1;
    if (!isimlibdata(objof(data)))
	return ici_argerror(0);
    if (!ispixmap(objof(pixmap)))
	return ici_argerror(1);
    Imlib_free_pixmap(data->i_data, pixmap->p_pixmap);
    return null_ret();
}
