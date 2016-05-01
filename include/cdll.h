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
#ifndef __CDLL_H__
#define __CDLL_H__

/*
** Circular Double Linked List Functions
*/
#define __cdll_add(__lst__, __new__)			\
   ({							\
      if((__lst__))					\
      {							\
	 (__new__)->next = (__lst__);			\
	 (__new__)->prev = (__lst__)->prev;		\
	 (__lst__)->prev->next = (__new__);		\
	 (__lst__)->prev = (__new__);			\
      }							\
      else						\
	 (__new__)->next = (__new__)->prev = (__new__);	\
   })

#define cdll_fill(__lst__, __new__)			 \
   ({							 \
      __cdll_add(__lst__, __new__);			 \
      if(!(__lst__))					 \
	 (__lst__) = (__new__);				 \
   })

#define cdll_push(__lst__, __new__)			\
   ({							\
      __cdll_add(__lst__, __new__);			\
      (__lst__) = (__new__);				\
   })

#define cdll_pop(__lst__)			     \
   ({						     \
      typeof((__lst__)) __tmp__ = 0;		     \
						     \
      if((__lst__))				     \
      {						     \
	 if((__lst__) == (__lst__)->next)	     \
	 {					     \
	    __tmp__ = (__lst__);		     \
	    (__lst__) = 0;			     \
	 }					     \
	 else					     \
	 {					     \
	    (__lst__)->next->prev = (__lst__)->prev; \
	    (__lst__)->prev->next = (__lst__)->next; \
	    					     \
	    __tmp__ = (__lst__);		     \
	    (__lst__) = (__lst__)->next;	     \
	 }					     \
      }						     \
      __tmp__;					     \
   })

#define cdll_del(__lst__,__cel__)					\
   ({									\
      if((__lst__))							\
      {									\
	 if((__lst__) == (__lst__)->next && (__lst__) == (__cel__))	\
	    (__lst__) = 0;						\
	 else								\
	 {								\
	    (__cel__)->next->prev = (__cel__)->prev;			\
	    (__cel__)->prev->next = (__cel__)->next;			\
									\
	    if((__cel__) == (__lst__))					\
	       (__lst__) = (__lst__)->next;				\
	 }								\
      }									\
   })

#define cdll_insert_before(__lst__, __cel__, __ref__ )	\
   ({							\
      if((__lst__))					\
      {							\
	 (__cel__)->prev = (__ref__)->prev;		\
	 (__ref__)->prev->next = (__cel__);		\
	 (__cel__)->next = (__ref__);			\
	 (__ref__)->prev = (__cel__);			\
							\
	 if((__ref__) == (__lst__))			\
	    (__lst__) = (__cel__);			\
      }							\
   })

#define cdll_search(__lst__,__cel__)					\
   ({									\
      typeof((__cel__)) __tmp__;					\
      int    __rEt__=0;							\
      if((__lst__))							\
      {									\
	 __tmp__ = (__lst__);						\
	 while( __tmp__ != (__cel__) && __tmp__->next != (__lst__))	\
	    __tmp__ = __tmp__->next;					\
	 if(__tmp__ == (__cel__))					\
	    __rEt__=1;							\
      }									\
      __rEt__;								\
   })

#endif
