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
 * Total amount of bugs: 42
 * Total amount of rr_addr_dep bugs: 21
 * Total amount of rw_addr_dep bugs: 21
 */

/*
 * =============================================================================
 * READ_ONCE() -> READ_ONCE() Address Dependencies
 * =============================================================================
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
 * This of course applies to the 'end' case and the 'rw' case as well.
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

/* BUGs: 2 */
static noinline int doitlk_rr_addr_dep_begin_two_endings_dimple(void)
{
	volatile int *r1;
	volatile int *r2;
	volatile int *r3;
	volatile int r4;
	volatile int r5;

	r1 = READ_ONCE(*foo);

	r2 = &r1[42];
	r3 = &r1[21];

	r4 = READ_ONCE(*r2);

	r5 = READ_ONCE(*r3);

	return 0;
}

/* BUGs: 1 */

/*
 * There is only one bug here because only one of the endings will be broken.
 * This of course applies to the corresponding 'end' and 'rw' cases too.
 */
static noinline int doitlk_rr_addr_dep_end_two_endings_dimple(void)
{
	volatile int *r1;
	volatile int *r2;
	volatile int *r3;
	volatile int r4;
	volatile int r5;

	r1 = READ_ONCE(*foo);

	r2 = &r1[42];
	r3 = &r1[21];

	r4 = READ_ONCE(*r2);

	r5 = READ_ONCE(*r3);

	return 0;
}

static noinline void
rr_addr_dep_begin_two_endings_in_calls_helper1(volatile int *r2)
{
	*x = READ_ONCE(*r2);
}

static noinline void
rr_addr_dep_begin_two_endings_in_calls_helper2(volatile int *r3)
{
	*x = READ_ONCE(*r3);
}

/* BUGs: 2 */
static noinline int doitlk_rr_addr_dep_begin_two_endings_in_calls(void)
{
	volatile int *r1;
	volatile int *r2;
	volatile int *r3;

	r1 = READ_ONCE(*foo);

	r2 = &r1[42];
	r3 = &r1[21];

	rr_addr_dep_begin_two_endings_in_calls_helper1(r2);
	rr_addr_dep_begin_two_endings_in_calls_helper2(r3);

	return 0;
}

/* BUGs: 1 */
static noinline void
doitlk_rr_addr_dep_end_two_endings_in_calls_helper1(volatile int *r2)
{
	*x = READ_ONCE(*r2);
}

/* BUGs: 1 */
static noinline void
doitlk_rr_addr_dep_end_two_endings_in_calls_helper2(volatile int *r3)
{
	*x = READ_ONCE(*r3);
}

static noinline int rr_addr_dep_end_two_endings_in_calls(void)
{
	volatile int *r1;
	volatile int *r2;
	volatile int *r3;

	r1 = READ_ONCE(*foo);

	r2 = &r1[42];
	r3 = &r1[21];

	doitlk_rr_addr_dep_end_two_endings_in_calls_helper1(r2);
	doitlk_rr_addr_dep_end_two_endings_in_calls_helper2(r3);

	return 0;
}

/* BUGs: 1 */
static volatile int *noinline
doitlk_rr_addr_dep_begin_beg_and_end_in_calls_helper1(void)
{
	volatile int *r1;

	r1 = READ_ONCE(*foo);

	return r1;
}

static noinline void
rr_addr_dep_begin_beg_and_end_in_calls_helper2(volatile int *r3)
{
	*x = READ_ONCE(*r3);
}

static noinline int rr_addr_dep_begin_beg_and_end_in_calls(void)
{
	volatile int *r2;
	volatile int *r3;

	r2 = doitlk_rr_addr_dep_begin_beg_and_end_in_calls_helper1();

	r3 = &r2[42];

	rr_addr_dep_begin_beg_and_end_in_calls_helper2(r3);

	return 0;
}

static volatile int *noinline rr_addr_dep_end_beg_and_end_in_calls_helper1(void)
{
	volatile int *r1;

	r1 = READ_ONCE(*foo);

	return r1;
}

/* BUGs: 1 */
static noinline void
doitlk_rr_addr_dep_end_beg_and_end_in_calls_helper2(volatile int *r3)
{
	*x = READ_ONCE(*r3);
}

