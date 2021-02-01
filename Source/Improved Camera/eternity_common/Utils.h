#ifndef __UTILS_H__
#define __UTILS_H__

#include <windows.h>

#ifdef QT_VERSION
#include <QString>
#endif

#include <stdint.h>

#include <string>
#include <sstream>
#include <vector>

#define UNUSED(x) (void)(x)

#ifdef _MSC_VER

#define FUNCNAME    __FUNCSIG__

// WARNING: not safe
#define snprintf    _snprintf

#define strncasecmp _strnicmp
#define strcasecmp _stricmp

#include <direct.h>

#else

#include <dirent.h>
#define FUNCNAME    __PRETTY_FUNCTION__

#define _rmdir rmdir

#endif

#ifdef  _MSC_VER
#define fseeko fseek
#define ftello ftell
#define fseeko64 _fseeki64
#define ftello64 _ftelli64
#define off64_t int64_t
#endif

#if defined(_M_X64) || defined(__x86_64__)
#define CPU_X86_64 1
#elif defined(_M_X86) || defined(__i386__)
#define CPU_X86 1
#endif

namespace Utils
{
	
    inline uint32_t DifPointer(const void *ptr1, const void *ptr2) // ptr1-ptr2
	{
		return (uint32_t) ((uint64_t)ptr1 - (uint64_t)ptr2);
	}

    inline uint64_t DifPointer64(const void *ptr1, const void *ptr2) // ptr1-ptr2
    {
        return ((uint64_t)ptr1 - (uint64_t)ptr2);
    }
	
    static inline size_t Align(size_t n, size_t alignment)
    {
        if ((n % alignment) != 0)
            n += (alignment - (n % alignment));

        return n;
    }

    // For power of 2 only
    static inline size_t Align2(size_t n, size_t alignment)
    {
        if (n & (alignment-1))
            n += (alignment - (n & (alignment-1)));

        return n;
    }   
}

#endif
