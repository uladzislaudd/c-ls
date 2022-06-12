#include "ls_os.h"
#include "ls_status.h"

#include <stdlib.h>

LS_STATUS ls_os_dir_iterate(const char *path, ls_os_dir_iterate_fn fn, void *ctx)
{
    LS_STATUS rv = LS_STATUS_FAILURE, _rv;
    LS_OS_DIR dir;
    LS_OS_DIRENT di;

    if (path == NULL || fn == NULL) {
        return LS_STATUS_ARGS_INVALID;
    }

    rv = ls_os_dir_open(path, &dir);
    if (rv != LS_STATUS_SUCCESS) {
        return rv;
    }

    while (1) {
        rv = ls_os_dir_read(dir, &di);
        if (rv == LS_STATUS_DIR_END) { rv = LS_STATUS_SUCCESS; break; }
        if (rv != LS_STATUS_SUCCESS) { break; }
        rv = fn(di, ctx);
        if (rv != LS_STATUS_SUCCESS) { break; }
    }
    
    _rv = ls_os_dir_close(dir);

    return rv == LS_STATUS_SUCCESS ? _rv : rv;
}
