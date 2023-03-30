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

QEMU = qemu-system-x86_64
QEMU_FLAGS = \
	-m 4G -debugcon stdio

all: clean-kernel $(OBJ)
	$(LD) $(LDFLAGS) -o $(KERNEL_BINARY) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(AS) $(ASFLAGS) -o $@ $<

qemu:
	$(QEMU) $(QEMU_FLAGS) -kernel $(KERNEL_BINARY)

clean-kernel:
	rm -rf $(OBJ) $(KERNEL_BINARY)

clean: clean-kernel
