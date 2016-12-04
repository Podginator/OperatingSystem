// Page Directory Entry handling

#include "vm_pde.h"

void PDE_AddAttribute(PageDirectoryEntry * entry, uint32_t attribute) 
{
	*entry |= attribute;
}

void PDE_RemoveAttribute(PageDirectoryEntry * entry, uint32_t attribute) 
{
	*entry &= ~attribute;
}

void PDE_SetFrame(PageDirectoryEntry * entry, uint32_t addr) 
{
	*entry = (*entry & ~I86_PDE_FRAME) | addr;
}

bool PDE_IsPresent(PageDirectoryEntry entry) 
{
	return entry & I86_PDE_PRESENT;
}

bool PDE_IsWritable(PageDirectoryEntry entry) 
{
	return entry & I86_PDE_WRITABLE;
}

uint32_t PDE_PhysicalAddress(PageDirectoryEntry entry) 
{
	return entry & I86_PDE_FRAME;
}

bool PDE_IsUser(PageDirectoryEntry entry) 
{
	return entry & I86_PDE_USER;
}

bool PDE_Is4MB(PageDirectoryEntry entry) 
{
	return entry & I86_PDE_4MB;
}

