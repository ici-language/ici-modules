#if defined(_WIN32)
# include "load-w32.h"
#elif defined(__BEOS__)
# include "load-beos.h"
#else /* Unix */
# include <sys/types.h>
# include <dlfcn.h>
# include <unistd.h>

typedef void *dll_t;
#define valid_dll(dll)  ((dll) != NULL)

static char *
get_dll_path(void)
{
    char	*path;

    if ((path = getenv("ICIPATH")) == NULL)
    	path = ".:" PREFIX "/lib/ici:/usr/X11R6/lib:/usr/local/lib:/usr/lib:/lib";
    return path ;
}

#endif /* _WIN32, __BEOS__ */

#ifndef RTLD_NOW
#define RTLD_NOW 1
#endif

#ifndef RTLD_GLOBAL
#define RTLD_GLOBAL 0
#endif
