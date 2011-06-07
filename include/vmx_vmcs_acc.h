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
#ifndef __VMX_VMCS_ACCESS_H__
#define __VMX_VMCS_ACCESS_H__

#include <types.h>
#include <vmx_vmcs_enc.h>

/*
** vmcs meta accessor
*/
typedef struct vmcs_field_access
{
   vmcs_field_enc_t enc;
   uint8_t          r:1;  /* (1) field is synchro with vmcs */
   uint8_t          w:1;  /* (0) field is synchro with vmcs */

} __attribute__((packed)) vmcs_field_access_t;

/*
** macro to declare vmcs fields
*/
#define vmcs_t(_x_)							\
   struct { _x_; vmcs_field_access_t __vmcs_access; } __attribute__((packed))

/*
** Functions
*/
struct information_data;

void    vmx_vmcs_encode(struct information_data*);
void    vmx_vmcs_commit(struct information_data*);
void    vmx_vmcs_collect(struct information_data*);
void    __vmcs_read(raw64_t*, vmcs_field_access_t*);
void    __vmcs_flush(raw64_t*, vmcs_field_access_t*);

#define vmcs_dirty(_field_)		  \
   ({					  \
      if(! _field_.__vmcs_access.w)	  \
      {					  \
	 _field_.__vmcs_access.r = 1;	  \
	 _field_.__vmcs_access.w = 1;	  \
      }					  \
   })

#define vmcs_read(_field_)						\
   ({									\
      if(! _field_.__vmcs_access.r)					\
      {									\
	 __vmcs_read((raw64_t*)&_field_.raw, &_field_.__vmcs_access);	\
	 /*printf("vmread("#_field_")\n");*/				\
      }									\
   })

#define vmcs_flush(_field_)						\
   ({									\
      _field_.__vmcs_access.r = 0;					\
      if(_field_.__vmcs_access.w)					\
	 vmcs_force_flush(_field_);					\
   })

#define vmcs_encode(_field_,_encoding_)					\
   ({									\
      _field_.__vmcs_access.w = 1;					\
      _field_.__vmcs_access.enc.raw = _encoding_;			\
   })

#define vmcs_force_flush(_field_)					\
   ({									\
      /*printf("vmwrite("#_field_")\n");*/				\
      __vmcs_flush((raw64_t*)&_field_.raw, &_field_.__vmcs_access);	\
   })

#endif
