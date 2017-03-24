/* Copyright JS Foundation and other contributors, http://js.foundation
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

#include "ecma-boolean-object.h"
#include "ecma-builtins.h"
#include "ecma-exceptions.h"
#include "ecma-function-object.h"
#include "ecma-gc.h"
#include "ecma-globals.h"
#include "ecma-helpers.h"
#include "ecma-jobqueue.h"
#include "ecma-objects.h"
#include "ecma-objects-general.h"
#include "ecma-promise-object.h"
#include "ecma-try-catch-macro.h"
#include "jmem.h"

#ifndef CONFIG_DISABLE_ES2015_PROMISE_BUILTIN

/** \addtogroup ecma ECMA
 * @{
 *
 * \addtogroup ecmapromiseobject ECMA Promise object related routines
 * @{
 */

inline bool
ecma_is_promise (ecma_object_t *obj_p)
{
  lit_magic_string_id_t class_id = ecma_object_get_class_name (obj_p);

  return class_id == LIT_MAGIC_STRING_PROMISE_UL;
}

inline ecma_value_t
ecma_promise_get_result (ecma_object_t *obj_p)
{
  ecma_extended_object_t *ext_object_p = (ecma_extended_object_t *) obj_p;

  return ext_object_p->u.class_prop.u.value;
}

inline void
ecma_promise_set_result (ecma_object_t *obj_p,
                         ecma_value_t result)
{
  ecma_extended_object_t *ext_object_p = (ecma_extended_object_t *) obj_p;

  ext_object_p->u.class_prop.u.value = result;
}

inline uint8_t
ecma_promise_get_state (ecma_object_t *obj_p)
{
  ecma_promise_object_t *promise_object_p = (ecma_promise_object_t *) obj_p;

  return promise_object_p->state;
}

inline void
ecma_promise_set_state (ecma_object_t *obj_p,
                        uint8_t state)
{
  ecma_promise_object_t *promise_object_p = (ecma_promise_object_t *) obj_p;

  promise_object_p->state = state;
}

static inline bool
is_already_resolved_value_true (ecma_value_t already_resolved)
{
  JERRY_ASSERT (ecma_is_value_object (already_resolved));

  ecma_object_t *already_resolved_p = ecma_get_object_from_value (already_resolved);
  ecma_extended_object_t *ext_object_p = (ecma_extended_object_t *) already_resolved_p;

  JERRY_ASSERT (ext_object_p->u.class_prop.class_id == LIT_MAGIC_STRING_BOOLEAN_UL);

  return ext_object_p->u.class_prop.u.value == ecma_make_boolean_value (true);
}

static inline void
set_already_resolved_value (ecma_value_t already_resolved,
                            bool is_true)
{
  ecma_object_t *already_resolved_p = ecma_get_object_from_value (already_resolved);
  ecma_extended_object_t *ext_object_p = (ecma_extended_object_t *) already_resolved_p;

  JERRY_ASSERT (ext_object_p->u.class_prop.class_id == LIT_MAGIC_STRING_BOOLEAN_UL);

  ext_object_p->u.class_prop.u.value = ecma_make_boolean_value (is_true);
}

/**
 * Takes a collection of Reactions and enqueues a new PromiseReactionJob for each Reaction
 *
 * See also: 25.4.1.8
 */
static ecma_value_t
trigger_promise_reactions (ecma_collection_header_t *reactions,
                           ecma_value_t reason)
{
  ecma_collection_iterator_t iter;
  ecma_collection_iterator_init (&iter, reactions);

  while (ecma_collection_iterator_next (&iter))
  {
    ecma_enqueue_promise_reaction_job (*iter.current_value_p, reason);
  }

  ecma_free_values_collection (reactions, true);

  return ecma_make_simple_value (ECMA_SIMPLE_VALUE_UNDEFINED);
} /* trigger_promise_reactions */

/**
 * Reject a Promise with a reason
 *
 * See also: 25.4.1.7
 */
