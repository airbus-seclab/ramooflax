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
#ifndef __DEV_KBD_H__
#define __DEV_KBD_H__

#include <types.h>
#include <io.h>

/*
** Keyboard controler ports
*/
#define KBD_DATA_PORT              0x60
#define KBD_CTRL_PORT              0x64

/*
** Keyboard Controler commands : out(CTRL)
*/
#define KBD_CMD_READ_CMD_BYTE      0x20
#define KBD_CMD_WRITE_CMD_BYTE     0x60

#define KBD_CMD_SELF_TEST          0xaa
#define KBD_CMD_INTERFACE_TEST     0xab

#define KBD_CMD_DISABLE_AUX        0xa7
#define KBD_CMD_ENABLE_AUX         0xa8
#define KBD_CMD_TEST_AUX           0xa9

#define KBD_CMD_DISABLE_KBD        0xad
#define KBD_CMD_ENABLE_KBD         0xae

#define KBD_CMD_READ_I_PORT        0xc0

#define KBD_CMD_READ_O_PORT        0xd0
#define KBD_CMD_WRITE_O_PORT       0xd1

#define KBD_CMD_WRITE_AUX_O_PORT   0xd3

#define KBD_CMD_READ_TEST_INPUT    0xe0

#define KBD_CMD_SYSTEM_RESET       0xfe

/*
** Keyboard commands : out(DATA)
*/
#define KBD_CMD_LED_OFF            0x00
#define KBD_CMD_LED                0xed

#define KBD_CMD_ECHO               0xee
#define KBD_CMD_RESET              0xff

#define KBD_CMD_ENABLE_SCAN        0xf4
#define KBD_CMD_DISABLE_SCAN       0xf5
#define KBD_CMD_SCANCODE_SET       0xf0

#define KBD_CMD_PS2_READ_ID        0xf2

/*
** Commands results
*/
#define KBD_ANSWER_INTERFACE_TEST_OK          0x00
#define KBD_ANSWER_INTERFACE_TEST_CLOCK_LOW   0x01
#define KBD_ANSWER_INTERFACE_TEST_CLOCK_HIGH  0x02
#define KBD_ANSWER_INTERFACE_TEST_DATA_LOW    0x03
#define KBD_ANSWER_INTERFACE_TEST_DATA_HIGH   0x04

#define KBD_ANSWER_ACK                        0xfa

#define KBD_ANSWER_SELF_TEST_OK               0x55
#define KBD_ANSWER_SELF_TEST_KO               0xfc

#define KBD_ANSWER_ECHO                       0xee
#define KBD_ANSWER_ERROR                      0xff

#define KBD_ANSWER_BAT                        0xaa
#define KBD_ANSWER_BAT_FAILED                 0xfc

#define KBD_ANSWER_TOOMUCHKEY                 0x00

#define KBD_ANSWER_STD_PS2_ID                 0x00

/*
** LED parameters
*/
#define KBD_WRITE_LED_SCROLL_ON    (1<<0)
#define KBD_WRITE_LED_NUM_ON       (1<<1)
#define KBD_WRITE_LED_CAPS_ON      (1<<2)

/*
** Scancode set parameters
*/
#define KBD_WRITE_SCANCODE_SET_1      0x1
#define KBD_WRITE_SCANCODE_SET_2      0x2
#define KBD_WRITE_SCANCODE_SET_3      0x3
#define KBD_WRITE_SCANCODE_GET        0x0
#define KBD_READ_SCANCODE_1           'C'
#define KBD_READ_SCANCODE_2           'A'
#define KBD_READ_SCANCODE_3           '?'


/*  -- The KBD Rate --
**
** The 2nd byte is:
**   - bits 0-4 rate
**   - bits 5 & 6 pause before repeat
**   - bit 7 always 0
*/
#define KBD_WRITE_RATE               0xf3
/* 30 keys/sec, 26 ... */
#define KBD_WRITE_RATE_30            0x00
#define KBD_WRITE_RATE_26            0x01
#define KBD_WRITE_RATE_24            0x02
#define KBD_WRITE_RATE_20            0x04
#define KBD_WRITE_RATE_15            0x08
#define KBD_WRITE_RATE_10            0x0a
#define KBD_WRITE_RATE_9             0x0d
#define KBD_WRITE_RATE_7             0x10
#define KBD_WRITE_RATE_5             0x14
#define KBD_WRITE_RATE_2             0x1f
/* 250ms, 500ms, ... */
#define KBD_WRITE_PAUSE_250          0x00
#define KBD_WRITE_PAUSE_500          0x01
#define KBD_WRITE_PAUSE_750          0x02
#define KBD_WRITE_PAUSE_1000         0x04


/*
** Status register bits
*/
#define KBD_STATUS_OBF    (1<<0)         /* output buffer (0:empty) can not read  on 0x60 (1:full) can     read  on 0x60 */
#define KBD_STATUS_IBF    (1<<1)         /* input  buffer (0:empty) can     write to 0x60 (1:full) can not write to 0x60 */
#define KBD_STATUS_SYS    (1<<2)         /* (0) system in power-on reset, (1) system already initialized */
#define KBD_STATUS_A2     (1<<3)         /* (1) if out(CTRL) (0) if out(DATA) */
#define KBD_STATUS_INH    (1<<4)         /* (0) keyboard inhibited (1) keyboard is not inhibited */
#define KBD_STATUS_MOBF   (1<<5)         /* Like OBF but for ps/2 mouse */
#define KBD_STATUS_TO     (1<<6)         /* (0) no timeout error, (1) send or receive timeout */
#define KBD_STATUS_PERR   (1<<7)         /* (1) parity error */

