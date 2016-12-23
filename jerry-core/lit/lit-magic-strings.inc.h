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

/*
 * List of ECMA magic strings
 *
 * These strings must be ascii strings and needs to be defined in size order
 * then by lexicographical order. The NULL character cannot be part of magic
 * strings, because it must be the terminator character of all magic strings.
 */

LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (0, LIT_MAGIC_STRING__EMPTY)
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING__EMPTY, "")
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (1, LIT_MAGIC_STRING_NEW_LINE_CHAR)
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_NEW_LINE_CHAR, "\n")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SPACE_CHAR, " ")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_DOUBLE_QUOTE_CHAR,"\"")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_LEFT_PARENTHESIS_CHAR, "(")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_RIGHT_PARENTHESIS_CHAR, ")")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_COMMA_CHAR, ",")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_MINUS_CHAR, "-")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_DOT_CHAR, ".")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SLASH_CHAR, "/")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_COLON_CHAR, ":")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_E_U, "E")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_TIME_SEP_U, "T")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_Z_CHAR, "Z")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_LEFT_SQUARE_CHAR, "[")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_BACKSLASH_CHAR, "\\")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_RIGHT_SQUARE_CHAR, "]")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_G_CHAR, "g")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_I_CHAR, "i")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_M_CHAR, "m")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_LEFT_BRACE_CHAR, "{")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_RIGHT_BRACE_CHAR, "}")
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (2, LIT_MAGIC_STRING_PI_U)
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_PI_U, "PI")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_OF, "of")
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (3, LIT_MAGIC_STRING_LN2_U)
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_LN2_U, "LN2")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_NAN, "NaN")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_UTC_U, "UTC")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_ABS, "abs")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_COS, "cos")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_EXP, "exp")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_GET, "get")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_LOG, "log")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_MAP, "map")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_MAX, "max")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_MIN, "min")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_NOW, "now")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_POP, "pop")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_POW, "pow")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SET, "set")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SIN, "sin")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_TAN, "tan")
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (4, LIT_MAGIC_STRING_EMPTY_NON_CAPTURE_GROUP)
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_EMPTY_NON_CAPTURE_GROUP, "(?:)")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_DATE_UL, "Date")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_JSON_U, "JSON")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_LN10_U, "LN10")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_MATH_UL, "Math")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_NULL_UL, "Null")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_ACOS, "acos")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_ASIN, "asin")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_ATAN, "atan")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_BIND, "bind")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_CALL, "call")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_CEIL, "ceil")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_EVAL, "eval")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_EXEC, "exec")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_FROM, "from")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_JOIN, "join")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_KEYS, "keys")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_NAME, "name")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_NULL, "null")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_PUSH, "push")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SEAL, "seal")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SOME, "some")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SORT, "sort")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SQRT, "sqrt")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_TEST, "test")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_TRIM, "trim")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_TRUE, "true")
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (5, LIT_MAGIC_STRING_ARRAY_UL)
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_ARRAY_UL, "Array")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_ERROR_UL, "Error")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_JERRY_UL, "Jerry")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_LOG2E_U, "LOG2E")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SQRT2_U, "SQRT2")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_APPLY, "apply")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_ATAN2, "atan2")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_EVERY, "every")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_FALSE, "false")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_FLOOR, "floor")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_INDEX, "index")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_INPUT, "input")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_IS_NAN, "isNaN")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_MATCH, "match")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_PARSE, "parse")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_PRINT, "print")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_ROUND, "round")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SHIFT, "shift")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SLICE, "slice")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SPLIT, "split")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_VALUE, "value")
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (6, LIT_MAGIC_STRING_LOG10E_U)
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_LOG10E_U, "LOG10E")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_NUMBER_UL, "Number")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_OBJECT_UL, "Object")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_REGEXP_UL, "RegExp")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_REGEXP_SOURCE_UL, "Source")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_STRING_UL, "String")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_BUFFER, "buffer")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_CALLEE, "callee")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_CALLER, "caller")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_CHAR_AT_UL, "charAt")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_CONCAT, "concat")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_CREATE, "create")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_ESCAPE, "escape")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_FILTER, "filter")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_FREEZE, "freeze")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_GET_DAY_UL, "getDay")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_GLOBAL, "global")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_IS_VIEW_UL, "isView")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_LENGTH, "length")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_NUMBER, "number")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_OBJECT, "object")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_RANDOM, "random")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_REDUCE, "reduce")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SEARCH, "search")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SOURCE, "source")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SPLICE, "splice")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_STRING, "string")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SUBSTR, "substr")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_TO_JSON_UL, "toJSON")
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (7, LIT_MAGIC_STRING_BOOLEAN_UL)
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_BOOLEAN_UL, "Boolean")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SQRT1_2_U, "SQRT1_2")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_BOOLEAN, "boolean")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_COMPILE, "compile")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_FOR_EACH_UL, "forEach")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_GET_DATE_UL, "getDate")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_GET_TIME_UL, "getTime")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_GET_YEAR_UL, "getYear")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_INDEX_OF_UL, "indexOf")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_IS_ARRAY_UL, "isArray")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_MESSAGE, "message")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_REPLACE, "replace")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_REVERSE, "reverse")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SET_DATE_UL, "setDate")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SET_TIME_UL, "setTime")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SET_YEAR_UL, "setYear")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_TO_FIXED_UL, "toFixed")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_UNSHIFT, "unshift")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_VALUE_OF_UL, "valueOf")
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (8, LIT_MAGIC_STRING_FUNCTION_UL)
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_FUNCTION_UL, "Function")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_INFINITY_UL, "Infinity")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_URI_ERROR_UL, "URIError")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_FUNCTION, "function")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_GET_HOURS_UL, "getHours")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_GET_MONTH_UL, "getMonth")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_IS_FINITE, "isFinite")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_IS_FROZEN_UL, "isFrozen")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_IS_SEALED_UL, "isSealed")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_PARSE_INT, "parseInt")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SET_HOURS_UL, "setHours")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SET_MONTH_UL, "setMonth")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_TO_STRING_UL, "toString")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_UNESCAPE, "unescape")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_WRITABLE, "writable")
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (9, LIT_MAGIC_STRING_NEGATIVE_INFINITY_UL)
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_NEGATIVE_INFINITY_UL, "-Infinity")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_ARGUMENTS_UL, "Arguments")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_EVAL_ERROR_UL, "EvalError")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_INT8_ARRAY_UL, "Int8Array")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_MAX_VALUE_U, "MAX_VALUE")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_MIN_VALUE_U, "MIN_VALUE")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_TYPE_ERROR_UL, "TypeError")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_UNDEFINED_UL, "Undefined")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_ARGUMENTS, "arguments")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_DECODE_URI, "decodeURI")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_ENCODE_URI, "encodeURI")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_GET_UTC_DAY_UL, "getUTCDay")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_LASTINDEX_UL, "lastIndex")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_MULTILINE, "multiline")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_PROTOTYPE, "prototype")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_STRINGIFY, "stringify")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SUBSTRING, "substring")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_UNDEFINED, "undefined")
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (10, LIT_MAGIC_STRING_INT16_ARRAY_UL)
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_INT16_ARRAY_UL, "Int16Array")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_INT32_ARRAY_UL, "Int32Array")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_RANGE_ERROR_UL, "RangeError")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_UINT8_ARRAY_UL, "Uint8Array")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_TYPED_ARRAY_UL, "TypedArray")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_BYTE_LENGTH_UL, "byteLength")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_BYTE_OFFSET_UL, "byteOffset")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_CHAR_CODE_AT_UL, "charCodeAt")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_ENUMERABLE, "enumerable")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_GET_MINUTES_UL, "getMinutes")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_GET_SECONDS_UL, "getSeconds")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_GET_UTC_DATE_UL, "getUTCDate")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_IGNORECASE_UL, "ignoreCase")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_PARSE_FLOAT, "parseFloat")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SET_MINUTES_UL, "setMinutes")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SET_SECONDS_UL, "setSeconds")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SET_UTC_DATE_UL, "setUTCDate")
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (11, LIT_MAGIC_STRING_ARRAY_BUFFER_UL)
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_ARRAY_BUFFER_UL, "ArrayBuffer")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SYNTAX_ERROR_UL, "SyntaxError")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_UINT16_ARRAY_UL, "Uint16Array")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_UINT32_ARRAY_UL, "Uint32Array")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_CONSTRUCTOR, "constructor")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_GET_FULL_YEAR_UL, "getFullYear")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_GET_UTC_HOURS_UL, "getUTCHours")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_GET_UTC_MONTH_UL, "getUTCMonth")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_LAST_INDEX_OF_UL, "lastIndexOf")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_REDUCE_RIGHT_UL, "reduceRight")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SET_FULL_YEAR_UL, "setFullYear")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SET_UTC_HOURS_UL, "setUTCHours")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SET_UTC_MONTH_UL, "setUTCMonth")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_TO_GMT_STRING_UL, "toGMTString")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_TO_ISO_STRING_UL, "toISOString")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_TO_LOWER_CASE_UL, "toLowerCase")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_TO_PRECISION_UL, "toPrecision")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_TO_UTC_STRING_UL, "toUTCString")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_TO_UPPER_CASE_UL, "toUpperCase")
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (12, LIT_MAGIC_STRING_FLOAT32_ARRAY_UL)
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_FLOAT32_ARRAY_UL, "Float32Array")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_FLOAT64_ARRAY_UL, "Float64Array")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_INVALID_DATE_UL, "Invalid Date")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_CONFIGURABLE, "configurable")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_FROM_CHAR_CODE_UL, "fromCharCode")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_IS_EXTENSIBLE, "isExtensible")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_TO_DATE_STRING_UL, "toDateString")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_TO_TIME_STRING_UL, "toTimeString")
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (13, LIT_MAGIC_STRING_GET_UTC_MINUTES_UL)
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_GET_UTC_MINUTES_UL, "getUTCMinutes")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_GET_UTC_SECONDS_UL, "getUTCSeconds")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_IS_PROTOTYPE_OF_UL, "isPrototypeOf")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_LOCALE_COMPARE_UL, "localeCompare")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SET_UTC_MINUTES_UL, "setUTCMinutes")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SET_UTC_SECONDS_UL, "setUTCSeconds")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_TO_EXPONENTIAL_UL, "toExponential")
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (14, LIT_MAGIC_STRING_REFERENCE_ERROR_UL)
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_REFERENCE_ERROR_UL, "ReferenceError")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_DEFINE_PROPERTY_UL, "defineProperty")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_GET_PROTOTYPE_OF_UL, "getPrototypeOf")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_GET_UTC_FULL_YEAR_UL, "getUTCFullYear")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_HAS_OWN_PROPERTY_UL, "hasOwnProperty")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SET_UTC_FULL_YEAR_UL, "setUTCFullYear")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_TO_LOCALE_STRING_UL, "toLocaleString")
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (15, LIT_MAGIC_STRING_GET_MILLISECONDS_UL)
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_GET_MILLISECONDS_UL, "getMilliseconds")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SET_MILLISECONDS_UL, "setMilliseconds")
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (16, LIT_MAGIC_STRING_DEFINE_PROPERTIES_UL)
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_DEFINE_PROPERTIES_UL, "defineProperties")
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (17, LIT_MAGIC_STRING_BYTES_PER_ELEMENT_U)
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_BYTES_PER_ELEMENT_U, "BYTES_PER_ELEMENT")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_NEGATIVE_INFINITY_U, "NEGATIVE_INFINITY")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_POSITIVE_INFINITY_U, "POSITIVE_INFINITY")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_UINT8_CLAMPED_ARRAY_UL, "Uint8ClampedArray")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_GET_TIMEZONE_OFFSET_UL, "getTimezoneOffset")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_PREVENT_EXTENSIONS_UL, "preventExtensions")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_TO_LOCALE_LOWER_CASE_UL, "toLocaleLowerCase")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_TO_LOCALE_UPPER_CASE_UL, "toLocaleUpperCase")
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (18, LIT_MAGIC_STRING_DECODE_URI_COMPONENT)
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_DECODE_URI_COMPONENT, "decodeURIComponent")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_ENCODE_URI_COMPONENT, "encodeURIComponent")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_GET_UTC_MILLISECONDS_UL, "getUTCMilliseconds")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_SET_UTC_MILLISECONDS_UL, "setUTCMilliseconds")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_TO_LOCALE_DATE_STRING_UL, "toLocaleDateString")
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_TO_LOCALE_TIME_STRING_UL, "toLocaleTimeString")
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (19, LIT_MAGIC_STRING_GET_OWN_PROPERTY_NAMES_UL)
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_GET_OWN_PROPERTY_NAMES_UL, "getOwnPropertyNames")
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (20, LIT_MAGIC_STRING_PROPERTY_IS_ENUMERABLE_UL)
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_PROPERTY_IS_ENUMERABLE_UL, "propertyIsEnumerable")
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (21, LIT_MAGIC_STRING_GET_OWN_PROPERTY_DESCRIPTOR_UL)
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (22, LIT_MAGIC_STRING_GET_OWN_PROPERTY_DESCRIPTOR_UL)
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (23, LIT_MAGIC_STRING_GET_OWN_PROPERTY_DESCRIPTOR_UL)
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (24, LIT_MAGIC_STRING_GET_OWN_PROPERTY_DESCRIPTOR_UL)
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING_GET_OWN_PROPERTY_DESCRIPTOR_UL, "getOwnPropertyDescriptor")
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (25, LIT_MAGIC_STRING__FUNCTION_TO_STRING)
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (26, LIT_MAGIC_STRING__FUNCTION_TO_STRING)
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (27, LIT_MAGIC_STRING__FUNCTION_TO_STRING)
LIT_MAGIC_STRING_FIRST_STRING_WITH_SIZE (28, LIT_MAGIC_STRING__FUNCTION_TO_STRING)
LIT_MAGIC_STRING_DEF (LIT_MAGIC_STRING__FUNCTION_TO_STRING, "function(){/* ecmascript */}")
