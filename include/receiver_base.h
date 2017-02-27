/* 
 * receiver_base.h
 * abstract interface for all receivers.
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

#ifndef _RECEIVER_BASE_H_
#define _RECEIVER_BASE_H_
#ifdef __cplusplus
extern "C" {
#endif
#if 0 //in order to supress unnecessary indentation
}
#endif

#include <stddef.h>

/*
 * return a buffer with *a_size large to which data from child
 * will be read. return NULL and *a_size == 0 with no changes on
 * *receiver when on err.
 */
#define RECEIVER_READ_BEGIN(x) void* (x)(void* receiver, size_t* a_size)
typedef RECEIVER_READ_BEGIN(receiver_read_begin);

/*
 * tell receiver that size bytes of data are read, or reading is
 * canceled if (size == 0).
 */
#define RECEIVER_READ_END(x) void (x)(void* receiver, size_t size)
typedef RECEIVER_READ_END(receiver_read_end);

typedef struct receiver_vtable {
  receiver_read_begin* read_begin;
  receiver_read_end* read_end;
}receiver_vtable;

#if 0 //in order to supress unnecessary indentation
{
#endif
#ifdef __cplusplus
}
#endif
#endif
