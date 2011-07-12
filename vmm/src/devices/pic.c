/*
** Copyright (C) 2011 EADS France, stephane duverger <stephane.duverger@eads.net>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License along
** with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include <dev_pic.h>
#include <irq.h>
#include <int.h>
#include <info_data.h>
#include <asmutils.h>
#include <debug.h>

extern info_data_t *info;

int __dev_pic_icw1(pic_t *pic, uint8_t data)
{
#ifdef DEBUG_RELEASE_IRQ
   __release_irq();
   out(data, pic->base);
#else
   pic->icw1.raw = data;

   debug(DEV_PIC, "@0x%x icw1 0x%x\n", __rip.low, data);

   if(pic->icw1.ltim)
   {
      debug(DEV_PIC, "level triggered interrupt mode not supported !\n");
      return 0;
   }

   /* clear mask register  */
   pic->imr.raw = 0;

   /* set slave addr to 7 */
   pic->icw3.raw = 0;

   if(pic->base == PIC1)
      pic->icw3.master.irq7 = 1;
   else
      pic->icw3.slave.id = 7;

   /* clear Special Mask Mode */
   pic->ocw3.smm = 0;

   /* read IRR */
   pic->ocw3.reg = 2;

   /* if no icw4 coming, reset it */
   if(!pic->icw1.icw4)
      pic->icw4.raw = 0;

   pic->wait_icw = 2;
#endif

   /* XXX: clear pending irq !? */
   irq_clear_all_pending();

   return 1;
}

int __dev_pic_icw2(pic_t *pic, uint8_t data)
{
   pic->icw2.raw = data & 0xf8;

   debug(DEV_PIC, "@0x%x icw2 0x%x\n", __rip.low, pic->icw2.raw);

   if(!pic->icw1.single)
      pic->wait_icw = 3;
   else if(pic->icw1.icw4)
      pic->wait_icw = 4;
   else
      pic->wait_icw = 0;

   return 1;
}

int __dev_pic_icw3(pic_t *pic, uint8_t data)
{
   pic->icw3.raw = data;

   debug(DEV_PIC, "@0x%x icw3 0x%x\n", __rip.low, pic->icw3.raw);

   if(pic->icw1.icw4)
      pic->wait_icw = 4;
   else
      pic->wait_icw = 0;

   return 1;
}

int __dev_pic_icw4(pic_t *pic, uint8_t data)
{
   pic->icw4.raw = data;

   debug(DEV_PIC, "@0x%x icw4 0x%x\n", __rip.low, data);

   if(!pic->icw4.x86)
   {
      debug(DEV_PIC, "MCS-80/85 mode not supported !\n");
      return 0;
   }

   if(pic->icw4.buff)
   {
      debug(DEV_PIC, "buffered mode not supported !\n");
      return 0;
   }

   /* aeoi only allowed on master */
   if(pic->icw4.aeoi)
   {
      if(pic->base == PIC2)
      {
	 debug(DEV_PIC, "forcing slave to non auto EOI !\n");
	 pic->icw4.aeoi = 0;
      }
      else
	 debug(DEV_PIC, "pic aeoi set !\n");
   }

   pic->wait_icw = 0;
   return 1;
}

int __dev_pic_imr(pic_t *pic, uint8_t vm_imr)
{
   pic_ocw1_t real_imr;
/*
   if((pic->imr.raw>>1) ^ (vm_imr>>1))
      debug(DEV_PIC, "pic%d imr change to %b\n", pic->base==PIC1?1:2, vm_imr);
*/
/*
   if(pic->base == PIC2)
      vm_imr &= ~(1<<3);
*/

   pic->imr.raw = vm_imr;

   if(pic->base == PIC1)
      real_imr.raw = vm_imr & ~((1<<PIC_UART1_IRQ)|(1<<PIC_SLAVE_IRQ));
   else
      real_imr.raw = vm_imr;

   if(pic->base == PIC1 && real_imr.raw & (1<<PIC_UART1_IRQ))
      debug(DEV_PIC, "mofo: 0x%x\n", real_imr.raw);

   out(real_imr.raw, PIC_IMR(pic->base));

   return resolve_hard_interrupt();
}

int __dev_pic_ocw2(pic_t *pic, uint8_t data)
{
   pic->ocw2.raw = data;

   out(data, PIC_OCW2(pic->base));
   return 1;
}

