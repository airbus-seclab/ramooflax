OUTPUT_FORMAT("elf64-x86-64","elf64-x86-64","elf64-x86-64");
OUTPUT_ARCH("i386:x86-64")

ENTRY(entry)

PHDRS
{
   load  PT_LOAD    FLAGS (7);
   dyn   PT_DYNAMIC FLAGS (6);
   drop  PT_LOAD    FLAGS (6);
}

SECTIONS
{
   . = 0;

   .info_hdr : { *(.info_hdr)   } : load
   .idt_jmp  : { *(.idt_jmp)    } : load
   .text     : { *(.text*)      } : load
   .rodata   : { *(.rodata)     } : load
   .got      : { *(.got)        } : load
   .data     : { *(.data*)      } : load
   .bss      : { *(.bss COMMON) } : load

   .dynamic  : { *(.dynamic)    } : drop : dyn
   .relocs   : { *(.rel*)       } : drop
   .got.plt  : { *(.got.plt)    } : drop

   /DISCARD/ : { *(.interp .note* .eh_frame .indent .comment .dynsym .dynstr .hash) }
}
