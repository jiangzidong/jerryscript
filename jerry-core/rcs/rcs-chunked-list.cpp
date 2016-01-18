/* Copyright 2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "rcs-chunked-list.h"



/**
 * Set previous node for the specified node
 */
void
rcs_chunked_list_set_prev (rcs_chunked_list_node_t *node_p, /**< node to set previous for */
                              rcs_chunked_list_node_t *prev_node_p) /**< the previous node */
{
  JERRY_ASSERT (node_p != NULL);

  MEM_CP_SET_POINTER (node_p->prev_cp, prev_node_p);
}

/**
 * Set next node for the specified node
 */
void
rcs_chunked_list_set_next (rcs_chunked_list_node_t *node_p, /**< node to set next for */
                              rcs_chunked_list_node_t *next_node_p) /**< the next node */
{
  JERRY_ASSERT (node_p != NULL);

  MEM_CP_SET_POINTER (node_p->next_cp, next_node_p);
}

/**
 * Get size of the node
 *
 * @return size of node, including header and data space
 */
size_t
rcs_chunked_list_get_node_size (void)
{
  size_t size =
  mem_heap_recommend_allocation_size (sizeof (rcs_chunked_list_node_t) + 1u);

  JERRY_ASSERT (size != 0 && size >= sizeof (rcs_chunked_list_node_t));

  return size;
}

/**
 * Assert that the list state is correct
 */
void
assert_chunked_list_is_correct (rcs_chunked_list_t *cl_p)
{
#ifndef JERRY_DISABLE_HEAVY_DEBUG
  for (rcs_chunked_list_node_t *node_iter_p = rcs_chunked_list_get_first (cl_p);
       node_iter_p != NULL;
       node_iter_p = rcs_chunked_list_get_next (node_iter_p))
  {
    rcs_chunked_list_node_t *prev_node_p = rcs_chunked_list_get_prev (node_iter_p);
    rcs_chunked_list_node_t *next_node_p = rcs_chunked_list_get_next (node_iter_p);

    JERRY_ASSERT ((node_iter_p == cl_p->head_p
                   && prev_node_p == NULL)
                  || (node_iter_p != cl_p->head_p
                      && prev_node_p != NULL
                      && get_next (prev_node_p) == node_iter_p));
    JERRY_ASSERT ((node_iter_p == cl_p->tail_p
                   && next_node_p == NULL)
                  || (node_iter_p != cl_p->tail_p
                      && next_node_p != NULL
                      && get_prev (next_node_p) == node_iter_p));
  }
#endif /* !JERRY_DISABLE_HEAVY_DEBUG */
}

/**
 * Assert that state of specified node is correct
 */
void
assert_chunked_list_node_is_correct (rcs_chunked_list_t *cl_p,
                                      rcs_chunked_list_node_t *node_p) /**< the node */
{
#ifndef JERRY_DISABLE_HEAVY_DEBUG
  JERRY_ASSERT (node_p != NULL);

  assert_chunked_list_is_correct (cl_p);

  bool is_in_list = false;
  for (rcs_chunked_list_node_t *node_iter_p = rcs_chunked_list_get_first (cl_p);
       node_iter_p != NULL;
       node_iter_p = rcs_chunked_list_get_next (node_iter_p))
  {
    if (node_iter_p == node_p)
    {
      is_in_list = true;

      break;
    }
  }

  JERRY_ASSERT (is_in_list);
#else /* !JERRY_DISABLE_HEAVY_DEBUG */
  (void) node_p;
#endif /* JERRY_DISABLE_HEAVY_DEBUG */
}


/**
 * Initializarion
 */
void
rcs_chunked_list_init (rcs_chunked_list_t *cl_p)
{
  cl_p->heap_p = NULL;
  cl_p->tail_p = NULL;
}

/**
 * Free
 */
void
rcs_chunked_list_free (rcs_chunked_list_t *cl_p)
{
  JERRY_ASSERT (cl_p->head_p == NULL);
  JERRY_ASSERT (cl_p->tail_p == NULL);
}

void
rcs_chunked_list_cleanup (rcs_chunked_list_t *cl_p)
{
  while (cl_p->head_p)
  {
    rcs_chunked_list_remove (chunked_list, cl_p->head_p);
  }
}



/**
 * Get first node of the list
 *
 * @return pointer to the first node
 */
rcs_chunked_list_node_t*
rcs_chunked_list_get_first (rcs_chunked_list_t *cl_p)
{
  return cl_p->heap_p; 
}


/**
 * Get last node of the list
 *
 * @return pointer to the last node
 */
rcs_chunked_list_node_t*
rcs_chunked_list_get_last (rcs_chunked_list_t *cl_p)
{
  return cl_p->tail_p; 
}

/**
 * Get node, previous to specified
 *
 * @return pointer to previous node
 */
rcs_chunked_list_node_t*
rcs_chunked_list_get_prev (rcs_chunked_list_node_t *node_p)
{
  JERRY_ASSERT (node_p != NULL);
  return MEM_CP_GET_POINTER (rcs_chunked_list_node_t, node_p->prev_cp);
}


