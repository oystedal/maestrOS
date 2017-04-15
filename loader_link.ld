ENTRY(_start)
/* OUTPUT_FORMAT(i686-elf) */

SECTIONS {
  . = 0x8000;
  _loader_start = .;
  .text BLOCK(4K) : ALIGN(4K) {
    *(.text)
  }

  .bss BLOCK(4K) : ALIGN(4K) {
    *(COMMON)
    *(.bss)
  }
  _loader_end = .;
}
