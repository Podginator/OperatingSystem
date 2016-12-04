
//	8237 ISA Direct Memory Access Controller (DMAC)

#include <Hal.h>
#include <dma.h>

// Mask a channel
void DMA_MaskChannel(uint8_t channel)
{
	if (channel <= 4)
	{
		HAL_OutputByteToPort(DMA0_CHANMASK_REG, (0x04 | channel));
	}
	else
	{
		HAL_OutputByteToPort(DMA1_CHANMASK_REG, (0x04 | (channel-4)));
	}
}

// Unmask a channel
void DMA_UnmaskChannel(uint8_t channel) 
{
	if (channel <= 4)
	{
		HAL_OutputByteToPort(DMA0_CHANMASK_REG, channel);
	}
	else
		HAL_OutputByteToPort(DMA1_CHANMASK_REG, channel - 4);
}

// Reset controller to defaults
void DMA_Reset(int dma)
{
	HAL_OutputByteToPort(DMA0_TEMP_REG, 0xff);
}

// Reset flipflop
void DMA_ResetFlipflop(int dma)
{
	if (dma < 2)
	{
		return;
	}
	HAL_OutputByteToPort((dma == 0) ? DMA0_CLEARBYTE_FLIPFLOP_REG : DMA1_CLEARBYTE_FLIPFLOP_REG, 0xff);
}

//! sets the address of a channel
void DMA_SetAddress(uint8_t channel, uint8_t low, uint8_t high) 
{
	if ( channel > 8 )
	{
		return;
	}
	unsigned short port = 0;
	switch (channel) 
	{
		case 0: 
			port = DMA0_CHAN0_ADDR_REG; 
			break;
			
		case 1: 
			port = DMA0_CHAN1_ADDR_REG; 
			break;
			
		case 2:
			port = DMA0_CHAN2_ADDR_REG; 
			break;
			
		case 3: 
			port = DMA0_CHAN3_ADDR_REG; 
			break;
			
		case 4:
			port = DMA1_CHAN4_ADDR_REG; 
			break;
			
		case 5: 
			port = DMA1_CHAN5_ADDR_REG; 
			break;
			
		case 6: 
			port = DMA1_CHAN6_ADDR_REG; 
			break;
			
		case 7: 
			port = DMA1_CHAN7_ADDR_REG; 
			break;
	}
	HAL_OutputByteToPort(port, low);
	HAL_OutputByteToPort(port, high);
}

//! sets the counter of a channel
void DMA_SetCount(uint8_t channel, uint8_t low, uint8_t high) 
{
	if (channel > 8)
	{
		return;
	}
	unsigned short port = 0;
	switch ( channel ) 
	{
		case 0: 
			port = DMA0_CHAN0_COUNT_REG; 
			break;
			
		case 1: 
			port = DMA0_CHAN1_COUNT_REG; 
			break;
			
		case 2: 
			port = DMA0_CHAN2_COUNT_REG; 
			break;
			
		case 3: 
			port = DMA0_CHAN3_COUNT_REG; 
			break;
			
		case 4: 
			port = DMA1_CHAN4_COUNT_REG; 
			break;
			
		case 5: 
			port = DMA1_CHAN5_COUNT_REG; 
			break;
			
		case 6: 
			port = DMA1_CHAN6_COUNT_REG; 
			break;
			
		case 7: 
			port = DMA1_CHAN7_COUNT_REG; 
			break;
	}
	HAL_OutputByteToPort(port, low);
	HAL_OutputByteToPort(port, high);
}

void DMA_SetMode(uint8_t channel, uint8_t mode) 
{
	int dma = (channel < 4) ? 0 : 1;
	int chan = (dma == 0) ? channel : channel-4;

	DMA_MaskChannel(channel);
	HAL_OutputByteToPort((channel < 4) ? (DMA0_MODE_REG) : DMA1_MODE_REG, chan | (mode) );
	DMA_UnmaskChannel(channel);
}

// Prepare channel for read
void DMA_SetRead(uint8_t channel) 
{
	DMA_SetMode(channel, DMA_MODE_READ_TRANSFER | DMA_MODE_TRANSFER_SINGLE);
}

// Prepare channel for write
void DMA_SetWrite(uint8_t channel) 
{
	DMA_SetMode(channel, DMA_MODE_WRITE_TRANSFER | DMA_MODE_TRANSFER_SINGLE);
}

// Write to an external page register
void DMA_SetExternalPageRegister(uint8_t reg, uint8_t val) 
{
	if (reg > 14)
	{
		return;
	}
	unsigned short port = 0;
	switch (reg) 
	{
		case 1: 
			port = DMA_PAGE_CHAN1_ADDRBYTE2;
			break;
			
		case 2: 
			port = DMA_PAGE_CHAN2_ADDRBYTE2; 
			break;
			
		case 3: 
			port = DMA_PAGE_CHAN3_ADDRBYTE2; 
			break;
			
		case 4: 
			// Nothing should ever write to register 4
			return;

		case 5: 
			port = DMA_PAGE_CHAN5_ADDRBYTE2; 
			break;
			
		case 6: 
			port = DMA_PAGE_CHAN6_ADDRBYTE2; 
			break;
			
		case 7: 
			port = DMA_PAGE_CHAN7_ADDRBYTE2; 
			break;
	}
	HAL_OutputByteToPort(port, val);
}

