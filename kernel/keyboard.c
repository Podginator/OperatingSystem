// Keyboard Driver

#include <string.h>
#include <ctype.h>
#include <hal.h>
#include <keyboard.h>

// keyboard encoder 

#define	KYBRD_ENC_INPUT_BUF		0x60
#define	KYBRD_ENC_CMD_REG		0x60

#define	KYBRD_ENC_CMD_SET_LED					0xED
#define	KYBRD_ENC_CMD_ECHO						0xEE
#define	KYBRD_ENC_CMD_SCAN_CODE_SET				0xF0
#define	KYBRD_ENC_CMD_ID						0xF2
#define	KYBRD_ENC_CMD_AUTODELAY					0xF3
#define	KYBRD_ENC_CMD_ENABLE					0xF4
#define	KYBRD_ENC_CMD_RESETWAIT					0xF5
#define	KYBRD_ENC_CMD_RESETSCAN					0xF6
#define	KYBRD_ENC_CMD_ALL_AUTO					0xF7
#define	KYBRD_ENC_CMD_ALL_MAKEBREAK				0xF8
#define	KYBRD_ENC_CMD_ALL_MAKEONLY				0xF9
#define	KYBRD_ENC_CMD_ALL_MAKEBREAK_AUTO		0xFA
#define	KYBRD_ENC_CMD_SINGLE_AUTOREPEAT			0xFB
#define	KYBRD_ENC_CMD_SINGLE_MAKEBREAK			0xFC
#define	KYBRD_ENC_CMD_SINGLE_BREAKONLY			0xFD
#define	KYBRD_ENC_CMD_RESEND					0xFE
#define	KYBRD_ENC_CMD_RESET						0xFF

// keyboard controller 

#define	KYBRD_CTRL_STATS_REG	0x64
#define	KYBRD_CTRL_CMD_REG		0x64

#define	KYBRD_CTRL_STATS_MASK_OUT_BUF		1		//00000001
#define	KYBRD_CTRL_STATS_MASK_IN_BUF		2		//00000010
#define	KYBRD_CTRL_STATS_MASK_SYSTEM		4		//00000100
#define	KYBRD_CTRL_STATS_MASK_CMD_DATA		8		//00001000
#define	KYBRD_CTRL_STATS_MASK_LOCKED		0x10	//00010000
#define	KYBRD_CTRL_STATS_MASK_AUX_BUF		0x20	//00100000
#define	KYBRD_CTRL_STATS_MASK_TIMEOUT		0x40	//01000000
#define	KYBRD_CTRL_STATS_MASK_PARITY		0x80	//10000000

#define	KYBRD_CTRL_CMD_READ					0x20
#define	KYBRD_CTRL_CMD_WRITE				0x60
#define	KYBRD_CTRL_CMD_SELF_TEST			0xAA
#define	KYBRD_CTRL_CMD_INTERFACE_TEST		0xAB
#define	KYBRD_CTRL_CMD_DISABLE				0xAD
#define	KYBRD_CTRL_CMD_ENABLE				0xAE
#define	KYBRD_CTRL_CMD_READ_IN_PORT			0xC0
#define	KYBRD_CTRL_CMD_READ_OUT_PORT		0xD0
#define	KYBRD_CTRL_CMD_WRITE_OUT_PORT		0xD1
#define	KYBRD_CTRL_CMD_READ_TEST_INPUTS		0xE0
#define	KYBRD_CTRL_CMD_SYSTEM_RESET			0xFE
#define	KYBRD_CTRL_CMD_MOUSE_DISABLE		0xA7
#define	KYBRD_CTRL_CMD_MOUSE_ENABLE			0xA8
#define	KYBRD_CTRL_CMD_MOUSE_PORT_TEST		0xA9
#define	KYBRD_CTRL_CMD_MOUSE_WRITE			0xD4

// Scan error codes 

#define	KYBRD_ERR_BUF_OVERRUN				0
#define	KYBRD_ERR_ID_RET					0x83AB
#define	KYBRD_ERR_BAT						0xAA	//note: can also be L. shift key make code
#define	KYBRD_ERR_ECHO_RET					0xEE
#define	KYBRD_ERR_ACK						0xFA
#define	KYBRD_ERR_BAT_FAILED				0xFC
#define	KYBRD_ERR_DIAG_FAILED				0xFD
#define	KYBRD_ERR_RESEND_CMD				0xFE
#define	KYBRD_ERR_KEY						0xFF

// Invalid scan code. Used to indicate the last scan code is not to be reused
#define INVALID_SCANCODE 	0

// Current scan code
static int _scancode = INVALID_SCANCODE;

// Lock keys status
static bool _numlock = false;
static bool _scrolllock = false;
static bool _capslock = false;

