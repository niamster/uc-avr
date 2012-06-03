#include <inttypes.h>

#include <mi/mi.h>

extern struct mi __mi_start[], __mi_end[];

static void mi_init(void)  __attribute__((constructor)) __attribute__ ((used));

static
void mi_init(void)
{
    struct mi *s = (struct mi *)__mi_start;
    struct mi *e = (struct mi *)__mi_end;

    while (s < e) {
        s->init();
        ++s;
    }
}
