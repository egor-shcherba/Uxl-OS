#ifndef _PMM_H
#define _PMM_H

#include <stddef.h>
#include <stdint.h>

struct memstat {
  size_t total;
  size_t used;
};

void pmm_init();

uint32_t alloc_pages(size_t count);
void free_pages(void *pages, size_t count);

void pmm_map(uint32_t phys_start, size_t count);

void pmm_get_memstat(struct memstat *memstat);

static inline uint32_t __attribute__((always_inline))
alloc_page(void)
{
  return alloc_pages(1);
}

static inline void __attribute__((always_inline)) 
free_page(void *page)
{
  free_pages(page, 1);
}

#endif /* NOT _PMM_H */
