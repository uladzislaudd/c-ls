#if !defined(__LS_OS_H__)
#define __LS_OS_H__

#include "ls_status.h"

#include <stdlib.h>

LS_STATUS ls_os_getcwd(char *buf, size_t size);
LS_STATUS ls_os_chdir(const char *path);
LS_STATUS ls_os_readlink(const char *path, char *buf, size_t size);

#endif // __LS_OS_H__
