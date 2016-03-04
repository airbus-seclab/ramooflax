/*
** Copyright (C) 2015 EADS France, stephane duverger <stephane.duverger@eads.net>
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

#include <config.h>
#include <types.h>

#include <vmx_vmcs_enc.h>
#include <vmx_insn.h>
#include <print.h>

/*
** macro to declare vmcs fields
*/
#define vmcs_t(_ft_,_fn_)   _ft_ _fn_; vmcs_field_enc_t _fn_##_enc

/*
** Functions
*/
#ifndef __INIT__
void vmx_vmcs_collect();
#endif

void vmx_vmcs_commit();

void __vmcs_force_read(raw64_t*, vmcs_field_enc_t)  __regparm__(2);
void __vmcs_force_flush(raw64_t*, vmcs_field_enc_t) __regparm__(2);

/*
** Wrappers
*/
#define vmcs_fake_it(_fld_)             (_fld_##_enc.fake = 1)
#define vmcs_encode(_fld_,_eNc_)	(_fld_##_enc.raw  = _eNc_)
#define vmcs_is_dirty(_fld_)            (_fld_##_enc.dirty != 0)
#define vmcs_set_read(_fld_)	        (_fld_##_enc.read = 1)
#define vmcs_clear(_fld_)	        (_fld_##_enc.dirty = 0)

#define vmcs_dirty(_fld_)		  \
   ({					  \
      if(!_fld_##_enc.dirty)		  \
      {					  \
	 _fld_##_enc.read  = 1;		  \
	 _fld_##_enc.dirty = 1;		  \
      }					  \
   })

#define vmcs_force_read(_fld_)					\
   __vmcs_force_read((raw64_t*)&_fld_.raw, _fld_##_enc);	\

#define vmcs_force_flush(_fld_)					\
   __vmcs_force_flush((raw64_t*)&_fld_.raw, _fld_##_enc);	\

#define vmcs_read(_fld_)		\
   ({					\
      if(!_fld_##_enc.read)		\
      {					\
	 vmcs_force_read(_fld_);	\
	 _fld_##_enc.read = 1;		\
      }					\
   })

#define vmcs_flush(_fld_)		\
   ({					\
      _fld_##_enc.read = 0;		\
      if(_fld_##_enc.dirty)		\
      {					\
	 _fld_##_enc.dirty = 0;		\
	 vmcs_force_flush(_fld_);	\
      }					\
   })

#define vmcs_flush_fixed(_fld_,_fx_)		\
   ({						\
      _fld_##_enc.read = 0;			\
      if(_fld_##_enc.dirty)			\
      {						\
	 _fld_##_enc.dirty = 0;			\
	 vmx_set_fixed(_fld_.raw, _fx_);	\
	 vmcs_force_flush(_fld_);		\
      }						\
   })

#define vmcs_cond(_will_wr_,_fld_)	\
   ({					\
      vmcs_read(_fld_);			\
      if(_will_wr_)			\
	 _fld_##_enc.dirty = 1;		\
   })

#endif