static noinline int rr_addr_dep_end_beg_and_end_in_calls(void)
{
	volatile int *r2;
	volatile int *r3;

	r2 = rr_addr_dep_end_beg_and_end_in_calls_helper1();

	r3 = &r2[42];

	doitlk_rr_addr_dep_end_beg_and_end_in_calls_helper2(r3);

	return 0;
}

// Example from original DoitLk talk at LPC 2020
struct tk_fast {
	seqcount_latch_t seq;
	struct tk_read_base base[2];
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

		now += timekeeping_delta_to_ns(
			tkr, clocksource_delta(tk_clock_read(tkr),
					       tkr->cycle_last, tkr->mask));
	} while (read_seqcount_latch_retry(&tkf->seq, seq));

	return now;
}

/* FIXME: have all of the previous rr cases as rw cases? What does this prove?
 * =============================================================================
 * READ_ONCE() -> WRITE_ONCE() Address Dependencies
 * =============================================================================
 */

/* BUGs: 1 */
static noinline int doitlk_rw_addr_dep_begin_simple(void)
{
	volatile int *r1;
	volatile int *r2;

	r1 = READ_ONCE(*foo);

	r2 = &r1[42];

	WRITE_ONCE(*r2, 42);

	return 0;
}

/* BUGs: 1 */
static noinline int doitlk_rw_addr_dep_end_simple(void)
{
	volatile int *r1;
	volatile int *r2;

	r1 = READ_ONCE(*foo);

	r2 = &r1[42];

	WRITE_ONCE(*r2, 42);

	return 0;
}

static noinline void rw_addr_dep_begin_call_ending_helper(volatile int *r2)
{
	WRITE_ONCE(*r2, 42);
}

/* BUGs: 1 */
static noinline int doitlk_rw_addr_dep_begin_call_ending(void)
{
	volatile int *r1;
	volatile int *r2;

	r1 = READ_ONCE(*foo);

	r2 = &r1[42];

	rw_addr_dep_begin_call_ending_helper(r2);

	return 0;
}

/* BUGs: 1 */
static noinline void doitlk_rw_addr_dep_end_call_ending_helper(volatile int *r2)
{
	WRITE_ONCE(*r2, 42);
}

static noinline int rw_addr_dep_end_call_ending(void)
{
	volatile int *r1;
	volatile int *r2;

	r1 = READ_ONCE(*foo);

	r2 = &r1[42];

	doitlk_rw_addr_dep_end_call_ending_helper(r2);

	return 0;
}

/* BUGs: 1 */
static volatile noinline int *
doitlk_rw_addr_dep_begin_call_beginning_helper(void)
{
	volatile int *r1;
	volatile int *r2;

	r1 = READ_ONCE(*foo);

	r2 = &r1[42];

	return r2;
}

static noinline int rw_addr_dep_begin_call_beginning(void)
{
	volatile int *r3;

	r3 = doitlk_rw_addr_dep_begin_call_beginning_helper();
	WRITE_ONCE(*r3, 42);

	return 0;
}

static volatile noinline int *rw_addr_dep_end_call_beginning_helper(void)
{
	volatile int *r1;
	volatile int *r2;

	r1 = READ_ONCE(*foo);

	r2 = &r1[42];

	return r2;
}

/* BUGs: 1 */
static noinline int doitlk_rw_addr_dep_end_call_beginning(void)
{
	volatile int *r3;

	r3 = rw_addr_dep_end_call_beginning_helper();
	WRITE_ONCE(*r3, 42);

	return 0;
}

static volatile int *noinline
rw_addr_dep_begin_call_dep_chain_helper(volatile int *r2)
{
	volatile int *r3;

	r3 = &r2[42];

	return r3;
}

/* BUGs: 1 */
static noinline int doitlk_rw_addr_dep_begin_call_dep_chain(void)
{
	volatile int *r1;
	volatile int *r4;

	r1 = READ_ONCE(*foo);

	r4 = rw_addr_dep_begin_call_dep_chain_helper(r1);

	WRITE_ONCE(*r4, 42);

	return 0;
}

