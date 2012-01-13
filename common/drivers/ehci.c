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
#include <ehci.h>
#include <config.h>
#include <string.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

#ifdef __INIT__
void ehci_init()
{
   dbgp_info_t *dbgp_i = &info->hrd.dev.dbgp;

   pci_cfg_dbgp(dbgp_i);

#ifndef CONFIG_EHCI_FULL
   ehci_usbcmd_reg_t usbcmd;
   ehci_usbsts_reg_t usbsts;
   ehci_cfgflg_reg_t cfgflg;

   usbcmd.raw = dbgp_i->ehci_opr->usbcmd.raw;
   usbsts.raw = dbgp_i->ehci_opr->usbsts.raw;
   cfgflg.raw = dbgp_i->ehci_opr->cfgflg.raw;

   if((usbcmd.run && !usbsts.hlt) && cfgflg.cf)
      ehci_fast_init(dbgp_i);
   else
#endif
      ehci_full_init(dbgp_i);

   debug(EHCI, "ehci ready\n");
}

void ehci_fast_init(dbgp_info_t *dbgp_i)
{
   debug(EHCI, "ehci fast init\n");
   ehci_dbgp_fast_init(dbgp_i);
}

void ehci_full_init(dbgp_info_t *dbgp_i)
{
   debug(EHCI, "ehci full init\n");
   ehci_controller_preinit(dbgp_i);
   ehci_dbgp_full_init(dbgp_i);
   ehci_controller_postinit(dbgp_i);
}

void ehci_controller_preinit(dbgp_info_t *dbgp_i)
{
   ehci_acquire(dbgp_i);
   /* ehci_remove_smi(dbgp_i); */
   ehci_reset(dbgp_i);
   ehci_setup(dbgp_i);
}

void ehci_controller_postinit(dbgp_info_t *dbgp_i)
{
   ehci_release(dbgp_i);
}

void ehci_acquire(dbgp_info_t *dbgp_i)
{
   pci_cfg_usblegsup_t legsup;

   debug(EHCI,"acquire ehci:");
   dbgp_i->pci.addr.reg =
      dbgp_i->ehci_cap->hcc.eecp + PCI_CFG_USB_LEGSUP_OFFSET;

   while(1)
   {
      legsup.raw = pci_cfg_read(dbgp_i->pci.addr);

      if(!legsup.bios_sem && legsup.os_sem)
	 break;

      legsup.os_sem = 1;
      pci_cfg_write(dbgp_i->pci.addr, legsup.raw);
      io_wait(100000);
   }
   debug(EHCI,"done.\n");
}

void ehci_release(dbgp_info_t *dbgp_i)
{
   pci_cfg_usblegsup_t legsup;

   debug(EHCI,"release ehci:");
   dbgp_i->pci.addr.reg =
      dbgp_i->ehci_cap->hcc.eecp + PCI_CFG_USB_LEGSUP_OFFSET;

   while(1)
   {
      legsup.raw = pci_cfg_read(dbgp_i->pci.addr);

      if(!legsup.os_sem && legsup.bios_sem)
	 break;

      legsup.os_sem = 0;
      pci_cfg_write(dbgp_i->pci.addr, legsup.raw);
      io_wait(100000);
   }
   debug(EHCI,"done.\n");
}

void ehci_remove_smi(dbgp_info_t *dbgp_i)
{
   pci_cfg_usblegctl_t legctl;

   debug(EHCI,"remove ehci SMI:");
   dbgp_i->pci.addr.reg =
      dbgp_i->ehci_cap->hcc.eecp + PCI_CFG_USB_LEGCTL_OFFSET;

   legctl.raw = pci_cfg_read(dbgp_i->pci.addr);

   legctl.smi_enbl = 0;
   legctl.smi_err_enbl = 0;
   legctl.smi_pc_enbl = 0;
   legctl.smi_flr_enbl = 0;
   legctl.smi_hse_enbl = 0;
   legctl.smi_aa_enbl = 0;
   legctl.smi_os_own_enbl = 0;
   legctl.smi_pci_cmd_enbl = 0;
   legctl.smi_bar_enbl = 0;

   pci_cfg_write(dbgp_i->pci.addr, legctl.raw);
   debug(EHCI,"done.\n");
}

