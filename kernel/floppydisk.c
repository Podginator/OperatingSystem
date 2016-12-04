#include <hal.h>
#include <floppydisk.h>

// Floppy disk support

//	Controller I/O Ports. 

#define FLPYDSK_DOR			0x3f2
#define	FLPYDSK_MSR			0x3f4
#define	FLPYDSK_FIFO		0x3f5
#define	FLPYDSK_CTRL		0x3f7

//	Bits 0-4 of command byte. 

#define	FDC_CMD_READ_TRACK		2
#define	FDC_CMD_SPECIFY			3
#define	FDC_CMD_CHECK_STAT		4
#define	FDC_CMD_WRITE_SECT		5
#define	FDC_CMD_READ_SECT		6
#define	FDC_CMD_CALIBRATE		7
#define	FDC_CMD_CHECK_INT		8
#define	FDC_CMD_FORMAT_TRACK	0xd
#define	FDC_CMD_SEEK			0xf

//	Additional command masks. Can be masked with above commands

#define	FDC_CMD_EXT_SKIP		0x20	//00100000
#define	FDC_CMD_EXT_DENSITY		0x40	//01000000
#define	FDC_CMD_EXT_MULTITRACK	0x80	//10000000

//	Digital Output Register

#define	FLPYDSK_DOR_MASK_DRIVE0				0	//00000000	
#define	FLPYDSK_DOR_MASK_DRIVE1				1	//00000001
#define	FLPYDSK_DOR_MASK_DRIVE2				2	//00000010
#define	FLPYDSK_DOR_MASK_DRIVE3				3	//00000011
#define	FLPYDSK_DOR_MASK_RESET				4	//00000100
#define	FLPYDSK_DOR_MASK_DMA				8	//00001000
#define	FLPYDSK_DOR_MASK_DRIVE0_MOTOR		16  //00010000
#define	FLPYDSK_DOR_MASK_DRIVE1_MOTOR		32	//00100000
#define	FLPYDSK_DOR_MASK_DRIVE2_MOTOR		64	//01000000
#define	FLPYDSK_DOR_MASK_DRIVE3_MOTOR		128	//10000000

//	Main Status Register

#define	FLPYDSK_MSR_MASK_DRIVE1_POS_MODE		1	//00000001
#define	FLPYDSK_MSR_MASK_DRIVE2_POS_MODE		2	//00000010
#define	FLPYDSK_MSR_MASK_DRIVE3_POS_MODE		4	//00000100
#define	FLPYDSK_MSR_MASK_DRIVE4_POS_MODE		8	//00001000
#define	FLPYDSK_MSR_MASK_BUSY					16	//00010000
#define	FLPYDSK_MSR_MASK_DMA					32	//00100000
#define	FLPYDSK_MSR_MASK_DATAIO					64  //01000000
#define	FLPYDSK_MSR_MASK_DATAREG				128	//10000000

//	Controller Status Port 0

#define	FLPYDSK_ST0_MASK_DRIVE0			0		//00000000	
#define	FLPYDSK_ST0_MASK_DRIVE1			1		//00000001
#define	FLPYDSK_ST0_MASK_DRIVE2			2		//00000010
#define	FLPYDSK_ST0_MASK_DRIVE3			3		//00000011
#define	FLPYDSK_ST0_MASK_HEADACTIVE		4		//00000100
#define	FLPYDSK_ST0_MASK_NOTREADY		8		//00001000
#define	FLPYDSK_ST0_MASK_UNITCHECK		16		//00010000
#define	FLPYDSK_ST0_MASK_SEEKEND		32		//00100000
#define	FLPYDSK_ST0_MASK_INTCODE		64		//11000000

// FLPYDSK_ST0_MASK_INTCODE types

#define	FLPYDSK_ST0_TYP_NORMAL			0
#define	FLPYDSK_ST0_TYP_ABNORMAL_ERR	1
#define	FLPYDSK_ST0_TYP_INVALID_ERR		2
#define	FLPYDSK_ST0_TYP_NOTREADY		3

//	GAP 3 sizes

#define	FLPYDSK_GAP3_LENGTH_STD 	    42
#define	FLPYDSK_GAP3_LENGTH_5_14	    32
#define	FLPYDSK_GAP3_LENGTH_3_5		    27

