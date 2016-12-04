#ifndef _GDT_H_INCLUDED
#define _GDT_H_INCLUDED

//	Global Descriptor Table handling (GDT) for i86 processors. This handles
//	the basic memory map for the system and permission levels
//
//	The system software should have a gdt set up prior to this being
//	used. This sets up a basic gdt interface that can be managed through
//	the HAL
//

#include <stdint.h>

// Maximum number of descriptors allowed
#define MAX_GDT_DESCRIPTORS			3

// GDT descriptor access bit flags.	

// Set access bit
#define I86_GDT_DESC_ACCESS			0x0001			//00000001

// Descriptor is readable and writable (default is read only)
#define I86_GDT_DESC_READWRITE		0x0002			//00000010

// Set expansion direction bit
#define I86_GDT_DESC_EXPANSION		0x0004			//00000100

// Executable code segment (Default is a data segment)
#define I86_GDT_DESC_EXEC_CODE		0x0008			//00001000

// Set code or data descriptor (Default is a system defined descriptor)
#define I86_GDT_DESC_CODEDATA		0x0010			//00010000

// Set DPL bits
#define I86_GDT_DESC_DPL			0x0060			//01100000

// Set "in memory" bit
#define I86_GDT_DESC_MEMORY			0x0080			//10000000

// GDT descriptor grandularity bit flags	

// masks out limitHi (High 4 bits of limit)
#define I86_GDT_GRAND_LIMITHI_MASK	0x0f			//00001111

// Set os defined bit
#define I86_GDT_GRAND_OS			0x10			//00010000

// Set if 32bit (default is 16 bit)
#define I86_GDT_GRAND_32BIT			0x40			//01000000

// 4k grandularity (default is none)
#define I86_GDT_GRAND_4K			0x80			//10000000

// GDT descriptor. A gdt descriptor defines the properties of a specific
// memory block and permissions.

typedef struct _GDT_Descriptor 
{
	uint16_t		limit;		// Bits 0-15 of segment limit
	uint16_t		baseLo;     // Bits 0-23 of base address
	uint8_t			baseMid;
	uint8_t			flags;		// Descriptor access flags
	uint8_t			grand;
	uint8_t			baseHi;		// Bits 24-32 of base address
} __attribute__((packed)) GDT_Descriptor;

// Setup a descriptor in the Global Descriptor Table
void I86_GDT_SetDescriptor(uint32_t i, 
						   uint64_t base, 
						   uint64_t limit, 
					       uint8_t access, 
					       uint8_t grand);

// Return GDT Descriptor
GDT_Descriptor* I86_GDT_GetDescriptor(int i);

// Initialise GDT
int I86_GDT_Initialise();

#endif
