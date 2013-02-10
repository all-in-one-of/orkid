#ifndef UNITTEST_CONFIG_H
#define UNITTEST_CONFIG_H

// Standard defines documented here: http://predef.sourceforge.net

#if defined _MSC_VER

   #pragma warning(disable:4127) // conditional expression is constant
   #pragma warning(disable:4702) // unreachable code
   #pragma warning(disable:4722) // destructor never returns, potential memory leak

#elif defined(unix) || defined(__unix__) || defined(__unix) || defined(linux) || \
      defined(__APPLE__) || defined (__NetBSD__) || defined (__OpenBSD__) || defined (__FreeBSD__)

    #define UNITTEST_POSIX

#elif defined (__MINGW32__)

    #define UNITTEST_MINGW

#elif defined(__psp__)

    #define UNITTEST_PSP

#elif defined(NITRO)

    #define UNITTEST_NDS
	#pragma force_active on

#elif defined(WII)

    #define UNITTEST_WII

#endif

// by default, MemoryOutStream is implemented in terms of std::ostringstream.
// uncomment this line to use the custom MemoryOutStream (no deps on std::ostringstream).
//#define UNITTEST_USE_CUSTOM_STREAMS

#endif
