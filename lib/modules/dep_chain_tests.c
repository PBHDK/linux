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
static const volatile int *foo = arr;
static const volatile int *xp, *bar;
static const volatile int *bar;

// DEP 1: address dependency within the same function - for breaking the begin annotation
static int noinline dep_1_same_function_begin(void)
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

// DEP 1: address dependency within the same function - for breaking the end annotation
static int noinline dep_1_same_function_end(void)
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

// DEP 2: address dependency accross two function. Dep begins in first function - for breaking begin annotation
static void noinline dep_2_begin_first_begin_helper(const volatile int *local_bar) {
	// End address dependency
	// y == x[42] == 0
	y = READ_ONCE(*local_bar);
}

static int noinline dep_2_begin_first_begin (void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];

  // copy bar into local var and dereference it in call
  dep_2_begin_first_begin_helper(bar);

	return 0;
}

// DEP 2: address dependency accross two function. Dep begins in first function - for breaking end annotation
static void noinline dep_2_begin_first_end_helper(const volatile int *local_bar) {
	// End address dependency
	// y == x[42] == 0
	y = READ_ONCE(*local_bar);
}

static int noinline dep_2_begin_first_end (void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];

  // copy bar into local var and dereference it in call
  dep_2_begin_first_end_helper(bar);

	return 0;
}

// DEP 3: address dependency accross two function. Dep begins in second function - for breaking beginn annotation
static const volatile int* noinline dep_3_begin_second_begin_helper(void) {
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];

  // copy bar and return it
  return bar;
}

static int noinline dep_3_begin_second_begin (void)
{
  // End address dependency
	// y == x[42] == 0
	y = READ_ONCE(*dep_3_begin_second_begin_helper());

  // alternative
  // const volatile int* yLocal = dep_2_begin_second_helper();
  // y = READ_ONCE(yLocal);

	return 0;
}

// DEP 3: address dependency accross two function. Dep begins in second function - for breaking end annotation
static const volatile int* noinline dep_3_begin_second_end_helper(void) {
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];

  // copy bar and return it
  return bar;
}

static int noinline dep_3_begin_second_end (void)
{
  // End address dependency
	// y == x[42] == 0
	y = READ_ONCE(*dep_3_begin_second_end_helper());

  // alternative
  // const volatile int* yLocal = dep_2_begin_second_helper();
  // y = READ_ONCE(yLocal);

	return 0;
}

// DEP 4: address dependency accross two function. Dep begins in first functions, runs through second function and ends in first function
static const volatile int* noinline dep_4_through_second_begin_helper(const volatile int *xpLocal) {
	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xpLocal[42];

  // copy bar and return it
  return bar;
}

static int noinline dep_4_through_second_begin (void)
{
  const volatile int *barLocal;

  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

  barLocal = dep_4_through_second_begin_helper(xp);

  // End address dependency
	// y == x[42] == 0
	y = READ_ONCE(*barLocal);

	return 0;
}

// DEP 4: address dependency accross two function. Dep begins in first functions, runs through second function and ends in first function
static const volatile int* noinline dep_4_through_second_end_helper(const volatile int *xpLocal) {
	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xpLocal[42];

  // copy bar and return it
  return bar;
}

static int noinline dep_4_through_second_end (void)
{
  const volatile int *barLocal;

  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

  barLocal = dep_4_through_second_end_helper(xp);

  // End address dependency
	// y == x[42] == 0
	y = READ_ONCE(*barLocal);

	return 0;
}

static int lkm_init(void)
{
  dep_1_same_function_begin();
	dep_1_same_function_end();
  dep_2_begin_first_begin();
	dep_2_begin_first_end();
  dep_3_begin_second_begin();
	dep_3_begin_second_end();
  dep_4_through_second_begin();
	dep_4_through_second_end();
  return 0;
}

static void lkm_exit(void)
{
	pr_debug("Bye\n");
}



module_init(lkm_init);
module_exit(lkm_exit);