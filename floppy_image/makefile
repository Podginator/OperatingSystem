# Create a blank 1.44M bootable floppy image file.
# This will only run under Linux (or the Bash shell on Windows 10)
# It will not run under Cygwin.

.SUFFIXES: .img 

uodos.img: 
	mkfs.vfat -R 6 -C uodos.img 1440

all: uodos.img

clean:
	rm -f uodos.img
