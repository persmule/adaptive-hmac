/* 
 * rcv_concat.h
 * Defines and types for a receiver to concatenate all its received data.
 * 
 * Copyright (c) 2017 Persmule. All Rights Reserved.
 *
 * The library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * The library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA 
 *
 */

#ifndef _RCV_CONCAT_H_
#define _RCV_CONCAT_H_
#ifdef __cplusplus
extern "C" {
#if 0 // (Keep Emacsens' auto-indent happy.) 
}
#endif
#endif

#include <stddef.h>
#include <stdlib.h>
#include "basepair.h"
#include "receiver_base.h"

typedef struct buf_node {
  bp_node node;
  size_t used_size; //size really used by data
  char buf[];
}buf_node;

typedef struct rcv_concat {
  size_t buf_usable_size; //usable size of each buf_node
  bp_node list;
  buf_node* newbuf;
}rcv_concat;

static inline int rcv_concat_is_sane(const rcv_concat* rcat)
{
  return ((rcat->buf_usable_size)
	  && (rcat->list.next)
	  && (rcat->list.prev)
	  && (rcat->newbuf));
}

extern buf_node* buf_node_new(size_t usable_size);
static inline void buf_node_free(buf_node* bn)
{
  free((void*)bn);
}

extern rcv_concat* rcv_concat_new(size_t buf_usable_size);
extern void rcv_concat_reset(rcv_concat* rcat);
extern void rcv_concat_free(rcv_concat* rcat);

extern buf_node* rcv_concat_cat(rcv_concat* rcat);

extern const receiver_vtable rcv_concat_vtbl;

#ifdef __cplusplus
#if 0 // (Keep Emacsens' auto-indent happy.)
{
#endif
}
#endif
#endif
