#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

// for timekeeping
#include <linux/timekeeper_internal.h>
#include "../kernel/time/timekeeping_internal.h"
#include <linux/seqlock.h>

MODULE_DESCRIPTION("Kernel module, which contains several dependeny chains used for testing CustomMemDep passes");
MODULE_AUTHOR("Paul Heidekruger");
MODULE_LICENSE("GPL");

#define MAX 1

// global declarations
static int x, y, z;
static int arr[50];
// implicitly convert arr to int*
static volatile int *foo = arr;
static volatile int *xp, *bar;

extern unsigned raw_read_seqcount_latch(const seqcount_latch_t *s);
extern u64 timekeeping_delta_to_ns(const struct tk_read_base *tkr, u64 delta);
extern u64 tk_clock_read(const struct tk_read_base *tkr);
extern u64 dummy_clock_read(struct clocksource *cs);

// Begin addr dep 1: address dependency within the same function - for breaking the begin annotation
static int noinline doitlk_rr_addr_dep_begin_1(void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];

	// End address dependency
	// y == x[42] == 0
	y = READ_ONCE(*bar);

	return 0;
}

// End addr dep 1: address dependency within the same function - for breaking the end annotation
static int noinline doitlk_rr_addr_dep_end_1(void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];

	// End address dependency
	// y == x[42] == 0
	y = READ_ONCE(*bar);

	return 0;
}

// Begin addr dep 2: address dependency accross two function. Dep begins in first function - for breaking begin annotation
static void noinline begin_2_helper(volatile int *local_bar) {
	// End address dependency
	// y == x[42] == 0
	y = READ_ONCE(*local_bar);
}

static int noinline doitlk_rr_addr_dep_begin_2 (void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];

  // copy bar into local var and dereference it in call
  begin_2_helper(bar);

	return 0;
}

// End addr dep 2: address dependency accross two function. Dep begins in first function - for breaking end annotation
static void noinline doitlk_rr_addr_dep_end_2_helper(volatile int *local_bar) {
	// End address dependency
	// y == x[42] == 0
	y = READ_ONCE(*local_bar);
}

static int noinline rr_addr_dep_end_2(void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];

  // copy bar into local var and dereference it in call
  doitlk_rr_addr_dep_end_2_helper(bar);

	return 0;
}

// Begin addr dep 3: address dependency accross two function. Dep begins in second function - for breaking beginn annotation
static volatile int* noinline doitlk_rr_addr_dep_begin_3_helper(void) {
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];

  // copy bar and return it
  return bar;
}

static int noinline rr_addr_dep_begin_3 (void)
{
  // End address dependency
	// y == x[42] == 0
	y = READ_ONCE(*doitlk_rr_addr_dep_begin_3_helper());

  // alternative
  // const volatile int* yLocal = dep_2_begin_second_helper();
  // y = READ_ONCE(yLocal);

	return 0;
}

// End addr dep 3: address dependency accross two function. Dep begins in second function - for breaking end annotation
static volatile int* noinline end_3_helper(void) {
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];

  // copy bar and return it
  return bar;
}

static int noinline doitlk_rr_addr_dep_end_3 (void)
{
  // End address dependency
	// y == x[42] == 0
	y = READ_ONCE(*end_3_helper());

  // alternative
  // const volatile int* yLocal = dep_2_begin_second_helper();
  // y = READ_ONCE(yLocal);

	return 0;
}

// Begin addr dep 4: address dependency accross two function. Dep begins in first functions, runs through second function and ends in first function
static volatile int* noinline begin_4_helper(volatile int *xpLocal) {
	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xpLocal[42];

  // copy bar and return it
  return bar;
}

static int noinline doitlk_rr_addr_dep_begin_4 (void)
{
  const volatile int *barLocal;

  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

  barLocal = begin_4_helper(xp);

  // End address dependency
	// y == x[42] == 0
	y = READ_ONCE(*barLocal);

	return 0;
}

// End addr dep 4: address dependency accross two function. Dep begins in first functions, runs through second function and ends in first function
static volatile int* noinline end_4_helper(volatile int *xpLocal) {
	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xpLocal[42];

  // copy bar and return it
  return bar;
}

static int noinline doitlk_rr_addr_dep_end_4 (void)
{
  const volatile int *barLocal;

  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

  barLocal = end_4_helper(xp);

  // End address dependency
	// y == x[42] == 0
	y = READ_ONCE(*barLocal);

	return 0;
}

