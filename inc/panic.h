#ifndef _PANIC_H
#define _PANIC_H

#include <sys/x86.h>
#include <debug/qemu.h>
#include <debug/colors.h>

#define PANIC(...)                                                         \
{                                                                          \
  pr_err(__VA_ARGS__);                                                     \
  pr_err("%sin file '%s'::%s on line %d\n",                                \
    __FG_WHITE__, __FILE__, __func__, __LINE__);                           \
                                                                           \
  cpu_disable_interrupt();                                                 \
  cpu_halt();                                                              \
}

#endif /* NOT _PANIC_H */