#ifndef _UXL_DIRENT_H
#define _UXL_DIRENT_H

#define FILENAME_LENGTH 60

struct uxl_dirent {
  uint32_t inum;
  char filename[FILENAME_LENGTH];
};

#endif /* NOT _UXL_DIRENT_T */