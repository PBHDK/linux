#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <DoitLk/dep_chain_tests.h>

MODULE_DESCRIPTION(
	"Kernel module, which contains several dependeny chains used for testing CustomMemDep passes");
MODULE_AUTHOR("Paul Heidekruger");
MODULE_LICENSE("GPL");

/**
 * Naming scheme: doitlk_(rr|rw)_(addr|ctrl)_(begin|end)_${test_name}
 */

/**
 * Test Cases:
 * Address Dependency:
 *	Test cases:
 *	Inside ctrl dep
 * Control Dependency:
 */

/* BUGs: 1 */
static noinline int doitlk_rr_addr_dep_begin_simple(void)
{
	volatile int *r1;
	volatile int *r2;
	volatile int r3;

	r1 = READ_ONCE(*foo);

	r2 = &r1[42];

	r3 = READ_ONCE(*r2);

	return r3;
}

/* BUGs: 1 */
static noinline int doitlk_rr_addr_dep_end_simple(void)
{
	volatile int *r1;
	volatile int *r2;
	volatile int r3;

	r1 = READ_ONCE(*foo);

	r2 = &r1[42];

	r3 = READ_ONCE(*r2);

	return r3;
}

static noinline void rr_addr_dep_begin_call_ending_helper(volatile int *r2)
{
	volatile int r3;

	r3 = READ_ONCE(*r2);
}

/* BUGs: 1 */
static noinline int doitlk_rr_addr_dep_begin_call_ending(void)
{
	volatile int *r1;
	volatile int *r2;

	r1 = READ_ONCE(*foo);

	r2 = &r1[42];

	rr_addr_dep_begin_call_ending_helper(r2);

	return 0;
}

static noinline void doitlk_rr_addr_dep_end_call_ending_helper(volatile int *r2)
{
	volatile int r3;

	r3 = READ_ONCE(*r2);
}

/* BUGs: 1*/
static noinline int rr_addr_dep_end_call_ending(void)
{
	volatile int *r1;
	volatile int *r2;

	r1 = READ_ONCE(*foo);

	r2 = &r1[42];

	doitlk_rr_addr_dep_end_call_ending_helper(r2);

	return 0;
}

static volatile noinline int *
doitlk_rr_addr_dep_begin_call_beginning_helper(void)
{
	volatile int *r1;
	volatile int *r2;

	r1 = READ_ONCE(*foo);

	r2 = &r1[42];

	return r2;
}

/* BUGs: 1 */
static noinline int rr_addr_dep_begin_call_beginning(void)
{
	volatile int *r3;
	volatile int r4;

	r3 = doitlk_rr_addr_dep_begin_call_beginning_helper();
	r4 = READ_ONCE(*r3);

	return 0;
}

/* BUGs: 1 */
static volatile noinline int *rr_addr_dep_end_call_beginning_helper(void)
{
	volatile int *r1;
	volatile int *r2;

	r1 = READ_ONCE(*foo);

	r2 = &r1[42];

	return r2;
}

static noinline int doitlk_rr_addr_dep_end_call_beginning(void)
{
	volatile int *r3;
	volatile int r4;

	r3 = rr_addr_dep_end_call_beginning_helper();
	r4 = READ_ONCE(*r3);

	return 0;
}

static volatile int *noinline
rr_addr_dep_begin_call_dep_chain_helper(volatile int *r2)
{
	volatile int *r3;

	r3 = &r2[42];

	return r3;
}

/* BUGs: 1 */
static noinline int doitlk_rr_addr_dep_begin_call_dep_chain(void)
{
	volatile int *r1;
	volatile int *r4;
	volatile int r5;

	r1 = READ_ONCE(*foo);

	r4 = rr_addr_dep_begin_call_dep_chain_helper(r1);

	r5 = READ_ONCE(*r4);

	return 0;
}

static noinline volatile int *
rr_addr_dep_end_call_dep_chain_helper(volatile int *r2)
{
	volatile int *r3;

	r3 = &r2[42];

	return r3;
}

/* BUGs: 1 */
static noinline int doitlk_rr_addr_dep_end_call_dep_chain(void)
{
	volatile int *r1;
	volatile int *r4;
	volatile int r5;

	r1 = READ_ONCE(*foo);

	r4 = rr_addr_dep_end_call_dep_chain_helper(r1);

	r5 = READ_ONCE(*r4);

	return 0;
}

/* BUGs: 1 */

/*
 * There is only one dependency here beacuse whatever way control flow takes,
 * the IDs will be the same.
 * This of course applies to the 'end' case as well.
 */
static noinline int doitlk_rr_addr_dep_begin_cond_dep_chain_full(void)
{
	volatile int *r1;
	volatile int *r2;
	volatile int r3;

	r1 = READ_ONCE(*foo);

	if (*bar)
		r2 = &r1[21];
	else
		r2 = &r1[42];

	r3 = READ_ONCE(*r2);

	return 0;
}

/* BUGs: 1 */
static noinline int doitlk_rr_addr_dep_end_cond_dep_chain_full(void)
{
	volatile int *r1;
	volatile int *r2;
	volatile int r3;

	r1 = READ_ONCE(*foo);

	if (*bar)
		r2 = &r1[21];
	else
		r2 = &r1[42];

	r3 = READ_ONCE(*r2);

	return 0;
}

