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

// DEP 1: address dependency within the same function
static int dep_1_same_function(void)
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

// DEP 2: address dependency accross two function. Dep begins in first function
static void noinline dep_2_begin_first_helper(const volatile int *local_bar) {
	// End address dependency
	// y == x[42] == 0
	y = READ_ONCE(*local_bar);
}

static int dep_2_begin_first (void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];

  // copy bar into local var and dereference it in call
  dep_2_begin_first_helper(bar);

	return 0;
}

// DEP 3: address dependency accross two function. Dep begins in second function
static const volatile int* noinline dep_3_begin_second_helper(void) {
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xp[42];

  // copy bar and return it
  return bar;
}

static int dep_3_begin_second (void)
{
  // End address dependency
	// y == x[42] == 0
	y = READ_ONCE(*dep_3_begin_second_helper());

  // alternative
  // const volatile int* yLocal = dep_2_begin_second_helper();
  // y = READ_ONCE(yLocal);

	return 0;
}

// DEP 3: address dependency accross two function. Dep begins in first functions, runs through second function and ends in first function
static const volatile int* noinline dep_4_through_second_helper(const volatile int *xpLocal) {
	// bar == x + 42 && bar == foo + 42 && *bar == x[42] == 0
	bar = &xpLocal[42];

  // copy bar and return it
  return bar;
}

static int dep_4_through_second (void)
{
  // Begin address dependency
	// xp == foo && *x == foo[0] after assignment
	xp = READ_ONCE(foo);

  const volatile int *barLocal = dep_4_through_second_helper(xp);

  // End address dependency
	// y == x[42] == 0
	y = READ_ONCE(*barLocal);

	return 0;
}

static int lkm_init(void)
{
  dep_1_same_function();
  dep_2_begin_first();
  dep_3_begin_second();
  dep_4_through_second();
  return 0;
}

static void lkm_exit(void)
{
	pr_debug("Bye\n");
}



module_init(lkm_init);
module_exit(lkm_exit);