void ehci_reset(dbgp_info_t *dbgp_i)
{
   ehci_usbcmd_reg_t usbcmd;
   ehci_usbsts_reg_t usbsts;

   debug(EHCI,"ehci stop:");
   usbcmd.raw = dbgp_i->ehci_opr->usbcmd.raw;
   usbcmd.run = 0;
   dbgp_i->ehci_opr->usbcmd.raw = usbcmd.raw;

   do
   {
      io_wait(1000);
      usbsts.raw = dbgp_i->ehci_opr->usbsts.raw;

   } while(!usbsts.hlt);
   debug(EHCI,"done.\n");

   debug(EHCI,"ehci reset:");
   usbcmd.raw = dbgp_i->ehci_opr->usbcmd.raw;
   usbcmd.rst = 0;
   dbgp_i->ehci_opr->usbcmd.raw = usbcmd.raw;

   do
   {
      io_wait(1000);
      usbcmd.raw = dbgp_i->ehci_opr->usbcmd.raw;

   } while(usbcmd.rst);
   debug(EHCI,"done.\n");
}

/* void ehci_ports_identify(dbgp_info_t *dbgp_i) */
/* { */
/*    while(1) */
/*    { */
/*       int i; */
/*       for(i=0;i<dbgp_i->ehci_cap->hcs.n_ports;i++) */
/*       { */
/* 	 ehci_portsc_reg_t portsc; */
/* 	 portsc.raw = dbgp_i->ehci_opr->portsc[i].raw; */
/* 	 debug(EHCI,"%d|%d ", portsc.conn_chg, portsc.conn); */
/*       } */
/*       debug(EHCI,"\n"); */
/*    } */
/* } */
#endif

void ehci_dbgp_full_init(dbgp_info_t *dbgp_i)
{
   debug(EHCI, "ehci dbgp full init\n");
   ehci_detect(dbgp_i, EHCI_PSC_DBC2DD);
   ehci_dbgp_init(dbgp_i);
}

void ehci_dbgp_fast_init(dbgp_info_t *dbgp_i)
{
   debug(EHCI, "ehci dbgp fast init\n");
   ehci_detect(dbgp_i, EHCI_PSC_DBC2RST);
   ehci_dbgp_init(dbgp_i);
}

void ehci_dbgp_stealth_reinit(dbgp_info_t *dbgp_i)
{
   ehci_cfgflg_reg_t cfgflg;
   ehci_usbcmd_reg_t usbcmd;
   ehci_usbsts_reg_t usbsts;
   ehci_usbint_reg_t usbint, shadow;
   ehci_portsc_reg_t portsc;

   debug(EHCI, "ehci stealth re-init\n");
   __ehci_show(dbgp_i);

   cfgflg.raw = dbgp_i->ehci_opr->cfgflg.raw;
   usbcmd.raw = dbgp_i->ehci_opr->usbcmd.raw;
   usbsts.raw = dbgp_i->ehci_opr->usbsts.raw;

   if((!usbcmd.run && usbsts.hlt) || !cfgflg.cf)
      ehci_setup(dbgp_i);

   usbint.raw = dbgp_i->ehci_opr->usbint.raw;
   shadow.raw = usbint.raw;
   if(usbint.port_change)
   {
      debug(EHCI, "disable port change interrupt\n");
      usbint.port_change = 0;
      dbgp_i->ehci_opr->usbint.raw = usbint.raw;
   }

   /* check if owned by companion */
   portsc.raw = dbgp_i->ehci_psc->raw;
   if(portsc.own)
   {
       debug(EHCI, "reset port owner\n");
      __ehci_port_own(dbgp_i);
      ehci_dbgp_full_init(dbgp_i);
   }
   else if(portsc.reset)
   {
      debug(EHCI, "port is in reset state\n");
      ehci_dbgp_full_init(dbgp_i);
   }
   else
      ehci_dbgp_fast_init(dbgp_i);

   dbgp_i->ehci_opr->usbint.raw = shadow.raw;
}

void ehci_dbgp_init(dbgp_info_t *dbgp_i)
{
   dbgp_enable(dbgp_i);
   dbgp_configure(dbgp_i);
}

void ehci_setup(dbgp_info_t *dbgp_i)
{
   ehci_usbcmd_reg_t usbcmd;
   ehci_usbsts_reg_t usbsts;
   ehci_cfgflg_reg_t cfgflg;

   debug(EHCI,"ehci setup:");

   usbcmd.raw = dbgp_i->ehci_opr->usbcmd.raw;
   usbcmd.light_rst    = 0;
   usbcmd.doorbell     = 0;
   usbcmd.p_sched_enbl = 0;
   usbcmd.a_sched_enbl = 0;
   usbcmd.run          = 1;
   dbgp_i->ehci_opr->usbcmd.raw = usbcmd.raw;

   cfgflg.raw = dbgp_i->ehci_opr->cfgflg.raw;
   cfgflg.cf = 1;
   dbgp_i->ehci_opr->cfgflg.cf = cfgflg.raw;

   do
   {
      io_wait(1000);
      usbsts.raw = dbgp_i->ehci_opr->usbsts.raw;

   } while(usbsts.hlt);
   debug(EHCI,"done.\n");
}