// Begin addr dep 5: address dependency within the same function - for breaking the begin annotation
static int noinline doitlk_rr_addr_dep_begin_5(void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];

	// End address dependency
	// y == x[42] == 0
	if((y = READ_ONCE(*bar)))
		bar = &xp[21];
	else bar = &xp[0];

	return 0;
}

// End addr dep 5: address dependency within the same function - for breaking the end annotation
static int noinline doitlk_rr_addr_dep_end_5(void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];

	// End address dependency
	// y == x[42] == 0
	if((y = READ_ONCE(*bar)))
		bar = &xp[21];
	else bar = &xp[0];

	return 0;
}

// Begin addr dep 6: address dependency within the same function - for breaking the begin annotation
static int noinline doitlk_rr_addr_dep_begin_6(void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];

	if(&xp[0])
		bar = &xp[21];
	else bar = &xp[0];

	// End address dependency
	y = READ_ONCE(*bar);

	return 0;
}

// End addr dep 6: address dependency within the same function - for breaking the end annotation
static int noinline doitlk_rr_addr_dep_end_6(void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];

	if(&xp[0])
		bar = &xp[21];
	else bar = &xp[0];

	// End address dependency
	y = READ_ONCE(*bar);

	return 0;
}

// Begin addr dep 7: address dependency within the same function - for breaking the begin annotation
static int noinline doitlk_rr_addr_dep_begin_7(void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	if(&xp[0])
		bar = &xp[42];

	// End address dependency
	y = READ_ONCE(*bar);

	return 0;
}

// End addr dep 7: address dependency within the same function - for breaking the end annotation
static int noinline doitlk_rr_addr_dep_end_7(void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	if(&xp[0])
		bar = &xp[42];

	// End address dependency
	y = READ_ONCE(*bar);

	return 0;
}

// Begin addr dep 8: two address dependencies with same beginning within the same function - for breaking the begin annotation
static int noinline doitlk_rr_addr_dep_begin_8(void)
{
	volatile int *barLocal;
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];
	barLocal = &xp[21];

	// End address dependency
	// y == x[42] == 0
	y = READ_ONCE(*bar);
	z = READ_ONCE(*barLocal);

	return 0;
}

// End addr dep 8: two address dependencies with same beginning within the same function - for breaking the end annotation
static int noinline doitlk_rr_addr_dep_end_8(void)
{
	volatile int *barLocal;
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];
	barLocal = &xp[21];

	// End address dependency
	// y == x[42] == 0
	y = READ_ONCE(*bar);
	z = READ_ONCE(*barLocal);

	return 0;
}

static void noinline rr_begin_9_helper1(volatile int *local_bar1) {
	// End address dependency
	y = READ_ONCE(*local_bar1);
}

static void noinline rr_begin_9_helper2(volatile int *local_bar2) {
	// End address dependency
	z = READ_ONCE(*local_bar2);
}

// Begin addr dep 9: two address dependencies with same beginning within the same function - for breaking the begin annotation
static int noinline doitlk_rr_addr_dep_begin_9(void)
{
	volatile int *barLocal;
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];
	barLocal = &xp[21];

	rr_begin_9_helper1(bar);
	rr_begin_9_helper2(barLocal);

	return 0;
}

// End addr dep 9: two address dependencies with same beginning within the same function - for breaking the end annotation
static void noinline doitlk_rr_addr_dep_end_9_helper1(volatile int *local_bar1) {
	// End address dependency
	y = READ_ONCE(*local_bar1);
}

static void noinline doitlk_rr_addr_dep_end_9_helper2(volatile int *local_bar2) {
	// End address dependency
	z = READ_ONCE(*local_bar2);
}

// Begin addr dep 9: two address dependencies with same beginning within the same function - for breaking the begin annotation
static int noinline rr_addr_dep_end_9(void)
{
	volatile int *barLocal;
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];
	barLocal = &xp[21];

	doitlk_rr_addr_dep_end_9_helper1(bar);
	doitlk_rr_addr_dep_end_9_helper2(barLocal);

	return 0;
}

// Begin addr dep 10: address dependencies through function call, but different chains
static volatile int* noinline begin_10_helper(volatile int *xpLocal) {
	volatile int *barLocal;
	
	bar = &xpLocal[42];

	// End address dependency
	y = READ_ONCE(*bar);

	// Begin address dependency
	xp = READ_ONCE(foo);	

	barLocal = &xp[42];

  // copy bar and return it
  return barLocal;
}

