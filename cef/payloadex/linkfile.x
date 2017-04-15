OUTPUT_FORMAT("elf32-littlemips")
OUTPUT_ARCH(mips)

ENTRY(_start)

SECTIONS
{
  . = 0x88FC0000;
  .text.start : {
    *(.text.start)
  }
  .text : {
    *(.text)
  }
  .data : {
    *(.data)
  }
  .rodata : {
    *(.rodata)
  }
  .bss : {
    *(.bss)
  }
}