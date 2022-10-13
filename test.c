#define MDC_IMPLEMENTATION
#include "mdc.h"
#include "foo.h"

int main()
{
    mdc_init();
    malloc(64);
    foo();

    mdc_debug();
    mdc_destroy();
}