static ecma_value_t
reject_promise (ecma_value_t promise,
                ecma_value_t reason)
{
  ecma_object_t *obj_p = ecma_get_object_from_value (promise);

  JERRY_ASSERT (ecma_promise_get_state (obj_p) == ECMA_PROMISE_STATE_PENDING);

  ecma_promise_set_result (obj_p, reason);

  ecma_promise_object_t *promise_p = (ecma_promise_object_t *) obj_p;
  ecma_collection_header_t *reactions = promise_p->reject_reactions;
  promise_p->reject_reactions = ecma_new_values_collection (NULL, 0, true);
  /* Free all fullfill_reactions */
  ecma_free_values_collection (promise_p->fulfill_reactions, true);
  promise_p->fulfill_reactions = ecma_new_values_collection (NULL, 0, true);

  ecma_promise_set_state (obj_p, ECMA_PROMISE_STATE_REJECTED);

  return trigger_promise_reactions (reactions, reason);
} /* reject_promise */

/**
 * Fulfill a Promise with a value
 *
 * See also: 25.4.1.4
 */
static ecma_value_t
fulfill_promise (ecma_value_t promise,
                 ecma_value_t value)
{
  ecma_object_t *obj_p = ecma_get_object_from_value (promise);

  JERRY_ASSERT (ecma_promise_get_state (obj_p) == ECMA_PROMISE_STATE_PENDING);

  ecma_promise_set_result (obj_p, value);

  ecma_promise_object_t *promise_p = (ecma_promise_object_t *) obj_p;
  ecma_collection_header_t *reactions = promise_p->fulfill_reactions;
  promise_p->fulfill_reactions = ecma_new_values_collection (NULL, 0, true);
  /* Free all reject_reactions */
  ecma_free_values_collection (promise_p->reject_reactions, true);
  promise_p->reject_reactions = ecma_new_values_collection (NULL, 0, true);

  ecma_promise_set_state (obj_p, ECMA_PROMISE_STATE_FULFILLED);

  return trigger_promise_reactions (reactions, value);
} /* fulfill_promise */


/**
 * Native handler for Promise Reject Function
 *
 * See also: ES2015 25.4.1.3.1
 */
static ecma_value_t
reject_function_handler (const ecma_value_t function,
                         const ecma_value_t this __attribute__((unused)),
                         const ecma_value_t argv[],
                         const ecma_length_t argc __attribute__((unused)))
{
  ecma_string_t *str_0 = ecma_new_ecma_string_from_uint32 (0);
  ecma_string_t *str_1 = ecma_new_ecma_string_from_uint32 (1);
  ecma_object_t* function_p = ecma_get_object_from_value (function);
  /* 2. */
  ecma_value_t promise = ecma_op_object_get (function_p, str_0);
  /* 1. */
  JERRY_ASSERT (ecma_is_promise (ecma_get_object_from_value (promise)));
  /* 3. */
  ecma_value_t already_resolved = ecma_op_object_get (function_p, str_1);
  /* 4. */
  if (is_already_resolved_value_true (already_resolved))
  {
    ecma_deref_ecma_string (str_0);
    ecma_deref_ecma_string (str_1);
    ecma_free_value (promise);
    ecma_free_value (already_resolved);
    return ecma_make_simple_value (ECMA_SIMPLE_VALUE_UNDEFINED);
  }
  /* 5. */
  set_already_resolved_value (already_resolved, true);
  /* 6. */
  ecma_value_t ret = reject_promise (promise, argv[0]);
  ecma_deref_ecma_string (str_0);
  ecma_deref_ecma_string (str_1);
  ecma_free_value (promise);
  ecma_free_value (already_resolved);
  return ret;
} /* reject_function_handler */

/**
 * Native handler for Promise Resolve Function
 *
 * See also: ES2015 25.4.1.3.2
 */