static int noinline doitlk_rr_addr_dep_begin_10 (void)
{
  volatile int *barLocal;

  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

  barLocal = begin_10_helper(xp);

	z = READ_ONCE(*barLocal);

	return 0;
}

// End addr dep 10: address dependencies through function call, but different chains
static volatile int* noinline end_10_helper(volatile int *xpLocal) {
	volatile int *barLocal;

	bar = &xpLocal[42];

	// End address dependency
	y = READ_ONCE(*bar);

	// Begin address dependency
	xp = READ_ONCE(foo);	

	barLocal = &xp[42];

  // copy bar and return it
  return barLocal;
}

static int noinline doitlk_rr_addr_dep_end_10 (void)
{
  volatile int *barLocal;

  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

  barLocal = end_10_helper(xp);

  // End address dependency
	// y == x[42] == 0
	z = READ_ONCE(*barLocal);

	return 0;
}

// Example from original DoitLK talk at LPC 2020
struct tk_fast {
	seqcount_latch_t	seq;
	struct tk_read_base	base[2];
};

static __always_inline u64 doitlk_ktime(struct tk_fast *tkf)
{
	struct tk_read_base *tkr;
	unsigned int seq;
	u64 now;

	do {
		seq = raw_read_seqcount_latch(&tkf->seq);
		tkr = tkf->base + (seq & 0x01);
		now = ktime_to_ns(READ_ONCE(tkr->base));

		now += timekeeping_delta_to_ns(tkr,
				clocksource_delta(
					tk_clock_read(tkr),
					tkr->cycle_last,
					tkr->mask));
	} while (read_seqcount_latch_retry(&tkf->seq, seq));

	return now;
}

// 
// ====
// Read -> Write Address Dependencies
// ====
// 

// Begin addr dep 1: rw address dependency within the same function - for breaking the begin annotation
static int noinline doitlk_rw_addr_dep_begin_1(void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];

	// End address dependency
	// y == x[42] == 0
	WRITE_ONCE(*bar, y);

	return 0;
}

// End addr dep 1: rw address dependency within the same function - for breaking the end annotation
static int noinline doitlk_rw_addr_dep_end_1(void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];

	// End address dependency
	// y == x[42] == 0
	WRITE_ONCE(*bar, y);

	return 0;
}

// Begin addr dep 2: address dependency accross two function. Dep begins in first function - for breaking begin annotation
static void noinline rw_begin_2_helper(volatile int *local_bar) {
	// End address dependency
	// y == x[42] == 0
	WRITE_ONCE(*local_bar, y);
}

static int noinline doitlk_rw_addr_dep_begin_2 (void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];

  // copy bar into local var and dereference it in call
  rw_begin_2_helper(bar);

	return 0;
}

// End addr dep 2: address dependency accross two function. Dep begins in first function - for breaking end annotation
static void noinline doitlk_rw_addr_dep_end_2_helper(volatile int *local_bar) {
	// End address dependency
	// y == x[42] == 0
	WRITE_ONCE(*local_bar, y);
}

static int noinline rw_addr_dep_end_2(void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];

  // copy bar into local var and dereference it in call
  doitlk_rw_addr_dep_end_2_helper(bar);

	return 0;
}

// Begin addr dep 3: address dependency accross two function. Dep begins in second function - for breaking beginn annotation
static int noinline rw_addr_dep_begin_3 (void)
{
	volatile int* barLocal = doitlk_rr_addr_dep_begin_3_helper();

  // End address dependency
	// y == x[42] == 0
	WRITE_ONCE(*barLocal, y);

  // alternative
  // const volatile int* yLocal = dep_2_begin_second_helper();
  // y = READ_ONCE(yLocal);

	return 0;
}

// End addr dep 3: address dependency accross two function. Dep begins in second function - for breaking end annotation
static int noinline doitlk_rw_addr_dep_end_3 (void)
{
  // End address dependency
	// y == x[42] == 0
	WRITE_ONCE(*end_3_helper(), y);

  // alternative
  // const volatile int* yLocal = dep_2_begin_second_helper();
  // y = READ_ONCE(yLocal);

	return 0;
}

