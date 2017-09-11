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

#include "jerryscript.h"
#include "jrt.h"

#include <emscripten/emscripten.h>

#include <string.h>
#include <include/jerryscript-core.h>

#define TYPE_ERROR \
    jerry_create_error(JERRY_ERROR_TYPE, NULL);

#define TYPE_ERROR_ARG \
    jerry_create_error( \
      JERRY_ERROR_TYPE, (const jerry_char_t *)"wrong type of argument");

#define TYPE_ERROR_FLAG \
    jerry_create_error( \
        JERRY_ERROR_TYPE, \
        (const jerry_char_t *)"argument cannot have an error flag");

////////////////////////////////////////////////////////////////////////////////
// Parser and Executor Function
////////////////////////////////////////////////////////////////////////////////

jerry_value_t jerry_eval(const jerry_char_t *source_p, size_t source_size,
                         bool is_strict) {
  return (jerry_value_t)EM_ASM_INT({
      // jerry_eval() uses an indirect eval() call,
      // so the global execution context is used.
      // Also see ECMA 5.1 -- 10.4.2 Entering Eval Code.
      var indirectEval = eval;
      try {
        var source = Module.Pointer_stringify($0, $1);
        var strictComment = __jerry.getUseStrictComment($2);
        return __jerry.ref(indirectEval(strictComment + source));
      } catch (e) {
        var error_ref = __jerry.ref(e);
        __jerry.setError(error_ref, true);
        return error_ref;
      }
    }, source_p, source_size, is_strict);
}

void
jerry_gc (void)
{
  EM_ASM({
    // Hint: use `node --expose-gc` to enable this!
    if (typeof gc === 'function') {
      gc();
    }
  });
} /* jerry_gc */

jerry_value_t
jerry_parse (const jerry_char_t *source_p, /**< script source */
             size_t source_size, /**< script source size */
             bool is_strict) /**< strict mode */
{
  return (jerry_value_t)EM_ASM_INT({
    var source = Module.Pointer_stringify($0, $1);
    var strictComment = __jerry.getUseStrictComment($2);
    var strictCommentAndSource = strictComment + source;
    try
    {
      // Use new Function just to parse the source
      // and immediately throw any syntax errors if needed.
      new Function (strictCommentAndSource);
      // If it parsed OK, use a function with a wrapped, indirect eval call
      // execute it later on when jerry_run is called.
      // Indirect eval is used because the global execution context must be
      // used when running the source to mirror the behavior of jerry_parse.
      var f = function() {
        var indirectEval = eval;
        return indirectEval(strictCommentAndSource);
      };
      return __jerry.ref(f);
    }
    catch (e)
    {
      var error_ref = __jerry.ref(e);
      __jerry.setError(error_ref, true);
      return error_ref;
    }
  }, source_p, source_size, is_strict);
} /* jerry_parse */

jerry_value_t
jerry_parse_named_resource (const jerry_char_t *name_p, /**< name (usually a file name) */
                            size_t name_length, /**< length of name */
                            const jerry_char_t *source_p, /**< script source */
                            size_t source_size, /**< script source size */
                            bool is_strict) /**< strict mode */
{
  JERRY_UNUSED (name_p);
  JERRY_UNUSED (name_length);
  return jerry_parse (source_p, source_size, is_strict);
} /* jerry_parse_named_resource */

jerry_value_t
jerry_run (const jerry_value_t func_val) /**< function to run */
{
  return (jerry_value_t) EM_ASM_INT ({
    var f = __jerry.get($0);
    try
    {
      if (typeof f !== 'function') {
        throw new Error('wrong type of argument');
      }
      var result = f();
      return __jerry.ref(result);
    }
    catch (e)
    {
      var error_ref = __jerry.ref(e);
      __jerry.setError(error_ref, true);
      return error_ref;
    };
  }, func_val);
} /* jerry_run */

bool
jerry_run_simple (const jerry_char_t *script_source_p, /**< script source */
                  size_t script_source_size, /**< script source size */
                  jerry_init_flag_t flags) /**< combination of Jerry flags */
{
  jerry_init (flags);
  const jerry_value_t eval_ret_val = jerry_eval(script_source_p, script_source_size, false /* is_strict */);
  const bool has_error = jerry_value_has_error_flag (eval_ret_val);
  jerry_release_value (eval_ret_val);
  jerry_cleanup ();
  return !has_error;
} /* jerry_run_simple */

jerry_value_t jerry_acquire_value(jerry_value_t value) {
  return (jerry_value_t)EM_ASM_INT({
      return __jerry.acquire($0);
    }, value);
}

void jerry_release_value(jerry_value_t value) {
  EM_ASM_INT({
      __jerry.release($0);
    }, value);
}

jerry_value_t
jerry_run_all_enqueued_jobs (void)
{
  EM_ASM({
    console.warn('jerry_run_all_enqueued_jobs() is not yet implemented.');
  });
  return jerry_create_undefined ();
} /* jerry_run_all_enqueued_jobs */

////////////////////////////////////////////////////////////////////////////////
// Get the global context
////////////////////////////////////////////////////////////////////////////////
jerry_value_t jerry_get_global_object(void) {
  return ((jerry_value_t)EM_ASM_INT_V({ \
      return __jerry.ref(Function('return this;')()); \
    }));
}

jerry_value_t jerry_get_global_builtin(const jerry_char_t *builtin_name) {
  return ((jerry_value_t)EM_ASM_INT({ \
      var global = Function('return this;')();
      return __jerry.ref(global[Module.Pointer_stringify($0)]); \
    }, builtin_name));
}

////////////////////////////////////////////////////////////////////////////////
// Jerry Value Type Checking
////////////////////////////////////////////////////////////////////////////////

