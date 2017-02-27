/* 
 * test-filter.c
 * Test and demo program for "Class" filter.
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

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdbool.h>

#include "filter.h"
#include "rcv_concat.h"

int main(int argc, char** argv)
{
  if (argc == 1) {
    fprintf(stderr,
	    "Usage: %s <-d> <any command>,\n"
	    "'-d' for debug.\n", argv[0]);
    return 0;
  }
  size_t cmdlen = 0;
  char* cmd = NULL;
  char ibuf[64];
  
  int firstarg = 1;

  if (0 == strcmp("-d", argv[1]))
    //enable debug output;
    firstarg = 2;

  //concatenate all argvs to a single string for spopen()
  {
    int i;
    for (i = firstarg;
	 i < argc;
	 i++){
      cmdlen += strlen(argv[i]) + sizeof(char);
    }

    cmd = (char*)malloc(cmdlen);

    {
      char* finger = cmd;
      for (i = firstarg;
	   i < argc;
	   i++){
	size_t arglen = strlen(argv[i]);
	strncpy(finger, argv[i], arglen);
	finger[arglen] = ' ';
	finger += arglen + 1;
      }
      assert(finger == cmd + cmdlen);
      assert(cmd[cmdlen - 1] == ' ');
    }
    cmd[cmdlen - 1] = '\0';
  }

  rcv_concat* rcat = rcv_concat_new(40);
  if(!rcat) {
    perror("Error: failed to construct *rcat");
    return -1;
  }

  filter* flt = FLT_new(cmd,
			true,
			(void*)rcat,
			&rcv_concat_vtbl);

  if(!flt) {
    perror("Error: failed to construct *flt");
    return -1;
  }
  free(cmd);

  do {
    {
      size_t isize = fread(ibuf, sizeof(char), sizeof(ibuf), stdin);
      int otime = FLT_feed(flt,
			   ibuf,
			   isize,
			   (isize < sizeof(ibuf)) ? true : false);

      if (otime == -1) {
	perror("Error: failed to feed data to flt");
	break;
      }

      if (firstarg == 2) {
	fprintf(stderr, "%d readings from child triggered.\n", otime);
      }

      buf_node* b = rcv_concat_cat(rcat);
      if(!b) {
	perror("Error: failed to concatenate buffered data");
	return -1;
      }

      if (b->used_size) {
	fwrite(b->buf, sizeof(char), b->used_size, stdout);
      }

      buf_node_free(b);
    }
  } while (!feof(stdin));

  int ret = FLT_close(flt);

  rcv_concat_free(rcat);
  
  return ret;
}