/* BUGs: 1 */
static noinline int doitlk_rr_addr_dep_begin_cond_dep_chain_partial(void)
{
	volatile int *r1;
	volatile int *r2 = *bar;
	volatile int r3;

	r1 = READ_ONCE(*foo);

	if (!*bar)
		r2 = &r1[42];

	r3 = READ_ONCE(*r2);

	return 0;
}

/* BUGs: 1 */
static noinline int doitlk_rr_addr_dep_end_cond_dep_chain_partial(void)
{
	volatile int *r1;
	volatile int *r2 = *bar;
	volatile int r3;

	r1 = READ_ONCE(*foo);

	if (!*bar)
		r2 = &r1[42];

	r3 = READ_ONCE(*r2);

	return 0;
}

// // Begin addr dep 8: two address dependencies with same beginning within the same function - for breaking the begin annotation
// static noinline int doitlk_rr_addr_dep_begin_8(void)
// {
// 	volatile int *r1, *r2;

// 	// Begin address dependency
// 	x = READ_ONCE(foo);

// 	r1 = &x[42];
// 	r2 = &x[21];

// 	// End address dependency
// 	y = READ_ONCE(*r1);

// 	// End address dependency
// 	z = READ_ONCE(*r2);

// 	return 0;
// }

// // End addr dep 8: two address dependencies with same beginning within the same function - for breaking the end annotation
// static noinline int doitlk_rr_addr_dep_end_8(void)
// {
// 	volatile int *r1, *r2;

// 	// Begin address dependency
// 	x = READ_ONCE(foo);

// 	r1 = &xp[42];
// 	r2 = &xp[21];

// 	// End address dependency
// 	y = READ_ONCE(*r1);

// 	// End address dependency
// 	z = READ_ONCE(*r2);

// 	return 0;
// }

// static noinline void rr_begin_9_helper1(volatile int *r1)
// {
// 	// End address dependency
// 	x = READ_ONCE(*r1);
// }

// static noinline void rr_begin_9_helper2(volatile int *r1)
// {
// 	// End address dependency
// 	z = READ_ONCE(*r1);
// }

// // Begin addr dep 9: two address dependencies with same beginning within the same function - for breaking the begin annotation
// static noinline int doitlk_rr_addr_dep_begin_9(void)
// {
// 	volatile int *r1, *r2;

// 	// Begin address dependency
// 	x = READ_ONCE(foo);

// 	r1 = &x[42];
// 	r2 = &x[21];

// 	rr_begin_9_helper1(r1);
// 	rr_begin_9_helper2(r2);

// 	return 0;
// }

// // End addr dep 9: two address dependencies with same beginning within the same function - for breaking the end annotation
// static noinline void doitlk_rr_addr_dep_end_9_helper1(volatile int *r1)
// {
// 	// End address dependency
// 	y = READ_ONCE(*r1);
// }

// static noinline void doitlk_rr_addr_dep_end_9_helper2(volatile int *r1)
// {
// 	// End address dependency
// 	z = READ_ONCE(*r1);
// }

// // Begin addr dep 9: two address dependencies with same beginning within the same function - for breaking the begin annotation
// static noinline int rr_addr_dep_end_9(void)
// {
// 	volatile int *r1, *r2;

// 	// Begin address dependency
// 	x = READ_ONCE(foo);

// 	r1 = &x[42];
// 	r2 = &x[21];

// 	doitlk_rr_addr_dep_end_9_helper1(r1);
// 	doitlk_rr_addr_dep_end_9_helper2(r2);

// 	return 0;
// }

// // Begin addr dep 10: address dependencies through function call, but different chains
// static volatile int *noinline begin_10_helper(volatile int *r1)
// {
// 	volatile int *r1, *r2, *r3;

// 	x = &r1[42];

// 	// End address dependency
// 	r1 = READ_ONCE(*x);

// 	// Begin address dependency
// 	r2 = READ_ONCE(*x);

// 	r3 = &r2[42];

// 	// copy bar and return it
// 	return r3;
// }

// static noinline int doitlk_rr_addr_dep_begin_10(void)
// {
// 	volatile int *barLocal;

// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	x = READ_ONCE(foo);

// 	y = begin_10_helper(x);

// 	z = READ_ONCE(*y);

// 	return 0;
// }

// // End addr dep 10: address dependencies through function call, but different chains
// static volatile int *noinline end_10_helper(volatile int *xpLocal)
// {
// 	volatile int *barLocal;

// 	bar = &xpLocal[42];

// 	// End address dependency
// 	y = READ_ONCE(*bar);

// 	// Begin address dependency
// 	xp = READ_ONCE(foo);

// 	barLocal = &xp[42];

// 	// copy bar and return it
// 	return barLocal;
// }

// static noinline int doitlk_rr_addr_dep_end_10(void)
// {
// 	volatile int *barLocal;

// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	barLocal = end_10_helper(xp);

// 	// End address dependency
// 	// y == x[42] == 0
// 	z = READ_ONCE(*barLocal);

// 	return 0;
// }

// static volatile int *noinline doitlk_rr_addr_dep_begin_11_helper1()
// {
// 	xp = READ_ONCE(foo);

// 	bar = &xp[42];

// 	return bar;
// }

// static noinline void begin_11_helper2(volatile int *r1)
// {
// 	y = READ_ONCE(*r1);
// }

// static noinline int rr_addr_dep_begin_11(void)
// {
// 	volatile int *r1;

