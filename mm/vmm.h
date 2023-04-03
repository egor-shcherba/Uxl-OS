#ifndef _VMM_H
#define _VMM_H

void vmm_init(void);

void* ksbrk(uint32_t pages);

uint32_t kstack_alloc();
void kstack_free();

#endif /* NOT _VMM_H */