/*
** Copyright (C) 2014 EADS France, stephane duverger <stephane.duverger@eads.net>
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
#include <e1000.h>
#include <pci_e1000.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

#ifdef __INIT__
static uint16_t e1k_eeprom_read(e1k_info_t *e1k, uint8_t addr)
{
   e1k_eerd_reg_t eerd;

   eerd.addr  = addr;
   eerd.start = 1;
   e1k->eerd->raw = eerd.raw;

   do
   {
      io_wait(10000);
      eerd.raw = e1k->eerd->raw;
   } while(!eerd.done);

   return eerd.data;
}

static void e1k_mac_init(e1k_info_t *e1k, mac_addr_t *mac)
{
   e1k_ra_reg_t ra;
   size_t       i;

   ra.low  = e1k->ra[0].low;
   ra.high = e1k->ra[0].high;

   debug(E1000, "read RAH/RAL for mac: 0x%x 0x%x\n", ra.high, ra.low);

   /* need to read EEPROM */
   if(!ra.low)
   {
      for(i=0 ; i<3 ; i++)
	 mac->word[i] = e1k_eeprom_read(e1k, E1000_EE_MACADDR+i);

      ra.as        = 0;
      ra.av        = 1;
      ra.ral.wlow  = mac->word[0];
      ra.ral.whigh = mac->word[1];
      ra.rah       = mac->word[2];

      /* not sure if device supports 64bits writes */
      e1k->ra[0].low  = ra.low;
      e1k->ra[0].high = ra.high;
   }
   else
   {
      mac->word[0] = ra.ral.wlow;
      mac->word[1] = ra.ral.whigh;
      mac->word[2] = ra.rah;
   }

   for(i=0 ; i<128 ; i++)
      e1k->mta[i] = 0;

#ifdef CONFIG_E1000_DBG
   {
      char str[MAC_STR_SZ];
      mac_str(mac, str);
      debug(E1000, "MAC %s\n", str);
   }
#endif
}

void e1k_init(net_info_t *net)
{
   e1k_info_t   *e1k = &net->arch;
   e1k_ctl_reg_t ctl;

   pci_cfg_e1k(net);

   /* interface reset */
   ctl.raw = 0;
   ctl.rst = 1;
   e1k->ctl->raw = ctl.raw;

   io_wait(1000);

   /* Interface control, link up */
   ctl.raw = e1k->ctl->raw;
   ctl.asde = 1;
   ctl.slu = 1;
   ctl.lrst = 0;
   ctl.phy_rst = 0;
   ctl.ilos = 0;
   ctl.vme = 0;
   e1k->ctl->raw = ctl.raw;

   e1k->fcal = e1k->fcah = e1k->fct = e1k->fcttv = 0;

   e1k_mac_init(e1k, &net->mac);
}

size_t e1k_mem_size()
{
   size_t size =
      RX_DESC_NR*sizeof(e1k_rdesc_t) +
      RX_DESC_NR*RX_BUFF_SZ          +
      TX_DESC_NR*sizeof(e1k_tdesc_t) +
      TX_DESC_NR*TX_BUFF_SZ;

   debug(E1000, "e1k mem size %D\n", size);
   return size;
}

offset_t e1k_mem_init(e1k_info_t *e1k, offset_t area)
{
   loc_t loc;

   loc.linear = area;

   e1k->mem.rdesc = (e1k_rdesc_t*)loc.addr;
   loc.linear += RX_DESC_NR * sizeof(e1k_rdesc_t);

   e1k->mem.rx_dma.linear = loc.linear;
   loc.linear += RX_DESC_NR * RX_BUFF_SZ;

   e1k->mem.tdesc = (e1k_tdesc_t*)loc.addr;
   loc.linear += TX_DESC_NR * sizeof(e1k_tdesc_t);

   e1k->mem.tx_dma.linear = loc.linear;
   loc.linear += TX_DESC_NR * TX_BUFF_SZ;

   debug(E1000,
	 "rdesc  0x%X, tdesc  0x%X\n"
	 "rxbuff 0x%X, txbuff 0x%X\n"
	 ,e1k->mem.rdesc, e1k->mem.tdesc
	 ,e1k->mem.rx_dma.linear, e1k->mem.tx_dma.linear);

   return loc.linear;
}

