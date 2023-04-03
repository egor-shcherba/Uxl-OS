#include <debug/headers/cpu.h>

#define __DEBUG_SUB_HEADER__          "PIT"
#define __DEBUG_SUB_HEADER_COLOR__    __FG_WHITE__
#define __DEBUG_CURRENT_SUB_LOG__     __LOG_SUB_CPU_PIT__

#include <debug/qemu.h>
#include <sys/pit.h>
#include <sys/pic_i8259.h>
#include <sys/io.h>
#include <kernel/sched.h>

#define PIT_FREQ        1193182

#define PIT_CMD_PORT    0x43
#define PIT_CHANNEL0    0x40

void
pit_tick(void)
{
  schedule();
}

void
pit_init(void)
{
  uint16_t latch = PIT_FREQ / HZ;

  outb(PIT_CMD_PORT, 0x36);
  outb(PIT_CHANNEL0, latch & 0xFF);
  outb(PIT_CHANNEL0, latch >> 8);

  pic_i8259_set_handler(0, &pit_tick);

  dprintf("initialized.\n");
}
