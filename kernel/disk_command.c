#include <disk_command.h>
#include <filesystem.h>
#include <size_t.h>
#include <keyboard.h>
#include <_null.h>
#include <console.h>
#include <string.h>

// The Filepath we're using
static char _pwd[2048];

// The Current Working Directory.
static FILE _cwd;

// Static Declarations
static void SetPresentWorkingDirectory(const char* pwd);

// Inline Declarations 
static inline char* PrepareFilePath(char* filepath); 
static inline void PrintDirectoryEntry(const pDirectoryEntry entry, const char* name);
static inline void GetDateCreated(uint16_t dateCreated, uint8_t* day, uint8_t* month, uint16_t* year);
static inline void GetTimeCreated(uint16_t timeCreated, uint8_t* hour, uint8_t* minutes, uint8_t* seconds);
static inline FILE GetFileFromPath(char* dir, char* outPath);

// Delegates Declarations
static bool ListFileDelegate(pDirectoryEntry entry, uintptr_t*);
static bool AutoCompleteDelegate(pDirectoryEntry entry, uintptr_t*);

//
// Static Definition
// 

// Set the Present Working Directory.
// @param pwd what to set the Present Working Directory as
static void SetPresentWorkingDirectory(const char* pwd)
{
    // Copy the pwd to the _pwd 
    strcpy(_pwd, pwd);
}

//
// Inline Definitions
//

// Prepare the file path.
// @param filepath - The file Path 
// @return the corrected file Path.
static inline char* PrepareFilePath(char* filepath) 
{
    char* temp = filepath;
    char* writeTo = filepath;

    while (*temp != 0)
    {
        // If there's 2, 1, or -1 characters between \ and the end character
        // then we're either \.\ \..\ or \. \.. (-1 indicates an \0)
        if (*temp == '\\' && (strchr((temp + 1), '\\') <= 2))
        {
            if (*(temp + 1) == '.') 
            {
                char secondChar = *(temp + 2);
                if (secondChar == '.') 
                {
                    while (*(--writeTo) != '\\');
                    temp += 3;
                    continue;
                }
                else if (secondChar == '\\' || !secondChar)
                {
                    // Otherwise skip over the . 
                    temp += 2;
                    continue;
                }
            }
        }

        *writeTo = *temp;
        writeTo++;
        temp++;
    } 
  
    // Remove the trailing \ from  the path if it's there
    if (*(writeTo - 1) == '\\')
    {
        writeTo--;
    }
    // Null Terminate at the end.
    *writeTo = 0;

    // We could have overshot root. 
    if (!filepath[0]) 
    {
        filepath[0] = '\\';
        filepath[1] = 0;
    }

    return filepath;
}

// Print the DirectoryEntry (Prints like Windows: CMD dir)
// @param entry the directory entry 
// @param isLongFileName - We want to handle it if we've passed in a long file name
static inline void PrintDirectoryEntry(const pDirectoryEntry entry, const char* name)
{
    // Do Writing. (like CMD)
    size_t counter = 0;

    //Write out the date
    uint8_t date;
    uint8_t month;
    uint16_t year; 
    GetDateCreated(entry->DateCreated, &date, &month, &year);
    ConsoleWriteInt(date, 10);
    ConsoleWriteCharacter('/');
    ConsoleWriteInt(month, 10);
    ConsoleWriteCharacter('/');
    ConsoleWriteInt(year, 10);

    ConsoleWriteString("  ");

    // Write the DateTime
    uint8_t  hour;
    uint8_t  minutes;
    uint8_t  seconds; 
    GetTimeCreated(entry->TimeCreated, &hour, &minutes, &seconds);
    ConsoleWriteInt(hour, 10);
    ConsoleWriteCharacter(':');
    if (minutes < 10)
    {
        ConsoleWriteInt(0, 10);
    }
    ConsoleWriteInt(minutes, 10);

    ConsoleWriteString("  ");

    // File or Directory
    if (entry->Attrib & 0x10)
    {
        ConsoleWriteString("<DIR>");
    } 
    else
    {
        ConsoleWriteString("<FIL>");
    }

    ConsoleWriteString("  ");                

    // Get the filename from the directory.
    ConsoleWriteString(name);
    ConsoleWriteString("  ");

    // Print the filesize
    if (!(entry->Attrib & DIR_DIRECTORY))
    {
        ConsoleWriteInt(entry->FileSize, 10);
    }       
}

