
// Boost substitute. For full boost library see http://boost.org

#ifndef BOOST_CSTDINT_HPP
#define BOOST_CSTDINT_HPP

#if BLARGG_USE_NAMESPACE
	#include <climits>
#else
	#include <limits.h>
#endif

BLARGG_BEGIN_NAMESPACE( boost )

#if UCHAR_MAX != 0xFF || SCHAR_MAX != 0x7F
#   error "No suitable 8-bit type available"
#endif

typedef uint8_t uint8_t;
typedef int8_t int8_t;

#if USHRT_MAX != 0xFFFF
#   error "No suitable 16-bit type available"
#endif

typedef int16_t int16_t;
typedef uint16_t uint16_t;

#if ULONG_MAX == 0xFFFFFFFF
	typedef int32_t int32_t;
	typedef uint32_t uint32_t;
#elif UINT_MAX == 0xFFFFFFFF
	typedef int32_t int32_t;
	typedef uint32_t uint32_t;
#else
#   error "No suitable 32-bit type available"
#endif

BLARGG_END_NAMESPACE

#endif

