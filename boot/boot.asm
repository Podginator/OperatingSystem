; Real-Mode Part of the Boot Loader
;
; When the PC starts, the processor is essentially emulating an 8086 processor, i.e. 
; a 16-bit processor.  So our initial boot loader code is 16-bit code that will 
; eventually switch the processor into 32-bit mode.

BITS 16

; Tell the assembler that we will be loaded at 7C00 (That's where the BIOS loads boot loader code).
ORG 7C00h
start:
	jmp 	Real_Mode_Start				; Jump past our sub-routines]

	times 11-$+start db 0				; Pad out so that there are 11 bytes before the BIOS Parameter Block.
	
%include "bpb.asm"	
%include "functions_16.asm"

;	Start of the actual boot loader code
	
Real_Mode_Start:
	cli
    xor 	ax, ax						; Set stack segment (SS) to 0 and set stack size to top of segment
    mov 	ss, ax
    mov 	sp, 0FFFFh

    mov 	ds, ax						; Set data segment registers (DS and ES) to 0.
	mov		es, ax						
	
	mov		[boot_device], dl			; Boot device number is passed in DL
	
	mov 	si, boot_message			; Display our greeting
	call 	Console_WriteLine_16

Reset_Floppy_Drive:
	mov		ah, 0						; Reset floppy disk function
	mov		dl, [boot_device]						
	int		13h						
	jc		Reset_Floppy_Drive			; If carry flag is set, there was an error. Try resetting again	
	
	mov		ah, 2						; BIOS Read sector functions_16
	mov		al, 5						; Number of sectors to read = 5
	mov		bx, 9000h					; The five segments will be loaded into memory at ES:BX (0000:9000h)
	mov		ch, 0						; Use cylinder 0
	mov 	dh, 0 						; Use head 0
	mov		dl, [boot_device] 			
	mov 	cl, 2						; Start reading at sector 2 (i.e. one after the boot sector)
	int 	13h
	cmp		al, 5						; AL returns the number of sectors read.  If this is not 5, report an error
	jne		Read_Failed
	    
	mov		dl, [boot_device]			; Pass boot device to second stage
    jmp 	9000h						; Run stage 2
	
Read_Failed:	
	mov 	si, read_failed_msg
	call 	Console_WriteLine_16

Quit_Boot:
	mov 	si, cannot_continue			; Display 'Cannot continue' message
	call 	Console_WriteLine_16
	
	hlt						
	
; Data

boot_device			db  0
boot_message:		db	'UODos V1.0', 0
read_failed_msg:	db	'Unable to read stage 2 of the boot process', 0
cannot_continue:  	db	'Cannot continue boot process', 0

; Pad out the boot loader so that it will be exactly 512 bytes
	times 510 - ($ - $$) db 0
	
; The segment must end with AA55h to indicate that it is a boot sector
	dw 0AA55h
	