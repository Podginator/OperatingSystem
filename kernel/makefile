# Create the kernel bin

.DEFAULT_GOAL:=all

CFLAGS= -ffreestanding -m32 -march=pentium -I../include/
OBJS= kernel_main.o console.o string.o exception.o physicalmemorymanager.o virtualmemorymanager.o vm_pte.o vm_pde.o command.o keyboard.o floppydisk.o filesystem.o disk_command.o
HAL_OBJS = hal/cpu.o hal/gdt.o hal/hal.o hal/idt.o hal/pic.o hal/pit.o hal/dma.o

.SUFFIXES: .bin .asm .sys .o

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

kernelentry.o: kernelentry.asm
	nasm -w+all -f elf -o kernelentry.o kernelentry.asm

hal.lib: $(HAL_OBJS)
		ar -cvq hal.lib $(HAL_OBJS)
	
kernel.sys: kernelentry.o $(OBJS) hal.lib
	ld -o kernel.bin -T linkerscript.txt -mi386pe kernelentry.o $(OBJS) hal.lib
	objcopy -O binary kernel.bin kernel.sys
	
all: kernel.sys

clean:
	rm -f *.o
	rm -f hal/*.o
	rm -f hal.lib
	rm -f kernel.bin
	rm -f kernel.sys
	
