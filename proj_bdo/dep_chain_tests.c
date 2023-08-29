#include "linux/random.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <proj_bdo/dep_chain_tests.h>

MODULE_DESCRIPTION(
	"Kernel module, which contains several dependeny chains used for testing CustomMemDep passes");
MODULE_AUTHOR("Paul Heidekruger");
MODULE_LICENSE("GPL");

#ifndef _PROJ_BDO_DEP_CHAIN_TESTS
#define _PROJ_BDO_DEP_CHAIN_TESTS

static int test_arr[10] = { 0x42424241, 0x42424242, 0x42424243, 0x42424244,
			    0x42424245, 0x42424246, 0x42424247, 0x42424248,
			    0x42424249, 0x4242424A };
static int *test_arr_ptrs[10];
static int **x = &test_arr_ptrs[0];
static int y;

/**
 * Naming scheme: proj_bdo_(rr|rw)_(addr|ctrl)_(begin|end)_${test_name}
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
static noinline int proj_bdo_rr_addr_dep_begin_simple(void)
{
	int *r1;
	int *r2;
	int r3;

	r1 = READ_ONCE(*x);

	r2 = &r1[8];

	r3 = READ_ONCE(*r2);

	return r3;
}

/* BUGs: 1 */
static noinline int proj_bdo_rr_addr_dep_end_simple(void)
{
	int *r1;
	int *r2;
	int r3;

	r1 = READ_ONCE(*x);

	r2 = &r1[8];

	r3 = READ_ONCE(*r2);

	return r3;
}

static noinline void rr_addr_dep_begin_call_ending_helper(int *r2)
{
	y = READ_ONCE(*r2);
}

/* BUGs: 1 */
static noinline int proj_bdo_rr_addr_dep_begin_call_ending(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);

	r2 = &r1[8];

	rr_addr_dep_begin_call_ending_helper(r2);

	return 0;
}

static noinline void proj_bdo_rr_addr_dep_end_call_ending_helper(int *r2)
{
	y = READ_ONCE(*r2);
}

/* BUGs: 1*/
static noinline int rr_addr_dep_end_call_ending(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);

	r2 = &r1[8];

	proj_bdo_rr_addr_dep_end_call_ending_helper(r2);

	return 0;
}

static noinline int *proj_bdo_rr_addr_dep_begin_call_beginning_helper(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);

	r2 = &r1[8];

	return r2;
}

/* BUGs: 1 */
static noinline int rr_addr_dep_begin_call_beginning(void)
{
	int *r3;
	int r4;

	r3 = proj_bdo_rr_addr_dep_begin_call_beginning_helper();
	r4 = READ_ONCE(*r3);

	return 0;
}

/* BUGs: 1 */
static noinline int *rr_addr_dep_end_call_beginning_helper(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);

	r2 = &r1[8];

	return r2;
}

static noinline int proj_bdo_rr_addr_dep_end_call_beginning(void)
{
	int *r3;
	int r4;

	r3 = rr_addr_dep_end_call_beginning_helper();
	r4 = READ_ONCE(*r3);

	return 0;
}

static int *noinline rr_addr_dep_begin_call_dep_chain_helper(int *r2)
{
	int *r3;

	r3 = &r2[8];

	return r3;
}

/* BUGs: 1 */
static noinline int proj_bdo_rr_addr_dep_begin_call_dep_chain(void)
{
	int *r1;
	int *r4;
	int r5;

	r1 = READ_ONCE(*x);

	r4 = rr_addr_dep_begin_call_dep_chain_helper(r1);

	r5 = READ_ONCE(*r4);

	return 0;
}

static noinline int *rr_addr_dep_end_call_dep_chain_helper(int *r2)
{
	int *r3;

	r3 = &r2[8];

	return r3;
}

