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

#include "ecma-function-object.h"
#include "ecma-globals.h"
#include "ecma-helpers.h"
#include "ecma-jobqueue.h"
#include "ecma-objects.h"
#include "ecma-objects-general.h"
#include "ecma-promise-object.h"
#include "jerry-port.h"
#include "jmem.h"
#include "jrt.h"

#ifndef CONFIG_DISABLE_ES2015_PROMISE_BUILTIN

/** \addtogroup ecma ECMA
 * @{
 *
 * \addtogroup ecmajobqueue ECMA Job Queue related routines
 * @{
 */

static ecma_job_promise_reaction_t*
create_promise_reaction_job (ecma_value_t reaction,
                             ecma_value_t argument)
{
  ecma_job_promise_reaction_t *job_p;
  job_p = (ecma_job_promise_reaction_t *) jmem_heap_alloc_block (sizeof (ecma_job_promise_reaction_t));

  job_p->reaction = ecma_copy_value (reaction);
  job_p->argument = ecma_copy_value (argument);

  return job_p;
}

static void
free_promise_reaction_job (ecma_job_promise_reaction_t* job_p)
{
  JERRY_ASSERT (job_p != NULL);

  ecma_free_value (job_p->reaction);
  ecma_free_value (job_p->argument);

  jmem_heap_free_block (job_p, sizeof (ecma_job_promise_reaction_t));
}

static ecma_job_promise_resolve_thenable_t*
create_promise_resolve_thenable_job (ecma_value_t promise,
                                     ecma_value_t thenable,
                                     ecma_value_t then)
{
  ecma_job_promise_resolve_thenable_t *job_p;
  job_p = (ecma_job_promise_resolve_thenable_t *) jmem_heap_alloc_block (sizeof (ecma_job_promise_resolve_thenable_t));

  job_p->promise = ecma_copy_value (promise);
  job_p->thenable = ecma_copy_value (thenable);
  job_p->then = ecma_copy_value (then);

  return job_p;
}

static void
free_promise_resolve_thenable_job (ecma_job_promise_resolve_thenable_t* job_p)
{
  JERRY_ASSERT (job_p != NULL);

  ecma_free_value (job_p->promise);
  ecma_free_value (job_p->thenable);
  ecma_free_value (job_p->then);

  jmem_heap_free_block (job_p, sizeof (ecma_job_promise_resolve_thenable_t));
}

/**
 * The processor for PromiseReactionJob
 *
 * See also: ES2015 25.4.2.1
 *
 * @return ecma value
 *         Returned value must be freed with ecma_free_value
 */
