#ifndef _DMA_H_INCLUDED
# define _DMA_H_INCLUDED

//  8237 ISA Direct Memory Access Controller (DMAC)

#include <stdint.h>

//	2 DMACs, 32 bit master & 16bit slave each having 8 channels

#define DMA_MAX_CHANNELS 16
#define DMA_CHANNELS_PER_DMAC 8

//	DMA0 address/count registers

#define	DMA0_CHAN0_ADDR_REG   		0
#define	DMA0_CHAN0_COUNT_REG  		1
#define	DMA0_CHAN1_ADDR_REG   		2
#define	DMA0_CHAN1_COUNT_REG  		3
#define	DMA0_CHAN2_ADDR_REG   		4
#define	DMA0_CHAN2_COUNT_REG  		5
#define	DMA0_CHAN3_ADDR_REG   		6
#define	DMA0_CHAN3_COUNT_REG  		7

//	Generic DMA0 registers

#define	DMA0_STATUS_REG 	 		0x08
#define	DMA0_COMMAND_REG 	 		0x08
#define	DMA0_REQUEST_REG 	 		0x09
#define	DMA0_CHANMASK_REG 	 		0x0a
#define	DMA0_MODE_REG 		 		0x0b
#define	DMA0_CLEARBYTE_FLIPFLOP_REG 0x0c
#define	DMA0_TEMP_REG 				0x0d
#define	DMA0_MASTER_CLEAR_REG 		0x0d
#define	DMA0_CLEAR_MASK_REG 		0x0e
#define	DMA0_MASK_REG 				0x0f

//	DMA1 address/count registers

#define	DMA1_CHAN4_ADDR_REG 		0xc0
#define	DMA1_CHAN4_COUNT_REG 		0xc2
#define	DMA1_CHAN5_ADDR_REG 		0xc4
#define	DMA1_CHAN5_COUNT_REG 		0xc6
#define	DMA1_CHAN6_ADDR_REG 		0xc8
#define	DMA1_CHAN6_COUNT_REG 		0xca
#define	DMA1_CHAN7_ADDR_REG 		0xcc
#define	DMA1_CHAN7_COUNT_REG 		0xce

//	DMA External Page Registers

#define	DMA_PAGE_EXTRA0 			0x80
#define	DMA_PAGE_CHAN2_ADDRBYTE2 	0x81
#define	DMA_PAGE_CHAN3_ADDRBYTE2 	0x82
#define	DMA_PAGE_CHAN1_ADDRBYTE2 	0x83
#define	DMA_PAGE_EXTRA1 			0x84
#define	DMA_PAGE_EXTRA2 			0x85
#define	DMA_PAGE_EXTRA3 			0x86
#define	DMA_PAGE_CHAN6_ADDRBYTE2 	0x87
#define	DMA_PAGE_CHAN7_ADDRBYTE2 	0x88
#define	DMA_PAGE_CHAN5_ADDRBYTE2 	0x89
#define	DMA_PAGE_EXTRA4 			0x8c
#define	DMA_PAGE_EXTRA5 			0x8d
#define	DMA_PAGE_EXTRA6 			0x8e
#define	DMA_PAGE_DRAM_REFRESH 		0x8f

//	Generic DMA1 registers

#define	DMA1_STATUS_REG 			0xd0
#define	DMA1_COMMAND_REG 			0xd0
#define	DMA1_REQUEST_REG 			0xd2
#define	DMA1_CHANMASK_REG 			0xd4
#define	DMA1_MODE_REG 				0xd6
#define	DMA1_CLEARBYTE_FLIPFLOP_REG 0xd8
#define	DMA1_INTER_REG				0xda
#define	DMA1_UNMASK_ALL_REG 		0xdc
#define	DMA1_MASK_REG 				0xde

//	DMA mode bit mask (Non MASK Values can be ORed together)

#define	DMA_MODE_MASK_SEL 			3

#define	DMA_MODE_MASK_TRA 			0xc
#define	DMA_MODE_SELF_TEST 			0
#define	DMA_MODE_READ_TRANSFER 		4
#define	DMA_MODE_WRITE_TRANSFER 	8

#define	DMA_MODE_MASK_AUTO 			0x10
#define	DMA_MODE_MASK_IDEC 			0x20

#define	DMA_MODE_MASK 				0xc0
#define	DMA_MODE_TRANSFER_ON_DEMAND 0
#define	DMA_MODE_TRANSFER_SINGLE 	0x40
#define	DMA_MODE_TRANSFER_BLOCK 	0x80
#define	DMA_MODE_TRANSFER_CASCADE 	0xC0

#define	DMA_CMD_MASK_MEMTOMEM 		1
#define	DMA_CMD_MASK_CHAN0ADDRHOLD 	2
#define	DMA_CMD_MASK_ENABLE 		4
#define	DMA_CMD_MASK_TIMING 		8
#define	DMA_CMD_MASK_PRIORITY 		0x10
#define	DMA_CMD_MASK_WRITESEL 		0x20
#define	DMA_CMD_MASK_DREQ 			0x40
#define	DMA_CMD_MASK_DACK 			0x80

void DMA_MaskChannel(uint8_t channel);
void DMA_UnmaskChannel(uint8_t channel); 
void DMA_Reset(int dma);
void DMA_ResetFlipflop(int dma);
void DMA_SetAddress(uint8_t channel, uint8_t low, uint8_t high); 
void DMA_SetCount(uint8_t channel, uint8_t low, uint8_t high); 
void DMA_SetMode(uint8_t channel, uint8_t mode); 
void DMA_SetRead(uint8_t channel); 
void DMA_SetWrite(uint8_t channel); 
void DMA_SetExternalPageRegister(uint8_t reg, uint8_t val); 

#endif
