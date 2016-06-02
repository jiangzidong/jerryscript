/* Copyright 2014-2016 Samsung Electronics Co., Ltd.
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

/**
 * Garbage collector implementation
 */

#include "ecma-alloc.h"
#include "ecma-globals.h"
#include "ecma-gc.h"
#include "ecma-helpers.h"
#include "ecma-lcache.h"
#include "ecma-property-hashmap.h"
#include "jrt.h"
#include "jrt-libc-includes.h"
#include "jrt-bit-fields.h"
#include "re-compiler.h"
#include "vm-defines.h"
#include "vm-stack.h"

#define JERRY_INTERNAL
#include "jerry-internal.h"

#include "ecma-lex-env.h"
#include "ecma-globals.h"
#include "ecma-property-hashmap.h"
#define ECMA_PROPERTY_HASHMAP_GET_TOTAL_SIZE(max_property_count) \
  (sizeof (ecma_property_hashmap_t) + (max_property_count * sizeof (mem_cpointer_t)) + (max_property_count >> 3))



typedef struct
{
  uint16_t size; /* Size of string in bytes */
  uint16_t length; /* Number of characters in the string */
} ecma_string_heap_header_t;

const char* string_type[]={"lit","ascki","utf8", "num", "uint", "magic", "m_ex"};
const char* simple_value[]={"empty", "undefined", "null", "false", "true", "array_hole", "reg_ref"};
const char* object_type[]={"gene", "func", "bultin-func", "arr", "str", "ext-func", "bound-func", "argu"};
const char* env_type[]={"","","","","","decl","obj","this+obj"};

/* TODO: Extract GC to a separate component */

/** \addtogroup ecma ECMA
 * @{
 *
 * \addtogroup ecmagc Garbage collector
 * @{
 */

/**
 * An object's GC color
 *
 * Tri-color marking:
 *   WHITE_GRAY, unvisited -> WHITE // not referenced by a live object or the reference not found yet
 *   WHITE_GRAY, visited   -> GRAY  // referenced by some live object
 *   BLACK                 -> BLACK // all referenced objects are gray or black
 */
typedef enum
{
  ECMA_GC_COLOR_WHITE_GRAY, /**< white or gray */
  ECMA_GC_COLOR_BLACK, /**< black */
  ECMA_GC_COLOR__COUNT /**< number of colors */
} ecma_gc_color_t;

/**
 * List of marked (visited during current GC session) and umarked objects
 */
static ecma_object_t *ecma_gc_objects_lists[ECMA_GC_COLOR__COUNT];

/**
 * Current state of an object's visited flag that indicates whether the object is in visited state:
 *  visited_field | visited_flip_flag | real_value
 *         false  |            false  |     false
 *         false  |             true  |      true
 *          true  |            false  |      true
 *          true  |             true  |     false
 */
static bool ecma_gc_visited_flip_flag = false;

/**
 * Number of currently allocated objects
 */
static size_t ecma_gc_objects_number = 0;

/**
 * Number of newly allocated objects since last GC session
 */
static size_t ecma_gc_new_objects_since_last_gc = 0;

static void ecma_gc_mark (ecma_object_t *object_p);
static void ecma_gc_sweep (ecma_object_t *object_p);

/**
 * Get next object in list of objects with same generation.
 */
static inline ecma_object_t *
ecma_gc_get_object_next (ecma_object_t *object_p) /**< object */
{
  JERRY_ASSERT (object_p != NULL);

  return ECMA_GET_POINTER (ecma_object_t, object_p->gc_next_cp);
} /* ecma_gc_get_object_next */

/**
 * Set next object in list of objects with same generation.
 */
static inline void
ecma_gc_set_object_next (ecma_object_t *object_p, /**< object */
                         ecma_object_t *next_object_p) /**< next object */
{
  JERRY_ASSERT (object_p != NULL);

  ECMA_SET_POINTER (object_p->gc_next_cp, next_object_p);
} /* ecma_gc_set_object_next */

/**
 * Get visited flag of the object.
 */
static inline bool
ecma_gc_is_object_visited (ecma_object_t *object_p) /**< object */
{
  JERRY_ASSERT (object_p != NULL);

  bool flag_value = (object_p->type_flags_refs & ECMA_OBJECT_FLAG_GC_VISITED) != 0;

  return flag_value != ecma_gc_visited_flip_flag;
} /* ecma_gc_is_object_visited */

/**
 * Set visited flag of the object.
 */
