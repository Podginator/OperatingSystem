#include <filesystem.h>
#include <bpb.h>
#include <floppydisk.h>
#include <stdint.h>
#include <size_t.h>
#include <_null.h>
#include <string.h>

#define DIR_DIRECTORY  0x10

// Store the offsets
uint32_t offsetFat;
uint32_t offsetRoot;
uint32_t offsetData;
uint32_t rootSize; 

// Store info into the FAT Table.
uint8_t FAT_Table[9 * SECTORS_PER_FAT_SECTOR];

// Tempory Entries to temporarily store when navigating.
DirectoryEntry _tempEntries[ENTRIES_PER_ROOT];

// ** Forward Declarations ** 
static inline void ExtractNextEntry(const char** filePath, char* filenameBuffer);
static inline FILE ConvertToFile(pDirectoryEntry entry, char* name);


// Convert a pointer to a directory entry to a FILE structure. 
// @param entry the entry to convert
// @param the name to give the file.
static inline FILE ConvertToFile(pDirectoryEntry entry, char* name)
{
    FILE file; 
    strcpy(file.Name, name);
    file.Eof = 0;
    file.Position = 0;
    file.CurrentCluster = entry->FirstCluster;
    file.FileLength = entry->FileSize;
    file.Flags = (entry->Attrib & 0x10) ? FS_DIRECTORY : FS_FILE;
    return file;
}

// Extract the name and extn from a correctly formatted file path 
// @param filePath the filepath to extract from (We Increment this each time)
// @param fileNameBuffer the buffer to put the filename in 
// @param extBuffer the buffer to put the ext in .`
// @return parsed valid file path
static inline void ExtractNextEntry(const char** filePath, char* filenameBuffer)
{
    // Ignore the first backslash
    if (*filePath[0] == '\\')
    {
        (*filePath)++;
    }
    
    int needle = strchr(*filePath, '\\');    
    if (needle == -1)
    {
        needle = strlen(*filePath); 
    }
    memcpy(filenameBuffer, *filePath, needle);
    
    // Nullterminate it.
    *(filenameBuffer + needle) = 0;
    *filePath += needle + 1;
}

// Get Name From DirectoryEntry 
// @param entry the directoy
// @param buffer OUT the buffer to write the name to 
// @param isLFN is this Long File Name?
void FsFat12_GetNameFromDirectoryEntry(pDirectoryEntry entry, char* buffer, bool isLFN)
{
    char* temp = buffer;
    if (!isLFN)
    {
        char* name = entry->Filename;
        int counter = 0;
        while(*name != ' ' && counter <= 8)
        {
            *temp++ = *name++;
            counter++;
        }

        if (!(entry->Attrib & 0x10))
        {
            *temp++ = '.';
            for (size_t i = 0; i < 3; i++)
            {
                *temp++ = entry->Ext[i];
            }
        }
    }
    else
    {

        bool ok = true;
        entry--;
        while (entry->Attrib == 0x0f && ok)
        {
            // There are 13 UTF-16 Characters in a LFN entry.
            uint16_t* utfs = ((pLongFileNameEntry) entry)->Filename_One;

            for (size_t i = 0; i < 13 && (ok = *utfs); i++)
            {
                // Otherwise copy 
                *temp++ = (char) *utfs++;
                
                if (i == 4) 
                {
                    utfs = ((pLongFileNameEntry) entry)->Filename_Two;
                }
                else if (i == 10)
                {
                    utfs = ((pLongFileNameEntry) entry)->Filename_Three;
                }
            }
            entry--;
        }
        
    }

    *temp = 0;
}

// Converts Sector Number to Directory  
// @param sectorNum the sector number
// @param entry - entry to write to 
// @pre : StoreBuffer should be the correct size. Which is 512 Bytes if copying non Directories
// @return Directory Entries Copied. 
size_t FsFat12_GetDirectoryFromSector(uint32_t sectorNum, pDirectoryEntry storeBuffer) 
{
    if (sectorNum >= 2) 
    {
        // Copy entire directory structure
        memcpy(storeBuffer, FloppyDriveReadSector(PHYSICAL_PADDING + offsetFat + (sectorNum - 2)), 512);

        return ENTRIES_PER_SECTOR;
    } 
    else 
    {
        // Copy the Entire Root Directory into here. 
        for (size_t i = 0; i < ROOT_DIRECTORY_SECTOR_SIZE; i++)
        {
            pDirectoryEntry tempDir = (pDirectoryEntry) FloppyDriveReadSector(offsetRoot + i);
            memcpy(((char*) storeBuffer) + (i << 9), tempDir, BYTES_PER_SECTOR); 

            // No More Directories found. Do not copy anymore after this 
            // Copy this one to ensure any loops terminating on names terminate correctly.  
            if (tempDir[0].Filename[0] == 0x00) 
            {
                break;
            }

        }

        return ENTRIES_PER_ROOT;
    }
}

