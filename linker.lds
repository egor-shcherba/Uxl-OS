OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)

ENTRY(_start)

SECTIONS
{
  . = 0x00100000;

  .text ALIGN(4K) :
  {
    *(.multiboot)
    *(.text)
    *(.rodata)
  }

  .data ALIGN(4K) :
  {
    *(.data)
  }

  .bss ALIGN(4K) :
  {
    *(.bss)
  }

  /DISCARD/ :
  {
    *(.eh_frame)
  }
}
