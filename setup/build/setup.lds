OUTPUT_FORMAT("elf64-x86-64","elf64-x86-64","elf64-x86-64");
OUTPUT_ARCH("i386:x86-64")

ENTRY(entry)

SECTIONS
{
   . = 0x300000;
   __kernel_start__ = .;

   .text      . : { *(.text)                        }
   .rodata      : { *(.rodata)                      }
   .data        : { *(.data)                        }
   .bss         : { *(.bss COMMON)                  }
   /DISCARD/  	: { *(.note* .indent .comment)      }
}

