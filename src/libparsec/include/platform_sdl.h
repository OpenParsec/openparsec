#ifndef PLATFORM_SDL_H_
#define PLATFORM_SDL_H_


// host cpu specification (parsec constants) ----------------------------------
//
#define SYSTEM_CPU_INTEL
//#define SYSTEM_CPU_POWERPC

#ifdef SYSTEM_TARGET_WINDOWS
#define SIGIOT SIGABRT
#endif

#ifdef SYSTEM_SDL
//#define DISABLE_JOYSTICK_CODE // No Joystick support in Cygwin port yet.
#endif


#endif // !PLATFORM_SDL_H_


