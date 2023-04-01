CC = cc
AS = as
LD = ld

OLEVEL = -O2

CFLAGS = -Wall -Wextra -pedantic -std=c11 $(OLEVEL) \
	-m32 -nostdinc -mgeneral-regs-only \
	-ffreestanding -fno-stack-protector \
	-fno-omit-frame-pointer \
	-fno-common \
	-mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx \
	-I./ -I./inc

ASFLAGS = \
	--32

LDFLAGS = \
	-static -nostdlib -O2 \
	-T ./linker.lds

SRC = \
	startup/boot.S \
	main.c

OBJ = $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(SRC)))

KERNEL_BINARY = ./kernel.elf

MKUXLFS = ./mkuxlfs
MKVDISK = ./mkvdisk

QEMU = qemu-system-x86_64
QEMU_FLAGS = \
	-m 4G -debugcon stdio

all: clean-kernel $(OBJ)
	$(LD) $(LDFLAGS) -o $(KERNEL_BINARY) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(AS) $(ASFLAGS) -o $@ $<

build-tools:
	rm -rf $(MKULXFS) $(MKVDISK)
	make -C ./tools/mkuxlfs/ BUILD_PATH=$(shell pwd) APP_NAME=$(MKUXLFS)
	make -C ./tools/mkvdisk/ BUILD_PATH=$(shell pwd)

qemu:
	$(QEMU) $(QEMU_FLAGS) -kernel $(KERNEL_BINARY)

clean-kernel:
	rm -rf $(OBJ) $(KERNEL_BINARY)

clean-tools:
	rm -rf $(MKUXLFS) $(MKVDISK)
	make -C ./tools/mkuxlfs/ APP_NAME=$(MKUXLFS) clean
	make -C ./tools/mkvdisk/ clean

clean: clean-kernel