typedef struct keyboard_status_register_fields
{
   uint8_t  obf:1;
   uint8_t  ibf:1;
   uint8_t  sys:1;
   uint8_t  cmd:1;
   uint8_t  inh:1;
   uint8_t  mobf:1;
   uint8_t  to:1;
   uint8_t  perr:1;

} __attribute__((packed)) kbd_status_reg_fields_t;

typedef union keyboard_status_register
{
   uint8_t raw;
   kbd_status_reg_fields_t;

} __attribute__((packed)) kbd_status_reg_t;

/*
** Command byte
*/
#define KBD_CMD_BYTE_INT   (1<<0)       /* (1) irq1  is generated if OBF=1 (0) polling OBF needed */
#define KBD_CMD_BYTE_INT2  (1<<1)       /* (1) irq12 is generated if mouse present */
#define KBD_CMD_BYTE_SYS   (1<<2)       /* (1) set SYS bit in status register (0) clear SYS bit in status register */
#define KBD_CMD_BYTE_RES1  (1<<3)       /* unused on PS/2, on AT it is inhibit switch  */
#define KBD_CMD_BYTE_EN    (1<<4)       /* (0) keyboard enabled (1) keyboard disabled */
#define KBD_CMD_BYTE_EN2   (1<<5)       /* (0) mouse  enabled (1) mouse disabled */
#define KBD_CMD_BYTE_XLAT  (1<<6)       /* (0) disable translation of scancode */
#define KBD_CMD_BYTE_RES2  (1<<7)       /* unused on AT and PS/2 */

typedef struct keyboard_cmd_byte_fields
{
   uint8_t  intr:1;
   uint8_t  intr2:1;
   uint8_t  sys:1;
   uint8_t  r1:1;
   uint8_t  _en:1;
   uint8_t  _en2:1;
   uint8_t  xlat:1;
   uint8_t  r2:1;

} __attribute__((packed)) kbd_cmd_byte_fields_t;

typedef union keyboard_cmd_byte
{
   uint8_t raw;
   kbd_cmd_byte_fields_t;

} __attribute__((packed)) kbd_cmd_byte_t;

/*
** Input Port
*/
#define KBD_INPUT_PORT_EXT_RAM      (1<<4)  /* (1) enable 2nd 256K of motherboard RAM */
#define KBD_INPUT_PORT_JUMPER       (1<<5)  /* (1) manufacturing jumper not installer */
#define KBD_INPUT_PORT_DISPLAY      (1<<6)  /* (1) MDA (0) CGA */
#define KBD_INPUT_PORT_NO_INHIBIT   (1<<7)  /* (1) kbd NOT inhibited */

typedef struct keyboard_input_port_fields
{
   uint8_t  undef:4;
   uint8_t  ext_ram:1;
   uint8_t  jmp:1;
   uint8_t  display:1;
   uint8_t  n_inhib:1;

} __attribute__((packed)) kbd_input_port_fields_t;

typedef union keyboard_input_port
{
   uint8_t raw;
   kbd_input_port_fields_t;

} __attribute__((packed)) kbd_input_port_t;

/*
** Output Port
*/
#define KBD_OUTPUT_PORT_SYS_RST      (1<<0)  /* System reset line */
#define KBD_OUTPUT_PORT_A20          (1<<1)  /* Gate A20 */
#define KBD_OUTPUT_PORT_OBF          (1<<4)  /* Output Buffer Full */
#define KBD_OUTPUT_PORT_IBE          (1<<5)  /* Input Buffer Empty */
#define KBD_OUTPUT_PORT_CLOCK        (1<<6)  /* Kbd Clock (output) */
#define KBD_OUTPUT_PORT_DATA         (1<<7)  /* Kbd Data (output) */

typedef struct keyboard_output_port_fields
{
   uint8_t  sys_rst:1;
   uint8_t  a20:1;
   uint8_t  undef:2;
   uint8_t  obf:1;
   uint8_t  ibe:1;
   uint8_t  clock:1;
   uint8_t  data:1;

} __attribute__((packed)) kbd_output_port_fields_t;

typedef union keyboard_output_port
{
   uint8_t raw;
   kbd_output_port_fields_t;

} __attribute__((packed)) kbd_output_port_t;

/*
** Intel 8042 Keyboard Controler
*/
typedef struct keyboard_controler
{
   /* Controler Part */
   uint8_t             last_cmd;
   uint8_t             answer;
   uint8_t             need_param;

   kbd_status_reg_t    status_reg;
   kbd_cmd_byte_t      cmd_byte;
   kbd_input_port_t    input_port;
   kbd_output_port_t   output_port;

   /* Keyboard Part */
   uint8_t             last_k_cmd;
   uint8_t             k_answer;
   uint8_t             k_need_param;

   /* Mouse Part */
   uint8_t             mouse_data;

} kbd_t;

/*
** Functions
*/
void  dev_kbd_init(kbd_t*);
int   dev_kbd(kbd_t*, io_insn_t*);

/*
** Internals
*/
int   __dev_kbd_ctrl(kbd_t*, io_insn_t*);
int   __dev_kbd_data(kbd_t*, io_insn_t*);


#endif

