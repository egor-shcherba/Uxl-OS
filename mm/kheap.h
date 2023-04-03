#ifndef _KHEAP_H
#define _KHEAP_H

#include <stddef.h>

void *kmalloc(size_t nbytes);
void kfree(void *p);

#endif /* NOT _KHEAP_H */