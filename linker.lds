OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)

ENTRY(_start)

SECTIONS
{
  VIRT_ADDR = 0xC0100000;
  . = 0x00100000;

  .text ALIGN(4K) :
  {
    PROVIDE(__KERN_CODE_START = . );
    *(.multiboot)
    *(.text)
    *(.rodata)
    PROVIDE(__KERN_CODE_END = . );
  }

  . += 0xC0000000;

  .data ALIGN(4K) :
  {
    PROVIDE(__KERN_DATA_START = . );
    *(.data)
    PROVIDE(__KERN_DATA_END = . );
  }

  .bss ALIGN(4K) :
  {
    PROVIDE(__KERN_BSS_START = . );
    *(.COMMON)
    *(.bss)
    *(.stack)
    PROVIDE(__KERN_BSS_END = . );
  }

  /DISCARD/ :
  {
    *(.eh_frame)
  }
}