// Initialize the file system.
void FsFat12_Initialise()
{
    // Retrieve the Bios Parameter Block
    pBootSector startSector = (pBootSector) FloppyDriveReadSector(0);

    // Offset to the first FAT
    offsetFat = startSector->Bpb.ReservedSectors;
    // Offset to the Root from the start
    offsetRoot = (startSector->Bpb.NumberOfFats *  startSector->Bpb.SectorsPerFat) + offsetFat;
    // Calculate the root size as the number of sectors.
    rootSize = (startSector->Bpb.NumDirEntries * ENTRY_SIZE) / startSector->Bpb.BytesPerSector;
    // Data is RootSize + Offset to the root.
    offsetData = offsetRoot + rootSize;


    size_t sizePerFat = startSector->Bpb.SectorsPerFat;
    for (size_t i = 0; i < sizePerFat; i++)
    {
        memcpy(FAT_Table + (i << 9), FloppyDriveReadSector(offsetFat + i), 512);
    }

}

// Open a file
// This starts from the Root.
// @param filename - filename
// @return the opened file, set to FS_INVALID if not found.
FILE FsFat12_Open(const char* filename)
{
    FILE res; 
    res.Flags = FS_INVALID; 
    int i = 0;

    // Handle the case where we're just retrieving the root directory
    if (strcmp(filename, "\\") == 0)
    {
        res.Flags = FS_DIRECTORY;
        res.CurrentCluster = 0;
        strcpy(res.Name, "\\");
        return res;
    }

    // Retrieve the initial Directory.
    for (size_t i = 0; i < ROOT_DIRECTORY_SECTOR_SIZE; i++)
    {
        memcpy(_tempEntries, FloppyDriveReadSector(offsetRoot + i), BYTES_PER_SECTOR); 
        pDirectoryEntry directory = (pDirectoryEntry) _tempEntries;

        if (directory->Filename[0] != 0x00) 
        {
            // Traverse the directory, hopefully finding more..
            res = FsFat12_OpenFrom(directory, filename);
            // We've either not traversed the directory at all 
            // or we have but have still not found a file. 
            if (!(res.Flags & FS_INVALID) || (res.Flags & FS_TRAVERSED))
            {   
                // In either case we either return a found file
                // Or nothing. This helps us terminate the loops early.
                return res;
            }
        } 
        else 
        {
            // There are no more files in this directory. Fail.
            return res;
        }
    }
        
    // Otherwise loop through until we get to the sub directory. 
    return res;
}

// Traverse the directory upwards to the filepath we pass in
// @param entry - the initial entry we wish to traverse from (usually from the root)
// @param filePath - the file path we wish to reach
// @return The FILE we have found, flag set to INVALID if file not found.
FILE FsFat12_OpenFrom(pDirectoryEntry entrySector, const char* filePath) 
{
    bool done = false;
    const char* temp = filePath;

    memcpy(_tempEntries, entrySector, 512);
    pDirectoryEntry tempEntry = (pDirectoryEntry) _tempEntries;
    FILE failed;
    failed.Flags = FS_INVALID;
    size_t entries = ENTRIES_PER_SECTOR; 

    // Declare nextFilename outside the scope of the while loop as we don't want to create and lose 
    char nextFilename[255];
    while (!done)
    {
        bool isLFN = false;
        ExtractNextEntry(&temp, nextFilename);

        // if we've reached the end of the path we're done. (IE: We've hit null in the path)
        done = !(*temp);
        for (size_t j = 0; j < entries; j++, tempEntry++)
        {

            // This directory entry is free.
            if (tempEntry->Filename[0] == 0xE5) 
            {
                continue;
            }

            // The first byte of the file name is empty. Meaning the rest of the directories 
            // Are free in this entry
            if (tempEntry->Filename[0] == 0x00) 
            {
                // There are no remaining files in this directory.
                return failed;
            }
 

            if (tempEntry->Attrib != 0x0F)
            {
  
                char filename[256];
                FsFat12_GetNameFromDirectoryEntry(tempEntry, filename, isLFN);

                // TODO: Case Insensitive Or NOT? have a strcasecmp
                // But since adding Long File Names seems redundant. Rather it work like BASH.
                if (strcmp(filename, nextFilename) == 0) 
                {
                    // We have traversed the file slighty.
                    // This indicates we have traversed the fs slightly
                    // If we failed now we know that the file doesn't exist. 
                    failed.Flags |= FS_TRAVERSED;
                    
                    if (done)
                    {
                        return ConvertToFile(tempEntry, filename);
                    }                    
                    else 
                    {
                        // If we're not at the last path
                        if (tempEntry->Attrib & DIR_DIRECTORY) 
                        {
                            // this should indicate how many entries we need to check on the subsequent read. 
                            // If we hit the root it'll be 224, otherwise it'll be 16.
                            entries = FsFat12_GetDirectoryFromSector(tempEntry->FirstCluster, _tempEntries);
                            tempEntry = (pDirectoryEntry) _tempEntries;
                        }
                        //Break and repeat the outer loop.
                        break;
                    }
                }
            }

            isLFN = tempEntry->Attrib == 0x0F;
        }
    }

    return failed;
}

