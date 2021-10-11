#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_DESCRIPTION("Kernel module, which contains several dependeny chains used for testing CustomMemDep passes");
MODULE_AUTHOR("Paul Heidekruger");
MODULE_LICENSE("GPL");

// global declarations
static int y;
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
static const volatile int* noinline doitlk_rr_addr_dep_begin_3_helper(void) {
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
static const volatile int* noinline end_3_helper(void) {
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
static const volatile int* noinline begin_4_helper(volatile int *xpLocal) {
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
static const volatile int* noinline end_4_helper(volatile int *xpLocal) {
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

// // DEP 5: address dependency running through a loop within the same function -  for breaking the begin annotation
// static int noinline doitlk_rr_addr_dep_begin_5_helper(void)
// {
// 	int i = 0;
//   // Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	for(; i < 42; ++i) {
// 		// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 		bar = &xp[i];

// 		// End address dependency
// 		// y == x[42] == 0
// 		y = READ_ONCE(*bar);
// 	}
// 	return 0;
// }

// static int noinline doitlk_rr_addr_dep_begin_5(void)
// {
// 	int i = 0;
//   // Begin address dependency
// 	// xp == foo && *x == foo[0] after assignment
// 	xp = READ_ONCE(foo);

// 	for(; i < 42; ++i) {
// 		// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
// 		bar = &xp[i];

// 		// End address dependency
// 		// y == x[42] == 0
// 		y = READ_ONCE(*bar);
// 	}
// 	return 0;
// }

// Begin addr dep 6: rw address dependency within the same function - for breaking the begin annotation
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

// End addr dep 6: rw address dependency within the same function - for breaking the end annotation
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
		y = READ_ONCE(*bar);
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
		y = READ_ONCE(*bar);
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
		y = READ_ONCE(*bar);
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
		y = READ_ONCE(*bar);
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
		y = READ_ONCE(*bar);
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
		y = READ_ONCE(*bar);
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
		y = READ_ONCE(*bar);
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
		y = READ_ONCE(*bar);
	}
	return 0;
}

static int lkm_init(void)
{
	// rr addr deps
  doitlk_rr_addr_dep_begin_1();
	doitlk_rr_addr_dep_end_1();
  doitlk_rr_addr_dep_begin_2();
	rr_addr_dep_end_2();
	rr_addr_dep_begin_3();
	doitlk_rr_addr_dep_end_3();
	doitlk_rr_addr_dep_begin_4();
	doitlk_rr_addr_dep_end_4();
	doitlk_rr_addr_dep_begin_5();
	doitlk_rr_addr_dep_end_5();
	doitlk_rr_addr_dep_begin_6();
	doitlk_rr_addr_dep_end_6();
	// dep_5_same_function_loop_begin();
	// dep_5_same_function_loop_end();
	// TODO: WRITE_ONCE() for second access
	// TODO: Address dep that runs through control dep and ends afterwards
	// TODO: add two deps with same beginning

	// rw addr deps
	doitlk_rw_addr_dep_begin_1();
	doitlk_rw_addr_dep_end_1();

	// ctrl deps
	doitlk_ctrl_dep_begin_1();
	doitlk_ctrl_dep_end_1();
	doitlk_ctrl_dep_begin_2();
	doitlk_ctrl_dep_end_2();
	doitlk_ctrl_dep_begin_3();
	doitlk_ctrl_dep_end_3();
	doitlk_ctrl_dep_begin_4();
	doitlk_ctrl_dep_end_4();
	
  return 0;
}

static void lkm_exit(void)
{
	pr_debug("Bye\n");
}



module_init(lkm_init);
module_exit(lkm_exit);