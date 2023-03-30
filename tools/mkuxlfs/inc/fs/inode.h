#ifndef _INODE_H
#define _INODE_H

#include <stdint.h>

#define NADDR 10

#define REGFILE 0x4000
#define DIRFILE 0x8000

struct inode {
  uint16_t mode;
  uint16_t links;
  uint16_t uid;
  uint16_t gid;

  uint32_t filelength;

  uint32_t ct_time;
  uint32_t md_time;
  uint32_t ac_time;

  uint32_t addr[NADDR];
};

#endif /* NOT _INODE_H */