static inline void
ecma_gc_set_object_visited (ecma_object_t *object_p, /**< object */
                            bool is_visited) /**< flag value */
{
  JERRY_ASSERT (object_p != NULL);

  if (is_visited != ecma_gc_visited_flip_flag)
  {
    object_p->type_flags_refs = (uint16_t) (object_p->type_flags_refs | ECMA_OBJECT_FLAG_GC_VISITED);
  }
  else
  {
    object_p->type_flags_refs = (uint16_t) (object_p->type_flags_refs & ~ECMA_OBJECT_FLAG_GC_VISITED);
  }
} /* ecma_gc_set_object_visited */

/**
 * Initialize GC information for the object
 */
inline void
ecma_init_gc_info (ecma_object_t *object_p) /**< object */
{
  ecma_gc_objects_number++;
  ecma_gc_new_objects_since_last_gc++;

  JERRY_ASSERT (ecma_gc_new_objects_since_last_gc <= ecma_gc_objects_number);

  JERRY_ASSERT (object_p->type_flags_refs < ECMA_OBJECT_REF_ONE);
  object_p->type_flags_refs = (uint16_t) (object_p->type_flags_refs | ECMA_OBJECT_REF_ONE);

  ecma_gc_set_object_next (object_p, ecma_gc_objects_lists[ECMA_GC_COLOR_WHITE_GRAY]);
  ecma_gc_objects_lists[ECMA_GC_COLOR_WHITE_GRAY] = object_p;

  /* Should be set to false at the beginning of garbage collection */
  ecma_gc_set_object_visited (object_p, false);
} /* ecma_init_gc_info */

/**
 * Increase reference counter of an object
 */
void
ecma_ref_object (ecma_object_t *object_p) /**< object */
{
  if (object_p->type_flags_refs < ECMA_OBJECT_MAX_REF)
  {
    object_p->type_flags_refs = (uint16_t) (object_p->type_flags_refs + ECMA_OBJECT_REF_ONE);
  }
  else
  {
    jerry_fatal (ERR_REF_COUNT_LIMIT);
  }
} /* ecma_ref_object */

/**
 * Decrease reference counter of an object
 */
void
ecma_deref_object (ecma_object_t *object_p) /**< object */
{
  JERRY_ASSERT (object_p->type_flags_refs >= ECMA_OBJECT_REF_ONE);
  object_p->type_flags_refs = (uint16_t) (object_p->type_flags_refs - ECMA_OBJECT_REF_ONE);
} /* ecma_deref_object */

/**
 * Initialize garbage collector
 */
void
ecma_gc_init (void)
{
  ecma_gc_objects_lists[ECMA_GC_COLOR_WHITE_GRAY] = NULL;
  ecma_gc_objects_lists[ECMA_GC_COLOR_BLACK] = NULL;
  ecma_gc_visited_flip_flag = false;
  ecma_gc_objects_number = 0;
  ecma_gc_new_objects_since_last_gc = 0;
} /* ecma_gc_init */

/**
 * Mark referenced object from property
 */
