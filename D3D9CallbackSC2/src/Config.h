//
// Config.h
//
// Options that affect the compiler, debug output, DirectX debug modes, etc.
//

//
// VS 8+ includes some bounds checking even in debug mode.  This include disable this and
// enables full optimization.
//
#ifndef _DEBUG
#define _SECURE_SCL 0
#endif

#define _WIN32_WINNT _WIN32_WINNT_WIN7
#define WINVER _WIN32_WINNT_WIN7

//
// Enables all standard C-runtime functions without warnings
//
#define _CRT_SECURE_NO_DEPRECATE

//
// commenting out the following lines will disable code involving the corresponding API.
// This can be useful if you lack some of the libraries involved with either API.
// Software rendering is always supported.
//
#define USE_D3D9
#define USE_D3D

//
// When defined, asserts are thrown on out-of-bounds vector access.
//
#ifdef _DEBUG
#define VECTOR_DEBUG
#endif

//
// Enables asserts in release mode
//
//#define ALWAYS_ASSERT

//
// Exclude rarely-used stuff from Windows headers
//
//#define WIN32_LEAN_AND_MEAN    

//
// Control over DirectX debug modes
//
//#define D3D_DEBUG_INFO
//#define DEBUG_PS
//#define DEBUG_VS
//#define USE_REF

#define USE_ANN
#define USE_KDTREE

//
// Compression and PNG support.  PNG requires ZLIB
//
#define USE_ZLIB
#define USE_PNG

//
// Matrix library support
//
//#define USE_TAUCS

//#define USE_COMPILED_SHADERS

//#define USE_WMF