/* BUGs: 1 */
static noinline int proj_bdo_rr_addr_dep_end_call_dep_chain(void)
{
	int *r1;
	int *r4;
	int r5;

	r1 = READ_ONCE(*x);

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
static noinline int proj_bdo_rr_addr_dep_begin_cond_dep_chain_full(void)
{
	int *r1;
	int *r2;
	int r3;

	r1 = READ_ONCE(*x);

	if (get_random_u8())
		r2 = &r1[5];
	else
		r2 = &r1[8];

	r3 = READ_ONCE(*r2);

	return 0;
}

/* BUGs: 1 */
static noinline int proj_bdo_rr_addr_dep_end_cond_dep_chain_full(void)
{
	int *r1;
	int *r2;
	int r3;

	r1 = READ_ONCE(*x);

	if (get_random_u8())
		r2 = &r1[5];
	else
		r2 = &r1[8];

	r3 = READ_ONCE(*r2);

	return 0;
}

/* BUGs: 1 */
static noinline int proj_bdo_rr_addr_dep_begin_cond_dep_chain_partial(void)
{
	int *r1;
	int *r2 = x[4];
	int r3;

	r1 = READ_ONCE(*x);

	if (!get_random_u8())
		r2 = r1 + 9;

	r3 = READ_ONCE(*r2);

	return 0;
}

/* BUGs: 1 */
static noinline int proj_bdo_rr_addr_dep_end_cond_dep_chain_partial(void)
{
	int *r1;
	int *r2 = x[4];
	int r3;

	r1 = READ_ONCE(*x);

	if (get_random_u8())
		r2 = r1 + 8;

	r3 = READ_ONCE(*r2);

	return 0;
}

/* BUGs: 2 */
static noinline int proj_bdo_rr_addr_dep_begin_two_endings_simple(void)
{
	int *r1;
	int *r2;
	int *r3;
	int r4;
	int r5;

	r1 = READ_ONCE(*x);

	r2 = &r1[8];
	r3 = &r1[5];

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
static noinline int proj_bdo_rr_addr_dep_end_two_endings_simple(void)
{
	int *r1;
	int *r2;
	int *r3;
	int r4;
	int r5;

	r1 = READ_ONCE(*x);

	r2 = &r1[8];
	r3 = &r1[5];

	r4 = READ_ONCE(*r2);

	r5 = READ_ONCE(*r3);

	return 0;
}

static noinline void rr_addr_dep_begin_two_endings_in_calls_helper1(int *r2)
{
	y = READ_ONCE(*r2);
}

static noinline void rr_addr_dep_begin_two_endings_in_calls_helper2(int *r3)
{
	y = READ_ONCE(*r3);
}

/* BUGs: 2 */
static noinline int proj_bdo_rr_addr_dep_begin_two_endings_in_calls(void)
{
	int *r1;
	int *r2;
	int *r3;

	r1 = READ_ONCE(*x);

	r2 = &r1[8];
	r3 = &r1[5];

	rr_addr_dep_begin_two_endings_in_calls_helper1(r2);
	rr_addr_dep_begin_two_endings_in_calls_helper2(r3);

	return 0;
}

/* BUGs: 1 */
static noinline void
proj_bdo_rr_addr_dep_end_two_endings_in_calls_helper1(int *r2)
{
	y = READ_ONCE(*r2);
}

/* BUGs: 1 */
static noinline void
proj_bdo_rr_addr_dep_end_two_endings_in_calls_helper2(int *r3)
{
	y = READ_ONCE(*r3);
}

static noinline int rr_addr_dep_end_two_endings_in_calls(void)
{
	int *r1;
	int *r2;
	int *r3;

	r1 = READ_ONCE(*x);

	r2 = &r1[8];
	r3 = &r1[5];

	proj_bdo_rr_addr_dep_end_two_endings_in_calls_helper1(r2);
	proj_bdo_rr_addr_dep_end_two_endings_in_calls_helper2(r3);

	return 0;
}

/* BUGs: 1 */
static int *noinline
proj_bdo_rr_addr_dep_begin_beg_and_end_in_calls_helper1(void)
{
	int *r1;

	r1 = READ_ONCE(*x);

	return r1;
}

static noinline void rr_addr_dep_begin_beg_and_end_in_calls_helper2(int *r3)
{
	y = READ_ONCE(*r3);
}

static noinline int rr_addr_dep_begin_beg_and_end_in_calls(void)
{
	int *r2;
	int *r3;

	r2 = proj_bdo_rr_addr_dep_begin_beg_and_end_in_calls_helper1();

	r3 = &r2[8];

	rr_addr_dep_begin_beg_and_end_in_calls_helper2(r3);

	return 0;
}

static int *noinline rr_addr_dep_end_beg_and_end_in_calls_helper1(void)
{
	int *r1;

	r1 = READ_ONCE(*x);

	return r1;
}

/* BUGs: 1 */
static noinline void
proj_bdo_rr_addr_dep_end_beg_and_end_in_calls_helper2(int *r3)
{
	y = READ_ONCE(*r3);
}

static noinline int rr_addr_dep_end_beg_and_end_in_calls(void)
{
	int *r2;
	int *r3;

	r2 = rr_addr_dep_end_beg_and_end_in_calls_helper1();

	r3 = &r2[8];

	proj_bdo_rr_addr_dep_end_beg_and_end_in_calls_helper2(r3);

	return 0;
}

/* FIXME: have all of the previous rr cases as rw cases? What does this prove?
 * =============================================================================
 * READ_ONCE() -> WRITE_ONCE() Address Dependencies
 * =============================================================================
 */

/* BUGs: 1 */
static noinline int proj_bdo_rw_addr_dep_begin_simple(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);

	r2 = &r1[8];

	WRITE_ONCE(*r2, 0x21212121);

	return 0;
}

/* BUGs: 1 */
static noinline int proj_bdo_rw_addr_dep_end_simple(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);

	r2 = &r1[8];

	WRITE_ONCE(*r2, 0x21212121);

	return 0;
}

