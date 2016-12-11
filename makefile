# Create a bootable CD ile from our boot code

.DEFAULT_GOAL:=all

IMAGE 		= uodos
BOOTLOADER  = boot/boot.bin boot/boot2.bin

.SUFFIXES: .iso .img .bin .asm .sys .o .lib
.PHONY:  bootloader kernel

bootloader:
	cd boot && $(MAKE)

kernel:
	cd kernel && $(MAKE)

$(IMAGE).img : bootloader kernel
#	Get the blank floppy disk image
	cp floppy_image/uodos.img $(IMAGE).img
#	Copy our new boot sector over to the floppy image
	dd status=noxfer conv=notrunc if=boot/boot.bin of=$(IMAGE).img
#   Copy our second stage of the boot to the floppy image
	dd status=noxfer conv=notrunc seek=1 if=boot/boot2.bin of=$(IMAGE).img
#	Mount floppy image file as Z:
	imdisk -a -t file -f $(IMAGE).img -o rem -m y:
	cp kernel/kernel.sys .
#	Now copy files to z: (we do it this way to avoid problems with cygwin and drive specifiers)
	cmd /c "copy kernel.sys y:KERNEL.SYS"
#	Unmount the floppy disk image
	imdisk -D -m y:
#	Copy the Test files.
	cmd /c "copytestfiles.bat"
# 	Launch Bocks #TO DO REMOVE.
	cmd /c "bochs -f bochsrc.bxrc -q -noconsole" 


$(IMAGE).iso: $(IMAGE).img
	rm -rf cdiso
	mkdir cdiso
	cp $(IMAGE).img cdiso/$(IMAGE).img
#	Make a bootable CD image from the floppy disk image
	mkisofs -o $(IMAGE).iso -b $(IMAGE).img cdiso/	

all: $(IMAGE).iso

clean:
	cd boot && make clean
	cd kernel && make clean
	rm -f kernel.sys
	rm -f $(IMAGE).img
	rm -f $(IMAGE).iso
	rm -rf cdiso
	