#define	FLPYDSK_SECTOR_DTL_128			0
#define	FLPYDSK_SECTOR_DTL_256			1
#define	FLPYDSK_SECTOR_DTL_512			2
#define	FLPYDSK_SECTOR_DTL_1024			4

// Floppy IRQ
const int FLOPPY_IRQ = 6;

// Sectors per track
const int FLPY_SECTORS_PER_TRACK = 18;

// dma tranfer buffer starts here and ends at 0x8000+64k
// You can change this as needed. It must be below 1MB and is a physical memory address
int DMA_BUFFER = 0x8000;

// FDC uses DMA channel 2
const int FDC_DMA_CHANNEL = 2;

// Current working drive. Defaults to 0 which should be fine on most systems
static uint8_t	_CurrentDrive = 0;

// Set when IRQ fires
static volatile uint8_t _FloppyDiskIRQ = 0;

typedef union
{
    uint8_t 		byte[4];
    unsigned long 	l;
} byte_accessable_long;

bool FloppyDriveDMAInitialise(uint8_t* buffer, unsigned int length)
{
	byte_accessable_long byteAccessableAddress;
	byte_accessable_long byteAccessableLength;

    byteAccessableAddress.l = (unsigned int)buffer;
    byteAccessableLength.l = length - 1;

	//Check for buffer issues
	if ((byteAccessableAddress.l >> 24) || 
        (byteAccessableLength.l >> 16) || 
	    (((byteAccessableAddress.l & 0xffff) + byteAccessableLength.l) >> 16))
	{
		return false;
    }
	DMA_Reset(1);
	DMA_MaskChannel(FDC_DMA_CHANNEL);
    DMA_ResetFlipflop(1);

    DMA_SetAddress(FDC_DMA_CHANNEL, byteAccessableAddress.byte[0], byteAccessableAddress.byte[1]);
    DMA_ResetFlipflop(1);

    DMA_SetCount(FDC_DMA_CHANNEL, byteAccessableLength.byte[0], byteAccessableLength.byte[1]);
	DMA_SetExternalPageRegister(2, 0);	

    DMA_UnmaskChannel(FDC_DMA_CHANNEL);
    return true;
}

// Set DMA base address
void FloppyDriveSetDMA(int addr) 
{
	DMA_BUFFER = addr;
}

// Basic Controller I/O Routines

// Return floppy disk controller status

uint8_t FloppyDriveReadStatus() 
{
	// Just return main status register
	return HAL_InputByteFromPort(FLPYDSK_MSR);
}

// Write to the floppy drive controller digital output register
void FloppyDriveWriteToDOR(uint8_t val ) 
{
	HAL_OutputByteToPort(FLPYDSK_DOR, val);
}

// Send command byte to floppy drive controller
void FloppyDriveSendCommand(uint8_t cmd) 
{
	while (!(FloppyDriveReadStatus() & FLPYDSK_MSR_MASK_DATAREG));
	HAL_OutputByteToPort(FLPYDSK_FIFO, cmd);
}

// Get data from floppy drive controller
uint8_t FloppyDriveReadData() 
{
	while (!(FloppyDriveReadStatus() & FLPYDSK_MSR_MASK_DATAREG));
	return HAL_InputByteFromPort(FLPYDSK_FIFO);
}

//  write to the configuation control register
void FloppyDriveWriteToCCR(uint8_t val) 
{
	HAL_OutputByteToPort(FLPYDSK_CTRL, val);
}

//	Interrupt Handling Routines

// Wait for IRQ to fire
void FloppyDriveWaitForInterrupt() 
{
	while (_FloppyDiskIRQ == 0)
	{
		HAL_Sleep(1);
	}
	_FloppyDiskIRQ = 0;
}

// Floppy disk IRQ handler
void I86_FloppyDriveInterruptHandler() 
{
	asm("pushal");
	asm("cli");

	// Indicate IRQ fired
	_FloppyDiskIRQ = 1;

	// Tell HAL we are done
	HAL_InterruptDone(FLOPPY_IRQ);

	asm("sti");
	asm("popal");
	asm("leave");
	asm("iret");
}

