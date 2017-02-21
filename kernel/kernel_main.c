#include <string.h>
#include <hal.h>
#include <console.h>
#include <keyboard.h>
#include <floppydisk.h>
#include <command.h>
#include "exception.h"
#include "physicalmemorymanager.h"
#include "virtualmemorymanager.h"
#include "bootinfo.h"
#include <filesystem.h>

BootInfo *	_bootInfo;

// This is a dummy __main.  For some reason, gcc puts in a call to 
// __main from main, so we just include a dummy.
 
void __main() {}

void InitialiseInterrupts()
{
	HAL_EnableInterrupts();

	// Install our exception handlers
	HAL_SetInterruptVector (0, DivideByZeroFault);
	HAL_SetInterruptVector (1, SingleStepTrap);
	HAL_SetInterruptVector (2, NMITrap);
	HAL_SetInterruptVector (3, BreakpointTrap);
	HAL_SetInterruptVector (4, OverflowTrap);
	HAL_SetInterruptVector (5, BoundsCheckFault);
	HAL_SetInterruptVector (6, InvalidOpcodeFault);
	HAL_SetInterruptVector (7, NoDeviceFault);
	HAL_SetInterruptVector (8, DoubleFaultAbort);
	HAL_SetInterruptVector (10, InvalidTSSFault);
	HAL_SetInterruptVector (11, NoSegmentFault);
	HAL_SetInterruptVector (12, StackFault);
	HAL_SetInterruptVector (13, GeneralProtectionFault);
	HAL_SetInterruptVector (14, PageFault);
	HAL_SetInterruptVector (16, FPUFault);
	HAL_SetInterruptVector (17, AlignmentCheckFault);
	HAL_SetInterruptVector (18, MachineCheckAbort);
	HAL_SetInterruptVector (19, SimdFPUFault);
}

void InitialisePhysicalMemory()
{
	// Initialise the physical memory manager
	// We place the memory bit map in the next available block of memory after the kernel

	uint32_t memoryMapAddress = 0x100000 + _bootInfo->KernelSize;
	if (memoryMapAddress % PMM_GetBlockSize() != 0)
	{
		// Make sure that the memory map is going to be aligned on a block boundary
		memoryMapAddress = (memoryMapAddress / PMM_GetBlockSize() + 1) * PMM_GetBlockSize();
	}
	uint32_t sizeOfMemoryMap = PMM_Initialise(_bootInfo, memoryMapAddress);

	// We now need to mark various regions as unavailable
	
	// Mark first page as unavailable (so that a null pointer is invalid)
	PMM_MarkRegionAsUnavailable(0x00, PMM_GetBlockSize());

	// Mark memory used by kernel as unavailable
	PMM_MarkRegionAsUnavailable(0x100000, _bootInfo->KernelSize);

	// Mark memory used by memory map as unavailable
	PMM_MarkRegionAsUnavailable(PMM_GetMemoryMap(), sizeOfMemoryMap);

	// Reserve two blocks for the stack and make unavailable (the stack is set at 0x90000 in boot loader)
	uint32_t stackSize = PMM_GetBlockSize() * 2;
	PMM_MarkRegionAsUnavailable(_bootInfo->StackTop - stackSize, stackSize);
	
	// Reserve block used for DMA transfers
	PMM_MarkRegionAsUnavailable(0x8000, 4096);
}

void Initialise()
{
	ConsoleClearScreen(0x1F);
	ConsoleWriteString("Welcome to TomOs. PG_UP/F1 to ScrollUp, PG_Down/F2 to ScrollDown. \n");
	HAL_Initialise();
	InitialiseInterrupts();
	InitialisePhysicalMemory();
	// Switch to using our own page tables, rather than the temporary
	// ones created by the boot loader
	VMM_Initialise();
	// Install keyboard driver
	KeyboardInstall(33);
	// Set boot drive as current drive
	FloppyDriveSetWorkingDrive(_bootInfo->BootDevice);
	// install floppy disk to interrupt vector 38, uses IRQ 6
	FloppyDriveInstall(38);
	FsFat12_Initialise();
}


// Test the Close Function 
void testFileClose()
{
	FILE file;
	file.Eof = 0; 
	FsFat12_Close(&file);
	if (file.Eof) 
	{
		ConsoleWriteString("File Successfully closed");
	}
	FsFat12_Close(NULL);
}

// Test reading a file that is NULL, or Invalid 
void testFileReadNull()
{
	char testBuffer[128];
	// This should return 0. 
	ConsoleWriteString("Testing with NULL : ");
	ConsoleWriteInt(FsFat12_Read(NULL, testBuffer, 128), 10);

	// Test a file with CurrentCluster set to 0.
	FILE file;
	file.Eof = 0; 
	file.FileLength = 100;
	file.CurrentCluster = 0;
	ConsoleWriteString("\nTesting with file with INVALID starting Cluster : ");
	// Should be 0 again.
	ConsoleWriteInt(FsFat12_Read(&file, testBuffer, 128), 10);
	ConsoleWriteString("\n");
}

void testOpen() 
{
	FILE file  = FsFat12_Open("\\Testing");
	if (file.Flags == FS_DIRECTORY)
	{
		ConsoleWriteString("\\Testing\\Testing Open Returned FS_DIRECTORY\n");
	}

	file = FsFat12_Open("\\Testing\\TestTwoLongFileName.txt");
	if (file.Flags == FS_FILE)
	{
		ConsoleWriteString("\\Testing\\TestTwoLongFileName.txt Open Returned FS_FILE\n");
	}

	// Test Opening NULL 
	file = FsFat12_Open(NULL); 
	if (file.Flags == FS_INVALID)
	{
		ConsoleWriteString("Null Open Returned FS_INVALID\n");
	}

	file = FsFat12_Open("");
	if (file.Flags == FS_INVALID)
	{
		ConsoleWriteString("empty string Open Returned FS_INVALID\n");
	}
}

void main(BootInfo * bootInfo) 
{
	_bootInfo = bootInfo;
	Initialise();

	//testFileClose();
	//testFileReadNull();
	//testOpen();

	Run();
	while (1);
}
