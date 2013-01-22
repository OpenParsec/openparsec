#ifndef PLATFORM_SDL_H_
#define PLATFORM_SDL_H_


// compiler specification (parsec constants) ----------------------------------
//
#if defined(_MSC_VER)
#	define SYSTEM_COMPILER_MSVC
#elif defined(__clang__)
#	define SYSTEM_COMPILER_CLANG
#elif defined(__GNUC__)
#	if defined(__llvm__)
#		define SYSTEM_COMPILER_LLVM_GCC
#	else
#		define SYSTEM_COMPILER_GCC
#	endif
#endif


// host cpu specification (parsec constants) ----------------------------------
//
#define SYSTEM_CPU_INTEL
//#define SYSTEM_CPU_POWERPC

#ifdef _WIN32
#define SIGIOT SIGABRT
#endif

#ifdef SYSTEM_SDL
//#define DISABLE_JOYSTICK_CODE // No Joystick support in Cygwin port yet.
#endif

/* XXX: Pretty sure none of this is needed anymore....
// Cygwin port 
#ifdef __CYGWIN__
#define SIGIOT SIGABRT // Cygwin port requires redefinition of SIGIOT
#define DISABLE_JOYSTICK_CODE // No Joystick support in Cygwin port yet.
#define SYSTEM_CYGWIN
#endif

#ifndef __CYGWIN__
#define USE_XF86_VIDMODE_EXTENSION
#endif


//NOTE:
// if nothing is defined here, reasonable guesses
// will be made later on. if one of these constants
// is defined, however, the cpu setting will be
// enforced (that is, not altered anywhere).
*/



#endif // !PLATFORM_SDL_H_


