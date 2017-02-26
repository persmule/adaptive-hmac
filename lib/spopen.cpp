/* 
 * spopen.cpp
 * Implementation for spopen() and spclose.
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

#include "spopen.h"
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <set>

typedef std::set<fd_t> SP_fdset;
typedef SP_fdset::iterator SP_fdset_iter;

extern "C" {
#if 0 //in order to supress unnecessary indentation
}
#endif

typedef union sock_pair {
  struct {
    fd_t parent_end;
    fd_t child_end;
  };
  fd_t pair[2];
}sock_pair;

static SP_fdset spset;
static pthread_mutex_t spset_lock = PTHREAD_MUTEX_INITIALIZER;

SP spopen(const char* command, int cloexec)
{
  sock_pair p;
  SP sp = SP_INVALID;
  
#ifdef SOCK_CLOEXEC
# define LOCALFLAG SOCK_CLOEXEC
#else
# define LOCALFLAG 0
#endif
  {
    int r = socketpair(AF_UNIX,
		       SOCK_STREAM|LOCALFLAG,
		       0,
		       p.pair);
    if (r < 0)
      return sp;
  }
#undef LOCALFLAG
  
  sp.child = fork();
  
  if (sp.child == 0) { //child's context
#ifndef SOCK_CLOEXEC
    //If we have SOCK_CLOEXEC the descriptor is marked for close-on-exec.
    close(p.parent_end);
#endif
    {
      dup2(p.child_end, STDIN_FILENO);
      dup2(p.child_end, STDOUT_FILENO);
#ifndef SOCK_CLOEXEC
      close(sp.child_end);
#endif
    }
#ifdef SOCK_CLOEXEC
    if ((p.child_end == STDIN_FILENO)
	      ||(p.child_end == STDOUT_FILENO)) {
      /* The descriptor is already the one we will use.  But it must
	 not be marked close-on-exec.  Undo the effects.  */
      fcntl(p.child_end, F_SETFD, 0);
    }
#endif
    /* POSIX.2:  "popen() shall ensure that any streams from previous
         popen() calls that remain open in the parent process are closed
	 in the new child process." 
	 This spopen follows such design.*/
    for (SP_fdset_iter it = spset.begin();
	 it != spset.end();
	 it++) {
      fd_t fd = *it;
      if ((fd != STDIN_FILENO)
	  && (fd != STDOUT_FILENO))
	close(fd);
    }
    execl("/bin/sh", "sh", "-c", command, (const char*)NULL);
    _exit(127);
  }
  close(p.child_end);
  
  if (sp.child < 0) {
    close(p.parent_end);
    return SP_INVALID;
  }

  if (cloexec) {
#ifndef SOCK_CLOEXEC
    fcntl(p.parent_end, F_SETFD, FD_CLOEXEC);
#endif
  } else {
#ifdef SOCK_CLOEXEC
    fcntl(p.parent_end, F_SETFD, 0);
#endif
  }
  sp.sock = p.parent_end;

  //put sp.sock into spset
  pthread_mutex_lock(&spset_lock);
  spset.insert(sp.sock);
  pthread_mutex_unlock(&spset_lock);

  return sp;
}

int spfinalize(SP sp)
{
  return shutdown(sp.sock, SHUT_WR);
}

int spclose(SP sp)
{
  int found = 0;
  int wstatus = -1;
  pid_t wait_pid = -1;
  //remove sp.sock from spset
  pthread_mutex_lock(&spset_lock);
  found = spset.erase(sp.sock);
  pthread_mutex_unlock(&spset_lock);
  if (!found || close (sp.sock) < 0)
    return -1;
  /* POSIX.2 Rationale:  "Some historical implementations of popen(3)
     either block or ignore the signals SIGINT, SIGQUIT, and SIGHUP
     while waiting for the child process to terminate.  Since this
     behavior is not described in POSIX.2, such implementations are
     not conforming." */
  do {
    wait_pid = waitpid(sp.child, &wstatus, 0);
  } while (wait_pid == -1 && errno == EINTR);
  if (wait_pid == -1)
    return -1;
  return wstatus;
}

#if 0 //in order to supress unnecessary indentation
{
#endif
}
