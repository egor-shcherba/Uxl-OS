#include <debug/headers/mem.h>

#define __DEBUG_SUB_HEADER__        "VMM"
#define __DEBUG_SUB_HEADER_COLOR__   __FG_MAGENTA__
#define __DEBUG_CURRENT_SUB_LOG__    __LOG_SUB_MEM_VMM__

#include <debug/qemu.h>
#include <mm/vmm.h>
#include <mm/kernlayout.h>
#include <mm/pmm.h>
#include <sys/x86.h>
#include <stdint.h>
#include <string.h>

typedef uint32_t pde_t;
typedef uint32_t pte_t;

pde_t kern_dir[1024] __attribute__((aligned(4096)));

/* kernel tables for high virtual address */
pte_t kern_tbls[1024U * 256U] __attribute__((aligned(4096)));

static uint32_t cur_heap_ptr = 0;

static inline void
kern_map_dir_tbls(void)
{
  for (int i = 0; i < 256; i++)
    {
      uint32_t vaddr = KERNBASE + i * 0x400000;
      kern_dir[PDX(vaddr)] = U32_V2P(kern_tbls + (PDX(vaddr) - 768) * PAGE_SIZE);
      kern_dir[PDX(vaddr)] |= PTE_P | PTE_W | PTE_U;
    }
}

static void
init_kern_map(uint32_t pstart, uint32_t vstart, uint32_t vend, uint32_t flags)
{
  uint32_t pg_count = PAGE_COUNT(vstart, vend);
  alloc_pages(pg_count);
  pte_t *table = NULL;

  for (uint32_t i = 0; i < pg_count; i++)
    {
      table = (pte_t*) PTE_ADDR(P2V(kern_dir[PDX(vstart)]));
      table[PTX(vstart)] = pstart | flags;
      vstart += PAGE_SIZE;
      pstart += PAGE_SIZE;
    }
}

static inline pte_t*
get_kernel_table(uint32_t vaddr)
{
  return (pte_t*) U32_P2V(PTE_ADDR(kern_dir[PDX(vaddr)]));
}

static inline uint32_t
__region_free_page(uint32_t addr)
{
  pte_t *table = get_kernel_table(addr);
  uint32_t phys_addr = PTE_ADDR(table[PTX(addr)]);
  free_page((void*) phys_addr);
  table[PTX(addr)] = 0;

  return phys_addr;
}

uint32_t
vm_page_region_alloc(void)
{
  uint32_t phys_addr = alloc_page();
  pte_t *table;

  if (phys_addr == 0)
    return 0;

  for (uint32_t vaddr = TASK_PAGES_START; vaddr < TASK_KSTACK_START; vaddr += PAGE_SIZE)
    {
      table = get_kernel_table(vaddr);

      if (table[PTX(vaddr)] == 0)
        {
          table[PTX(vaddr)] = phys_addr | PTE_P | PTE_W | PTE_U;
          memset((void*) vaddr, 0, PAGE_SIZE);
          dprintf("page region allocate page vaddr %p phys %p\n", vaddr, phys_addr);
          return vaddr;
        }
    }

  free_page((void*) phys_addr);
  return 0;
}

void
vm_page_region_free(uint32_t addr)
{
  uint32_t phys_addr = __region_free_page(addr);
  dprintf("page region free vaddr %p phys %p\n", addr, phys_addr);
}

uint32_t
kstack_alloc(void)
{
  uint32_t vstart = TASK_KSTACK_START + PAGE_SIZE;
  uint32_t phys_addr = alloc_page();
  pte_t *table = NULL;

  if (phys_addr == 0)
    return 0;

  for (; vstart < KERN_HEAP_START; vstart += 2 * PAGE_SIZE)
    {
      table = get_kernel_table(vstart);

      if (table[PTX(vstart)] == 0)
        {
          table[PTX(vstart)] = (phys_addr | PTE_P |PTE_W);
          memset((void*) vstart, 0, PAGE_SIZE);
          dprintf("kstack region allocate vaddr %p phys addr %p\n",
            PGROUNDUP(vstart), phys_addr);
          return PGROUNDUP(vstart);
        }
    }

  free_page((void*) phys_addr);

  return 0;
}

void
kstack_free(uint32_t addr)
{
  uint32_t phys_addr = __region_free_page(addr);
  dprintf("kstack region free vaddr %p phys %p\n", addr, phys_addr);
}

void*
ksbrk(uint32_t pages)
{
  if (pages == 0)
    return (void*) cur_heap_ptr;

  for (uint32_t i = 0; i < pages; i++)
    {
      uint32_t phys_addr = alloc_page();
      pte_t *table = get_kernel_table(cur_heap_ptr);
      table[PTX(cur_heap_ptr)] = phys_addr | PTE_P | PTE_W;
      dprintf("ksrb alloc page virt %p phys %p\n", cur_heap_ptr, PTE_ADDR(phys_addr));
      cur_heap_ptr += PAGE_SIZE;
    }

  return (void*) cur_heap_ptr;
}

uint32_t
kv2p(void *addr)
{
  pte_t *table = get_kernel_table((uint32_t) addr);
  return PTE_ADDR(table[PTX((uint32_t) addr)]);
}

void
vmm_init(void)
{
  cur_heap_ptr = KERN_HEAP_START;
  pde_t *init_dir = (pte_t*) cpu_read_cr3();

  kern_map_dir_tbls();

  /* map kernel code */
  init_kern_map(
    KERNPHYS, U32_P2V(KERN_CODE_START), U32_P2V(KERN_CODE_END), PTE_U | PTE_P
  );

  /* map kernel data */
  init_kern_map(
    U32_V2P(KERN_DATA_START), KERN_DATA_START, KERN_DATA_END, PTE_U | PTE_W | PTE_P
  );

  /* map kernel bss*/
  init_kern_map(
    U32_V2P(KERN_BSS_START), KERN_BSS_START, KERN_BSS_END, PTE_U | PTE_W | PTE_P
  );

  kern_dir[0] = init_dir[0];
  cpu_load_cr3(U32_V2P(kern_dir));

  /* init first user stack */
  uint32_t ustack_addr = 0x0C000000 - PAGE_SIZE;
  pte_t *table = (pte_t*) vm_page_region_alloc();
  kern_dir[PDX(ustack_addr)] = kv2p((void*) table) | PTE_W | PTE_U |PTE_P;
  pte_t *page = (pte_t*) vm_page_region_alloc();
  table[PTX(ustack_addr)] = kv2p((void*) page) | PTE_W | PTE_U | PTE_P;

  dprintf("region pages  size %p - %p\n", TASK_PAGES_START, TASK_KSTACK_START);
  dprintf("region kstack size %p - %p\n", TASK_KSTACK_START, KERN_HEAP_START);
  dprintf("region heap   size %p - %p\n", KERN_HEAP_START, 0xFFFFF000);

  dprintf("Initialized\n");
}
