# mdc - Memory Debugger for C
Aims to make catching memory leaks trivial.

### Setup
Clone the repo recursively to get the submodule dependency:
```sh
git clone --recursive git@github.com:LytixDev/mdc.git
```

Include the header file into your C project:
```c
#define MDC_IMPLEMENTATION /* defining the implemention once is sufficient */
#include "mdc.h"
```

Then you will need to initialize, and later destroy, mdc by doing:
```c
int main()
{
    mdc_init();
    ...
    mdc_destroy();
}
```

This creates macros overriding the standard malloc, calloc, realloc and free functions provided by stdlib.
```c
#define malloc(m) mdc_malloc(m, __FILE__, __LINE__)
#define calloc(m, n) mdc_calloc(m, n, __FILE__, __LINE__)
#define realloc(m, n) mdc_realloc(m, n, __FILE__, __LINE__)
#define free(m) mdc_free(m, __FILE__, __LINE__)
```

Internally, all allocations are logged to a hashtable. Once an allocation is freed, it is removed from the hastable.

To print out all non-freed allocation, simply call:
```c
mdc_debug();
```

See `test.c` for more.
