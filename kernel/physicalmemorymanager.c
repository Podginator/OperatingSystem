// The Implementation of the physical memory management.
#include "physicalmemorymanager.h"
#include <string.h>

// The amount of blocks
#define PMM_BLOCK_SIZE 4096

// The memory bitmap, as a 32 bit array pointer.
uint32_t *_memBitmap = 0;

// The amount of total blocks.
uint32_t _totalBlocks = 0;

// The Total amount of blocks in use.
uint32_t _usedBlocks = 0;

//Declare Private Functions
inline static uint32_t getIndex(uint32_t);
inline static uint32_t getSize(MemoryRegion *);

// Returns the frame
// @param base : The base of the address to get the frame from
// @return The Index of the Frame at base.
inline static uint32_t getIndex(uint32_t size)
{
    // Get the remainder, if it's not 0 round up to the next instance of PMM_BLOCK_SIZE
    uint32_t remainder = size % PMM_BLOCK_SIZE;
    if (remainder == 0)
    {
        return (size / PMM_BLOCK_SIZE);
    }

    return (size + (PMM_BLOCK_SIZE - remainder)) / PMM_BLOCK_SIZE;
}

// Get Size
// @param region : Pointer to the memory region
// @return The Size Of All the mem stored in the memory region, including gaps.
inline static uint32_t getSize(MemoryRegion *region)
{
    MemoryRegion *temp = region;
    MemoryRegion *lastPtr = temp;
    int i = 0;
    do
    {
        lastPtr = temp->Type == 1 ? temp : lastPtr;
        temp++;
    } while (temp->StartOfRegionLow != 0);

    return (uint32_t)(lastPtr->StartOfRegionLow + lastPtr->SizeOfRegionLow);
}

// Initialise the physical memory manager
// @param bootInfo the Information about the BootDevice
// @param bitmap   the location of the bitmap in memory
// @return The Size of the bitmap in bytes.
uint32_t PMM_Initialise(BootInfo *bootInfo, uint32_t bitmap)
{
    // Get the total uint32_t
    MemoryRegion *region = bootInfo->MemoryRegions;
    // Ensure that the _totalSize is divisible by 4096, this should echo down
    // So that the totalblocks is divisble by 32 and byteSize is divisible by 4. 
    uint32_t totalSize = getSize(region);
    _totalBlocks = totalSize / PMM_BLOCK_SIZE;
    _totalBlocks += _totalBlocks % 32; 
    _usedBlocks = _totalBlocks;
    _memBitmap = (uint32_t *)bitmap;
    uint32_t totalBlockStorageSize = (_totalBlocks / 8);

    // Then set them all as used
    memset((void *)_memBitmap, ~0, totalBlockStorageSize);

    // // TEST:: PMM_AvailableCount
    // uint32_t totalBlocksAvailable = PMM_GetAvailableBlockCount();
    // uint32_t totalSetBlocks = PMM_GetUsedBlockCount();

    // ConsoleWriteString("\nTotal Blocks: ");
    // ConsoleWriteInt(totalBlocksAvailable, 10);
    // if (totalBlocksAvailable == totalSetBlocks) 
    // {
    //     ConsoleWriteString("\nAvailable (Total) Same as used after Memset");
    // }

    // if (totalBlocksAvailable == 8192) 
    // { 
    //     ConsoleWriteString("\nExpected Amount Returned in Bochs 32mb");
    // }

    // TEST :: (Have we assigned all the bits correctly)
    // __asm__("xchg %bx, %bx \n\t");


    // Set the regions that are available for us to use.
    do
    {
        if (region->Type == 1)
        {
            PMM_MarkRegionAsAvailable(region->StartOfRegionLow, region->SizeOfRegionLow);
        }
        region++;
    } while (region->StartOfRegionLow != 0);

    return totalBlockStorageSize;
}

