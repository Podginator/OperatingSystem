#ifndef _STDINT_H
#define _STDINT_H

//  stdint.h
//  Standard C/C++ integral types

// Fixed-width types

typedef signed char			bool;

#define false 				0
#define true				1

typedef signed char 		int8_t;
typedef unsigned char 	 	uint8_t;
typedef short  				int16_t;
typedef unsigned short  	uint16_t;
typedef int  				int32_t;
typedef unsigned   			uint32_t;
typedef long long  			int64_t;
typedef unsigned long long  uint64_t;

// Minimum width types

typedef signed char 		int_least8_t;
typedef unsigned char   	uint_least8_t;
typedef short  				int_least16_t;
typedef unsigned short  	uint_least16_t;
typedef int  				int_least32_t;
typedef unsigned   			uint_least32_t;
typedef long long  			int_least64_t;
typedef unsigned long long  uint_least64_t;

//  Fastest minimum-width integer types 
//  Not actually guaranteed to be fastest for all purposes
//  Here we use the exact-width types for 8 and 16-bit ints. 

typedef char 				int_fast8_t;
typedef unsigned char 		uint_fast8_t;
typedef short  				int_fast16_t;
typedef unsigned short  	uint_fast16_t;
typedef int  				int_fast32_t;
typedef unsigned int  		uint_fast32_t;
typedef long long  			int_fast64_t;
typedef unsigned long long  uint_fast64_t;

// Integer types capable of holding object pointers 

typedef int 				intptr_t;
typedef unsigned int		uintptr_t;

//  Greatest-width integer types 

typedef long long 			intmax_t;
typedef unsigned long long  uintmax_t;

// Limits of specified-width integer types 

// Limits of exact-width integer types 

#define INT8_MIN  (-128) 
#define INT16_MIN (-32768)
#define INT32_MIN (-2147483647 - 1)
#define INT64_MIN (-9223372036854775807LL - 1)

#define INT8_MAX  127
#define INT16_MAX 32767
#define INT32_MAX 2147483647
#define INT64_MAX 9223372036854775807LL

#define UINT8_MAX  0xff 
#define UINT16_MAX 0xffff 
#define UINT32_MAX 0xffffffff  
#define UINT64_MAX 0xffffffffffffffffULL 

//  Limits of minimum-width integer types 

#define INT_LEAST8_MIN  INT8_MIN
#define INT_LEAST16_MIN INT16_MIN
#define INT_LEAST32_MIN INT32_MIN
#define INT_LEAST64_MIN INT64_MIN

#define INT_LEAST8_MAX  INT8_MAX
#define INT_LEAST16_MAX INT16_MAX
#define INT_LEAST32_MAX INT32_MAX
#define INT_LEAST64_MAX INT64_MAX

#define UINT_LEAST8_MAX  UINT8_MAX
#define UINT_LEAST16_MAX UINT16_MAX
#define UINT_LEAST32_MAX UINT32_MAX
#define UINT_LEAST64_MAX UINT64_MAX

// Limits of fastest minimum-width integer types 

#define INT_FAST8_MIN  INT8_MIN
#define INT_FAST16_MIN INT16_MIN
#define INT_FAST32_MIN INT32_MIN
#define INT_FAST64_MIN INT64_MIN

#define INT_FAST8_MAX  INT8_MAX
#define INT_FAST16_MAX INT16_MAX
#define INT_FAST32_MAX INT32_MAX
#define INT_FAST64_MAX INT64_MAX

#define UINT_FAST8_MAX  UINT8_MAX
#define UINT_FAST16_MAX UINT16_MAX
#define UINT_FAST32_MAX UINT32_MAX
#define UINT_FAST64_MAX UINT64_MAX

//  Limits of integer types capable of holding object pointers 

#define INTPTR_MIN  INT32_MIN
#define INTPTR_MAX  INT32_MAX
#define UINTPTR_MAX UINT32_MAX

// Limits of greatest-width integer types 

#define INTMAX_MIN  INT64_MIN
#define INTMAX_MAX  INT64_MAX
#define UINTMAX_MAX UINT64_MAX

// Limits of other integer types 

#define PTRDIFF_MIN INT32_MIN
#define PTRDIFF_MAX INT32_MAX

#define SIG_ATOMIC_MIN INT32_MIN
#define SIG_ATOMIC_MAX INT32_MAX

#define SIZE_MAX UINT32_MAX

//!Set bit

inline uint8_t SetBit(uint8_t word, uint8_t mask) 
{
	return word | mask;
}

// Clear bit

inline uint8_t ClrBit(uint8_t word, uint8_t mask) 
{
	return word & (~mask);
}

// Assigns the bit defined in mask to a value

inline uint8_t AssignBit(uint8_t word, uint8_t mask, bool value) 
{
	return value ? SetBit(word, mask) : ClrBit(word, mask);
}

#endif
