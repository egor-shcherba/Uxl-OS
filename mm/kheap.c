#include <debug/headers/mem.h>

#define __DEBUG_SUB_HEADER__        "KHEAP"
#define __DEBUG_SUB_HEADER_COLOR__  __FG_GREEN__
#define __DEBUG_CURRENT_SUG_LOG__   __LOG_SUB_MEM_KMALLOC__


#include <debug/qemu.h>
#include <mm/vmm.h>
#include <mm/kheap.h>
#include <mm/mmu.h>
#include <stddef.h>
#include <stdint.h>

#define P_AS_U32(x)         ((uint32_t*) (x))
#define MASK_SIZE           0xFFFFFFF8
#define MEM_ALLOCATED       0x1

#define HEADER_SIZE         8
#define HEADER_FULL_SIZE    HEADER_SIZE * 2

#define RESIZE_THRESHOLD    16

uint8_t *heap_start = NULL;
uint8_t *heap_end   = NULL;
uint8_t *first_chunk_free = NULL;

static inline size_t
chunk_size(uint8_t *p)
{
  return *P_AS_U32(p) & MASK_SIZE;
}

static inline int
is_chunk_alloc(uint8_t *p)
{
  return *P_AS_U32(p) & MEM_ALLOCATED;
}

static inline int
chunk_is_free(uint8_t *p)
{
  return !is_chunk_alloc(p);
}

static inline uint8_t*
next_chunk(uint8_t *p)
{
  return (p + chunk_size(p) + HEADER_FULL_SIZE);
}

static inline uint8_t*
chunk_set_size(uint8_t *p, size_t size)
{
  *P_AS_U32(p) = size;
  *P_AS_U32(p + size + HEADER_SIZE) = size;
  return p;
}

static inline void
chunk_set_free(uint8_t *p)
{
  *P_AS_U32(p) &= ~MEM_ALLOCATED;
  *P_AS_U32(p + chunk_size(p) + HEADER_SIZE) &= ~MEM_ALLOCATED;
}

static inline void
chunk_set_allocated(uint8_t *p)
{
  *P_AS_U32(p) |= MEM_ALLOCATED;
  *P_AS_U32(p + chunk_size(p) + HEADER_SIZE) |= MEM_ALLOCATED;
}

static inline size_t
align8(size_t size)
{
  uint32_t flags = size & 0x7;
  return flags ? size + 8 - flags : size;
}

static uint8_t*
find_first_fit(size_t size)
{
  for (uint8_t *p = first_chunk_free; p < heap_end; p = next_chunk(p))
    {
      if (chunk_size(p) >= size && chunk_is_free(p))
        return p;
    }

  return NULL;
}

static uint8_t*
alloc_new_memory(size_t nbytes)
{
  uint32_t num_pages = (nbytes / PAGE_SIZE) + 1;

  if (nbytes % PAGE_SIZE < HEADER_FULL_SIZE)
    num_pages++;

  size_t size = num_pages * PAGE_SIZE;
  uint8_t *prev_end = heap_end;
  heap_end = ksbrk(num_pages);

  return chunk_set_size(prev_end, size - HEADER_FULL_SIZE);
}

void
kheap_init(void)
{
  size_t size = PAGE_SIZE - HEADER_FULL_SIZE;
  heap_start = ksbrk(0);
  heap_end = ksbrk(1);
  first_chunk_free = heap_start;

  chunk_set_size(heap_start, size);
  chunk_set_free(heap_start);

  dprintf("initialized.\n");
}

void*
kmalloc(size_t nbytes)
{
  if (!heap_start)
    kheap_init();

  size_t size = align8(nbytes);

  uint8_t *p = find_first_fit(size);

  if (p == NULL)
    p = alloc_new_memory(size);

  size_t csize = chunk_size(p);
  size_t diff = csize - size;

  if (diff > RESIZE_THRESHOLD)
    {
      size_t next_size = diff - HEADER_FULL_SIZE;
      chunk_set_size(p, size);
      chunk_set_size(next_chunk(p), next_size);
    }

  chunk_set_allocated(p);

  dprintf("allocate chunk %p :: %u byte's\n", p + HEADER_SIZE, size);

  return p + HEADER_SIZE;
}

void
kfree(void *p)
{
  uint8_t *chunk = (uint8_t*) p - HEADER_SIZE;
  size_t size = chunk_size(chunk);
  size_t csize = size;
  uint8_t *next = next_chunk(chunk);
  uint8_t *prev = chunk - HEADER_SIZE;

  if (next < heap_end && chunk_is_free(next))
    size += chunk_size(next) + HEADER_FULL_SIZE;

  chunk_set_free(chunk);
  chunk_set_size(chunk, size);

  if (prev > heap_start)
    {
      prev -= chunk_size(prev) + HEADER_SIZE;
      size += chunk_size(prev) + HEADER_FULL_SIZE;
      chunk_set_size(prev, size);
      chunk = prev;
    }

  if (chunk < first_chunk_free)
    first_chunk_free = chunk;

  dprintf("free chunk %p :: %u byte's\n", p, csize);
}
