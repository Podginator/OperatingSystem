
#ifndef _FLPYDSK_DRIVER_H
#define _FLPYDSK_DRIVER_H

// Floppy disk support

#include <stdint.h>

// Set address for floppy drive to use for DMA transfers
void FloppyDriveSetDMA(int addr);

// Convert LBA to CHS
void FloppyDriveLBAToCHS(int lba,int *head,int *track,int *sector); 

// Install floppy driver
void FloppyDriveInstall(int irq); 

// Set current working drive
void FloppyDriveSetWorkingDrive(uint8_t drive); 

//! get current working drive
uint8_t FloppyDriveGetWorkingDrive(); 

// Read a sector
uint8_t* FloppyDriveReadSector(int sectorLBA); 

#endif
