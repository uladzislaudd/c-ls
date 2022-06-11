#include "ls_os.h"
#include "ls_stat.h"
#include "ls_status.h"
#include "ls.h"

#include <dirent.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

static LS_STATUS ls_l_file(struct stat *s, struct ls_stat *ls)
{
    LS_STATUS rv = LS_STATUS_SUCCESS;

    char buf1[10], buf2[16], link[256], t = '-';
    struct passwd *u = NULL;
    struct group *g = NULL;
    struct stat s1;
    struct ls_stat ls1;

    switch (s->st_mode & __S_IFMT) {
    case __S_IFBLK:  t = '-'; break;
    case __S_IFCHR:  t = 'c'; break;
    case __S_IFDIR:  t = 'd'; break;
    case __S_IFIFO:  t = '-'; break;
    case __S_IFLNK:  t = 'l'; break;
    case __S_IFREG:  t = '-'; break;
    case __S_IFSOCK: t = '-'; break;
    default: return LS_STATUS_SYSCALL_FAILURE;
    }

    sprintf(ls->m, "%c%s", t, mode_str(s->st_mode, buf1));
    sprintf(ls->l, "%lu", s->st_nlink);
    u = getpwuid(s->st_uid); u ? sprintf(ls->u, "%s", u->pw_name) : sprintf(ls->u, "%d", s->st_uid);
    g = getgrgid(s->st_gid); g ? sprintf(ls->g, "%s", g->gr_name) : sprintf(ls->g, "%d", s->st_gid);
    sprintf(ls->s, "%ld", s->st_size);
    sprintf(ls->t, "%s", time_str(s->st_mtime, buf2));

    switch (s->st_mode & __S_IFMT) {
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
        if (stat(link, &s1) != 0) { rv = errno; break; }
        ls1.n = link;
        rv = ls_l_file(&s1, &ls1);
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

static LS_STATUS ls_dir_l_do(LS_OS_DIR dir, struct ls_stat *lss, size_t count)
{
    LS_STATUS rv = LS_STATUS_FAILURE;
    int i = 0;
    const char *name = NULL;
    size_t lens[LS_STAT_NUMBER_OF_FIELDS] = { 0, 0, 0, 0, 0, 0 };
    struct stat s;
    LS_OS_DIRENT di = NULL;

    while (1) {
        rv = ls_os_dir_read(dir, &di);
        if (rv == LS_STATUS_DIR_END) { break; }
        if (!LS_STATUS_OK(rv)) { return rv; }

        name = ls_os_dirent_name(di);
        if (name[0] == '.') {
            continue;
        }

        errno = 0;
        if (0 != (lstat(name, &s))) {
            return errno;
        }

        lss[i].n = name;
        rv = ls_l_file(&s, &(lss[i]));
        if (!LS_STATUS_OK(rv)) {
            return rv;
        }

        ls_stat_lens(&(lss[i]), lens);

        i++;
    }

    qsort(lss, count, sizeof(struct ls_stat), ls_stat_compare);

    for (i = 0; i < count; i++) {
        ls_stat_print(&(lss[i]), lens);
    }

    return LS_STATUS_SUCCESS;
}

static LS_STATUS ls_dir_counter(LS_OS_DIRENT di, void *ctx)
{
    const char *name = ls_os_dirent_name(di);
    if (di == NULL) { return LS_STATUS_FAILURE; }
    if (name[0] != '.') { (*((size_t *)ctx))++; }
    return LS_STATUS_SUCCESS;
}

static LS_STATUS ls_dir_l(LS_OS_DIR dir)
{
    LS_STATUS rv = LS_STATUS_FAILURE;
    size_t count = 0;
    struct ls_stat *lss = NULL;
    
    rv = ls_os_dir_iterate(dir, ls_dir_counter, &count);
    if (rv != LS_STATUS_SUCCESS) {
        return rv;
    }

    lss = malloc(sizeof(struct ls_stat) * count);
    if (lss == NULL) {
        return LS_STATUS_NO_MEMORY;
    }

    rv = ls_dir_l_do(dir, lss, count);

    if (lss != NULL) {
        free(lss);
    }

    return rv;
}

static LS_STATUS ls_dir(const char *path)
{
    LS_STATUS rv = LS_STATUS_FAILURE;
    LS_OS_DIR dir = NULL;

    rv = ls_os_dir_open(path, &dir);
    if (rv == LS_STATUS_SUCCESS) {
        rv = ls_os_chdir(path);
        if (rv == LS_STATUS_SUCCESS) {
            rv = ls_dir_l(dir);
        }
    }

    (void)closedir(dir);    /*  ingoring result because who cares   */

    return rv;
}

static LS_STATUS ls_is_dir(const char *path)
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


static LS_STATUS ls_file(const char *path)
{
    static const size_t lens[LS_STAT_NUMBER_OF_FIELDS] = { 0, 0, 0, 0, 0, 0 };

    LS_STATUS rv = LS_STATUS_FAILURE;
    struct stat s;
    struct ls_stat ls;

    errno = 0;
    if (lstat(path, &s) != 0) {
        return errno;
    }

    ls.n = path;
    rv = ls_l_file(&s, &ls);
    if (rv == LS_STATUS_SUCCESS) {
        ls_stat_print(&ls, lens);
    }

    return rv;
}

LS_STATUS ls(const char *path)
{
    LS_STATUS rv;

    if (path == NULL) {
        return LS_STATUS_ARGS_INVALID;
    }
    
    rv = ls_is_dir(path);
    switch (rv) {
    case LS_STATUS_DIR:    rv = ls_dir(path);  break;
    case LS_STATUS_FILE:   rv = ls_file(path); break;
    }

    return rv;
}