static void
ecma_gc_mark_property (ecma_property_t *property_p) /**< property */
{
  switch (ECMA_PROPERTY_GET_TYPE (property_p))
  {
    case ECMA_PROPERTY_TYPE_NAMEDDATA:
    {
      ecma_value_t value = ecma_get_named_data_property_value (property_p);

      if (ecma_is_value_object (value))
      {
        ecma_object_t *value_obj_p = ecma_get_object_from_value (value);

        ecma_gc_set_object_visited (value_obj_p, true);
      }
      break;
    }
    case ECMA_PROPERTY_TYPE_NAMEDACCESSOR:
    {
      ecma_object_t *getter_obj_p = ecma_get_named_accessor_property_getter (property_p);
      ecma_object_t *setter_obj_p = ecma_get_named_accessor_property_setter (property_p);

      if (getter_obj_p != NULL)
      {
        ecma_gc_set_object_visited (getter_obj_p, true);
      }

      if (setter_obj_p != NULL)
      {
        ecma_gc_set_object_visited (setter_obj_p, true);
      }
      break;
    }
    case ECMA_PROPERTY_TYPE_INTERNAL:
    {
      uint32_t property_value = ECMA_PROPERTY_VALUE_PTR (property_p)->value;

      switch (ECMA_PROPERTY_GET_INTERNAL_PROPERTY_TYPE (property_p))
      {
        case ECMA_INTERNAL_PROPERTY_PRIMITIVE_STRING_VALUE: /* compressed pointer to a ecma_string_t */
        case ECMA_INTERNAL_PROPERTY_PRIMITIVE_NUMBER_VALUE: /* compressed pointer to a ecma_number_t */
        case ECMA_INTERNAL_PROPERTY_PRIMITIVE_BOOLEAN_VALUE: /* a simple boolean value */
        case ECMA_INTERNAL_PROPERTY_CLASS: /* an enum */
        case ECMA_INTERNAL_PROPERTY_CODE_BYTECODE: /* compressed pointer to a bytecode array */
        case ECMA_INTERNAL_PROPERTY_NATIVE_CODE: /* an external pointer */
        case ECMA_INTERNAL_PROPERTY_NATIVE_HANDLE: /* an external pointer */
        case ECMA_INTERNAL_PROPERTY_FREE_CALLBACK: /* an object's native free callback */
        case ECMA_INTERNAL_PROPERTY_BUILT_IN_ID: /* an integer */
        case ECMA_INTERNAL_PROPERTY_BUILT_IN_ROUTINE_DESC: /* an integer */
        case ECMA_INTERNAL_PROPERTY_NON_INSTANTIATED_BUILT_IN_MASK_0_31: /* an integer (bit-mask) */
        case ECMA_INTERNAL_PROPERTY_NON_INSTANTIATED_BUILT_IN_MASK_32_63: /* an integer (bit-mask) */
        case ECMA_INTERNAL_PROPERTY_REGEXP_BYTECODE:
        {
          break;
        }

        case ECMA_INTERNAL_PROPERTY_BOUND_FUNCTION_BOUND_THIS: /* an ecma value */
        {
          if (ecma_is_value_object (property_value))
          {
            ecma_object_t *obj_p = ecma_get_object_from_value (property_value);

            ecma_gc_set_object_visited (obj_p, true);
          }

          break;
        }

        case ECMA_INTERNAL_PROPERTY_BOUND_FUNCTION_BOUND_ARGS: /* a collection of ecma values */
        {
          ecma_collection_header_t *bound_arg_list_p = ECMA_GET_INTERNAL_VALUE_POINTER (ecma_collection_header_t,
                                                                                        property_value);

          ecma_collection_iterator_t bound_args_iterator;
          ecma_collection_iterator_init (&bound_args_iterator, bound_arg_list_p);

          for (ecma_length_t i = 0; i < bound_arg_list_p->unit_number; i++)
          {
            bool is_moved = ecma_collection_iterator_next (&bound_args_iterator);
            JERRY_ASSERT (is_moved);

            if (ecma_is_value_object (*bound_args_iterator.current_value_p))
            {
              ecma_object_t *obj_p = ecma_get_object_from_value (*bound_args_iterator.current_value_p);

              ecma_gc_set_object_visited (obj_p, true);
            }
          }

          break;
        }

        case ECMA_INTERNAL_PROPERTY_BOUND_FUNCTION_TARGET_FUNCTION: /* an object */
        case ECMA_INTERNAL_PROPERTY_SCOPE: /* a lexical environment */
        case ECMA_INTERNAL_PROPERTY_PARAMETERS_MAP: /* an object */
        {
          ecma_object_t *obj_p = ECMA_GET_INTERNAL_VALUE_POINTER (ecma_object_t, property_value);

          ecma_gc_set_object_visited (obj_p, true);

          break;
        }
        case ECMA_INTERNAL_PROPERTY__COUNT: /* not a real internal property type,
                                             * but number of the real internal property types */
        {
          JERRY_UNREACHABLE ();
          break;
        }
      }
      break;
    }
    default:
    {
      JERRY_UNREACHABLE ();
      break;
    }
  }
} /* ecma_gc_mark_property */



/*************************************
 *
 * Zidong hack
 *
 **************************************/



static uint32_t ecma_string_to_char_array(const ecma_string_t *string_p, char *buf, uint32_t buf_size)
{
  uint32_t heap_size = 0;
  if (ecma_string_get_size (string_p) > buf_size) {
    const char *msg="..long string";
    buf = strncpy (buf, msg, buf_size);
  }
  else {
    uint32_t t = ecma_string_to_utf8_string(string_p, (lit_utf8_byte_t *)buf, buf_size);
    buf[t]='\0';
  }
  switch (ECMA_STRING_GET_CONTAINER (string_p))
  {
    case ECMA_STRING_CONTAINER_HEAP_UTF8_STRING:
    {
      ecma_string_heap_header_t *const data_p = ECMA_GET_NON_NULL_POINTER (ecma_string_heap_header_t,
                                                                           string_p->u.utf8_collection_cp);

      heap_size = (data_p->size + (uint32_t)sizeof (ecma_string_heap_header_t) + MEM_ALIGNMENT - 1) / MEM_ALIGNMENT * MEM_ALIGNMENT;

      break;
    }
    case ECMA_STRING_CONTAINER_HEAP_ASCII_STRING:
    {

      heap_size = (string_p->u.ascii_string.size + MEM_ALIGNMENT - 1) / MEM_ALIGNMENT * MEM_ALIGNMENT;

      break;
    }
    case ECMA_STRING_CONTAINER_HEAP_NUMBER:
    {

      heap_size = CONFIG_MEM_POOL_CHUNK_SIZE;

      break;
    }
    case ECMA_STRING_CONTAINER_LIT_TABLE:
    case ECMA_STRING_CONTAINER_UINT32_IN_DESC:
    case ECMA_STRING_CONTAINER_MAGIC_STRING:
    case ECMA_STRING_CONTAINER_MAGIC_STRING_EX:
    {
      /* only the string descriptor itself should be freed */
      break;
    }
  }
  return heap_size +  CONFIG_MEM_POOL_CHUNK_SIZE;

}