void e1k_rx_init(e1k_info_t *e1k)
{
   e1k_rctl_reg_t       rctl;
   e1k_ic_reg_t         ims;
   size_t               i;
   uint32_t __unused__  clr;
   loc_t                base;

   /* clear interrupts */
   e1k->imc->raw = -1U;
   clr = e1k->icr->raw;

   /* enable interrupts */
   ims.lsc = 1;
   ims.rxo = 1;
   ims.rxseq = 1;
   ims.rxdmt0 = 1;
   ims.rxt0 = 1;

   e1k->ims->raw = ims.raw;

   /* receive descriptors */
   for(i=0 ; i<RX_DESC_NR ; i++)
   {
      e1k->mem.rdesc[i].addr = e1k->mem.rx_dma.linear + i*RX_BUFF_SZ;
      e1k->mem.rdesc[i].sts.raw = 0;

      debug(E1000, "set rdesc%D 0x%X buffer addr = 0x%X\n"
	    ,i, &e1k->mem.rdesc[i], e1k->mem.rdesc[i].addr);
   }

   /* receive ring buffer */
   debug(E1000, "read RDBAH/RDBAL = 0x%x 0x%x\n"
	 ,e1k->rx.bah->raw, e1k->rx.bal->raw);

   base.addr = (void*)e1k->mem.rdesc;
   e1k->rx.bah->raw = base.high;
   e1k->rx.bal->raw = base.low;

   debug(E1000, "read RDBAH/RDBAL = 0x%x 0x%x\n"
	 ,e1k->rx.bah->raw, e1k->rx.bal->raw);

   /* len/head/tail pointers */
   debug(E1000, "read RDL/RDH/RDT = 0x%x 0x%x 0x%x\n"
	 ,e1k->rx.dl->raw, e1k->rx.dh->raw, e1k->rx.dt->raw);

   e1k->rx.dl->raw  = RX_DESC_NR * sizeof(e1k_rdesc_t);
   e1k->rx.dh->raw  = 0;
   e1k->rx.tail.raw = RX_DESC_NR - 1;
   e1k->rx.dt->raw  = e1k->rx.tail.raw;

   debug(E1000, "read RDL/RDH/RDT = 0x%x 0x%x 0x%x\n"
	 ,e1k->rx.dl->raw, e1k->rx.dh->raw, e1k->rx.dt->raw);

   /* receive control */
   rctl.raw = 0;
   rctl.sbp = 1;
   rctl.upe = 1;
   rctl.mpe = 1;
   rctl.rdmts = E1K_RCTL_MIN_TH_8;

   rctl.secrc = 1;
   rctl.lpe = 1;
   rctl.bam = 1;
   e1k_rctl_sz_2k(&rctl);

   e1k->rx.ctl->raw = rctl.raw;
}

void e1k_tx_init(e1k_info_t *e1k)
{
   e1k_tctl_reg_t  tctl;
   e1k_tipg_reg_t  ipg;
   size_t          i;
   loc_t           base;

   /* transmit desc */
   for(i=0 ; i<TX_DESC_NR ; i++)
   {
      e1k->mem.tdesc[i].addr = e1k->mem.tx_dma.linear + i*TX_BUFF_SZ;
      debug(E1000, "set tdesc%D 0x%X buffer addr = 0x%X\n"
	    ,i, &e1k->mem.tdesc[i], e1k->mem.tdesc[i].addr);
   }

   /* transmit ring buffer */
   debug(E1000, "read TDBAH/TDBAL = 0x%x 0x%x\n"
	 ,e1k->tx.bah->raw, e1k->tx.bal->raw);

   base.addr = (void*)e1k->mem.tdesc;
   e1k->tx.bah->raw = base.high;
   e1k->tx.bal->raw = base.low;

   debug(E1000, "read TDBAH/TDBAL = 0x%x 0x%x\n"
	 ,e1k->tx.bah->raw, e1k->tx.bal->raw);

   /* len/head/tail pointers */
   debug(E1000, "read TDL/TDH/TDT = 0x%x 0x%x 0x%x\n"
	 ,e1k->tx.dl->raw, e1k->tx.dh->raw, e1k->tx.dt->raw);

   e1k->tx.dl->raw = TX_DESC_NR * sizeof(e1k_tdesc_t);
   e1k->tx.dh->raw = 0;
   e1k->tx.tail.raw = 0;
   e1k->tx.dt->raw = e1k->tx.tail.raw;

   debug(E1000, "read TDL/TDH/TDT = 0x%x 0x%x 0x%x\n"
	 ,e1k->tx.dl->raw, e1k->tx.dh->raw, e1k->tx.dt->raw);

   /* transmit control */
   tctl.raw = 0;
   tctl.psp = 1;
   tctl.cold = 0x40;
   tctl.en = 1;
   e1k->tx.ctl->raw = tctl.raw;

   /* transmit IPG */
   ipg.raw = 0;
   ipg.ipgt = 10;
   ipg.ipgr1 = 8;
   ipg.ipgr2 = 6;
   e1k->tx.ipg->raw = ipg.raw;
}
#endif