static ecma_value_t
resolve_function_handler (const ecma_value_t function,
                          const ecma_value_t this __attribute__((unused)),
                          const ecma_value_t argv[],
                          const ecma_length_t argc __attribute__((unused)))
{
  ecma_string_t *str_0 = ecma_new_ecma_string_from_uint32 (0);
  ecma_string_t *str_1 = ecma_new_ecma_string_from_uint32 (1);
  ecma_object_t* function_p = ecma_get_object_from_value (function);
  /* 2. */
  ecma_value_t promise = ecma_op_object_get (function_p, str_0);
  /* 1. */
  JERRY_ASSERT (ecma_is_promise (ecma_get_object_from_value (promise)));
  /* 3. */
  ecma_value_t already_resolved = ecma_op_object_get (function_p, str_1);

  ecma_value_t ret;
  /* 4. */
  if (is_already_resolved_value_true (already_resolved))
  {
    ret = ecma_make_simple_value (ECMA_SIMPLE_VALUE_UNDEFINED);
    goto end_of_resolve_function;
  }
  /* 5. */
  set_already_resolved_value (already_resolved, true);

  /* 6. */
  if (argv[0] == promise)
  {
    ecma_object_t *error_p = ecma_new_standard_error (ECMA_ERROR_TYPE);
    ret = reject_promise (promise, ecma_make_object_value (error_p));
    ecma_deref_object (error_p);
    goto end_of_resolve_function;
  }
  /* 7. */
  if (!ecma_is_value_object (argv[0]))
  {
    ret = fulfill_promise (promise, argv[0]);
    goto end_of_resolve_function;
  }
  /* 8. */
  ecma_string_t *str_then = ecma_new_ecma_string_from_magic_string_id (LIT_MAGIC_STRING_THEN);
  ecma_value_t then = ecma_op_object_get (ecma_get_object_from_value (argv[0]), str_then);

  if (ECMA_IS_VALUE_ERROR (then))
  {
    /* 9. */
    ret = reject_promise (promise, then);
  }
  else if (!ecma_op_is_callable (then))
  {
    /* 11 .*/
    ret = fulfill_promise(promise, argv[0]);
  }
  else
  {
    /* 12 */
    ecma_enqueue_promise_resolve_thenable_job (promise, argv[0], then);
    ret = ecma_make_simple_value (ECMA_SIMPLE_VALUE_UNDEFINED);
  }

  ecma_deref_ecma_string (str_then);
  ecma_free_value (then);

end_of_resolve_function:
  ecma_deref_ecma_string (str_0);
  ecma_deref_ecma_string (str_1);
  ecma_free_value (promise);
  ecma_free_value (already_resolved);
  return ret;
} /* resolve_function_handler */

/**
 * Native handler for CapabilityiesExecutor Function
 *
 * See also: ES2015 25.4.1.5.1
 */
static ecma_value_t
executor_function_handler (const ecma_value_t function,
                           const ecma_value_t this __attribute__((unused)),
                           const ecma_value_t argv[],
                           const ecma_length_t argc __attribute__((unused)))
{
  ecma_string_t *str_0 = ecma_new_ecma_string_from_uint32 (0);
  ecma_string_t *str_1 = ecma_new_ecma_string_from_uint32 (1);
  ecma_string_t *str_2 = ecma_new_ecma_string_from_uint32 (2);
  ecma_value_t ret = ecma_make_simple_value (ECMA_SIMPLE_VALUE_UNDEFINED);
  ecma_value_t resolve = ecma_make_simple_value (ECMA_SIMPLE_VALUE_UNDEFINED);
  ecma_value_t reject = ecma_make_simple_value (ECMA_SIMPLE_VALUE_UNDEFINED);
  /* 2. string '0' indicates [[Capability]] of the function object */
  ecma_value_t capability = ecma_op_object_get (ecma_get_object_from_value (function), str_0);
  /* 3. string '1' indicates [[Resolve]] of the capability */
  resolve = ecma_op_object_get (ecma_get_object_from_value (capability), str_1);

  if (resolve != ecma_make_simple_value (ECMA_SIMPLE_VALUE_UNDEFINED))
  {
    ret = ecma_raise_type_error (ECMA_ERR_MSG ("'resolve' function should be undefined."));
    goto end_of_executor_function;
  }
  /* 4. string '2' indicates [[Reject]] of the capability */
  reject = ecma_op_object_get (ecma_get_object_from_value (capability), str_2);

  if (reject != ecma_make_simple_value (ECMA_SIMPLE_VALUE_UNDEFINED))
  {
    ret = ecma_raise_type_error (ECMA_ERR_MSG ("'reject' function should be undefined."));
    goto end_of_executor_function;
  }

  /* 5. */
  ecma_op_object_put (ecma_get_object_from_value (capability),
                      str_1,
                      argv[0],
                      false);
  /* 6. */
  ecma_op_object_put (ecma_get_object_from_value (capability),
                      str_2,
                      argv[1],
                      false);

end_of_executor_function:

  ecma_free_value (reject);
  ecma_free_value (resolve);
  ecma_free_value (capability);
  ecma_deref_ecma_string (str_0);
  ecma_deref_ecma_string (str_1);
  ecma_deref_ecma_string (str_2);

  return ret;
} /* executor_function_handler */