//	Controller Command Routines

// Check interrupt status command
void FloppyDriveCheckInterruptStatus(uint32_t* st0, uint32_t* cyl) 
{
	FloppyDriveSendCommand(FDC_CMD_CHECK_INT);
	*st0 = FloppyDriveReadData();
	*cyl = FloppyDriveReadData();
}

// Turn the current floppy drives motor on/off
void FloppyDriveControlMotor(bool b) 
{
	// Sanity check for invalid drive
	if (_CurrentDrive > 3)
	{
		return;
	}
	uint8_t motor = 0;

	//! select the correct mask based on current drive
	switch (_CurrentDrive) 
	{
		case 0:
			motor = FLPYDSK_DOR_MASK_DRIVE0_MOTOR;
			break;
		case 1:
			motor = FLPYDSK_DOR_MASK_DRIVE1_MOTOR;
			break;
		case 2:
			motor = FLPYDSK_DOR_MASK_DRIVE2_MOTOR;
			break;
		case 3:
			motor = FLPYDSK_DOR_MASK_DRIVE3_MOTOR;
			break;
	}

	// Turn on or off the motor of that drive
	if (b)
	{
		FloppyDriveWriteToDOR((uint8_t)(_CurrentDrive | motor | FLPYDSK_DOR_MASK_RESET | FLPYDSK_DOR_MASK_DMA));
	}
	else
	{
		FloppyDriveWriteToDOR(FLPYDSK_DOR_MASK_RESET);
	}
	// In all cases, wait a little bit for the motor to spin up/turn off
	HAL_Sleep(20);
}

// Configure drive
void FloppyDriveConfigure(uint8_t stepr, uint8_t loadt, uint8_t unloadt, bool dma ) 
{
	uint8_t data = 0;

	FloppyDriveSendCommand(FDC_CMD_SPECIFY);
	data = ((stepr & 0xf) << 4) | (unloadt & 0xf);
	FloppyDriveSendCommand(data);
	data = (( loadt << 1 ) | ( (dma) ? 0 : 1 ));
	FloppyDriveSendCommand(data);
}

// Calibrate the drive
int FloppyDriveCalibrate(uint8_t drive) 
{
	uint32_t st0;
	uint32_t cyl;

	if (drive >= 4)
	{
		return -2;
	}
	// Turn on the motor
	FloppyDriveControlMotor(true);
	for (int i = 0; i < 10; i++) 
	{
		FloppyDriveSendCommand(FDC_CMD_CALIBRATE);
		FloppyDriveSendCommand( drive );
		FloppyDriveWaitForInterrupt();
		FloppyDriveCheckInterruptStatus( &st0, &cyl);

		// Did we find cylinder 0? if so, we are done
		if (!cyl) 
		{
			FloppyDriveControlMotor(false);
			return 0;
		}
	}
	FloppyDriveControlMotor(false);
	return -1;
}

// Disable controller
void FloppyDriveDisableController() 
{
	FloppyDriveWriteToDOR(0);
}

//! enable controller
void FloppyDriveEnableController() 
{
	FloppyDriveWriteToDOR(FLPYDSK_DOR_MASK_RESET | FLPYDSK_DOR_MASK_DMA);
}

// Reset controller
void FloppyDriveReset() 
{
	uint32_t st0;
	uint32_t cyl;

	FloppyDriveDisableController();
	FloppyDriveEnableController();
	FloppyDriveWaitForInterrupt();

	// Send CHECK_INT/SENSE INTERRUPT command to all drives
	for (int i=0; i<4; i++)
	{
		FloppyDriveCheckInterruptStatus(&st0,&cyl);
	}
	// Transfer speed 500kb/s
	FloppyDriveWriteToCCR(0);

	// Pass mechanical drive info. steprate=3ms, unload time=240ms, load time=16ms
	FloppyDriveConfigure(3,16,240,true);

	//! calibrate the disk
	FloppyDriveCalibrate( _CurrentDrive );
}

