#include <dirent.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>

#include <fs/uxlfs.h>
#include <fs/disk.h>
#include <fs/log2.h>
#include <fs/bitmap.h>

static struct superblock *sp  = NULL;
static struct inode *inodes   = NULL;
static uint8_t *bitmap = NULL;

static uint32_t block_size = 0;
static uint32_t blk_for_bitmap = 0; 

#define ROOT_INODE 1

#define BPB (block_size * 8)

#define BINS  (block_size / 4)  /* blocks in single ndirect block */

#define PANIC(msg)              \
{                               \
  printf(msg);                  \
  sync_fs();                    \
  exit(EXIT_FAILURE);           \
}

void
spinfo(void)
{
  printf("total blk %u\n", sp->total_blk);
  printf("total inodes %u\n", sp->total_inodes);
  printf("total data %u\n", sp->total_data);
  printf("bitmap start %u\n", sp->bitmap_blk_start);
  printf("data block start %u\n", sp->data_blk_start);

  printf("free inodes %u\n", sp->free_inodes);
  printf("free data %u\n", sp->free_data);

  printf("logn %u\n", sp->logn);
  printf("block size %u\n", 1 << sp->logn);
}

static uint32_t
ialloc(void)
{
  for (uint32_t i = 2; i < sp->total_inodes; i++)
    {
      if (inodes[i].links == 0)
        {
          sp->free_inodes--;
          inodes[i].links++;
          return i;
        }
    }

  PANIC("critical :: inode cannot be allocated\n");
}

static uint32_t
balloc(void)
{
  int32_t num_block = bitmap_get_free(bitmap, sp->total_data);

  if (num_block == -1)
    PANIC("critical :: block cannot be allocated\n");

  sp->free_data--;
  bitmap_set(bitmap, num_block);
  return sp->data_blk_start + num_block;
}

static uint32_t
iget_phys_block(struct inode *inode, uint32_t i)
{
  struct block *block = NULL;
  uint32_t *ptr = NULL;
  uint32_t phys_block = 0;
 
  if (i < 8)
    {
      if (inode->addr[i] == 0)
        inode->addr[i] = balloc();

      return inode->addr[i];
     }

  i -= 8;

  /* single ndirect block */
  if (i < BINS)
    {
      if (inode->addr[8] == 0)
        inode->addr[8] = balloc();

      block = disk_get_block(inode->addr[8]);
      ptr = (uint32_t*) block->data;

      if (ptr[i] == 0)
        ptr[i] = balloc();

      phys_block = ptr[i];
      disk_put_block(block);

      return phys_block;
     }

  /* double ndirect block */
  i -= BINS;

  if (i > BINS * BINS)
    PANIC("out memory\n");

  if (inode->addr[9] == 0)
    inode->addr[9] = balloc();

  block = disk_get_block(inode->addr[9]);
  ptr = (uint32_t*) block->data;

  if (ptr[i / BINS] == 0)
    ptr[i / BINS] = balloc();

  phys_block = ptr[i / BINS];
  disk_put_block(block);

  block = disk_get_block(phys_block);
  ptr = (uint32_t*) block->data;

  if (ptr[i % BINS] == 0)
    ptr[i % BINS] = balloc();

  phys_block = ptr[i % BINS];
  disk_put_block(block);

  return phys_block;
}

static int
iwrite(void *src, uint32_t seek, uint32_t nsize, struct inode *inode)
{
  uint32_t bytes_to_write = 0;
  uint32_t offset = 0;
  uint32_t phys_block = 0;
  uint32_t total_write = nsize;

  for (uint32_t i = (seek / block_size); nsize != 0; i++)
    {
      offset = seek % block_size;
      bytes_to_write = (block_size < nsize) ? block_size - seek % block_size : nsize;

      phys_block = iget_phys_block(inode, i);
      disk_write(src, phys_block, offset, bytes_to_write);

      seek += bytes_to_write;
      nsize -= bytes_to_write;
    }

  if (seek > inode->filelength)
    inode->filelength = seek;

  return total_write;
}