ecma_promise_resolving_functions_t *
ecma_promise_create_resolving_functions (ecma_object_t *object_p)
{
  /* 1. */
  ecma_value_t already_resolved = ecma_op_create_boolean_object (ecma_make_boolean_value (false));

  ecma_string_t *str_0 = ecma_new_ecma_string_from_uint32 (0);
  ecma_string_t *str_1 = ecma_new_ecma_string_from_uint32 (1);
  /* 2. */
  ecma_object_t *resolve_p = ecma_op_create_external_function_object ((ecma_external_pointer_t) resolve_function_handler);

  /* 3. string '0' here indicates [[Promise]] */
  ecma_op_object_put (resolve_p,
                      str_0,
                      ecma_make_object_value (object_p),
                      false);
  /* 4. string '1' here indicates [[AlreadyResolved]] */
  ecma_op_object_put (resolve_p,
                      str_1,
                      already_resolved,
                      false);
  /* 5. */
  ecma_object_t *reject_p = ecma_op_create_external_function_object ((ecma_external_pointer_t) reject_function_handler);
  /* 6. string '0' here indicates [[Promise]] */
  ecma_op_object_put (reject_p,
                      str_0,
                      ecma_make_object_value (object_p),
                      false);
  /* 7. string '1' here indicates [[AlreadyResolved]] */
  ecma_op_object_put (reject_p,
                      str_1,
                      already_resolved,
                      false);

  /* 8. */
  ecma_promise_resolving_functions_t *funcs = jmem_heap_alloc_block (sizeof (ecma_promise_resolving_functions_t));
  funcs->resolve = ecma_make_object_value (resolve_p);
  funcs->reject = ecma_make_object_value (reject_p);

  ecma_deref_ecma_string (str_0);
  ecma_deref_ecma_string (str_1);
  ecma_free_value(already_resolved);

  return funcs;
}

void
ecma_promise_free_resolving_functions (ecma_promise_resolving_functions_t* funcs)
{
  ecma_free_value (funcs->resolve);
  ecma_free_value (funcs->reject);
  jmem_heap_free_block (funcs, sizeof (ecma_promise_resolving_functions_t));
}