int __dev_pic_status(pic_t *pic, io_insn_t *io)
{
   uint8_t   *target;
   io_size_t sz = { .available = 1 };

   if((io->port & 1) == 0)
   {
      if(pic->ocw3.poll)
	 target = &pic->poll;
      else if(pic->ocw3.reg == 2)
	 target = &pic->irr;
      else if(pic->ocw3.reg == 3)
	 target = &pic->isr;
      else
      {
	 debug(DEV_PIC, "read status from bad register (0x%x) !\n", pic->ocw3.reg);
	 return 0;
      }

      *target = in(io->port);

      /* XXX: never signal interrupt for uart1 */
      if(pic->base == PIC1 && !pic->ocw3.poll)
	 *target &= ~(1<<PIC_UART1_IRQ);
   }
   else
      target = &pic->imr.raw;

   return dev_io_insn(io, target, &sz);
}

int __dev_pic_ocw3(pic_t *pic, uint8_t data)
{
   pic->ocw3.raw = data;

   debug(DEV_PIC, "@0x%x ocw3 0x%x\n", __rip.low, data);

   if(pic->ocw3.smm == PIC_OCW3_SPECIAL_MASK_MODE_ENABLED)
   { 
      debug(DEV_PIC, "special mask mode not supported !\n");
      return 0;
   }

   out(data, PIC_OCW3(pic->base));

   if(pic->ocw3.poll)
      debug(DEV_PIC, "poll command\n");

   return 1;
}

int dev_pic(pic_t *pic, io_insn_t *io)
{
   io_size_t sz = { .available = 1 };

   if(io->in)
      return __dev_pic_status(pic, io);
   else
   {
      uint8_t data;

      if(!dev_io_insn(io, &data, &sz))
	 return 0;

      /* ICW1/OCW2/OCW3 */
      if((io->port & 1) == 0)
      {
	 if(is_pic_icw1(data))
	    return __dev_pic_icw1(pic, data);
	 else if(is_pic_ocw2(data))
	    return __dev_pic_ocw2(pic, data);
	 else if(is_pic_ocw3(data))
	    return __dev_pic_ocw3(pic, data);
	 else
	 {
	    debug(DEV_PIC, "invalid command (0x%x) !\n", data);
	    return 0;
	 }
      }
      /* IMR/ICW2/ICW3/ICW4 : complete/uncomplete configuration */
      else
      {
	 if(pic->wait_icw < 2)
	    return __dev_pic_imr(pic, data);
	 else if(pic->wait_icw == 2)
	    return __dev_pic_icw2(pic, data);
	 else if(pic->wait_icw == 3)
	    return __dev_pic_icw3(pic, data);
	 else
	    return __dev_pic_icw4(pic, data);
      }
   }

   return 1;
}

int dev_pic_acting(uint8_t irq, uint32_t *vector)
{
   pic_t   *pic;
   uint8_t r_irq;

   if(irq < 8)
   {
      r_irq = irq;
      pic = &info->vm.dev.pic[0];
   }
   else
   {
      r_irq = irq - 8;
      pic = &info->vm.dev.pic[1];
   }

   *vector = pic->icw2.raw + r_irq;

   if((~(pic->imr.raw) & (1<<r_irq)))
   {
      /* only master */
      if(pic->icw4.aeoi)
	 pic_eoi(pic->base);

      return 1;
   }

   /* spurious */
   pic->isr = pic_isr(pic->base);

   if(!(pic->isr & (1<<r_irq)))
      return 1;


   /* disabled irq line */
   debug(DEV_PIC,
	  "#DIS_IRQ: irq %d (%d) (base:%d) vector %d\n",
	  irq, r_irq, pic->icw2.raw, *vector
     );

   debug(DEV_PIC,
	  "pic :\n"
	  " irr %b\n"
	  " imr %b\n"
	  " isr %b\n",
	  pic_imr(pic->base), pic_isr(pic->base), pic_irr(pic->base)
     );

   return 0;
}

uint8_t pic_irr(uint8_t base)
{
   uint8_t irr;
   pic_t   *pic;

   if(base == PIC1)
      pic = &info->vm.dev.pic[0];
   else
      pic = &info->vm.dev.pic[1];

   out(0xa, PIC_OCW3(base));
   irr = in(base);
   out(pic->ocw3.raw, PIC_OCW3(base));

   return irr;
}

uint8_t pic_isr(uint8_t base)
{
   uint8_t isr;
   pic_t   *pic;

   if(base == PIC1)
      pic = &info->vm.dev.pic[0];
   else
      pic = &info->vm.dev.pic[1];

   out(0xb, PIC_OCW3(base));
   isr = in(base);
   out(pic->ocw3.raw, PIC_OCW3(base));

   return isr;
}
