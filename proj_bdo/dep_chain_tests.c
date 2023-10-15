#include "linux/printk.h"
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

#define MAX 4242

static int test_arr[10] = { 0x42424241, 0x42424242, 0x42424243, 0x42424244,
			    0x42424245, 0x42424246, 0x42424247, 0x42424248,
			    0x42424249, 0x4242424A };
static int *test_arr_ptrs[10];
static int **x = &test_arr_ptrs[0];
static int y;
extern volatile int c;

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

static noinline int proj_bdo_rr_addr_dep_begin_cond_dep_chain_dep_cond(void)
{
	int *r1;
	int *r2;
	int r3;

	r1 = READ_ONCE(*x);

	if (r1 == (int *)0x424242) {
		r2 = &r1[5];
		printk("hello");
	} else
		r2 = &r1[8];

	r3 = READ_ONCE(*r2);

	return 0;
}

/* BUGs: 1 */
static noinline int proj_bdo_rr_addr_dep_end_cond_dep_chain_dep_cond(void)
{
	int *r1;
	int *r2;
	int r3;

	r1 = READ_ONCE(*x);

	if (r1 == (int *)0x424242) {
		r2 = &r1[5];
		printk("hello");
	} else
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
 * READ_ONCE() -> WRITE_ONCE() Control Dependencies 
 * =============================================================================
 */

/* BUGs: 1 */
/* Begin ctrl dep 1: memory-barriers.txt case 1, control dependency within the same function */
static noinline int doitlk_ctrl_dep_begin_1(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);

	if (r1) {
		r2 = &r1[4];

		WRITE_ONCE(y, 42);
	}

	return 0;
}

/* BUGs: 1 */
static noinline int doitlk_ctrl_dep_end_1(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);

	if (r1) {
		r2 = &r1[4];

		WRITE_ONCE(y, 42);
	}

	return 0;
}

/* BUGs: 0 */
static noinline int doitlk_ctrl_dep_begin_2(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);

	if (x[4]) {
		r2 = &r1[8];

		WRITE_ONCE(y, *r2);
	}

	return 0;
}

/* BUGs: 0 */
static noinline int doitlk_ctrl_dep_end_2(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);

	if (x[4]) {
		r2 = &r1[8];

		WRITE_ONCE(y, *r2);
	}

	return 0;
}

/* BUGs: 0 */
static noinline int doitlk_ctrl_dep_begin_3(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);

	if (0) {
		r2 = &r1[42];

		WRITE_ONCE(y, 44);
	}

	return 0;
}

/* BUGs: 0 */
static noinline int doitlk_ctrl_dep_end_3(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);

	if (0) {
		r2 = &r1[42];

		WRITE_ONCE(y, 44);
	}

	return 0;
}

static noinline int doitlk_ctrl_dep_begin_4(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);
	if (r1) {
		r2 = &r1[6];

		WRITE_ONCE(*r2, 42);
	}
	return 0;
}

static noinline int doitlk_ctrl_dep_end_4(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);
	if (r1) {
		r2 = &r1[6];

		WRITE_ONCE(*r2, 42);
	}
	return 0;
}

// Begin ctrl dep 5: control dependency with end in for loop -  for breaking the begin annotation
static noinline int doitlk_ctrl_dep_begin_5(void)
{
	int i = 0;
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);

	for (; i < 10; ++i) {
		r2 = &r1[i];

		WRITE_ONCE(*r2, 1);
	}
	return 0;
}

static noinline int doitlk_ctrl_dep_end_5(void)
{
	int i = 0;
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);

	for (; i < 10; ++i) {
		r2 = &r1[i];

		WRITE_ONCE(*r2, 1);
	}
	return 0;
}

static noinline int doitlk_ctrl_dep_begin_6(void)
{
	int *r1;

	r1 = READ_ONCE(*x);
	if (*r1 % MAX == 0)
		WRITE_ONCE(*r1, y);
	return 0;
}

static noinline int doitlk_ctrl_dep_end_6(void)
{
	int *r1;

	r1 = READ_ONCE(*x);
	if (*r1 % MAX == 0)
		WRITE_ONCE(*r1, y);
	return 0;
}