static noinline volatile int *
rw_addr_dep_end_call_dep_chain_helper(volatile int *r2)
{
	volatile int *r3;

	r3 = &r2[42];

	return r3;
}

/* BUGs: 1 */
static noinline int doitlk_rw_addr_dep_end_call_dep_chain(void)
{
	volatile int *r1;
	volatile int *r4;

	r1 = READ_ONCE(*foo);

	r4 = rw_addr_dep_end_call_dep_chain_helper(r1);

	WRITE_ONCE(*r4, 42);

	return 0;
}

/* BUGs: 1 */
static noinline int doitlk_rw_addr_dep_begin_cond_dep_chain_full(void)
{
	volatile int *r1;
	volatile int *r2;

	r1 = READ_ONCE(*foo);

	if (*bar)
		r2 = &r1[21];
	else
		r2 = &r1[42];

	WRITE_ONCE(*r2, 42);

	return 0;
}

/* BUGs: 1 */
static noinline int doitlk_rw_addr_dep_end_cond_dep_chain_full(void)
{
	volatile int *r1;
	volatile int *r2;

	r1 = READ_ONCE(*foo);

	if (*bar)
		r2 = &r1[21];
	else
		r2 = &r1[42];

	WRITE_ONCE(*r2, 42);

	return 0;
}

/* BUGs: 1 */
static noinline int doitlk_rw_addr_dep_begin_cond_dep_chain_partial(void)
{
	volatile int *r1;
	volatile int *r2 = *bar;

	r1 = READ_ONCE(*foo);

	if (!*bar)
		r2 = &r1[42];

	WRITE_ONCE(*r2, 42);

	return 0;
}

/* BUGs: 1 */
static noinline int doitlk_rw_addr_dep_end_cond_dep_chain_partial(void)
{
	volatile int *r1;
	volatile int *r2 = *bar;

	r1 = READ_ONCE(*foo);

	if (!*bar)
		r2 = &r1[42];

	WRITE_ONCE(*r2, 42);

	return 0;
}

/* BUGs: 2 */
static noinline int doitlk_rw_addr_dep_begin_two_endings_dimple(void)
{
	volatile int *r1;
	volatile int *r2;
	volatile int *r3;

	r1 = READ_ONCE(*foo);

	r2 = &r1[42];
	r3 = &r1[21];

	WRITE_ONCE(*r2, 24);

	WRITE_ONCE(*r3, 42);

	return 0;
}

/* BUGs: 1 */
static noinline int doitlk_rw_addr_dep_end_two_endings_dimple(void)
{
	volatile int *r1;
	volatile int *r2;
	volatile int *r3;

	r1 = READ_ONCE(*foo);

	r2 = &r1[42];
	r3 = &r1[21];

	WRITE_ONCE(*r2, 24);

	WRITE_ONCE(*r3, 42);

	return 0;
}

static noinline void
rw_addr_dep_begin_two_endings_in_calls_helper1(volatile int *r2)
{
	WRITE_ONCE(*r2, 24);
}

static noinline void
rw_addr_dep_begin_two_endings_in_calls_helper2(volatile int *r3)
{
	WRITE_ONCE(*r3, 42);
}

/* BUGs: 2 */
static noinline int doitlk_rw_addr_dep_begin_two_endings_in_calls(void)
{
	volatile int *r1;
	volatile int *r2;
	volatile int *r3;

	r1 = READ_ONCE(*foo);

	r2 = &r1[42];
	r3 = &r2[21];

	rw_addr_dep_begin_two_endings_in_calls_helper1(r2);
	rw_addr_dep_begin_two_endings_in_calls_helper2(r3);

	return 0;
}

/* BUGs: 1 */
static noinline void
doitlk_rw_addr_dep_end_two_endings_in_calls_helper1(volatile int *r2)
{
	WRITE_ONCE(*r2, 24);
}

/* BUGs: 1 */
static noinline void
doitlk_rw_addr_dep_end_two_endings_in_calls_helper2(volatile int *r3)
{
	WRITE_ONCE(*r3, 42);
}

