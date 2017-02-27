/* 
 * rcv_concat.c
 * Implementation for a receiver to concatenate all its received data.
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


#include "rcv_concat.h"
#include <assert.h>
#include <string.h>

static RECEIVER_READ_BEGIN(rcv_concat_read_begin);
static RECEIVER_READ_END(rcv_concat_read_end);

const receiver_vtable rcv_concat_vtbl =
  (receiver_vtable){
  rcv_concat_read_begin, 
  rcv_concat_read_end,
};

buf_node* buf_node_new(size_t usable_size)
{
  buf_node* newnode = (buf_node*)malloc(usable_size + sizeof(buf_node));
  if (newnode == NULL)
    return NULL;
  bp_reset(&newnode->node);
  newnode->used_size = 0;
  return newnode;
}

rcv_concat* rcv_concat_new(size_t buf_usable_size)
{
  if (buf_usable_size == 0)
    return NULL;
  rcv_concat* rcat = (rcv_concat*)malloc(sizeof(rcv_concat));
  if (rcat == NULL)
    return NULL;
  rcat->buf_usable_size = buf_usable_size;
  bp_reset(&rcat->list);
  rcat->newbuf = buf_node_new(rcat->buf_usable_size);
  return rcat;
}

void rcv_concat_reset(rcv_concat* rcat)
{
  if(rcat == NULL)
    return;
  bp_node* head = &rcat->list;
  bp_collapse(head) {
    buf_node_free(container_of(bp_pick_next(head),
			       buf_node,
			       node));
  }
}

void rcv_concat_free(rcv_concat* rcat)
{
  if(rcat == NULL)
    return;
  rcv_concat_reset(rcat);
  buf_node_free(rcat->newbuf);
  free(rcat);
}

buf_node* rcv_concat_cat(rcv_concat* rcat)
{
  if(rcat == NULL)
    return NULL;
  size_t total = 0;
  bp_node* it = NULL;
  {
    bp_foreach(it, &rcat->list) {
      const buf_node* b = container_of(it,
				       buf_node,
				       node);
      total += b->used_size;
    }
  }
  
  buf_node* obuf = buf_node_new(total);
  if(obuf == NULL)
    return NULL;
  
  {
    char* finger = obuf->buf;
    bp_foreach(it, &rcat->list) {
      const buf_node* b = container_of(it,
				       buf_node,
				       node);
      memcpy(finger, b->buf, b->used_size);
      finger += b->used_size;
    }
    assert(finger == obuf->buf + total);
    obuf->used_size = total;
  }
  
  return obuf;
}

static RECEIVER_READ_BEGIN(rcv_concat_read_begin)
{
  rcv_concat* rcat = (rcv_concat*)receiver;
  *a_size = rcat->buf_usable_size;
  return (void*)rcat->newbuf->buf;
}

static RECEIVER_READ_END(rcv_concat_read_end)
{
  if (size == 0)
    return;
  rcv_concat* rcat = (rcv_concat*)receiver;
  if (!rcv_concat_is_sane(rcat))
    return;
  rcat->newbuf->used_size = size;
  bp_insert_before(&rcat->list, &rcat->newbuf->node);
  rcat->newbuf = buf_node_new(rcat->buf_usable_size);
}
