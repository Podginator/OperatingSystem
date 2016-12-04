//  pit.c
//
//	8253 Programmable Interval Timer

#include "idt.h"
#include "pit.h"
#include "pic.h"
#include <hal.h>

//	Controller Registers

#define		I86_PIT_REG_COUNTER0		0x40
#define		I86_PIT_REG_COUNTER1		0x41
#define		I86_PIT_REG_COUNTER2		0x42
#define		I86_PIT_REG_COMMAND			0x43

//! Global Tick count
static volatile uint32_t			_pit_ticks = 0;

// Test if pit is initialized
static bool							_pit_IsInitialised = false;

//	PIT timer interrupt handler
void  I86_PIT_TimerInterruptHandler() 
{
	// Prevent further interrupts while
	// we are processing this one
	asm("cli");
	
	// Save the registers
	asm("pushal");

	// Increment tick count
	_pit_ticks++;

	// Tell hal we are done
	HAL_InterruptDone(0);

	// Restore registers
	asm("popal");
	
	// Reenable interrupts
	asm("sti");
	
	// Since this is an interrupt handler, we
	// need to finish via iret, not ret
	asm("leave");
	asm("iret");
}

// Sets new pit tick count and returns previouw. value
uint32_t I86_PIT_SetTickCount(uint32_t i) 
{
	uint32_t ret = _pit_ticks;
	_pit_ticks = i;
	return ret;
}

// Returns current tick count
uint32_t I86_PIT_HAL_GetTickCount() 
{
	return _pit_ticks;
}

// Send command to pit
void I86_PIT_SendCommand(uint8_t cmd) 
{
	HAL_OutputByteToPort(I86_PIT_REG_COMMAND, cmd);
}

// send data to a counter
void I86_PIT_SendData(uint16_t data, uint8_t counter) 
{
	uint8_t	port = (counter == I86_PIT_OCW_COUNTER_0) ? I86_PIT_REG_COUNTER0 :
													    ((counter == I86_PIT_OCW_COUNTER_1) ? I86_PIT_REG_COUNTER1 : I86_PIT_REG_COUNTER2);
	HAL_OutputByteToPort(port, (uint8_t)data);
}

// Read data from counter
uint8_t I86_PIT_ReadData(uint16_t counter) 
{
	uint8_t	port = (counter == I86_PIT_OCW_COUNTER_0) ? I86_PIT_REG_COUNTER0 :
		                                                ((counter == I86_PIT_OCW_COUNTER_1) ? I86_PIT_REG_COUNTER1 : I86_PIT_REG_COUNTER2);
	return HAL_InputByteFromPort(port);
}

// Start a counter
void I86_PIT_StartCounter(uint32_t freq, uint8_t counter, uint8_t mode) 
{
	if (freq == 0)
	{
		return;
	}
	uint16_t divisor = (uint16_t)(1193181 / (uint16_t)freq);

	// Send operational command
	uint8_t ocw=0;
	ocw = (ocw & ~I86_PIT_OCW_MASK_MODE) | mode;
	ocw = (ocw & ~I86_PIT_OCW_MASK_RL) | I86_PIT_OCW_RL_DATA;
	ocw = (ocw & ~I86_PIT_OCW_MASK_COUNTER) | counter;
	I86_PIT_SendCommand(ocw);

	// Set frequency rate
	I86_PIT_SendData(divisor & 0xff, 0);
	I86_PIT_SendData((divisor >> 8) & 0xff, 0);

	// Reset tick count
	_pit_ticks = 0;
}

// Initialise minidriver
void  I86_PIT_Initialise() 
{
	// Install our interrupt handler (IRQ 0 uses interrupt 32)
	HAL_SetInterruptVector (32, I86_PIT_TimerInterruptHandler);

	// We are initialised
	_pit_IsInitialised = true;
}

// Test if pit interface is initialized
bool  I86_PIT_IsInitialised () 
{
	return _pit_IsInitialised;
}

