#ifndef _MI_H_
#define _MI_H_

struct mi {
    void (*init)(void);
};

#define MI_LINK_SECTION(s) __attribute__ ((section (s))) __attribute__ ((used))

#define MI_INIT_MODULE(prio, i)                                     \
    static struct mi __mi_ ## i MI_LINK_SECTION(".mi." #prio) = {   \
        .init = i,                                                  \
    }

#endif
