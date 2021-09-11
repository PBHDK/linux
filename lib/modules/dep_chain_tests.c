#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_DESCRIPTION("Kernel module, which contains several dependeny chains used for testing CustomMemDep passes");
MODULE_AUTHOR("Paul Heidekruger");
MODULE_LICENSE("GPL");

static int lkm_init(void)
{
  return 0;
}

static void lkm_exit(void)
{
	pr_debug("Bye\n");
}

module_init(lkm_init);
module_exit(lkm_exit);