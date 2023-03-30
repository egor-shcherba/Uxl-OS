#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include <string.h>
#include <fs/uxlfs.h>

void
missing_args(void)
{
  printf(
    "uxlfs: missing arguments\n"
    "Try: uxlfs --help for more information\n"
  );

  exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
  char *file = NULL;
  char *copy_dir = NULL;
  uint32_t bs = 512;
  uint32_t count = 10000;

  int option_index = 0;
  struct option options[] = {
    { "create-fs",    required_argument, 0, 'c'},
    { "bs",           required_argument, 0, 'b'},
    { "count",        required_argument, 0, 'C'},
    { "dirrectory",   required_argument, 0, 'd'},
    { 0, 0, 0, 0 }
  };

  while (1)
    {
      char c = getopt_long(argc, argv, "c:b:C:d:", options, &option_index);

      if (c == -1)
        break;

      switch (c)
        {
          case 'c':
            file = optarg;
            break;

          case 'b':
            bs = atoi(optarg);
            break;

          case 'C':
            count = atoi(optarg);
            break;

          case 'd':
            copy_dir = optarg;
            break;
        }
    }

  if (file != NULL)
    {
      create_fs(file, bs, count);

      if (copy_dir != NULL)
        {
          printf("copy dirretcory %s\n", copy_dir);
          sync_fs();
          mount_fs(file);
          cpdir_in(copy_dir, "/");
        }

      sync_fs();
      exit(EXIT_SUCCESS);
    }

  mount_fs(argv[1]);

  if (strcmp(argv[2], "cpdir-in") == 0)
    cpdir_in(argv[3], argv[4]);

  if (strcmp(argv[2], "cpdir-out") == 0)
    cpdir_out(argv[3], argv[4]);

  if (strcmp(argv[2], "cpin") == 0 )
    {
      printf("copy '%s' to image '%s'", argv[3], argv[4]);
      cpdir_out(argv[3], argv[4]);
      printf("done.\n");
    }

  if (strcmp(argv[2], "cpout") == 0)
    {
      printf("extract from image '%s' to '%s'", argv[3], argv[4]);
      cpdir_out(argv[3], argv[4]);
      printf("done.\n");
    }

  if (strcmp(argv[2], "ls") == 0)
    ls(argv[3]);

  if (strcmp(argv[2], "spinfo") == 0)
    spinfo();

  sync_fs();

  return EXIT_SUCCESS;
}