void e1k_rx_on(net_info_t *net)
{
   e1k_info_t      *e1k = &net->arch;
   e1k_rctl_reg_t  rctl;

   debug(E1000, "enable receive control\n");
   io_wait(100000);

   rctl.raw = e1k->rx.ctl->raw;
   rctl.en = 1;
   e1k->rx.ctl->raw = rctl.raw;
}

void e1k_tx_on(net_info_t *net)
{
   e1k_info_t     *e1k = &net->arch;
   e1k_tctl_reg_t tctl;

   tctl.raw = e1k->tx.ctl->raw;
   tctl.en = 1;
   e1k->tx.ctl->raw = tctl.raw;
}

/*
** XXX:
** Better do a memory allocator for TX dma packet buffers
** and store them in tx desc only when needed
*/
offset_t e1k_tx_get_pktbuf(e1k_info_t *e1k)
{
   debug(E1000, "--> get TX pktbuf [TDT %d]\n", e1k->tx.tail.raw);
   return e1k->mem.tdesc[e1k->tx.tail.raw].addr;
}

void e1k_send_pkt(net_info_t *net, loc_t pkt, size_t len)
{
   e1k_info_t           *e1k;
   volatile e1k_tdesc_t *dsc;
   e1k_tdesc_cmd_t       cmd;

   e1k = &net->arch;
   dsc = &e1k->mem.tdesc[e1k->tx.tail.raw];

   dsc->addr = pkt.linear;
   dsc->len = len;

   debug(E1000, "--> [TDT %d] len %d\n", e1k->tx.tail.raw, dsc->len);

   cmd.raw = 0;
   cmd.eop = 1;
   cmd.rs = 1;
   dsc->cmd.raw = cmd.raw;

   e1k->tx.tail.raw = (e1k->tx.tail.raw+1)%TX_DESC_NR;
   e1k->tx.dt->raw = e1k->tx.tail.raw;

#ifdef CONFIG_E1000_DBG
   {
      e1k_tdesc_sts_t sts;
      debug(E1000, "sending packet ... ");
      do
      {
	 io_wait(10000);
	 sts.raw = dsc->sts.raw;

	 if(e1k->tx.ctl->raw == -1U)
	 {
	    int i;
	    pci_cfg_val_t *pci = &net->pci;

	    debug(E1000, "CTL 0x%x TCTL 0x%x\n", e1k->ctl->raw, e1k->tx.ctl->raw);

	    pci->addr.reg = PCI_CFG_CMD_STS_OFFSET;
	    pci_cfg_read(pci);

	    debug(PCI_E1000, "e1k CMD io %d mm %d dma %d\n"
		  ,pci->cs.cmd.io, pci->cs.cmd.mm, pci->cs.cmd.bus_master);

	    for(i=0 ; i<6 ; i++)
	    {
	       pci->addr.reg = PCI_CFG_BAR_OFFSET + i*4;
	       pci_cfg_read(pci);
	       debug(E1000, "e1k BAR%d 0x%x)", i, pci->br.raw);
	    }
	    panic("corrupted e1k config space !");
	 }
      } while(!sts.dd);
      debug(E1000, "done.\n");
   }
#endif
}