ecma_value_t
ecma_op_create_promise_object (const ecma_value_t *arguments_list_p, /**< list of arguments that
                                                                      *   are passed to Promise constructor */
                               ecma_length_t arguments_list_len) /**< length of the arguments' list */
{
  JERRY_ASSERT (arguments_list_len == 0 || arguments_list_p != NULL);
  /* 2. */
  if (arguments_list_len == 0 || !ecma_op_is_callable (arguments_list_p[0]))
  {
    return ecma_raise_type_error (ECMA_ERR_MSG ("first parameter must be callable."));
  }

  ecma_value_t executor = arguments_list_p[0];
  /* 3. */
  ecma_object_t *prototype_obj_p = ecma_builtin_get (ECMA_BUILTIN_ID_PROMISE_PROTOTYPE);
  ecma_object_t *object_p = ecma_create_object (prototype_obj_p,
                                                sizeof (ecma_promise_object_t),
                                                ECMA_OBJECT_TYPE_CLASS);
  ecma_deref_object (prototype_obj_p);
  ecma_extended_object_t *ext_object_p = (ecma_extended_object_t *) object_p;
  ext_object_p->u.class_prop.class_id = LIT_MAGIC_STRING_PROMISE_UL;

  ecma_promise_set_result (object_p, ecma_make_simple_value (ECMA_SIMPLE_VALUE_EMPTY));
  /* 5 */
  ecma_promise_set_state (object_p, ECMA_PROMISE_STATE_PENDING);
  /* 6-7. */
  ecma_promise_object_t *promise_object_p = (ecma_promise_object_t *) object_p;
  promise_object_p->fulfill_reactions = ecma_new_values_collection (NULL, 0, true);
  promise_object_p->reject_reactions = ecma_new_values_collection (NULL, 0, true);
  /* 8. */
  ecma_promise_resolving_functions_t *funcs = ecma_promise_create_resolving_functions (object_p);
  /* 9. */
  ecma_value_t argv[] = {funcs->resolve, funcs->reject};
  ecma_value_t completion = ecma_op_function_call (ecma_get_object_from_value (executor),
                                                   ecma_make_simple_value (ECMA_SIMPLE_VALUE_UNDEFINED),
                                                   argv,
                                                   2);

  ecma_value_t status = ecma_make_simple_value (ECMA_SIMPLE_VALUE_EMPTY);

  if (ECMA_IS_VALUE_ERROR (completion))
  {
    /* 10.a. */
    status = ecma_op_function_call (ecma_get_object_from_value (funcs->reject),
                                    ecma_make_simple_value (ECMA_SIMPLE_VALUE_UNDEFINED),
                                    &completion,
                                    1);
  }

  ecma_promise_free_resolving_functions (funcs);
  ecma_free_value (completion);

  /* 10.b. */
  if (ECMA_IS_VALUE_ERROR (status))
  {
    ecma_deref_object (object_p);
    return status;
  }

  /* 11. */
  ecma_free_value (status);

  return ecma_make_object_value (object_p);
}

/**
 * Create a new PromiseCapability
 *
 * See also: 25.4.1.5
 */
ecma_value_t
ecma_promise_new_capability(void)
{
  /* 3. */
  ecma_object_t *capability_p = ecma_op_create_object_object_noarg ();
  ecma_string_t *str_0 = ecma_new_ecma_string_from_uint32 (0);

  /* 4. */
  ecma_object_t *executor_p = ecma_op_create_external_function_object ((ecma_external_pointer_t) executor_function_handler);
  ecma_value_t executor = ecma_make_object_value (executor_p);
  /* 5. string '0' here indicates the [[Capability]] of executor */
  ecma_op_object_put (executor_p,
                      str_0,
                      ecma_make_object_value (capability_p),
                      false);

  /* 6. */
  ecma_value_t promise = ecma_op_create_promise_object (&executor, 1);

  /* 10. string '0' here indicates the [[Promise]] of Capability */
  ecma_op_object_put (capability_p,
                      str_0,
                      promise,
                      false);

  ecma_deref_object (executor_p);
  ecma_deref_ecma_string (str_0);
  /* 7. */
  if (ECMA_IS_VALUE_ERROR (promise))
  {
    ecma_free_value (promise);
    ecma_deref_object (capability_p);
    return promise;
  }

  ecma_free_value (promise);
  /* 8. str '1' indicates [[Resolve]] of Capability */
  ecma_string_t *str_1 = ecma_new_ecma_string_from_uint32 (1);
  ecma_value_t resolve = ecma_op_object_get (capability_p, str_1);
  ecma_deref_ecma_string (str_1);

  if (!ecma_op_is_callable (resolve))
  {
    ecma_free_value (resolve);
    ecma_deref_object (capability_p);
    return ecma_raise_type_error (ECMA_ERR_MSG ("'resolve' parameter must be callable."));
  }

  ecma_free_value (resolve);
  /* 9. str '2' indicates [[Reject]] of Capability*/
  ecma_string_t *str_2 = ecma_new_ecma_string_from_uint32 (2);
  ecma_value_t reject = ecma_op_object_get (capability_p, str_2);
  ecma_deref_ecma_string (str_2);

  if (!ecma_op_is_callable (reject))
  {
    ecma_free_value (reject);
    ecma_deref_object (capability_p);
    return ecma_raise_type_error (ECMA_ERR_MSG ("'reject' parameter must be callable."));
  }
  ecma_free_value (reject);
  /* 11. */
  return ecma_make_object_value (capability_p);
} /* ecma_promise_new_capability */

