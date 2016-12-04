//    gdt.c
//
//	Global Descriptor table (GDT) for i86 processors. This handles
//	the basic memory map for the system and permission levels

#include "gdt.h"
#include <string.h>

// Processor gdtr register points to base of gdt. This helps
// us set up the pointer
typedef struct  
{
	uint16_t		m_limit;	// Size of gdt
	uint32_t		m_base;		// Base address of gdt
} __attribute__((packed)) gdtr;


// Global descriptor table is an array of descriptors
static GDT_Descriptor _gdt [MAX_GDT_DESCRIPTORS];

// gdtr data
static gdtr	_gdtr;

// Install gdtr

static void GDT_Install() 
{
	asm("lgdt %0"  : : "m" (_gdtr));
}

//  Setup a descriptor in the Global Descriptor Table

void I86_GDT_SetDescriptor(uint32_t i, 
						   uint64_t base, 
						   uint64_t limit, 
						   uint8_t access, 
						   uint8_t grand)
{
	if (i > MAX_GDT_DESCRIPTORS)
	{
		return;
	}
	// Null out the descriptor
	memset((void*)&_gdt[i], 0, sizeof(GDT_Descriptor));

	// Wet limit and base addresses
	_gdt[i].baseLo	= (uint16_t)(base & 0xffff);
	_gdt[i].baseMid	= (uint8_t)((base >> 16) & 0xff);
	_gdt[i].baseHi	= (uint8_t)((base >> 24) & 0xff);
	_gdt[i].limit	= (uint16_t)(limit & 0xffff);

	// set flags and granularity bytes
	_gdt[i].flags = access;
	_gdt[i].grand = (uint8_t)((limit >> 16) & 0x0f);
	_gdt[i].grand |= grand & 0xf0;
}

// Return descriptor in gdt

GDT_Descriptor* I86_GDT_GetDescriptor(int i) 
{
	if (i > MAX_GDT_DESCRIPTORS)
	{
		return 0;
	}
	return &_gdt[i];
}

// Initialise GDT

int I86_GDT_Initialise() 
{

	// Setup gdtr
	_gdtr.m_limit = (sizeof(GDT_Descriptor) * MAX_GDT_DESCRIPTORS) - 1;
	_gdtr.m_base = (uint32_t)&_gdt[0];

	// Set null descriptor
	I86_GDT_SetDescriptor(0, 0, 0, 0, 0);

	// Set default code descriptor
	I86_GDT_SetDescriptor(1,0,0xffffffff,
						  I86_GDT_DESC_READWRITE|I86_GDT_DESC_EXEC_CODE|I86_GDT_DESC_CODEDATA|I86_GDT_DESC_MEMORY,
						  I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK);

	// Set default data descriptor
	I86_GDT_SetDescriptor(2,0,0xffffffff,
					      I86_GDT_DESC_READWRITE|I86_GDT_DESC_CODEDATA|I86_GDT_DESC_MEMORY,
						  I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK);

	// Install gdtr
	GDT_Install();
	return 0;
}