#define JERRY_VALUE_HAS_TYPE(ref, typename) \
    ((bool)EM_ASM_INT({ \
             return typeof __jerry.get($0) === (typename); \
           }, (ref)))

#define JERRY_VALUE_IS_INSTANCE(ref, type) \
    ((bool)EM_ASM_INT({ \
             return __jerry.get($0) instanceof (type); \
           }, (ref)))

bool jerry_value_is_array(const jerry_value_t value) {
  return JERRY_VALUE_IS_INSTANCE(value, Array);
}

bool jerry_value_is_boolean(const jerry_value_t value) {
  return JERRY_VALUE_HAS_TYPE(value, 'boolean');
}

bool jerry_value_is_constructor(const jerry_value_t value) {
  return jerry_value_is_function(value);
}

bool jerry_value_is_function(const jerry_value_t value) {
  return JERRY_VALUE_HAS_TYPE(value, 'function');
}

bool jerry_value_is_number(const jerry_value_t value) {
  return JERRY_VALUE_HAS_TYPE(value, 'number');
}

bool jerry_value_is_null(const jerry_value_t value) {
  return ((bool)EM_ASM_INT({
      return __jerry.get($0) === null;
    }, value));
}

bool jerry_value_is_object(const jerry_value_t value) {
  return (bool)EM_ASM_INT({
    var value = __jerry.get($0);
    if (value === null) {
      return false;
    }
    var typeStr = typeof value;
    return typeStr === 'object' || typeStr === 'function' ;
  }, value);
}

bool jerry_value_is_string(const jerry_value_t value) {
  return JERRY_VALUE_HAS_TYPE(value, 'string');
}

bool jerry_value_is_undefined(const jerry_value_t value) {
  return JERRY_VALUE_HAS_TYPE(value, 'undefined');
}

