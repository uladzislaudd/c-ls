#include "ls_os.h"
#include "ls_status.h"

#include <stdlib.h>

LS_STATUS ls_os_dir_iterate(LS_OS_DIR dir, ls_os_dir_iterate_fn fn, void *ctx)
{
    LS_STATUS rv = LS_STATUS_FAILURE, _rv;
    long cl = 0;
    LS_OS_DIRENT di;

    if (dir == NULL) {
        return LS_STATUS_ARGS_INVALID;
    }

    rv = ls_os_dir_tell(dir, &cl);
    if (!LS_STATUS_OK(rv)) {
        return rv;
    }

    rv = ls_os_dir_rewind(dir);
    if (!LS_STATUS_OK(rv)) {
        return rv;
    }

    while (1) {
        rv = ls_os_dir_read(dir, &di);
        if (rv == LS_STATUS_DIR_END) { rv = LS_STATUS_SUCCESS; break; }
        if (!LS_STATUS_OK(rv)) { break; }
        rv = fn(di, ctx);
        if (!LS_STATUS_OK(rv)) { break; }
    }
    
    _rv = ls_os_dir_seek(dir, cl);

    return rv == LS_STATUS_SUCCESS ? _rv : rv;
}
