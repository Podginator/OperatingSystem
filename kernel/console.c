//	Basic Console Output.

#include <stdint.h>
#include <string.h>
#include <console.h>

#define CONSOLE_HEIGHT		25
#define CONSOLE_WIDTH		80
#define MAX_PAGE_STORAGE    3
#define PAGE_SIZE           CONSOLE_HEIGHT * CONSOLE_WIDTH
#define CONSOLE_STORAGE     PAGE_SIZE * MAX_PAGE_STORAGE
#define DEFAULT_COLOUR      0x1F


//Array of characters storing a buffer of characters.
uint16_t _pageStored[CONSOLE_STORAGE] = { [0 ... CONSOLE_STORAGE - 1] = (DEFAULT_COLOUR << 8) }; 
uint16_t *_pageOffset = _pageStored + (PAGE_SIZE * 2);

// Video memory
uint16_t *_videoMemory = (uint16_t*) 0xB8000;

// Current cursor position
uint8_t _cursorX = 0;
uint8_t _cursorY = 0;
int _currPage = MAX_PAGE_STORAGE - 1;

// Current color
uint8_t	_colour = DEFAULT_COLOUR;

char stringBuffer[40];
char hexChars[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

// Write byte to device through port mapped io
void  OutputByteToVideoController(unsigned short portid, unsigned char value) 
{
	asm volatile("movb	%1, %%al \n\t"
				 "movw	%0, %%dx \n\t"
				  "out	%%al, %%dx"
				 : : "m" (portid), "m" (value));
				 
}

// Update hardware cursor position

void UpdateCursorPosition(int x, int y) 
{
    uint16_t cursorLocation = y * 80 + x;

	// Send location to VGA controller to set cursor
	// Send the high byte
	OutputByteToVideoController(0x3D4, 14);
	OutputByteToVideoController(0x3D5, cursorLocation >> 8); 
	// Send the low byte
	OutputByteToVideoController(0x3D4, 15);
	OutputByteToVideoController(0x3D5, cursorLocation);      
}


// Display the Page. 
// @param the page to display. 
void DisplayPage(size_t pageToDisplay)
{
	if (pageToDisplay > MAX_PAGE_STORAGE)
	{
		return;
	}

	uint16_t* offset = _pageStored + ((pageToDisplay * PAGE_SIZE));
	for (int i = 0; i < PAGE_SIZE; i++)
	{
		_videoMemory[i] = offset[i];
	}

	//Hide the cursor on non pages.
	if (pageToDisplay != MAX_PAGE_STORAGE - 1) 
	{
		UpdateCursorPosition(0, CONSOLE_HEIGHT+1);
	}
	else
	{
		UpdateCursorPosition(_cursorX, _cursorY);
	}
}

// Scroll Page Up 
void ConsoleScrollPageDown()
{
	if (_currPage > 0)	{
		_currPage--;
		DisplayPage(_currPage);
	}
}

// Scroll Page Down
void ConsoleScrollPageUp()
{
	if (_currPage < MAX_PAGE_STORAGE - 1)
	{
		_currPage++;
		DisplayPage(_currPage);
	}
}

void Scroll() 
{
	if (_cursorY >= CONSOLE_HEIGHT) 
	{

		uint16_t attribute = _colour << 8;

		// Move current display up one line
		int line25 = (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH;
		memcpy(_pageStored, _pageStored + CONSOLE_WIDTH, ((CONSOLE_HEIGHT * CONSOLE_WIDTH) * MAX_PAGE_STORAGE) * 2);

		for (int i = 0; i < line25; i++)
		{
			_videoMemory[i] = _videoMemory[i + CONSOLE_WIDTH];
			_pageOffset[i] = _videoMemory[i];
		}


		// Clear the bottom line
		for (int i = line25; i < line25 + 80; i++)
		{
			_videoMemory[i] = attribute | ' ';
			_pageOffset[i] = _videoMemory[i];
		}

		_cursorY = 25;
	}
}

// Displays a character
void ConsoleWriteCharacter(unsigned char c) 
{
	// Go down to the normal if we're print this.'
	if(_currPage != MAX_PAGE_STORAGE - 1)
	{
		_currPage = MAX_PAGE_STORAGE - 1;
		DisplayPage(MAX_PAGE_STORAGE - 1);
	}

    uint16_t attribute = _colour << 8;

    if (c == 0x08 && _cursorX)
	{
		// Backspace character
        _cursorX--;
	}
    else if (c == 0x09)
	{
		// Tab character
        _cursorX = (_cursorX + 8) & ~(8 - 1);
	}
    else if (c == '\r')
	{
		// Carriage return
        _cursorX = 0;
	}
	else if (c == '\n') 
	{
		// New line
        _cursorX = 0;
        _cursorY++;
	}
    else if (c >= ' ') 
	{
		// Printable characters
		// Display character on screen
        uint16_t* location = _videoMemory + (_cursorY * CONSOLE_WIDTH + _cursorX);
		uint16_t* storedLoc = _pageOffset + (_cursorY * CONSOLE_WIDTH + _cursorX);
        *location = c | attribute;
		*storedLoc = *location;
        _cursorX++;
    }
    // If we are at edge of row, go to new line
    if (_cursorX >= CONSOLE_WIDTH) 
	{
        _cursorX = 0;
        _cursorY++;
    }
	// If we are at the last line, scroll up
	if (_cursorY >= CONSOLE_HEIGHT)
	{
		Scroll();
		_cursorY = CONSOLE_HEIGHT - 1;
	}
    //! update hardware cursor
	UpdateCursorPosition (_cursorX, _cursorY);
}

void ConsoleWriteInt(unsigned int i, unsigned int base) 
{
    int pos = 0;
	
    if (i == 0 || base > 16) 
    {
		ConsoleWriteCharacter('0');
    }
	else
	{
		while (i != 0) 
		{
			stringBuffer[pos] = hexChars[i % base];
			pos++;
			i /= base;
		}
		while (--pos >= 0)
		{
			ConsoleWriteCharacter(stringBuffer[pos]);
		}
	}
}

// Sets new font colour and returns the old colour
unsigned int ConsoleSetColour(const uint8_t c) 
{
	unsigned int t = (unsigned int)_colour;
	_colour = c;
	return t;
}

// Set new cursor position
void ConsoleGotoXY(unsigned int x, unsigned int y) 
{
	// Go to the current page, first.
	DisplayPage(MAX_PAGE_STORAGE - 1);

	if (_cursorX <= CONSOLE_WIDTH - 1)
	{
	    _cursorX = x;
	}
	if (_cursorY <= CONSOLE_HEIGHT - 1)
	{
	    _cursorY = y;
	}
	// update hardware cursor to new position
	UpdateCursorPosition(_cursorX, _cursorY);
}

// Returns cursor position
void ConsoleGetXY(unsigned* x, unsigned* y) 
{
	if (x == 0 || y == 0)
	{
		return;
	}
	*x = _cursorX;
	*y = _cursorY;
}

// Return horzontal width
int ConsoleGetWidth() 
{
	return CONSOLE_WIDTH;
}

// Return vertical height
int ConsoleGetHeight() 
{
	return CONSOLE_HEIGHT;
}

//! Clear screen
void ConsoleClearScreen(const uint8_t c) 
{
	_colour = c;
	uint16_t blank = ' ' | (c << 8);
	size_t i = 0;
	for (; i < PAGE_SIZE; i++)
	{
		_pageStored[i] = blank;
        _videoMemory[i] = blank;
	}

	// then ensure we also clear the storage.
	for (; i < (MAX_PAGE_STORAGE * PAGE_SIZE); i++)
	{
		_pageStored[i] = blank;
	}
	
	ConsoleGotoXY(0, 0);
}
	
// Returns the current colour
uint8_t ConsoleGetCurrentColour() 
{ 
	return _colour;
}

// Display specified string
void ConsoleWriteString(const char* str) 
{
	if (!str)
	{
		return;
	}
	while (*str)
	{
		ConsoleWriteCharacter(*str++);
	}
}



