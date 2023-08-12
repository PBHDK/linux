#define _PROJ_BDO_MAX 1

static int arr[10] = { 0x42424241, 0x42424242, 0x42424243, 0x42424244,
		       0x42424245, 0x42424246, 0x42424247, 0x42424248,
		       0x42424249, 0x4242424A };
static int *x = &arr[0];
static int *y = &arr[3];
static int *z = &arr[6];

static int **foo = (int **)&x;
static int **bar = (int **)&y;
static int **baz = (int **)&z;

int proj_bdo_run_tests(void);
