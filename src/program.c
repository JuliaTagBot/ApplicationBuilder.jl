// This file is a part of Julia. License is MIT: http://julialang.org/license

// Standard headers
#include <string.h>
#include <stdint.h>

// Julia headers (for initialization and gc commands)
#include "uv.h"
#include "julia.h"

#ifdef JULIA_DEFINE_FAST_TLS // only available in Julia v0.7 and above
JULIA_DEFINE_FAST_TLS()
#endif

// Declare C prototype of a function defined in Julia
extern int julia_main(jl_array_t*);

// Declare C prototype of a function defined in Julia
extern void set_program_file(jl_value_t*);

// main function (windows UTF16 -> UTF8 argument conversion code copied from julia's ui/repl.c)
int main(int argc, char *argv[])
{
    int retcode;
    int i;
    uv_setup_args(argc, argv); // no-op on Windows

    // initialization
    libsupport_init();

    // jl_options.compile_enabled = JL_OPTIONS_COMPILE_OFF;
    // JULIAC_PROGRAM_LIBNAME defined on command-line for compilation
    jl_options.image_file = JULIAC_PROGRAM_LIBNAME;
    julia_init(JL_IMAGE_JULIA_HOME);

    // build arguments array: `String[ unsafe_string(argv[i]) for i in 1:argc ]`
    jl_array_t *ARGS = jl_alloc_array_1d(jl_apply_array_type(jl_string_type, 1), 0);
    JL_GC_PUSH1(&ARGS);
    jl_array_grow_end(ARGS, argc - 1);
    for (i = 1; i < argc; i++) {
        jl_value_t *s = (jl_value_t*)jl_cstr_to_string(argv[i]);
        jl_arrayset(ARGS, s, i - 1);
    }
    // Set PROGRAM_FILE manually, since it's not set in the default program
    // provided at `~/.julia/v0.6/PackageCompiler/examples/program.c`.
    // For more info, please see the following issue:
    // https://github.com/JuliaLang/PackageCompiler.jl/issues/54
    jl_set_global(jl_base_module, jl_symbol("PROGRAM_FILE"), (jl_value_t*)jl_cstr_to_string(argv[0]));

    // Pass the arguments to julia, so it can provide them as globals.
    // This also initializes PROGRAM_FILE.
    //jl_set_ARGS(argc, argv);

    // call the work function, and get back a value
    retcode = julia_main(ARGS);
    JL_GC_POP();

    // Cleanup and gracefully exit
    jl_atexit_hook(retcode);
    return retcode;
}
