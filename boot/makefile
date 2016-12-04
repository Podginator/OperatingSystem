# Create the boot loader binaries

.DEFAULT_GOAL:=all

.SUFFIXES: .bin .asm 

%.bin: %.asm
	nasm -w+all -f bin -o $@ $<

boot.bin: boot.asm functions_16.asm bpb.asm

boot2.bin: boot2.asm functions_16.asm a20.asm messages.asm console.asm bpb.asm floppy16.asm fat12.asm memory.asm paging.asm

all: boot2.bin boot.bin

clean:
	rm -f boot.bin
	rm -f boot2.bin
