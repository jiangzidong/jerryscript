#include "jerry.h"
#include "main-101.h"
#include "jerry-core/jerry-api.h"
#include "jerry-core/jerry-port.h"


int jerry_port_putchar (int c)
{
  return 1;
}

#define JERRY_MCU_SCRIPT "var a = 111; a = a + 2; print('hello'); print(a);";

static const char generated_source[] = JERRY_MCU_SCRIPT;

int
jerry_main (void)
{
  const char *source_p = generated_source;
  const size_t source_size = sizeof (generated_source);

  jerry_completion_code_t ret_code = jerry_run_simple ((jerry_api_char_t *) source_p, source_size, JERRY_FLAG_EMPTY);

  if (ret_code == JERRY_COMPLETION_CODE_OK)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}