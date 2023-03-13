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

// TODO: PTR-PTE test cases where dep chain value is overwritten in a function
// call one level or several levels deep

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
static noinline int doitlk_rr_addr_dep_begin_two_endings_simple(void)
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
/* FIXME: Currently not being caught because artificially broken dep runs through "call void @llvm.lifetime.end.p0", i.e. an intrinsic. */
/*
 * There is only one bug here because only one of the endings will be broken.
 * This of course applies to the corresponding 'end' and 'rw' cases too.
 */
static noinline int doitlk_rr_addr_dep_end_two_endings_simple(void)
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
static noinline int doitlk_rw_addr_dep_begin_two_endings_simple(void)
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
static noinline int doitlk_rw_addr_dep_end_two_endings_simple(void)
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

static int lkm_init(void)
{
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

	doitlk_rr_addr_dep_begin_two_endings_simple();
	doitlk_rr_addr_dep_end_two_endings_simple();

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

	doitlk_rw_addr_dep_begin_two_endings_simple();
	doitlk_rw_addr_dep_end_two_endings_simple();

	doitlk_rw_addr_dep_begin_two_endings_in_calls();
	rw_addr_dep_end_two_endings_in_calls();

	rw_addr_dep_begin_beg_and_end_in_calls();
	rw_addr_dep_end_beg_and_end_in_calls();

	return 0;
}

static void lkm_exit(void)
{
	pr_debug("Bye\n");
}

module_init(lkm_init);
module_exit(lkm_exit);
