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

#ifndef ECMA_JOB_QUEUE_H
#define ECMA_JOB_QUEUE_H

#ifndef CONFIG_DISABLE_ES2015_PROMISE_BUILTIN

/** \addtogroup ecma ECMA
 * @{
 *
 * \addtogroup ecmajobqueue ECMA Job Queue related routines
 * @{
 */

typedef struct
{
  ecma_value_t reaction;
  ecma_value_t argument;
} ecma_job_promise_reaction_t;

typedef struct
{
  ecma_value_t promise;
  ecma_value_t thenable;
  ecma_value_t then;
} ecma_job_promise_resolve_thenable_t;

void ecma_enqueue_promise_reaction_job (ecma_value_t reaction, ecma_value_t argument);
void ecma_enqueue_promise_resolve_thenable_job (ecma_value_t promise, ecma_value_t thenable, ecma_value_t then);

/**
 * @}
 * @}
 */
#endif /* !CONFIG_DISABLE_ES2015_PROMISE_BUILTIN */
#endif /* !ECMA_JOB_QUEUE_H */
