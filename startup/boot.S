.code32

.set MULTIBOOT_HEADER,    0x1BADB002
.set MULTIBOOT_FLAGS,     0x0
.set MULTIBOOT_CHECKSUM, -(MULTIBOOT_HEADER + MULTIBOOT_FLAGS)

.section .multiboot
  .long MULTIBOOT_HEADER
  .long MULTIBOOT_FLAGS
  .long MULTIBOOT_CHECKSUM

.section .text
.globl _start
_start:
  jmp main

  hlt
