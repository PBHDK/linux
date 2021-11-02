#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_DESCRIPTION("Kernel module, which contains several dependeny chains used for testing CustomMemDep passes");
MODULE_AUTHOR("Paul Heidekruger");
MODULE_LICENSE("GPL");

// global declarations
static int y, z;
static int arr[50];
// implicitly convert arr to int*
static volatile int *foo = arr;
static volatile int *xp, *bar;
static volatile int *bar;

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

// Begin addr dep 9: two address dependencies with same beginning within the same function - for breaking the begin annotation
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

// End addr dep 9: two address dependencies with same beginning within the same function - for breaking the end annotation
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

// Begin addr dep 9: two address dependencies with same beginning within the same function - for breaking the begin annotation
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

// End addr dep 9: two address dependencies with same beginning within the same function - for breaking the end annotation
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

// Begin ctrl dep 1: control dependency within the same function - independent - for breaking the begin annotation
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

// End ctrl dep 1: control dependency within the same function - independent condition - for breaking the end annotation
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

static int lkm_init(void)
{
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
	// TODO in and out different chains
	// simple case, end in if condition
	doitlk_rr_addr_dep_begin_5();
	doitlk_rr_addr_dep_end_5();
	// Simple Case - Chain Through If-Else
	doitlk_rr_addr_dep_begin_6();
	doitlk_rr_addr_dep_end_6();
	// Simple Case - Chain Through If
	doitlk_rr_addr_dep_begin_7();
	doitlk_rr_addr_dep_end_7();
	// TODO duplicate
	// simple case, fan out
	doitlk_rr_addr_dep_begin_8();
	doitlk_rr_addr_dep_end_8();
	// TODO in and out but different chains
	// TODO chain fanning in
	// TODO chain fanning out
	// TODO chain fanning in and out
	// TODO doitlk example

	// rw addr deps
	doitlk_rw_addr_dep_begin_1();
	doitlk_rw_addr_dep_end_1();
	doitlk_rw_addr_dep_begin_2();
	rw_addr_dep_end_2();
	rw_addr_dep_begin_3();
	doitlk_rw_addr_dep_end_3();
	doitlk_rw_addr_dep_begin_4();
	doitlk_rw_addr_dep_end_4();
	// dep 5 ommitted since we can't use WRITE_ONCE() as if condition
	doitlk_rw_addr_dep_begin_6();
	doitlk_rw_addr_dep_end_6();
	doitlk_rw_addr_dep_begin_7();
	doitlk_rw_addr_dep_end_7();
	doitlk_rw_addr_dep_begin_8();
	doitlk_rw_addr_dep_end_8();
	// TODO in and out but different chains
	// TODO chain fanning in
	// TODO chain fanning out
	// TODO chain fanning in and out
	// TODO doitlk example

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
	// TODO all cases from above
	
  return 0;
}

static void lkm_exit(void)
{
	pr_debug("Bye\n");
}



module_init(lkm_init);
module_exit(lkm_exit);