void __ehci_port_own(dbgp_info_t *dbgp_i)
{
   ehci_portsc_reg_t portsc;

   portsc.raw = dbgp_i->ehci_psc->raw;
   portsc.own = 0;
   dbgp_i->ehci_psc->raw = portsc.raw;

   do
   {
      io_wait(1000);
      portsc.raw = dbgp_i->ehci_psc->raw;
   
   } while(portsc.own);
}

void __ehci_port_leave_disabled(dbgp_info_t *dbgp_i)
{
   ehci_portsc_reg_t portsc;
   ehci_usbsts_reg_t usbsts;

   do
   {
      io_wait(1000);
      portsc.raw = dbgp_i->ehci_psc->raw;
      usbsts.raw = dbgp_i->ehci_opr->usbsts.raw;

   } while(!(usbsts.port_change && portsc.conn_chg && portsc.conn));

   portsc.conn_chg    = 1;
   usbsts.port_change = 1;

   dbgp_i->ehci_psc->raw = portsc.raw;
   dbgp_i->ehci_opr->usbsts.raw = usbsts.raw;
}

int __ehci_port_debounce_from_disabled(dbgp_info_t *dbgp_i)
{
   ehci_portsc_reg_t portsc;
   ehci_usbsts_reg_t usbsts;

   __ehci_port_leave_disabled(dbgp_i);

   io_wait(200000); /* 100ms at least debounce time */

   portsc.raw = dbgp_i->ehci_psc->raw;

   if(portsc.conn)
   {
      debug(EHCI,"device connected\n");

      /* high or full speed device, need reset */
      if(portsc.line_sts != 1)
	 return EHCI_PSC_DBC2RST;

      /* low-speed so transfert and wait disabled */
      debug(EHCI,"low-speed detected, need high-speed\n");
      __ehci_port_transfert_companion(dbgp_i);
   }

   /* now disabled disconnected so clean-up */
   portsc.raw = dbgp_i->ehci_psc->raw;
   usbsts.raw = dbgp_i->ehci_opr->usbsts.raw;

   portsc.conn_chg = 1;
   usbsts.port_change = 1;

   dbgp_i->ehci_psc->raw = portsc.raw;
   dbgp_i->ehci_opr->usbsts.raw = usbsts.raw;

   return EHCI_PSC_DBC2DD;
}

void __ehci_port_reset(dbgp_info_t *dbgp_i)
{
   ehci_portsc_reg_t portsc;

   portsc.raw = dbgp_i->ehci_psc->raw;

   /* can not reset if over current */
   if(portsc.over)
   {
      debug(EHCI,"portsc over-current detected !\n");

      do
      {
	 io_wait(10000);
	 portsc.raw = dbgp_i->ehci_psc->raw;

      } while(portsc.over);
   }

   portsc.reset = 1;
   dbgp_i->ehci_psc->raw = portsc.raw;

   do
   {
      io_wait(1000);
      portsc.raw = dbgp_i->ehci_psc->raw;

   } while(!portsc.reset);

   io_wait(100000); /* 50 ms at least */

   portsc.reset = 0;
   dbgp_i->ehci_psc->raw = portsc.raw;

   do
   {
      io_wait(1000);
      portsc.raw = dbgp_i->ehci_psc->raw;

   } while(portsc.reset);
}

int __ehci_port_reset_from_debounce(dbgp_info_t *dbgp_i)
{
   ehci_portsc_reg_t portsc;
   ehci_usbsts_reg_t usbsts;

   __ehci_port_reset(dbgp_i);

   portsc.raw = dbgp_i->ehci_psc->raw;

   if(portsc.conn)
   {
      /* high-speed device connected */
      if(portsc.enbl)
      {
	 __ehci_port_hs_active_from_reset(dbgp_i);
	 return EHCI_PSC_RST2HS;
      }

      /* full-speed so transfert and wait disabled */
      debug(EHCI,"full-speed detected, need high-speed\n");
      __ehci_port_transfert_companion(dbgp_i);

      /* now disabled disconnected so clean-up */
      portsc.raw = dbgp_i->ehci_psc->raw;
      usbsts.raw = dbgp_i->ehci_opr->usbsts.raw;

      portsc.conn_chg = 1;
      usbsts.port_change = 1;

      dbgp_i->ehci_psc->raw = portsc.raw;
      dbgp_i->ehci_opr->usbsts.raw = usbsts.raw;
   }

   return EHCI_PSC_RST2DD;
}

