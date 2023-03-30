#ifndef _UXLFS_H
#define _UXLFS_H

#include <fs/superblock.h>
#include <fs/inode.h>
#include <fs/dirent.h>

#include <stdint.h>

void mount_fs(const char *filename);
void sync_fs(void);

void create_fs(const char *filename, uint32_t bs, uint32_t count);

void cli_touch(const char *filename);
void cli_mkdir(const char *dirname);

void cpout(const char *src, const char *dst);
void cpin(const char *src, const char *dst);

void cpdir_in(const char *src, const char *dst);
void cpdir_out(const char *src, const char *out);

void ls(const char *path);

void spinfo(void);

#endif /* NOT _UXLFS_H */