bool
jerry_value_is_promise (const jerry_value_t value)
{
  EM_ASM({
             console.warn('jerry_value_is_promise() is not yet implemented');
         });
  JERRY_UNUSED (value);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Jerry Value Getter Functions
////////////////////////////////////////////////////////////////////////////////

bool jerry_get_boolean_value(const jerry_value_t value) {
  if (!jerry_value_is_boolean(value))
  {
    return false;
  }
  return (bool)EM_ASM_INT({
      return (__jerry.get($0) === true);
    }, value);
}

double jerry_get_number_value(const jerry_value_t value) {
  if (!jerry_value_is_number(value))
  {
    return 0.0;
  }
  return EM_ASM_DOUBLE({
      return __jerry.get($0);
    }, value);
}
////////////////////////////////////////////////////////////////////////////////
// Functions for UTF-8 encoded string values
// JerryScript's internal string encoding is CESU-8.
////////////////////////////////////////////////////////////////////////////////
jerry_size_t jerry_get_utf8_string_size(const jerry_value_t value) {
  if (!jerry_value_is_string(value))
  {
    return 0;
  }
  return (jerry_size_t)EM_ASM_INT({
      return Module.lengthBytesUTF8(__jerry.get($0));
    }, value);
}

jerry_size_t jerry_string_to_utf8_char_buffer(const jerry_value_t value,
                                              jerry_char_t *buffer_p,
                                              jerry_size_t buffer_size) {
  const jerry_size_t str_size = jerry_get_utf8_string_size(value);
  if (str_size == 0 || buffer_size < str_size || buffer_p == NULL)
  {
    return 0;
  }

  return (jerry_size_t) EM_ASM_INT({
      var str = __jerry.get($0);
      return Module.stringToUTF8DataOnly(str, $1, $2);
  }, value, buffer_p, buffer_size);
}

jerry_size_t
jerry_substring_to_char_buffer (const jerry_value_t value, /**< input string value */
                                jerry_length_t start_pos, /**< position of the first character */
                                jerry_length_t end_pos, /**< position of the last character */
                                jerry_char_t *buffer_p, /**< [out] output characters buffer */
                                jerry_size_t buffer_size) /**< size of output buffer */
{
  if (buffer_p == NULL || buffer_size == 0 || !jerry_value_is_string (value))
  {
    return 0;
  }
  return (jerry_size_t) EM_ASM_INT({
    var str = __jerry.get($0);
    var substr = str.slice($1, $2);
    return Module.stringToCESU8DataOnly(substr, $3, $4);
  }, value, start_pos, end_pos, buffer_p, buffer_size);
} /* jerry_substring_to_char_buffer */

bool
jerry_is_valid_utf8_string (const jerry_char_t *utf8_buf_p, /**< UTF-8 string */
                            jerry_size_t buf_size) /**< string size */
{
  EM_ASM({
    console.warn('jerry_is_valid_utf8_string() is not yet implemented');
  });
  JERRY_UNUSED (utf8_buf_p);
  JERRY_UNUSED (buf_size);
  return true;
} /* jerry_is_valid_utf8_string */

bool
jerry_is_valid_cesu8_string (const jerry_char_t *cesu8_buf_p, /**< CESU-8 string */
                             jerry_size_t buf_size) /**< string size */
{
  EM_ASM({
           console.warn('jerry_is_valid_cesu8_string() is not yet implemented');
         });
  JERRY_UNUSED (cesu8_buf_p);
  JERRY_UNUSED (buf_size);
  return true;
} /* jerry_is_valid_cesu8_string */

jerry_length_t
jerry_get_utf8_string_length (const jerry_value_t value) /**< input string */
{
  if (!jerry_value_is_string (value))
  {
    return 0;
  }
  return (jerry_length_t) EM_ASM_INT({
    var str = __jerry.get($0);
    var utf8Length = str.length;
    for (var i = 0; i < str.length; ++i) {
      var utf16 = str.charCodeAt (i);
      if (utf16 >= 0xD800 && utf16 <= 0xDFFF) {
        // Lead surrogate code point
        --utf8Length;
        ++i;
      }
    }
    return utf8Length;
  }, value);
} /* jerry_get_utf8_string_length */

jerry_size_t
jerry_substring_to_utf8_char_buffer (const jerry_value_t value, /**< input string value */
                                     jerry_length_t start_pos, /**< position of the first character */
                                     jerry_length_t end_pos, /**< position of the last character */
                                     jerry_char_t *buffer_p, /**< [out] output characters buffer */
                                     jerry_size_t buffer_size) /**< size of output buffer */
{
  if (buffer_p == NULL || buffer_size == 0 || !jerry_value_is_string (value))
  {
    return 0;
  }
  return (jerry_size_t) EM_ASM_INT({
    var str = __jerry.get($0);
    // String.prototype.slice()'s beginIndex/endIndex arguments aren't
    // Unicode codepoint positions: surrogates are counted separately...
    var utf8Pos = 0;
    var utf16StartPos;
    var utf16EndPos = str.length;
    for (var i = 0; i < str.length; ++i) {
      if (utf8Pos === $1) {
        utf16StartPos = i;
      }
      ++utf8Pos;
      var utf16 = str.charCodeAt (i);
      if (utf16 >= 0xD800 && utf16 <= 0xDFFF) {
        // Lead surrogate code point
        ++i; // Skip over the trailing surrogate
      }
      if (utf8Pos === $2) {
        utf16EndPos = i;
        break;
      }
    }
    if (utf16StartPos === undefined) {
      return 0;
    }
    var substr = str.slice(utf16StartPos, utf16EndPos + 1);
    console.log(utf16StartPos, utf16EndPos, substr.length, str.length);
    return Module.stringToUTF8DataOnly(substr, $3, $4);
  }, value, start_pos, end_pos, buffer_p, buffer_size);
} /* jerry_substring_to_utf8_char_buffer */

jerry_value_t
jerry_create_string_from_utf8 (const jerry_char_t *str_p) /**< pointer to string */
{
  // Just call jerry_create_string_sz, it auto-detects UTF-8.
  return jerry_create_string_sz (str_p, strlen ((const char *)str_p));
} /* jerry_create_string_from_utf8 */

jerry_value_t jerry_create_string_sz(const jerry_char_t *str_p,
                                     jerry_size_t str_size) {
  if (!str_p)
  {
    return jerry_create_undefined();
  }
  return (jerry_value_t)EM_ASM_INT({
    // Auto-detects ASCII vs UTF-8:
    return __jerry.ref(Module.Pointer_stringify($0, $1));
  }, str_p, str_size);
}

jerry_value_t jerry_create_string(const jerry_char_t *str_p) {
  if (!str_p)
  {
    return jerry_create_undefined();
  }
  return jerry_create_string_sz(str_p, strlen((const char *)str_p));
}

jerry_size_t jerry_get_string_size(const jerry_value_t value) {
  if (!jerry_value_is_string(value))
  {
    return 0;
  }
  return (jerry_size_t)EM_ASM_INT({
    var str = __jerry.get($0);
    var cesu8Size = 0;
    for (var i = 0; i < str.length; ++i) {
      var utf16 = str.charCodeAt(i);
      if (utf16 <= 0x7F) {
        ++cesu8Size;
      } else if (utf16 <= 0x7FF) {
        cesu8Size += 2;
      } else if (utf16 <= 0xFFFF) {
        cesu8Size += 3;
      }
    }
    return cesu8Size;
  }, value);
}

void
jerry_register_magic_strings (const jerry_char_ptr_t *ex_str_items_p, /**< character arrays, representing
                                                                       *   external magic strings' contents */
                              uint32_t count, /**< number of the strings */
                              const jerry_length_t *str_lengths_p) /**< lengths of all strings */
{
  // No-op. This is part of an internal implementation detail to optimize string performance.
  JERRY_UNUSED (ex_str_items_p);
  JERRY_UNUSED (count);
  JERRY_UNUSED (str_lengths_p);
} /* jerry_register_magic_strings */

////////////////////////////////////////////////////////////////////////////////
// Functions for array object values
////////////////////////////////////////////////////////////////////////////////
uint32_t jerry_get_array_length(const jerry_value_t value) {
  if (!jerry_value_is_array(value))
  {
    return 0;
  }
  return (uint32_t)EM_ASM_INT({
      return __jerry.get($0).length;
    }, value);
}

////////////////////////////////////////////////////////////////////////////////
// Jerry Value Creation API
////////////////////////////////////////////////////////////////////////////////
jerry_value_t jerry_create_array(uint32_t size) {
  return (jerry_value_t)EM_ASM_INT({
      return __jerry.ref(new Array($0));
    }, size);
}

jerry_value_t jerry_create_boolean(bool value) {
  return (jerry_value_t)EM_ASM_INT({
      return __jerry.ref(Boolean($0));
    }, value);
}

jerry_value_t jerry_create_error(jerry_error_t error_type,
                                 const jerry_char_t *message_p) {
  return jerry_create_error_sz(error_type,
                               message_p, strlen((const char *)message_p));
}

#define JERRY_ERROR(type, msg, sz) (jerry_value_t)(EM_ASM_INT({ \
      return __jerry.ref(new (type)(Module.Pointer_stringify($0, $1))) \
    }, (msg), (sz)))