// Begin addr dep 4: address dependency accross two function. Dep begins in first functions, runs through second function and ends in first function
static int noinline doitlk_rw_addr_dep_begin_4 (void)
{
  volatile int *barLocal;

  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

  barLocal = begin_4_helper(xp);

  // End address dependency
	// y == x[42] == 0
	WRITE_ONCE(*barLocal, y);

	return 0;
}

// End addr dep 4: address dependency accross two function. Dep begins in first functions, runs through second function and ends in first function
static int noinline doitlk_rw_addr_dep_end_4 (void)
{
  volatile int *barLocal;

  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

  barLocal = end_4_helper(xp);

  // End address dependency
	// y == x[42] == 0
	WRITE_ONCE(*barLocal, y);

	return 0;
}

// Begin addr dep 6: address dependency within the same function - for breaking the begin annotation
static int noinline doitlk_rw_addr_dep_begin_6(void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];

	if(&xp[0])
		bar = &xp[21];
	else bar = &xp[0];

	// End address dependency
	WRITE_ONCE(*bar, y);

	return 0;
}

// End addr dep 6: address dependency within the same function - for breaking the end annotation
static int noinline doitlk_rw_addr_dep_end_6(void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];

	if(&xp[0])
		bar = &xp[21];
	else bar = &xp[0];

	// End address dependency
	WRITE_ONCE(*bar, y);

	return 0;
}

// Begin addr dep 7: address dependency within the same function - for breaking the begin annotation
static int noinline doitlk_rw_addr_dep_begin_7(void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	if(&xp[0])
		bar = &xp[42];

	// End address dependency
	WRITE_ONCE(*bar, y);

	return 0;
}

// End addr dep 7: address dependency within the same function - for breaking the end annotation
static int noinline doitlk_rw_addr_dep_end_7(void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	if(&xp[0])
		bar = &xp[42];

	// End address dependency
	WRITE_ONCE(*bar, y);

	return 0;
}

// Begin addr dep 8: two address dependencies with same beginning within the same function - for breaking the begin annotation
static int noinline doitlk_rw_addr_dep_begin_8(void)
{
	volatile int *barLocal;
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];
	barLocal = &xp[21];

	// End address dependency
	// y == x[42] == 0
	WRITE_ONCE(*bar, y);
	WRITE_ONCE(*barLocal, y);

	return 0;
}

// End addr dep 8: two address dependencies with same beginning within the same function - for breaking the end annotation
static int noinline doitlk_rw_addr_dep_end_8(void)
{
	volatile int *barLocal;
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];
	barLocal = &xp[21];

	// End address dependency
	// y == x[42] == 0
	WRITE_ONCE(*bar, y);
	WRITE_ONCE(*barLocal, y);

	return 0;
}

static void noinline rw_begin_9_helper1(volatile int *local_bar1) {
	// End address dependency
	WRITE_ONCE(*local_bar1, y);
}

static void noinline rw_begin_9_helper2(volatile int *local_bar2) {
	// End address dependency
	WRITE_ONCE(*local_bar2, z);
}

// Begin addr dep 9: two address dependencies with same beginning within the same function - for breaking the begin annotation
static int noinline doitlk_rw_addr_dep_begin_9(void)
{
	volatile int *barLocal1, *barLocal2;
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	barLocal1 = &xp[42];
	barLocal2 = &xp[21];

	rw_begin_9_helper1(barLocal1);
	rw_begin_9_helper2(barLocal2);

	return 0;
}

// End addr dep 9: two address dependencies with same beginning within the same function - for breaking the end annotation
static void noinline doitlk_rw_addr_dep_end_9_helper1(volatile int *local_bar1) {
	// End address dependency
	WRITE_ONCE(*local_bar1, y);
}

static void noinline doitlk_rw_addr_dep_end_9_helper2(volatile int *local_bar2) {
	// End address dependency
	WRITE_ONCE(*local_bar2, z);
}

// Begin addr dep 9: two address dependencies with same beginning within the same function - for breaking the begin annotation
static int noinline rw_addr_dep_end_9(void)
{
	volatile int *barLocal1, *barLocal2;
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	barLocal1 = &xp[42];
	barLocal2 = &xp[21];

	doitlk_rw_addr_dep_end_9_helper1(barLocal1);
	doitlk_rw_addr_dep_end_9_helper2(barLocal2);

	return 0;
}

