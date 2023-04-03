#ifndef _MMU_H
#define _MMU_H

#define KERNPHYS        0x00100000
#define KERNBASE        0xC0000000

#define PDX(x)          (x >> 22)
#define PTX(x)          ((x >> 12) & 0x3FF)

#define U32_P2V(x)      ((uint32_t) (KERNBASE + x))
#define U32_V2P(x)      ((uint32_t) x - KERNBASE)

#define P2V(x)          ((void*) U32_P2V(x))
#define V2P(x)          ((void*) U32_V2P(x))

#define PTE_ADDR(x)     ((uint32_t) x & ~0x3FF)
#define PTE_FLAGS(x)    ((uint32_t) x & 0x3FF)

#define PTE_P           (1 << 0)
#define PTE_W           (1 << 1)
#define PTE_U           (1 << 2)

#define PD_ENTRIES      1024
#define PT_ENTRIES      1024
#define PAGE_SIZE       4096

#define PGROUNDUP(x)    ((x) + PAGE_SIZE - 1)

#define PAGE_COUNT(x, y) \
  ((y - x - 1) / PAGE_SIZE + 1)

#endif /* NOT _MMU_H */