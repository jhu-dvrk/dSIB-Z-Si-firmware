/* STM32G474CBT6 compatible layout: 128 KiB Flash, 128 KiB SRAM total.
   SRAM1 + SRAM2 are contiguous and DMA-accessible. CCM is CPU-only. */
MEMORY
{
  FLASH  (rx)  : ORIGIN = 0x08000000, LENGTH = 128K
  RAM    (xrw) : ORIGIN = 0x20000000, LENGTH = 96K
  CCMRAM (xrw) : ORIGIN = 0x10000000, LENGTH = 32K
}

ENTRY(Reset_Handler)

_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
  .isr_vector :
  {
    KEEP(*(.isr_vector))
  } > FLASH

  .text :
  {
    *(.text*)
    *(.rodata*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame*)
  } > FLASH

  .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } > FLASH
  __exidx_start = .;
  .ARM.exidx : { *(.ARM.exidx*) } > FLASH
  __exidx_end = .;

  . = ALIGN(4);
  _sidata = LOADADDR(.data);
  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } > RAM AT> FLASH

  .bss (NOLOAD) :
  {
    . = ALIGN(4);
    _sbss = .;
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
  } > RAM

  .ccmram (NOLOAD) :
  {
    *(.ccmram*)
  } > CCMRAM

  ._user_heap_stack (NOLOAD):
  {
    . = ALIGN(8);
    PROVIDE(end = .);
    PROVIDE(_end = .);
    . = . + 0x1000;
    . = ALIGN(8);
  } > RAM
}

PROVIDE(_Heap_Begin = _end);
PROVIDE(_Heap_Limit = _estack);
