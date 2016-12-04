#ifndef _HAL_H
#define _HAL_H
//	Hardware Abstraction Layer Interface
//
//	The Hardware Abstraction Layer (HAL) provides an abstract interface
//	to control the basic motherboard hardware devices. This is accomplished
//	by abstracting hardware dependencies behind this interface.

#include <stdint.h>
#include <dma.h>

#define far
#define near

// Initialize hardware abstraction layer
int	 HAL_Initialise();

// Shutdown hardware abstraction layer
int	 HAL_Shutdown();

// Is HAL initialised?
bool HAL_IsInitialised();

// Generate interrupt
void HAL_GenerateInterrupt(int n);

// Notify hal interrupt is done
void HAL_InterruptDone(unsigned int intno);

// Read byte from device using port mapped io
uint8_t HAL_InputByteFromPort(uint16_t portid); 

// Write byte to device through port mapped io
void  HAL_OutputByteToPort(uint16_t portid, uint8_t value); 

// HAL_EnableInterrupts all hardware interrupts
void  HAL_EnableInterrupts();

// HAL_DisableInterrupts all hardware interrupts
void  HAL_DisableInterrupts();

// Set new interrupt vector
void  HAL_SetInterruptVector(int intno, void (*vect)());

// Return current interrupt vector
void (*HAL_GetInterruptVector(int intno))();

// Return CPU vender
const char *  HAL_GetCPUVendor();

// Return current tick count 
uint32_t HAL_GetTickCount();

// Wait for a specified number of tick counts
void HAL_Sleep(uint32_t tickCount); 

// Routines to enable/disable paging and load/get the page directory register

void HAL_EnablePaging(); 

void HAL_DisablePaging();

bool HAL_IsPaging(); 

void HAL_LoadPageDirectoryBaseRegister(uint32_t addr); 

uint32_t HAL_GetPageDirectoryBaseRegister(); 

#endif