jerry_value_t jerry_create_error_sz(jerry_error_t error_type,
                                    const jerry_char_t *message_p,
                                    jerry_size_t message_size) {
  jerry_value_t error_ref = 0;
  switch (error_type) {
    case JERRY_ERROR_COMMON:
      error_ref = JERRY_ERROR(Error, message_p, message_size);
      break;
    case JERRY_ERROR_EVAL:
      error_ref = JERRY_ERROR(EvalError, message_p, message_size);
      break;
    case JERRY_ERROR_RANGE:
      error_ref = JERRY_ERROR(RangeError, message_p, message_size);
      break;
    case JERRY_ERROR_REFERENCE:
      error_ref = JERRY_ERROR(ReferenceError, message_p, message_size);
      break;
    case JERRY_ERROR_SYNTAX:
      error_ref = JERRY_ERROR(SyntaxError, message_p, message_size);
      break;
    case JERRY_ERROR_TYPE:
      error_ref = JERRY_ERROR(TypeError, message_p, message_size);
      break;
    case JERRY_ERROR_URI:
      error_ref = JERRY_ERROR(URIError, message_p, message_size);
      break;
    default:
      EM_ASM_INT({
          abort('Cannot create error type: ' + $0);
        }, error_type);
      break;
  }
  jerry_value_set_error_flag(&error_ref);
  return error_ref;
}

jerry_value_t jerry_create_external_function(jerry_external_handler_t handler_p) {
  return (jerry_value_t)EM_ASM_INT({
      return __jerry.create_external_function($0);
    }, handler_p);
}

jerry_value_t jerry_create_number(double value) {
  return (jerry_value_t)EM_ASM_INT({
      return __jerry.ref($0);
    }, value);
}

jerry_value_t jerry_create_number_infinity(bool negative) {
  return (jerry_value_t)EM_ASM_INT({
    return __jerry.ref($0 ? -Infinity : Infinity);
  }, negative);
}

jerry_value_t jerry_create_number_nan(void) {
  return (jerry_value_t)EM_ASM_INT_V({
    return __jerry.ref(NaN);
  });
}

jerry_value_t jerry_create_null(void) {
  return (jerry_value_t)EM_ASM_INT_V({
    return __jerry.ref(null);
  });
}

jerry_value_t jerry_create_object(void) {
  return (jerry_value_t)EM_ASM_INT_V({
    return __jerry.ref(new Object());
  });
}

jerry_value_t jerry_create_undefined(void) {
  return (jerry_value_t)EM_ASM_INT_V({
    return __jerry.ref(undefined);
  });
}

jerry_value_t jerry_create_promise (void) {
  EM_ASM({
             console.warn('jerry_set_vm_exec_stop_callback() is not implemented, ignoring the call.');
         });
  return jerry_create_undefined ();
}

////////////////////////////////////////////////////////////////////////////////
// General API Functions of JS Objects
////////////////////////////////////////////////////////////////////////////////

jerry_value_t
jerry_has_property (const jerry_value_t obj_val, /**< object value */
                    const jerry_value_t prop_name_val) /**< property name (string value) */
{
  if (!jerry_value_is_object(obj_val) ||
      !jerry_value_is_string(prop_name_val))
  {
    return false;
  }
  const bool has_property = (bool)EM_ASM_INT({
      var obj = __jerry.get($0);
      var name = __jerry.get($1);
      return (name in obj);
    }, obj_val, prop_name_val);
  return jerry_create_boolean (has_property);
} /* jerry_has_property */

jerry_value_t
jerry_has_own_property (const jerry_value_t obj_val, /**< object value */
                        const jerry_value_t prop_name_val) /**< property name (string value) */
{
  if (!jerry_value_is_object(obj_val) ||
      !jerry_value_is_string(prop_name_val))
  {
    return false;
  }
  const bool has_property = (bool)EM_ASM_INT({
      var obj = __jerry.get($0);
      var name = __jerry.get($1);
      return obj.hasOwnProperty(name);
    }, obj_val, prop_name_val);
  return jerry_create_boolean (has_property);
} /* jerry_has_own_property */

bool jerry_delete_property(const jerry_value_t obj_val,
                           const jerry_value_t prop_name_val) {
  if (!jerry_value_is_object(obj_val) ||
      !jerry_value_is_string(prop_name_val))
  {
    return false;
  }
  return (bool)EM_ASM_INT({
      var obj = __jerry.get($0);
      var name = __jerry.get($1);
      try {
        return delete obj[name];
      } catch (e) {
        // In strict mode, delete throws SyntaxError if the property is an
        // own non-configurable property.
        return false;
      }
      return true;
    }, obj_val, prop_name_val);
}

bool jerry_delete_property_by_index (const jerry_value_t obj_val, uint32_t index)
{
  EM_ASM({
             console.warn('jerry_delete_property_by_index() is not yet implemented.');
  });
  JERRY_UNUSED (obj_val);
  JERRY_UNUSED (index);
  return false;
}

jerry_value_t jerry_get_property(const jerry_value_t obj_val,
                                 const jerry_value_t prop_name_val) {
  if (!jerry_value_is_object(obj_val) ||
      !jerry_value_is_string(prop_name_val))
  {
    return TYPE_ERROR_ARG;
  }
  return (jerry_value_t)EM_ASM_INT({
      var obj = __jerry.get($0);
      var name = __jerry.get($1);
      return __jerry.ref(obj[name]);
    }, obj_val, prop_name_val);
}

jerry_value_t jerry_get_property_by_index(const jerry_value_t obj_val,
                                          uint32_t index) {
  if (!jerry_value_is_object(obj_val))
  {
    return TYPE_ERROR;
  }
  return (jerry_value_t)EM_ASM_INT({
      var obj = __jerry.get($0);
      return __jerry.ref(obj[$1]);
    }, obj_val, index);
}

