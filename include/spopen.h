/* 
 * spopen.h
 * Defines and types for spopen() and spclose().
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


#ifndef _SPOPEN_H_
#define _SPOPEN_H_
#ifdef __cplusplus
extern "C" {
# if 0 //in order to supress unnecessary indentation
}
# endif
#endif


#include <unistd.h>

typedef int fd_t;

typedef struct SP {
  pid_t child;
  fd_t sock;
}SP;

extern SP spopen(const char* command, int cloexec);

extern int spfinalize(SP sp); //finalize write to sp

extern int spclose(SP sp); //returns the return value of the child process

#define SP_INVALID (SP){ -1, -1 }


#ifdef __cplusplus
# if 0 //in order to supress unnecessary indentation
{
# endif
}
#endif
#endif
