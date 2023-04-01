#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include <string.h>

struct mbr_part {
  uint32_t bootable   : 8;
  uint32_t chs_start  : 24;
  uint32_t part_type  : 8;
  uint32_t chs_last   : 24;
  uint32_t lba_first  : 32;
  uint32_t length     : 32;
};

#define SECTOR_SIZE 512
#define BUFFER_SIZE 2048

char *buffer[BUFFER_SIZE] = { 0x0 };

static inline void
clear_buffer()
{
  memset(buffer, 0, 1024);
  memset((char*) buffer + 1024, 0, 1024);
}

static inline void
error_if_cant_open(FILE **fd, const char *filename, const char *flags)
{
  *fd = fopen(filename, flags);

  if (*fd == NULL)
    {
      perror(filename);
      exit(EXIT_FAILURE);
    }
}

static void
print_mbr_partition(const char *image_filename)
{
  struct mbr_part m_part;
  FILE *fd_disk = NULL;

  error_if_cant_open(&fd_disk, image_filename, "rb");

  fseek(fd_disk, 0x1be, SEEK_SET);
  for (int i = 0; i < 3; i++)
    {
      int rc = fread(&m_part, 1, sizeof(struct mbr_part), fd_disk);
      (void) rc;

      printf("primary %u type %.2x filesystem %.2x start %.8x length %.8x\n",
        i, m_part.bootable, m_part.part_type, m_part.lba_first, m_part.length);
    }

  fclose(fd_disk);
  exit(EXIT_SUCCESS);
}


static void
create_with_bootsector(const char *disk_filename, const char *boot_filename)
{
  FILE *fd_image = NULL;
  FILE *fd_boot  = NULL;
  int rc = 0;
  (void) rc;

  error_if_cant_open(&fd_image, disk_filename, "w+b");
  error_if_cant_open(&fd_boot, boot_filename, "r+b");

  clear_buffer();

  rc = fread(buffer, 1, BUFFER_SIZE, fd_boot);
  rc = fwrite(buffer, 1, BUFFER_SIZE, fd_image);

  fclose(fd_image);
  fclose(fd_boot);

  printf("create fd_disk '%s' with bootsector %s\n", disk_filename, boot_filename);
  exit(EXIT_SUCCESS);
}

static void
append_primary(
   const char *disk_filename, const char *part_filename, int fstype, int bootable)
{
  FILE *fd_part = NULL;
  FILE *fd_image = NULL;
  struct mbr_part part;
  int table_offset = 0x1be;
  uint32_t lba_first = 0x800;
  uint32_t length = 0;
  int rc = -1;

  error_if_cant_open(&fd_image, disk_filename, "r+b");

  fseek(fd_image, 0x1be, SEEK_SET);

  for (int i = 0; i < 3; i++)
    {
      rc = fread(&part, 1, sizeof(struct mbr_part), fd_image);
      (void) rc;

      lba_first += part.length * SECTOR_SIZE;
      table_offset = 0x1be + i * sizeof(struct mbr_part);

      if (part.lba_first == 0 && part.length == 0)
        break;

    }

  if (part.length != 0)
    {
      printf("primary partition is fully\n");
      fclose(fd_image);
      exit(EXIT_FAILURE);
    }

  error_if_cant_open(&fd_part, part_filename, "rb");

  rc = -1;
  fseek(fd_image, lba_first, SEEK_SET);
  while (rc != 0)
    {
      clear_buffer();
      rc = fread(buffer, 1, 2048, fd_part);

      length += rc;

      if (rc != 0)
        fwrite(buffer, 1, 2048, fd_image);
    }

  part.lba_first = lba_first;
  part.length = ((length - 1) / 2048 + 1) * 4;
  part.bootable = bootable;
  part.part_type = fstype;

  fseek(fd_image, table_offset, SEEK_SET);
  fwrite(&part, 1, sizeof(struct mbr_part), fd_image);

  printf("added partion type %#x bootable %#x\n", fstype, bootable);
  printf("lba start %.8x length %.8x\n", part.lba_first, part.length);

  fclose(fd_image);
  fclose(fd_part);
  exit(0);
}

static inline void
missing_args(void)
{
  printf(
    "mkvdisk: missing arguments\n"
    "try: mkvdisk --help for more information\n"
  );
  exit(EXIT_FAILURE);
}

static inline void
cli_help(void)
{
  printf(
    "mkvdisk --image <diskimage> --boot <bootfile>\n"
    "   create disk image with bootsector <bootfile>\n"
    "mkvdisk --image <diskimage> --append-partition <partimage> "
    "[bootable] [filesystem] [type]\n"
    "   append a partition image filesystem to disk image\n"
    "mkvdisk --image <diskimage> --print\n"
    "   display partition table\n"
  );

  exit(EXIT_SUCCESS);
}

int
main(int argc, char *argv[])
{
  int fstype = 0x74;
  int bootable = 0;
  int print_flag = 0;

  const char *boot_filename = NULL;
  const char *image_filename = NULL;
  const char *part_filename = NULL;

  char *type = "primary";

  if (argc >= 1 && strcmp(argv[1], "--help") == 0)
    cli_help();

  if (argc < 3)
    missing_args();

  int option_index = 0;
  struct option options[] = {
    { "boot", required_argument, 0, 'b' },
    { "image", required_argument, 0, 'i' },
    { "append-partition", required_argument, 0, 'a' },
    { "bootable", optional_argument, 0, 'B' },
    { "filesystem", required_argument, 0, 'f' },
    { "type", required_argument, 0, 't' },
    { "print", optional_argument, 0, 'p' },
    { 0, 0, 0, 0}
  };

  while (1)
    {
      char c = getopt_long(argc, argv, "b:i:a:Bf:t:p", options, &option_index);

      if (c == -1)
        break;

      switch (c)
        {
          case 'b':
            boot_filename = optarg;
            break;

          case 'i':
            image_filename = optarg;
            break;

          case 'a':
            part_filename = optarg;
            break;

          case 'f':
            sscanf(optarg, "%x", &fstype);
            break;

          case 't':
            type = optarg;
            break;

          case 'B':
            bootable = 0x80;
            break;

          case 'p':
            print_flag = 1;
            break;
        }
    }

  if (image_filename == NULL)
    missing_args();

  if (print_flag == 1)
    print_mbr_partition(image_filename);

  if (boot_filename != NULL)
    create_with_bootsector(image_filename, boot_filename);

  if (part_filename != NULL)
    {
      if (strcmp(type, "primary") == 0)
        append_primary(image_filename, part_filename, fstype, bootable);
      else if (strcmp(type, "extended") == 0)
        {
          printf("extended partition implemeted be later\n");
          exit(EXIT_FAILURE);
        }

      printf("unexpected partition type '%s'\n", type);
      exit(EXIT_FAILURE);
    }

  return EXIT_SUCCESS;
}
