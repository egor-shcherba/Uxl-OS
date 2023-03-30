#ifndef _SUPERBLOCK_H
#define _SUPERBLOCK_H

#include <stdint.h>

#define UXLFS_SIGNATURE 0x002A53464C58552A  /* "*UXLFS*" in ASCII*/

struct superblock {
  uint64_t signature;

  uint32_t total_blk;
  uint32_t total_inodes;
  uint32_t total_data;

  uint32_t logn;

  uint32_t free_inodes;
  uint32_t free_data;

  uint32_t bitmap_blk_start;
  uint32_t data_blk_start;
};

#endif /* NOT _SUPERBLOCK_H */