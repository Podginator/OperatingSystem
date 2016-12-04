// Page Table Entry management
 
#include "vm_pte.h"

void PTE_AddAttribute(PageTableEntry * entry, uint32_t attribute) 
{
	*entry |= attribute;
}

void PTE_RemoveAttribute(PageTableEntry * entry, uint32_t attribute) 
{
	*entry &= ~attribute;
}

void PTE_SetFrame(PageTableEntry* entry, uint32_t addr) 
{
	*entry = (*entry & ~I86_PTE_FRAME) | addr;
}

bool PTE_IsPresent(PageTableEntry entry) 
{
	return entry & I86_PTE_PRESENT;
}

bool PTE_IsWritable(PageTableEntry entry) 
{
	return entry & I86_PTE_WRITABLE;
}

uint32_t PTE_PhysicalAddress(PageTableEntry entry) 
{
	return entry & I86_PTE_FRAME;
}

