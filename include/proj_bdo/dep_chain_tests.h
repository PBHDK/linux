#define _PROJ_BDO_MAX 1

static int arr[100];
static int *x = &arr[0];
static int *y = &arr[21];
static int *z = &arr[42];

static int **foo = (int **)&x;
static int **bar = (int **)&y;
static int **baz = (int **)&z;

int proj_bdo_run_tests(void);
