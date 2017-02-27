/* 
 * test-spopen.c
 * test program for basepair.h
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
#include "basepair.h"

typedef struct test_entry_st
{
  int data;
  bp_node node;
}t_entry;

int main(void)
{
  bp_node head0,head1;
  bp_reset(&head0);
  bp_reset(&head1);

  {
    int i;
    for(i=0;i<10;i++)
      {
	t_entry* ep=malloc(sizeof(t_entry));
	{
	  bp_reset(&(ep->node));
	  ep->data=i;
	}
	bp_insert_after(&head0,&(ep->node));
      }
  }

  bp_node* first=NULL;
  bp_node* last=NULL;

  {
    bp_node* it;
    bp_foreach(it,&head0)
      {
	t_entry* ep=container_of(it,t_entry,node);
	printf("Entry is located at %p, node at %p, data field is %d,\n"
	       "while \"it\" is %p.\n",
	       ep,&(ep->node),ep->data,it);
	{
	  if(ep->data==5)
	    first=it;
	  if(ep->data==2)
	    last=it;
	}
      }
  }

  bp_list_join_after(bp_list_split_n(first,last),&head1);

  {
    bp_node* it;
    puts("Entries linked to head0 is...\n");
    bp_foreach(it,&head0)
      {
	t_entry* ep=container_of(it,t_entry,node);
	printf("(%d,(%p,%p)\n",
	       ep->data,ep->node.next,ep->node.prev);
      }
  }
  {
    bp_node* it;
    puts("Entries linked to head1 is...\n");
    bp_foreach(it,&head1)
      {
	t_entry* ep=container_of(it,t_entry,node);
	printf("(%d,(%p,%p)\n",
	       ep->data,ep->node.next,ep->node.prev);
      }
  }

  {
    bp_collapse(&head0)
      {
	free(container_of(bp_pick_next(&head0),t_entry,node));
      }
  }

  {
    bp_collapse(&head1)
      {
	free(container_of(bp_pick_next(&head1),t_entry,node));
      }
  }

  return 0;
}
