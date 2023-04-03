#ifndef _KERN_LAYOUT_H
#define _KERN_LAYOUT_H

extern uint32_t __KERN_CODE_START;
extern uint32_t __KERN_CODE_END;
extern uint32_t __KERN_DATA_START;
extern uint32_t __KERN_DATA_END;
extern uint32_t __KERN_BSS_START;
extern uint32_t __KERN_BSS_END;

#include <mm/mmu.h>

#define KERN_CODE_START ((uint32_t) &__KERN_CODE_START)
#define KERN_CODE_END   ((uint32_t) &__KERN_CODE_END)
#define KERN_DATA_START ((uint32_t) &__KERN_DATA_START)
#define KERN_DATA_END   ((uint32_t) &__KERN_DATA_END)
#define KERN_BSS_START  ((uint32_t) &__KERN_BSS_START)
#define KERN_BSS_END    ((uint32_t) &__KERN_BSS_END)

#define REGION_PAGES_SIZE   (256 * 1024 * 1024) /* 256 Mb*/
#define REGION_KSTAK_SIZE   (128 * 1024 * 1024) /* 128 Mb */

#define TASK_PAGES_START    (PTE_ADDR(KERN_BSS_END) + PAGE_SIZE)
#define TASK_KSTACK_START   (TASK_PAGES_START + REGION_PAGES_SIZE + PAGE_SIZE)
#define KERN_HEAP_START     (TASK_KSTACK_START + REGION_KSTAK_SIZE + PAGE_SIZE)

#define KERN_CODE_SIZE  \
  ((KERN_CODE_END - KERN_CODE_START - 1) / PAGE_SIZE + 1)

#define KERN_DATA_SIZE  \
  ((KERN_DATA_END - KERN_DATA_START - 1) / PAGE_SIZE + 1)

#define KERN_BSS_SIZE  \
  ((KERN_BSS_END - KERN_BSS_START - 1) / PAGE_SIZE + 1)

#define KERN_TOTAL_SIZE \
  (KERN_BSS_END - (KERNBASE + KERN_CODE_START))

#endif /* NOT _KERN_LAYOUT_H */