void __ehci_port_transfert_companion(dbgp_info_t *dbgp_i)
{
   ehci_portsc_reg_t portsc;
   ehci_usbsts_reg_t usbsts;

   portsc.raw = dbgp_i->ehci_psc->raw;
   portsc.own = 1;
   dbgp_i->ehci_psc->raw = portsc.raw;

   debug(EHCI,"companion disconnect:");
   do
   {
      io_wait(1000);
      portsc.raw = dbgp_i->ehci_psc->raw;
      usbsts.raw = dbgp_i->ehci_opr->usbsts.raw;

   } while(!(usbsts.port_change && portsc.conn_chg &&
	     portsc.own && !portsc.conn && !portsc.enbl));
   debug(EHCI,"done.\n");
}

void __ehci_port_hs_active_from_reset(dbgp_info_t *dbgp_i)
{
   ehci_portsc_reg_t portsc;
   ehci_usbsts_reg_t usbsts;

   portsc.raw = dbgp_i->ehci_psc->raw;
   usbsts.raw = dbgp_i->ehci_opr->usbsts.raw;

   portsc.enbl_chg = 1;
   usbsts.port_change = 1;

   dbgp_i->ehci_psc->raw = portsc.raw;
   dbgp_i->ehci_opr->usbsts.raw = usbsts.raw;
}

int __ehci_port_disabled_connected(dbgp_info_t *dbgp_i)
{
   ehci_portsc_reg_t portsc;
   ehci_usbsts_reg_t usbsts;

   portsc.raw = dbgp_i->ehci_psc->raw;
   usbsts.raw = dbgp_i->ehci_opr->usbsts.raw;

   if(!(usbsts.port_change && portsc.conn && !portsc.enbl && portsc.enbl_chg))
      return 0;

   portsc.enbl_chg = 1;
   usbsts.port_change = 1;

   if(portsc.over)
      portsc.over_chg = 1;

   dbgp_i->ehci_psc->raw = portsc.raw;
   dbgp_i->ehci_opr->usbsts.raw = usbsts.raw;
   return 1;
}

void __ehci_port_hide(dbgp_info_t *dbgp_i)
{
   ehci_portsc_reg_t portsc;

   portsc.raw = dbgp_i->ehci_psc->raw;
   portsc.enbl = 0;
   dbgp_i->ehci_psc->raw = portsc.raw;

   do
   {
      io_wait(1000);
      portsc.raw = dbgp_i->ehci_psc->raw;

   } while(portsc.enbl);

   debug(EHCI, "portsc disabled\n");
}

void dbgp_own(dbgp_info_t *dbgp_i)
{
   ehci_dbgp_ctrl_reg_t ctrl;

   ctrl.raw = dbgp_i->ehci_dbg->ctrl.raw;
   ctrl.ownd = 1;
   ctrl.used = 1;
   dbgp_i->ehci_dbg->ctrl.raw = ctrl.raw;
}

void dbgp_release(dbgp_info_t *dbgp_i)
{
   ehci_dbgp_ctrl_reg_t ctrl;

   ctrl.raw = dbgp_i->ehci_dbg->ctrl.raw;
   ctrl.enbl = 0;
   ctrl.ownd = 0;
   ctrl.used = 0;
   dbgp_i->ehci_dbg->ctrl.raw = ctrl.raw;
}

void ehci_detect(dbgp_info_t *dbgp_i, int transition)
{
   dbgp_own(dbgp_i);

   io_wait(1000);
   debug(EHCI,"waiting device ...\n");
   do
   {
      while(transition != EHCI_PSC_DBC2RST)
	 transition = __ehci_port_debounce_from_disabled(dbgp_i);

      transition = __ehci_port_reset_from_debounce(dbgp_i);

   } while(transition != EHCI_PSC_RST2HS);

   debug(EHCI,"high-speed device connected\n");
}

