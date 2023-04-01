#include <debug/headers/cpu.h>

#define __DEBUG_SUB_HEADER__        "PIC"
#define __DEBUG_CURRENT_SUB_LOG__   __LOG_SUB_CPU_PIC__

#include <debug/qemu.h>
#include <sys/pic_i8259.h>
#include <sys/io.h>
#include <sys/x86.h>
#include <stdint.h>

#define MASTER_CMD_PORT   0x20
#define MASTER_DATA_PORT  0x21

#define SLAVE_CMD_PORT    0xA0
#define SLAVE_DATA_PORT   0xA1

#define CMD_EOI           0x20

#define ICW1_INIT         0x10
#define ICW1_ICW4         0x01
#define ICW4_8086         0x01

#define HANDLERS_SIZE     15

void (*handlers[HANDLERS_SIZE])(void) = { NULL };

void
pic_i8259_init(void)
{
  outb(MASTER_CMD_PORT, ICW1_INIT | ICW1_ICW4);
  outb(SLAVE_CMD_PORT, ICW1_INIT | ICW1_ICW4);

  outb(MASTER_DATA_PORT, 0x20);
  outb(SLAVE_DATA_PORT,  0x28);

  outb(MASTER_DATA_PORT, 2);
  outb(SLAVE_DATA_PORT, 4);

  outb(MASTER_DATA_PORT, ICW4_8086);
  outb(SLAVE_DATA_PORT, ICW4_8086);

  outb(MASTER_DATA_PORT, 0xFF);
  outb(SLAVE_DATA_PORT, 0xFF);

  dprintf("initialized\n");
}

void
pic_i8259_irq_enable(int irq_line)
{
  uint16_t port = MASTER_DATA_PORT;

  if (irq_line >= 8)
    port = SLAVE_DATA_PORT;

  uint8_t data = inb(port) & ~(1 << (irq_line % 8));
  outb(port, data);

  dprintf("enable line %u\n", irq_line);
}

void
pic_i8259_irq_disable(int irq_line)
{
  uint16_t port = MASTER_DATA_PORT;

  if (irq_line >= 8)
    port = SLAVE_DATA_PORT;

  uint8_t data = inb(port) | (1 << (irq_line % 8));
  outb(port, data);

  dprintf("disable line %u\n", irq_line);
}

void
pic_i8259_set_handler(int irq_line, void (*handler)(void))
{
  handlers[irq_line] = handler;

  dprintf("set handler LINE %u ADDR %p\n", irq_line, handler)

  pic_i8259_irq_enable(irq_line);
}

void
pic_i8259_main_handler(struct regs *regs)
{
  pr_extra("recevies an interrupt requets %u\n", regs->intr_number);

  if (handlers[regs->intr_number] != NULL)
      handlers[regs->intr_number]();

  if (regs->intr_number > 7)
    outb(SLAVE_CMD_PORT, CMD_EOI);

  outb(MASTER_CMD_PORT, CMD_EOI);
}
