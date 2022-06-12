#if !defined(__LS_OS_H__)
#define __LS_OS_H__

#include "ls_stat.h"
#include "ls_status.h"

#include <stdlib.h>

int ls_os_status_print(LS_STATUS status);

LS_STATUS ls_os_getcwd(char *buf, size_t size);
LS_STATUS ls_os_chdir(const char *path);
LS_STATUS ls_os_readlink(const char *path, char *buf, size_t size);
LS_STATUS ls_os_realpath(const char *path, char *resolved_path);
LS_STATUS ls_os_dirname(char *path, char **name);
LS_STATUS ls_os_basename(char *path, char **name);

typedef void *LS_OS_DIR;
LS_STATUS ls_os_dir_open(const char *path, LS_OS_DIR *dir);
LS_STATUS ls_os_dir_close(LS_OS_DIR dir);
LS_STATUS ls_os_dir_tell(LS_OS_DIR dir, long *pos);
LS_STATUS ls_os_dir_rewind(LS_OS_DIR dir);
LS_STATUS ls_os_dir_seek(LS_OS_DIR dir, long pos);

typedef void *LS_OS_DIRENT;
LS_STATUS ls_os_dir_read(LS_OS_DIR dir, LS_OS_DIRENT *dirent);
typedef LS_STATUS (*ls_os_dir_iterate_fn)(LS_OS_DIRENT dirent, void *ctx);
LS_STATUS ls_os_dir_iterate(const char *path, ls_os_dir_iterate_fn fn, void *ctx);
const char *ls_os_dirent_name(LS_OS_DIRENT dirent);

LS_STATUS ls_os_is_dir(const char *path);

LS_STATUS ls_os_stat(const char *path, struct ls_stat *ls);

#endif // __LS_OS_H__
