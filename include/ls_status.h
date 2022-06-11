#if !defined(__LS_STATUS_H__)
#define __LS_STATUS_H__

typedef int LS_STATUS;
enum {
    LS_STATUS_SUCCESS = 0x0000,

    LS_STATUS_FAILURE = -0x1000,
    LS_STATUS_NO_MEMORY,
    LS_STATUS_PATH_INVALID,
    LS_STATUS_ARGS_INVALID,

    LS_STATUS_DIR = -0x2000,
    LS_STATUS_FILE,
};

int ls_status_print(LS_STATUS status);

#endif // __LS_STATUS_H__