static void show_property_name(ecma_string_t *name_p, uint32_t depth, ecma_property_t *property_p) {
  for(uint32_t i = 0; i<depth; i++) {
    printf("\t");
  }
  char prop_name[20];
  uint32_t refs;
  if(ECMA_PROPERTY_GET_TYPE (property_p)==ECMA_PROPERTY_TYPE_INTERNAL){
    printf("<k>:internal_%d\n", ECMA_PROPERTY_GET_INTERNAL_PROPERTY_TYPE (property_p));
  } else {
    refs = name_p->refs_and_container >> 3;
    uint32_t size = ecma_string_to_char_array(name_p, prop_name, 20);
    const char* type = string_type[ECMA_STRING_GET_CONTAINER (name_p)];
    printf("<k>:STR %s [0x%x]\ttype:%s\trefs:%d\tsize:%d\n", prop_name, name_p, type, refs, size);
  }
}

static void show_value_info(ecma_value_t value, uint32_t depth) {
  for(uint32_t i = 0; i<depth; i++) {
    printf("\t");
  }
  switch(value & ECMA_VALUE_TYPE_MASK) {
    case ECMA_TYPE_OBJECT:
    {
      printf("<v>:OBJ [0x%x]\n", ecma_get_object_from_value(value));
      break;
    }
    case ECMA_TYPE_STRING:
    {
      ecma_string_t *str_p = ecma_get_string_from_value(value);
      char str_buf[20];
      uint32_t refs;
      refs = str_p->refs_and_container >> 3;
      uint32_t size = ecma_string_to_char_array(str_p, str_buf, 20);
      const char* type = string_type[ECMA_STRING_GET_CONTAINER (str_p)];
      printf("<v>:STR %s [0x%x]\ttype:%s\trefs:%d\tsize:%d\n", str_buf, str_p, type, refs, size);
      break;
    }
    case ECMA_TYPE_NUMBER:
    {
      ecma_number_t *num_p = ecma_get_number_from_value(value);
      ecma_number_t num = *num_p;
      if (num>=0) {
        printf("<v>:NUM %zu.%04zu\tsize:%d\n", (uint32_t)num, (uint32_t)((num - (uint32_t)num)*10000)%10000 ,CONFIG_MEM_POOL_CHUNK_SIZE);
      } else {
        num = -num;
        printf("<v>:NUM -%zu.%04zu\tsize:%d\n", (uint32_t)num, (uint32_t)((num - (uint32_t)num)*10000)%10000, CONFIG_MEM_POOL_CHUNK_SIZE);
      }
      break;
    }
    default:
    {
      printf("<v>:SIM %s\n", simple_value[value>>ECMA_VALUE_SHIFT]);
    }
  }
}
static void show_property_info(ecma_property_pair_t *prop_pair_p, uint32_t index)
{
  ecma_property_t *property_p = prop_pair_p->header.types + index;
  uint32_t property_value = ECMA_PROPERTY_VALUE_PTR (property_p)->value;
  ecma_string_t *name_p = ECMA_GET_POINTER (ecma_string_t, prop_pair_p->names_cp[index]);
  //name
  show_property_name(name_p, 3, property_p);
  //value
  switch (ECMA_PROPERTY_GET_TYPE (property_p)) {
    case ECMA_PROPERTY_TYPE_NAMEDACCESSOR:
    {
      ecma_object_t *getter_obj_p = ecma_get_named_accessor_property_getter (property_p);
      ecma_object_t *setter_obj_p = ecma_get_named_accessor_property_setter (property_p);
      printf("\t\t\t<v>:OBJ getter:[0x%x]\tsetter[0x%x]\n", getter_obj_p, setter_obj_p);
      break;
    }
    case ECMA_PROPERTY_TYPE_NAMEDDATA:
    {
      show_value_info(property_value, 3);
      break;
    }
    case ECMA_PROPERTY_TYPE_INTERNAL:
    {
      switch (ECMA_PROPERTY_GET_INTERNAL_PROPERTY_TYPE (property_p)) {
        case ECMA_INTERNAL_PROPERTY_PRIMITIVE_STRING_VALUE:
        {
          ecma_string_t *str_p = ECMA_GET_INTERNAL_VALUE_POINTER (ecma_string_t, property_value);
          char str_buf[20];
          uint32_t refs;
          refs = str_p->refs_and_container >> 3;
          uint32_t size = ecma_string_to_char_array(str_p, str_buf, 20);
          const char* type = string_type[ECMA_STRING_GET_CONTAINER (str_p)];
          printf("\t\t\t<v>:STR %s [0x%x]\ttype:%s\trefs:%d\tsize:%d\n", str_buf, str_p, type, refs, size);
          break;
        }
        case ECMA_INTERNAL_PROPERTY_PRIMITIVE_NUMBER_VALUE:
        {
          ecma_number_t *num_p = ECMA_GET_INTERNAL_VALUE_POINTER (ecma_number_t, property_value);
          ecma_number_t num = *num_p;
          if (num>=0) {
            printf("\t\t\t<v>:NUM %zu.%04zu\tsize:%d\n", (uint32_t)num, (uint32_t)((num - (uint32_t)num)*10000)%10000 ,CONFIG_MEM_POOL_CHUNK_SIZE);
          } else {
            num = -num;
            printf("\t\t\t<v>:NUM -%zu.%04zu\tsize:%d\n", (uint32_t)num, (uint32_t)((num - (uint32_t)num)*10000)%10000, CONFIG_MEM_POOL_CHUNK_SIZE);
          }
          break;
        }
        case ECMA_INTERNAL_PROPERTY_BOUND_FUNCTION_BOUND_THIS:
        {
          printf("\t\t\t<v>:OBJ [0x%x]\n", ecma_get_object_from_value(property_value));
          break;
        }
        case ECMA_INTERNAL_PROPERTY_BOUND_FUNCTION_TARGET_FUNCTION: // an object
        case ECMA_INTERNAL_PROPERTY_SCOPE: // a lexical environment
        case ECMA_INTERNAL_PROPERTY_PARAMETERS_MAP: // an object
        {
          printf("\t\t\t<v>:OBJ [0x%x]\n", ECMA_GET_INTERNAL_VALUE_POINTER (ecma_object_t, property_value));
          break;
        }
        case ECMA_INTERNAL_PROPERTY_NATIVE_CODE: // an external pointer
        case ECMA_INTERNAL_PROPERTY_NATIVE_HANDLE: // an external pointer
        case ECMA_INTERNAL_PROPERTY_FREE_CALLBACK: // an external pointer
        {
          #ifndef ECMA_VALUE_CAN_STORE_UINTPTR_VALUE_DIRECTLY
            printf("\t\t\t<v>:EXT_P\tsize:%d\n", CONFIG_MEM_POOL_CHUNK_SIZE);
          #else// ECMA_VALUE_CAN_STORE_UINTPTR_VALUE_DIRECTLY
            printf("\t\t\t<v>:EXT_P\n");
          #endif
          break;

        }
        case ECMA_INTERNAL_PROPERTY_CODE_BYTECODE:
        case ECMA_INTERNAL_PROPERTY_REGEXP_BYTECODE:
        {
          ecma_compiled_code_t *bytecode_p = ECMA_GET_INTERNAL_VALUE_POINTER (ecma_compiled_code_t, property_value);
          if (bytecode_p != NULL)
          {
             printf("\t\t\t<v>:BC [0x%x]\trefs:%d\tsize:%d\n", bytecode_p, bytecode_p->refs, ((size_t) bytecode_p->size) << MEM_ALIGNMENT_LOG);
          }
          break;

        }
        case ECMA_INTERNAL_PROPERTY_BOUND_FUNCTION_BOUND_ARGS:
        {
          ecma_collection_header_t *bound_arg_list_p = ECMA_GET_INTERNAL_VALUE_POINTER (ecma_collection_header_t,
                                                                                        property_value);
          const size_t values_in_chunk = JERRY_SIZE_OF_STRUCT_MEMBER (ecma_collection_chunk_t, data) / sizeof (ecma_value_t);
          printf("\t\t\t<v>:COLL [0x%x]\tsize:%d\tlength:%d\n",
            bound_arg_list_p,
            ((bound_arg_list_p->unit_number + values_in_chunk - 1)/values_in_chunk + 1)*CONFIG_MEM_POOL_CHUNK_SIZE,
            bound_arg_list_p->unit_number);
          ecma_collection_iterator_t bound_args_iterator;
          ecma_collection_iterator_init (&bound_args_iterator, bound_arg_list_p);

          for (ecma_length_t i = 0; i < bound_arg_list_p->unit_number; i++)
          {
            bool is_moved = ecma_collection_iterator_next (&bound_args_iterator);
            JERRY_ASSERT (is_moved);
            show_value_info(*bound_args_iterator.current_value_p, 4);
          }
          break;
        }

        default:
        {

          printf("\t\t\t<v>:%d\n", property_value);
        }

      }
      break;
    }
    default:
    {
      break;
    }
  }
}



