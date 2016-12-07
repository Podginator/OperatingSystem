#ifndef _DISK_COMMAND_H
#define _DISK_COMMAND_H


void DiskCommand_Init();
// Get the present working directory
char* DiskCommand_GetPresentWorkingDirectory();
// Process the Change Directory Command 
void DiskCommand_ChangeDirectory(char* dir);
// Process the LS Command 
void DiskCommand_ListFiles();
// Process The ReadFile  
void DiskCommand_ReadFile(char* filePath);
// Autocomplete working path
void DiskCommand_AutoComplete(char* path, int* num);

#endif