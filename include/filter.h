/* 
 * filter.h
 * Defines and types for higher wrapper of SP objects.
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

#ifndef _FILTER_H_
#define _FILTER_H_
#ifdef __cplusplus
extern "C" {
#endif
#if 0 //in order to supress unnecessary indentation
}
#endif

#include "spopen.h"
#include "receiver_base.h"
#include <stdio.h>

typedef struct filter {
  SP sp;
  void* receiver;
  const receiver_vtable* rvt;
  FILE* wfp;
  fd_t rfd;
  int err;
}filter;

filter* FLT_new_sp(SP sp,
		   void* receiver,
		   const receiver_vtable* rvt);

static inline filter* FLT_new(const char* command,
			      int cloexec,
			      void* receiver,
			      const receiver_vtable* rvt)
{
  return FLT_new_sp(spopen(command, cloexec), receiver, rvt);
}

static inline int FLT_is_sane(const filter* flt)
{
  return ((flt->sp.child > 0)
	  && (flt->sp.sock >= 0)
	  && (flt->receiver)
	  && (flt->rvt)
	  && (flt->wfp)
	  && (flt->rfd > 0));
}

static inline int FLT_geterr(const filter* flt)
{
  return flt->err;
}

int FLT_free(filter* flt);

/*
 * Feed data to flt, and optionally perform finalization. 
 * if child generates output, callbacks for receiver will
 * be called. return value is the number of times that 
 * actual output from child occurs, or -1 when on error.
 */
int FLT_feed(filter* flt,
		   const void* data,
		   size_t datalen,
		   int finalize);

#if 0 //in order to supress unnecessary indentation
{
#endif
#ifdef __cplusplus
}
#endif
#endif