// Read from a file system
// @param file - file to read
// @param buffer - buffer to read to)
// @param length - length to read to. 
// @return The Length of what we read.
// @pre/post : to be considered: This doesn't expect to be called multiple times,
//        if the same buffer the programmer should memcpy the buffer themselves if permanence is needed.
unsigned int FsFat12_Read(PFILE file, unsigned char* buffer, unsigned int length)
{
    int read = 0;
    if (file != NULL)
    {
        // Get the Max, the file Length or the length of the file. We should not read 
        // Over the length of the file.     
        // If the pointer exists continue
        int totalFileRemaining = (file->FileLength - file->Position);
        int lenRemaining = length > totalFileRemaining  ? totalFileRemaining : length;
        int remainder = file->Position % BYTES_PER_SECTOR;

        // What's our increment? Max we can pass is a sector. 
        int increment = length > BYTES_PER_SECTOR ? BYTES_PER_SECTOR : length;

        // We should check CurrentCluster is correct and the LenRemaining is greater than 0
        // We don't want to evaluate this EVERYTIME, though;
        bool ok = (file->CurrentCluster > 2) && (lenRemaining > 0);
        unsigned char* temp = buffer; 
        while (ok)
        {  
            // Todo make better.
            // Basically len = min(512, min(bytes_per_sector - remainder, length))
            // there's gotta be a better way osf doing that.
            int len = BYTES_PER_SECTOR - remainder;
            len = len > increment ? increment : len; 
            len = lenRemaining > len ? len : lenRemaining;

            // Copy into a buffer and increment the remainder
            memcpy(temp, FloppyDriveReadSector(PHYSICAL_PADDING + offsetFat + (file->CurrentCluster - 2)) + remainder, len); 

            // Modify the variables for the next passthrough
            lenRemaining -= len;
            temp += len;
            read += len;
            file->Position += len;
            remainder = file->Position % BYTES_PER_SECTOR;
            
            // n == even (low four bits in location 1+(3*n)/2 with 8 bits in location (3*n)/2
            // n == odd, high four bits in location (3*n)/2 with 8 bits in location 1+(3*n)/2 

            //(3 * n) / 2 is equivalent to  1.5 * n (We can use a bitshift, which should be more efficient.)
            size_t baseInd = file->CurrentCluster + (file->CurrentCluster >> 1);

            // Set the cluster to be the next one in the FAT Map. 
            // Rules Dictated in the comments above.
            if (remainder == 0) 
            {
                file->CurrentCluster = (file->CurrentCluster % 2 == 0) 
                                        ?  ((FAT_Table[1 + baseInd] & 0x0F) << 8) | FAT_Table[baseInd] 
                                        :  ((FAT_Table[baseInd]) >> 4) |  (FAT_Table[1 + baseInd] << 4); 

                // We have hit the end of the current cluster, either by hitting the END flag 
                // OR by hitting something invalid.
                if (file->CurrentCluster >= 0xff8 || file->CurrentCluster == 0x00) 
                { 
                    // Set the flag then break, meaning we will return whatever length was remaining.
                    file->Eof = 1;
                    break;
                }
            }

            ok = lenRemaining > 0;
        }

        if (file->Position == file->FileLength)
        {
            file->Eof = 1;
        }

        // We might not read everything, so work out how much we have read.
        return read;
    }

    return 0;
}

// Close the file
// @param file - Pointer to the file to close
void FsFat12_Close(PFILE file)
{
    if (file != NULL)
    { 
        // Indicate we're at the end of a file
        file->Eof = 1; 
    }
}