/**
 * It performs the "then" operation on promise using onFulfilled
 * and onRejected as its settlement actions.
 * The result is resultCapability’s promise.
 *
 * See also: 25.4.5.3.1
 */
ecma_value_t
ecma_promise_then (ecma_value_t promise,
                   ecma_value_t on_fulfilled,
                   ecma_value_t on_rejected,
                   ecma_value_t result_capability)
{
  ecma_string_t *str_0 = ecma_new_ecma_string_from_uint32 (0);
  ecma_string_t *str_1 = ecma_new_ecma_string_from_uint32 (1);
  /* 3. boolean true indicates "indentity" */
  if (!ecma_op_is_callable (on_fulfilled))
  {
    on_fulfilled = ecma_make_boolean_value (true);
  }
  /* 4. boolean false indicates "thrower" */
  if (!ecma_op_is_callable (on_rejected))
  {
    on_rejected = ecma_make_boolean_value (false);
  }
  /* 5-6. string '0' indicates [[Capability]] of a PromiseReaction, '1' indicates [[Handler]] */
  ecma_object_t *fulfill_reaction_p = ecma_op_create_object_object_noarg ();
  ecma_object_t *reject_reaction_p = ecma_op_create_object_object_noarg ();
  ecma_op_object_put (fulfill_reaction_p,
                      str_0,
                      result_capability,
                      false);
  ecma_op_object_put (fulfill_reaction_p,
                      str_1,
                      on_fulfilled,
                      false);

  ecma_op_object_put (reject_reaction_p,
                      str_0,
                      result_capability,
                      false);
  ecma_op_object_put (reject_reaction_p,
                      str_1,
                      on_rejected,
                      false);

  ecma_object_t *obj_p = ecma_get_object_from_value (promise);
  ecma_promise_object_t *promise_p = (ecma_promise_object_t *) obj_p;

  if (ecma_promise_get_state (obj_p) == ECMA_PROMISE_STATE_PENDING)
  {
    /* 7. */
    ecma_append_to_values_collection (promise_p->fulfill_reactions,
                                      ecma_make_object_value (fulfill_reaction_p),
                                      true);

    ecma_append_to_values_collection (promise_p->reject_reactions,
                                      ecma_make_object_value (reject_reaction_p),
                                      true);
  }
  else if (ecma_promise_get_state (obj_p) == ECMA_PROMISE_STATE_FULFILLED)
  {
    /* 8. */
    ecma_value_t value = ecma_promise_get_result (obj_p);
    ecma_enqueue_promise_reaction_job (ecma_make_object_value (fulfill_reaction_p), value);
  }
  else
  {
    /* 9. */
    ecma_value_t reason = ecma_promise_get_result (obj_p);
    ecma_enqueue_promise_reaction_job (ecma_make_object_value (reject_reaction_p), reason)  ;
  }

  /* 10. string '0' indicates [[Promise]] of a Capability */
  ecma_value_t ret = ecma_op_object_get (ecma_get_object_from_value (result_capability), str_0);

  ecma_deref_object (fulfill_reaction_p);
  ecma_deref_object (reject_reaction_p);
  ecma_deref_ecma_string (str_0);
  ecma_deref_ecma_string (str_1);

  return ret;
} /* ecma_promise_then */

/**
 * @}
 * @}
 */
#endif /* !CONFIG_DISABLE_ES2015_PROMISE_BUILTIN */
