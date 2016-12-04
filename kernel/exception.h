
#ifndef _EXCEPTION_H
#define _EXCEPTION_H

// Divide by zero
void DivideByZeroFault(); 

// Single Step
void SingleStepTrap();

// Non-Maskable Interrupt
void NMITrap(); 

// Breakpoint hit
void BreakpointTrap(); 

// Overflow
void OverflowTrap(); 

// Bounds check
void BoundsCheckFault(); 

// Invalid opcode / instruction
void InvalidOpcodeFault(); 

// Device not available
void NoDeviceFault(); 

// Double fault
void DoubleFaultAbort(unsigned int err); 

// Invalid Task State Segment (TSS)
void InvalidTSSFault(unsigned int err); 

// Segment not present
void NoSegmentFault(unsigned int err); 

// Stack fault
void StackFault(unsigned int err); 

// General protection fault
void GeneralProtectionFault(unsigned int err); 

// Page fault
void PageFault(unsigned int err); 

// Floating Point Unit (FPU) error
void FPUFault(); 

// Alignment check
void AlignmentCheckFault(unsigned int err); 

// machine check
void MachineCheckAbort(); 

// Floating Point Unit (FPU) Single Instruction Multiple Data (SIMD) error
void SimdFPUFault(); 

#endif