// Extract the date created from the entry->DateCreated property. 
// @param dateCreated the Date Created 
// @param day OUT parameter to write day to
// @param month OUT parameter to write month to
// @param year OUT parameter to write year to
// @pre: Date Created should be the Directory Entries 
static inline void GetDateCreated(const uint16_t dateCreated, uint8_t* day, uint8_t* month, uint16_t* year) 
{
    *day = ((dateCreated) & 0x00ff) & ~(0b111 << 5);
    *month = ((dateCreated >> 5) & 0x000f);
    *year = 1980 + (dateCreated >> 9);
}

// Extract the TIME created from the DirectoryEntry TimeCreated Property. 
// @param timeCreated the time it was created
// @param hour OUT the hour it was created
// @param minutes OUT the minutes it was created 
// @param seconds OUT Number of seconds (0-29) - 2 second accuracy 
// @pre:  timeCreated should be in the DirectoryEntry timeCreated format.
static inline void GetTimeCreated(uint16_t timeCreated, uint8_t* hour, uint8_t* minutes, uint8_t* seconds)
{
    // Bitpacked as follows:
    // Bit 0-4 : Seconds (0-29), i.e. it only stores to a 2 second accuracy
    // Bit 5-10 : Minutes (0-59)
    // Bit 11-15 : Hours (0-23)
    *seconds = ((timeCreated) & 0x00ff) & ~(0b111 << 5);
    *minutes = ((timeCreated >> 5) & 0x00ff) & ~(0b11 << 6);
    *hour =    (timeCreated >> 11);
}

// Return a file from the a filepath. 
// @ param filepath the filepath to retrieve from 
// @ param fullFilePath OUT if not null use to store the fullFilePath
// @ return the FILE returned, FS_INVALID if none.
// @ post if outPath is not null the outpath will be set the the pwd, this handles ../. 
static inline FILE GetFileFromPath(char* dir, char* outPath) 
{
    FILE directory; 
    directory.Flags = FS_INVALID;

    // If the first character is not a backspace we are not starting from root.
    if (dir[0] != '\\') 
    {
        directory = FsFat12_OpenFrom(_cwd, dir);
        
        if (outPath)
        {
            if (_pwd[1] != NULL) 
            {
                strcat(outPath, _pwd, "\\");
            }
            strcat(outPath, outPath, dir);
        }
    } 
    else 
    {
        if (outPath) 
        {
            // Otherwise let's just Copy 
            strcpy(outPath, dir);
        }
        directory = FsFat12_Open(dir);
    }

    return directory;
}

// 
// Delegate Defintions
//

// Print each file.
// @param the Entry to read from
// @param unused = pass NULL
// @return Always False, we want to navigate all the folders. 
bool ListFileDelegate(pDirectoryEntry entry,  uintptr_t* ptrs)
{
    if (FsFat12_RetrieveNameFromDirectoryEntry(entry, _longFileName))
    {
        PrintDirectoryEntry(entry, _longFileName);
        ConsoleWriteString("\n");
    }
    return false;
}

// Fills a buffer with partial matches from the 
// @param the Entry to read from
// @param pts Pointers to needed parameters, 
//            0 - str to compare to 
//            1 - comparison length (passed to avoid recalcuating each time)
//            2 - pointer to the buffer to store into 
//            3 - num of entries retrieved. 
// @return Always False, we want to navigate all the files. 
bool AutoCompleteDelegate(pDirectoryEntry entry, uintptr_t* ptrs)
{
    char* compare = (char*) ptrs[0];
    size_t compareLen = *((size_t*) ptrs[2]);
    char** testBuffer = (char**) ptrs[1];
    int* num = (int*) ptrs[3];

    if (FsFat12_RetrieveNameFromDirectoryEntry(entry, *testBuffer))
    {
        // Get the Short File name if previous entries weren't a long filename directory.   
        if (strncmp(*testBuffer, compare, compareLen) == 0)
        {
            size_t lenComplete = strlen(*testBuffer);
            *testBuffer += lenComplete + 1;
            *(*testBuffer - 1) = ',';
            *num = *num + 1;
        }
    }
    return false;
}

