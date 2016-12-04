#ifndef _REGS_H_INCLUDED
# define _REGS_H_INCLUDED
//	Processor register structures and declarations. This interface abstracts
//	register names behind a common, portable interface

#include <stdint.h>

// 32 bit registers
struct _R32BIT 
{
    uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
	uint32_t ebp;
	uint32_t esp;
	uint32_t eflags;
    uint8_t  cflag;
};

// 16 bit registers
struct _R16BIT 
{
    uint16_t ax;
	uint16_t bx;
	uint16_t cx;
	uint16_t dx;
	uint16_t si;
	uint16_t di;
	uint16_t bp;
	uint16_t sp;
	uint16_t es;
	uint16_t cs;
	uint16_t ss;
	uint16_t ds;
	uint16_t flags;
    uint8_t  cflag;
};

// 16 bit registers expressed in 32 bit registers
struct _R16BIT32 
{
    uint16_t ax;
	uint16_t axh;
	uint16_t bx;
	uint16_t bxh;
	uint16_t cx;
	uint16_t cxh;
	uint16_t dx;
	uint16_t dxh;
	uint16_t si;
	uint16_t di;
	uint16_t bp;
	uint16_t sp;
	uint16_t es;
	uint16_t cs;
	uint16_t ss;
	uint16_t ds;
	uint16_t flags;
	uint8_t  cflags;
};

// 8 bit registers
struct _R8BIT 
{
    uint8_t al;
	uint8_t ah;
	uint8_t bl;
	uint8_t bh;
	uint8_t cl;
	uint8_t ch;
	uint8_t dl;
	uint8_t dh;
};

// 8 bit registers expressed in 32 bit registers
struct _R8BIT32 
{
    uint8_t  al; 
	uint8_t  ah; 
	uint16_t axh; 
	uint8_t  bl;
	uint8_t  bh; 
	uint16_t bxh; 
	uint8_t  cl;
	uint8_t  ch; 
	uint16_t cxh; 
	uint8_t  dl;
	uint8_t  dh; 
	uint16_t dxh; 
};

// 8 and 16 bit registers union
union _INTR16 
{
    struct _R16BIT x;
    struct _R8BIT  h;
};

//! 32 bit, 16 bit and 8 bit registers union
union _INTR32 
{
	struct _R32BIT   x;
	struct _R16BIT32 l;
	struct _R8BIT32  h;
};

#endif
