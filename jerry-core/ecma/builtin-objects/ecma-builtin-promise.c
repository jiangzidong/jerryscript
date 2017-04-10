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

#include "ecma-exceptions.h"
#include "ecma-function-object.h"
#include "ecma-globals.h"
#include "ecma-promise-object.h"

#ifndef CONFIG_DISABLE_ES2015_PROMISE_BUILTIN

#define ECMA_BUILTINS_INTERNAL
#include "ecma-builtins-internal.h"

#define BUILTIN_INC_HEADER_NAME "ecma-builtin-promise.inc.h"
#define BUILTIN_UNDERSCORED_ID promise
#include "ecma-builtin-internal-routines-template.inc.h"

/** \addtogroup ecma ECMA
 * @{
 *
 * \addtogroup ecmabuiltins
 * @{
 *
 * \addtogroup promise ECMA Promise object built-in
 * @{
 */

/**
 * The common function for 'reject' and 'resolve'.
 *
 * @return ecma value
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_promise_reject_or_resolve (ecma_value_t this_arg, /**< "this" argument */
                                        ecma_value_t argument, /**< argument for reject or resolve */
                                        bool is_resolve) /** whether it is for resolve routine */
{
  if (!ecma_is_value_object (this_arg))
  {
    return ecma_raise_type_error (ECMA_ERR_MSG ("'this' is not an object."));
  }

  uint8_t builtin_id = ecma_get_object_builtin_id (ecma_get_object_from_value (this_arg));

  if (builtin_id != ECMA_BUILTIN_ID_PROMISE)
  {
    return ecma_raise_type_error (ECMA_ERR_MSG ("'this' is not the Promise constructor."));
  }

  if (is_resolve)
  {
    if (ecma_is_value_object (argument)
        && ecma_is_promise (ecma_get_object_from_value (argument)))
    {
      return ecma_copy_value (argument);
    }
  }

  ecma_value_t capability = ecma_promise_new_capability ();

  if (ECMA_IS_VALUE_ERROR (capability))
  {
    return capability;
  }

  ecma_string_t *str;

  if (is_resolve)
  {
    /* String '1' indicates [[Resolve]] of Capability. */
    str = ecma_new_ecma_string_from_uint32 (1);
  }
  else
  {
    /* String '2' indicates [[Reject]] of Capability. */
    str = ecma_new_ecma_string_from_uint32 (2);
  }

  ecma_value_t func = ecma_op_object_get (ecma_get_object_from_value (capability), str);
  ecma_deref_ecma_string (str);

  ecma_value_t call_ret = ecma_op_function_call (ecma_get_object_from_value (func),
                                                 ecma_make_simple_value (ECMA_SIMPLE_VALUE_UNDEFINED),
                                                 &argument,
                                                 1);

  ecma_free_value (func);

  if (ECMA_IS_VALUE_ERROR (call_ret))
  {
    return call_ret;
  }

  ecma_free_value (call_ret);

  /* String '0' here indicates the [[Promise]] of Capability. */
  ecma_string_t *str_0 = ecma_new_ecma_string_from_uint32 (0);
  ecma_value_t promise_new = ecma_op_object_get (ecma_get_object_from_value (capability), str_0);
  ecma_deref_ecma_string (str_0);
  ecma_free_value (capability);


  return promise_new;
} /* ecma_builtin_promise_reject_or_resolve */


/**
 * Reject the promise if the value is error.
 *
 * See also:
 *         ES2015 25.4.1.1.1
 *
 * @return ecma value of the new promise.
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_promise_reject_abrupt (ecma_value_t abrupt_value,
                                    ecma_value_t capability)
{
  ecma_value_t reason = ecma_get_value_from_error_value (abrupt_value);
  /* String '2' indicates [[Reject]] of Capability. */
  ecma_string_t *str_2 = ecma_new_ecma_string_from_uint32 (2);
  ecma_value_t reject = ecma_op_object_get (ecma_get_object_from_value (capability), str_2);
  ecma_deref_ecma_string (str_2);

  ecma_value_t call_ret = ecma_op_function_call (ecma_get_object_from_value (reject),
                                                 ecma_make_simple_value (ECMA_SIMPLE_VALUE_UNDEFINED),
                                                 &reason,
                                                 1);
  ecma_free_value (call_ret);

  if (ECMA_IS_VALUE_ERROR (call_ret))
  {
    return call_ret;
  }

  ecma_free_value (call_ret);

  /* String '0' here indicates the [[Promise]] of Capability. */
  ecma_string_t *str_0 = ecma_new_ecma_string_from_uint32 (0);
  ecma_value_t promise_new = ecma_op_object_get (ecma_get_object_from_value (capability), str_0);
  ecma_deref_ecma_string (str_0);

  return promise_new;
} /* ecma_builtin_promise_reject_abrupt */