// Begin addr dep 10: address dependencies through function call, but different chains
static volatile int* noinline rw_begin_10_helper(volatile int *xpLocal) {
	volatile int *barLocal;
	
	bar = &xpLocal[42];

	// End address dependency
	WRITE_ONCE(*bar, y);

	// Begin address dependency
	xp = READ_ONCE(foo);	

	barLocal = &xp[42];

  // copy bar and return it
  return barLocal;
}

static int noinline doitlk_rw_addr_dep_begin_10 (void)
{
  volatile int *barLocal;

  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

  barLocal = rw_begin_10_helper(xp);

	WRITE_ONCE(*barLocal, z);

	return 0;
}

// End addr dep 10: address dependencies through function call, but different chains
static volatile int* noinline rw_end_10_helper(volatile int *xpLocal) {
	volatile int *barLocal;
	
	bar = &xpLocal[42];

	// End address dependency
	WRITE_ONCE(*bar, y);

	// Begin address dependency
	xp = READ_ONCE(foo);	

	barLocal = &xp[42];

  // copy bar and return it
  return barLocal;
}

static int noinline doitlk_rw_addr_dep_end_10 (void)
{
  volatile int *barLocal;

  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

  barLocal = rw_end_10_helper(xp);

  // End address dependency
	// y == x[42] == 0
	WRITE_ONCE(*barLocal, z);

	return 0;
}

// Begin ctrl dep 1: memory-barriers.txt case 1, control dependency within the same function - dependent - for breaking the begin annotation
static int noinline doitlk_ctrl_dep_begin_1(void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	if(xp) {
		// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
		bar = &xp[42];

		// End address dependency
		// y == x[42] == 0
		WRITE_ONCE(y, *bar);
	}
	return 0;
}

// End ctrl dep 1: control dependency within the same function - dependent condition - for breaking the end annotation
static int noinline doitlk_ctrl_dep_end_1(void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	if(xp) {
		// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
		bar = &xp[42];

		// End address dependency
		// y == x[42] == 0
		WRITE_ONCE(y, *bar);
	}
	return 0;
}

// Begin ctrl dep 2: control dependency within the same function - dependent condition - for breaking the begin annotation
static int noinline doitlk_ctrl_dep_begin_2(void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	if(foo) {
		// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
		bar = &xp[42];

		// End address dependency
		// y == x[42] == 0
		WRITE_ONCE(y, *bar);
	}
	return 0;
}

// End ctrl dep 2: control dependency within the same function - dependent condition - for breaking the end annotation
static int noinline doitlk_ctrl_dep_end_2(void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	if(foo) {
		// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
		bar = &xp[42];

		// End address dependency
		// y == x[42] == 0
		WRITE_ONCE(y, *bar);
	}
	return 0;
}

// Begin ctrl dep 3: control dependency with dead branch within the same function -  for breaking the begin annotation
static int noinline doitlk_ctrl_dep_begin_3(void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	if(0) {
		// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
		bar = &xp[42];

		// End address dependency
		// y == x[42] == 0
		WRITE_ONCE(y, *bar);
	}
	return 0;
}

// End ctrl dep 3: control dependency with dead branch within the same function -  for breaking the begin annotation
static int noinline doitlk_ctrl_dep_end_3(void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	if(0) {
		// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
		bar = &xp[42];

		// End address dependency
		// y == x[42] == 0
		WRITE_ONCE(y, *bar);
	}
	return 0;
}

// Begin ctrl dep 4: control dependency with dead branch within the same function -  for breaking the begin annotation
static int noinline doitlk_ctrl_dep_begin_4(void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	if((xp = READ_ONCE(foo))) {
		// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
		bar = &xp[42];

		// End address dependency
		// y == x[42] == 0
		WRITE_ONCE(y, *bar);
	}
	return 0;
}

// End ctrl dep 4: control dependency with dead branch within the same function -  for breaking the begin annotation
static int noinline doitlk_ctrl_dep_end_4(void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	if((xp = READ_ONCE(foo))) {
		// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
		bar = &xp[42];

		// End address dependency
		// y == x[42] == 0
		WRITE_ONCE(y, *bar);
	}
	return 0;
}

// Begin ctrl dep 5: control dependency with end in for loop -  for breaking the begin annotation
static int noinline doitlk_ctrl_dep_begin_5(void)
{
	int i = 0;
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	for(; i < 42; ++i) {
		// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
		bar = &xp[i];

		// End address dependency
		// y == x[42] == 0
		WRITE_ONCE(y, *bar);
	}
	return 0;
}