static ecma_value_t
process_promise_reaction_job (void *obj_p) /**< the job to be operated */
{
  ecma_job_promise_reaction_t *job_p = (ecma_job_promise_reaction_t *) obj_p;
  ecma_object_t *reaction_p = ecma_get_object_from_value (job_p->reaction);

  ecma_string_t *str_0 = ecma_new_ecma_string_from_uint32 (0);
  ecma_string_t *str_1 = ecma_new_ecma_string_from_uint32 (1);
  ecma_string_t *str_2 = ecma_new_ecma_string_from_uint32 (2);
  /* 2. string '0' indicates the [[Capability]] of reaction */
  ecma_value_t capability = ecma_op_object_get (reaction_p, str_0);
  /* 3. string '1' indicates the [[Handler]] of reaction */
  ecma_value_t handler = ecma_op_object_get (reaction_p, str_1);

  JERRY_ASSERT (ecma_is_value_boolean (handler) || ecma_op_is_callable (handler));

  ecma_value_t handler_result;

  if (ecma_is_value_boolean (handler))
  {
    /* 4-5. True indicates "identity" and false indicates "thrower" */
    handler_result = ecma_copy_value (job_p->argument);
  }

  else
  {
    /* 6. */
    handler_result = ecma_op_function_call (ecma_get_object_from_value (handler),
                                            ecma_make_simple_value (ECMA_SIMPLE_VALUE_UNDEFINED),
                                            &(job_p->argument),
                                            1);
  }

  ecma_value_t status;
  /* 7. */
  if (ecma_is_value_false (handler) || ECMA_IS_VALUE_ERROR (handler_result))
  {
    /* str '2' indicates [[Reject]] of Capability */
    ecma_value_t reject = ecma_op_object_get (ecma_get_object_from_value (capability), str_2);

    JERRY_ASSERT (ecma_op_is_callable (reject));

    status = ecma_op_function_call (ecma_get_object_from_value (reject),
                                    ecma_make_simple_value (ECMA_SIMPLE_VALUE_UNDEFINED),
                                    &handler_result,
                                    1);
    ecma_free_value (reject);
  }
  /* 8 */
  else
  {
    /* str '1' indicates [[Resolve]] of Capability */
    ecma_value_t resolve = ecma_op_object_get (ecma_get_object_from_value (capability), str_1);

    JERRY_ASSERT (ecma_op_is_callable (resolve));

    status = ecma_op_function_call (ecma_get_object_from_value (resolve),
                                    ecma_make_simple_value (ECMA_SIMPLE_VALUE_UNDEFINED),
                                    &handler_result,
                                    1);
    ecma_free_value (resolve);
  }

  ecma_free_value (handler_result);
  ecma_free_value (handler);
  ecma_free_value (capability);
  ecma_deref_ecma_string (str_0);
  ecma_deref_ecma_string (str_1);
  ecma_deref_ecma_string (str_2);
  free_promise_reaction_job (job_p);

  return status;
} /* ecma_job_process_promise_reaction_job */

/**
 * Process the PromiseResolveThenableJob
 *
 * See also: ES2015 25.4.2.2
 *
 * @return ecma value
 *         Returned value must be freed with ecma_free_value
 */
static ecma_value_t
process_promise_resolve_thenable_job (void *obj_p) /**< the job to be operated */
{
  ecma_job_promise_resolve_thenable_t *job_p = (ecma_job_promise_resolve_thenable_t *) obj_p;
  ecma_object_t *promise_p = ecma_get_object_from_value (job_p->promise);
  ecma_promise_resolving_functions_t *funcs;
  funcs = ecma_promise_create_resolving_functions (promise_p);

  ecma_value_t argv[] = {funcs->resolve, funcs->reject};
  ecma_value_t ret;
  ecma_value_t then_call_result = ecma_op_function_call (ecma_get_object_from_value (job_p->then),
                                                         job_p->thenable,
                                                         argv,
                                                         2);

  ret = then_call_result;

  if (ECMA_IS_VALUE_ERROR (then_call_result))
  {
    ret = ecma_op_function_call (ecma_get_object_from_value (funcs->reject),
                                 ecma_make_simple_value (ECMA_SIMPLE_VALUE_UNDEFINED),
                                 &then_call_result,
                                 1);

    ecma_free_value (then_call_result);
  }

  ecma_promise_free_resolving_functions (funcs);
  free_promise_resolve_thenable_job (job_p);

  return ret;
} /* process_promise_resolve_thenable_job */

void
ecma_enqueue_promise_reaction_job (ecma_value_t reaction,
                                   ecma_value_t argument)
{
  ecma_job_promise_reaction_t *job_p = create_promise_reaction_job (reaction, argument);
  jerry_port_jobqueue_enqueue (process_promise_reaction_job, job_p);
}

void
ecma_enqueue_promise_resolve_thenable_job (ecma_value_t promise,
                                           ecma_value_t thenable,
                                           ecma_value_t then)
{
  ecma_job_promise_resolve_thenable_t *job_p = create_promise_resolve_thenable_job (promise,
                                                                                    thenable,
                                                                                    then);
  jerry_port_jobqueue_enqueue (process_promise_resolve_thenable_job, job_p);
}

/**
 * @}
 * @}
 */
#endif /* !CONFIG_DISABLE_ES2015_PROMISE_BUILTIN */
