/*
 * driver - driver for loading mruby source code
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/irep.h"
#include "mruby/dump.h"
#include "mruby/proc.h"

/* The generated mruby bytecodes are stored in this array */
extern const uint8_t app_irep[];

#ifdef HAS_REQUIRE
void mrb_enable_require(mrb_state *mrb);
#endif

/*
 * Print levels:
 * 0 - Do not print anything
 * 1 - Print errors only
 * 2 - Print errors and results
 */
static int check_and_print_errors(mrb_state* mrb, mrb_value result,
                                  int print_level)
{
  if (mrb->exc && (print_level > 0)) {
    mrb_p(mrb, mrb_obj_value(mrb->exc));
    mrb->exc = 0;
    return 1;
  }

  if (print_level > 1) {
    mrb_p(mrb, result);
  }
  return 0;
}

int webruby_internal_run_bytecode(mrb_state* mrb, const uint8_t *bc,
                                  int print_level)
{
  return check_and_print_errors(mrb, mrb_load_irep(mrb, bc), print_level);
}

int webruby_internal_run(mrb_state* mrb, int print_level)
{
  return webruby_internal_run_bytecode(mrb, app_irep, print_level);
}

int webruby_internal_run_source(mrb_state* mrb, const char *s, int print_level)
{
  return check_and_print_errors(mrb, mrb_load_string(mrb, s), print_level);
}

int webruby_internal_compile(mrb_state* mrb, const char *file_name, const char *code, int print_level)
{
  mrb_value result;
  int ret = -1;
  int debuginfo = 0;
  mrb_irep *irep;
  mrbc_context *cxt;

  cxt = mrbc_context_new(mrb);
  cxt->no_exec = 1;

  result = mrb_load_nstring_cxt(mrb, code, strlen(code), cxt);
  if (mrb_undef_p(result)) {
    mrbc_context_free(mrb, cxt);
    mrb_close(mrb);
    return MRB_DUMP_GENERAL_FAILURE;
  }

  FILE* file = fopen(file_name, "wb");

  irep = mrb_proc_ptr(result)->body.irep;
  ret = mrb_dump_irep_binary(mrb, irep, debuginfo, file);

  mrbc_context_free(mrb, cxt);
  fclose(file);

  return ret;
}

int webruby_internal_setup(mrb_state* mrb)
{
#ifdef HAS_REQUIRE
  mrb_enable_require(mrb);
#endif
  return 0;
}
