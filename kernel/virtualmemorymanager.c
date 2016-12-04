//	Virtual Memory Manager

#include <string.h>
#include "virtualmemorymanager.h"
#include "physicalmemorymanager.h"

// Page table represents 4mb address space
#define PTABLE_ADDR_SPACE_SIZE 0x400000

// Directory table represents 4GB address space
#define DTABLE_ADDR_SPACE_SIZE 0x100000000

// page sizes are 4k
#define PAGE_SIZE 4096

// Current directory table
PageDirectory*		_current_PageDirectory = 0;

// Current page directory base register
uint32_t		_current_pdbr = 0;

PageTableEntry* VMM_LookupPageTableEntry(PageTable * p,virtual_address addr) 
{
	if (p)
	{
		return &p->entries[PAGE_TABLE_INDEX(addr)];
	}
	return 0;
}

PageDirectoryEntry* VMM_LookupPageDirectoryEntry(PageDirectory * p, virtual_address addr) 
{
	if (p)
	{
		return &p->entries[PAGE_TABLE_INDEX(addr)];
	}
	return 0;
}

bool VMM_SwitchPageDirectory(PageDirectory* dir) 
{
	if (!dir)
	{
		return false;
	}
	_current_PageDirectory = dir;
	HAL_LoadPageDirectoryBaseRegister(_current_pdbr);
	return true;
}

void VMM_FlushTLBEntry(virtual_address addr) 
{
	asm volatile("cli \n\t"
				 "invlpg (%0)\n\t"
				 "sti" : : "b"(addr) : "memory" );
}

PageDirectory* VMM_GetDirectory() 
{
	return _current_PageDirectory;
}

bool VMM_AllocatePage(PageTableEntry* e) 
{
	// Allocate a free physical frame
	void* p = PMM_AllocateBlock();
	if (!p)
	{
		return false;
	}
	// Map it to the page
	PTE_SetFrame(e, (uint32_t)p);
	PDE_AddAttribute(e, I86_PTE_PRESENT);
	return true;
}

void VMM_FreePage(PageTableEntry* e) 
{
	void* p = (void*)PTE_PhysicalAddress(*e);
	if (p)
	{
		PMM_FreeBlock(p);
	}
	PTE_RemoveAttribute(e, I86_PTE_PRESENT);
}

void VMM_MapPage(void* phys, void* virt) 
{
    // Get page directory
    PageDirectory* pageDirectory = VMM_GetDirectory();

    // Get page table
    PageDirectoryEntry* e = &pageDirectory->entries[PAGE_DIRECTORY_INDEX((uint32_t)virt)];
    if ((*e & I86_PTE_PRESENT) != I86_PTE_PRESENT) 
    {
		// Page table not present, so allocate it
		PageTable* table = (PageTable*)PMM_AllocateBlock();
		if (!table)
		{
			return;
		}
		// Clear page table
		memset(table, 0, sizeof(PageTable));

		// create a new entry in the directory
		PageDirectoryEntry* entry = &pageDirectory->entries[PAGE_DIRECTORY_INDEX((uint32_t)virt)];

		// Map in the table (
		PDE_AddAttribute(entry, I86_PDE_PRESENT);
		PDE_AddAttribute(entry, I86_PDE_WRITABLE);
		PDE_SetFrame(entry, (uint32_t)table);
	}

	// Get page table
	PageTable* table = (PageTable*)PAGE_GET_PHYSICAL_ADDRESS(e);

	// Get page
	PageTableEntry* page = &table->entries[PAGE_TABLE_INDEX((uint32_t)virt)];

    // Map it in 
    PTE_SetFrame(page, (uint32_t) phys);
    PTE_AddAttribute( page, I86_PTE_PRESENT);
}

void VMM_Initialise() 
{
	// Allocate default page table
	PageTable* table = (PageTable*)PMM_AllocateBlock();
	if (!table)
    {
		return;
    }	
    // Allocate 3GB page table
    PageTable* table2 = (PageTable*)PMM_AllocateBlock();
    if (!table2)
	{
		return;
	}
	// Clear page table
	memset(table, 0, sizeof (PageTable));
    // The first 4MB of virtual addresses are mapped to the same physical addresses
    for (int i = 0, frame=0x0, virt=0x00000000; i<1024; i++, frame += 4096, virt += 4096) 
	{
		// Create a new page
		PageTableEntry page = 0;
		PTE_AddAttribute(&page, I86_PTE_PRESENT);
		PTE_SetFrame(&page, frame);
		// and add it to the page table
		table->entries[PAGE_TABLE_INDEX(virt)] = page;
	}

	// Map 16mb to 3GB (where our kernel is)
	memset(table2, 0, sizeof (PageTable));
	for (int i=0, frame=0x100000, virt=0xc0000000; i<1024; i++, frame += 4096, virt += 4096) 
	{
		// Create a new page
		PageTableEntry page = 0;
		PTE_AddAttribute(&page, I86_PTE_PRESENT);
		PTE_SetFrame(&page, frame);
		// and add it to the page table
		table2->entries[PAGE_TABLE_INDEX(virt)] = page;
	}

	// Create default directory table
	PageDirectory* dir = (PageDirectory*)PMM_AllocateBlocks(3);
	if (!dir)
	{
			return;
	}
	// clear directory table and set it as current
	memset(dir, 0, sizeof(PageDirectory));

	// Get first entry in directory table and set it up to point to our table
	PageDirectoryEntry* entry = &dir->entries[PAGE_DIRECTORY_INDEX(0x00000000)];
	PDE_AddAttribute(entry, I86_PDE_PRESENT);
	PDE_AddAttribute(entry, I86_PDE_WRITABLE);
	PDE_SetFrame(entry, (uint32_t)table);

	// Set entry that points to the kernel
	PageDirectoryEntry* entry2 = &dir->entries[PAGE_DIRECTORY_INDEX(0xc0000000)];
    PDE_AddAttribute(entry2, I86_PDE_PRESENT);
    PDE_AddAttribute(entry2, I86_PDE_WRITABLE);
    PDE_SetFrame(entry2, (uint32_t)table2);

    // Store current PDBR
    _current_pdbr = (uint32_t)&dir->entries;

    // Switch to using our page directory
    VMM_SwitchPageDirectory(dir);

	// Enable paging
    HAL_EnablePaging();
}
