#include "ls_os.h"
#include "ls_stat.h"
#include "ls_status.h"

#include <dirent.h>
#include <errno.h>
#include <grp.h>
#include <libgen.h>
#include <limits.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <time.h>

int ls_os_status_print(LS_STATUS status)
{
    fprintf(stderr, "%s\n", status > 0 ? strerror(status) : "General failure!");
    return 1;
}

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

LS_STATUS ls_os_realpath(const char *path, char *resolved_path)
{
    errno = 0;
    return realpath(path, resolved_path) != NULL ? LS_STATUS_SUCCESS : errno;
}

LS_STATUS ls_os_dirname(char *path, char **name)
{
    *name = dirname(path);
    return LS_STATUS_SUCCESS;
}

LS_STATUS ls_os_basename(char *path, char **name)
{
    *name = basename(path);
    return LS_STATUS_SUCCESS;
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

LS_STATUS ls_os_is_dir(const char *path)
{
    LS_STATUS rv = LS_STATUS_FAILURE;
    struct stat path_stat;

    errno = 0;
    if (stat(path, &path_stat) == 0) {
        rv = S_ISDIR(path_stat.st_mode) ? LS_STATUS_DIR : LS_STATUS_FILE;
    } else {
        rv = errno;
    }
    
    return rv;
}

static char *mode_str(__mode_t mode, char output[10])
{
    const char chars[] = "rwxrwxrwx";
    for (size_t i = 0; i < 9; i++) {
        output[i] = (mode & (1 << (8-i))) ? chars[i] : '-';
    }
    output[9] = '\0';
    return output;
}

static const char months[12][4] = {"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec" };
static char *time_str(__time_t time, char output[16])
{
    struct tm *ti = localtime(&time);
    if (ti != NULL) {
        snprintf(output, 16, "%s %2d %.2d:%.2d", months[ti->tm_mon - 1], ti->tm_mday, ti->tm_hour, ti->tm_min);
    }
    return output;
}

LS_STATUS ls_os_stat(const char *path, struct ls_stat *ls)
{
    LS_STATUS rv = LS_STATUS_SUCCESS;

    char buf1[10], buf2[16], link[256], t = '-';
    struct passwd *u = NULL;
    struct group *g = NULL;
    struct stat s;
    struct ls_stat ls1;

    errno = 0;
    if (lstat(path, &s) != 0) {
        return errno;
    }

    switch (s.st_mode & __S_IFMT) {
    case __S_IFBLK:  t = '-'; break;
    case __S_IFCHR:  t = 'c'; break;
    case __S_IFDIR:  t = 'd'; break;
    case __S_IFIFO:  t = '-'; break;
    case __S_IFLNK:  t = 'l'; break;
    case __S_IFREG:  t = '-'; break;
    case __S_IFSOCK: t = '-'; break;
    default: return LS_STATUS_SYSCALL_FAILURE;
    }

    sprintf(ls->m, "%c%s", t, mode_str(s.st_mode, buf1));
    sprintf(ls->l, "%lu", s.st_nlink);
    u = getpwuid(s.st_uid); u ? sprintf(ls->u, "%s", u->pw_name) : sprintf(ls->u, "%d", s.st_uid);
    g = getgrgid(s.st_gid); g ? sprintf(ls->g, "%s", g->gr_name) : sprintf(ls->g, "%d", s.st_gid);
    sprintf(ls->s, "%ld", s.st_size);
    sprintf(ls->t, "%s", time_str(s.st_mtime, buf2));

    switch (s.st_mode & __S_IFMT) {
    case __S_IFBLK:
        /*  failthrough */
    case __S_IFCHR:
        sprintf(ls->p, "\e[1;40m\e[1;33m%s\e[0m", ls->n);
        break;

    case __S_IFDIR:
        sprintf(ls->p, "\e[1;34m%s\e[0m", ls->n);
        break;

    case __S_IFIFO:
        sprintf(ls->p, "\e[1;40m\e[2;31m%s\e[0m", ls->n);
        break;

    case __S_IFLNK:
        memset(link, 0x00, sizeof(link));
        memset(&ls1, 0x00, sizeof(ls1));
        rv = ls_os_readlink(ls->n, link, sizeof(link));
        if (rv != LS_STATUS_SUCCESS) { break; }
        ls1.n = link;
        rv = ls_os_stat(link, &ls1);
        if (rv != LS_STATUS_SUCCESS) { break; }
        sprintf(ls->p, "\e[1;36m%s\e[0m -> %s", ls->n, ls1.p);
        break;

    case __S_IFREG:
        sprintf(ls->p, "\e[0m%s\e[0m", ls->n);
        break;

    case __S_IFSOCK:
        sprintf(ls->p, "\e[1;35m%s\e[0m", ls->n);
        break;

    default:
        rv = LS_STATUS_SYSCALL_FAILURE;
        break;
    }

    return rv;
}

