//
// C main.c file (main application file)
// for the Tiny C Compiler (NWA) app for the Numwoks calculators
//

#include "crt_stubs.h"
#include "tcc_stubs.h"

#include <stdlib.h> // FIXME: it's not available for the NumWorks, innit?
#include <stdio.h> // FIXME: it's not available for the NumWorks, innit?
#include <stdint.h> // FIXME: it's not available for the NumWorks, innit?
#include <string.h> // FIXME: it's not available for the NumWorks, innit?

void handle_error(void *opaque, const char *msg) {
    fprintf(opaque, "%s\n", msg);
}

#include <eadk.h>

// TODO: write a wrapper file/lib so that the C code interpreted on the NumWorks has correct access to the EADK lib!
// #include "eadk_lib.h"

// this function is called by the generated code
int add(int a, int b) {
    return a + b;
}

// this function is opened to the generated code
void eadk_timing_msleep_int(int ms) {
  return eadk_timing_msleep((uint32_t) ms);
}

// this string is referenced by the generated code
const char hello[] = "Hello World (from TCC)!";

// this long string is the default program to be run if nothing is read from 'tcc.py'
char long_test_program[] =
//"#include <tcclib.h>\n" /* include the "Simple libc header for TCC" */
"extern int add(int a, int b);\n"
"extern void eadk_timing_msleep_int(int ms);\n"
"extern const char hello[];\n"
"int fib(int n) {\n"
"    if (n <= 2) {\n"
"        return 1;\n"
"    } else {\n"
"        return fib(n-1) + fib(n-2);\n"
"    }\n"
"}\n"
"\n"
"int main(int argc, char** argv) {\n"
"    printf(\"%s\\n\", hello);\n"
"    eadk_timing_msleep_int(1000);\n"
"    int n = 2; // Default value.\n"
"    printf(\"fib(%d) = %d\\n\", n, fib(n));\n"
"    eadk_timing_msleep_int(1000);\n"
"    printf(\"add(%d, %d) = %d\\n\", n, 2 * n, add(n, 2 * n));\n"
"    eadk_timing_msleep_int(1000);\n"
"    return 0;\n"
"}\n";

// this long string is the default program to be run if nothing is read from 'tcc.py'
char default_program[] =
//"#include <tcclib.h>\n" /* include the "Simple libc header for TCC" */
"extern int add(int a, int b);\n"
"extern void eadk_timing_msleep_int(int ms);\n"
"extern const char hello[];\n"
"\n"
"int main(int argc, char** argv) {\n"
"    printf(\"%s\\n\", hello);\n"
"    return 0;\n"
"}\n";


#include "storage.h"

#include "libtcc.h"

const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Tiny C Compiler";
const uint32_t eadk_api_level  __attribute__((section(".rodata.eadk_api_level"))) = 0;

// TODO: Check why __exidx_start/__exidx_end is needed
void __exidx_start() { }
void __exidx_end() { }


// int main() {
int main(int argc, char ** argv) {

  printf("Tiny C Compiler v0.0.1\n");
  eadk_timing_msleep(1000);

  printf("Reading from 'tcc.py' file...\n");
  eadk_timing_msleep(1000);

  // We read "tcc.py"
  size_t file_len = 0;
  const char * code_from_file = extapp_fileRead("tcc.py", &file_len);

  // DONE: I wasn't able to compile while depending on external data, but it works if reading from a local 'tcc.py' file.
  // const char * code = eadk_external_data;

  const char * code = (code_from_file == NULL && file_len <= 0) ? "printf(\"\\nHi from C interpreter! sleep(3s)\")\neadk_timing_msleep_int(3000)\nreturn 0;" : (code_from_file + 1);

  // From https://github.com/Tiny-C-Compiler/tinycc-mirror-repository/blob/mob/tests/libtcc_test.c
  TCCState *tcc_state;
  int (*func_main_our_code)(int);

  tcc_state = tcc_new();
  if (!tcc_state) {
    fprintf(stderr, "ERR: failed create TCC state\n");
    tcc_delete(tcc_state); // delete the state
    eadk_timing_msleep(2000);
    return 1;
  }

  /* set custom error/warning printer */
  tcc_set_error_func(tcc_state, stderr, handle_error);

  // Getting ready to execute the code
  printf("Executing code...\n");
  eadk_timing_msleep(1000);

  // MUST BE CALLED before any compilation
  // the output type is in memory, not on a file
  tcc_set_output_type(tcc_state, TCC_OUTPUT_MEMORY);

  // TODO: first test a tiny C code, then more!
  // const char * code_to_execute = default_program;
  const char * code_to_execute = code;

  if (tcc_compile_string(tcc_state, code_to_execute) == -1) {
    fprintf(stderr, "ERR: couldn't compile\n");
    tcc_delete(tcc_state); // delete the state
    eadk_timing_msleep(2000);
    return 1;
  }

  // as a test, we add symbols that the compiled program can use.
  // You may also open a dll with tcc_add_dll() and use symbols from that
  tcc_add_symbol(tcc_state, "add", add);
  tcc_add_symbol(tcc_state, "eadk_timing_msleep_int", eadk_timing_msleep_int);
  tcc_add_symbol(tcc_state, "hello", hello);

  // FIXME: this tcc_relocate
  // // Relocate the code (prepare for execution)
  // // if (tcc_relocate(tcc_state, TCC_RELOCATE_AUTO) < 0) { // XXX: didn't work
  // if (tcc_relocate(tcc_state) < 0) {
  //   fprintf(stderr, "ERR: couldn't relocate code\n");
  //   tcc_delete(tcc_state); // delete the state
  //   eadk_timing_msleep(2000);
  //   return 1;
  // }

  // // get entry symbol
  // func_main_our_code = tcc_get_symbol(tcc_state, "main");
  // if (!func_main_our_code) {
  //   fprintf(stderr, "ERR: no main function?\n");
  //   tcc_delete(tcc_state); // delete the state
  //   eadk_timing_msleep(2000);
  //   return 1;
  // }

  // // run the compiled code, print the return value (for debugging)
  // int ret_val = func_main_our_code(42);

  int ret_val = tcc_run(tcc_state, argc, argv);

  fprintf(stderr, "ERR: Return: %d\n", ret_val);
  eadk_timing_msleep(2000);

  // Clean up TCC state
  tcc_delete(tcc_state); // delete the state

  eadk_timing_msleep(2000);
  printf("End of interpretation of 'tcc.py'...\n");
  eadk_timing_msleep(2000);

  return 0;
}
