// Physical Memory Manager

#include <string.h>
#include "physicalmemorymanager.h"

// Each byte in the memory map indicates 8 blocks of memory

#define PMM_BLOCKS_PER_BYTE 	8

// The block size is 4096 bytes (4K)
#define PMM_BLOCK_SIZE			4096

// Blocks are alligned on block size boundaries

#define PMM_BLOCK_ALIGNMENT		PMM_BLOCK_SIZE

// Size of physical memory

static	uint32_t	_physicalMemorySize = 0;

// number of blocks currently in use
static	uint32_t	_usedBlocks = 0;

// maximum number of available memory blocks

static	uint32_t	_maximumBlockCount = 0;

// Memory map bit array. Each bit represents a memory block

static	uint32_t*	_memoryMap = 0;

static uint32_t 	_memoryMapSize = 0;

// Private functions

// Set any bit within the memory map bit array
//
// This marks the block as being in use

void MemoryMapSetBit(uint32_t bit) 
{
	_memoryMap[bit / 32] |= (1 << (bit % 32));
}

// Clear (unset) any bit within the memory map bit array.
//
// This marks the block as being available for use

void MemoryMapClearBit(uint32_t bit) 
{
	_memoryMap[bit / 32] &= ~(1 << (bit % 32));
}

// Test if any bit is set within the memory map bit array
//
// This tests to see if a block is in use

bool MemoryMapTestBit(uint32_t bit) 
{
	return _memoryMap[bit / 32] &  (1 << (bit % 32));
}

// Find first free block in the bit array and returns its index

uint32_t MemoryMapFindFirstFree() 
{
	for (uint32_t i = 0; i < _memoryMapSize; i++)
	{
		// If the entire dword is marked as in-use, we do not
		// need to check every bit
		if (_memoryMap[i] != 0xffffffff)
		{
			for (int j = 0; j < 32; j++) 
			{				
				// Test each bit in the dword
				uint32_t bit = 1 << j;
				if (!(_memoryMap[i] & bit))
				{
					// Return offset of bit in bitmap
					return i * 32 + j;
				}
			}
		}
	}
	// Indicate that no free blocks have been found
	return 0xFFFFFFFF;
}

// Finds first free "size" number of blocks and returns its index

uint32_t MemoryMapFindFirstFreeSize(size_t size) 
{
	if (size == 0)
	{
		return 0xFFFFFFFF;
	}
	uint32_t startingBit = MemoryMapFindFirstFree();
	if (size == 1 || startingBit == 0xFFFFFFFF)
	{
		return startingBit;
	}
	while (startingBit + size <= _maximumBlockCount)
	{
		// Search for first free bit
		while (MemoryMapTestBit(startingBit))
		{
			startingBit++;
		}
		// Now look at the following bits to see if there are enough free contiguous blocks
		uint32_t freeBlocks = 0;
		while (freeBlocks < size && !MemoryMapTestBit(startingBit + freeBlocks))
		{
			freeBlocks++;
		}
		if (freeBlocks == size)
		{
			return startingBit;
		}
		// Adjust startingBit to point to the next non-free block
		startingBit = startingBit + freeBlocks;
	}
	return 0xFFFFFFFF;
}

// Initialise the physical memory manager
//
// On entry: memSize = Amount of memory
//			 bitmap  = Address of the area of memory we will use for our bitmap

uint32_t PMM_Initialise(BootInfo * bootInfo, uint32_t bitmap) 
{
	MemoryRegion *	region = bootInfo->MemoryRegions;
	uint32_t totalAddressableMemory;
	uint32_t amountOfAvailableMemory = 0;
	int i = 0;
	while (i == 0 || region[i].StartOfRegionLow != 0)
	{
		if (region[i].Type == MEMORY_REGION_AVAILABLE)
		{
			amountOfAvailableMemory += region[i].SizeOfRegionLow;
			totalAddressableMemory = region[i].StartOfRegionLow + region[i].SizeOfRegionLow;
		}
		i++;
	}
	_physicalMemorySize	= amountOfAvailableMemory  / 1024;
	if (bitmap % PMM_BLOCK_SIZE != 0)
	{
		bitmap = (bitmap / PMM_BLOCK_SIZE + 1) * PMM_BLOCK_SIZE;
	}	
	_memoryMap = (uint32_t*)bitmap;
	_maximumBlockCount = amountOfAvailableMemory / PMM_BLOCK_SIZE;
	_usedBlocks	= _maximumBlockCount;

	uint32_t sizeOfMemoryMap = totalAddressableMemory / PMM_BLOCK_SIZE / PMM_BLOCKS_PER_BYTE;
	if (sizeOfMemoryMap % 4 != 0)
	{
		sizeOfMemoryMap = (sizeOfMemoryMap / 4 + 1) * 4;
	}	
	_memoryMapSize = sizeOfMemoryMap / 4;
	// By default, all of memory is in use
	memset(_memoryMap, 0xff, sizeOfMemoryMap );
	i = 0;
	while (i == 0 || region[i].StartOfRegionLow != 0)
	{
		if (region[i].Type == MEMORY_REGION_AVAILABLE)
		{
			PMM_MarkRegionAsAvailable(region[i].StartOfRegionLow, region[i].SizeOfRegionLow);
			
		}
		i++;
	}
	return sizeOfMemoryMap;
}