static int
iread(void *dst, uint32_t seek, uint32_t nsize, struct inode *inode)
{
  uint32_t bytes_to_read = 0;
  uint32_t offset = 0;
  uint32_t phys_block = 0;
  uint32_t total_read = 0;

  for (uint32_t i = (seek / block_size); nsize != 0; i++)
    {
      offset = seek % block_size;
      bytes_to_read = (block_size < nsize) ? block_size - seek % block_size : nsize;

      if (seek + bytes_to_read > inode->filelength)
        bytes_to_read = inode->filelength - seek;

      if (bytes_to_read <= 0)
        break;

      phys_block = iget_phys_block(inode, i);
      disk_read(dst, phys_block, offset, bytes_to_read);

      seek += bytes_to_read;
      nsize -= bytes_to_read;
      total_read += bytes_to_read;
    }

  return total_read;
}

static inline uint32_t
inode_get_number(struct inode *inode)
{
  return ((uint64_t) inode - (uint64_t)&inodes[0]) / sizeof(struct inode);
}

static struct inode*
ilookup(struct inode *dir, const char *filename)
{
  struct uxl_dirent entity;

  for (uint32_t i = 0; i < dir->filelength; i += sizeof(struct uxl_dirent))
    {
      iread(&entity, i, sizeof(struct uxl_dirent), dir);

      if (strcmp(entity.filename, filename) == 0)
        return &inodes[entity.inum];
    }

  return NULL;
}


static inline void
dir_write(struct inode *dir, uint32_t inum, const char *filename)
{
  struct uxl_dirent entity;
  entity.inum = inum;
  memset(entity.filename, 0, FILENAME_LENGTH);
  strcpy(entity.filename, filename);

  iwrite(&entity, dir->filelength, sizeof(struct uxl_dirent), dir);
}

static void
dir_create_entity(struct inode *dir, const char *name, uint32_t type)
{
  uint32_t num = ialloc();
  struct inode *inode = &inodes[num];
  uint32_t dir_num = inode_get_number(dir);

  time_t current_time;
  time(&current_time);


  if (ilookup(dir, name) != NULL)
    {
      printf("'%s' is already exists\n", name);
      exit(EXIT_FAILURE);
    }

  if (type == DIRFILE)
    {
      dir_write(inode, num, ".");
      dir_write(inode, dir_num, "..");
    }

  inode->mode = type;
  inode->ct_time = current_time;
  inode->md_time = current_time;
  inode->ac_time = current_time;

  dir_write(dir, num, name);
}

static struct inode*
iname(const char *fullpath)
{
  char path[BUFSIZ];
  strcpy(path, fullpath);

  struct inode *parent_dir = &inodes[ROOT_INODE];

  char *token = strtok(path, "/");

  while (token != NULL)
    {
      parent_dir = ilookup(parent_dir, token);

      if (parent_dir == NULL)
        {
          printf("cannot access '%s' no such file or dirrectory\n", token);
          exit(EXIT_FAILURE);
        }

      token = strtok(NULL, "/");
    }

  return parent_dir;
}

static void
get_path(char *buf, const char *path)
{
  char *p = strrchr(path, '/');

  if (p == NULL || p == path)
    {
      *buf++ = '/';
      *buf = '\0';
      return;
    }

  while (p != path)
    *buf++ = *path++;

  *buf = '\0';
}

