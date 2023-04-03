#include <debug/headers/mem.h>

#define __DEBUG_SUB_HEADER__        "PMM"
#define __DEBUG_CURRENT_SUG_LOG__   __LOG_SUB_MEM_PMM__

#include <debug/qemu.h>
#include <mm/pmm.h>
#include <mm/mmu.h>
#include <panic.h>

#define TOTAL_PAGES 1048575

#define BPE         32      /* bit per entries */

#define BITMAP_SIZE 32768

static uint32_t bitmap[BITMAP_SIZE] = { 0x0 };
static uint32_t _free_pages = TOTAL_PAGES;
static uint32_t start = 0;

static inline uint32_t __attribute__((always_inline))
test_bit(uint32_t bit)
{
  return ((bitmap[bit / BPE]) & (1 << (bit % BPE))) != 0;
}

static inline void __attribute__((always_inline))
set_bit(uint32_t bit)
{
  bitmap[bit / BPE] |= (1 << (bit % BPE));
}

static inline void __attribute__((always_inline))
unset_bit(uint32_t bit)
{
  bitmap[bit / BPE] &= ~(1 << (bit % BPE));
}

static inline void
bitmap_sets(uint32_t start, uint32_t count)
{
  for (uint32_t i = start; i < start + count; i++)
    set_bit(i);
}

static inline void
bitmap_unsets(uint32_t start, uint32_t count)
{
  for (uint32_t i = start; i < start + count; i++)
    unset_bit(i);
}

uint32_t
alloc_pages(uint32_t pg_count)
{
  uint32_t pg_start = 0;
  uint32_t count = pg_count;
  uint32_t i = start;

  for (; i < TOTAL_PAGES; i++)
    {
      pg_start = i;

      if (!test_bit(pg_start++))
        {
          if (!--count)
            goto found;
        }
      else
        {
          i = pg_start;
          count = pg_count;
        }
    }

  PANIC("Out of memory\n");

found:
  start = pg_start;
  pg_start -= pg_count;
  bitmap_sets(pg_start, pg_count);
  _free_pages -= pg_count;

  dprintf("alloc pages %p count %u\n", pg_start * PAGE_SIZE, pg_count);
  return (pg_start * PAGE_SIZE);
}

void
free_pages(void *pages, uint32_t count)
{
  uint32_t n = (uint32_t) pages / PAGE_SIZE;
  bitmap_unsets(n, count);

  if (start > n)
    start = n;

  dprintf("free pages %p count %u\n", pages, count);
}

void
pmm_get_memstat(struct memstat *memstat)
{
  memstat->total = TOTAL_PAGES;
  memstat->used  = TOTAL_PAGES - _free_pages;
}

void
pmm_init(void)
{
  /* map first 1MB physical memory */
  bitmap_sets(0, 256);
  _free_pages -= 256;
  dprintf("initialized.\n");
}
