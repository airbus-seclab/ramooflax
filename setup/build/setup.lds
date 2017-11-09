OUTPUT_FORMAT("elf64-x86-64","elf64-x86-64","elf64-x86-64");
OUTPUT_ARCH("i386:x86-64")

ENTRY(entry)

PHDRS
{
   phstack PT_LOAD FLAGS (6);
   phsetup PT_LOAD FLAGS (7);
}

SECTIONS
{
   . = 0x300000;
   .stack    : { *(.stack)                       } : phstack

   __kernel_start__ = .;
   .text     : { *(.text)                        } : phsetup
   .rodata   : { *(.rodata)                      } : phsetup
   .data     : { *(.data)                        } : phsetup
   .bss      : { *(.bss COMMON)                  } : phsetup
   /DISCARD/ : { *(.note* .indent .comment)      } : phsetup
}
