#include "ls_os.h"
#include "ls_stat.h"
#include "ls_status.h"
#include "ls.h"

static void usage();
int main(int argc, char const *argv[])
{
    LS_STATUS rv;
    const char *path = NULL;
    char buf[256];

    switch (argc) {
    case 2:
        path = argv[1];
        break;

    case 1:
        rv = ls_os_getcwd(buf, sizeof(buf));
        if (rv != LS_STATUS_SUCCESS) {
            return ls_status_print(rv);
        }
        path = buf;
        break;

    default:
        usage();
        break;
    }

    return ls_status_print(ls(path));
}

static void usage()
{

}
