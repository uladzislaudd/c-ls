#include <dirent.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>

typedef int STATUS;
enum {
    STATUS_SUCCESS = 0x0000,

    STATUS_FAILURE = -0x1000,
    STATUS_PATH_INVALID,
    STATUS_ARGS_INVALID,

    STATUS_DIR = -0x2000,
    STATUS_FILE,
};

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

static STATUS ls_l_file(struct stat *s, const char *name)
{
    char buf1[10], buf2[20], t = '-';
    struct passwd *u = NULL;
    struct group *g = NULL;

    switch (s->st_mode & __S_IFMT) {
    case __S_IFBLK:  t = '-'; break;
    case __S_IFCHR:  t = 'c'; break;
    case __S_IFDIR:  t = 'd'; break;
    case __S_IFIFO:  t = '-'; break;
    case __S_IFLNK:  t = 'l'; break;
    case __S_IFREG:  t = '-'; break;
    case __S_IFSOCK: t = '-'; break;
    }

    fprintf(stdout, "%c%s ", t, mode_str(s->st_mode, buf1));
    fprintf(stdout, "%lu ", s->st_nlink);
    u = getpwuid(s->st_uid); u ? fprintf(stdout, "%s ", u->pw_name) : fprintf(stdout, "%d", s->st_uid);
    g = getgrgid(s->st_gid); g ? fprintf(stdout, "%s ", g->gr_name) : fprintf(stdout, "%d", s->st_gid);
    fprintf(stdout, "%ld ", s->st_size);
    fprintf(stdout, "%s ", time_str(s->st_mtime, buf2));
    fprintf(stdout, "%s ", name);
    fprintf(stdout, "\n");

    return STATUS_SUCCESS;
}

static STATUS ls_l(DIR *dir)
{
    struct stat s;
    struct dirent *di = NULL;
 
    while (NULL != (di = readdir(dir))) {
        if (di->d_name[0] == '.') {
            continue;
        }
        if (0 != (lstat(di->d_name, &s))) {
            break;
        }
        if (0 != ls_l_file(&s, di->d_name)) {
            break;
        }
    }
    
    return errno;
}

static STATUS ls_dir(const char *path)
{
    int rv = STATUS_FAILURE;
    DIR *dir = NULL;

    dir = opendir(path);
    if (dir == NULL) {
        rv = STATUS_PATH_INVALID;
        goto exit;
    }

    if (chdir(path) == 0) {
        rv = ls_l(dir);
    } else {
        rv = errno;
    }
 exit:
    if (dir != NULL) {
        closedir(dir);
    }

    return rv;
}

static STATUS ls_is_dir(const char *path)
{
    STATUS rv = STATUS_FAILURE;
    struct stat path_stat;

    if (stat(path, &path_stat) == 0) {
        rv = S_ISDIR(path_stat.st_mode) ? STATUS_DIR : STATUS_FILE;
    } else {
        rv = errno;
    }
    
    return rv;
}

static STATUS ls_file(const char *path)
{
    STATUS rv = STATUS_FAILURE;
    struct stat s;

    if (lstat(path, &s) != 0) {
        return errno;
    }

    return ls_l_file(&s, path);
}

static STATUS ls(const char *path)
{
    STATUS rv;

    if (path == NULL) {
        return STATUS_ARGS_INVALID;
    }
    
    rv = ls_is_dir(path);
    switch (rv) {
    case STATUS_DIR:    rv = ls_dir(path);  break;
    case STATUS_FILE:   rv = ls_file(path); break;
    }

    return rv;
}

static int ls_error(STATUS _rv);
static void usage();
int main(int argc, char const *argv[])
{
    const char *path = NULL;
    char buf[256];

    switch (argc) {
    case 2:
        path = argv[1];
        break;

    case 1:
        path = getcwd(buf, sizeof(buf));
        break;

    default:
        usage();
        break;
    }

    return ls_error(ls(path));
}

static int ls_error(STATUS _rv)
{
    int rv = 1;

    switch (_rv) {
    case STATUS_SUCCESS:
        /*  failthrough */
    case STATUS_DIR:
        /*  failthrough */
    case STATUS_FILE:
        rv = 0;
        break;

    case STATUS_FAILURE:
        fprintf(stderr, "Unknown error!");
        usage();
        break;

    case STATUS_PATH_INVALID:
        fprintf(stderr, "Invalid path!\n");
        break;

    case STATUS_ARGS_INVALID:
        fprintf(stderr, "Invalid argument!\n");
        break;
    
    default:
        fprintf(stderr, "%s\n", strerror(_rv));
        break;
    }

    return 0;
}

static void usage()
{

}
