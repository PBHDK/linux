#define _PROJ_BDO_MAX 1

// global declarations
static int *x, *y, *z;
static int arr[100];
// implicitly convert arr to int*
static int **foo = (int **)&arr;
static int **bar;
static int **baz;

int proj_bdo_run_tests(void);
