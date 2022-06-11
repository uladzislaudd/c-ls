#if !defined(__LS_STAT_H__)
#define __LS_STAT_H__

#include <stdlib.h>

#define LS_STAT_NUMBER_OF_FIELDS 6
typedef struct ls_stat {
    const char *n;
    char m[16];
    char l[8];
    char u[16];
    char g[16];
    char s[16];
    char t[16];
    char p[256];
} ls_stat;

int ls_stat_compare(const void *ls1, const void *ls2);

void ls_stat_print(struct ls_stat *s, const size_t lens[LS_STAT_NUMBER_OF_FIELDS]);
void ls_stat_lens(struct ls_stat *s, size_t lens[LS_STAT_NUMBER_OF_FIELDS]);

#endif // __LS_STAT_H__