#ifndef EHCI_DBG
void __ehci_light_show(dbgp_info_t __unused__ *dbgp_i){}
void __ehci_show(dbgp_info_t __unused__ *dbgp_i){}
#else
void __ehci_light_show(dbgp_info_t *dbgp_i)
{
   ehci_usbcmd_reg_t usbcmd;
   ehci_usbsts_reg_t usbsts;
   ehci_cfgflg_reg_t cfgflg;
   ehci_portsc_reg_t portsc;
   ehci_dbgp_reg_t   dbgp;

   usbcmd.raw = dbgp_i->ehci_opr->usbcmd.raw;
   usbsts.raw = dbgp_i->ehci_opr->usbsts.raw;
   cfgflg.raw = dbgp_i->ehci_opr->cfgflg.raw;
   portsc.raw = dbgp_i->ehci_psc->raw;
   dbgp.ctrl.raw = dbgp_i->ehci_dbg->ctrl.raw;
   dbgp.pid.raw  = dbgp_i->ehci_dbg->pid.raw;

   debug(EHCI,
	 "D e%d o%d w%d r%d x%x p%x | "
	 "E r%d h%d c%d | "
	 "P e%d c%d s%d r%d\n"
	 ,dbgp.ctrl.enbl,dbgp.ctrl.ownd,dbgp.ctrl.wr
	 ,dbgp.ctrl.err, dbgp.ctrl.excp
	 ,dbgp.pid.raw
	 ,usbcmd.run,usbsts.hlt,cfgflg.cf
	 ,portsc.enbl, portsc.conn, portsc.suspend, portsc.reset);
}

void __ehci_show(dbgp_info_t *dbgp_i)
{
   ehci_usbcmd_reg_t usbcmd;
   ehci_usbsts_reg_t usbsts;
   ehci_cfgflg_reg_t cfgflg;
   ehci_portsc_reg_t portsc;
   ehci_dbgp_reg_t   dbgp;

   usbcmd.raw = dbgp_i->ehci_opr->usbcmd.raw;
   usbsts.raw = dbgp_i->ehci_opr->usbsts.raw;
   cfgflg.raw = dbgp_i->ehci_opr->cfgflg.raw;
   portsc.raw = dbgp_i->ehci_psc->raw;

   dbgp.ctrl.raw = dbgp_i->ehci_dbg->ctrl.raw;
   dbgp.pid.raw  = dbgp_i->ehci_dbg->pid.raw;
   dbgp.dev.raw  = dbgp_i->ehci_dbg->dev.raw;
   dbgp.data.raw = dbgp_i->ehci_dbg->data.raw;

   debug(EHCI,"###### dbgp:\n"
	 " - ctrl en %d own %d wr %d usd %d len %d\n"
	 " - err %d excp 0x%x\n"
	 " - pid tkn 0x%x snd 0x%x rcv 0x%x\n"
	 " - addr %d ep %d\n"
	 " - data 0x%X\n"
	 ,dbgp.ctrl.enbl,dbgp.ctrl.ownd,dbgp.ctrl.wr,dbgp.ctrl.used,dbgp.ctrl.dlen
	 ,dbgp.ctrl.err, dbgp.ctrl.excp
	 ,dbgp.pid.tkn,dbgp.pid.snd,dbgp.pid.rcv
	 ,dbgp.dev.addr, dbgp.dev.ep
	 ,dbgp.data.raw);

   debug(EHCI,"###### ehci ctrl:\n"
	 " - cmd rst %d run %d\n"
	 " - sts err %d pch %d hlt %d\n"
	 " - cfg %d\n",
	 usbcmd.rst, usbcmd.run,
	 usbsts.sys_err, usbsts.port_change, usbsts.hlt,
	 cfgflg.cf);

   debug(EHCI,"###### psc:\n"
	 " - en %d en_chg %d con %d con_chg %d own %d\n"
	 " - susp %d rst %d line 0x%x \n",
	 portsc.enbl, portsc.enbl_chg, portsc.conn, portsc.conn_chg, portsc.own,
	 portsc.suspend, portsc.reset, portsc.line_sts);
}
#endif

/*
** set_addr() behaves strangely !
** needs another request on old addr
** before being taken into account
** maybe this is linux dependant
** (ie. behavior noticed with linux gadget)
*/
void dbgp_configure(dbgp_info_t *dbgp_i)
{
   uint32_t dev, times;

   times = DBGP_RETRY;

__retry:
   for(dev=0 ; dev<128 ; dev++)
   {
      dbgp_i->addr = dev;
      if(__dbgp_get_descriptor(dbgp_i))
	 break;
   }

   if(dev == 128)
      goto __fail;

   if(dev != DBGP_DEV_ADDR)
   {
      if(!__dbgp_set_addr(dbgp_i))
	 goto __fail;

      dbgp_i->addr = DBGP_DEV_ADDR;
   }

   if(!__dbgp_set_debug_mode(dbgp_i))
   {
      /* retry with previous addr */
      dbgp_i->addr = dev;
      if(!__dbgp_set_debug_mode(dbgp_i))
	 goto __fail;

      dbgp_i->addr = DBGP_DEV_ADDR;
   }

   /* success */
   return;

__fail:
   if(--times)
   {
      io_wait(100000);
      goto __retry;
   }

   panic("fail to configure dbgp !");
}

