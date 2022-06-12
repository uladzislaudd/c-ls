#include "ls_stat.h"

#include <stdio.h>
#include <string.h>

int ls_stat_compare(const void *ls1, const void *ls2)
{
    return strcasecmp(((struct ls_stat *)ls1)->n, ((struct ls_stat *)ls2)->n);
}

void ls_stat_print(struct ls_stat *s, const size_t lens[LS_STAT_NUMBER_OF_FIELDS])
{
    fprintf(stdout, "%*s %*s %*s %*s %*s %*s %s\n",
            (int)lens[0], s->m,
            (int)lens[1], s->l,
            (int)lens[2], s->u,
            (int)lens[3], s->g,
            (int)lens[4], s->s,
            (int)lens[5], s->t,
            s->p);
}

void ls_stat_lens(struct ls_stat *s, size_t lens[LS_STAT_NUMBER_OF_FIELDS])
{
    size_t n;
    n = strlen(s->m); if (n > lens[0]) { lens[0] = n ; }
    n = strlen(s->l); if (n > lens[1]) { lens[1] = n ; }
    n = strlen(s->u); if (n > lens[2]) { lens[2] = n ; }
    n = strlen(s->g); if (n > lens[3]) { lens[3] = n ; }
    n = strlen(s->s); if (n > lens[4]) { lens[4] = n ; }
    n = strlen(s->t); if (n > lens[5]) { lens[5] = n ; }
}