jerry_value_t jerry_set_property(const jerry_value_t obj_val,
                                 const jerry_value_t prop_name_val,
                                 const jerry_value_t value_to_set) {
  if (jerry_value_has_error_flag(value_to_set) ||
      !jerry_value_is_object(obj_val) ||
      !jerry_value_is_string(prop_name_val))
  {
    return TYPE_ERROR_ARG;
  }
  return (jerry_value_t)EM_ASM_INT({
      var obj = __jerry.get($0);
      var name = __jerry.get($1);
      var to_set = __jerry.get($2);
      obj[name] = to_set;
      return __jerry.ref(true);
    }, obj_val, prop_name_val, value_to_set);
}

jerry_value_t jerry_set_property_by_index(const jerry_value_t obj_val,
                                          uint32_t index,
                                          const jerry_value_t value_to_set) {
  if (jerry_value_has_error_flag(value_to_set) ||
      !jerry_value_is_object(obj_val))
  {
    return TYPE_ERROR_ARG;
  }
  return (jerry_value_t)EM_ASM_INT({
      var obj = __jerry.get($0);
      var to_set = __jerry.get($2);
      obj[$1] = to_set;
      return __jerry.ref(true);
    }, obj_val, index, value_to_set);
}

void jerry_init_property_descriptor_fields(jerry_property_descriptor_t *prop_desc_p) {
  *prop_desc_p = (jerry_property_descriptor_t) {
    .value = jerry_create_undefined(),
    .getter = jerry_create_undefined(),
    .setter = jerry_create_undefined(),
  };
}

jerry_value_t jerry_define_own_property(const jerry_value_t obj_val,
                                        const jerry_value_t prop_name_val,
                                        const jerry_property_descriptor_t *pdp) {
  if (!jerry_value_is_object(obj_val) && !jerry_value_is_string(obj_val))
  {
    return TYPE_ERROR_ARG;
  }
  if ((pdp->is_writable_defined || pdp->is_value_defined)
      && (pdp->is_get_defined || pdp->is_set_defined))
  {
    return TYPE_ERROR_ARG;
  }
  if (pdp->is_get_defined && !jerry_value_is_function(pdp->getter))
  {
    return TYPE_ERROR_ARG;
  }
  if (pdp->is_set_defined && !jerry_value_is_function(pdp->setter))
  {
    return TYPE_ERROR_ARG;
  }

  return (jerry_value_t)(EM_ASM_INT({
      var obj = __jerry.get($12 /* obj_val */);
      var name = __jerry.get($13 /* prop_name_val */);
      var desc = {};
      if ($0 /* is_value_defined */)
      {
        desc.value = __jerry.get($9);
      }
      if ($1 /* is_get_defined */)
      {
        desc.get = __jerry.get($10);
      }
      if ($2 /* is_set_defined */)
      {
        desc.set = __jerry.get($11);
      }
      if ($3 /* is_writable_defined */)
      {
        desc.writable = Boolean($4 /* is_writable */);
      }
      if ($5 /* is_enumerable_defined */)
      {
        desc.enumerable = Boolean($6 /* is_enumerable */);
      }
      if ($7 /* is_configurable */)
      {
        desc.configurable = Boolean($8 /* is_configurable */);
      }

      Object.defineProperty(obj, name, desc);
      return __jerry.ref(Boolean(true));
    }, pdp->is_value_defined,    /* $0 */
       pdp->is_get_defined,      /* $1 */
       pdp->is_set_defined,      /* $2 */
       pdp->is_writable_defined, /* $3 */
       pdp->is_writable,         /* $4 */
       pdp->is_enumerable_defined,   /* $5 */
       pdp->is_enumerable,           /* $6 */
       pdp->is_configurable_defined, /* $7 */
       pdp->is_configurable,         /* $8 */
       pdp->value,   /* $9  */
       pdp->getter,  /* $10 */
       pdp->setter,  /* $11 */
       obj_val,      /* $12 */
       prop_name_val /* $13 */
    ));
}