void dbgp_enable(dbgp_info_t *dbgp_i)
{
   ehci_dbgp_ctrl_reg_t ctrl;

   ctrl.raw = dbgp_i->ehci_dbg->ctrl.raw;
   ctrl.ownd = 1;
   ctrl.enbl = 1;
   ctrl.used = 1;
   dbgp_i->ehci_dbg->ctrl.raw = ctrl.raw;

   __ehci_port_hide(dbgp_i);
   __ehci_light_show(dbgp_i);
}

/* void __dbgp_enable_without_ehci_cfg(dbgp_info_t *dbgp_i) */
/* { */
/*    ehci_dbgp_ctrl_reg_t ctrl; */
/*    ehci_portsc_reg_t    portsc; */
/*    ehci_usbcmd_reg_t    usbcmd; */
/*    ehci_usbsts_reg_t    usbsts; */

/* __retry: */
/*    ctrl.raw = dbgp_i->ehci_dbg->ctrl.raw; */
/*    ctrl.ownd = 1; */
/*    dbgp_i->ehci_dbg->ctrl.raw = ctrl.raw; */

/*    do */
/*    { */
/*       io_wait(1000); */
/*       portsc.raw = dbgp_i->ehci_psc->raw; */

/*    } while(!portsc.conn || portsc.line_sts == 1); */

/*    usbcmd.raw = dbgp_i->ehci_opr->usbcmd.raw; */
/*    usbcmd.run = 1; */
/*    dbgp_i->ehci_opr->usbcmd.raw = usbcmd.raw; */

/*    __ehci_port_reset(dbgp_i); */

/*    portsc.raw = dbgp_i->ehci_psc->raw; */

/*    if(!(portsc.enbl && portsc.conn)) */
/*    { */
/*       ctrl.raw = dbgp_i->ehci_dbg->ctrl.raw; */
/*       ctrl.ownd = 0; */
/*       ctrl.enbl = 0; */
/*       dbgp_i->ehci_dbg->ctrl.raw = ctrl.raw; */
/*       goto __retry; */
/*    } */

/*    ctrl.raw = dbgp_i->ehci_dbg->ctrl.raw; */
/*    ctrl.enbl = 1; */
/*    ctrl.used = 1; */
/*    dbgp_i->ehci_dbg->ctrl.raw = ctrl.raw; */

/*    do */
/*    { */
/*       io_wait(1000); */
/*       ctrl.raw = dbgp_i->ehci_dbg->ctrl.raw; */

/*    } while(!ctrl.enbl); */

/*    __ehci_port_hide(dbgp_i); */

/*    usbcmd.raw = dbgp_i->ehci_opr->usbcmd.raw; */
/*    usbcmd.run = 0; */
/*    dbgp_i->ehci_opr->usbcmd.raw = usbcmd.raw; */

/*    do */
/*    { */
/*       io_wait(1000); */
/*       usbsts.raw = dbgp_i->ehci_opr->usbsts.raw; */

/*    } while(!usbsts.hlt); */
/* } */

int __dbgp_get_descriptor(dbgp_info_t *dbgp_i)
{
   usb_dev_req_t dvr;
   buffer_t      buf;

   dvr.raw = 0ULL;

   dvr.req_type.recipient = USB_REQTYPE_RECIPIENT_DEVICE;
   dvr.req_type.type = USB_REQTYPE_TYPE_STANDARD;
   dvr.req_type.dtd = USB_REQTYPE_DTD_D2H;

   dvr.request = USB_REQ_GET_DESCRIPTOR;
   dvr.value = USB_DESC_TYPE_DEBUG<<8;
   dvr.index = 0;
   dvr.len = sizeof(usb_dbg_desc_t);

   debug(EHCI, "send \"get debug descriptor\"\n");
   if(!__dbgp_setup(dbgp_i, &dvr))
      return 0;

   buf.data.addr = (void*)&dbgp_i->desc;
   buf.sz = sizeof(usb_dbg_desc_t);

   debug(EHCI, "recv debug descriptor\n");
   if(!__dbgp_read(dbgp_i, &buf, 1) || buf.sz != sizeof(usb_dbg_desc_t))
      return 0;

   debug(EHCI, "debug desc ep: in %d out %d\n",
	 dbgp_i->desc.ep_in, dbgp_i->desc.ep_out);
   return 1;
}

