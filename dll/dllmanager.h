#ifndef ICI_DLLMANAGER_H
#define ICI_DLLMANAGER_H

typedef struct dllmanager
{
    object_t    o_head;
    struct_t    *dllm_struct;
}
    dllmanager_t;

#define dllmanagerof(o)     ((dllmanager_t *)(o))
#define isdllmanager(o)     ((o)->o_type == &dllm_type)

extern type_t   dllmanager_type;

#endif /* #ifndef ICI_DLLMANAGER_H */
