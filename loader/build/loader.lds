OUTPUT_FORMAT("elf32-i386","elf32-i386","elf32-i386");
OUTPUT_ARCH("i386")

ENTRY(entry)

SECTIONS
{
   . = 0x200000;
   __kernel_start__ = .;

   .mbh       . : { *(.mbh) . = ALIGN(4);           }
   .text      . : { *(.text)                        }
   .rodata      : { *(.rodata)                      }
   .data        : { *(.data)                        }
   .bss         : { *(.bss COMMON)                  }
   /DISCARD/  	: { *(.note* .indent .comment)      }
}