static noinline void rw_addr_dep_begin_call_ending_helper(int *r2)
{
	WRITE_ONCE(*r2, 0x21212121);
}

/* BUGs: 1 */
static noinline int proj_bdo_rw_addr_dep_begin_call_ending(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);

	r2 = &r1[8];

	rw_addr_dep_begin_call_ending_helper(r2);

	return 0;
}

/* BUGs: 1 */
static noinline void proj_bdo_rw_addr_dep_end_call_ending_helper(int *r2)
{
	WRITE_ONCE(*r2, 0x21212121);
}

static noinline int rw_addr_dep_end_call_ending(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);

	r2 = &r1[8];

	proj_bdo_rw_addr_dep_end_call_ending_helper(r2);

	return 0;
}

/* BUGs: 1 */
static noinline int *proj_bdo_rw_addr_dep_begin_call_beginning_helper(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);

	r2 = &r1[8];

	return r2;
}

static noinline int rw_addr_dep_begin_call_beginning(void)
{
	int *r3;

	r3 = proj_bdo_rw_addr_dep_begin_call_beginning_helper();
	WRITE_ONCE(*r3, 0x21212121);

	return 0;
}

static noinline int *rw_addr_dep_end_call_beginning_helper(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);

	r2 = &r1[8];

	return r2;
}

/* BUGs: 1 */
static noinline int proj_bdo_rw_addr_dep_end_call_beginning(void)
{
	int *r3;

	r3 = rw_addr_dep_end_call_beginning_helper();
	WRITE_ONCE(*r3, 0x21212121);

	return 0;
}

static int *noinline rw_addr_dep_begin_call_dep_chain_helper(int *r2)
{
	int *r3;

	r3 = &r2[8];

	return r3;
}

/* BUGs: 1 */
static noinline int proj_bdo_rw_addr_dep_begin_call_dep_chain(void)
{
	int *r1;
	int *r4;

	r1 = READ_ONCE(*x);

	r4 = rw_addr_dep_begin_call_dep_chain_helper(r1);

	WRITE_ONCE(*r4, 0x21212121);

	return 0;
}

static noinline int *rw_addr_dep_end_call_dep_chain_helper(int *r2)
{
	int *r3;

	r3 = &r2[8];

	return r3;
}

/* BUGs: 1 */
static noinline int proj_bdo_rw_addr_dep_end_call_dep_chain(void)
{
	int *r1;
	int *r4;

	r1 = READ_ONCE(*x);

	r4 = rw_addr_dep_end_call_dep_chain_helper(r1);

	WRITE_ONCE(*r4, 0x21212121);

	return 0;
}

/* BUGs: 1 */
static noinline int proj_bdo_rw_addr_dep_begin_cond_dep_chain_full(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);

	if (get_random_u8())
		r2 = &r1[5];
	else
		r2 = &r1[8];

	WRITE_ONCE(*r2, 0x21212121);

	return 0;
}

/* BUGs: 1 */
static noinline int proj_bdo_rw_addr_dep_end_cond_dep_chain_full(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);

	if (get_random_u8())
		r2 = &r1[5];
	else
		r2 = &r1[8];

	WRITE_ONCE(*r2, 0x21212121);

	return 0;
}

/* BUGs: 1 */
static noinline int proj_bdo_rw_addr_dep_begin_cond_dep_chain_partial(void)
{
	int *r1;
	int *r2 = x[4];

	r1 = READ_ONCE(*x);

	if (!get_random_u8())
		r2 = &r1[8];

	WRITE_ONCE(*r2, 0x21212121);

	return 0;
}