// Mark a region as being available for use
// @param base : where we want to set the memory is available.
// @param size : the size of the region we want to claim.
void PMM_MarkRegionAsAvailable(uint32_t base, size_t size)
{
    // find the Block it's located in and the number needed.
    uint32_t blockOffset = base % PMM_BLOCK_SIZE;
    // This should round the block location down to the nearest PMM_BLOCK_SIZE
    uint32_t blockLoc = ((base - blockOffset) / PMM_BLOCK_SIZE);
    // This should adjust the size to accomodate for the rounding down.  
    uint32_t index = getIndex(size + blockOffset);

    // Ensure that we cant' set any regions above the total amount of blocks
    if ((blockLoc + index) > _totalBlocks)
    {
        return;
    }

    while (index)
    {
        // If the bit is already available, no need to decrement.
        // Use these variables to avoid recalculating them when we set.
        uint32_t bitFiddle = 0x1 << (blockLoc % 32);
        uint32_t bitIndex = blockLoc / 32;
        if (_memBitmap[bitIndex] & bitFiddle)
        {
            // Index into the nearest address of an 32bit array
            _memBitmap[bitIndex] &= ~bitFiddle;
            _usedBlocks--;
        }
        // Decrement the index,
        index--;
        blockLoc++;
    }
}

// Mark a region as not being available for use
// @param base : where we want to set the memory is available.
// @param size : the size of the region we want to claim.
void PMM_MarkRegionAsUnavailable(uint32_t base, size_t size)
{
    // find the Block it's located in and the number needed.
    uint32_t blockOffset = base % PMM_BLOCK_SIZE;
    // This should round the block location down to the nearest PMM_BLOCK_SIZE
    uint32_t blockLoc = ((base - blockOffset) / PMM_BLOCK_SIZE);
    // This should adjust the size to accomodate for the rounding down.  
    uint32_t index = getIndex(size + blockOffset);

    // Is this correct?
    if (blockLoc + index > _totalBlocks)
    {
        return;
    }

    while (index)
    {
        // If the bit is already available, no need to decrement.
        uint32_t bitFiddle = 0x1 << (blockLoc % 32);
        uint32_t bitIndex = blockLoc / 32;
        if (!(_memBitmap[bitIndex] & bitFiddle))
        {
            // Index into the nearest address of an 32bit array
            _memBitmap[bitIndex] |= bitFiddle;
            _usedBlocks++;
        }

        blockLoc++;
        index--;
    }
}

// Allocate a single memory block
// Here we can just use the BSR instruction with a ~ to find where the first
// 0 in a 32bit map is. Once we've found a free memory block.ah
// This saves potentially 31 jumps. (If the 0 is in the MSB.)
// @return the returned memory address.
void* PMM_AllocateBlock()
{
    uint32_t* temp = _memBitmap;
    uint32_t iter =  _totalBlocks >> 5;

    if (PMM_GetFreeBlockCount() != 0) 
    {
        // Temp ++ Iterates over 32 bit regions of bits. 
        for (uint32_t i = 0; i < iter; i++, temp++) 
        {
            int tempVal = *temp;
            // Are all the bits set? 
            if (tempVal != ~0)
            {
                int ctz = __builtin_ctz(~tempVal);
                uint32_t memAddr = (((i * 32) + ctz) * PMM_BLOCK_SIZE);
                PMM_MarkRegionAsUnavailable(memAddr, PMM_GetBlockSize());
                return (void*) memAddr;
            }
        }
    }

    // We've failed. Return NULL.
    return (void*) NULL;
}

// Free a single memory block
// @param the memory we want to free.
void PMM_FreeBlock(void *p)
{
    uint32_t addr = (uint32_t)p;

    // We should never attempt to free null
    if (addr != NULL)
    {
        PMM_MarkRegionAsAvailable(addr, 1);
    }
}

