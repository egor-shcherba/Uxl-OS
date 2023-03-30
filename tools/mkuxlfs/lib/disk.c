#include <stdio.h>
#include <stdlib.h>

#include <fs/disk.h>
#include <fs/log2.h>

static FILE *image = NULL;
static uint32_t bsize = 9;

void
disk_create(const char *filename, uint32_t block_count)
{
  uint8_t *zero_blk = calloc(1, 1 << bsize);

  image = fopen(filename, "wb");

  if (image == 0)
    {
      perror(filename);
      exit(EXIT_FAILURE);
    }

  fseek(image, (block_count - 1) << bsize, SEEK_SET);
  fwrite(zero_blk, 1, 1 << bsize, image);

  free(zero_blk);
  fclose(image);
}

void
disk_set_bsize(uint32_t size)
{
  bsize = LOG2(size);
}

void
disk_open(const char *filename)
{
  image = fopen(filename, "r+b");

  if (image == NULL)
    {
      perror(filename);
      exit(EXIT_FAILURE);
    }
}

void
disk_close(void)
{
  fclose(image);
}

int
disk_read(void *dst, uint32_t num_block, uint32_t offset, uint32_t size)
{
  fseek(image, (num_block << bsize) + offset, SEEK_SET);
  int rc = fread(dst, 1, size, image);

  return rc;
}

int
disk_write(void *src, uint32_t num_block, uint32_t offset, uint32_t size)
{
  fseek(image, (num_block << bsize) + offset, SEEK_SET);
  int rc = fwrite(src, 1, size, image);

  return rc;
}

struct block*
disk_get_block(uint32_t num)
{
  struct block *block = malloc(sizeof(struct block));
  block->num = num;
  block->data = malloc(1 << bsize);

  fseek(image, num << bsize, SEEK_SET);
  fread(block->data, 1, 1 << bsize, image);

  return block;
}

void
disk_put_block(struct block *block)
{
  fseek(image, block->num << bsize, SEEK_SET);
  fwrite(block->data, 1 << bsize, 1, image);

  free(block->data);
  free(block);
}