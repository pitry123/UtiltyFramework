/// @file	.\Include\core\os.h.
/// @brief	Declares operating system dependent definitions.
#pragma once
#include <cstring>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#define DLL_EXPORT __declspec(dllexport)

#define STRCPY(a,a_size,b) strcpy_s(a,a_size,b)
#define MEMCPY(dst,dst_size,src,src_size) memcpy_s(dst,dst_size,src,src_size)
#ifdef _MSC_VER
const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

inline void set_current_thread_name(const char* name)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = name;
	info.dwThreadID = static_cast<DWORD>(-1); // Indicates current thread;
	info.dwFlags = 0;

	__try
	{
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
}
#else
inline void set_current_thread_name(const char* name)
{
	// Doing nothing... Setting thread names is currently supported only for MSVC under Windows
}
#endif
#else
#ifdef LINUX_PTHREAD_SET_NAME
#include <sys/prctl.h>
#endif

#define DLL_EXPORT
#define STRCPY(a,a_size,b) std::strcpy(a, b);
#define MEMCPY(dst,dst_size,src,src_size) std::memcpy(dst,src,src_size)

inline void set_current_thread_name(const char* name)
{
#ifdef LINUX_PTHREAD_SET_NAME
	if (name != nullptr)
		prctl(PR_SET_NAME, name);
#endif
}
#endif

#ifdef GCC_492

#include <cstddef>
#include <cstdint>

namespace std
{
    ///-------------------------------------------------------------------
	///  @brief	Extends missing STL functions in GCC 4.9.2
	///  		http://en.cppreference.com/w/cpp/memory/align
    ///-------------------------------------------------------------------
    inline void *align( std::size_t alignment, std::size_t size,
                        void *&ptr, std::size_t &space ) {
        std::uintptr_t pn = reinterpret_cast< std::uintptr_t >( ptr );
        std::uintptr_t aligned = ( pn + alignment - 1 ) & - alignment;
        std::size_t padding = aligned - pn;
        if ( space < size + padding ) return nullptr;
        space -= padding;
        return ptr = reinterpret_cast< void * >( aligned );
    }
}

#endif