static void scan_one_object (ecma_object_t *object_p)
{
  bool traverse_properties = true;
  if (ecma_is_lexical_environment (object_p))
  {
    ecma_object_t *lex_env_p = ecma_get_lex_env_outer_reference (object_p);
    printf("\tENV_%s [0x%x]\trefs=%d\n", env_type[(object_p->type_flags_refs & ECMA_OBJECT_TYPE_MASK)], object_p,  (object_p->type_flags_refs)>>6);
    printf("\t\touter [0x%x]\n", lex_env_p);

    if (ecma_get_lex_env_type (object_p) != ECMA_LEXICAL_ENVIRONMENT_DECLARATIVE)
    {
      ecma_object_t *binding_object_p = ecma_get_lex_env_binding_object (object_p);
      printf("\t\tbind [0x%x]\n", binding_object_p);
      traverse_properties = false;
    }
  }
  else
  {
    ecma_object_t *proto_p = ecma_get_object_prototype (object_p);
    printf("\tOBJ_%s [0x%x]\trefs=%d\n", object_type[(object_p->type_flags_refs & ECMA_OBJECT_TYPE_MASK)], object_p, (object_p->type_flags_refs)>>6);
    printf("\t\tprototype [0x%x]\n", proto_p);
  }

  if (traverse_properties)
  {
    ecma_property_header_t *prop_iter_p = ecma_get_property_list (object_p);

    if (prop_iter_p != NULL
        && ECMA_PROPERTY_GET_TYPE (prop_iter_p->types + 0) == ECMA_PROPERTY_TYPE_HASHMAP)
    {
      uint32_t hashmap_total_size = (uint32_t)ECMA_PROPERTY_HASHMAP_GET_TOTAL_SIZE(((ecma_property_hashmap_t *)prop_iter_p)->max_property_count);
      printf("\t\thashmap\theap_size=%d\n", (hashmap_total_size + MEM_ALIGNMENT - 1) / MEM_ALIGNMENT * MEM_ALIGNMENT);

      prop_iter_p = ECMA_GET_POINTER (ecma_property_header_t,
                                      prop_iter_p->next_property_cp);
    }

    while (prop_iter_p != NULL)
    {

      JERRY_ASSERT (ECMA_PROPERTY_IS_PROPERTY_PAIR (prop_iter_p));
      printf("\t\tprop_pair\theap_size=%d\n", sizeof(ecma_property_pair_t));
      if (prop_iter_p->types[0].type_and_flags != ECMA_PROPERTY_TYPE_DELETED)
      {
        show_property_info((ecma_property_pair_t *) prop_iter_p, 0);
      }

      if (prop_iter_p->types[1].type_and_flags != ECMA_PROPERTY_TYPE_DELETED)
      {
        show_property_info((ecma_property_pair_t *) prop_iter_p, 1);
      }

      prop_iter_p = ECMA_GET_POINTER (ecma_property_header_t,
                                      prop_iter_p->next_property_cp);
    }
  }
}