int __dbgp_set_addr(dbgp_info_t *dbgp_i)
{
   usb_dev_req_t dvr;

   dvr.raw = 0ULL;

   dvr.req_type.recipient = USB_REQTYPE_RECIPIENT_DEVICE;
   dvr.req_type.type = USB_REQTYPE_TYPE_STANDARD;
   dvr.req_type.dtd = USB_REQTYPE_DTD_H2D;

   dvr.request = USB_REQ_SET_ADDRESS;
   dvr.value = DBGP_DEV_ADDR;
   dvr.index = 0;
   dvr.len = 0;

   debug(EHCI, "set addr\n");
   return __dbgp_setup(dbgp_i, &dvr);
}

int __dbgp_set_debug_mode(dbgp_info_t *dbgp_i)
{
   usb_dev_req_t dvr;

   dvr.raw = 0ULL;

   dvr.req_type.recipient = USB_REQTYPE_RECIPIENT_DEVICE;
   dvr.req_type.type = USB_REQTYPE_TYPE_STANDARD;
   dvr.req_type.dtd = USB_REQTYPE_DTD_H2D;

   dvr.request = USB_REQ_SET_FEATURE;
   dvr.value = USB_FEAT_DEBUG_MODE;
   dvr.index = 0;
   dvr.len = 0;

   debug(EHCI, "set debug mode\n");
   return __dbgp_setup(dbgp_i, &dvr);
}

int __dbgp_setup(dbgp_info_t *dbgp_i, usb_dev_req_t *dvr)
{
   ehci_dbgp_reg_t shadow;
   buffer_t        buf;

   shadow.pid.tkn = EHCI_TOKEN_PID_SETUP;
   shadow.pid.snd = EHCI_DATA_PID_0;
   shadow.dev.ep  = 0;

   buf.data.u64 = &dvr->raw;
   buf.sz = sizeof(usb_dev_req_t);

   return __dbgp_emit(dbgp_i, &shadow, &buf);
}

int __dbgp_read(dbgp_info_t *dbgp_i, buffer_t *buf, uint8_t setup)
{
   static uint32_t rd_toggle = EHCI_DATA_PID_1;
   ehci_dbgp_reg_t shadow;

   shadow.pid.tkn = EHCI_TOKEN_PID_IN;
   shadow.pid.snd = ehci_pid_swap(rd_toggle);
   shadow.dev.ep  = setup ? 0 : dbgp_i->desc.ep_in;

   return __dbgp_emit(dbgp_i, &shadow, buf);
}

int _dbgp_read(dbgp_info_t *dbgp_i, buffer_t *buf)
{
   return __dbgp_read(dbgp_i, buf, 0);
}

int __dbgp_write(dbgp_info_t *dbgp_i, buffer_t *buf)
{
   static uint32_t wr_toggle = EHCI_DATA_PID_1;
   static int      status = DBGP_EMIT_OK;
   ehci_dbgp_reg_t shadow;

   shadow.pid.tkn = EHCI_TOKEN_PID_OUT;
   shadow.dev.ep  = dbgp_i->desc.ep_out;

   if(status == DBGP_EMIT_OK)
      shadow.pid.snd = ehci_pid_swap(wr_toggle);

   status = __dbgp_emit(dbgp_i, &shadow, buf);
   return status;
}