// Mark an area of physical memory as being available for use

void PMM_MarkRegionAsAvailable(uint32_t base, size_t size) 
{
	uint32_t align = base / PMM_BLOCK_SIZE;
	uint32_t offsetInBlock = base % PMM_BLOCK_SIZE;
	uint32_t adjustedSize = offsetInBlock == 0 ? size : (PMM_BLOCK_SIZE - offsetInBlock) + size;
	int blockCount = adjustedSize / PMM_BLOCK_SIZE;
	if (adjustedSize % PMM_BLOCK_SIZE != 0)
	{
		blockCount++;
	}
	for (int blocks = blockCount; blocks > 0; blocks--) 
	{
		MemoryMapClearBit(align++);
		_usedBlocks--;
	}
}

// Mark a region of physical memory as being unavailable for use

void PMM_MarkRegionAsUnavailable(uint32_t base, size_t size) 
{
	uint32_t align = base / PMM_BLOCK_SIZE;
	uint32_t offsetInBlock = base % PMM_BLOCK_SIZE;
	uint32_t adjustedSize = offsetInBlock == 0 ? size : (PMM_BLOCK_SIZE - offsetInBlock) + size;
	int blockCount = adjustedSize / PMM_BLOCK_SIZE;
	if (adjustedSize % PMM_BLOCK_SIZE != 0)
	{
		blockCount++;
	}
	for (int blocks = blockCount; blocks > 0; blocks--) 
	{
		MemoryMapSetBit(align++);
		_usedBlocks++;
	}
}

// Allocate a single memory block

void* PMM_AllocateBlock() 
{
	if (PMM_GetFreeBlockCount() <= 0)
	{
		// We are out of memory
		return 0;	
	}
	uint32_t frame = MemoryMapFindFirstFree();
	if (frame == 0xFFFFFFFF)
	{
		// Unable to allocate a block of memory
		return 0;	
	}
	// Set the block as being used
	MemoryMapSetBit(frame);
	// Convert to a physical address
	uint32_t addr = frame * PMM_BLOCK_SIZE;
	_usedBlocks++;
	return (void*)addr;
}

// Free a single memory block

void PMM_FreeBlock(void* p) 
{
	uint32_t addr = (uint32_t)p;
	uint32_t frame = addr / PMM_BLOCK_SIZE;

	MemoryMapClearBit(frame);
	_usedBlocks--;
}

// Allocate size blocks of memory

void * PMM_AllocateBlocks(size_t size) 
{
	if (PMM_GetFreeBlockCount() <= size)
	{
		// Not enough free space
		return 0;	
	}
	uint32_t frame = MemoryMapFindFirstFreeSize(size);
	if (frame == 0xFFFFFFFF)
	{
		// Not enough space
		return 0;	
	}
	for (uint32_t i=0; i < size; i++)
	{
		// Mark the memory as used
		MemoryMapSetBit(frame+i);
	}
	uint32_t addr = frame * PMM_BLOCK_SIZE;
	_usedBlocks += size;
	return (void*)addr;
}

// Free size blocks

void PMM_FreeBlocks(void* p, size_t size) 
{
	uint32_t addr = (uint32_t)p;
	uint32_t frame = addr / PMM_BLOCK_SIZE;

	for (uint32_t i = 0; i < size; i++)
	{
		// Mark the memory as freed
		MemoryMapClearBit(frame+i);
	}
	_usedBlocks -= size;
}

// Get the amount of physical memory

size_t PMM_GetAvailableMemorySize() 
{
	return _physicalMemorySize;
}

// Get the total number of blocks

uint32_t PMM_GetAvailableBlockCount() 
{
	return _maximumBlockCount;
}

uint32_t PMM_GetUsedBlockCount() 
{
	return _usedBlocks;
}

uint32_t PMM_GetFreeBlockCount() 
{
	return _maximumBlockCount - _usedBlocks;
}

uint32_t PMM_GetBlockSize() 
{
	return PMM_BLOCK_SIZE;
}

uint32_t PMM_GetMemoryMap()
{
	return (uint32_t)_memoryMap;
}