// 	r1 = doitlk_rr_addr_dep_begin_11_helper1();

// 	begin_11_helper2(r1);

// 	return 0;
// }

// static volatile int *noinline end_11_helper1()
// {
// 	xp = READ_ONCE(foo);

// 	bar = &xp[42];

// 	return bar;
// }

// static noinline void doitlk_rr_addr_dep_end_11_helper2(volatile int *r1)
// {
// 	y = READ_ONCE(*r1);
// }

// static noinline int rr_addr_dep_end_11(void)
// {
// 	volatile int *r1;

// 	r1 = end_11_helper1();

// 	doitlk_rr_addr_dep_end_11_helper2(r1);

// 	return 0;
// }

// static volatile int *noinline
// doitlk_rr_addr_dep_begin_12_helper(volatile int *foo)
// {
// 	xp = READ_ONCE(foo);

// 	xp += 42;

// 	return xp;
// }

// static noinline int rr_addr_dep_begin_12(void)
// {
// 	volatile int *r1;

// 	r1 = doitlk_rr_addr_dep_begin_12_helper(foo);

// 	return *doitlk_rr_addr_dep_begin_12_helper(r1);
// }

// //FIXME LLVM currently looses the annotation metadata here after the bug was inserted
// static volatile int *noinline doitlk_rr_addr_dep_end_12_helper(volatile int *foo)
// {
// 	xp = READ_ONCE(foo);

// 	xp += 42;

// 	return xp;
// }

// static noinline int rr_addr_dep_end_12(void)
// {
// 	volatile int *r1;

// 	r1 = doitlk_rr_addr_dep_end_12_helper(foo);

// 	return *doitlk_rr_addr_dep_end_12_helper(r1);
// }

// // Example from original DoitLk talk at LPC 2020
// struct tk_fast {
// 	seqcount_latch_t seq;
// 	struct tk_read_base base[2];
// };

// static __always_inline u64 doitlk_ktime(struct tk_fast *tkf)
// {
// 	struct tk_read_base *tkr;
// 	unsigned int seq;
// 	u64 now;

// 	do {
// 		seq = raw_read_seqcount_latch(&tkf->seq);
// 		tkr = tkf->base + (seq & 0x01);
// 		now = ktime_to_ns(READ_ONCE(tkr->base));

// 		now += timekeeping_delta_to_ns(
// 			tkr, clocksource_delta(tk_clock_read(tkr),
// 					       tkr->cycle_last, tkr->mask));
// 	} while (read_seqcount_latch_retry(&tkf->seq, seq));

// 	return now;
// }

// //
// // ====
// // Read -> Write Address Dependencies
// // ====
// //

// // Begin addr dep 1: rw address dependency within the same function - for breaking the begin annotation
// static noinline int doitlk_rw_addr_dep_begin_1(void)
// {
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 	bar = &xp[42];

// 	// End address dependency
// 	// y == x[42] == 0
// 	WRITE_ONCE(*bar, y);

// 	return 0;
// }

// // End addr dep 1: rw address dependency within the same function - for breaking the end annotation
// static noinline int doitlk_rw_addr_dep_end_1(void)
// {
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 	bar = &xp[42];

// 	// End address dependency
// 	// y == x[42] == 0
// 	WRITE_ONCE(*bar, y);

// 	return 0;
// }

// // Begin addr dep 2: address dependency accross two function. Dep begins in first function - for breaking begin annotation
// static noinline void rw_begin_2_helper(volatile int *r1)
// {
// 	// End address dependency
// 	// y == x[42] == 0
// 	WRITE_ONCE(*r1, y);
// }

// static noinline int doitlk_rw_addr_dep_begin_2(void)
// {
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 	bar = &xp[42];

// 	// copy bar into local var and dereference it in call
// 	rw_begin_2_helper(bar);

// 	return 0;
// }

// // End addr dep 2: address dependency accross two function. Dep begins in first function - for breaking end annotation
// static noinline void doitlk_rw_addr_dep_end_2_helper(volatile int *r1)
// {
// 	// End address dependency
// 	// y == x[42] == 0
// 	WRITE_ONCE(*r1, y);
// }

// static noinline int rw_addr_dep_end_2(void)
// {
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 	bar = &xp[42];

// 	// copy bar into local var and dereference it in call
// 	doitlk_rw_addr_dep_end_2_helper(bar);

// 	return 0;
// }

// // Begin addr dep 3: address dependency accross two function. Dep begins in second function - for breaking beginn annotation
// static noinline int rw_addr_dep_begin_3(void)
// {
// 	volatile int *barLocal = doitlk_rr_addr_dep_begin_3_helper();

// 	// End address dependency
// 	// y == x[42] == 0
// 	WRITE_ONCE(*barLocal, y);

// 	// alternative
// 	// const volatile int* yLocal = dep_2_begin_second_helper();
// 	// y = READ_ONCE(yLocal);

// 	return 0;
// }

// // End addr dep 3: address dependency accross two function. Dep begins in second function - for breaking end annotation
// static noinline int doitlk_rw_addr_dep_end_3(void)
// {
// 	// End address dependency
// 	// y == x[42] == 0
// 	WRITE_ONCE(*end_3_helper(), y);

// 	// alternative
// 	// const volatile int* yLocal = dep_2_begin_second_helper();
// 	// y = READ_ONCE(yLocal);