static noinline int rw_addr_dep_end_two_endings_in_calls(void)
{
	volatile int *r1;
	volatile int *r2;
	volatile int *r3;

	r1 = READ_ONCE(*foo);

	r2 = &r1[42];
	r3 = &r1[21];

	doitlk_rw_addr_dep_end_two_endings_in_calls_helper1(r2);
	doitlk_rw_addr_dep_end_two_endings_in_calls_helper2(r3);

	return 0;
}

/* BUGs : 1 */
static volatile int *noinline
doitlk_rw_addr_dep_begin_beg_and_end_in_calls_helper1(void)
{
	volatile int *r1;

	r1 = READ_ONCE(*foo);

	return r1;
}

static noinline void
rw_addr_dep_begin_beg_and_end_in_calls_helper2(volatile int *r3)
{
	WRITE_ONCE(*r3, 42);
}

static noinline int rw_addr_dep_begin_beg_and_end_in_calls(void)
{
	volatile int *r2;
	volatile int *r3;

	r2 = doitlk_rw_addr_dep_begin_beg_and_end_in_calls_helper1();

	r3 = &r2[42];

	rw_addr_dep_begin_beg_and_end_in_calls_helper2(r3);

	return 0;
}

static volatile int *noinline rw_addr_dep_end_beg_and_end_in_calls_helper1(void)
{
	volatile int *r1;

	r1 = READ_ONCE(*foo);

	return r1;
}

/* BUGs: 1 */
static noinline void
doitlk_rw_addr_dep_end_beg_and_end_in_calls_helper2(volatile int *r3)
{
	WRITE_ONCE(*r3, 42);
}

static noinline int rw_addr_dep_end_beg_and_end_in_calls(void)
{
	volatile int *r2;
	volatile int *r3;

	r2 = rw_addr_dep_end_beg_and_end_in_calls_helper1();

	r3 = &r2[42];

	doitlk_rw_addr_dep_end_beg_and_end_in_calls_helper2(r3);

	return 0;
}

/* TODO: Full to partial addr dep conversions
 * =============================================================================
 *
 * =============================================================================
 */

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
	static struct clocksource dummy_clock = {
		.read = dummy_clock_read,
	};

#define FAST_TK_INIT                                                 \
	{                                                            \
		.clock = &dummy_clock, .mask = CLOCKSOURCE_MASK(64), \
		.mult = 1, .shift = 0,                               \
	}

	static struct tk_fast tk_fast_raw ____cacheline_aligned = {
		.seq = SEQCNT_LATCH_ZERO(tk_fast_raw.seq),
		.base[0] = FAST_TK_INIT,
		.base[1] = FAST_TK_INIT,
	};

	doitlk_ktime(&tk_fast_raw);

	/* rr_addr_dep cases */
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

	doitlk_rr_addr_dep_begin_two_endings_dimple();
	doitlk_rr_addr_dep_end_two_endings_dimple();

	doitlk_rr_addr_dep_begin_two_endings_in_calls();
	rr_addr_dep_end_two_endings_in_calls();

	rr_addr_dep_begin_beg_and_end_in_calls();
	rr_addr_dep_end_beg_and_end_in_calls();

	/* rw_addr_dep cases */
	doitlk_rw_addr_dep_begin_simple();
	doitlk_rw_addr_dep_end_simple();

	doitlk_rw_addr_dep_begin_call_ending();
	rw_addr_dep_end_call_ending();

	rw_addr_dep_begin_call_beginning();
	doitlk_rw_addr_dep_end_call_beginning();

	doitlk_rw_addr_dep_begin_call_dep_chain();
	doitlk_rw_addr_dep_end_call_dep_chain();

	doitlk_rw_addr_dep_begin_cond_dep_chain_full();
	doitlk_rw_addr_dep_end_cond_dep_chain_full();

	doitlk_rw_addr_dep_begin_cond_dep_chain_partial();
	doitlk_rw_addr_dep_end_cond_dep_chain_partial();

	doitlk_rw_addr_dep_begin_two_endings_dimple();
	doitlk_rw_addr_dep_end_two_endings_dimple();

	doitlk_rw_addr_dep_begin_two_endings_in_calls();
	rw_addr_dep_end_two_endings_in_calls();

	rw_addr_dep_begin_beg_and_end_in_calls();
	rw_addr_dep_end_beg_and_end_in_calls();

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
