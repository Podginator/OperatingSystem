#ifndef _MMNGR_VIRT_PTE_H
#define _MMNGR_VIRT_PTE_H

// Page Table Entry management

#include <stdint.h>
#include "physicalmemorymanager.h"	

// i86 architecture defines this format 

#define	I86_PTE_PRESENT				1			//0000000000000000000000000000001
#define	I86_PTE_WRITABLE			2			//0000000000000000000000000000010
#define	I86_PTE_USER				4			//0000000000000000000000000000100
#define	I86_PTE_WRITETHOUGH			8			//0000000000000000000000000001000
#define	I86_PTE_NOT_CACHEABLE		0x10		//0000000000000000000000000010000
#define	I86_PTE_ACCESSED			0x20		//0000000000000000000000000100000
#define	I86_PTE_DIRTY				0x40		//0000000000000000000000001000000
#define	I86_PTE_PAT					0x80		//0000000000000000000000010000000
#define	I86_PTE_CPU_GLOBAL			0x100		//0000000000000000000000100000000
#define	I86_PTE_LV4_GLOBAL			0x200		//0000000000000000000001000000000
#define	I86_PTE_FRAME				0x7FFFF000 	//1111111111111111111000000000000

typedef uint32_t PageTableEntry;

void PTE_AddAttribute(PageTableEntry * entry, uint32_t attribute); 
void PTE_RemoveAttribute(PageTableEntry * entry, uint32_t attribute); 
void PTE_SetFrame(PageTableEntry* entry, uint32_t addr); 
bool PTE_IsPresent(PageTableEntry entry); 
bool PTE_IsWritable(PageTableEntry entry); 
uint32_t PTE_PhysicalAddress(PageTableEntry entry); 

#endif
