#ifndef _CPU_H_INCLUDED
#define _CPU_H_INCLUDED

#include <stdint.h>
#include "regs.h"

// Initialize the processors
int I86_CPU_Initialise();

// Shutdown the processors
void I86_CPU_Shutdown();

// Get cpu vender
char * I86_CPU_GetVendor();

#endif