bool
jerry_get_own_property_descriptor (const jerry_value_t  obj_val, /**< object value */
                                   const jerry_value_t prop_name_val, /**< property name (string value) */
                                   jerry_property_descriptor_t *prop_desc_p) /**< property descriptor */
{
  if (!jerry_value_is_object (obj_val)
      || !jerry_value_is_string (prop_name_val))
  {
    return false;
  }
  // Emscripten's setValue () only works with aligned accesses. The bool fields
  // of jerry_property_descriptor_t are not word aligned, so use word-sized temporary variables:
  int32_t is_configurable_defined;
  int32_t is_configurable;
  int32_t is_enumerable_defined;
  int32_t is_enumerable;
  int32_t is_writable_defined;
  int32_t is_writable;
  int32_t is_value_defined;
  jerry_value_t value;
  int32_t is_set_defined;
  jerry_value_t setter;
  int32_t is_get_defined;
  jerry_value_t getter;

  const bool success = (bool) EM_ASM_INT({
    try {
      var obj = __jerry.get($0);
      var propName = __jerry.get($1);
      var propDesc = Object.getOwnPropertyDescriptor(obj, propName);

      var assignFieldPair = function(fieldName, isDefinedTarget, valueTarget)
      {
        var isDefined = propDesc.hasOwnProperty(fieldName);
        setValue(isDefinedTarget, isDefined, 'i32*');
        if (isDefined)
        {
          var value = propDesc[fieldName];
          switch (fieldName) {
            case 'value':
              value = __jerry.ref(value);
              break;
            case 'set':
            case 'get':
              value = __jerry.ref(value ? value : null);
              break;
          }
          setValue(valueTarget, value, 'i32*');
        }
        else
        {
          setValue(valueTarget, __jerry.ref(undefined), 'i32*');
        }
      };
      assignFieldPair('configurable', $2, $3);
      assignFieldPair('enumerable', $4, $5);
      assignFieldPair('writable', $6, $7);
      assignFieldPair('value', $8, $9);
      assignFieldPair('set', $10, $11);
      assignFieldPair('get', $12, $13);
    }
    catch (e)
    {
      return false;
    }
    return true;
  }, /* $0: */ obj_val, /* $1: */ prop_name_val,
     /* $2: */ &is_configurable_defined, /* $3: */ &is_configurable,
     /* $4: */ &is_enumerable_defined, /* $5: */ &is_enumerable,
     /* $6: */ &is_writable_defined, /* $7: */ &is_writable,
     /* $8: */ &is_value_defined, /* $9: */ &value,
     /* $10: */ &is_set_defined, /* $11: */ &setter,
     /* $12: */ &is_get_defined, /* $13: */ &getter);

  if (success) {
    *prop_desc_p = (jerry_property_descriptor_t) {
        .is_configurable_defined = (bool) is_configurable_defined,
        .is_configurable = (bool) is_configurable,
        .is_enumerable_defined = (bool) is_enumerable_defined,
        .is_enumerable = (bool) is_enumerable,
        .is_value_defined = (bool) is_value_defined,
        .value = value,
        .is_set_defined = (bool) is_set_defined,
        .setter = setter,
        .is_get_defined = (bool) is_get_defined,
        .getter = getter,
    };
  }
  return success;
} /* jerry_get_own_property_descriptor */

void
jerry_free_property_descriptor_fields (const jerry_property_descriptor_t *prop_desc_p) /**< property descriptor */
{
  if (prop_desc_p->is_value_defined)
  {
    jerry_release_value (prop_desc_p->value);
  }

  if (prop_desc_p->is_get_defined)
  {
    jerry_release_value (prop_desc_p->getter);
  }

  if (prop_desc_p->is_set_defined)
  {
    jerry_release_value (prop_desc_p->setter);
  }
} /* jerry_free_property_descriptor_fields */


static jerry_value_t __attribute__((used))
_jerry_call_external_handler(jerry_external_handler_t func_obj_p,
                             const jerry_value_t func_obj_val,
                             const jerry_value_t this_val,
                             const jerry_value_t args_p[],
                             jerry_size_t args_count) {
  return (func_obj_p)(func_obj_val, this_val, args_p, args_count);
}

jerry_value_t jerry_call_function(const jerry_value_t func_obj_val,
                                  const jerry_value_t this_val,
                                  const jerry_value_t args_p[],
                                  jerry_size_t args_count) {
  if (!jerry_value_is_function(func_obj_val))
  {
    return TYPE_ERROR_ARG;
  }

  return (jerry_value_t)EM_ASM_INT({
        var func_obj = __jerry.get($0);
        var this_val = __jerry.get($1);
        var args = [];
        for (var i = 0; i < $3; ++i) {
          args.push(__jerry.get(getValue($2 + i*4, 'i32')));
        }
        try {
          var rv = func_obj.apply(this_val, args);
        } catch (e) {
          var error_ref = __jerry.ref(e);
          __jerry.setError(error_ref, true);
          return error_ref;
        }
        return __jerry.ref(rv);
    }, func_obj_val, this_val, args_p, args_count);
}

jerry_value_t jerry_construct_object(const jerry_value_t func_obj_val,
                                     const jerry_value_t args_p[],
                                     jerry_size_t args_count) {
  if (!jerry_value_is_constructor(func_obj_val))
  {
    return TYPE_ERROR_ARG;
  }
  return (jerry_value_t)EM_ASM_INT({
    var constructor = __jerry.get($0);
    var args = [];
    for (var i = 0; i < $2; ++i) {
      args.push(__jerry.get(getValue($1 + (i * 4 /* sizeof(i32) */), 'i32')));
    }
    // Call the constructor with new object as `this`
    var bindArgs = [null].concat(args);
    var boundConstructor = constructor.bind.apply(constructor, bindArgs);
    try {
      var rv = new boundConstructor ();
    } catch (e) {
      var error_ref = __jerry.ref(e);
      __jerry.setError(error_ref, true);
      return error_ref;
    }
    return __jerry.ref(rv);
  }, func_obj_val, args_p, args_count);
}

jerry_length_t
jerry_get_string_length (const jerry_value_t value) /**< input string */
{
  if (!jerry_value_is_string (value))
  {
    return 0;
  }

  return (jerry_length_t) EM_ASM_INT({
    var str = __jerry.get($0);
    return str.length;
  }, value);
} /* jerry_get_string_length */

jerry_size_t jerry_string_to_char_buffer(const jerry_value_t value,
                                         jerry_char_t *buffer_p,
                                         jerry_size_t buffer_size) {
  const jerry_size_t str_size = jerry_get_string_size(value);
  if (str_size == 0 || buffer_size < str_size || buffer_p == NULL)
  {
    return 0;
  }
  return (jerry_size_t) EM_ASM_INT({
    var str = __jerry.get($0);
    return Module.stringToCESU8DataOnly(str, $1, $2);
  }, value, buffer_p, buffer_size);
}

jerry_size_t jerry_object_to_string_to_utf8_char_buffer(const jerry_value_t object,
                                                        jerry_char_t *buffer_p,
                                                        jerry_size_t buffer_size) {
  jerry_value_t str_ref = (jerry_value_t)EM_ASM_INT({
    var str = __jerry.ref(String(__jerry.get($0)));
    return str;
  }, object);
  jerry_size_t len = jerry_string_to_utf8_char_buffer(str_ref, buffer_p, buffer_size);
  jerry_release_value(str_ref);

  return len;
}