// End ctrl dep 5: control dependency with end in for loop -  for breaking the begin annotation
static int noinline doitlk_ctrl_dep_end_5(void)
{
	int i = 0;
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	for(; i < 42; ++i) {
		// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
		bar = &xp[i];

		// End address dependency
		// y == x[42] == 0
		WRITE_ONCE(y, *bar);
	}
	return 0;
}

// Begin ctrl dep 6: memory-barriers.txt case 2, constant dependent condition
static int noinline doitlk_ctrl_dep_begin_6(void)
{
	x = READ_ONCE(*foo);
	if(x % MAX == 0) {
		WRITE_ONCE(*bar, y);
	}
	return 0;
}

// End ctrl dep 6: memory-barriers.txt case 2, constant dependent condition
static int noinline doitlk_ctrl_dep_end_6(void)
{
	x = READ_ONCE(*foo);
	if(x % MAX == 0) {
		WRITE_ONCE(*bar, y);
	}
	return 0;
}

// Begin ctrl dep 7: memory-barriers.txt case 3, constant dependent condition
static int noinline doitlk_ctrl_dep_begin_7(void)
{
	x = READ_ONCE(*foo);
	if (x || 1 > 0)
		WRITE_ONCE(y, 1);
	return 0;
}

// End ctrl dep 7: memory-barriers.txt case 3, constant dependent condition
static int noinline doitlk_ctrl_dep_end_7(void)
{
	x = READ_ONCE(*foo);
	if (x || 1 > 0)
		WRITE_ONCE(y, 1);
	return 0;
}

// Begin ctrl dep 8: constant dependent condition
static int noinline doitlk_ctrl_dep_begin_8(void)
{
	x = READ_ONCE(*foo);
	if (x > 0) {
		if(x > 42)
			WRITE_ONCE(y, 42);
		else 
			WRITE_ONCE(y, 0);
	}
	return 0;
}

// End ctrl dep 8: constant dependent condition
static int noinline doitlk_ctrl_dep_end_8(void)
{
	x = READ_ONCE(*foo);
	if (x > 0) {
		if(x > 42)
			WRITE_ONCE(y, 42);
		else 
			WRITE_ONCE(y, 0);
	}
	return 0;
}

// Begin ctrl dep 9: two dep chains in condition
static int noinline doitlk_ctrl_dep_begin_9(void)
{
	x = READ_ONCE(*foo);
	y = READ_ONCE(*bar);
	if (x > y)
		WRITE_ONCE(z, 1);
	return 0;
}

// End ctrl dep 9: two dep chains in condition
static int noinline doitlk_ctrl_dep_end_9(void)
{
	x = READ_ONCE(*foo);
	y = READ_ONCE(*bar);
	if (x > y)
		WRITE_ONCE(z, 1);
	return 0;
}

// Begin ctrl dep 10: three dep chains in condition
static int noinline doitlk_ctrl_dep_begin_10(void)
{
	x = READ_ONCE(*foo);
	y = READ_ONCE(*bar);
	z = READ_ONCE(*xp);
	if (x > y || z < y)
		WRITE_ONCE(*foo, 1);
	return 0;
}

// End ctrl dep 10: three dep chains in condition
static int noinline doitlk_ctrl_dep_end_10(void)
{
	x = READ_ONCE(*foo);
	y = READ_ONCE(*bar);
	z = READ_ONCE(*xp);
	if (x > y || z < y)
		WRITE_ONCE(*foo, 1);
	return 0;
}

// Begin ctrl dep 11: nested if
static int noinline doitlk_ctrl_dep_begin_11(void)
{
	x = READ_ONCE(*foo);
	if(x) {
		y = READ_ONCE(*bar);
		if(y)
			WRITE_ONCE(*foo, 1);
	}
	return 0;
}

// End ctrl dep 11: nested if
static int noinline doitlk_ctrl_dep_end_11(void)
{
	x = READ_ONCE(*foo);
	if(x) {
		y = READ_ONCE(*bar);
		if(y)
			WRITE_ONCE(*foo, 1);
	}
	return 0;
}

