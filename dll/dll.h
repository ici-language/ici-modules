#ifndef ICI_DLL_H
#define ICI_DLL_H

/*
 * We'd like to call this dll_t. But that name has been taken.
 */
typedef struct dllib
{
    object_t    o_head;
    dll_t       *dll_lib;
    struct_t    *dll_struct;
}
    dllib_t;

#define dllof(o)        ((dllib_t *)(o))
#define isdll(o)        ((o)->o_type == &dll_type)

dllib_t                 *new_dll(dll_t lib);
cfunc_t                 *new_dllfunc(dll_t *lib, string_t *s);
extern type_t           dll_type;

#endif /* ICI_DLL_H */
