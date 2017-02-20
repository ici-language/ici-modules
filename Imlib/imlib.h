#ifndef ICI_IMLIB_H
#define	ICI_IMLIB_H

#include <Imlib.h>

#ifndef ICI_OBJECT_H
#include <object.h>
#endif

typedef struct
{
    object_t	o_head;
    ImlibData	*i_data;
}
ici_Imlib_Data_t;

#define	imlibdataof(o)	((ici_Imlib_Data_t *)(o))
#define	isimlibdata(o)	((o)->o_type == &ici_Imlib_Data_type)

extern type_t		ici_Imlib_Data_type;
ici_Imlib_Data_t	*ici_Imlib_new_ImlibData(ImlibData *);

typedef struct
{
    object_t	o_head;
    ImlibImage	*i_image;
    int		i_destroyed;
}
ici_Imlib_Image_t;

#define	imlibimageof(o)	((ici_Imlib_Image_t *)(o))
#define	isimlibimage(o)	((o)->o_type == &ici_Imlib_Image_type)

extern type_t		ici_Imlib_Image_type;
ici_Imlib_Image_t	*ici_Imlib_new_ImlibImage(ImlibImage *);

#define	CHECK_IMAGE(i)					\
    do							\
	if ((i)->i_destroyed)				\
	{						\
	    error = "attempt to use destroyed image";	\
	    return 1;					\
	}						\
    while (0)

#endif
