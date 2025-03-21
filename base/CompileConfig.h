
#ifndef ___COMPILE_CONFIG_H_INCLUDED__
#define ___COMPILE_CONFIG_H_INCLUDED__

// The defines for different operating system are:
// __XBOX_PLATFORM_ for XBox
// __WINDOWS_ for all supported Windows versions
// __WINDOWS_API_ for Windows or XBox
// __LINUX_PLATFORM_ for Linux (it is defined here if no other os is defined)
// __SOLARIS_PLATFORM_ for Solaris
// __POSIX_API_ for Posix compatible systems
// __USE_SDL_DEVICE_ for platform independent SDL framework
// __USE_WINDOWS_DEVICE_ for Windows API based device
// __USE_LINUX_DEVICE_ for X11 based device
// MACOSX for Mac OS X

//#define __USE_SDL_DEVICE_ 1

//! WIN32 for Windows32
//! WIN64 for Windows64
#if defined(WIN32) || defined(WIN64)
#define __WINDOWS_
#define __WINDOWS_API_
#ifndef __USE_SDL_DEVICE_
#define __USE_WINDOWS_DEVICE_
#endif
#endif

#if defined(_XBOX)
#define __XBOX_PLATFORM_
#define __WINDOWS_API_
#endif

#if !defined(__WINDOWS_API_) && !defined(MACOSX)
#if defined(__sparc__) || defined(__sun__)
#define __BIG_ENDIAN__
#define __SOLARIS_PLATFORM_
#else
#define __LINUX_PLATFORM_
#endif
#define __POSIX_API_

#ifndef __USE_SDL_DEVICE_
#define __USE_LINUX_DEVICE_
#endif
#endif

#include <stdio.h> // TODO: Although included elsewhere this is required at least for mingw

#if defined(__WINDOWS_API_) //윈도우 일때..

#define __COMPILE_WITH_DIRECT3D_9_

#endif

// Define __COMPILE_WITH_OPENGL_ to compile the with OpenGL.
// If you do not wish the engine to be compiled with OpengGL, comment this define out. 
#define __COMPILE_WITH_OPENGL_

// Define __COMPILE_WITH_X11_ to compile with X11 support.
// If you do not wish the engine to be compiled with X11, comment this define out. 
// Only used in LinuxDevice.
#define __COMPILE_WITH_X11_

// Define __OPENGL_USE_EXTPOINTER_ if the OpenGL renderer should use OpenGL extensions via function pointers.
// On some systems there is no support for the dynamic extension of OpenGL via function pointers such that this has to be undef'ed. 
#if !defined(MACOSX) && !defined(__SOLARIS_PLATFORM_)
#define __OPENGL_USE_EXTPOINTER_
#endif

// On some Linux systems the XF86 vidmode extension or X11 RandR are missing. Use these flags
// to remove the dependencies such that XDWorld will compile on those systems, too.
#if defined(__LINUX_PLATFORM_)
#define __LINUX_X11_VIDMODE_
//#define __LINUX_X11_RANDR_
#endif


// Define __COMPILE_WITH_ZLIB_ to enable compiling the engine using zlib.
// This enables the engine to read from compressed .zip archives. If you disable this feature, the engine can still read archives, but only uncompressed ones. 
#define __COMPILE_WITH_ZLIB_

// Define __USE_NON_SYSTEM_ZLIB_ to let use the zlib which comes
// If this is commented out, Engine will try to compile using the zlib installed in the system.
// This is only used when __COMPILE_WITH_ZLIB_ is defined. 
#define __USE_NON_SYSTEM_ZLIB_

// Define __COMPILE_WITH_LIBPNG_ to enable compiling the engine using libpng.
// This enables the engine to read png images. If you comment this out, the engine will no longer read .png images. 
#define __COMPILE_WITH_LIBPNG_


#define BURNINGVIDEO_RENDERER_BEAUTIFUL
//#define BURNINGVIDEO_RENDERER_FAST
//#define BURNINGVIDEO_RENDERER_ULTRA_FAST


// Set FPU settings
// Should use approximate float and integer fpu techniques precision will be lower but speed higher. currently X86 only
#if !defined(MACOSX) && !defined(__SOLARIS_PLATFORM_)
	//#define _FAST_MATH
#endif

// Some cleanup
// XBox does not have OpenGL or DirectX9
#if defined(__XBOX_PLATFORM_)
#undef __COMPILE_WITH_OPENGL_
#undef __COMPILE_WITH_DIRECT3D_9_
#endif

#endif // ___COMPILE_CONFIG_H_INCLUDED__

