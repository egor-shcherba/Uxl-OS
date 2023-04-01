#define __DEBUG_HEADER__        "KERNEL"
#define __DEBUG_HEADER_COLOR__  __FG_GREEN__
#define __DEBUG_SUB_HEADER__    "MAIN"

#include <debug/qemu.h>
#include <sys/gdt.h>
#include <sys/idt.h>
#include <sys/pic_i8259.h>


void
main(void)
{
  dprintf("initilization...\n");

  gdt_init();
  idt_init();
  pic_i8259_init();

  dprintf("all subsystem initialized.\n");

  while (1)
    ;
}
