#ifndef _MMNGR_VIRT_H
#define _MMNGR_VIRT_H

// Virtual Memory Manager

#include <stdint.h>
#include <hal.h>
#include "vm_pte.h"
#include "vm_pde.h"

typedef uint32_t virtual_address;

// i86 architecture defines 1024 entries per table

#define PAGES_PER_TABLE 1024
#define PAGES_PER_DIR	1024

#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3ff)
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff)
#define PAGE_GET_PHYSICAL_ADDRESS(x) (*x & ~0xfff)

// Page table
typedef struct _PageTable 
{
	PageTableEntry entries[PAGES_PER_TABLE];
} PageTable;

// Page directory
typedef struct _PageDirectory 
{
	PageDirectoryEntry entries[PAGES_PER_DIR];
} PageDirectory;

PageTableEntry* VMM_LookupPageTableEntry(PageTable * p,virtual_address addr); 
PageDirectoryEntry* VMM_LookupPageDirectoryEntry(PageDirectory * p, virtual_address addr); 
bool VMM_SwitchPageDirectory(PageDirectory* dir); 
void VMM_FlushTLBEntry(virtual_address addr); 
PageDirectory* VMM_GetDirectory(); 
bool VMM_AllocatePage(PageTableEntry* e); 
void VMM_FreePage(PageTableEntry* e); 
void VMM_MapPage(void* phys, void* virt); 
void VMM_Initialise(); 

#endif
