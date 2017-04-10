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