/**
 * The Promise.reject routine.
 *
 * See also:
 *         ES2015 25.4.4.4
 *
 * @return ecma value of the new promise.
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_promise_reject (ecma_value_t this_arg, /**< 'this' argument */
                             ecma_value_t reason) /**< the reason for reject */
{
  return ecma_builtin_promise_reject_or_resolve (this_arg, reason, false);
} /* ecma_builtin_promise_reject */

/**
 * The Promise.resolve routine.
 *
 * See also:
 *         ES2015 25.4.4.5
 *
 * @return ecma value of the new promise.
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_promise_resolve (ecma_value_t this_arg, /**< 'this' argument */
                              ecma_value_t argument) /**< the argument for resolve */
{
  return ecma_builtin_promise_reject_or_resolve (this_arg, argument, true);
} /* ecma_builtin_promise_resolve */

/**
 * Runtime Semantics: PerformPromiseRace.
 *
 * See also:
 *         ES2015 25.4.4.3.1
 *
 * @return ecma value of the new promise.
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_promise_do_race (ecma_value_t array, /**< the array for race */
                              ecma_value_t capability, /**< PromiseCapability record */
                              ecma_value_t ctor) /**< the caller of Promise.race */
{

  JERRY_ASSERT (ecma_is_value_object (capability)
                && ecma_is_value_object (array)
                && ecma_is_value_object (ctor));
  JERRY_ASSERT (ecma_get_object_builtin_id (ecma_get_object_from_value (ctor)) == ECMA_BUILTIN_ID_PROMISE);
  JERRY_ASSERT (ecma_get_object_type (ecma_get_object_from_value (array)) == ECMA_OBJECT_TYPE_ARRAY);

  ecma_value_t ret = ecma_make_simple_value (ECMA_SIMPLE_VALUE_EMPTY);

  ecma_string_t *magic_string_length_p = ecma_new_ecma_length_string ();
  ecma_object_t *array_p = ecma_get_object_from_value (array);
  ecma_value_t len_value = ecma_op_object_get (array_p, magic_string_length_p);
  ecma_deref_ecma_string (magic_string_length_p);
  ecma_length_t len = ecma_get_uint32_from_value (len_value);
  ecma_fast_free_value (len_value);

  ecma_length_t index = 0;
  ecma_string_t *str_0 = ecma_new_ecma_string_from_uint32 (0);
  ecma_string_t *str_1 = ecma_new_ecma_string_from_uint32 (1);
  ecma_string_t *str_2 = ecma_new_ecma_string_from_uint32 (2);

  while (true)
  {
    JERRY_ASSERT (index <= len);

    /* b-d. */
    if (index == len)
    {
      /* String '0' here indicates the [[Promise]] of Capability. */
      ret = ecma_op_object_get (ecma_get_object_from_value (capability), str_0);
      break;
    }

    /* e. */
    ecma_string_t *str_index = ecma_new_ecma_string_from_uint32 (index);
    ecma_value_t array_item = ecma_op_object_get (array_p, str_index);
    ecma_deref_ecma_string (str_index);

    /* h */
    ecma_value_t next_promise = ecma_builtin_promise_resolve (ctor, array_item);
    ecma_free_value (array_item);

    /* i */
    if (ECMA_IS_VALUE_ERROR (next_promise))
    {
      ret = next_promise;
      break;
    }

    /* j. String '1' indicates [[Resolve]] and '2' indicates [[Reject]]. */
    ecma_value_t resolve = ecma_op_object_get (ecma_get_object_from_value (capability),
                                               str_1);
    ecma_value_t reject = ecma_op_object_get (ecma_get_object_from_value (capability),
                                              str_2);

    ecma_value_t then_result = ecma_promise_then (next_promise, resolve, reject);

    ecma_free_value (reject);
    ecma_free_value (resolve);
    ecma_free_value (next_promise);

    /* k */
    if (ECMA_IS_VALUE_ERROR (then_result))
    {
      ret = then_result;
      break;
    }

    ecma_free_value (then_result);

    index ++;
  }

  ecma_deref_ecma_string (str_0);
  ecma_deref_ecma_string (str_1);
  ecma_deref_ecma_string (str_2);

  JERRY_ASSERT (!ecma_is_value_empty (ret));

  return ret;
} /* ecma_builtin_promise_do_race */