void
create_fs(const char *filename, uint32_t bs, uint32_t count)
{
  uint32_t blk_for_inodes = (count * 0.1);
  blk_for_bitmap = (count - blk_for_inodes - 1) / (bs * 8) + 1;
  time_t current_time;
  uint32_t root_blk = 0;
  time(&current_time);

  sp = (struct superblock*) malloc(sizeof(*sp));
  sp->signature = UXLFS_SIGNATURE;
  sp->logn = LOG2(bs);
  sp->total_blk = count;
  sp->total_inodes = (blk_for_inodes << sp->logn) / sizeof(struct inode);
  sp->total_data = count - blk_for_inodes - blk_for_bitmap - 1;
  sp->bitmap_blk_start = 1 + blk_for_inodes;
  sp->data_blk_start = sp->bitmap_blk_start + blk_for_bitmap;
  sp->free_inodes = sp->total_inodes - 2;
  sp->free_data = sp->total_data;

  inodes = (struct inode*) calloc(sizeof(*inodes), sp->total_inodes);
  inodes[ROOT_INODE].mode = DIRFILE;
  inodes[ROOT_INODE].ac_time = current_time;
  inodes[ROOT_INODE].md_time = current_time;
  inodes[ROOT_INODE].ct_time = current_time;
  inodes[ROOT_INODE].filelength = sizeof(struct uxl_dirent) * 2;

  bitmap = calloc(1, blk_for_bitmap << sp->logn);
  root_blk = balloc();

  inodes[ROOT_INODE].addr[0] = root_blk;

  disk_set_bsize(bs);
  disk_create(filename, count);
  disk_open(filename);

  struct block *block = disk_get_block(root_blk);
  struct uxl_dirent *uxl_dirent = (struct uxl_dirent*) block->data;

  uxl_dirent[0].inum = ROOT_INODE;
  strcpy(uxl_dirent[0].filename, ".");
  uxl_dirent[1].inum = ROOT_INODE;
  strcpy(uxl_dirent[1].filename, "..");

  disk_put_block(block);

}

void
mount_fs(const char *filename)
{
  uint32_t bytes_per_bitmap = 0;
  disk_open(filename);

  sp = malloc(sizeof(*sp));
  disk_read(sp, 0, 0, sizeof(*sp));

  if (sp->signature != UXLFS_SIGNATURE)
    {
      printf("this file '%s' is not uxl filesystem\n", filename);
      exit(EXIT_FAILURE);
    }

  block_size = 1 << sp->logn;
  disk_set_bsize(block_size);

  inodes = malloc(sizeof(struct inode) * sp->total_inodes);
  disk_read(inodes, 1, 0, sizeof(struct inode) * sp->total_inodes);

  block_size = 1 << sp->logn;
  blk_for_bitmap = (sp->total_data - 1) / (block_size * 8) + 1;
  bytes_per_bitmap = blk_for_bitmap << sp->logn;

  bitmap = malloc(bytes_per_bitmap);
  disk_read(bitmap, sp->bitmap_blk_start, 0, bytes_per_bitmap);
}

void
sync_fs(void)
{
  disk_write(sp, 0, 0, sizeof(*sp));
  disk_write(inodes, 1, 0, sizeof(*inodes) * sp->total_inodes);
  disk_write(bitmap, sp->bitmap_blk_start, 0, (sp->total_data - 1) / 8 + 1);
  disk_close();

  free(sp);
  free(inodes);
  free(bitmap);
}

void
ls(const char *path)
{
  struct inode *root = iname(path);
  struct uxl_dirent entity;
  time_t time;

  if (root->filelength == 0)
    return;

  for (uint32_t i = 0; i < root->filelength; i += sizeof(struct uxl_dirent))
    {
      iread(&entity, i, sizeof(struct uxl_dirent), root);
      struct inode *inode = &inodes[entity.inum];
      time = inode->md_time;
      char c = inode->mode == DIRFILE ? 'd' : '-';
      uint32_t size = inode->filelength;

      if (c == 'd')
        size = ((size - 1) / block_size + 1) * block_size;

      printf("%c %-4u %8u %.16s %s\n",
        c, entity.inum, size, ctime(&time), entity.filename);
    }
}


void
cli_touch(const char *filepath)
{
  char path_tmp[BUFSIZ];
  char path[BUFSIZ];
  char *name = NULL;

  strcpy(path_tmp, filepath);
  get_path(path, filepath);
  name = basename(path_tmp);

  struct inode *dir = iname(path);

  dir_create_entity(dir, name, REGFILE);
}

void
cli_mkdir(const char *filepath)
{
  char path_tmp[BUFSIZ];
  char path[BUFSIZ];
  char *name = NULL;

  strcpy(path_tmp, filepath);
  get_path(path, filepath);
  name = basename(path_tmp);

  struct inode *dir = iname(path);

  dir_create_entity(dir, name, DIRFILE);
}