// FIXME: Propery do CESU-8 => UTF-8 conversion.
jerry_size_t jerry_object_to_string_to_char_buffer(const jerry_value_t object,
                                                   jerry_char_t *buffer_p,
                                                   jerry_size_t buffer_size) {
  return jerry_object_to_string_to_utf8_char_buffer(object,
                                                    buffer_p,
                                                    buffer_size);
}

jerry_value_t jerry_get_object_keys(const jerry_value_t value) {
  if (!jerry_value_is_object(value))
  {
    return TYPE_ERROR_ARG;
  }
  return (jerry_value_t)EM_ASM_INT({
      return __jerry.ref(Object.keys(__jerry.get($0)));
    }, value);
}

jerry_value_t jerry_get_prototype(const jerry_value_t value) {
  if (!jerry_value_is_object(value))
  {
    return TYPE_ERROR_ARG;
  }
  return (jerry_value_t)EM_ASM_INT({
    if (!__jerry.hasProto)
    {
      throw new Error('Not implemented, host engine does not implement __proto__.');
    }
    return __jerry.ref(__jerry.get($0).__proto__);
  }, value);
}

jerry_value_t jerry_set_prototype(const jerry_value_t obj_val,
                                  const jerry_value_t proto_obj_val) {
  EM_ASM_ARGS({
    if (!__jerry.hasProto)
    {
      throw new Error('Not implemented, host engine does not implement __proto__.');
    }
    var obj = __jerry.get($0);
    var proto = __jerry.get($1);
    obj.__proto__ = proto;
  }, obj_val, proto_obj_val);
  return jerry_create_boolean (true);
}

bool jerry_get_object_native_handle(const jerry_value_t obj_val,
                                    uintptr_t *out_handle_p) {
  if (!jerry_value_is_object (obj_val))
  {
    return false;
  }
  return (bool) EM_ASM_INT({
    var value = __jerry.get ($0);
    var internalProps = value.__jerryInternalProps;
    var handle = internalProps.nativeHandle;
    if (handle === undefined)
    {
      return false;
    }
    if ($1)
    {
      Module.setValue ($1, handle, '*');
    }
    return true;
  }, obj_val, out_handle_p);
}

void jerry_set_object_native_handle(const jerry_value_t obj_val,
                                    uintptr_t handle_p,
                                    jerry_object_free_callback_t freecb_p) {
  if (jerry_value_is_object (obj_val))
  {
    EM_ASM_INT({
      var value = __jerry.get ($0);
      var internalProps = value.__jerryInternalProps;
      internalProps.nativeHandle = $1;
      internalProps.nativeHandleFreeCb = $2;
    }, obj_val, handle_p, freecb_p);
  }
}

void
jerry_set_object_native_pointer (const jerry_value_t obj_val, /**< object to set native pointer in */
                                 void *native_pointer_p, /**< native pointer */
                                 const jerry_object_native_info_t *native_info_p) /**< object's native type info */
{
  if (jerry_value_is_object (obj_val))
  {
    EM_ASM_INT({
      var value = __jerry.get ($0);
      var internalProps = value.__jerryInternalProps;
      internalProps.nativePtr = $1;
      internalProps.nativeInfo = $2;
    }, obj_val, native_pointer_p, native_info_p);
  }
} /* jerry_set_object_native_pointer */

bool
jerry_get_object_native_pointer (const jerry_value_t obj_val, /**< object to get native pointer from */
                                 void **out_native_pointer_p, /**< [out] native pointer */
                                 const jerry_object_native_info_t **out_native_info_p) /**< [out] the type info
                                                                                        *   of the native pointer */
{
  if (!jerry_value_is_object (obj_val))
  {
    return false;
  }
  return (bool) EM_ASM_INT({
    var value = __jerry.get ($0);
    var internalProps = value.__jerryInternalProps;
    var ptr = internalProps.nativePtr;
    if (ptr === undefined)
    {
      return false;
    }
    if ($1)
    {
      Module.setValue ($1, ptr, '*');
    }
    if ($2)
    {
      Module.setValue ($2, internalProps.nativeInfo, '*');
    }
    return true;
  }, obj_val, out_native_pointer_p, out_native_info_p);
} /* jerry_get_object_native_pointer */

static bool __attribute__((used))
_jerry_call_foreach_cb(jerry_object_property_foreach_t foreach_p,
                       const jerry_value_t property_name,
                       const jerry_value_t property_value,
                       void *user_data_p)
{
  return foreach_p(property_name, property_value, user_data_p);
}

bool
jerry_foreach_object_property (const jerry_value_t obj_val, /**< object value */
                               jerry_object_property_foreach_t foreach_p, /**< foreach function */
                               void *user_data_p) /**< user data for foreach function */
{
  return (bool) EM_ASM_INT ({
    var obj = __jerry.get($0);
    try {
      for (var propName in obj)
      {
        var propNameRef = __jerry.ref(propName);
        var propValRef = __jerry.ref(obj[propName]);
        var shouldContinue = Module.ccall(
          '_jerry_call_foreach_cb',
          'number',
          ['number', 'number'],
          [$1, propNameRef, propValRef, $2]);
        if (!shouldContinue) {
          return true;
        }
      }
    }
    catch (e)
    {
      return false;
    };
    return true;
  }, obj_val, foreach_p, user_data_p);
} /* jerry_foreach_object_property */

