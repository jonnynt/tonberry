//
// CompileOptions.h
//
// Options that affect the AI behavior but must be done at compile time and cannot be put into the parameter file
//

//
// ULTRA_FAST will make many hard-coded optimizations that will greatly help the compiler
// when inlining and compiling code by disabling logging and other features.
//
//#define ULTRA_FAST
#define WRITING_TO_FILES

#if !defined(ULTRA_FAST) && defined(WRITING_TO_FILES)
//
// When this is enabled asserts will be outputted to g_Context->Files.Assert
//
#define USE_ASSERT_FILE
#endif

//
// When this is enabled all important function enter/exits will be logged to a file.  This is
// needed because standard debugging is very hard since SCII is a full screen DirectX app with
// debug protection.
//
//#define USE_FUNCTION_DEBUGGER