static int lkm_init(void)
{
	static struct clocksource dummy_clock = {
		.read = dummy_clock_read,
	};

	#define FAST_TK_INIT						\
	{							\
		.clock		= &dummy_clock,			\
		.mask		= CLOCKSOURCE_MASK(64),		\
		.mult		= 1,				\
		.shift		= 0,				\
	}

	static struct tk_fast tk_fast_raw  ____cacheline_aligned = {
		.seq     = SEQCNT_LATCH_ZERO(tk_fast_raw.seq),
		.base[0] = FAST_TK_INIT,
		.base[1] = FAST_TK_INIT,
	};

	// rr addr deps
	// simple case
  doitlk_rr_addr_dep_begin_1();
	doitlk_rr_addr_dep_end_1();
	// in via function parameter
  doitlk_rr_addr_dep_begin_2();
	rr_addr_dep_end_2();
	// out via function return
	rr_addr_dep_begin_3();
	doitlk_rr_addr_dep_end_3();
	// in and out same chain
	doitlk_rr_addr_dep_begin_4();
	doitlk_rr_addr_dep_end_4();
	// simple case, end in if condition
	doitlk_rr_addr_dep_begin_5();
	doitlk_rr_addr_dep_end_5();
	// Simple Case - Chain Through If-Else
	doitlk_rr_addr_dep_begin_6();
	doitlk_rr_addr_dep_end_6();
	// Simple Case - Chain Through If
	doitlk_rr_addr_dep_begin_7();
	doitlk_rr_addr_dep_end_7();
	// simple case, fan out
	doitlk_rr_addr_dep_begin_8();
	doitlk_rr_addr_dep_end_8();
	// dep chain fanning out
	doitlk_rr_addr_dep_begin_9();
	rr_addr_dep_end_9();
	// in and out, but different chains 
	doitlk_rr_addr_dep_begin_10();
	doitlk_rr_addr_dep_end_10();
	// chain fanning in not relevant
	// doitlk example
	doitlk_ktime(&tk_fast_raw);

	// rw addr deps
	// simple case
	doitlk_rw_addr_dep_begin_1();
	doitlk_rw_addr_dep_end_1();
	// in via function parameter
	doitlk_rw_addr_dep_begin_2();
	rw_addr_dep_end_2();
	// out via function return
	rw_addr_dep_begin_3();
	doitlk_rw_addr_dep_end_3();
	// in and out same chain
	doitlk_rw_addr_dep_begin_4();
	doitlk_rw_addr_dep_end_4();
	// dep 5 ommitted since we can't use WRITE_ONCE() as if condition
	// Simple Case - Chain Through If-Else
	doitlk_rw_addr_dep_begin_6();
	doitlk_rw_addr_dep_end_6();
	// Simple Case - Chain Through If
	doitlk_rw_addr_dep_begin_7();
	doitlk_rw_addr_dep_end_7();
	// simple case, fan out
	doitlk_rw_addr_dep_begin_8();
	doitlk_rw_addr_dep_end_8();
	// dep chain fanning out
	doitlk_rw_addr_dep_begin_9();
	rw_addr_dep_end_9();
	// in and out, but different chains 
	doitlk_rw_addr_dep_begin_10();
	doitlk_rw_addr_dep_end_10();

	// ctrl deps
	doitlk_ctrl_dep_begin_1();
	doitlk_ctrl_dep_end_1();
	doitlk_ctrl_dep_begin_2();
	doitlk_ctrl_dep_end_2();
	doitlk_ctrl_dep_begin_3();
	doitlk_ctrl_dep_end_3();
	doitlk_ctrl_dep_begin_4();
	doitlk_ctrl_dep_end_4();
	doitlk_ctrl_dep_begin_5();
	doitlk_ctrl_dep_end_5();
	doitlk_ctrl_dep_begin_6();
	doitlk_ctrl_dep_end_6();
	doitlk_ctrl_dep_begin_7();
	doitlk_ctrl_dep_end_7();
	doitlk_ctrl_dep_begin_8();
	doitlk_ctrl_dep_end_8();
	doitlk_ctrl_dep_begin_9();
	doitlk_ctrl_dep_end_9();
	doitlk_ctrl_dep_begin_10();
	doitlk_ctrl_dep_end_10();
	doitlk_ctrl_dep_begin_11();
	doitlk_ctrl_dep_end_11();
	// TODO all cases from above?
	
  return 0;
}

static void lkm_exit(void)
{
	pr_debug("Bye\n");
}



module_init(lkm_init);
module_exit(lkm_exit);