void
cpin(const char *src, const char *dst)
{
  FILE *copyfile = fopen(src, "r+b");
  int rc = 0;
  uint8_t *data = malloc(block_size);
  struct inode *inode = NULL;

  if (copyfile == NULL)
    {
      perror(src);
      exit(EXIT_FAILURE);
    }

  cli_touch(dst);
  inode = iname(dst);

  if (inode == NULL)
    exit(EXIT_FAILURE);

  do
    {
      memset(data, 0, block_size);
      rc = fread(data, 1, block_size, copyfile);
      iwrite(data, inode->filelength, rc, inode);
    }
  while (rc != 0);

  free(data);
  fclose(copyfile);
}

void
cpout(const char *src, const char *dst)
{
  struct inode *inode = iname(src);
  int rc = -1;
  uint8_t *data = malloc(block_size);
  FILE *copyfile = fopen(dst, "wb");
  uint32_t off = 0;

  if (copyfile == NULL)
    {
      perror(dst);
      exit(EXIT_FAILURE);
    }

  while (rc != 0)
    {
      memset(data, 0, block_size);
      rc = iread(data, off, block_size, inode);
      fwrite(data, 1, rc, copyfile);
      off += rc;
    }

  free(data);
  fclose(copyfile);
}

void
cpdir_in(const char *src, const char *dst)
{
  DIR *dir = opendir(src);
  char *filein = calloc(1, 1024);
  char *fileout = calloc(1, 1024);

  if (dir == NULL)
    {
      printf("cannout open dir '%s'\n", src);
      exit(EXIT_FAILURE);
    }

  if (strcmp(dst, "/") != 0)
    cli_mkdir(dst);

  struct dirent *entity = readdir(dir);

  while (entity != NULL)
    {
      memset(filein, 0, 1024);
      memset(fileout, 0, 1024);

      strcpy(filein, src);
      strcpy(fileout, dst);

      strcat(filein, "/");
      strcat(filein, entity->d_name);

      strcat(fileout, "/");
      strcat(fileout, entity->d_name);

      if (entity->d_type == DT_DIR)
        {
          if ((strcmp(entity->d_name, ".") != 0) && (strcmp(entity->d_name, "..") != 0))
            {
              printf("cloning dir '%s' to '%s' \n", filein, fileout);
              cpdir_in(filein, fileout);
            }
        }

      if (entity->d_type == DT_REG)
        {
          printf("copy '%s' to '%s' ...", src, dst);
          cpin(filein, fileout);
          printf("done.\n");
        }

      entity = readdir(dir);
    }

  closedir(dir);
}

void
cpdir_out(const char *src, const char *dst)
{
  struct inode *dir = iname(src);
  struct uxl_dirent entity;
  struct inode *ifile = NULL;

  char *filein = malloc(1024);
  char *fileout = malloc(1024);

  if (dir->mode != DIRFILE)
    {
      printf("'%s' is not dirrectory\n", src);
      exit(EXIT_FAILURE);
    }

  mkdir(dst, S_IRWXU);

  for (uint32_t i = 0; i < dir->filelength; i += sizeof(struct uxl_dirent))
    {
      memset(filein, 0, 1024);
      memset(fileout, 0, 1024);

      strcpy(filein, src);
      strcpy(fileout, dst);

      strcat(filein, "/");
      strcat(fileout, "/");

      iread(&entity, i, sizeof(struct uxl_dirent), dir);

      strcat(filein, entity.filename);
      strcat(fileout, entity.filename);

      ifile = iname(filein);

      if (ifile->mode == REGFILE)
        {
          printf("extract from image '%s' to '%s'", filein, fileout);
          cpout(filein, fileout);
          printf("done.\n");
        }

      if (ifile->mode == DIRFILE)
        {
          if (strcmp(entity.filename, ".") != 0 && strcmp(entity.filename, ".."))
            {
              printf("extract dirrectory from image '%s' to '%s'\n", filein, fileout);
              cpdir_out(filein, fileout);
            }
        }
    }

  free(filein);
  free(fileout);
}