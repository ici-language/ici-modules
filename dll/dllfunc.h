#ifndef ICI_DLLFUNC_H
#define ICI_DLLFUNC_H

typedef enum
{
    CT_UNKNOWN,
    CT_CDECL,           /* Push RH arg first, caller pops. */
    CT_STDCALL,         /* Push RH arg first, called function pops. */
    CT_FASTCALL         /* First 2 args in regs, then as per cdecl. */
}
    calltype_t;

typedef struct dllfunc
{
    object_t        o_head;
    string_t        *df_name;
    string_t        *df_decl;
    struct dllib    *df_dllib;
    void            *df_addr;
    calltype_t      df_calltype;
}
    dllfunc_t;

#define dllfuncof(o)     ((dllfunc_t *)(o))
#define isdllfunc(o)     (objof(o)->o_tcode == dllfunc_tcode)

extern type_t   dllfunc_type;
extern int      dllfunc_tcode;
dllfunc_t       *new_dllfunc(dllib_t *dllib, ici_str_t *name, ici_str_t *decl);

#endif /* #ifndef ICI_DLLFUNC_H */
