; Entry point to the C kernel.  This is prepended to the start of 
; the kernel by the linker and forms a 'known' entry point that we
; jump to from the boot assembler.

bits 32
extern _main
	push	eax				; Push the pointer to the bootinfo structure
	call	_main
	; We should never get back here
	cli
	hlt
	