/* BUGs: 1 */
static noinline int proj_bdo_rw_addr_dep_end_cond_dep_chain_partial(void)
{
	int *r1;
	int *r2 = x[4];

	r1 = READ_ONCE(*x);

	if (!get_random_u8())
		r2 = &r1[8];

	WRITE_ONCE(*r2, 0x21212121);

	return 0;
}

/* BUGs: 2 */
static noinline int proj_bdo_rw_addr_dep_begin_two_endings_simple(void)
{
	int *r1;
	int *r2;
	int *r3;

	r1 = READ_ONCE(*x);

	r2 = &r1[8];
	r3 = &r1[5];

	WRITE_ONCE(*r2, 0x21212120);

	WRITE_ONCE(*r3, 0x21212121);

	return 0;
}

/* BUGs: 1 */
static noinline int proj_bdo_rw_addr_dep_end_two_endings_simple(void)
{
	int *r1;
	int *r2;
	int *r3;

	r1 = READ_ONCE(*x);

	r2 = &r1[8];
	r3 = &r1[5];

	WRITE_ONCE(*r2, 0x21212120);

	WRITE_ONCE(*r3, 0x21212121);

	return 0;
}

static noinline void rw_addr_dep_begin_two_endings_in_calls_helper1(int *r2)
{
	WRITE_ONCE(*r2, 0x21212120);
}

static noinline void rw_addr_dep_begin_two_endings_in_calls_helper2(int *r3)
{
	WRITE_ONCE(*r3, 0x21212121);
}

/* BUGs: 2 */
static noinline int proj_bdo_rw_addr_dep_begin_two_endings_in_calls(void)
{
	int *r1;
	int *r2;
	int *r3;

	r1 = READ_ONCE(*x);

	r2 = &r1[5];
	r3 = &r2[8];

	rw_addr_dep_begin_two_endings_in_calls_helper1(r2);
	rw_addr_dep_begin_two_endings_in_calls_helper2(r3);

	return 0;
}

/* BUGs: 1 */
static noinline void
proj_bdo_rw_addr_dep_end_two_endings_in_calls_helper1(int *r2)
{
	WRITE_ONCE(*r2, 0x21212120);
}

/* BUGs: 1 */
static noinline void
proj_bdo_rw_addr_dep_end_two_endings_in_calls_helper2(int *r3)
{
	WRITE_ONCE(*r3, 0x21212121);
}

static noinline int rw_addr_dep_end_two_endings_in_calls(void)
{
	int *r1;
	int *r2;
	int *r3;

	r1 = READ_ONCE(*x);

	r2 = &r1[8];
	r3 = &r1[5];

	proj_bdo_rw_addr_dep_end_two_endings_in_calls_helper1(r2);
	proj_bdo_rw_addr_dep_end_two_endings_in_calls_helper2(r3);

	return 0;
}

/* BUGs : 1 */
static int *noinline
proj_bdo_rw_addr_dep_begin_beg_and_end_in_calls_helper1(void)
{
	int *r1;

	r1 = READ_ONCE(*x);

	return r1;
}

static noinline void rw_addr_dep_begin_beg_and_end_in_calls_helper2(int *r3)
{
	WRITE_ONCE(*r3, 0x21212121);
}

static noinline int rw_addr_dep_begin_beg_and_end_in_calls(void)
{
	int *r2;
	int *r3;

	r2 = proj_bdo_rw_addr_dep_begin_beg_and_end_in_calls_helper1();

	r3 = &r2[82];

	rw_addr_dep_begin_beg_and_end_in_calls_helper2(r3);

	return 0;
}

static int *noinline rw_addr_dep_end_beg_and_end_in_calls_helper1(void)
{
	int *r1;

	r1 = READ_ONCE(*x);

	return r1;
}

/* BUGs: 1 */
static noinline void
proj_bdo_rw_addr_dep_end_beg_and_end_in_calls_helper2(int *r3)
{
	WRITE_ONCE(*r3, 0x21212121);
}

static noinline int rw_addr_dep_end_beg_and_end_in_calls(void)
{
	int *r2;
	int *r3;

	r2 = rw_addr_dep_end_beg_and_end_in_calls_helper1();

	r3 = &r2[8];

	proj_bdo_rw_addr_dep_end_beg_and_end_in_calls_helper2(r3);

	return 0;
}

