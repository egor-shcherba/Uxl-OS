#define __DEBUG_HEADER__        "KERNEL"
#define __DEBUG_HEADER_COLOR__  __FG_GREEN__
#define __DEBUG_SUB_HEADER__    "MAIN"

#include <debug/qemu.h>
#include <sys/gdt.h>
#include <sys/idt.h>
#include <sys/pic_i8259.h>
#include <sys/pit.h>
#include <kernel/sched.h>
#include <sys/x86.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

void
init_process(void)
{
  for (;;)
    ;
}

void
main(void)
{
  dprintf("initilization...\n");

  gdt_init();
  idt_init();
  pmm_init();
  vmm_init();
  sched_init();

  pic_i8259_init();
  pit_init();

  dprintf("all subsystem initialized.\n");

  task_create_init_proc("init process", &init_process);

  sched_enable();
  /* never reached */
}
