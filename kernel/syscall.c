#include <debug/headers/syscall.h>
#include <debug/qemu.h>
#include <sys/x86.h>

void
syscall_main(struct regs *regs)
{
  (void) regs;
  dprintf("received syscall number %u\n", regs->eax);
}

