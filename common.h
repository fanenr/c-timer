#ifndef TIME_H
#define TIME_H

#include <assert.h>
#include <stdlib.h>
#include <time.h>

#define TIME_ST() clock_t __tm_st__ = clock ();
#define TIME_ED() clock_t __tm_ed__ = clock ();
#define TIME_VAL() ((double)(__tm_ed__ - __tm_st__) / CLOCKS_PER_SEC)

static void rand_init (void);
static char *rand_string (size_t len);
static long rand_long (long from, long to);

static inline void
rand_init (void)
{
  srand ((unsigned)time (NULL));
}

static inline long
rand_long (long from, long to)
{
  assert (from < to);
  return rand () % (to - from) + from;
}

static inline char *
rand_string (size_t len)
{
  char *ret;

  if (!(ret = (char *)malloc (len + 1)))
    return NULL;

  for (size_t i = 0; i < len; i++)
    ret[i] = (char)rand_long (32, 127);

  ret[len] = 0;
  return ret;
}

#endif
