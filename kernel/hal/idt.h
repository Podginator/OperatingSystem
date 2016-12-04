#ifndef _IDT_H
#define _IDT_H

//	Interrupt Descriptor Table. The IDT is responsible for providing
//  the interface for managing interrupts, installing, setting, requesting,
//	generating, and interrupt callback managing.
//

#include <stdint.h>

//  i86 defines 256 possible interrupt handlers (0-255)
#define I86_MAX_INTERRUPTS		256

#define I86_IDT_DESC_BIT16		0x06	//00000110
#define I86_IDT_DESC_BIT32		0x0E	//00001110
#define I86_IDT_DESC_RING1		0x40	//01000000
#define I86_IDT_DESC_RING2		0x20	//00100000
#define I86_IDT_DESC_RING3		0x60	//01100000
#define I86_IDT_DESC_PRESENT	0x80	//10000000

// Interrupt handler without error code
// Note: interrupt handlers are called by the processor. The stack setup may change
// so we leave it up to the interrupts' implimentation to handle it and properly return

typedef void (*I86_IRQ_HANDLER)();

// Interrupt descriptor
typedef struct _IDT_Descriptor 
{
	uint16_t		baseLo;		// Bits 0-16 of interrupt routine (ir) address
	uint16_t		sel;		// Code selector in GDT
	uint8_t			reserved;	// Reserved. Should be 0
	uint8_t			flags;		// Bit flags. Set with flags above
	uint16_t		baseHi;		// Bits 16-32 of ir address
} __attribute__((packed)) IDT_Descriptor;

// Return interrupt descriptor
IDT_Descriptor* I86_IDT_GetInterruptHandler(uint32_t i);

// Installs interrupt handler. When INT is fired, it will call this callback
int I86_IDT_InstallInterruptHandler(uint32_t i, 
									uint16_t flags, 
									uint16_t sel, 
									I86_IRQ_HANDLER irq);

									// Initialise basic IDT
int I86_IDT_Initialise(uint16_t codeSel);

#endif
