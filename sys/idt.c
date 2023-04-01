#include <debug/headers/cpu.h>

#define __DEBUG_SUB_HEADER__        "IDT"
#define __DEBUG_SUB_HEADER_COLOR__   __FG_MAGENTA__
#define __DEBUG_CURRENT_SUB_LOG__   __LOG_SUB_CPU_IDT__

#include <debug/qemu.h>
#include <sys/idt.h>
#include <sys/exception.h>
#include <sys/irq.h>
#include <stdint.h>
#include <string.h>

extern void syscall_enter(void);

struct idt_entry {
  uint32_t handler_low  : 16;
  uint32_t code_sel     : 16;
  uint32_t ignored      : 8;
  uint32_t type         : 5;
  uint32_t dpl          : 2;
  uint32_t p            : 1;
  uint32_t handler_high : 8;
};

struct idt_pointer {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

#define IDT_TABLE_SIZE  256
static struct idt_entry table[IDT_TABLE_SIZE] = { 0x0 };
static struct idt_pointer idt_pointer;

#define TRAP_GATE   0xF
#define INTR_GATE   0xE

#define DPL_KERN    0x0
#define DPL_USER    0x3

static inline void
log_register_interrupt(uint32_t num, void (*handler)(void), uint32_t type, uint8_t dpl)
{
  const char *type_str  = "INTR GATE";

  if (type == TRAP_GATE)
    type_str = "TRAP GATE";

  dprintf("register interrupt %#.2x ADDR %p, TYPE '%s' DPL %.2x\n",
    num, handler, type_str, dpl); 
}

static void
register_interrupt(uint32_t num, void (*handler)(void), uint32_t type, uint8_t dpl)
{
  struct idt_entry *entry = &table[num];

  entry->handler_low  = (uint32_t) handler & 0xFFFF;
  entry->handler_high = (uint32_t) handler >> 16;

  entry->code_sel     = 0x8;
  entry->type         = type;
  entry->ignored      = 0;
  entry->dpl          = dpl;
  entry->p            = 1;

  log_register_interrupt(num, handler, type, dpl);
}

void
idt_init(void)
{
  for (int i = 0; i < IDT_TABLE_SIZE; i++)
    memset(&table[i], 0, sizeof(struct idt_entry));

  register_interrupt(0x00, &exc_divide_error, TRAP_GATE, DPL_KERN);
  register_interrupt(0x01, &exc_debug, TRAP_GATE, DPL_KERN);
  register_interrupt(0x02, &exc_nmi, TRAP_GATE, DPL_KERN);
  register_interrupt(0x03, &exc_breakpoint, TRAP_GATE, DPL_KERN);
  register_interrupt(0x04, &exc_overflow, TRAP_GATE, DPL_KERN);
  register_interrupt(0x05, &exc_boundrange, TRAP_GATE, DPL_KERN);
  register_interrupt(0x06, &exc_inv_opcode, TRAP_GATE, DPL_KERN);
  register_interrupt(0x07, &exc_no_dev, TRAP_GATE, DPL_KERN);
  register_interrupt(0x08, &exc_double_fault, TRAP_GATE, DPL_KERN);
  register_interrupt(0x0A, &exc_inv_tss, TRAP_GATE, DPL_KERN);
  register_interrupt(0x0B, &exc_seg_not_present, TRAP_GATE, DPL_KERN);
  register_interrupt(0x0C, &exc_ss, TRAP_GATE, DPL_KERN);
  register_interrupt(0x0D, &exc_general_prot, TRAP_GATE, DPL_KERN);
  register_interrupt(0x0E, &exc_page_fault, TRAP_GATE, DPL_KERN);
  register_interrupt(0x10, &exc_x87, TRAP_GATE, DPL_KERN);
  register_interrupt(0x11, &exc_aligned_check, TRAP_GATE, DPL_KERN);
  register_interrupt(0x12, &exc_machine_check, TRAP_GATE, DPL_KERN);
  register_interrupt(0x13, &exc_simd, TRAP_GATE, DPL_KERN);

  register_interrupt(0x20, &irq_0, INTR_GATE, DPL_KERN);
  register_interrupt(0x21, &irq_1, INTR_GATE, DPL_KERN);
  register_interrupt(0x22, &irq_2, INTR_GATE, DPL_KERN);
  register_interrupt(0x23, &irq_3, INTR_GATE, DPL_KERN);
  register_interrupt(0x24, &irq_4, INTR_GATE, DPL_KERN);
  register_interrupt(0x25, &irq_5, INTR_GATE, DPL_KERN);
  register_interrupt(0x26, &irq_6, INTR_GATE, DPL_KERN);
  register_interrupt(0x27, &irq_7, INTR_GATE, DPL_KERN);
  register_interrupt(0x28, &irq_8, INTR_GATE, DPL_KERN);
  register_interrupt(0x29, &irq_9, INTR_GATE, DPL_KERN);
  register_interrupt(0x2A, &irq_10, INTR_GATE, DPL_KERN);
  register_interrupt(0x2B, &irq_11, INTR_GATE, DPL_KERN);
  register_interrupt(0x2C, &irq_12, INTR_GATE, DPL_KERN);
  register_interrupt(0x2D, &irq_13, INTR_GATE, DPL_KERN);
  register_interrupt(0x2E, &irq_14, INTR_GATE, DPL_KERN);
  register_interrupt(0x2F, &irq_15, INTR_GATE, DPL_KERN);

  register_interrupt(0x80, &syscall_enter, INTR_GATE, DPL_USER);

  idt_pointer.limit = sizeof(table) - 1;
  idt_pointer.base  = (uint32_t) table; 

  __asm__ volatile
  (
    "lidt %0 " :: "m" (idt_pointer)
  );

  dprintf("initialized.\n");
}