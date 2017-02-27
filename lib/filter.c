/* 
 * filter.c
 * Implementation for higher wrapper of SP objects.
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

#include "filter.h"
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>

filter* FLT_new_sp(SP sp,
		   void* receiver,
		   const receiver_vtable* rvt)
{
  if(!(receiver
       && rvt
       && (sp.child > 0)
       && (sp.sock >= 0)))
    return NULL;
  
  filter* newflt = (filter*)calloc(1, sizeof(filter));

  if(!newflt)
    return NULL;

  newflt->sp = sp;
  newflt->receiver = receiver;
  newflt->rvt = rvt;

  newflt->rfd = dup(sp.sock);
  fcntl(newflt->rfd, F_SETFL, O_NONBLOCK);
  newflt->wfp = fdopen(dup(sp.sock), "wb");

  if (!((newflt->rfd >= 0) && newflt->wfp)) {
    kill(newflt->sp.child, SIGKILL);
    spclose(newflt->sp);
    return NULL;
  }

  return newflt;
}

int FLT_close(filter* flt)
{
  if (!(flt
	&& FLT_is_sane(flt)))
    return 0;
  if (flt->wfp) {
    fclose(flt->wfp);
    flt->wfp = NULL;
  }

  close(flt->rfd);

  if (flt->err)
    kill(flt->sp.child, SIGKILL);
  
  int ret = spclose(flt->sp);
  free(flt);
  
  return ret;
}

int FLT_feed(filter* flt,
		   const void* data,
		   size_t datalen,
		   int finalize)
{
  
  if (!(flt
	&& FLT_is_sane(flt)))
    return 0;
  int ret = 0;
  fwrite(data, sizeof(char), datalen, flt->wfp);
  if (finalize) {
    fclose(flt->wfp);
    flt->wfp = NULL;
    spfinalize(flt->sp);//make stdin of child reach EOF
  }

  ptrdiff_t osize = 0;
  
  do {
    size_t asize = 0;
    
    void* buf = flt->rvt->read_begin(flt->receiver, &asize);
    if (!(buf && asize))
      goto onerr;
    osize = read(flt->rfd, buf, asize);
    if (osize > 0) {
      //read successfully complete, commit data to receiver.
      flt->rvt->read_end(flt->receiver, osize);
      ret++;
      continue;
    } else if (errno == EAGAIN) {
      //child has nothing to output, cancel this reading.
      flt->rvt->read_end(flt->receiver, 0);
      if(finalize)
	continue;
      else
	break;
    } else {
      goto onerr;
    }
    /* 
     * osize == 0
     * EOF of flt->rfd has reached with no data read,
     * cancel this reading.
     */
    flt->rvt->read_end(flt->receiver, 0);
    
  } while (osize != 0); //EOF of rfd
  
  return ret;
 onerr:
  {
    flt->err = errno;
    return -1;
  }
}