void scan_all_objects (void)
{
  printf("=== SCAN START ===\n");
  for (ecma_object_t *obj_iter_p = ecma_gc_objects_lists[ECMA_GC_COLOR_WHITE_GRAY];
     obj_iter_p != NULL;
     obj_iter_p = ecma_gc_get_object_next (obj_iter_p))
  {
    scan_one_object(obj_iter_p);
  }
  printf("=== SCAN DONE ===\n");
}

/*************************************
 *
 * Zidong hack done
 *
 **************************************/

/**
 * Mark objects as visited starting from specified object as root
 */
void
ecma_gc_mark (ecma_object_t *object_p) /**< object to mark from */
{

  JERRY_ASSERT (object_p != NULL);
  JERRY_ASSERT (ecma_gc_is_object_visited (object_p));

  bool traverse_properties = true;

  if (ecma_is_lexical_environment (object_p))
  {
    ecma_object_t *lex_env_p = ecma_get_lex_env_outer_reference (object_p);
    if (lex_env_p != NULL)
    {
      ecma_gc_set_object_visited (lex_env_p, true);
    }

    if (ecma_get_lex_env_type (object_p) != ECMA_LEXICAL_ENVIRONMENT_DECLARATIVE)
    {
      ecma_object_t *binding_object_p = ecma_get_lex_env_binding_object (object_p);
      ecma_gc_set_object_visited (binding_object_p, true);

      traverse_properties = false;
    }
  }
  else
  {
    ecma_object_t *proto_p = ecma_get_object_prototype (object_p);

    if (proto_p != NULL)
    {
      ecma_gc_set_object_visited (proto_p, true);
    }
  }

  if (traverse_properties)
  {
    ecma_property_header_t *prop_iter_p = ecma_get_property_list (object_p);

    if (prop_iter_p != NULL
        && ECMA_PROPERTY_GET_TYPE (prop_iter_p->types + 0) == ECMA_PROPERTY_TYPE_HASHMAP)
    {

      prop_iter_p = ECMA_GET_POINTER (ecma_property_header_t,
                                      prop_iter_p->next_property_cp);
    }

    while (prop_iter_p != NULL)
    {

      JERRY_ASSERT (ECMA_PROPERTY_IS_PROPERTY_PAIR (prop_iter_p));

      if (prop_iter_p->types[0].type_and_flags != ECMA_PROPERTY_TYPE_DELETED)
      {
        ecma_gc_mark_property (prop_iter_p->types + 0);
      }

      if (prop_iter_p->types[1].type_and_flags != ECMA_PROPERTY_TYPE_DELETED)
      {
        ecma_gc_mark_property (prop_iter_p->types + 1);
      }

      prop_iter_p = ECMA_GET_POINTER (ecma_property_header_t,
                                      prop_iter_p->next_property_cp);
    }
  }
} /* ecma_gc_mark */