/* TODO: Full to partial addr dep conversions
 * =============================================================================
 *
 * =============================================================================
 */

/*
 * =============================================================================
 * DynDepChecker
 * =============================================================================
 */

__attribute__((used, retain)) extern noinline int
proj_bdo_ddd_max_func_external(int, int);

static noinline int proj_bdo_ddd_max_func(int a, int b)
{
	return a > b ? a : b;
}

static noinline int proj_bdo_ddd_dep_through_extern_func(void)
{
	int *r1;
	int *r2;
	int r3;
	int r4;

	r1 = READ_ONCE(*x);

	r2 = r1 + 5;

	r3 = proj_bdo_ddd_max_func(*r1, *r2);

	r4 = READ_ONCE(r3);

	return r4;
}

static noinline int proj_bdo_ddd_dep_through_intrinsic(void)
{
	int *r1;
	int *r2;
	int r3;
	int r4;

	r1 = READ_ONCE(*x);

	r2 = r1 + 5;

	r3 = proj_bdo_ddd_max_func(*r1, *r2);

	r4 = READ_ONCE(r3);

	return r4;
}

static void noinline initialise_test_data(void)
{
	for (int i = 0; i < 10; ++i)
		test_arr_ptrs[i] = &test_arr[i];
}

int proj_bdo_run_tests(void)
{
	initialise_test_data();

	/* rr_addr_dep cases */
	proj_bdo_rr_addr_dep_begin_simple();
	proj_bdo_rr_addr_dep_end_simple();

	proj_bdo_rr_addr_dep_begin_call_ending();
	rr_addr_dep_end_call_ending();

	rr_addr_dep_begin_call_beginning();
	proj_bdo_rr_addr_dep_end_call_beginning();

	proj_bdo_rr_addr_dep_begin_call_dep_chain();
	proj_bdo_rr_addr_dep_end_call_dep_chain();

	proj_bdo_rr_addr_dep_begin_cond_dep_chain_full();
	proj_bdo_rr_addr_dep_end_cond_dep_chain_full();

	proj_bdo_rr_addr_dep_begin_cond_dep_chain_partial();
	proj_bdo_rr_addr_dep_end_cond_dep_chain_partial();

	proj_bdo_rr_addr_dep_begin_two_endings_simple();
	proj_bdo_rr_addr_dep_end_two_endings_simple();

	proj_bdo_rr_addr_dep_begin_two_endings_in_calls();
	rr_addr_dep_end_two_endings_in_calls();

	rr_addr_dep_begin_beg_and_end_in_calls();
	rr_addr_dep_end_beg_and_end_in_calls();

	/* rw_addr_dep cases */
	proj_bdo_rw_addr_dep_begin_simple();
	proj_bdo_rw_addr_dep_end_simple();

	proj_bdo_rw_addr_dep_begin_call_ending();
	rw_addr_dep_end_call_ending();

	rw_addr_dep_begin_call_beginning();
	proj_bdo_rw_addr_dep_end_call_beginning();

	proj_bdo_rw_addr_dep_begin_call_dep_chain();
	proj_bdo_rw_addr_dep_end_call_dep_chain();

	proj_bdo_rw_addr_dep_begin_cond_dep_chain_full();
	proj_bdo_rw_addr_dep_end_cond_dep_chain_full();

	proj_bdo_rw_addr_dep_begin_cond_dep_chain_partial();
	proj_bdo_rw_addr_dep_end_cond_dep_chain_partial();

	proj_bdo_rw_addr_dep_begin_two_endings_simple();
	proj_bdo_rw_addr_dep_end_two_endings_simple();

	proj_bdo_rw_addr_dep_begin_two_endings_in_calls();
	rw_addr_dep_end_two_endings_in_calls();

	rw_addr_dep_begin_beg_and_end_in_calls();
	rw_addr_dep_end_beg_and_end_in_calls();

	/* DynDepChecker */
	proj_bdo_ddd_dep_through_extern_func();
	proj_bdo_ddd_dep_through_intrinsic();

	if (proj_bdo_ddd_max_func_external(0, 1))
		printk("Hello!");

	return 0;
}

static int lkm_init(void)
{
	return proj_bdo_run_tests();
}

static void lkm_exit(void)
{
	pr_debug("Bye\n");
}

module_init(lkm_init);
module_exit(lkm_exit);

#endif /* _PROJ_BDO_DEP_CHAIN_TESTS */
