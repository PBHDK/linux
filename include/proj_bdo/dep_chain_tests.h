#define MAX 1

// global declarations
static int *x, *y, *z;
static int arr[100];
// implicitly convert arr to int*
static int **foo = (int **)&arr;
static int **bar;
static int **baz;