#ifndef _DISK_H
#define _DISK_H

#include <stdint.h>

struct block {
  uint32_t num;
  uint8_t *data;
};

void disk_set_bsize(uint32_t size);
void disk_create(const char *filename, uint32_t block_count);

void disk_open(const char *filename);
void disk_close(void);

int disk_read(void *dst, uint32_t num_block, uint32_t offset, uint32_t size);
int disk_write(void *src, uint32_t num_block, uint32_t offset, uint32_t size);

struct block* disk_get_block(uint32_t num);
void disk_put_block(struct block *block);

#endif /* NOT DISK_H */