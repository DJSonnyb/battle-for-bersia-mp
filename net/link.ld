SECTIONS {
  . = 0xAF1000;
  .text : { *(.text.net_*) *(.text*) }
  /* v95: constants and initialised data live in section .kjb (0xB16000), zero data at 0xB1A000 - the .kuf net area
     (0xAF1000-0xB00000, Keaton's fixes from 0xB00000) only holds code now */
  .rodata 0xB16000 : { *(.rodata) *(.rodata.cst*) }
  .data : { *(.data*) }
  .bss 0xB1A000 (NOLOAD) : { *(.bss*) *(COMMON) }
  /* v99: overflow code (load-time / rare functions) in .kjb at 0xB1C000 - section .kjb is executable */
  .ktext 0xB1C000 : { *(.ktext*) *(.rodata.str*) }   /* TEST23: strings moved here (data area full) */
  /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) }
}