/**
 * Free specified object
 */
void
ecma_gc_sweep (ecma_object_t *object_p) /**< object to free */
{
  JERRY_ASSERT (object_p != NULL
                && !ecma_gc_is_object_visited (object_p)
                && object_p->type_flags_refs < ECMA_OBJECT_REF_ONE);

  if (!ecma_is_lexical_environment (object_p))
  {
    /* if the object provides free callback, invoke it with handle stored in the object */

    ecma_external_pointer_t freecb_p;
    ecma_external_pointer_t native_p;

    bool is_retrieved = ecma_get_external_pointer_value (object_p,
                                                         ECMA_INTERNAL_PROPERTY_FREE_CALLBACK,
                                                         &freecb_p);
    if (is_retrieved)
    {
      is_retrieved = ecma_get_external_pointer_value (object_p,
                                                      ECMA_INTERNAL_PROPERTY_NATIVE_HANDLE,
                                                      &native_p);
      JERRY_ASSERT (is_retrieved);

      jerry_dispatch_object_free_callback (freecb_p, native_p);
    }
  }

  if (!ecma_is_lexical_environment (object_p)
      || ecma_get_lex_env_type (object_p) == ECMA_LEXICAL_ENVIRONMENT_DECLARATIVE)
  {
    ecma_property_header_t *prop_iter_p = ecma_get_property_list (object_p);

    if (prop_iter_p != NULL
        && ECMA_PROPERTY_GET_TYPE (prop_iter_p->types + 0) == ECMA_PROPERTY_TYPE_HASHMAP)
    {
      ecma_property_hashmap_free (object_p);
      prop_iter_p = ecma_get_property_list (object_p);
    }

    while (prop_iter_p != NULL)
    {
      JERRY_ASSERT (ECMA_PROPERTY_IS_PROPERTY_PAIR (prop_iter_p));

      /* Both cannot be deleted. */
      JERRY_ASSERT (prop_iter_p->types[0].type_and_flags != ECMA_PROPERTY_TYPE_DELETED
                    || prop_iter_p->types[1].type_and_flags != ECMA_PROPERTY_TYPE_DELETED);

      ecma_property_pair_t *prop_pair_p = (ecma_property_pair_t *) prop_iter_p;

      for (int i = 0; i < ECMA_PROPERTY_PAIR_ITEM_COUNT; i++)
      {
        if (prop_iter_p->types[i].type_and_flags != ECMA_PROPERTY_TYPE_DELETED)
        {
          ecma_string_t *name_p = ECMA_GET_POINTER (ecma_string_t, prop_pair_p->names_cp[i]);

          ecma_free_property (object_p, name_p, prop_iter_p->types + i);

          if (name_p != NULL)
          {
            ecma_deref_ecma_string (name_p);
          }
        }
      }

      /* Both must be deleted. */
      JERRY_ASSERT (prop_iter_p->types[0].type_and_flags == ECMA_PROPERTY_TYPE_DELETED
                    && prop_iter_p->types[1].type_and_flags == ECMA_PROPERTY_TYPE_DELETED);

      prop_iter_p = ECMA_GET_POINTER (ecma_property_header_t,
                                      prop_iter_p->next_property_cp);

      ecma_dealloc_property_pair (prop_pair_p);
    }
  }

  JERRY_ASSERT (ecma_gc_objects_number > 0);
  ecma_gc_objects_number--;

  ecma_dealloc_object (object_p);
} /* ecma_gc_sweep */