// 	return 0;
// }

// // Begin addr dep 4: address dependency accross two function. Dep begins in first functions, runs through second function and ends in first function
// static noinline int doitlk_rw_addr_dep_begin_4(void)
// {
// 	volatile int *barLocal;

// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	barLocal = begin_4_helper(xp);

// 	// End address dependency
// 	// y == x[42] == 0
// 	WRITE_ONCE(*barLocal, y);

// 	return 0;
// }

// // End addr dep 4: address dependency accross two function. Dep begins in first functions, runs through second function and ends in first function
// static noinline int doitlk_rw_addr_dep_end_4(void)
// {
// 	volatile int *barLocal;

// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	barLocal = end_4_helper(xp);

// 	// End address dependency
// 	// y == x[42] == 0
// 	WRITE_ONCE(*barLocal, y);

// 	return 0;
// }

// // Begin addr dep 6: address dependency within the same function - for breaking the begin annotation
// static noinline int doitlk_rw_addr_dep_begin_6(void)
// {
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 	bar = &xp[42];

// 	if (&xp[0])
// 		bar = &xp[21];
// 	else
// 		bar = &xp[0];

// 	// End address dependency
// 	WRITE_ONCE(*bar, y);

// 	return 0;
// }

// // End addr dep 6: address dependency within the same function - for breaking the end annotation
// static noinline int doitlk_rw_addr_dep_end_6(void)
// {
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 	bar = &xp[42];

// 	if (&xp[0])
// 		bar = &xp[21];
// 	else
// 		bar = &xp[0];

// 	// End address dependency
// 	WRITE_ONCE(*bar, y);

// 	return 0;
// }

// // Begin addr dep 7: address dependency within the same function - for breaking the begin annotation
// static noinline int doitlk_rw_addr_dep_begin_7(void)
// {
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 	if (&xp[0])
// 		bar = &xp[42];

// 	// End address dependency
// 	WRITE_ONCE(*bar, y);

// 	return 0;
// }

// // End addr dep 7: address dependency within the same function - for breaking the end annotation
// static noinline int doitlk_rw_addr_dep_end_7(void)
// {
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	if (&xp[0])
// 		bar = &xp[42];

// 	// End address dependency
// 	WRITE_ONCE(*bar, y);

// 	return 0;
// }

// // Begin addr dep 8: two address dependencies with same beginning within the same function - for breaking the begin annotation
// static noinline int doitlk_rw_addr_dep_begin_8(void)
// {
// 	volatile int *barLocal;
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 	bar = &xp[42];
// 	barLocal = &xp[21];

// 	// End address dependency
// 	// y == x[42] == 0
// 	WRITE_ONCE(*bar, y);
// 	WRITE_ONCE(*barLocal, y);

// 	return 0;
// }

// // End addr dep 8: two address dependencies with same beginning within the same function - for breaking the end annotation
// static noinline int doitlk_rw_addr_dep_end_8(void)
// {
// 	volatile int *barLocal;
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 	bar = &xp[42];
// 	barLocal = &xp[21];

// 	// End address dependency
// 	// y == x[42] == 0
// 	WRITE_ONCE(*bar, y);
// 	WRITE_ONCE(*barLocal, y);

// 	return 0;
// }

// static noinline void rw_begin_9_helper1(volatile int *r11)
// {
// 	// End address dependency
// 	WRITE_ONCE(*r11, y);
// }

// static noinline void rw_begin_9_helper2(volatile int *r12)
// {
// 	// End address dependency
// 	WRITE_ONCE(*r12, z);
// }

// // Begin addr dep 9: two address dependencies with same beginning within the same function - for breaking the begin annotation
// static noinline int doitlk_rw_addr_dep_begin_9(void)
// {
// 	volatile int *barLocal1, *barLocal2;
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 	barLocal1 = &xp[42];
// 	barLocal2 = &xp[21];

// 	rw_begin_9_helper1(barLocal1);
// 	rw_begin_9_helper2(barLocal2);

// 	return 0;
// }

// // End addr dep 9: two address dependencies with same beginning within the same function - for breaking the end annotation
// static noinline void doitlk_rw_addr_dep_end_9_helper1(volatile int *r11)
// {
// 	// End address dependency
// 	WRITE_ONCE(*r11, y);
// }

// static noinline void doitlk_rw_addr_dep_end_9_helper2(volatile int *r12)
// {
// 	// End address dependency
// 	WRITE_ONCE(*r12, z);
// }

// // Begin addr dep 9: two address dependencies with same beginning within the same function - for breaking the begin annotation
// static noinline int rw_addr_dep_end_9(void)
// {
// 	volatile int *barLocal1, *barLocal2;
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 	barLocal1 = &xp[42];
// 	barLocal2 = &xp[21];

// 	doitlk_rw_addr_dep_end_9_helper1(barLocal1);
// 	doitlk_rw_addr_dep_end_9_helper2(barLocal2);

// 	return 0;
// }

// // Begin addr dep 10: address dependencies through function call, but different chains
// static volatile int *noinline rw_begin_10_helper(volatile int *xpLocal)
// {
// 	volatile int *barLocal;

// 	bar = &xpLocal[42];

// 	// End address dependency
// 	WRITE_ONCE(*bar, y);

// 	// Begin address dependency
// 	xp = READ_ONCE(foo);

// 	barLocal = &xp[42];

