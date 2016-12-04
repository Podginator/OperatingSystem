#ifndef _PMM_H
#define _PMM_H

// Physical Memory Manager

#include <size_t.h>
#include <stdint.h>
#include "bootinfo.h"

// Initialise the physical memory manager

uint32_t PMM_Initialise(BootInfo * bootInfo, uint32_t bitmap);

// Mark a region as being available for use

void PMM_MarkRegionAsAvailable(uint32_t base, size_t size); 

// Mark a region as not being available for use

void PMM_MarkRegionAsUnavailable(uint32_t base, size_t size); 

// Allocate a single memory block

void* PMM_AllocateBlock(); 

// Free a single memory block

void PMM_FreeBlock(void* p); 

// Allocate 'size' blocks of memory

void * PMM_AllocateBlocks(size_t size); 

// Free size blocks

void PMM_FreeBlocks(void* p, size_t size); 

// Get the amount of available physical memory (in K)

size_t PMM_GetAvailableMemorySize(); 

// Get the total number of blocks of available memory

uint32_t PMM_GetAvailableBlockCount(); 

// Get the number of used blocks of available memory

uint32_t PMM_GetUsedBlockCount(); 

// Get the number of free blocks of available memory

uint32_t PMM_GetFreeBlockCount(); 

// Get the size of a block

uint32_t PMM_GetBlockSize(); 

// Return the address of the memory map

uint32_t PMM_GetMemoryMap();


#endif
