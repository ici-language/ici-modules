#ifndef ICI_WINGS_H
#define ICI_WINGS_H

#define	TYPE(NAME,CTYPE,CNAME)\
    typedef struct\
    {\
	object_t	o_head;\
	CTYPE	CNAME;\
    }\
    NAME ## _t;\
    \
    static type_t NAME ## _type;\
    \
    inline static int\
    is ## NAME (object_t *o)\
    {\
	return o->o_type == & NAME ## _type;\
    }\
    \
    inline static NAME ## _t *\
    NAME ## of (void *p)\
    {\
	return p;\
    }

#define	DEFINE_TYPE(NAME, CTYPE, CNAME)\
static unsigned long \
mark_ ## NAME (object_t *o)\
{\
    o->o_flags |= O_MARK;\
    return sizeof (NAME ## _t);\
}\
\
static type_t	NAME ## _type =\
{\
    mark_ ## NAME ,\
    free_simple,\
    hash_unique,\
    cmp_unique,\
    copy_simple,\
    assign_simple,\
    fetch_simple,\
    # NAME\
};\
\
static NAME ## _t * \
new_ ## NAME(CTYPE val)\
{\
    NAME ## _t	*o;\
\
    if ((o = talloc(NAME ## _t)) != NULL)\
    {\
	objof(o)->o_tcode = TC_OTHER;\
	objof(o)->o_flags = 0;\
	objof(o)->o_nrefs = 1;\
	objof(o)->o_type  = & NAME ## _type;\
	rego(o);\
	o->CNAME = val;\
    }\
    return o;\
}

#define	DEFINE_ATOMIC_TYPE(NAME, CTYPE, CNAME)\
\
static unsigned long \
mark_ ##NAME (object_t *o)\
{\
    o->o_flags |= O_MARK;\
    return sizeof (NAME ## _t);\
}\
\
static unsigned long \
hash_ ## NAME(object_t *o)\
{\
    return (unsigned long)NAME ## of(o)->CNAME * UNIQUE_PRIME;\
}\
\
static int \
cmp_ ## NAME (object_t *a, object_t *b)\
{\
    return NAME ## of(a)->CNAME != NAME ## of(b)->CNAME;\
}\
\
static type_t	NAME ## _type =\
{\
    mark_ ## NAME,\
    free_simple,\
    hash_ ## NAME,\
    cmp_ ## NAME,\
    copy_simple,\
    assign_simple,\
    fetch_simple,\
    # NAME\
};\
\
static NAME ##_t * \
atom_ ## NAME(CTYPE val)\
{\
    object_t	**po, *o;\
\
    for\
    (\
	po = &atoms[ici_atom_hash_index((unsigned long)val * UNIQUE_PRIME)];\
	(o = *po) != NULL;\
	--po < atoms ? po = atoms + atomsz - 1 : NULL\
    )\
    {\
	if (is ## NAME(o) && NAME ## of(o)->CNAME == val)\
	{\
	    incref(o);\
	    return NAME ## of(o);\
	}\
    }\
    return NULL;\
}\
\
static NAME ##_t *\
new_ ## NAME(CTYPE val)\
{\
    NAME ## _t	* NAME;\
\
    if ((NAME = atom_ ##NAME(val)) != NULL)\
	return NAME;\
    if ((NAME = talloc(NAME ## _t)) == NULL)\
	return NULL;\
    objof(NAME)->o_tcode = TC_OTHER;\
    objof(NAME)->o_flags = 0;\
    objof(NAME)->o_nrefs = 1;\
    objof(NAME)->o_type  = & NAME ## _type;\
    rego(NAME);\
    NAME->CNAME = val;\
    return NAME ## of(atom(objof(NAME), 1));\
}\

TYPE(wmcolor, WMColor *, wm_color);
TYPE(wmhandlerid, WMHandlerID *, wm_handlerid);
TYPE(wmfont, WMFont *, wm_font);
TYPE(wmpixmap, WMPixmap *, wm_pixmap);
TYPE(wmscreen, WMScreen *, wm_screen);
TYPE(wmview, WMView *, wm_view);
TYPE(wmwindow, WMWindow *, wm_window);
TYPE(wmframe, WMFrame *, wm_frame);
TYPE(wmbutton, WMButton *, wm_button);
TYPE(wmlabel, WMLabel *, wm_label);
TYPE(wmtextfield, WMTextField *, wm_textfield);
TYPE(wmscroller, WMScroller *, wm_scroller);
TYPE(wmscrollview, WMScrollView *, wm_scrollview);
TYPE(wmlist, WMList *, wm_list);
TYPE(wmbrowser, WMBrowser *, wm_browser);
TYPE(wmpopupbutton, WMPopUpButton *, wm_popupbutton);
TYPE(wmcolorwell, WMColorWell *, wm_colorwell);
TYPE(wmslider, WMSlider *, wm_slider);
TYPE(wmsplitview, WMSplitView *, wm_splitview);
TYPE(rcontext, RContext *, r_context);
TYPE(wmalertpanel, WMAlertPanel *, wm_alertpanel);
TYPE(wminputpanel, WMInputPanel *, wm_inputpanel);
TYPE(wmlistitem, WMListItem *, wm_listitem);
TYPE(wmopenpanel, WMOpenPanel *, wm_openpanel);
TYPE(wmsavepanel, WMSavePanel *, wm_savepanel);
TYPE(wmfontpanel, WMFontPanel *, wm_fontpanel);

#endif /* #ifndef ICI_WINGS_H */
