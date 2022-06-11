#include "ls_os.h"
#include "ls_status.h"

#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

LS_STATUS ls_os_getcwd(char *buf, size_t size)
{
    errno = 0;
    return getcwd(buf, size) == NULL ? LS_STATUS_SUCCESS : errno;
}

LS_STATUS ls_os_chdir(const char *path)
{
    errno = 0;
    return chdir(path) == 0 ? LS_STATUS_SUCCESS : errno;
}

LS_STATUS ls_os_readlink(const char *path, char *buf, size_t size)
{
    errno = 0;
    return readlink(path, buf, size) != -1 ? LS_STATUS_SUCCESS : errno;
}

LS_STATUS ls_os_dir_open(const char *path, LS_OS_DIR *dir)
{
    LS_STATUS rv = LS_STATUS_FAILURE;
    DIR *d = NULL;

    if (dir == NULL) { return LS_STATUS_ARGS_INVALID; }

    errno = 0;
    d = opendir(path);
    if (d == NULL) {
        rv = errno;
    } else {
        rv = LS_STATUS_SUCCESS;
        *dir = (LS_OS_DIR)d;
    }

    return rv;
}

LS_STATUS ls_os_dir_close(LS_OS_DIR dir)
{
    if (dir == NULL) { return LS_STATUS_ARGS_INVALID; }
    errno = 0;
    return closedir((DIR *)dir) == 0 ? LS_STATUS_SUCCESS : errno;
}

LS_STATUS ls_os_dir_read(LS_OS_DIR dir, LS_OS_DIRENT *dirent)
{
    LS_STATUS rv = LS_STATUS_FAILURE;
    struct dirent *di = NULL;

    if (dir == NULL || dirent == NULL) {
        return LS_STATUS_ARGS_INVALID;
    }

    errno = 0;
    di = readdir((DIR *)dir);
    if (di == NULL) {
        rv = errno == 0 ? LS_STATUS_DIR_END : errno;
    } else {
        rv = LS_STATUS_SUCCESS;
        *dirent = (LS_OS_DIRENT)di;
    }

    return rv;
}

LS_STATUS ls_os_dir_tell(LS_OS_DIR dir, long *pos)
{
    if (dir == NULL || pos == NULL) {
        return LS_STATUS_ARGS_INVALID;
    }

    errno = 0;
    *pos = telldir((DIR *)dir);

    return *pos == -1 ? errno : LS_STATUS_SUCCESS;
}

LS_STATUS ls_os_dir_rewind(LS_OS_DIR dir)
{
    if (dir == NULL) { return LS_STATUS_ARGS_INVALID; }
    rewinddir((DIR *)dir);
    return LS_STATUS_SUCCESS;
}

LS_STATUS ls_os_dir_seek(LS_OS_DIR dir, long pos)
{
    if (dir == NULL) { return LS_STATUS_ARGS_INVALID; }
    seekdir((DIR *)dir, pos);
    return LS_STATUS_SUCCESS;
}

const char *ls_os_dirent_name(LS_OS_DIRENT dirent)
{
    return (void *)dirent == NULL ? NULL : ((struct dirent *)dirent)->d_name;
}
