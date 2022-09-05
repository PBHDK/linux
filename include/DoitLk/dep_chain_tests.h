
// for timekeeping
#include <linux/timekeeper_internal.h>
#include "../kernel/time/timekeeping_internal.h"
#include <linux/seqlock.h>

#define MAX 1

// global declarations
static int *x, *y, *z;
static int arr[100];
// implicitly convert arr to int*
static int **foo = (int **)&arr;
static int **bar;
static int **baz;

extern unsigned raw_read_seqcount_latch(const seqcount_latch_t *s);
extern u64 timekeeping_delta_to_ns(const struct tk_read_base *tkr, u64 delta);
extern u64 tk_clock_read(const struct tk_read_base *tkr);
extern u64 dummy_clock_read(struct clocksource *cs);