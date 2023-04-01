#include <debug/headers/cpu.h>

#define __DEBUG_SUB_HEADER__        "GDT"
#define __DEBUG_SUB_HEADER_COLOR__  __FG_GREEN__
#define __DEBUG_CURRENT_SUB_LOG__   __LOG_SUB_CPU_GDT__

#include <debug/qemu.h>
#include <sys/gdt.h>
#include <sys/tss.h>
#include <stdint.h>
#include <string.h>

struct gdt_entry {
  uint32_t limit_low  : 16;
  uint32_t base_low   : 16;
  uint32_t base_mid   : 8;
  uint32_t type       : 4;
  uint32_t s          : 1;
  uint32_t dpl        : 2;
  uint32_t p          : 1;
  uint32_t limit_high : 4;
  uint32_t avl        : 1;
  uint32_t l          : 1;
  uint32_t db         : 1;
  uint32_t g          : 1;
  uint32_t base_high  : 8;
};

struct tss_entry {
  uint32_t limit_low  : 16;
  uint32_t base_low   : 24;
  uint32_t type       : 5;
  uint32_t dpl        : 2;
  uint32_t p          : 1;
  uint32_t limit_high : 4;
  uint32_t avl        : 3;
  uint32_t g          : 1;
  uint32_t base_high  : 8;
} __attribute__ ((packed));

struct gdt_pointer {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

#define GDT_TABLE_SIZE  6
static struct gdt_entry
gdt_table[GDT_TABLE_SIZE] __attribute__ ((aligned(4096))) = { 0x0 };

static struct gdt_pointer gdt_pointer = { 0x0 };

#define DPL_KERNEL  0x0
#define DPL_USER    0x30

#define CODE_TYPE   0xA
#define DATA_TYPE   0x2

struct tss sys_tss = { 0x0 };

void
tss_set_stack(void *kstack)
{
  sys_tss.esp0 = (uint32_t) kstack;
}

void*
tss_get_stack(void)
{
  return (void*) sys_tss.esp0;
}

static inline void
log_set_gate(uint32_t num, uint32_t base, uint32_t limit, uint8_t dpl, uint8_t type)
{
  const char *type_str = "DATA";

  if ((type & 0xF) == CODE_TYPE)
    type_str = "CODE";

  dprintf("set selector %#.2x BASE %p LIMIT %p DPL %.2x TYPE '%s'\n",
    num, base, limit, dpl, type_str);
}

static void
gdt_set_entry(struct gdt_entry *entry, uint32_t base, uint32_t limit, uint8_t type)
{
  entry->limit_low  = (limit & 0xFFFF);
  entry->limit_high = (limit >> 16) & 0xFF;

  entry->base_low  = (base & 0xFFFF);
  entry->base_mid  = (base >> 16) & 0xFF;
  entry->base_high = (base >> 24);

  entry->dpl = (type >> 4) & 0x3;
  entry->type = (type & 0xF);

  entry->p = 1;
  entry->s = 1;
  entry->avl = 0;
  entry->l = 0;
  entry->db = 1;
  entry->g = 1;

  log_set_gate((uint32_t) (entry - gdt_table), base, limit, entry->dpl, type);
}

static void
gdt_init_tss(void)
{
  struct tss_entry *entry = (struct tss_entry*) &gdt_table[5];

  uint32_t base = (uint32_t) &sys_tss;
  uint32_t limit = sizeof(struct tss) ;

  entry->limit_low  = limit & 0xFFFF;
  entry->limit_high = limit & 0xF;

  entry->base_low   = base & 0xFFFFFF;
  entry->base_high  = base >> 24;

  entry->type = 0x9;  /* 0b1001 */
  entry->dpl  = 0x0;
  entry->p    = 1;

  entry->avl  = 0;
  entry->g    = 1;

  memset(&sys_tss, 0, sizeof(struct tss));

  sys_tss.esp0 = 0xA00000;
  sys_tss.ss0  = 0x10;

  dprintf("set tss selector %#.2x\n", 0x5);
}

void
gdt_init(void)
{
  /* NULL SELECTOR */
  gdt_set_entry(&gdt_table[0], 0x0, 0x0, 0x0);

  /* kERNEL CODE SELECTOR */
  gdt_set_entry(&gdt_table[1], 0x0, 0xFFFFFFFF, CODE_TYPE | DPL_KERNEL);

  /* KERNEL DATA SELECTOR */
  gdt_set_entry(&gdt_table[2], 0x0, 0xFFFFFFFF, DATA_TYPE | DPL_KERNEL);

  /* USER CODE SELECTOR */
  gdt_set_entry(&gdt_table[3], 0x0, 0xFFFFFFFF, CODE_TYPE | DPL_USER);

  /* USER DATA SELECTOR */
  gdt_set_entry(&gdt_table[4], 0x0, 0xFFFFFFFF, DATA_TYPE | DPL_USER);

  gdt_init_tss();

  gdt_pointer.limit = sizeof(gdt_table) - 1;
  gdt_pointer.base = (uint32_t) gdt_table;

  __asm__ volatile
  (
    "lgdt %0;"
    "mov $0x10, %%eax;"
    "mov %%eax, %%gs;"
    "mov %%eax, %%fs;"
    "mov %%eax, %%ds;"
    "mov %%eax, %%es;"
    "mov %%eax, %%ss;"
    "ljmp $0x8, $1f;"
    "1:"
    :: "m" (gdt_pointer)
  );

  dprintf("load gdt table\n");

  __asm__ volatile
  (
    "mov $0x28, %ax;"
    "ltr %ax"
  );

  dprintf("load tss selector\n");

  dprintf("initialized.\n");
}