//
// Header Declarations
//

// Initialize
void DiskCommand_Init()
{    
    // Copy the root directory.
    _cwd = FsFat12_Open("\\");

    // Initialize to the pwd being empty.
    SetPresentWorkingDirectory("\\");
}

// Get the present working directory
// @return The Present Working Directory.
char* DiskCommand_GetPresentWorkingDirectory()
{
    return _pwd;
}

// Process the Change Directory Command 
// @param dir the directory to change to. 
void DiskCommand_ChangeDirectory(char* dir)
{
    FILE directory = GetFileFromPath(dir, _tempBuffer);

    //If we've returned a directory, we've accessed the correct thing
    if (directory.Flags == FS_DIRECTORY)
    {
        _cwd = directory;
        SetPresentWorkingDirectory(PrepareFilePath(_tempBuffer));
    }
    else
    {
        ConsoleWriteString("\nNo Directory Found at ");
        ConsoleWriteString(PrepareFilePath(_tempBuffer));
    }
}

// Process the LS Command 
// @param the filePath of the file to read files from. 
void DiskCommand_ListFiles()
{
    FsFat12_IterateFolder(_cwd, ListFileDelegate, NULL);
}

// Process The ReadFile  
// @param the filePath of the file we wish to read.
void DiskCommand_ReadFile(char* filePath)
{
    // We don't particularly care about the full filepath in this instance
    FILE directory = GetFileFromPath(filePath, NULL);

    //If we've returned a directory, we've accessed the correct thing
    if (directory.Flags == FS_FILE)
    {
        ConsoleWriteString("\n");
        
        // Read 32 bytes at a time, hitting enter to advance. 
        while (directory.Eof == 0) 
        {
            char buffer[32];
            size_t totalSize = FsFat12_Read(&directory, buffer, 32);
            for (size_t i = 0; i < totalSize; i++) 
            {
                // write the buffer character.
                ConsoleWriteCharacter(buffer[i]);
            }

            // Continue reading 32 bytes at a time until return is hit
            while (KeyboardGetCharacter() != KEY_RETURN) 
            {
                // Or until CTRL + C is clicked
                if (KeyboardGetCtrlKeyState() && KeyboardGetCharacter() == KEY_C) {
                    // If we have hit Ctrl+C we abandon this and therefore return.
                    return;
                }
            }
        }
    }
    else
    {
        ConsoleWriteString("\nNo File to be read at ");
        ConsoleWriteString(PrepareFilePath(filePath));
    }
}

// Autocomplete the path 
// @param path OUT path to autocomplete
// @param num OUT number of results foumd 
void DiskCommand_AutoComplete(char* path, int* num)
{
    // First Step: 
    //  Get to the end of path but exclude the last one. 
    //  IE: path = /root/test/one/te We want to travere to /root/test/one/ 
    //  and get 'te' to autocorrect.    
    char* temp = path;
    char* tempBuffer = _tempBuffer;
    int charLoc = 0;
    FILE file = _cwd; 


    for (int loc = -1; loc != 0; loc = strchr((temp + charLoc), '\\') + 1, charLoc += loc);
    if (charLoc > 0) 
    {
        char character = *(temp + charLoc); 
        *(temp + charLoc) = 0;
        file = GetFileFromPath(temp, NULL);
        *(temp + charLoc) = character;
    }


    // Second Step:
    // Iterate through each file and store any file with the same first n characters.
    char* compare = temp + charLoc;
    size_t strSize = strlen(compare);
    uintptr_t pointers[4];
    pointers[0] = (uintptr_t) compare; 
    pointers[1] = (uintptr_t) &tempBuffer;
    pointers[2] = (uintptr_t) &strSize;    
    pointers[3] = (uintptr_t) num;
    FsFat12_IterateFolder(file, AutoCompleteDelegate, pointers); 
    *tempBuffer = 0;
    
    // Final Step:
    if (*num == 1)
    {
        // If we've just found one, change the buffer 
        int delimiterLoc = strchr(_tempBuffer, ',');
        memcpy(compare, _tempBuffer, delimiterLoc);
    }
    else if (*num > 0)
    {
        // Otherwise just print them out.
        ConsoleWriteCharacter('\n');
        ConsoleWriteString(_tempBuffer);
    }
} 