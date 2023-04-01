#include <debug/headers/cpu.h>

#define __DEBUG_SUB_HEADER__        "EXCEPTION"
#define __DEBUG_CURRENT_SUB_LOG__   __LOG_SUB_CPU_EXCEPTION__

#include <debug/qemu.h>
#include <sys/x86.h>

static inline const char*
get_exception_name(int num)
{
  switch (num)
    {
      case 0x00: return "Divide By Zero";
      case 0x01: return "Debug";
      case 0x02: return "No Mascalable Interrupt";
      case 0x03: return "Breakpoint";
      case 0x04: return "Overflow";
      case 0x05: return "Bound-Range";
      case 0x06: return "Invalid Opcode";
      case 0x07: return "Device Not Avalible";
      case 0x08: return "Double Faults";
      case 0x0A: return "Invalid TSS";
      case 0x0B: return "Segment Not Present";
      case 0x0C: return "Stack Segment";
      case 0x0D: return "General Protection";
      case 0x0E: return "Page Faults";
      case 0x10: return "x87 Floating Point";
      case 0x11: return "Aligned Check";
      case 0x12: return "Machine Check";
      case 0x13: return "Simd";
    }

  return "UNKNOW";
}

static inline void
log_exception_user(struct regs *regs)
{
  dprintf("'%s' num %d in 'USER SPACE'\n",
    get_exception_name(regs->intr_number), regs->intr_number);

  qprintf("CS %p EIP %p EFALGS %p\n", regs->cs, regs->eip, regs->eflags);
  qprintf("SS %p ESP %p\n", regs->ss, regs->esp);
}

static inline void
log_exception_kernel(struct regs *regs)
{
  pr_err("'%s' num %d in 'KERNEL SPACE'\n",
    get_exception_name(regs->intr_number), regs->intr_number);

  pr_err("EAX %p EBX %p ECX %p EDX %p\n",
    regs->eax, regs->ebx, regs->ecx, regs->edx);

  pr_err("ESI %p EDI %p\n", regs->esi, regs->edi);

  pr_err("DS %p ES %p GS %p FS %p\n",
    regs->ds, regs->es, regs->gs, regs->fs);

  pr_err("CS %p EIP %p EFALGS %p\n", regs->cs, regs->eip, regs->eflags);
}

void
exception_handler(struct regs *regs)
{
  if (regs->cs != 0x8)
    log_exception_user(regs);
  else
    log_exception_kernel(regs);

  cpu_disable_interrupt();
  cpu_halt();
}
