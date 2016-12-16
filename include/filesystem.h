// Filesystem Definitions

#ifndef _FSYS_H
#define _FSYS_H

#include <stdint.h>

//	File flags (Bit Flags)
#define FS_FILE       0b1
#define FS_DIRECTORY  0b10
#define FS_INVALID    0b100

// Statics
#define ENTRY_SIZE 32
#define PHYSICAL_PADDING 32
#define BYTES_PER_SECTOR 512
#define SECTORS_PER_FAT_SECTOR 512
#define ROOT_DIRECTORY_SECTOR_SIZE 14
#define ENTRIES_PER_SECTOR 16

// File
typedef struct _File 
{
	char        Name[256];
	uint32_t    Flags;
	uint32_t    FileLength;
	uint32_t    Id;
	uint32_t    Eof;
	uint32_t    Position;
	uint32_t    CurrentCluster;
} FILE;

typedef FILE * PFILE;

typedef struct _DirectoryEntry 
{
	uint8_t   Filename[8];
	uint8_t   Ext[3];
	uint8_t   Attrib;
	uint8_t   HasLFN;  			// Use a previously reserved byte to store whether we have a LFN 
	uint8_t   TimeCreatedMs;
	uint16_t  TimeCreated;
	uint16_t  DateCreated;
	uint16_t  DateLastAccessed;
	uint16_t  FirstClusterHiBytes;
	uint16_t  LastModTime;
	uint16_t  LastModDate;
	uint16_t  FirstCluster;
	uint32_t  FileSize;

} __attribute__((packed)) DirectoryEntry;

typedef DirectoryEntry * pDirectoryEntry;


// Bytes   Content
// 0       Bits 0-4: sequence number; bit 6: final part of name
// 1-10    Unicode characters 1-5
// 11      Attribute: 0xf
// 12      Type: 0
// 13      Checksum of short name
// 14-25   Unicode characters 6-11
// 26-27   Starting cluster: 0
// 28-31   Unicode characters 12-13


// The long file name entry, packed slightly differently.
// Uses utf-16(??)
typedef struct _LongFileNameEntry
{
	uint8_t  SequenceNumber;
	uint16_t Filename_One[5];
	uint8_t  Flag;
	uint8_t  Reserved;
	uint8_t  CheckSum;
	uint16_t Filename_Two[6];
	uint16_t Reserved_Two; 
	uint16_t Filename_Three[2];
} __attribute__((packed)) LongFileNameEntry;
typedef LongFileNameEntry * pLongFileNameEntry;

// A Function pointer that uses a pDirectoryEntry and a set of pointers
typedef bool (*DirectoryDelegate)(pDirectoryEntry, uintptr_t* ptrs);


// A temporary buffer to store a Sectors worth of entries.
// Extern so that we may use it elsewhere.
extern DirectoryEntry _tempEntries[ENTRIES_PER_SECTOR];

// Many of the delegates need to keep track of whether we are 
// looking at a LFN
extern bool _delegateIsLFN;

// A Temporary Buffer used for Autocomplete and ChangeDirectory.
extern char  _tempBuffer[2048];

// Temporary storage for a _LFN, as we call this multiple times
extern char _longFileName[256];

bool FsFat12_HandleName(pDirectoryEntry entry, char* fname);

// Get Name From DirectoryEntry 
// @param entry the directoy
// @param buffer OUT the buffer to write the name to 
// @param isLFN is this Long File Name?
void FsFat12_GetNameFromDirectoryEntry(pDirectoryEntry entry, const char* name);

// Get Name From DirectoryEntry 
// @param entry the directoy
// @param buffer OUT the buffer to write the name to 
// @param isLFN is this Long File Name?
void FsFat12_BuildLongFileName(pLongFileNameEntry entry, char* buffer);

// Initialise the fs
void FsFat12_Initialise();

// Open a file
// @param filename - filename
FILE FsFat12_Open(const char* filename);

// Open a file from a directory
// @ param entry the entry to run from 
// @param filename - filename
FILE FsFat12_OpenFrom(FILE dir, const char* filePath);


// Iterate a folder applying a function. 
// If the FileFunction returns true quite iterating. 
// @param the directory we wish to iterate. 
// @param DirectoryDelegate the file function 
// @param void* Passin - Variable we wish to modify, to pass in to the fileFN 
void FsFat12_IterateFolder(FILE dir, DirectoryDelegate fileFn, uintptr_t* ptrs);

// Read from a file system
// @param file - file to read
// @param buffer - buffer to read to
// @param length - length to read to. 
unsigned int FsFat12_Read(PFILE file, unsigned char* buffer, unsigned int length);

// Close the file
// @param file - Pointer to the file to close
void FsFat12_Close(PFILE file);

// Converts Sector Number to Directory  
// @param Pointer to the file
// @param entry - entry to write to 
pDirectoryEntry FsFat12_GetDirectoryFromSector(uint32_t sectorNum);


#endif