static noinline int doitlk_ctrl_dep_begin_7(void)
{
	int *r1;

	r1 = READ_ONCE(*x);
	if (r1 || 1 > 0)
		WRITE_ONCE(y, 1);
	return 0;
}

static noinline int doitlk_ctrl_dep_end_7(void)
{
	int *r1;

	r1 = READ_ONCE(*x);
	if (r1 || 1 > 0)
		WRITE_ONCE(y, 1);
	return 0;
}

static noinline int doitlk_ctrl_dep_begin_8(void)
{
	int *r1;

	r1 = READ_ONCE(*x);
	if (*r1 > 0) {
		if (*r1 > 42)
			WRITE_ONCE(y, 42);
		else
			WRITE_ONCE(y, 0);
	}
	return 0;
}

static noinline int doitlk_ctrl_dep_end_8(void)
{
	int *r1;

	r1 = READ_ONCE(*x);
	if (*r1 > 0) {
		if (*r1 > 42)
			WRITE_ONCE(y, 42);
		else
			WRITE_ONCE(y, 0);
	}
	return 0;
}

static noinline int doitlk_ctrl_dep_begin_9(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*(x + 4));
	r2 = READ_ONCE(*x);
	if (*r1 > *r2)
		WRITE_ONCE(y, 0);
	return 0;
}

static noinline int doitlk_ctrl_dep_end_9(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*(x + 4));
	r2 = READ_ONCE(*x);
	if (*r1 > *r2)
		WRITE_ONCE(y, 0);
	return 0;
}

static noinline int doitlk_ctrl_dep_begin_10(void)
{
	int *r1;
	int *r2;
	int *r3;

	r1 = READ_ONCE(*(x + 1));
	r2 = READ_ONCE(*(x + 2));
	r3 = READ_ONCE(*(x + 3));

	if (r1 > r2 || r3 < r1)
		WRITE_ONCE(y, 1);

	return 0;
}

static noinline int doitlk_ctrl_dep_end_10(void)
{
	int *r1;
	int *r2;
	int *r3;

	r1 = READ_ONCE(*(x + 1));
	r2 = READ_ONCE(*(x + 2));
	r3 = READ_ONCE(*(x + 3));

	if (r1 > r2 || r3 < r1)
		WRITE_ONCE(y, 1);

	return 0;
}

static noinline int doitlk_ctrl_dep_begin_11(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);
	if (*r1) {
		r2 = READ_ONCE(*(x + 9));
		if (*r2)
			WRITE_ONCE(*r2, 1);
	}
	return 0;
}

static noinline int doitlk_ctrl_dep_end_11(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);
	if (*r1) {
		r2 = READ_ONCE(*(x + 9));
		if (*r2)
			WRITE_ONCE(*r2, 1);
	}
	return 0;
}

static noinline int ctrl_dep_12_begin_helper(void)
{
	int *r1;

	r1 = READ_ONCE(*x);
	if (r1)
		return 21;
	else
		return 42;
}

static noinline int doitlk_ctrl_dep_begin_12(void)
{
	WRITE_ONCE(y, ctrl_dep_12_begin_helper());
	return 0;
}

static noinline int ctrl_dep_12_end_helper(void)
{
	int *r1;

	r1 = READ_ONCE(*x);
	if (r1)
		return 21;
	else
		return 42;
}

static noinline int doitlk_ctrl_dep_end_12(void)
{
	WRITE_ONCE(y, ctrl_dep_12_end_helper());
	return 0;
}

static int *noinline ctrl_dep_begin_13_helper(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);

	r2 = &r1[4];

	return r2;
}

static noinline int doitlk_ctrl_dep_begin_13(void)
{
	int *r1;

	r1 = ctrl_dep_begin_13_helper();
	if (r1)
		WRITE_ONCE(*r1, 1);

	return 0;
}

static int *noinline ctrl_dep_end_13_helper(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);

	r2 = &r1[4];

	return r2;
}

static noinline int doitlk_ctrl_dep_end_13(void)
{
	int *r1;

	r1 = ctrl_dep_end_13_helper();

	if (r1)
		WRITE_ONCE(*r1, 1);

	return 0;
}

static int *noinline ctrl_dep_begin_14_helper1(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);

	r2 = &r1[6];

	return r2;
}

static noinline void ctrl_dep_begin_14_helper2(int *r1)
{
	WRITE_ONCE(*r1, y);
}