// uses __builtin_ctz(num) to count the trailing bits using the BSR instruction
// This reduces the amount of loops and replaces it with a performant assembly instruction.
// This is faster than a straight for loop. 
// @param needed : the amount of frames needed
// @return the free memory, or NULL if no memory available.
void* PMM_AllocateBlocks(size_t needed)
{
    // First, check that we have enough blocks available before
    // looping at all. This avoids needless cycles.
    if (needed <= PMM_GetFreeBlockCount())
    {
        // Instantiate variables inside, as we only need them in this block,
        // and it is more efficient to do so.
        uint32_t storedCounter = 0;
        // Total = TotalBlocks / 32 (or >> 5)
        uint32_t total = (_totalBlocks >> 5);
        uint32_t *temp = _memBitmap;
        void *index = NULL;

        // Temp ++ Iterates over 32 bit regions of blocks.
        for (uint32_t i = 0; i < total; i++, temp++)
        {
            uint32_t tempVal = (uint32_t)*temp;
            // Are all the bits set? If so we can skip this block.
            if (tempVal != ~0)
            {
                
                int seenCounter = 0;
                // Iterate through until we've seen all _totalBlocks
                while (seenCounter < 32)
                {
                    //  __builtin_ctz() uses assembly instruction BitScan, which is very efficient at finding trailing bitSet
                    //  It has undefined behaviour with 0, but we know in this case that it'll be 32 trailing 0s.
                    int ctz = (tempVal != 0) ? __builtin_ctz(tempVal) : 32;
                    seenCounter += ctz;
                    storedCounter += ctz;

                    if (storedCounter >= needed)
                    {
                        // If we've found enough bits (there were >= needed trailing bits)
                        // And we weren't just trying to get to the next 0 in the LSB.
                        // Then return the index of the free 'contiguous bits'
                        index = (void *)(((i * 32) + seenCounter - needed - (storedCounter - needed)) * PMM_BLOCK_SIZE);
                        PMM_MarkRegionAsUnavailable((uint32_t)index, needed * PMM_BLOCK_SIZE);

                        // If we didn't the above statement we'll return null here.
                        return index;
                    }

                    // rorl so that during the next iteration we are observing a set of free bits
                    asm("rorl %1, %0 "
                        : "+r"(tempVal)
                        : "c"((uint8_t)ctz));

                    // Then find the next 0 bit and ensure that the next iteration will have a '0' in the lsb.
                    ctz = (~tempVal != 0) ? __builtin_ctz(~tempVal) : 32;

                    if (ctz > 0 && seenCounter < 32)
                    {
                        seenCounter += ctz;
                        // if there were trailing zeros we've hit a one, so reset our variables.
                        storedCounter = 0;
                        // And rotate to get the 0bit in the lsb.
                        asm("rorl %1, %0 "
                            : "+r"(tempVal)
                            : "c"((uint8_t)ctz));
                    }
                }
            }
            else
            {
                // If we went from 00000000 to 11111111
                // We wouldn't be able to reset the stored.
                // So reset it here.
                storedCounter = 0;
            }
        }
    }

    // We've failed. Return NULL
    return (void*) NULL;
}

// Free size blocks
// @param The Memory we want to free
// @param The size of the memory location we want to free?
void PMM_FreeBlocks(void *p, size_t size)
{
    //Convert to mem location
    uint32_t baseLoc = (uint32_t)p;

    // We should never attempt to free null
    if (baseLoc != NULL)
    {
        PMM_MarkRegionAsAvailable(baseLoc, (size * PMM_BLOCK_SIZE));
    }
}

// Get the amount of available physical memory (in K)
// @return the size of the available memory in kb.
uint32_t PMM_GetAvailableMemorySize()
{
    // The free blocks * 4096 (Bytes in a block) / 1024
    return (PMM_GetFreeBlockCount() * PMM_BLOCK_SIZE) >> 10;
}

// Get the total number of blocks of available memory
// @return the available blocks. (Is this how many are available, or the total. I don't know.)
uint32_t PMM_GetAvailableBlockCount()
{
    return _totalBlocks;
}

// Get the number of used blocks of available memory
// @return the used blocks
uint32_t PMM_GetUsedBlockCount()
{
    return _usedBlocks;
}

// Get the number of free blocks of available memory
// @return the free blocks remaining. (Is this not available)
uint32_t PMM_GetFreeBlockCount()
{
    return _totalBlocks - _usedBlocks;
}

// Get the size of a block
// @return the Block Size
uint32_t PMM_GetBlockSize()
{
    return PMM_BLOCK_SIZE;
}

// Return the address of the memory map
// @return the address to the mem map
uint32_t PMM_GetMemoryMap()
{
    // We have to convert it back to a 32 bit address before returning
    return (uint32_t) _memBitmap;
}