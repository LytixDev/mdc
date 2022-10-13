#define MDC_IMPLEMENTATION
#include "mdc.h"

int main()
{
    mdc_init();
    malloc(64);

    mdc_debug();
    mdc_destroy();
}