// Shift, alt, and ctrl keys current state
static bool _shift = false;
static bool _alt = false;
static bool _ctrl = false;

// Set if keyboard error
static int _keyboardError = 0;

// Set if the Basic Assurance Test (BAT) failed
static bool _keyboardBATResult = false;

// Set if diagnostics failed
static bool _keyboardDiagResult = false;

// Set if system should resend last command
static bool _keyboardResendResult = false;

// Set if keyboard is disabled
static bool _keyboardDisabled = false;

// Original XT scan code set. Array index == make code
static int _keyboardScancode[] = 
{
	KEY_UNKNOWN,	//0
	KEY_ESCAPE,		//1
	KEY_1,			//2
	KEY_2,			//3
	KEY_3,			//4
	KEY_4,			//5
	KEY_5,			//6
	KEY_6,			//7
	KEY_7,			//8
	KEY_8,			//9
	KEY_9,			//0xa
	KEY_0,			//0xb
	KEY_MINUS,		//0xc
	KEY_EQUAL,		//0xd
	KEY_BACKSPACE,	//0xe
	KEY_TAB,		//0xf
	KEY_Q,			//0x10
	KEY_W,			//0x11
	KEY_E,			//0x12
	KEY_R,			//0x13
	KEY_T,			//0x14
	KEY_Y,			//0x15
	KEY_U,			//0x16
	KEY_I,			//0x17
	KEY_O,			//0x18
	KEY_P,			//0x19
	KEY_LEFTBRACKET,//0x1a
	KEY_RIGHTBRACKET,//0x1b
	KEY_RETURN,		//0x1c
	KEY_LCTRL,		//0x1d
	KEY_A,			//0x1e
	KEY_S,			//0x1f
	KEY_D,			//0x20
	KEY_F,			//0x21
	KEY_G,			//0x22
	KEY_H,			//0x23
	KEY_J,			//0x24
	KEY_K,			//0x25
	KEY_L,			//0x26
	KEY_SEMICOLON,	//0x27
	KEY_QUOTE,		//0x28
	KEY_GRAVE,		//0x29
	KEY_LSHIFT,		//0x2a
	KEY_BACKSLASH,	//0x2b
	KEY_Z,			//0x2c
	KEY_X,			//0x2d
	KEY_C,			//0x2e
	KEY_V,			//0x2f
	KEY_B,			//0x30
	KEY_N,			//0x31
	KEY_M,			//0x32
	KEY_COMMA,		//0x33
	KEY_DOT,		//0x34
	KEY_SLASH,		//0x35
	KEY_RSHIFT,		//0x36
	KEY_KP_ASTERISK,//0x37
	KEY_RALT,		//0x38
	KEY_SPACE,		//0x39
	KEY_CAPSLOCK,	//0x3a
	KEY_F1,			//0x3b
	KEY_F2,			//0x3c
	KEY_F3,			//0x3d
	KEY_F4,			//0x3e
	KEY_F5,			//0x3f
	KEY_F6,			//0x40
	KEY_F7,			//0x41
	KEY_F8,			//0x42
	KEY_F9,			//0x43
	KEY_F10,		//0x44
	KEY_KP_NUMLOCK,	//0x45
	KEY_SCROLLLOCK,	//0x46
	KEY_HOME,		//0x47
	KEY_KP_8,		//0x48	//keypad up arrow
	KEY_PAGEUP,		//0x49
	KEY_KP_2,		//0x50	//keypad down arrow
	KEY_KP_3,		//0x51	//keypad page down
	KEY_KP_0,		//0x52	//keypad insert key
	KEY_KP_DECIMAL,	//0x53	//keypad delete key
	KEY_UNKNOWN,	//0x54
	KEY_UNKNOWN,	//0x55
	KEY_UNKNOWN,	//0x56
	KEY_F11,		//0x57
	KEY_F12			//0x58
};

// Private functions

//! read status from keyboard controller
uint8_t KeyboardReadControllerStatus() 
{
	return HAL_InputByteFromPort(KYBRD_CTRL_STATS_REG);
}

// Send command byte to keyboard controller
void KeyboardSendControllerCommand(uint8_t cmd) 
{
	// Wait for keyboard controller input buffer to be clear
	while ((KeyboardReadControllerStatus () & KYBRD_CTRL_STATS_MASK_IN_BUF) != 0);
	HAL_OutputByteToPort(KYBRD_CTRL_CMD_REG, cmd);
}

// Read keyboard encoder buffer
uint8_t KeyboardReadEncoderBuffer() 
{
	return HAL_InputByteFromPort(KYBRD_ENC_INPUT_BUF);
}

