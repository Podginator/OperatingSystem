//    cpu.c
//
//	  This is the processor interface. Everything outside of this module
//	must use this interface when working on processor data.
//
//	A processor is a module that manages the very basic data structures
//	and data within the system. The processor interface provides the interface
//	for managing processors, processor cores, accessing processor structures,
//	and more

#include "cpu.h"
#include "gdt.h"
#include "idt.h"

// Initialises CPU resources

int I86_CPU_Initialise() 
{
	// initialise processor tables
	I86_GDT_Initialise();
	I86_IDT_Initialise(0x8);
	return 0;
}

// Shut down CPU resources

void I86_CPU_Shutdown() 
{
	// Nothing to do here
}

// Returns vender name of CPU

char * I86_CPU_GetVendor() 
{
	static char	vendor[32] = {0};

	asm volatile("movl 	$0, %%eax \n\t"
	             "cpuid \n\t"
				 "movl	%%ebx, (%%edi) \n\t"
				 "movl	%%edx, 4(%%edi) \n\t" 
				 "movl	%%ecx, 8(%%edi)" 
				 : 
				 :[vendor] "D" (vendor));
	return vendor;
}