static noinline int doitlk_ctrl_dep_begin_14(void)
{
	int *r1;

	r1 = ctrl_dep_begin_14_helper1();
	if (r1)
		ctrl_dep_begin_14_helper2(r1);

	return 0;
}

static int *noinline ctrl_dep_end_14_helper1(void)
{
	int *r1;
	int *r2;

	r1 = READ_ONCE(*x);

	r2 = &r1[6];

	return r2;
}

static noinline void ctrl_dep_end_14_helper2(int *r1)
{
	WRITE_ONCE(*r1, y);
}

static noinline int doitlk_ctrl_dep_end_14(void)
{
	int *r1;

	r1 = ctrl_dep_end_14_helper1();

	if (r1)
		ctrl_dep_end_14_helper2(r1);

	return 0;
}

static noinline void ctrl_dep_begin_15_helper(int *r1)
{
	WRITE_ONCE(*r1, y);
}

static noinline int doitlk_ctrl_dep_begin_15(void)
{
	int *r1;

	r1 = READ_ONCE(*x);

	if (r1)
		ctrl_dep_begin_15_helper(r1);

	return 0;
}

static noinline void ctrl_dep_end_15_helper(int *r1)
{
	WRITE_ONCE(*r1, y);
}

static noinline int doitlk_ctrl_dep_end_15(void)
{
	int *r1;

	r1 = READ_ONCE(*x);

	if (r1)
		ctrl_dep_end_15_helper(r1);

	return 0;
}

static noinline int doitlk_ctrl_dep_begin_16(void)
{
	int *r1;
loop:
	r1 = READ_ONCE(*x);

	if (r1)
		goto end;

	// do something with x
	x++;
	goto loop;

end:
	WRITE_ONCE(y, *r1);

	return *r1;
}

static noinline int doitlk_ctrl_dep_end_16(void)
{
	int *r1;
loop:
	r1 = READ_ONCE(*x);

	if (r1)
		goto end;

	// do something with x
	x++;
	goto loop;

end:
	WRITE_ONCE(y, *r1);

	return *r1;
}

static noinline int doitlk_ctrl_dep_begin_17(void)
{
	int *r1;
loop:
	r1 = READ_ONCE(*x);

	if (*r1 == *x[4]) {
		WRITE_ONCE(y, *r1);
		return *r1;
	}

	*r1 += 1;
	goto loop;
}

static noinline int doitlk_ctrl_dep_end_17(void)
{
	int *r1;
loop:
	r1 = READ_ONCE(*x);

	if (*r1 == *x[4]) {
		WRITE_ONCE(y, *r1);
		return *r1;
	}

	*r1 += 1;
	goto loop;
}

// Begin ctrl dep 18: memory-barriers.txt case 2, constant dependent condition
static noinline int doitlk_ctrl_dep_begin_18(void)
{
	int *r1;

	r1 = READ_ONCE(*x);

	if (*r1 % MAX == 0)
		WRITE_ONCE(*r1, y);
	return 0;
}

// End ctrl dep 18: memory-barriers.txt case 2, constant dependent condition
static noinline int doitlk_ctrl_dep_end_18(void)
{
	int *r1;

	r1 = READ_ONCE(*x);

	if (*r1 % MAX == 0)
		WRITE_ONCE(*r1, y);
	return 0;
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

	proj_bdo_rr_addr_dep_begin_cond_dep_chain_dep_cond();
	proj_bdo_rr_addr_dep_end_cond_dep_chain_dep_cond();

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

	/* Ctrl dependencies */
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
	doitlk_ctrl_dep_begin_12();
	doitlk_ctrl_dep_end_12();
	doitlk_ctrl_dep_begin_13();
	doitlk_ctrl_dep_end_13();
	doitlk_ctrl_dep_begin_14();
	doitlk_ctrl_dep_end_14();
	doitlk_ctrl_dep_begin_15();
	doitlk_ctrl_dep_end_15();
	doitlk_ctrl_dep_begin_16();
	doitlk_ctrl_dep_end_16();
	doitlk_ctrl_dep_begin_17();
	doitlk_ctrl_dep_end_17();
	doitlk_ctrl_dep_begin_18();
	doitlk_ctrl_dep_end_18();

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
