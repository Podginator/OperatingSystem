#ifndef _CONSOLE_H
#define _CONSOLE_H
#include <stdint.h>


// Scroll the Page up
void ConsoleScrollPageDown();

// Scroll the page Down
void ConsoleScrollPageUp();

// Output the specified character the current cursor position.  
// The attribute at that position remains unchanged.

void ConsoleWriteCharacter(unsigned char c); 

// Write the null-terminated string str to the current cursor position on the screen

void ConsoleWriteString(const char* str); 

// Write the unsigned integer to the screen, using the specified base, e.g. for
// the number to be displayed as a decimal number, you would specify base 10.  For 
// a number to be displayed as hexadecimal, you would specify base 16.

void ConsoleWriteInt(unsigned int i, unsigned int base); 

// Set the attribute to be used for all subsequent calls to ConsoleWriteXXXX routines

unsigned int ConsoleSetColour(const uint8_t c); 

// Position the cursor at the specified X and Y position. All subsequent calls to 
// ConsoleWriteXXXX routines will start at this position.

void ConsoleGotoXY(unsigned int x, unsigned int y); 

// Get the current X and Y position of the cursor

void ConsoleGetXY(unsigned* x, unsigned* y);

// Get the width of the console window

int ConsoleGetWidth();

// Get the height of the console window

int ConsoleGetHeight(); 

// Clear the screen.  Set the attribute byte for all characters on the 
// screen to be c. Set the cursor position to 0,0.

void ConsoleClearScreen(const uint8_t c); 

// Return the current Colour.
// @ return the current colour
uint8_t ConsoleGetCurrentColour();

#endif