/**
 * The common function for both Promise.race and Promise.all.
 *
 * @return ecma value of the new promise.
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_promise_race_or_all (ecma_value_t this_arg, /**< 'this' argument */
                                  ecma_value_t array, /**< the items to be resolved */
                                  bool is_race) /**< indicates whether it is race function */
{
  if (!ecma_is_value_object (this_arg))
  {
    return ecma_raise_type_error (ECMA_ERR_MSG ("'this' is not an object."));
  }

  uint8_t builtin_id = ecma_get_object_builtin_id (ecma_get_object_from_value (this_arg));

  if (builtin_id != ECMA_BUILTIN_ID_PROMISE)
  {
    return ecma_raise_type_error (ECMA_ERR_MSG ("'this' is not the Promise constructor."));
  }

  ecma_value_t capability = ecma_promise_new_capability ();
  ecma_value_t ret = ecma_make_simple_value (ECMA_SIMPLE_VALUE_EMPTY);

  if (!ecma_is_value_object (array)
      || ecma_get_object_type (ecma_get_object_from_value (array)) != ECMA_OBJECT_TYPE_ARRAY)
  {
    ecma_value_t error = ecma_raise_type_error (ECMA_ERR_MSG ("Second argument is not an array."));
    ret = ecma_builtin_promise_reject_abrupt (error, capability);
    ecma_free_value (capability);

    return ret;
  }

  if (is_race)
  {
    ret = ecma_builtin_promise_do_race (array, capability, this_arg);
  }

  if (ECMA_IS_VALUE_ERROR (ret))
  {
    ret = ecma_get_value_from_error_value (ret);
  }

  ecma_free_value (capability);

  return ret;
} /* ecma_builtin_promise_race_or_all */

/**
 * The Promise.race routine.
 *
 * See also:
 *         ES2015 25.4.4.3
 *
 * @return ecma value of the new promise.
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_promise_race (ecma_value_t this_arg, /**< 'this' argument */
                           ecma_value_t array) /**< the items to be resolved */
{
  return ecma_builtin_promise_race_or_all (this_arg, array, true);
} /* ecma_builtin_promise_race */

/**
 * The Promise.all routine.
 *
 * See also:
 *         ES2015 25.4.4.1
 *
 * @return ecma value of the new promise.
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_promise_all (ecma_value_t this_arg, /**< 'this' argument */
                           ecma_value_t array) /**< the items to be resolved */
{
  return ecma_builtin_promise_race_or_all (this_arg, array, false);
} /* ecma_builtin_promise_all */

/**
 * Handle calling [[Call]] of built-in Promise object.
 *
 * ES2015 25.4.3 Promise is not intended to be called
 * as a function and will throw an exception when called
 * in that manner.
 *
 * @return ecma value
 */
ecma_value_t
ecma_builtin_promise_dispatch_call (const ecma_value_t *arguments_list_p, /**< arguments list */
                                    ecma_length_t arguments_list_len) /**< number of arguments */
{
  JERRY_ASSERT (arguments_list_len == 0 || arguments_list_p != NULL);

  return ecma_raise_type_error (ECMA_ERR_MSG ("Constructor Promise requires 'new'."));
} /* ecma_builtin_promise_dispatch_call */

/**
 * Handle calling [[Construct]] of built-in Promise object.
 *
 * @return ecma value
 */
ecma_value_t
ecma_builtin_promise_dispatch_construct (const ecma_value_t *arguments_list_p, /**< arguments list */
                                         ecma_length_t arguments_list_len) /**< number of arguments */
{
  JERRY_ASSERT (arguments_list_len == 0 || arguments_list_p != NULL);

  if (arguments_list_len == 0 || !ecma_op_is_callable (arguments_list_p[0]))
  {
    return ecma_raise_type_error (ECMA_ERR_MSG ("First parameter must be callable."));
  }

  return ecma_op_create_promise_object (arguments_list_p[0], true);
} /* ecma_builtin_promise_dispatch_construct */

/**
 * @}
 * @}
 * @}
 */

#endif /* !CONFIG_DISABLE_ES2015_PROMISE_BUILTIN */
