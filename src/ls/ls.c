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
static char *time_str(__time_t time, char output[20])
{
    struct tm *ti = localtime(&time);
    if (ti != NULL) {
        snprintf(output, 20, "%s %2d %.2d:%.2d", months[ti->tm_mon - 1], ti->tm_mday, ti->tm_hour, ti->tm_min);
    }
    return output;
}

static LS_STATUS ls_l_file(struct stat *s, struct ls_stat *ls)
{
    LS_STATUS rv = LS_STATUS_SUCCESS;

    char buf1[10], buf2[20], link[256], t = '-';
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
    }

    return rv;
}

static LS_STATUS ls_l(DIR *dir)
{
    LS_STATUS rv = LS_STATUS_FAILURE;
    int i = 0;
    unsigned long count = 0;
    size_t lens[LS_STAT_NUMBER_OF_FIELDS] = { 0, 0, 0, 0, 0, 0 };
    struct stat s;
    struct dirent *di = NULL;
    struct ls_stat *lss = NULL;

    errno = 0;
    while (NULL != (di = readdir(dir))) {
        if (di->d_name[0] != '.') {
            count++;
        }
    }
    if (errno != 0) {
        return errno;
    }

    lss = malloc(sizeof(struct ls_stat) * count);
    if (lss == NULL) {
        return LS_STATUS_NO_MEMORY;
    }

    rewinddir(dir);
    errno = 0;
    while (NULL != (di = readdir(dir))) {
        if (di->d_name[0] == '.') {
            continue;
        }

        errno = 0;
        if (0 != (lstat(di->d_name, &s))) {
            rv = errno;
            goto exit;
        }

        lss[i].n = di->d_name;
        rv = ls_l_file(&s, &(lss[i]));
        if (rv != LS_STATUS_SUCCESS) {
            goto exit;
        }

        ls_stat_lens(&(lss[i]), lens);

        i++;
    }
    if (errno != 0) {
        rv = errno;
        goto exit;
    }

    qsort(lss, count, sizeof(struct ls_stat), ls_stat_compare);

    for (i = 0; i < count; i++) {
        ls_stat_print(&(lss[i]), lens);
    }

 exit:
    if (lss != NULL) {
        free(lss);
    }

    return rv;
}

static LS_STATUS ls_dir(const char *path)
{
    int rv = LS_STATUS_FAILURE;
    DIR *dir = NULL;

    errno = 0;
    dir = opendir(path);
    if (dir == NULL) {
        rv = errno;
        goto exit;
    }

    rv = ls_os_chdir(path);
    if (rv == LS_STATUS_SUCCESS) {
        rv = ls_l(dir);
    }
 exit:
    if (dir != NULL) {
        closedir(dir);
    }

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
