#ifndef _X86_H
#define _x86_H

#include <stdint.h>

struct regs {
  uint32_t fs, gs, es, ds;
  uint32_t ebp, esi, edi, edx, ecx, ebx, eax;

  uint32_t intr_number;
  uint32_t error_code;

  uint32_t eip;
  uint32_t cs;
  uint32_t eflags;

  uint32_t esp;
  uint32_t ss;
};

static inline void
cpu_disable_interrupt(void)
{
  __asm__ volatile ("cli");
}

static inline void
cpu_enable_interrupt(void)
{
  __asm__ volatile ("sti");
}

static inline void
cpu_halt(void)
{
  __asm__ volatile ("hlt;");
}

#endif /* NOT _X86_H */