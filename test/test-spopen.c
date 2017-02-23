/* 
 * test-spopen.c
 * Test and example for spopen() and spclose().
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
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <sys/socket.h>
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
  size_t isize = 0, osize = 0;

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
    cmd[cmdlen] = '\0';
  }
  filter flt = spopen(cmd, 1);
  if ((flt.sock == -1)
      ||(flt.child == -1)) {
    fputs("Error: unable to perform spopen!\n", stderr);
    return -1;
  }

  free(cmd);
  
  FILE* rfp = fdopen(dup(flt.sock), "rb");
  FILE* wfp = fdopen(dup(flt.sock), "wb");

  if (!(rfp && wfp)) {
    fputs("Error: unable to perform fdopen! killing child process...\n",
	  stderr);
    kill(flt.child, SIGKILL);
    spclose(flt);
    return -1;
  }
    
  do {
    isize = fread(ibuf, sizeof(char), sizeof(ibuf), stdin);
    fwrite(ibuf, sizeof(char), isize, wfp);

    if (isize < sizeof(ibuf)) {
      assert(feof(stdin) && !ferror(stdin));
      fclose(wfp);
      shutdown(flt.sock, SHUT_WR);
      break;
    }
    osize = fread(obuf, sizeof(char), sizeof(obuf), rfp);
    fwrite(obuf, sizeof(char), osize, stdout);
  } while (osize == sizeof(obuf));

  //read remaining data until eof.
  do {
    osize = fread(obuf, sizeof(char), sizeof(obuf), rfp);
    fwrite(obuf, sizeof(char), osize, stdout);
  } while (osize == sizeof(obuf));

  assert(feof(rfp) && !ferror(stdout));

  fclose(rfp);

  return spclose(flt);
}