jerry_value_t jerry_resolve_or_reject_promise (jerry_value_t promise, jerry_value_t argument, bool is_resolve)
{
  EM_ASM({
             console.warn('jerry_resolve_or_reject_promise() is not yet implemented');
         });
  JERRY_UNUSED (promise);
  JERRY_UNUSED (argument);
  JERRY_UNUSED (is_resolve);
  return jerry_create_undefined ();
}

static void __attribute__((used))
_jerry_call_native_object_free_callbacks(const jerry_object_native_info_t *native_info_p,
                                         void *native_pointer_p,
                                         jerry_object_free_callback_t native_handle_freecb_p,
                                         uintptr_t native_handle) {
  if (native_info_p && native_info_p->free_cb)
  {
    native_info_p->free_cb(native_pointer_p);
  }
  if (native_handle_freecb_p)
  {
    native_handle_freecb_p(native_handle);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Error flag manipulation functions
////////////////////////////////////////////////////////////////////////////////
//
// The error flag is stored alongside the value in __jerry.
// This allows for us to keep a valid value, like jerryscript does, and be able
// to add / remove a flag specifying whether there was an error or not.

bool jerry_value_has_error_flag(const jerry_value_t value) {
  return (bool)(EM_ASM_INT({
      return __jerry.getError($0);
    }, value));
}

void jerry_value_clear_error_flag(jerry_value_t *value_p) {
  EM_ASM_INT({
      return __jerry.setError($0, false);
    }, *value_p);
}

void jerry_value_set_error_flag(jerry_value_t *value_p) {
  EM_ASM_INT({
      return __jerry.setError($0, true);
    }, *value_p);
}
////////////////////////////////////////////////////////////////////////////////
// Converters of `jerry_value_t`
////////////////////////////////////////////////////////////////////////////////

bool jerry_value_to_boolean(const jerry_value_t value) {
  if (jerry_value_has_error_flag(value))
  {
    return false;
  }
  return (bool)EM_ASM_INT({
      return Boolean(__jerry.get($0));
    }, value);
}



jerry_value_t jerry_value_to_number(const jerry_value_t value) {
  if (jerry_value_has_error_flag(value))
  {
    return TYPE_ERROR_FLAG;
  }
  return (jerry_value_t)EM_ASM_INT({
      return __jerry.ref(Number(__jerry.get($0)));
    }, value);
}

jerry_value_t jerry_value_to_object(const jerry_value_t value) {
  if (jerry_value_has_error_flag(value))
  {
    return TYPE_ERROR_FLAG;
  }
  return (jerry_value_t)EM_ASM_INT({
      return __jerry.ref(new Object(__jerry.get($0)));
    }, value);
}

jerry_value_t jerry_value_to_primitive(const jerry_value_t value) {
  if (jerry_value_has_error_flag(value))
  {
    return TYPE_ERROR_FLAG;
  }
  return (jerry_value_t)EM_ASM_INT({
      var val = __jerry.get($0);
      var rv;
      if ((typeof val === 'object' && val != null)
          || (typeof val === 'function'))
      {
        rv = val.valueOf(); // unbox
      }
      else
      {
        rv = val; // already a primitive
      }
      return __jerry.ref(rv);
    }, value);
}

jerry_value_t jerry_value_to_string(const jerry_value_t value) {
  if (jerry_value_has_error_flag(value))
  {
    return TYPE_ERROR_FLAG;
  }
  return (jerry_value_t)EM_ASM_INT({
      return __jerry.ref(String(__jerry.get($0)));
    }, value);
}

int jerry_obj_refcount(jerry_value_t o) {
  return EM_ASM_INT({
    try {
      return __jerry.getRefCount($0);
    } catch (e) {
      return 0;
    }
  }, o);
}

void jerry_get_memory_limits(size_t *out_data_bss_brk_limit_p,
                             size_t *out_stack_limit_p) {
  *out_data_bss_brk_limit_p = 0;
  *out_stack_limit_p = 0;
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

void jerry_init(jerry_init_flag_t flags) {
  EM_ASM(__jerry.reset());
  (void)flags;
}

void jerry_cleanup(void) {
}

void *
jerry_get_context_data (const jerry_context_data_manager_t *manager_p)
{
  EM_ASM({
             console.warn('jerry_get_context_data() is not yet implemented.');
         });
  JERRY_UNUSED (manager_p);
  return NULL;
}

bool jerry_is_feature_enabled (const jerry_feature_t feature)
{
  switch (feature) {
    case JERRY_FEATURE_ERROR_MESSAGES:
    case JERRY_FEATURE_JS_PARSER:
      return true;
    case JERRY_FEATURE_CPOINTER_32_BIT:
    case JERRY_FEATURE_DEBUGGER:
    case JERRY_FEATURE_MEM_STATS:
    case JERRY_FEATURE_PARSER_DUMP:
    case JERRY_FEATURE_REGEXP_DUMP:
    case JERRY_FEATURE_SNAPSHOT_EXEC:
    case JERRY_FEATURE_SNAPSHOT_SAVE:
    case JERRY_FEATURE_VM_EXEC_STOP:
    case JERRY_FEATURE__COUNT:
      return false;
  }
} /* jerry_is_feature_enabled */

void
jerry_set_vm_exec_stop_callback (jerry_vm_exec_stop_callback_t stop_cb, /**< periodically called user function */
                                 void *user_p, /**< pointer passed to the function */
                                 uint32_t frequency) /**< frequency of the function call */
{
  EM_ASM({
    console.warn('jerry_set_vm_exec_stop_callback() is not implemented, ignoring the call.');
  });
  JERRY_UNUSED (stop_cb);
  JERRY_UNUSED (user_p);
  JERRY_UNUSED (frequency);
} /* jerry_set_vm_exec_stop_callback */
