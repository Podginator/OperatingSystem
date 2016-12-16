#include <command.h>
#include <stdint.h>
#include <string.h>
#include <keyboard.h>
#include <console.h>
#include <floppydisk.h>
#include <disk_command.h>

char _prompt[25];
char _buffer[2048];
bool isRunning = true;
int bufferCounter = 0; 

// Command to clear the screen.
void Command_ClearScreen();
// Exit the OS
void Command_Exit();
//  Change the prompt.
void Command_Prompt(char* cmd);
// Process the command
void Command_ProcessCommand(char* cmd);
// Process Disk Command
void Command_Disk(size_t, char type);


// Run the command
void Run() 
{
    memset(_prompt, '\0', 25);
    DiskCommand_Init();
    Command_Prompt("Command>");
    ConsoleWriteString(_prompt);
    while(isRunning) 
    { 
        keycode code = KeyboardGetCharacter();
        
        if (code == KEY_RETURN) { 
            _buffer[bufferCounter] = '\0';
            Command_ProcessCommand(_buffer);
            ConsoleWriteCharacter('\n');
            
            // Clear the bufferf
            memset(_buffer, 0, 128);
            bufferCounter = 0;
            
            //Write Prompt
            if (isRunning) 
            {
                ConsoleWriteString(_prompt);
            }
        } 
        else if (code == KEY_BACKSPACE) 
        {
            if (bufferCounter > 0) { 
                int x, y;
                ConsoleGetXY(&x, &y);
                ConsoleGotoXY(x - 1, y);
                ConsoleWriteCharacter(' ');
                ConsoleGotoXY(x - 1, y);
                _buffer[--bufferCounter] = 0;
            }
        } 
        else if (code == KEY_TAB)
        {
            int space = strchr(_buffer, ' ');
            space = space == -1 ? 0 : space + 1;
            int found = 0;
            DiskCommand_AutoComplete((_buffer + space), &found);

            if (found == 1)
            {
                // go back to the start
                int x, y;
                ConsoleGetXY(&x, &y);
                ConsoleGotoXY(x - bufferCounter, y);
                bufferCounter = 0; 
            } 
            else if (found > 0)
            {
                ConsoleWriteCharacter('\n');
                // Clear the buffer
                bufferCounter = 0;
                ConsoleWriteString(_prompt);
            }

            while(_buffer[bufferCounter] != 0)
            {
                ConsoleWriteCharacter(_buffer[bufferCounter]);
                bufferCounter++;
            }
        }
        else
        {       
            char keyChar = KeyboardConvertKeyToASCII(code);
            if (keyChar != NULL) 
            {                
                _buffer[bufferCounter] = keyChar;
                bufferCounter++;
                ConsoleWriteCharacter(keyChar); 
            }
        }
    }
}

// Command ClearScreen
void Command_ClearScreen() 
{
    // Return the current. 
    ConsoleClearScreen(ConsoleGetCurrentColour());
}

// Exit the OS
void Command_Exit() 
{
    ConsoleWriteString("\nShutting Down TomOs");
    isRunning = false; 
}

//  Change the prompt.
void Command_Prompt(char* cmd) 
{
    int sizeString =  strlen(cmd);
    memcpy(_prompt, cmd, sizeString);
    _prompt[sizeString] = '\0';
}

// Disk
// @param sector
// @param type 
void Command_Disk(size_t sector, char type) 
{
    ConsoleWriteInt(sector, 10);
    uint8_t* addr = FloppyDriveReadSector(sector);
    ConsoleWriteCharacter('\n');
    for (size_t i = 0; i < 512; i += 4) 
    {

        if (type != 'c')
        {
            unsigned int base = type == 'h' ? 16 : 10;
            ConsoleWriteInt(addr[i], base);
            ConsoleWriteInt(addr[i + 1], base);
            ConsoleWriteInt(addr[i + 2], base);
            ConsoleWriteInt(addr[i + 3], base);
        } 
        else 
        {
            ConsoleWriteCharacter((char)addr[i]);
            ConsoleWriteCharacter((char)addr[i + 1]);
            ConsoleWriteCharacter((char)addr[i + 2]);
            ConsoleWriteCharacter((char)addr[i + 3]);
        }

        // Do nothing until Keyboard is not return
        while (KeyboardGetCharacter() != KEY_RETURN) {}
    }
}

// Process the command
void Command_ProcessCommand(char* cmd)
{
    if (strcasecmp("exit", cmd) == 0) 
    {
        Command_Exit();
    }
    else if (strcasecmp("cls", cmd) == 0)
    {
        Command_ClearScreen();
    }
    else if (strncasecmp("prompt ", cmd, 7) == 0)
    {
        char* temp = cmd + 7;
        Command_Prompt(temp);
    }
    else if (strncasecmp("readdisk ", cmd, 9) == 0) 
    {
        char* temp = cmd + 9;

        // Will return 0 in the case where temp == null
        int num = atoi(temp);
        
        // Add two as it will return the first index of the char (0 indexed)
        // and we want to get just past that. 
        int strPos = strchr(temp, ' ') + 1;
        char limiter = 'h';

        // If we have a /c
        if (strPos > -1 && (temp += strPos)[0] == '/')
        {
            // Set the read mode as the character proceeding it
            limiter = temp[1]; 
        }   

        // Call our function
        Command_Disk(num, limiter);
    }
    else if (strncasecmp("cd ", cmd, 3) == 0) 
    {
        DiskCommand_ChangeDirectory(cmd + 3);
    }
    else if (strcasecmp("pwd", cmd) == 0) 
    {
        ConsoleWriteCharacter('\n');
        ConsoleWriteString(DiskCommand_GetPresentWorkingDirectory());
    }
    else if (strcasecmp("ls", cmd) == 0 || strncasecmp ("dir", cmd) == 0) 
    {
        ConsoleWriteCharacter('\n');
        DiskCommand_ListFiles();
    }
    else if (strncasecmp("read ", cmd, 5) == 0)
    {
        DiskCommand_ReadFile(cmd + 5);
    }
    else 
    {
        ConsoleWriteString("\nCommand Not Recognized"); 
    }
}   