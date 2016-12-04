#ifndef _BOOTINFO_H
#define _BOOTINFO_H

#define MEMORY_REGION_AVAILABLE  	1    
#define MEMORY_REGION_NOTAVAILABLE	2
#define MEMORY_REGION_ACPI_RECLAIM	3
#define MEMORY_REGION_ACPI_NVS		4

// Format of a memory region returned by the BIOS call to get the memory map

typedef struct _MemoryRegion 
{
	uint32_t	StartOfRegionLow;
	uint32_t	StartOfRegionHigh;
	uint32_t	SizeOfRegionLow;
	uint32_t	SizeOfRegionHigh;
	uint32_t	Type;
} MemoryRegion;


// The information passed from the boot loader

typedef struct _BootInfo
{
	uint32_t	   StackTop;		// Location of the top of stack
	uint32_t	   KernelSize;		// Size of the kernel in bytes
	MemoryRegion * MemoryRegions;	// Pointer to the memory map returned by the BIOS
	uint8_t		   BootDevice;		// Id of boot device
} BootInfo;

#endif