#include "ls_status.h"

#include <errno.h>

#include <string.h>
#include <stdio.h>

int ls_status_print(LS_STATUS status)
{
    int rv = 1;

    switch (status) {
    case LS_STATUS_SUCCESS:
        /*  failthrough */
    case LS_STATUS_DIR:
        /*  failthrough */
    case LS_STATUS_FILE:
        rv = 0;
        break;

    case LS_STATUS_FAILURE:
        fprintf(stderr, "Unknown error!\n");
        break;

    case LS_STATUS_PATH_INVALID:
        fprintf(stderr, "Invalid path!\n");
        break;

    case LS_STATUS_ARGS_INVALID:
        fprintf(stderr, "Invalid argument!\n");
        break;
    
    default:
        fprintf(stderr, "%s\n", strerror(status));
        break;
    }

    return rv;
}
