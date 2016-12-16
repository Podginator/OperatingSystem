#include <filesystem.h>
#include <bpb.h>
#include <floppydisk.h>
#include <stdint.h>
#include <size_t.h>
#include <_null.h>
#include <string.h>

#define DIR_DIRECTORY  0x10

// Store the offsets
static uint32_t offsetFat;
static uint32_t offsetRoot;
static uint32_t offsetData;
static uint32_t rootSize; 

// Store info into the FAT Table.
static uint8_t FAT_Table[9 * SECTORS_PER_FAT_SECTOR];

DirectoryEntry _tempEntries[ENTRIES_PER_SECTOR];

// Temp Buffer for files.
char _tempBuffer[2048];
bool _delegateIsLFN = false;

// ** Forward Declarations ** 
static inline void ExtractNextEntry(const char** filePath, char* filenameBuffer);
static inline FILE ConvertToFile(pDirectoryEntry entry, char* name);

// Delegate Declarations.
static inline void IterateSubDirectory(FILE file, DirectoryDelegate fileFn, uint32_t* ptrs);
static inline void IterateRootFolder(DirectoryDelegate fileFn, uint32_t* ptrs);
static inline bool IterateSector(pDirectoryEntry entry, DirectoryDelegate fileFn, uint32_t* ptrs);
static inline bool MatchDelegate(pDirectoryEntry entry, uint32_t* filename);

//
//   STATIC DECLARATIONS
//

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
// @param fileNameBuffer  buffer to put the filename in 
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

//
//    DELEGATE DECLARATION
//

// Iterate over a sector of Directory Entries 
// @param entry the Sector to iterate over 
// @param fileFn The File Function to apply. 
// @param ptrs Pointers to required paramters for the delegate. (If I had more time I would use VarArgs)
// @return False if we can continue, true otherwise.
bool IterateSector(pDirectoryEntry entry, DirectoryDelegate fileFn, uintptr_t* ptrs)
{
    pDirectoryEntry tempEntry = entry;
    for (size_t i = 0; i < ENTRIES_PER_SECTOR; i++, tempEntry++)
    {
        // This directory entry is free.
        if (tempEntry->Filename[0] == 0xE5) 
        {
            // But there might still be more.
            continue;
        }

        // The first byte of the file name is empty. Meaning the rest of the directories 
        // Are free in this entry
        if (tempEntry->Filename[0] == 0x00) 
        {
            // There are no remaining files in this directory.
            return true;
        }

        if (fileFn(tempEntry, ptrs))
        {
            return true;
        }
    }

    return false;
}

// Iterate over a SubDirectory
// @param dir the subdirectory to iterate over 
// @param fileFn The File Function to apply. 
// @param ptrs Pointers to required paramters for the delegate. (If I had more time I would use VarArgs)
void IterateSubDirectory(FILE dir, DirectoryDelegate fileFn, uintptr_t* ptrs)
{
    _delegateIsLFN = false;
    if (!(dir.Flags & FS_DIRECTORY))
    {
        return; 
    }

    do
    {
        FsFat12_Read(&dir, (char * )_tempEntries, BYTES_PER_SECTOR);
        if (IterateSector(_tempEntries, fileFn, ptrs))
        {
            return;
        }
       
    }
    while (!dir.Eof);
}

// Iterate over the root folder
// @param dir the subdirectory to iterate over 
// @param fileFn The File Function to apply. 
// @param ptrs Pointers to required paramters for the delegate. (If I had more time I would use VarArgs)
void IterateRootFolder(DirectoryDelegate fileFn, uintptr_t* ptrs)
{
    _delegateIsLFN = false;
    for (size_t i = 0; i < ROOT_DIRECTORY_SECTOR_SIZE; i++)
    {
        memcpy(_tempEntries, FloppyDriveReadSector(offsetRoot + i), BYTES_PER_SECTOR); 
        if (IterateSector(_tempEntries, fileFn, ptrs))
        {
            return;
        }
    }
}

// Returns true and copies a file if 
// @param entrySector The entry Sector 
// @param nextFile The next file name
// @return FILE the file we've retrieved.
static bool MatchDelegate(pDirectoryEntry entry, uintptr_t* ptrs)
{
    // Extract the pointers
    char* nextFile = (char*) ptrs[0]; 
    PFILE res = (PFILE) ptrs[1];
    res->Flags = FS_INVALID;
    pDirectoryEntry tempEntry = entry;

    char filename[255];
    if (tempEntry->Attrib != 0x0F)
    {
        FsFat12_GetNameFromDirectoryEntry(tempEntry, filename, _delegateIsLFN);
        if (strcmp(filename, nextFile) == 0) 
        {
            *res = ConvertToFile(tempEntry, filename);
            return true;
        }
    }

    _delegateIsLFN = tempEntry->Attrib == 0x0F;
    return false;
}

