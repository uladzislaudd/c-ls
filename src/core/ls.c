#include <dirent.h>
#include <errno.h>
#include <errno.h>
#include <sys/stat.h>
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

static STATUS ls_l_file(const char *name)
{
    fprintf(stdout, "%s ", name);
    return STATUS_SUCCESS;
}

static int ls_l(DIR *dir)
{
    struct dirent *di = NULL;
    while (NULL != (di = readdir(dir))) {
        if (di->d_name[0] == '.') {
            continue;
        }

        ls_l_file(di->d_name);
    }
    fprintf(stdout, "\n");
    
    return errno;
}

static int ls_dir(const char *path)
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
    
    rv = ls_l_file(path);
    fprintf(stdout, "\n");

    return rv;
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
