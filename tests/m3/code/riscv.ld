OUTPUT_ARCH( "riscv" )
ENTRY( _start )
SECTIONS
{

  /* text: test code section */
  . = 0x80000000;
 .text :
 {
 *(.text)
 }

  .tohost ALIGN(0x1000) : { *(.tohost) }

  .fromhost ALIGN(0x40): { *(.fromhost) }

  /* data segment */
  .data : { *(.data) }

  /* End of uninitalized data segement */
  _end = .;
}
