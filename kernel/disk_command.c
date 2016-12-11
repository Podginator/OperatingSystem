#include <disk_command.h>
#include <filesystem.h>
#include <size_t.h>
#include <keyboard.h>
#include <_null.h>
#include <console.h>
#include <string.h>

// The Filepath we're using (More?)
static char _pwd[2048];
// A Temporary Buffer used for Autocomplete and ChangeDirectory.
static char  _tempBuffer[2048];


// 16 Directory Entries per current directory.
static DirectoryEntry _cwd[16]; 

// Forward declarations
static inline char* PrepareFilePath(char* filepath); 
static inline void GetDateCreated(uint16_t dateCreated, uint8_t* day, uint8_t* month, uint16_t* year);
static inline void GetTimeCreated(uint16_t timeCreated, uint8_t* hour, uint8_t* minutes, uint8_t* seconds);
static inline void PrintDirectoryEntry(const pDirectoryEntry entry, bool isLFN);
static void SetPresentWorkingDirectory(const char* pwd);

// Extract the date created from the entry->DateCreated property. 
// @param dateCreated the Date Created 
// @param day OUT parameter to write day to
// @param month OUT parameter to write month to
// @param year OUT parameter to write year to
// @pre: Date Created should be the Directory Entries 
static void GetDateCreated(const uint16_t dateCreated, uint8_t* day, uint8_t* month, uint16_t* year) 
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
static void GetTimeCreated(uint16_t timeCreated, uint8_t* hour, uint8_t* minutes, uint8_t* seconds)
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
// @post if outPath is not null the outpath will be set the the pwd, this handles ../. 
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
            strcpy(outPath, _pwd);
            if (_pwd[1] != NULL)
            { 
                // Currently PWD is not '\'
                strcat(outPath, _pwd, "\\");
            }

            strcat(outPath, outPath, dir);
        }
    } 
    else 
    {
        if (outPath) {
            // Otherwise let's just copy
            strcpy(outPath, dir);
        }
        directory = FsFat12_Open(dir);
    }

    return directory;
}

// Prepare the file path.
// @param filepath - The file Path 
// @return the corrected file Path.
static char* PrepareFilePath(char* filepath) 
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
                if (*(temp + 2) == '.') 
                {
                    // Loop Back to the previous \. 
                    while (*(--writeTo) != '\\');
                    temp += 3;
                }
                else
                {
                    // Otherwise skip over the . 
                    temp += 2;
                }

                continue;
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
static inline void PrintDirectoryEntry(const pDirectoryEntry entry, bool isLongFileName)
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
    char fileName[256];
    FsFat12_GetNameFromDirectoryEntry(entry, fileName, isLongFileName);
    ConsoleWriteString(fileName);
    ConsoleWriteString("  ");

    // Print the filesize
    if (!(entry->Attrib & 0x10))
    {
        ConsoleWriteInt(entry->FileSize, 10);
    } 
}

// Initialize
void DiskCommand_Init()
{    
    // Copy the root directory.
    memcpy(_cwd, FsFat12_GetDirectoryFromSector(2), 512);
 
    // Initialize to the pwd being empty.
    SetPresentWorkingDirectory("\\");
}

// Set the Present Working Directory.
// @param pwd what to set the Present Working Directory as
static void SetPresentWorkingDirectory(const char* pwd)
{
    // Copy the pwd to the _pwd 
    strcpy(_pwd, pwd);
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
    // This is here as it's used only in this function. 
    // The functions proceeding this do not require much stack space (We will not blow the stack)
    // We merely memcpy it to the _pwd if we have returned the correct directory.
    FILE directory = GetFileFromPath(dir, _tempBuffer);

    //If we've returned a directory, we've accessed the correct thing
    if (directory.Flags == FS_DIRECTORY)
    {
        // Return current directory? 
        // Is the memcpy a better use of our resources. Probably. Actually.
        memcpy(_cwd, FsFat12_GetDirectoryFromSector(directory.CurrentCluster), 512);

        // Copy the pwd.
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
    bool done = false;
    bool nextLFN = false; 
    pDirectoryEntry tempEntry = _cwd;
    while (!done)
    {
        // Read all entries in the current directory.
        for (size_t j = 0; j < 16; j++, tempEntry++)
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
                    return;
                }

                if (tempEntry->Attrib != 0x0f && !(tempEntry->Attrib & 0x02))
                {
                    PrintDirectoryEntry(tempEntry, nextLFN);
                    ConsoleWriteString("\n");
                }

                nextLFN = tempEntry->Attrib == 0x0F;
        }
    }
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
    //  and get te to autocorrect.    
    DirectoryEntry entry[16];

    char* temp = path;
    int charLoc = -1;
    int loc = -1;

    while ((loc = strchr((temp + charLoc), '\\') + 1) > 0)
    {
        charLoc += loc;
    }

    if (charLoc > -1) 
    {
        char character = *(temp + charLoc); 
        *(temp + charLoc) = 0;
        FILE file = GetFileFromPath(temp, NULL);
        *(temp + charLoc) = character;

        if (file.Flags == FS_DIRECTORY)
        {   
            memcpy(entry, (pDirectoryEntry)FsFat12_GetDirectoryFromSector(file.CurrentCluster), 512);
        } 
    }
    else
    {
        // Set Charloc to 0 (As we later use this to find what to compare too) 
        charLoc = 0;
        memcpy(entry, _cwd, 512);
    }


    bool nextLFN = false;
    char* tempFound = _tempBuffer;
    // If the first byte is 0 there's nothing remaining. 
    if (entry[0].Filename[0] != 0x00) 
    {
        pDirectoryEntry tempEntry = entry;
        char* compare = temp + charLoc;
        size_t compareLen = strlen(compare);

        if (compareLen > 0) 
        {
            for (size_t j = 0; j < 16; j++, tempEntry++)
            {
                if (tempEntry->Attrib != 0x0f)
                {
                    FsFat12_GetNameFromDirectoryEntry(tempEntry, tempFound, nextLFN);

                    if (strncmp(tempFound, compare, compareLen) == 0)
                    {
                        *num = *num + 1;                    
                        size_t tempLen = strlen(tempFound);
                        tempFound += tempLen + 1;
                        *(tempFound - 1) = ' ';
                    }
                }

                nextLFN = tempEntry->Attrib == 0x0F;
            }
        }
        *(tempFound) = 0;
        
        // If we've only found one, autocomplete
        if (*num == 1)
        {
            int delimiterLoc = strchr(_tempBuffer, ' ');
            memcpy(compare, _tempBuffer, delimiterLoc);
        }
        else if (*num > 0)
        {
            // Otherwise just print them out.`
            ConsoleWriteCharacter('\n');
            ConsoleWriteString(_tempBuffer);
        }
    }
} 