/**
 * Run garbage collecting
 */
void
ecma_gc_run (void)
{
  scan_all_objects();
  ecma_gc_new_objects_since_last_gc = 0;

  JERRY_ASSERT (ecma_gc_objects_lists[ECMA_GC_COLOR_BLACK] == NULL);

  /* if some object is referenced from stack or globals (i.e. it is root), mark it */
  for (ecma_object_t *obj_iter_p = ecma_gc_objects_lists[ECMA_GC_COLOR_WHITE_GRAY];
       obj_iter_p != NULL;
       obj_iter_p = ecma_gc_get_object_next (obj_iter_p))
  {
    JERRY_ASSERT (!ecma_gc_is_object_visited (obj_iter_p));

    if (obj_iter_p->type_flags_refs >= ECMA_OBJECT_REF_ONE)
    {
      ecma_gc_set_object_visited (obj_iter_p, true);
    }
  }

  bool marked_anything_during_current_iteration = false;

  do
  {
    marked_anything_during_current_iteration = false;

    for (ecma_object_t *obj_iter_p = ecma_gc_objects_lists[ECMA_GC_COLOR_WHITE_GRAY], *obj_prev_p = NULL, *obj_next_p;
         obj_iter_p != NULL;
         obj_iter_p = obj_next_p)
    {
      obj_next_p = ecma_gc_get_object_next (obj_iter_p);

      if (ecma_gc_is_object_visited (obj_iter_p))
      {
        /* Moving the object to list of marked objects */
        ecma_gc_set_object_next (obj_iter_p, ecma_gc_objects_lists[ECMA_GC_COLOR_BLACK]);
        ecma_gc_objects_lists[ECMA_GC_COLOR_BLACK] = obj_iter_p;

        if (likely (obj_prev_p != NULL))
        {
          JERRY_ASSERT (ecma_gc_get_object_next (obj_prev_p) == obj_iter_p);

          ecma_gc_set_object_next (obj_prev_p, obj_next_p);
        }
        else
        {
          ecma_gc_objects_lists[ECMA_GC_COLOR_WHITE_GRAY] = obj_next_p;
        }

        ecma_gc_mark (obj_iter_p);
        marked_anything_during_current_iteration = true;
      }
      else
      {
        obj_prev_p = obj_iter_p;
      }
    }
  }
  while (marked_anything_during_current_iteration);

  /* Sweeping objects that are currently unmarked */
  for (ecma_object_t *obj_iter_p = ecma_gc_objects_lists[ECMA_GC_COLOR_WHITE_GRAY], *obj_next_p;
       obj_iter_p != NULL;
       obj_iter_p = obj_next_p)
  {
    obj_next_p = ecma_gc_get_object_next (obj_iter_p);

    JERRY_ASSERT (!ecma_gc_is_object_visited (obj_iter_p));

    ecma_gc_sweep (obj_iter_p);
  }

  /* Unmarking all objects */
  ecma_gc_objects_lists[ECMA_GC_COLOR_WHITE_GRAY] = ecma_gc_objects_lists[ECMA_GC_COLOR_BLACK];
  ecma_gc_objects_lists[ECMA_GC_COLOR_BLACK] = NULL;

  ecma_gc_visited_flip_flag = !ecma_gc_visited_flip_flag;

#ifndef CONFIG_ECMA_COMPACT_PROFILE_DISABLE_REGEXP_BUILTIN
  /* Free RegExp bytecodes stored in cache */
  re_cache_gc_run ();
#endif /* !CONFIG_ECMA_COMPACT_PROFILE_DISABLE_REGEXP_BUILTIN */
} /* ecma_gc_run */

/**
 * Try to free some memory (depending on severity).
 */
void
ecma_try_to_give_back_some_memory (mem_try_give_memory_back_severity_t severity) /**< severity of
                                                                                  *   the request */
{
  if (severity == MEM_TRY_GIVE_MEMORY_BACK_SEVERITY_LOW)
  {
    /*
     * If there is enough newly allocated objects since last GC, probably it is worthwhile to start GC now.
     * Otherwise, probability to free sufficient space is considered to be low.
     */
    if (ecma_gc_new_objects_since_last_gc * CONFIG_ECMA_GC_NEW_OBJECTS_SHARE_TO_START_GC > ecma_gc_objects_number)
    {
      ecma_gc_run ();
    }
  }
  else
  {
    JERRY_ASSERT (severity == MEM_TRY_GIVE_MEMORY_BACK_SEVERITY_HIGH);

    /* Freeing as much memory as we currently can */
    ecma_lcache_invalidate_all ();

    ecma_gc_run ();
  }
} /* ecma_try_to_give_back_some_memory */

/**
 * @}
 * @}
 */
