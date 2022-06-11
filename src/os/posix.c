#include "ls_os.h"
#include "ls_status.h"

#include <errno.h>
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
