#ifndef _MMNGR_VIRT_PDE_H
#define _MMNGR_VIRT_PDE_H
//	Page Directory Entries (PDE) management

#include <stdint.h>
#include "physicalmemorymanager.h"	

// This format is defined by the i86 architecture

#define I86_PDE_PRESENT				1			//0000000000000000000000000000001
#define	I86_PDE_WRITABLE			2			//0000000000000000000000000000010
#define I86_PDE_USER				4			//0000000000000000000000000000100
#define	I86_PDE_PWT					8			//0000000000000000000000000001000
#define	I86_PDE_PCD					0x10		//0000000000000000000000000010000
#define	I86_PDE_ACCESSED			0x20		//0000000000000000000000000100000
#define	I86_PDE_DIRTY				0x40		//0000000000000000000000001000000
#define	I86_PDE_4MB					0x80		//0000000000000000000000010000000
#define	I86_PDE_CPU_GLOBAL			0x100		//0000000000000000000000100000000
#define	I86_PDE_LV4_GLOBAL			0x200		//0000000000000000000001000000000
#define	I86_PDE_FRAME				0x7FFFF000 	//1111111111111111111000000000000

// A page directory entry
typedef uint32_t PageDirectoryEntry;

void PDE_AddAttribute(PageDirectoryEntry * entry, uint32_t attribute);
void PDE_RemoveAttribute(PageDirectoryEntry * entry, uint32_t attribute); 
void PDE_SetFrame(PageDirectoryEntry * entry, uint32_t addr); 
bool PDE_IsPresent(PageDirectoryEntry entry); 
bool PDE_IsWritable(PageDirectoryEntry entry); 
uint32_t PDE_PhysicalAddress(PageDirectoryEntry entry); 
bool PDE_IsUser(PageDirectoryEntry entry); 
bool PDE_Is4MB(PageDirectoryEntry entry); 

#endif
