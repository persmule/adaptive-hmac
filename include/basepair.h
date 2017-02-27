/* 
 * basepair.h
 * Defines and types for a bidirectional link list.
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

#ifndef _BASEPAIR_H_
#define _BASEPAIR_H_
#ifdef __cplusplus
extern "C" {
#if 0 // (Keep Emacsens' auto-indent happy.) 
}
#endif
#endif
  
#include <stddef.h>
#include <assert.h>

#define GCC_INLINE static inline

#define container_of(ptr, type, member) ({			\
      const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
      (type *)( (char *)__mptr - offsetof(type,member) );})
  
struct linux_list_node_st
{
  struct linux_list_node_st* next;
  struct linux_list_node_st* prev;
};

typedef struct linux_list_node_st bp_node;

GCC_INLINE bp_node* bp_getnext(const bp_node* node)
{
  return node->next;
}

GCC_INLINE bp_node* bp_getprev(const bp_node* node)
{
  return node->prev;
}

//define the "sticky" end of chains
GCC_INLINE int bp_isnsticky(const bp_node* node)
{
  return node->next==node;
}

GCC_INLINE int bp_ispsticky(const bp_node* node)
{
  return node->prev==node;
}

GCC_INLINE int bp_islonely(const bp_node* node)
{
  return (bp_isnsticky(node)&&bp_ispsticky(node));
}
  
//make the "next side" of a node sticky
GCC_INLINE void bp_mknsticky(bp_node* node)
{
  node->next=node;
}

//make the "prev side" of a node sticky
GCC_INLINE void bp_mkpsticky(bp_node* node)
{
  node->prev=node;
}

GCC_INLINE void bp_reset(bp_node* node)
{
  bp_mknsticky(node);
  bp_mkpsticky(node);
}

//cut chain and make two sticky end
GCC_INLINE bp_node* bp_cut_after(bp_node* pos)
{
  bp_node* nextnode=pos->next;
  {
    bp_mknsticky(pos);
    bp_mkpsticky(nextnode);
  }
  return nextnode;
}

GCC_INLINE bp_node* bp_cut_before(bp_node* pos)
{
  bp_node* prevnode=pos->prev;
  {
    bp_mkpsticky(pos);
    bp_mknsticky(prevnode);
  }
  return prevnode;
}

//join two sticky end to form a chain
GCC_INLINE void bp_join(bp_node* left,bp_node* right)
{
  assert(bp_isnsticky(left));
  assert(bp_ispsticky(right));
  {
    left->next=right;
    right->prev=left;
  }
}


/*
 *swap the nexts of these two nodes,
 *if these two node belong to the same circular chain list,
 *the effect is to split the chain in two, just between either of these nodes and their nexts;
 *otherwise, its effect is to merge the two circular chain lists into one.
 */
GCC_INLINE void bp_swap_next(bp_node* node1,bp_node* node2)
{
  bp_node* next1=bp_cut_after(node1);
  bp_node* next2=bp_cut_after(node2);
  bp_join(node1,next2);
  bp_join(node2,next1);
}
  
//the "prev" version of the function above
GCC_INLINE void bp_swap_prev(bp_node* node1,bp_node* node2)
{
  bp_node* prev1=bp_cut_before(node1);
  bp_node* prev2=bp_cut_before(node2);
  bp_join(prev2,node1);
  bp_join(prev1,node2);
}


//Both of them can be used to implement insertion, picking, splitting and joining operation
GCC_INLINE void bp_insert_after(bp_node* pos,bp_node* node)
{
  assert(bp_islonely(node));
  bp_swap_next(pos,node);
}

GCC_INLINE void bp_insert_before(bp_node* pos,bp_node* node)
{
  assert(bp_islonely(node));
  bp_swap_prev(pos,node);
}

GCC_INLINE bp_node* bp_pick_next(bp_node* pos)
{
  bp_node* ret=pos->next;
  bp_swap_next(pos,ret);
  return ret;
}

GCC_INLINE bp_node* bp_pick_prev(bp_node* pos)
{
  bp_node* ret=pos->prev;
  bp_swap_prev(pos,ret);
  return ret;
}

//split a part of list [first,last) to a temporary list
GCC_INLINE bp_node* bp_list_split_n(bp_node* first,bp_node* last)
{
  bp_swap_prev(first,last);
  return first;
}

//the reversed version of the above function, split (rlast,rfirst]
GCC_INLINE bp_node* bp_list_split_p(bp_node* rfirst,bp_node* rlast)
{
  bp_swap_next(rfirst,rlast);
  return rfirst;
}

//join @tmplist after @pos, the node @tmplist points to becomes the next of @pos
GCC_INLINE void bp_list_join_after(bp_node* tmplist,bp_node* pos)
{
  bp_swap_prev(pos->next,tmplist);
}

//join @tmplist before @pos, the node @tmplist points to becomes the prev of @pos
GCC_INLINE void bp_list_join_before(bp_node* tmplist,bp_node* pos)
{
  bp_swap_next(pos->prev,tmplist);
}
  
#define bp_foreach(it,head) for(it = (head)->next; it != (head); it=bp_getnext(it))

#define bp_foreach_r(it,head) for(it = (head)->prev; it != (head); it=bp_getprev(it))

#define bp_collapse(head) while((head)!=(head)->next)

#define bp_collapse_r(head) while((head)!=(head)->prev)

#ifdef __cplusplus
#if 0 // (Keep Emacsens' auto-indent happy.)
{
#endif
}
#endif
#endif