// 	// copy bar and return it
// 	return barLocal;
// }

// static noinline int doitlk_rw_addr_dep_begin_10(void)
// {
// 	volatile int *barLocal;

// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	barLocal = rw_begin_10_helper(xp);

// 	WRITE_ONCE(*barLocal, z);

// 	return 0;
// }

// // End addr dep 10: address dependencies through function call, but different chains
// static volatile int *noinline rw_end_10_helper(volatile int *xpLocal)
// {
// 	volatile int *barLocal;

// 	bar = &xpLocal[42];

// 	// End address dependency
// 	WRITE_ONCE(*bar, y);

// 	// Begin address dependency
// 	xp = READ_ONCE(foo);

// 	barLocal = &xp[42];

// 	// copy bar and return it
// 	return barLocal;
// }

// static noinline int doitlk_rw_addr_dep_end_10(void)
// {
// 	volatile int *barLocal;

// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	barLocal = rw_end_10_helper(xp);

// 	// End address dependency
// 	// y == x[42] == 0
// 	WRITE_ONCE(*barLocal, z);

// 	return 0;
// }

// static volatile int *noinline doitlk_rw_addr_dep_begin_11_helper1()
// {
// 	xp = READ_ONCE(foo);

// 	bar = &xp[42];

// 	return bar;
// }

// static noinline void rw_begin_11_helper2(volatile int *r1)
// {
// 	WRITE_ONCE(*r1, 42);
// }

// static noinline int rw_addr_dep_begin_11(void)
// {
// 	volatile int *r1;

// 	r1 = doitlk_rw_addr_dep_begin_11_helper1();

// 	rw_begin_11_helper2(r1);

// 	return 0;
// }

// static volatile int *noinline rw_end_11_helper1()
// {
// 	xp = READ_ONCE(foo);

// 	bar = &xp[42];

// 	return bar;
// }

// static noinline void doitlk_rw_addr_dep_end_11_helper2(volatile int *r1)
// {
// 	WRITE_ONCE(*r1, 42);
// }

// static noinline int rw_addr_dep_end_11(void)
// {
// 	volatile int *r1;

// 	r1 = rw_end_11_helper1();

// 	doitlk_rw_addr_dep_end_11_helper2(r1);

// 	return 0;
// }

