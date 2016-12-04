// Bios Parameter Block

#ifndef BPB_H
#define BPB_H

#include <stdint.h>

typedef struct _BIOSParameterBlock 
{
	uint8_t			OEMName[8];
	uint16_t		BytesPerSector;
	uint8_t			SectorsPerCluster;
	uint16_t		ReservedSectors;
	uint8_t			NumberOfFats;
	uint16_t		NumDirEntries;
	uint16_t		NumSectors;
	uint8_t			Media;
	uint16_t		SectorsPerFat;
	uint16_t		SectorsPerTrack;
	uint16_t		HeadsPerCyl;
	uint32_t		HiddenSectors;
	uint32_t		LongSectors;
} __attribute__((packed)) BIOSParameterBlock;

typedef BIOSParameterBlock * pBIOSParameterBlock;

// Bios Parameter Block extended attributes

typedef struct _BIOSParameterBlockExt
{
	uint32_t			SectorsPerFat32;
	uint16_t			Flags;
	uint16_t			Version;
	uint32_t			RootCluster;
	uint16_t			InfoCluster;
	uint16_t			BackupBoot;
	uint16_t			Reserved[6];
} __attribute__((packed)) BIOSParameterBlockExt;

typedef BIOSParameterBlockExt * pBIOSParameterBlockExt;

// Boot sector

typedef struct _BootSector
{
	uint8_t					Ignore[3];			
	BIOSParameterBlock		Bpb;
	BIOSParameterBlockExt	BpbExt;
	uint8_t					Filler[448];  // Padding out to 512 bytes		
} __attribute__((packed)) BootSector;

typedef BootSector * pBootSector;

#endif
