/* 
 * test-spopen.c
 * Test and demo program for spopen() and spclose().
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
#include <signal.h>
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include "spopen.h"

int main(int argc, char** argv)
{
  if (argc == 1) {
    fprintf(stderr, "Usage: %s <any command>.\n", argv[0]);
    return 0;
  }
  size_t cmdlen = 0;
  char* cmd = NULL;
  char ibuf[64], obuf[64];
  size_t isize = 0;
  ptrdiff_t osize = 0;

  //concatenate all argvs to a single string for spopen()
  {
    int i;
    for (i = 1;
	 i < argc;
	 i++){
      cmdlen += strlen(argv[i]) + sizeof(char);
    }

    cmd = (char*)malloc(cmdlen);

    {
      char* finger = cmd;
      for (i = 1;
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
  filter flt = spopen(cmd, 1);
  if ((flt.sock == -1)
      ||(flt.child == -1)) {
    fputs("Error: unable to perform spopen!\n", stderr);
    return -1;
  }

  free(cmd);
  
  //FILE* rfp = fdopen(dup(flt.sock), "rb");
  fd_t rfd = dup(flt.sock);
  fcntl(rfd, F_SETFL, O_NONBLOCK);
  FILE* wfp = fdopen(dup(flt.sock), "wb");

  if (!((rfd > 0) && wfp)) {
    fputs("Error: unable to perform fdopen! killing child process...\n",
	  stderr);
    kill(flt.child, SIGKILL);
    spclose(flt);
    return -1;
  }
    
  do {
    if (!feof(stdin)) {
      isize = fread(ibuf, sizeof(char), sizeof(ibuf), stdin);
      fwrite(ibuf, sizeof(char), isize, wfp);
      
    
      if (isize < sizeof(ibuf)) {
	assert(feof(stdin) && !ferror(stdin));
	fclose(wfp);
	wfp = NULL;
	spfinalize(flt);//make stdin of child reach EOF
      }
    }

    do {
      osize = read(rfd, obuf, sizeof(obuf));
      if (osize > 0) {
	//process data output by child, and repeat for more output.
	fwrite(obuf, sizeof(char), osize, stdout);
	continue;
      } else if (errno == EAGAIN) {
	//child has nothing to output, feed more data.
	break;
      } else {
	  perror("Error: fail to read from child process.");
	  if (wfp) {
	    fclose(wfp);
	    wfp = NULL;
	  }
	  close(rfd);
	  kill(flt.child, SIGKILL);
	  spclose(flt);
	  return -1;
      }
      //EOF of rfd has reached.
    } while (osize != 0); //EOF of rfd
  } while (osize != 0);

  //read remaining data until eof.

  assert(!ferror(stdout));

  close(rfd);

  return spclose(flt);
}
