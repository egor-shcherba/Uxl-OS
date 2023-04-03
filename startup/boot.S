.code32
.set MULTIBOOT_HEADER,   0x1BADB002
.set MULTIBOOT_FLAGS,    0x0
.set MULTIBOOT_CHECKSUM, -(MULTIBOOT_HEADER + MULTIBOOT_FLAGS)

.set VADDR, 0xC0000000

.section .multiboot
  .long MULTIBOOT_HEADER
  .long MULTIBOOT_FLAGS
  .long MULTIBOOT_CHECKSUM

.section .bss, "aw", @nobits
BOOT_DIR:
  .skip 0x1000

BOOT_TABLE_0:
  .skip 0x1000

.section .text
.globl _start
_start:

  # calcualte size bss section
  mov $__KERN_BSS_START, %ebx
  mov $__KERN_BSS_END, %ecx

  sub %ebx, %ecx
  shr $0x2, %ecx

  # calculate physical address bss section
  mov $__KERN_BSS_START, %edi
  sub $VADDR, %edi

  # fill zero bss section
  xor %eax, %eax
0:
  stosl
  loop 0b

  # map first 4mb virt_addr = phys_addr
  # calcualte physical address boot_table_0
  mov $BOOT_TABLE_0, %edi
  sub $VADDR, %edi

  # save phys address boot_table_0 in stack
  push %edi

  # start phys addr
  mov $0x0, %ebx
  # fill boot_table_0
  mov $0x400, %ecx
1:
  mov %ebx, %eax
  orl $0x7, %eax
  orl %ebx, %eax
  stosl
  addl $0x1000, %ebx
  loop 1b

  # caluclate phys address page dir
  mov $BOOT_DIR, %edi
  sub $VADDR, %edi

  # restore phys addr boot_table_0 from stack
  pop %eax
  orl $0x7, %eax

  # ebx = phys addr page_table_0
  mov %eax, %ebx

  # save
  push %edi

  # set first entry in page_dir
  mov %eax, (%edi)

  # set 0xC00... addr entry in page dir
  mov $VADDR, %eax
  shr $22, %eax
  shl $2, %eax
  addl %eax, %edi
  mov %ebx, (%edi)

  # mov phys addr page_dir in CR3
  pop %edi
  mov %edi, %eax
  mov %eax, %cr3

  # enable pagging
  mov %cr0, %eax
  orl $0x80000000, %eax
  mov %eax, %cr0

  jmp main
  hlt
