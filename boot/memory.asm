; Basic memory routines

bits 16

;  Memory map entry structure

struc	MemoryMapEntry
	.baseAddress		resq	1
	.length				resq	1
	.type				resd	1
endstruc

;  Get memory map from BIOS
;
;  Note that even though this is 16-bit code, we are using the extended registers EAX, EBX, ECX and EDX

;
;  On entry:  es:di = Destination buffer for entries
;  On exit:	  bp    = Entry count
;			  Carry flag is set if an error occured

BIOSGetMemoryMap:
	pushad
	xor		ebx, ebx
	xor		bp, bp									; Clear number of entries
	mov		edx, 'PAMS'								; Set EDX to 'SMAP'
	mov		eax, 0e820h
	mov		ecx, 20									; Memory map entry struct is 24 bytes
	int		15h										; Get first entry
	jc		GetMemoryMap_Error	
	cmp		eax, 'PAMS'								; BIOS returns SMAP in eax. Quit if not
	jne		GetMemoryMap_Error
	test	ebx, ebx								; If ebx=0 then list is one entry long. That's an error
	je		GetMemoryMap_Error
	jmp		GetMemoryMap_Start

GetMemoryMap_NextEntry:
	mov		edx, 'PAMS'								; Some bios's trash this register, so we need to reload it
	mov		ecx, 20									
	mov		eax, 0e820h
	int		0x15									; Get next entry

GetMemoryMap_Start:
	jcxz	GetMemoryMap_SkipEntry					; If actual returned bytes is 0, skip entry
	mov		ecx, [es:di + MemoryMapEntry.length]	; Get length (low dword)
	or		ecx, [es:di + MemoryMapEntry.length + 4]; OR with upper dword to test for 0
	jz		GetMemoryMap_SkipEntry					; If length is 0 skip it

GetMemoryMap_GoodEntry:
	inc		bp										; increment entry count
	add		di, 20									; point di to next entry in buffer

GetMemoryMap_SkipEntry:
	cmp		ebx, 0									; If ebx return is 0, list is done
	jne		GetMemoryMap_NextEntry					; Get next entry
	clc
	jmp		GetMemoryMap_Done

GetMemoryMap_Error:
	stc

GetMemoryMap_Done:
	popad
	ret

