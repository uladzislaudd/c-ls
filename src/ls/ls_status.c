#include "ls_os.h"
#include "ls_status.h"

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
        fprintf(stderr, "General failure!\n");
        break;

    case LS_STATUS_SYSCALL_FAILURE:
        fprintf(stderr, "Syscall unexepcted behavior detected!\n");
        break;

    case LS_STATUS_PATH_INVALID:
        fprintf(stderr, "Invalid path!\n");
        break;

    case LS_STATUS_ARGS_INVALID:
        fprintf(stderr, "Invalid argument!\n");
        break;
    
    default:
        rv = ls_os_status_print(status);
        break;
    }

    return rv;
}