int __dbgp_emit(dbgp_info_t *dbgp_i, ehci_dbgp_reg_t *pconfig, buffer_t *buf)
{
   ehci_dbgp_ctrl_reg_t status;
   ehci_dbgp_reg_t      config;
   size_t               retry = DBGP_RETRY;

   config.ctrl.raw = dbgp_i->ehci_dbg->ctrl.raw;

   if(!(config.ctrl.enbl && config.ctrl.ownd))
   {
      ehci_dbgp_stealth_reinit(dbgp_i);
      config.ctrl.raw = dbgp_i->ehci_dbg->ctrl.raw;
   }

   config.pid.raw  = dbgp_i->ehci_dbg->pid.raw;
   config.dev.raw  = dbgp_i->ehci_dbg->dev.raw;

   config.pid.tkn  = pconfig->pid.tkn;
   config.pid.snd  = pconfig->pid.snd;
   config.dev.ep   = pconfig->dev.ep;
   config.dev.addr = dbgp_i->addr;

   if(pconfig->pid.tkn == EHCI_TOKEN_PID_IN)
      config.ctrl.wr = 0;
   else
   {
      config.data.raw  = *buf->data.u64;
      config.ctrl.dlen = buf->sz; /* safe caller */
      config.ctrl.wr   = 1;
   }

   config.ctrl.used = 1;
   config.ctrl.go = 1;

__emit:
   if(config.ctrl.wr)
   {
      dbgp_i->ehci_dbg->data.low  = config.data.low;
      dbgp_i->ehci_dbg->data.high = config.data.high;
   }

   dbgp_i->ehci_dbg->pid.raw  = config.pid.raw;
   dbgp_i->ehci_dbg->dev.raw  = config.dev.raw;
   dbgp_i->ehci_dbg->ctrl.raw = config.ctrl.raw;

   do
   {
      io_wait(1);
      status.raw = dbgp_i->ehci_dbg->ctrl.raw;

   } while(!status.done);

   /* clear done bit */
   dbgp_i->ehci_dbg->ctrl.raw = status.raw;
   config.pid.raw = dbgp_i->ehci_dbg->pid.raw;

   if(status.err || config.pid.rcv == EHCI_STATUS_PID_STALL)
      goto __failure;

   if(pconfig->pid.tkn == EHCI_TOKEN_PID_IN)
   {
      if(config.pid.rcv == EHCI_STATUS_PID_NAK)
      {
	 buf->sz = 0;
	 return DBGP_EMIT_OK;
      }

      config.data.low  = dbgp_i->ehci_dbg->data.low;
      config.data.high = dbgp_i->ehci_dbg->data.high;

      if(status.dlen < buf->sz)
	 buf->sz = status.dlen;

      memcpy(buf->data.addr, (void*)&config.data.raw, buf->sz);
      return DBGP_EMIT_OK;
   }
   else if(config.pid.rcv != EHCI_STATUS_PID_NAK)
      return DBGP_EMIT_OK;

   /* out && nak */
   if(--retry)
   {
      io_wait(100);
      goto __emit;
   }

   debug(EHCI, "__dbgp_emit retry exhausted\n");

__failure:
   if(++dbgp_i->fails > DBGP_FAIL_TRSH)
   {
      dbgp_i->fails = 0;
      dbgp_release(dbgp_i);
      ehci_dbgp_fast_init(dbgp_i);
   }

   __ehci_light_show(dbgp_i);
   debug(EHCI, "__dbgp_emit failure\n");
   return DBGP_EMIT_FAIL;
}

static size_t __dbgp_rw(uint8_t *data, size_t n, dbgp_rw_t fn)
{
   buffer_t buf;
   size_t   times, last, cnt;

   __divrm(n, DBGP_PKT_SZ, times, last);

   buf.data.u8 = data;
   buf.sz = DBGP_PKT_SZ;
   cnt = 0;

   while(times--)
   {
      if(!fn(&info->hrd.dev.dbgp, &buf))
      {
	 debug(EHCI,"__dbgp_rw(%s): cnt %D from 0x%X bytes [0x%X]\n\n"
	       ,(fn==_dbgp_read)?"read":"write"
	       ,cnt, buf.data.linear, *buf.data.u64);
	 return cnt;
      }

      cnt += buf.sz;

      if(buf.sz < DBGP_PKT_SZ)
	 return cnt;

      buf.data.linear += buf.sz;
   }

   if(last)
   {
      buf.sz = last;
      if(!fn(&info->hrd.dev.dbgp, &buf))
      {
	 debug(EHCI,"__dbgp_rw(%s): cnt %D from 0x%X bytes [0x%X]\n\n"
	       ,(fn==_dbgp_read)?"read":"write"
	       ,cnt, buf.data.linear, *buf.data.u64);
	 return cnt;
      }

      cnt += buf.sz;
   }

   return cnt;
}

size_t dbgp_read(uint8_t *data, size_t n)
{
   return __dbgp_rw(data, n, _dbgp_read);
}

size_t dbgp_write(uint8_t *data, size_t n)
{
   buffer_t buf;

   buf.data.u8 = data;
   buf.sz = 0;

   while(buf.sz < n)
      buf.sz += __dbgp_rw(&buf.data.u8[buf.sz], n - buf.sz, __dbgp_write);

   return buf.sz;
}