// Read a sector
void FloppyDriveReadSectorHTS(uint8_t head, uint8_t track, uint8_t sector) 
{
	uint32_t st0;
	uint32_t cyl;

	// Initialize DMA
	FloppyDriveDMAInitialise((uint8_t*)DMA_BUFFER, 512 );

	// Set the DMA for read transfer
	DMA_SetRead(FDC_DMA_CHANNEL);
	
	// Read in a sector
	FloppyDriveSendCommand(FDC_CMD_READ_SECT | FDC_CMD_EXT_MULTITRACK | FDC_CMD_EXT_SKIP | FDC_CMD_EXT_DENSITY);
	FloppyDriveSendCommand(head << 2 | _CurrentDrive);
	FloppyDriveSendCommand(track);
	FloppyDriveSendCommand(head);
	FloppyDriveSendCommand(sector);
	FloppyDriveSendCommand(FLPYDSK_SECTOR_DTL_512 );
	uint8_t endSector = sector + 1;
	if (endSector >= FLPY_SECTORS_PER_TRACK)
	{
		endSector = FLPY_SECTORS_PER_TRACK; 
	}
	FloppyDriveSendCommand(endSector);
	FloppyDriveSendCommand(FLPYDSK_GAP3_LENGTH_3_5 );
	FloppyDriveSendCommand(0xff);
	FloppyDriveWaitForInterrupt();
	
	// Read status info
	for (int j=0; j<7; j++)
	{
		FloppyDriveReadData();
	}
	// Let FDC know we handled interrupt
	FloppyDriveCheckInterruptStatus(&st0,&cyl);
}

// Seek to given track/cylinder
int FloppyDriveSeek(uint8_t cyl, uint8_t head) 
{
	uint32_t st0;
	uint32_t cyl0;

	if (_CurrentDrive >= 4)
	{
		return -1;
	}
	for (int i = 0; i < 10; i++ ) 
	{
		FloppyDriveSendCommand(FDC_CMD_SEEK);
		FloppyDriveSendCommand((head) << 2 | _CurrentDrive);
		FloppyDriveSendCommand(cyl);
		FloppyDriveWaitForInterrupt();
		FloppyDriveCheckInterruptStatus(&st0,&cyl0);
		if (cyl0 == cyl)
		{
			// We have found the cylinder
			return 0;
		}
	}
	return -1;
}

// Convert LBA to CHS
void FloppyDriveLBAToCHS(int lba,int *head,int *track,int *sector) 
{
   *head = (lba % (FLPY_SECTORS_PER_TRACK * 2 )) / (FLPY_SECTORS_PER_TRACK);
   *track = lba / (FLPY_SECTORS_PER_TRACK * 2 );
   *sector = lba % FLPY_SECTORS_PER_TRACK + 1;
}

// Install floppy driver
void FloppyDriveInstall(int irq) 
{
	// Install interrupt handler
	HAL_SetInterruptVector(irq, I86_FloppyDriveInterruptHandler);
	// Reset the floppy drive controller
	FloppyDriveReset();
	// Set drive information
	FloppyDriveConfigure(13, 1, 0xf, true);
}

// Set current working drive
void FloppyDriveSetWorkingDrive(uint8_t drive) 
{
	if (drive < 4)
	{
		_CurrentDrive = drive;
	}
}

//! get current working drive
uint8_t FloppyDriveGetWorkingDrive() 
{
	return _CurrentDrive;
}

// read a sector
uint8_t* FloppyDriveReadSector(int sectorLBA) 
{
	// The following line is put it because we were
	// encountering problems under Bochs when we do a seek
	// following a read - the seek command could not be sent.
	FloppyDriveReset();
	if (_CurrentDrive >= 4)
	{
		return 0;
	}
	// Convert LBA sector to CHS
	int head = 0;
	int	track = 0;
	int sector = 1;
	FloppyDriveLBAToCHS(sectorLBA, &head, &track, &sector);
	// Turn motor on and seek to track on both heads
	FloppyDriveControlMotor(true);
	if (FloppyDriveSeek((uint8_t)track, (uint8_t)head) != 0)
	{
		return 0;
	}
	HAL_Sleep(10);
	// Read sector and turn motor off
	FloppyDriveReadSectorHTS((uint8_t)head, (uint8_t)track, (uint8_t)sector);
	FloppyDriveControlMotor(false);

	return (uint8_t*)DMA_BUFFER;
}