//! Send command byte to keyboard encoder
void KeyboardSendEncoderCommand(uint8_t cmd) 
{
	// wait for keyboard controller input buffer to be clear
	while ( (KeyboardReadControllerStatus () & KYBRD_CTRL_STATS_MASK_IN_BUF) != 0);
	HAL_OutputByteToPort(KYBRD_ENC_CMD_REG, cmd);
}

//	Keyboard interrupt handler
void I86KeyboardInterruptHandler() 
{
	asm("pushal");
	asm("cli");

	static bool _extended = false;

	int code = 0;

	// Read scan code only if the keyboard controller output buffer is full (scan code is in it)
	if (KeyboardReadControllerStatus() & KYBRD_CTRL_STATS_MASK_OUT_BUF) 
	{
		// Read the scan code
		code = KeyboardReadEncoderBuffer();

		// Is this an extended code? If so, set the flag and return
		if (code == 0xE0 || code == 0xE1)
		{
			_extended = true;
		}
		else 
		{
			// Either the second byte of an extended scan code or a single byte scan code
			_extended = false;

			// Test if this is a break code (Original XT Scan Code Set specific)
			if (code & 0x80) 
			{	
				// Convert the break code into its make code equivelant
				code -= 0x80;
				// Get the key
				int key = _keyboardScancode[code];
				// Test if a special key has been released and clear the appropriate flag
				switch (key) 
				{
					case KEY_LCTRL:
					case KEY_RCTRL:
						_ctrl = false;
						break;

					case KEY_LSHIFT:
					case KEY_RSHIFT:
						_shift = false;
						break;

					case KEY_LALT:
					case KEY_RALT:
						_alt = false;
						break;
				}
			}
			else 
			{
				// Save the scan code
				_scancode = code;
				// Get the key
				int key = _keyboardScancode[code];
				// Test if user is holding down any special keys and set the appropriate flags
				switch (key) 
				{
					case KEY_LCTRL:
					case KEY_RCTRL:
						_ctrl = true;
						break;

					case KEY_LSHIFT:
					case KEY_RSHIFT:
						_shift = true;
						break;

					case KEY_LALT:
					case KEY_RALT:
						_alt = true;
						break;

					case KEY_CAPSLOCK:
						_capslock = (_capslock) ? false : true;
						break;

					case KEY_KP_NUMLOCK:
						_numlock = (_numlock) ? false : true;
						break;

					case KEY_SCROLLLOCK:
						_scrolllock = (_scrolllock) ? false : true;
						break;
				}
			}
		}

		// Watch for errors
		switch (code) 
		{
			case KYBRD_ERR_BAT_FAILED:
				_keyboardBATResult = false;
				break;

			case KYBRD_ERR_DIAG_FAILED:
				_keyboardDiagResult = false;
				break;

			case KYBRD_ERR_RESEND_CMD:
				_keyboardResendResult = true;
				break;
		}
	}

	// Tell hal we are done
	HAL_InterruptDone(0);

	// Return from interrupt handler
	asm("sti");
	asm("popal");
	asm("leave");
	asm("iret");
}

// Public functions

// Return scroll lock state
bool KeyboardGetScrollLockState() 
{
	return _scrolllock;
}

// Return num lock state
bool KeyboardGetNumLockState() 
{
	return _numlock;
}

// Return caps lock state
bool KeyboardGetCapsLockState()	
{
	return _capslock;
}

// Return status of control key
bool KeyboardGetCtrlKeyState()	
{
	return _ctrl;
}

// Return status of alt key
bool KeyboardGetAltKeyState() 
{
	return _alt;
}

// Returns status of shift key
bool KeyboardGetShiftKeyState()	
{
	return _shift;
}

// Tell driver to ignore last resend request
void KeyboardIgnoreResend()	
{
	_keyboardResendResult = false;
}

// Return if system should redo last commands
bool KeyboardGetResendResult() 
{
	return _keyboardResendResult;
}

// Return diagnostics test result
bool KeyboardGetDiagnosticResult() 
{
	return _keyboardDiagResult;
}

// Return BAT test result
bool KeyboardGetBATResult() 
{
	return _keyboardBATResult;
}

// Return last scan code
uint8_t KeyboardGetLastScancode()	
{
	return _scancode;
}

// Set keyboard LED indicators
void KeyboardSetLeds(bool num, bool caps, bool scroll) 
{
	uint8_t data = 0;

	if (scroll)
	{
		data = data | 1;
	}
	if (num)
	{
		data = data | 2;
	}
	if (caps)
	{
		data = data | 4;
	}
	// Send the command to update the LEDs
	KeyboardSendEncoderCommand(KYBRD_ENC_CMD_SET_LED);
	KeyboardSendEncoderCommand(data);
}

// Get last key stroke
keycode KeyboardGetLastKey() 
{
	return (_scancode != INVALID_SCANCODE) ? ((keycode)_keyboardScancode[_scancode]) : (KEY_UNKNOWN);
}