//
//  HEADER DECLARATIONS
//

// Initialize the file system.
void FsFat12_Initialise()
{
    // Retrieve the Bios Parameter Block
    pBootSector startSector = (pBootSector) FloppyDriveReadSector(0);

    offsetFat = startSector->Bpb.ReservedSectors;
    offsetRoot = (startSector->Bpb.NumberOfFats *  startSector->Bpb.SectorsPerFat) + offsetFat;
    rootSize = (startSector->Bpb.NumDirEntries * ENTRY_SIZE) / startSector->Bpb.BytesPerSector;
    offsetData = offsetRoot + rootSize;

    size_t sizePerFat = startSector->Bpb.SectorsPerFat;
    for (size_t i = 0; i < sizePerFat; i++)
    {
        memcpy(FAT_Table + (i << 9), FloppyDriveReadSector(offsetFat + i), 512);
    }
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
        ConsoleWriteInt(((pLongFileNameEntry) entry)->SequenceNumber, 2);
        ConsoleWriteInt(((pLongFileNameEntry) entry)->Reserved_Two, 10);
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

// Open a file 
// This starts from the Root.
// @param filename - filename
// @return the opened file, set to FS_INVALID if not found.
FILE FsFat12_Open(const char* filePath)
{
    FILE res;
    res.Flags = FS_DIRECTORY;
    strcpy(res.Name, "\\");
    res.Eof = res.CurrentCluster = res.Position = 0;

    // If we are merely requesting root then we are good. 
    if (strcmp("\\", filePath) == 0) 
    {
        return res;
    }

    uintptr_t pointers[2];    
    pointers[1] = (uintptr_t) &res;
    bool done = false;     
    do 
    {
        char nextFile[255];
        ExtractNextEntry(&filePath, nextFile);
        done = !(*filePath);

        pointers[0] = (uintptr_t) nextFile;
        FsFat12_IterateFolder(res, MatchDelegate, pointers);

        if (done && res.Flags & (FS_FILE | FS_DIRECTORY))
        {
            return res; 
        }
        else if (!done && (res.Flags & (FS_INVALID | FS_FILE)))
        {
            res.Flags = FS_INVALID;
            return res;
        }
    }
    while (!done);

    res.Flags = FS_INVALID;
    return res;
}

// Traverse the directory upwards to the filepath we pass in
// @param entry - the initial entry we wish to traverse from (usually from the root)
// @param filePath - the file path we wish to reach
// @return The FILE we have found, flag set to INVALID if file not found.
FILE FsFat12_OpenFrom(FILE dir, const char* filePath) 
{
    FILE res;
    res.Flags = FS_INVALID;
    const char* tempFile = filePath;

    uintptr_t pointers[2];
    pointers[1] = (uintptr_t) &res;
    bool done = false;     
    do 
    {
        char nextFile[255];
        ExtractNextEntry(&tempFile, nextFile);
        done = !(*tempFile);

        pointers[0] = (uintptr_t) nextFile;
        FsFat12_IterateFolder(dir, MatchDelegate, pointers);
        
        if (done) 
        {
            return res; 
        }
        else if (!done && res.Flags & (FS_INVALID | FS_FILE))
        {
            res.Flags = FS_INVALID;
            return res;
        }

        dir = res;
    }
    while (!done);

    res.Flags = FS_INVALID;
    return res;
}

// Iterate a folder applying a function. 
// If the DirectoryDelegate returns true quit iterating. 
// @param the directory we wish to iterate. 
// @param fileFN the file function 
// @param passIn Variable to pass to the file function
void FsFat12_IterateFolder(FILE dir, DirectoryDelegate fileFn, uint32_t* ptrs)
{
    if (dir.CurrentCluster < 2)
    {
        IterateRootFolder(fileFn, ptrs);
    }
    else
    {
        IterateSubDirectory(dir, fileFn, ptrs);
    }
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
        int lenRemaining = length;
        // If we're a file we should consider the fact that we have a FileLength to consder.
        if (file->Flags == FS_FILE)
        {
            int totalFileRemaining = (file->FileLength - file->Position);
            lenRemaining = length > totalFileRemaining  ? totalFileRemaining : length;
        }
        
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
            
            //  n == even (low four bits in location 1+(3*n)/2 with 8 bits in location (3*n)/2
            //  n == odd, high four bits in location (3*n)/2 with 8 bits in location 1+(3*n)/2 
            // (3 * n) / 2 is equivalent to  1.5 * n (We can use a bitshift, which should be more efficient.)
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