/**
 * Get node, next to specified
 *
 * @return pointer to next node
 */
rcs_chunked_list_node_t*
rcs_chunked_list_get_next (rcs_chunked_list_node_t *node_p)
{
  JERRY_ASSERT (node_p != NULL);
  return MEM_CP_GET_POINTER (rcs_chunked_list_node_t, node_p->next_cp);
}

/**
 * Append new node to end of the list
 *
 * @return pointer to the new node
 */
rcs_chunked_list_node_t*
rcs_chunked_list_append_new (rcs_chunked_list_t *cl_p)
{
  assert_chunked_list_is_correct (cl_p);
  rcs_chunked_list_node_t *node_p = (rcs_chunked_list_node_t *) 
        mem_heap_alloc_chunked_block (MEM_HEAP_ALLOC_LONG_TERM);
  rcs_chunked_list_set_prev (node_p, cl_p->tail_p);
  rcs_chunked_list_set_next (node_p, NULL);

  if (cl_p->head_p == NULL)
  {
    JERRY_ASSERT (cl_p->tail == NULL);
    cl_p->head_p = node_p;
    cl_p->tail_p = node_p;
  }
  else
  {
    JERRY_ASSERT (cl_p->tail !== NULL);
    rcs_chunked_list_set_next (cl_p->tail_p, node_p);
    cl_p->tail_p = node_p;
  }
  assert_chunked_list_node_is_correct (cl_p, node_p);

  return node_p;
}

/**
 * Insert new node after the specified node
 *
 * @return pointer to the new node
 */
rcs_chunked_list_node_t*
rcs_chunked_list_insert_new (rcs_chunked_list_t *cl_p,
                            rcs_chunked_list_node_t *after_p)
{
  assert_chunked_list_is_correct (cl_p);
  rcs_chunked_list_node_t *node_p = (rcs_chunked_list_node_t *) 
        mem_heap_alloc_chunked_block (MEM_HEAP_ALLOC_LONG_TERM); 
  JERRY_ASSERT (cl_p->head_p == NULL);
  JERRY_ASSERT (cl_p->tail_p == NULL);
  assert_chunked_list_node_is_correct (cl_p, after_p);

  rcs_chunked_list_set_next (after_p, node_p);
  if (cl_p->tail_p == after_p)
  {
    cl_p->tail_p = node_p;
  }
  rcs_chunked_list_set_prev (node_p, after_p);
  rcs_chunked_list_set_next (node_p, NULL);

  assert_chunked_list_node_is_correct (cl_p, node_p);
  return node_p;
}

/**
 * Remove specified node
 */
void
rcs_chunked_list_remove (rcs_chunked_list_t *cl_p,
                         rcs_chunked_list_node_t *node_p)
{
  JERRY_ASSERT (cl_p->head_p == NULL);
  JERRY_ASSERT (cl_p->tail_p == NULL);

  assert_chunked_list_node_is_correct (node_p);
  rcs_chunked_list_node_t *prev_node_p, *next_node_p;
  prev_node_p = rcs_chunked_list_get_prev (node_p);
  next_node_p = rcs_chunked_list_get_next (node_p);

  if (prev_node_p == NULL)
  {
    JERRY_ASSERT (cl_p->head_p == node_p);
    cl_p->head_p = next_node_p; 
  }
  else
  {
    rcs_chunked_list_set_next (prev_node_p, next_node_p);
  }

  if (next_node_p == NULL)
  {
    JERRY_ASSERT (cl_p->tail_p == node_p);
    cl_p->tail_p = prev_node_p;
  }
  else
  {
    rcs_chunked_list_set_prev (next_node_p, prev_node_p);
  }

  mem_heap_free_block (node_p);

  assert_chunked_list_is_correct (cl_p);
}

/**
 * Find node containing space, pointed by specified pointer
 *
 * @return pointer to the node that contains the pointed area
 */
rcs_chunked_list_node_t*
rcs_chunked_list_get_node_from_pointer (rcs_chunked_list_t *cl_p,
                                         void* ptr) /**< the pointer value */
{
  rcs_chunked_list_node_t *node_p = 
  (rcs_chunked_list_node_t*) mem_heap_get_chunked_block_start (ptr);

  assert_chunked_list_node_is_correct (cl_p, node_p);

  return node_p;
}

/**
 * Get the node's data space
 *
 * @return pointer to beginning of the node's data space
 */
uint8_t *
rcs_chunked_list_get_node_data_space (rcs_chunked_list_t *cl_p,
                                      rcs_chunked_list_node_t *node_p) /**< the node */
{
  assert_chunked_list_node_is_correct (cl_p, node_p);

  return (uint8_t *) (node_p + 1);
} 

/**
 * Get size of a node's data space
 *
 * @return size
 */
size_t
rcs_chunked_list_get_node_data_space_size (void)
{
  return rcs_chunked_list_get_node_size () - sizeof (rcs_chunked_list_node_t);
}