size_t e1k_recv_pkt(net_info_t *net, loc_t data, size_t len)
{
   e1k_info_t           *e1k;
   volatile e1k_rdesc_t *dsc;
   e1k_rdesc_sts_t       sts;

   e1k = &net->arch;
   e1k->rx.tail.raw = (e1k->rx.tail.raw+1)%RX_DESC_NR;
   dsc = &e1k->mem.rdesc[e1k->rx.tail.raw];
   sts.raw = dsc->sts.raw;

   if(!sts.dd)
      return 0;

   if(!sts.eop)
      debug(E1000, "[RDT %d] !!! WRN !!! eop == 0\n", e1k->rx.tail.raw);

   memcpy(data.addr, (void*)dsc->addr, min(dsc->len, len));
   debug(E1000, "<-- [RDT %d] len %d eop %d\n", e1k->rx.tail.raw,dsc->len,sts.eop);

#ifdef CONFIG_E1000_DBG
   {
      volatile e1k_rdesc_t *d;
      e1k_rdesc_sts_t s;
      size_t i=0;

      for(i=0 ; i<dsc->len ; i++)
	 printf(" %x", data.u8[i]);
      printf("\n");

      debug(E1000, "[deep check] RDT %d RDH %d\n",e1k->rx.dt->raw,e1k->rx.dh->raw);
      for(i=0 ;i<RX_DESC_NR ; i++)
      {
	 d = &e1k->mem.rdesc[i];
	 s.raw = d->sts.raw;
	 printf(" %d:%d", i, s.dd);
      }
      printf("\n");
   }
#endif

   dsc->sts.raw = 0;
   e1k->rx.dt->raw = e1k->rx.tail.raw;
   return dsc->len;
}

/* static void e1k_recv_status(net_info_t *net) */
/* { */
/*    e1k_info_t      *e1k = &net->arch; */
/*    e1k_sts_reg_t   sts; */
/*    e1k_ic_reg_t    icr; */

/*    loc_t rdfh  = {.linear = e1k->base.linear + 0x2410}; */
/*    loc_t rdft  = {.linear = e1k->base.linear + 0x2418}; */
/*    loc_t rdfhs = {.linear = e1k->base.linear + 0x2420}; */
/*    loc_t rdfts = {.linear = e1k->base.linear + 0x2428}; */
/*    loc_t rdfpc = {.linear = e1k->base.linear + 0x2430}; */

/*    sts.raw = e1k->sts->raw; */
/*    icr.raw = e1k->icr->raw; */

/*    debug(E1000, */
/* 	 "e1k status:" */
/* 	 " fd %d lu %d fid %d txoff %d speed %d tbi %d" */
/* 	 " asdv %d pci66 %d bus64 %d pcix %d  pcispeed %d\n" */
/* 	 ,sts.fd,sts.lu,sts.fid,sts.txoff,sts.speed,sts.tbimode */
/* 	 ,sts.asdv,sts.pci66,sts.bus64,sts.pcix,sts.pcixspd */
/*       ); */

/*    debug(E1000, */
/* 	 "e1k icr:" */
/* 	 " txdw %d txqe %d lsc %d rxseq %d rxdmt0 %d" */
/* 	 " rxo %d rxt0 %d mdac %d rxcfg %d phyint %d" */
/* 	 " gpi %d:%d txdlow %d srpd %d\n" */
/* 	 ,icr.txdw,icr.txqe,icr.lsc,icr.rxseq,icr.rxdmt0 */
/* 	 ,icr.rxo,icr.rxt0,icr.mdac,icr.rxcfg,icr.phyint */
/* 	 ,icr.gpi_sdp6,icr.gpi_sdp7,icr.txdlow,icr.srpd */
/*       ); */

/*    debug(E1000, "e1k rx fifo: h 0x%x t 0x%x hs 0x%x ts 0x%x pc 0x%x\n" */
/* 	 ,*rdfh.u32, *rdft.u32, *rdfhs.u32, *rdfts.u32, *rdfpc.u32); */
/* } */
