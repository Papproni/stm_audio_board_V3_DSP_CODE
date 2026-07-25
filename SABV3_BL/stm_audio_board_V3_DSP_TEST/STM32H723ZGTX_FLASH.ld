/*
******************************************************************************
**
**  File        : LinkerScript_BOOTLOADER.ld
**
**  Abstract    : BOOTLOADER linker for STM32H723ZGT6
**                Bootloader at 0x08000000 (128 KB)
**
****************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* Stack */
_estack = ORIGIN(RAM_D1) + LENGTH(RAM_D1);
_Min_Heap_Size = 0x2000;
_Min_Stack_Size = 0x4000;

/* MEMORY MAP */
MEMORY
{
  ITCMRAM   (xrw) : ORIGIN = 0x00000000, LENGTH = 64K
  DTCMRAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 128K

  /* Bootloader Flash */
  FLASH_BOOT (rx)   : ORIGIN = 0x08000000, LENGTH = 128K
  FLASH  (rx)       : ORIGIN = 0x08020000, LENGTH = 796K
  USER_FLASH (rxw)  : ORIGIN = 0x080E0000,   LENGTH = 128k

  RAM_D1    (xrw) : ORIGIN = 0x24000000, LENGTH = 320K
  RAM_D2    (xrw) : ORIGIN = 0x30000000, LENGTH = 32K
  RAM_D3    (xrw) : ORIGIN = 0x38000000, LENGTH = 16K
}

SECTIONS
{
  /* VECTOR TABLE IN BOOT FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH_BOOT

  /* CODE IN BOOT FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text*)
    *(.glue_7*)
    *(.glue_7t*)
    *(.eh_frame)

    KEEP(*(.init))
    KEEP(*(.fini))

    . = ALIGN(4);
    _etext = .;
  } >FLASH_BOOT

  /* CONSTANTS IN BOOT FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata*)
    . = ALIGN(4);
  } >FLASH_BOOT

  /* EXCEPTION TABLES */
  .ARM.extab : { *(.ARM.extab*) } >FLASH_BOOT
  .ARM :
  {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH_BOOT

  /* DATA SECTION */
  _sidata = LOADADDR(.data);

  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data*)
    *(.RamFunc*)
    . = ALIGN(4);
    _edata = .;
  } >RAM_D1 AT> FLASH_BOOT

  /* BSS */
  .bss :
  {
    _sbss = .;
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
  } >RAM_D1

  /* STACK + HEAP */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM_D1

  /* DISCARD UNUSED LIBRARIES */
  /DISCARD/ :
  {
    libc.a (*)
    libm.a (*)
    libgcc.a (*)
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}