// // Begin ctrl dep 1: memory-barriers.txt case 1, control dependency within the same function - dependent - for breaking the begin annotation
// static noinline int doitlk_ctrl_dep_begin_1(void)
// {
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	if (xp) {
// 		// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 		bar = &xp[42];

// 		// End address dependency
// 		// y == x[42] == 0
// 		WRITE_ONCE(y, *bar);
// 	}
// 	return 0;
// }

// // End ctrl dep 1: control dependency within the same function - dependent condition - for breaking the end annotation
// static noinline int doitlk_ctrl_dep_end_1(void)
// {
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	if (xp) {
// 		// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 		bar = &xp[42];

// 		// End address dependency
// 		// y == x[42] == 0
// 		WRITE_ONCE(y, *bar);
// 	}
// 	return 0;
// }

// // Begin ctrl dep 2: control dependency within the same function - dependent condition - for breaking the begin annotation
// static noinline int doitlk_ctrl_dep_begin_2(void)
// {
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	if (foo) {
// 		// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 		bar = &xp[42];

// 		// End address dependency
// 		// y == x[42] == 0
// 		WRITE_ONCE(y, *bar);
// 	}
// 	return 0;
// }

// // End ctrl dep 2: control dependency within the same function - dependent condition - for breaking the end annotation
// static noinline int doitlk_ctrl_dep_end_2(void)
// {
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	if (foo) {
// 		// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 		bar = &xp[42];

// 		// End address dependency
// 		// y == x[42] == 0
// 		WRITE_ONCE(y, *bar);
// 	}
// 	return 0;
// }

// // Begin ctrl dep 3: control dependency with dead branch within the same function -  for breaking the begin annotation
// static noinline int doitlk_ctrl_dep_begin_3(void)
// {
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	if (0) {
// 		// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 		bar = &xp[42];

// 		// End address dependency
// 		// y == x[42] == 0
// 		WRITE_ONCE(y, *bar);
// 	}
// 	return 0;
// }

// // End ctrl dep 3: control dependency with dead branch within the same function -  for breaking the begin annotation
// static noinline int doitlk_ctrl_dep_end_3(void)
// {
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	if (0) {
// 		// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 		bar = &xp[42];

// 		// End address dependency
// 		// y == x[42] == 0
// 		WRITE_ONCE(y, *bar);
// 	}
// 	return 0;
// }

// // Begin ctrl dep 4: control dependency with dead branch within the same function -  for breaking the begin annotation
// static noinline int doitlk_ctrl_dep_begin_4(void)
// {
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);
// 	if (xp) {
// 		// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 		bar = &xp[42];

// 		// End address dependency
// 		// y == x[42] == 0
// 		WRITE_ONCE(y, *bar);
// 	}
// 	return 0;
// }

// // End ctrl dep 4: control dependency with dead branch within the same function -  for breaking the begin annotation
// static noinline int doitlk_ctrl_dep_end_4(void)
// {
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);
// 	if (xp) {
// 		// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 		bar = &xp[42];

// 		// End address dependency
// 		// y == x[42] == 0
// 		WRITE_ONCE(y, *bar);
// 	}
// 	return 0;
// }

// // Begin ctrl dep 5: control dependency with end in for loop -  for breaking the begin annotation
// static noinline int doitlk_ctrl_dep_begin_5(void)
// {
// 	int i = 0;
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	for (; i < 42; ++i) {
// 		// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 		bar = &xp[i];

// 		// End address dependency
// 		// y == x[42] == 0
// 		WRITE_ONCE(y, *bar);
// 	}
// 	return 0;
// }
// // TODO syntactic dep?
// // End ctrl dep 5: control dependency with end in for loop -  for breaking the begin annotation
// static noinline int doitlk_ctrl_dep_end_5(void)
// {
// 	int i = 0;
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	for (; i < 42; ++i) {
// 		// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 		bar = &xp[i];

// 		// End address dependency
// 		// y == x[42] == 0
// 		WRITE_ONCE(y, *bar);
// 	}
// 	return 0;
// }

// // Begin ctrl dep 6: memory-barriers.txt case 2, constant dependent condition
// static noinline int doitlk_ctrl_dep_begin_6(void)
// {
// 	x = READ_ONCE(*foo);
// 	if (x % MAX == 0)
// 		WRITE_ONCE(*bar, y);
// 	return 0;
// }

// // End ctrl dep 6: memory-barriers.txt case 2, constant dependent condition
// static noinline int doitlk_ctrl_dep_end_6(void)
// {
// 	x = READ_ONCE(*foo);
// 	if (x % MAX == 0)
// 		WRITE_ONCE(*bar, y);
// 	return 0;
// }

// // Begin ctrl dep 7: memory-barriers.txt case 3, constant dependent condition
// static noinline int doitlk_ctrl_dep_begin_7(void)
// {
// 	x = READ_ONCE(*foo);
// 	if (x || 1 > 0)
// 		WRITE_ONCE(y, 1);
// 	return 0;
// }

// // End ctrl dep 7: memory-barriers.txt case 3, constant dependent condition
// static noinline int doitlk_ctrl_dep_end_7(void)
// {
// 	x = READ_ONCE(*foo);
// 	if (x || 1 > 0)
// 		WRITE_ONCE(y, 1);
// 	return 0;
// }

// // Begin ctrl dep 8: constant dependent condition
// static noinline int doitlk_ctrl_dep_begin_8(void)
// {
// 	x = READ_ONCE(*foo);
// 	if (x > 0) {
// 		if (x > 42)
// 			WRITE_ONCE(y, 42);
// 		else
// 			WRITE_ONCE(y, 0);
// 	}
// 	return 0;
// }

// // End ctrl dep 8: constant dependent condition
// static noinline int doitlk_ctrl_dep_end_8(void)
// {
// 	x = READ_ONCE(*foo);
// 	if (x > 0) {
// 		if (x > 42)
// 			WRITE_ONCE(y, 42);
// 		else
// 			WRITE_ONCE(y, 0);
// 	}
// 	return 0;
// }

// // Begin ctrl dep 9: two dep chains in condition
// static noinline int doitlk_ctrl_dep_begin_9(void)
// {
// 	x = READ_ONCE(*foo);
// 	y = READ_ONCE(*bar);
// 	if (x > y)
// 		WRITE_ONCE(z, 1);
// 	return 0;
// }

// // End ctrl dep 9: two dep chains in condition
// static noinline int doitlk_ctrl_dep_end_9(void)
// {
// 	x = READ_ONCE(*foo);
// 	y = READ_ONCE(*bar);
// 	if (x > y)
// 		WRITE_ONCE(z, 1);
// 	return 0;
// }

// // Begin ctrl dep 10: three dep chains in condition
// static noinline int doitlk_ctrl_dep_begin_10(void)
// {
// 	x = READ_ONCE(*foo);
// 	y = READ_ONCE(*bar);
// 	z = READ_ONCE(*xp);
// 	if (x > y || z < y)
// 		WRITE_ONCE(*foo, 1);
// 	return 0;
// }

// // End ctrl dep 10: three dep chains in condition
// static noinline int doitlk_ctrl_dep_end_10(void)
// {
// 	x = READ_ONCE(*foo);
// 	y = READ_ONCE(*bar);
// 	z = READ_ONCE(*xp);
// 	if (x > y || z < y)
// 		WRITE_ONCE(*foo, 1);
// 	return 0;
// }

// // Begin ctrl dep 11: nested if
// static noinline int doitlk_ctrl_dep_begin_11(void)
// {
// 	x = READ_ONCE(*foo);
// 	if (x) {
// 		y = READ_ONCE(*bar);
// 		if (y)
// 			WRITE_ONCE(*foo, 1);
// 	}
// 	return 0;
// }

// // End ctrl dep 11: nested if
// static noinline int doitlk_ctrl_dep_end_11(void)
// {
// 	x = READ_ONCE(*foo);
// 	if (x) {
// 		y = READ_ONCE(*bar);
// 		if (y)
// 			WRITE_ONCE(*foo, 1);
// 	}
// 	return 0;
// }

// // Begin ctrl dep 12: conditional return
// static noinline int ctrl_dep_12_begin_helper(void)
// {
// 	x = READ_ONCE(*foo);
// 	if (x)
// 		return 21;
// 	else
// 		return 42;
// }

// static noinline int doitlk_ctrl_dep_begin_12(void)
// {
// 	WRITE_ONCE(*foo, ctrl_dep_12_begin_helper());
// 	return 0;
// }

// // End ctrl dep 11: nested if
// static noinline int ctrl_dep_12_end_helper(void)
// {
// 	x = READ_ONCE(*foo);
// 	if (x)
// 		return 21;
// 	else
// 		return 42;
// }

// static noinline int doitlk_ctrl_dep_end_12(void)
// {
// 	WRITE_ONCE(*foo, ctrl_dep_12_end_helper());
// 	return 0;
// }

// static volatile int *noinline ctrl_dep_begin_13_helper(void)
// {
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 	bar = &xp[42];

// 	// copy bar and return it
// 	return bar;
// }

// static volatile int *noinline ctrl_dep_end_13_helper(void)
// {
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 	bar = &xp[42];

// 	// copy bar and return it
// 	return bar;
// }

// static noinline int doitlk_ctrl_dep_begin_13(void)
// {
// 	foo = ctrl_dep_begin_13_helper();
// 	if (foo)
// 		WRITE_ONCE(*foo, 1);
// 	return 0;
// }

// static noinline int doitlk_ctrl_dep_end_13(void)
// {
// 	foo = ctrl_dep_end_13_helper();
// 	if (foo)
// 		WRITE_ONCE(*foo, 1);
// 	return 0;
// }

// static volatile int *noinline ctrl_dep_begin_14_helper1(void)
// {
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 	bar = &xp[42];

// 	// copy bar and return it
// 	return bar;
// }

// static volatile int *noinline ctrl_dep_end_14_helper1(void)
// {
// 	// Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 	bar = &xp[42];

// 	// copy bar and return it
// 	return bar;
// }

// static noinline void ctrl_dep_begin_14_helper2(volatile int *r1)
// {
// 	WRITE_ONCE(*r1, y);
// }

// static noinline void ctrl_dep_end_14_helper2(volatile int *r1)
// {
// 	WRITE_ONCE(*r1, y);
// }

// static noinline int doitlk_ctrl_dep_begin_14(void)
// {
// 	foo = ctrl_dep_begin_14_helper1();
// 	if (foo)
// 		ctrl_dep_begin_14_helper2(foo);
// 	return 0;
// }

// static noinline int doitlk_ctrl_dep_end_14(void)
// {
// 	foo = ctrl_dep_end_14_helper1();
// 	if (foo)
// 		ctrl_dep_end_14_helper2(foo);
// 	return 0;
// }

// static noinline void ctrl_dep_begin_15_helper(volatile int *r1)
// {
// 	WRITE_ONCE(*r1, y);
// }

// static noinline void ctrl_dep_end_15_helper(volatile int *r1)
// {
// 	WRITE_ONCE(*r1, y);
// }

// static noinline int doitlk_ctrl_dep_begin_15(void)
// {
// 	foo = READ_ONCE(bar);
// 	if (foo)
// 		ctrl_dep_begin_15_helper(foo);
// 	return 0;
// }

// static noinline int doitlk_ctrl_dep_end_15(void)
// {
// 	foo = READ_ONCE(bar);
// 	if (foo)
// 		ctrl_dep_end_15_helper(foo);
// 	return 0;
// }

// static noinline int doitlk_ctrl_dep_begin_16(void)
// {
// loop:
// 	foo = READ_ONCE(bar);
// 	if (foo)
// 		goto end;

// 	// do something with x
// 	x++;
// 	goto loop;

// end:
// 	WRITE_ONCE(*foo, *bar);
// 	return x;
// }

// static noinline int doitlk_ctrl_dep_end_16(void)
// {
// loop:
// 	foo = READ_ONCE(bar);
// 	if (*foo == x)
// 		goto end;

// 	// do something with x
// 	x++;
// 	goto loop;

// end:
// 	WRITE_ONCE(*foo, *bar);
// 	return x;
// }

// static noinline int doitlk_ctrl_dep_begin_17(void)
// {
// loop:
// 	foo = READ_ONCE(bar);
// 	if (*foo == x) {
// 		WRITE_ONCE(*foo, *bar);
// 		return x;
// 	}

// 	// do something with x
// 	x++;
// 	goto loop;
// }

// static noinline int doitlk_ctrl_dep_end_17(void)
// {
// loop:
// 	foo = READ_ONCE(bar);
// 	if (*foo == x) {
// 		WRITE_ONCE(*foo, *bar);
// 		return x;
// 	}

// 	// do something with x
// 	x++;
// 	goto loop;
// }

// // Begin ctrl dep 18: memory-barriers.txt case 2, constant dependent condition
// static noinline int doitlk_ctrl_dep_begin_18(void)
// {
// 	xUnsigned = READ_ONCE(*foo);
// 	if (xUnsigned % MAX == 0)
// 		WRITE_ONCE(*bar, y);
// 	return 0;
// }

// // End ctrl dep 18: memory-barriers.txt case 2, constant dependent condition
// static noinline int doitlk_ctrl_dep_end_18(void)
// {
// 	xUnsigned = READ_ONCE(*fooUnsigned);
// 	if (xUnsigned % MAX == 0)
// 		WRITE_ONCE(*bar, y);
// 	return 0;
// }

static int lkm_init(void)
{
	// 	static struct clocksource dummy_clock = {
	// 		.read = dummy_clock_read,
	// 	};

	// #define FAST_TK_INIT                                                 \
// 	{                                                            \
// 		.clock = &dummy_clock, .mask = CLOCKSOURCE_MASK(64), \
// 		.mult = 1, .shift = 0,                               \
// 	}

	// static struct tk_fast tk_fast_raw ____cacheline_aligned = {
	// 	.seq = SEQCNT_LATCH_ZERO(tk_fast_raw.seq),
	// 	.base[0] = FAST_TK_INIT,
	// 	.base[1] = FAST_TK_INIT,
	// };

	doitlk_rr_addr_dep_begin_simple();
	doitlk_rr_addr_dep_end_simple();

	doitlk_rr_addr_dep_begin_call_ending();
	rr_addr_dep_end_call_ending();

	rr_addr_dep_begin_call_beginning();
	doitlk_rr_addr_dep_end_call_beginning();

	doitlk_rr_addr_dep_begin_call_dep_chain();
	doitlk_rr_addr_dep_end_call_dep_chain();

	doitlk_rr_addr_dep_begin_cond_dep_chain_full();
	doitlk_rr_addr_dep_end_cond_dep_chain_full();

	doitlk_rr_addr_dep_begin_cond_dep_chain_partial();
	doitlk_rr_addr_dep_end_cond_dep_chain_partial();
	// // simple case, fan out
	// doitlk_rr_addr_dep_begin_8();
	// doitlk_rr_addr_dep_end_8();
	// // dep chain fanning out
	// doitlk_rr_addr_dep_begin_9();
	// rr_addr_dep_end_9();
	// // in and out, but different chains
	// doitlk_rr_addr_dep_begin_10();
	// doitlk_rr_addr_dep_end_10();
	// rr_addr_dep_begin_11();
	// rr_addr_dep_end_11();
	// rr_addr_dep_begin_12();
	// rr_addr_dep_end_12();
	// // chain fanning in not relevant
	// // doitlk example
	// doitlk_ktime(&tk_fast_raw);

	// // rw addr deps
	// // simple case
	// doitlk_rw_addr_dep_begin_1();
	// doitlk_rw_addr_dep_end_1();
	// // in via function parameter
	// doitlk_rw_addr_dep_begin_2();
	// rw_addr_dep_end_2();
	// // out via function return
	// rw_addr_dep_begin_3();
	// doitlk_rw_addr_dep_end_3();
	// // in and out same chain
	// doitlk_rw_addr_dep_begin_4();
	// doitlk_rw_addr_dep_end_4();
	// // dep 5 ommitted since we can't use WRITE_ONCE() as if condition
	// // Simple Case - Chain Through If-Else
	// doitlk_rw_addr_dep_begin_6();
	// doitlk_rw_addr_dep_end_6();
	// // Simple Case - Chain Through If
	// doitlk_rw_addr_dep_begin_7();
	// doitlk_rw_addr_dep_end_7();
	// // simple case, fan out
	// doitlk_rw_addr_dep_begin_8();
	// doitlk_rw_addr_dep_end_8();
	// // dep chain fanning out
	// doitlk_rw_addr_dep_begin_9();
	// rw_addr_dep_end_9();
	// // in and out, but different chains
	// doitlk_rw_addr_dep_begin_10();
	// doitlk_rw_addr_dep_end_10();
	// rw_addr_dep_begin_11();
	// rw_addr_dep_end_11();

	// // ctrl deps
	// doitlk_ctrl_dep_begin_1();
	// doitlk_ctrl_dep_end_1();
	// doitlk_ctrl_dep_begin_2();
	// doitlk_ctrl_dep_end_2();
	// doitlk_ctrl_dep_begin_3();
	// doitlk_ctrl_dep_end_3();
	// doitlk_ctrl_dep_begin_4();
	// doitlk_ctrl_dep_end_4();
	// doitlk_ctrl_dep_begin_5();
	// doitlk_ctrl_dep_end_5();
	// doitlk_ctrl_dep_begin_6();
	// doitlk_ctrl_dep_end_6();
	// doitlk_ctrl_dep_begin_7();
	// doitlk_ctrl_dep_end_7();
	// doitlk_ctrl_dep_begin_8();
	// doitlk_ctrl_dep_end_8();
	// doitlk_ctrl_dep_begin_9();
	// doitlk_ctrl_dep_end_9();
	// doitlk_ctrl_dep_begin_10();
	// doitlk_ctrl_dep_end_10();
	// doitlk_ctrl_dep_begin_11();
	// doitlk_ctrl_dep_end_11();
	// doitlk_ctrl_dep_begin_12();
	// doitlk_ctrl_dep_end_12();
	// doitlk_ctrl_dep_begin_13();
	// doitlk_ctrl_dep_end_13();
	// doitlk_ctrl_dep_begin_14();
	// doitlk_ctrl_dep_end_14();
	// doitlk_ctrl_dep_begin_15();
	// doitlk_ctrl_dep_end_15();
	// doitlk_ctrl_dep_begin_16();
	// doitlk_ctrl_dep_end_16();
	// doitlk_ctrl_dep_begin_17();
	// doitlk_ctrl_dep_end_17();
	// doitlk_ctrl_dep_begin_18();
	// doitlk_ctrl_dep_end_18();
	// TODO all cases from above?

	return 0;
}

static void lkm_exit(void)
{
	pr_debug("Bye\n");
}

module_init(lkm_init);
module_exit(lkm_exit);
