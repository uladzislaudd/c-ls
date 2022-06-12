#include "ls_os.h"
#include "ls_stat.h"
#include "ls_status.h"
#include "ls.h"

#include <string.h>

typedef struct ls_s {
    struct ls_stat *lss;
    size_t i;
} ls_t;


static LS_STATUS ls_dir_entry_stat_filler(LS_OS_DIRENT di, void *ctx)
{
    LS_STATUS rv = LS_STATUS_FAILURE;
    const char *name = NULL;
    struct ls_s *ls;

    if (ctx == NULL) {
        return LS_STATUS_ARGS_INVALID;
    }

    name = ls_os_dirent_name(di);
    if (name[0] == '.') {
        return LS_STATUS_SUCCESS;
    }

    ls = (struct ls_s *)ctx;
    ls->lss[ls->i].n = name;
    rv = ls_os_stat(name, &(ls->lss[ls->i]));
    if (rv == LS_STATUS_SUCCESS) {
        ls->i++;
    }

    return rv;
}

static LS_STATUS ls_dir_(const char *path, struct ls_stat *lss, size_t count)
{
    LS_STATUS rv = LS_STATUS_FAILURE;
    struct ls_s ctx = { .lss = lss, .i = 0 };
    size_t lens[LS_STAT_NUMBER_OF_FIELDS] = { 0, 0, 0, 0, 0, 0 };

    rv = ls_os_chdir(path);
    if (rv != LS_STATUS_SUCCESS) {
        return rv;
    }

    rv = ls_os_dir_iterate(".", ls_dir_entry_stat_filler, &ctx);
    if (rv != LS_STATUS_SUCCESS) {
        return rv;
    }

    qsort(lss, count, sizeof(struct ls_stat), ls_stat_compare);

    for (size_t i = 0; i < count; i++) {
        ls_stat_lens(&(lss[i]), lens);
    }

    for (size_t i = 0; i < count; i++) {
        ls_stat_print(&(lss[i]), lens);
    }

    return LS_STATUS_SUCCESS;
}

static LS_STATUS ls_dir_entry_counter(LS_OS_DIRENT di, void *ctx)
{
    const char *name = ls_os_dirent_name(di);
    if (di == NULL) { return LS_STATUS_FAILURE; }
    if (name[0] != '.') { (*((size_t *)ctx))++; }
    return LS_STATUS_SUCCESS;
}

static LS_STATUS ls_dir(const char *path)
{
    LS_STATUS rv = LS_STATUS_FAILURE;
    size_t count = 0;
    struct ls_stat *lss = NULL;

    rv = ls_os_chdir(path);
    if (rv != LS_STATUS_SUCCESS) {
        return rv;
    }

    rv = ls_os_dir_iterate(".", ls_dir_entry_counter, &count);
    if (rv != LS_STATUS_SUCCESS) {
        return rv;
    }

    lss = malloc(sizeof(struct ls_stat) * count);
    if (lss == NULL) {
        return LS_STATUS_NO_MEMORY;
    }

    rv = ls_dir_(".", lss, count);

    free(lss);

    return rv;
}

static LS_STATUS ls_file(const char *path)
{
    static const size_t lens[LS_STAT_NUMBER_OF_FIELDS] = { 0, 0, 0, 0, 0, 0 };

    LS_STATUS rv = LS_STATUS_FAILURE;
    struct ls_stat ls;

    ls.n = path;
    rv = ls_os_stat(path, &ls);
    if (rv == LS_STATUS_SUCCESS) {
        ls_stat_print(&ls, lens);
    }

    return rv;
}

LS_STATUS ls(const char *path)
{
    LS_STATUS rv;
    char resolved_path[256], *basename, *dirname, buf1[256], buf2[256];

    if (path == NULL) {
        return LS_STATUS_ARGS_INVALID;
    }

    rv = ls_os_realpath(path, resolved_path);
    if (rv != LS_STATUS_SUCCESS) {
        return rv;
    }

    strncpy(buf1, resolved_path, sizeof(buf1));
    rv = ls_os_basename(buf1, &basename);
    if (rv != LS_STATUS_SUCCESS) {
        return rv;
    }

    strncpy(buf2, resolved_path, sizeof(buf2));
    rv = ls_os_dirname(buf2, &dirname);
    if (rv != LS_STATUS_SUCCESS) {
        return rv;
    }

    rv = ls_os_chdir(dirname);
    if (rv != LS_STATUS_SUCCESS) {
        return rv;
    }

    rv = ls_os_is_dir(basename);
    switch (rv) {
    case LS_STATUS_DIR:    rv = ls_dir(basename);  break;
    case LS_STATUS_FILE:   rv = ls_file(basename); break;
    }

    return rv;
}