// Discard last scan code
void KeyboardDiscardLastKey() 
{
	_scancode = INVALID_SCANCODE;
}

// Convert key to an ascii character
char KeyboardConvertKeyToASCII(keycode code) 
{
	uint8_t key = (uint8_t)code;

	// Ensure key is an ascii character
	if (isascii(code)) 
	{
		// If shift key is down or caps lock is on, make the key uppercase
		if (_shift || _capslock)
		{
			if (key >= 'a' && key <= 'z')
			{
				key -= 32;
			}
		}
		if (_shift && !_capslock)
		{
			// Handle shift on numeric keys
			if (key >= '0' && key <= '9')
			{
				switch(key) 
				{
					case '0':
						key = KEY_RIGHTPARENTHESIS;
						break;
					case '1':
						key = KEY_EXCLAMATION;
						break;
					case '2':
						key = KEY_AT;
						break;
					case '3':
						key = KEY_EXCLAMATION;
						break;
					case '4':
						key = KEY_HASH;
						break;
					case '5':
						key = KEY_PERCENT;
						break;
					case '6':
						key = KEY_CARRET;
						break;
					case '7':
						key = KEY_AMPERSAND;
						break;
					case '8':
						key = KEY_ASTERISK;
						break;
					case '9':
						key = KEY_LEFTPARENTHESIS;
						break;
				}
			}
			else 
			{
				switch (key) 
				{
					case KEY_COMMA:
						key = KEY_LESS;
						break;

					case KEY_DOT:
						key = KEY_GREATER;
						break;

					case KEY_SLASH:
						key = KEY_QUESTION;
						break;

					case KEY_SEMICOLON:
						key = KEY_COLON;
						break;

					case KEY_QUOTE:
						key = KEY_QUOTEDOUBLE;
						break;

					case KEY_LEFTBRACKET :
						key = KEY_LEFTCURL;
						break;

					case KEY_RIGHTBRACKET :
						key = KEY_RIGHTCURL;
						break;

					case KEY_GRAVE:
						key = KEY_TILDE;
						break;

					case KEY_MINUS:
						key = KEY_UNDERSCORE;
						break;

					case KEY_EQUAL:
						key = KEY_PLUS;
						break;

					case KEY_BACKSLASH:
						key = KEY_BAR;
						break;
				}
			}
		}
		// return the key
		return key;
	}

	// scan code != a valid ascii char so no convertion is possible
	return 0;
}

// Disable the keyboard
void KeyboardDisable() 
{
	KeyboardSendControllerCommand(KYBRD_CTRL_CMD_DISABLE);
	_keyboardDisabled = true;
}

// Enable the keyboard
void KeyboardEnable () 
{
	KeyboardSendControllerCommand(KYBRD_CTRL_CMD_ENABLE);
	_keyboardDisabled = false;
}

// Return true if keyboard is disabled
bool KeyboardIsDisabled() 
{
	return _keyboardDisabled;
}

// Reset the system
void KeyboardReset() 
{
	// Writes 11111110 to the output port (sets reset system line low)
	KeyboardSendControllerCommand(KYBRD_CTRL_CMD_WRITE_OUT_PORT);
	KeyboardSendEncoderCommand(0xfe);
}

// Run keyboard self test
bool KeyboardSelfTest() 
{
	// Send command
	KeyboardSendControllerCommand(KYBRD_CTRL_CMD_SELF_TEST);

	// Wait for output buffer to be full
	while (!(KeyboardReadControllerStatus () & KYBRD_CTRL_STATS_MASK_OUT_BUF));
	// if output buffer == 0x55, test passed
	return (KeyboardReadEncoderBuffer() == 0x55) ? true : false;
}

// Wait for a keystroke

keycode	KeyboardGetCharacter() 
{
	keycode key = KEY_UNKNOWN;

	// Wait for a keypress
	while (key == KEY_UNKNOWN)
	{
		key = KeyboardGetLastKey();
	}
	KeyboardSetLeds(_numlock, _capslock, _scrolllock);
	// Discard last keypress (we handled it) and return
	KeyboardDiscardLastKey();
	return key;
}

// Prepare keyboard driver for use
void KeyboardInstall(int irq) 
{
	// Install our interrupt handler (irq 1 uses interrupt 33)
	HAL_SetInterruptVector(irq, I86KeyboardInterruptHandler);

	// assume BAT test is good. If there is a problem, the IRQ handler will catch the error
	_keyboardBATResult = true;
	_scancode = 0;

	// Set lock keys and led lights
	_numlock = false;
	_scrolllock = false;
	_capslock = false;
	KeyboardSetLeds(false, false, false);

	// Shift, ctrl, and alt keys
	_shift = false;
	_alt = false;
	_ctrl = false;
}

