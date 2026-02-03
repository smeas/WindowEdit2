#pragma once
#include <cstdint>

#define SK_LOG_LEVEL SK_LOG_LEVEL_VERBOSE

#define SK_LOG_LEVEL_DEBUG 5
#define SK_LOG_LEVEL_VERBOSE 4
#define SK_LOG_LEVEL_ERROR 3
#define SK_LOG_LEVEL_WARNING 2
#define SK_LOG_LEVEL_INFO 1
#define SK_LOG_LEVEL_NONE 0

#define SK_INTERNAL static

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

#define U64_MAX (~((u64)0))
#define U32_MAX (~((u32)0))

#define SK_INLINE __forceinline
#define SK_NOINLINE _declspec(noinline)

//
// Asserts
//

#ifdef _DEBUG
#define SK_ASSERT(expr) (void)( ( !!(expr) ) || ( ::sk::AssertFailW(_CRT_WIDE(__FILE__), (u32)(__LINE__), _CRT_WIDE(#expr)), 0 ) )
//#define SK_ASSERT(expr, message) (void)( ( !!(expr) ) || ( ::sk::AssertFailW(_CRT_WIDE(__FILE__), (u32)(__LINE__), _CRT_WIDE("Failed on '" #expr "':\n" message)), 0 ) )
#define SK_ASSERTF(expr, message, ...) (void)( ( !!(expr) ) || ( ::sk::AssertFailFW(_CRT_WIDE(__FILE__), (u32)(__LINE__), _CRT_WIDE(#message), __VA_ARGS__), 0 ) )
#else
#define SK_ASSERT(expr)
#define SK_ASSERTF(expr, message, ...)
#endif
#define SK_VERIFY(expr) (void)( ( !!(expr) ) || ( ::sk::AssertFailW(_CRT_WIDE(__FILE__), (u32)(__LINE__), _CRT_WIDE(#expr)), 0 ) )
//#define SK_VERIFY(expr, message) (void)( ( !!(expr) ) || ( ::sk::VerifyFailW(_CRT_WIDE(__FILE__), (u32)(__LINE__), _CRT_WIDE("Failed on '" #expr "':\n" message)), 0 ) )
#define SK_VERIFYF(expr, message, ...) (void)( ( !!(expr) ) || ( ::sk::AssertFailFW(_CRT_WIDE(__FILE__), (u32)(__LINE__), _CRT_WIDE(#message), __VA_ARGS__), 0 ) )

#define SK_NOIMPL SK_VERIFYF(false, "Not implemented.")

namespace sk
{
void AssertFailW(wchar_t const* file, u32 line, wchar_t const* message);
void AssertFailFW(wchar_t const* file, u32 line, wchar_t const* format, ...);
// void VerifyFailW(wchar_t const* file, u32 line, wchar_t const* message);
// void VerifyFailFW(wchar_t const* file, u32 line, wchar_t const* format, ...);
}

//
// Logging
//

#ifndef SK_LOG_LEVEL
#define SK_LOG_LEVEL 0
#endif

#if SK_LOG_LEVEL >= SK_LOG_LEVEL_DEBUG
#define SK_LOG_D(...) printf(__VA_ARGS__)
#else
#define SK_LOG_D(...)
#endif

#if SK_LOG_LEVEL >= SK_LOG_LEVEL_VERBOSE
#define SK_LOG_V(...) printf(__VA_ARGS__)
#else
#define SK_LOG_V(...)
#endif

#if SK_LOG_LEVEL >= SK_LOG_LEVEL_ERROR
#define SK_LOG_E(...) printf(__VA_ARGS__)
#else
#define SK_LOG_E(...)
#endif

#if SK_LOG_LEVEL >= SK_LOG_LEVEL_WARNING
#define SK_LOG_W(...) printf(__VA_ARGS__)
#else
#define SK_LOG_W(...)
#endif

#if SK_LOG_LEVEL >= SK_LOG_LEVEL_INFO
#define SK_LOG(...) printf(__VA_ARGS__)
#else
#define SK_LOG(...)
#endif
