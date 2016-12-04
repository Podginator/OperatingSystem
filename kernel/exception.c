//	System exception handlers. These are registered during system
//	initialization and called automatically when they are encountered

#include "exception.h"
#include <hal.h>
#include <console.h>

// For now, all of these interrupt handlers just disable hardware interrupts
// and calls kernal_panic(). This displays an error and halts the system

void KernelPanic (const char* message);

// Divide by zero
void DivideByZeroFault() 
{
	KernelPanic("Divide by 0");
	for (;;);
}

// Single Step
void SingleStepTrap() 
{
	KernelPanic("Single step");
	for (;;);
}

// Non-Maskable Interrupt
void NMITrap() 
{
	KernelPanic("NMI trap");
	for (;;);
}

// Breakpoint hit
void BreakpointTrap() 
{
	KernelPanic("Breakpoint trap");
	for (;;);
}

// Overflow
void OverflowTrap() 
{
	KernelPanic("Overflow trap");
	for (;;);
}

// Bounds check
void BoundsCheckFault() 
{
	KernelPanic("Bounds check fault");
	for (;;);
}

// Invalid opcode / instruction
void InvalidOpcodeFault() 
{
	KernelPanic("Invalid opcode");
	for (;;);
}

// Device not available
void NoDeviceFault() 
{
	KernelPanic("Device not found");
	for (;;);
}

// Double fault
void DoubleFaultAbort(unsigned int err) 
{
	KernelPanic("Double fault");
	for (;;);
}

// Invalid Task State Segment (TSS)
void InvalidTSSFault(unsigned int err) 
{
	KernelPanic("Invalid TSS");
	for (;;);
}

// Segment not present
void NoSegmentFault(unsigned int err) 
{
	KernelPanic("Invalid segment");
	for (;;);
}

// Stack fault
void StackFault(unsigned int err) 
{
	KernelPanic("Stack fault");
	for (;;);
}

// General protection fault
void GeneralProtectionFault(unsigned int err) 
{
	KernelPanic("General Protection Fault");
	for (;;);
}

// Page fault
void PageFault(unsigned int err) 
{
	KernelPanic("Page Fault");
	for (;;);
}

// Floating Point Unit (FPU) error
void FPUFault() 
{
	KernelPanic("FPU Fault");
	for (;;);
}

// Alignment check
void AlignmentCheckFault(unsigned int err) 
{
	KernelPanic("Alignment Check");
	for (;;);
}

// machine check
void MachineCheckAbort() 
{
	KernelPanic("Machine Check");
	for (;;);
}

// Floating Point Unit (FPU) Single Instruction Multiple Data (SIMD) error
void SimdFPUFault() 
{
	KernelPanic("FPU SIMD fault");
	for (;;);
}

// Something has gone very wrong. We cannot continue.
void KernelPanic(const char* message) 
{
	HAL_DisableInterrupts();

	//ConsoleClearScreen(0x1f);
	ConsoleWriteString((char *)message);
	for (;;);
}

