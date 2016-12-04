; Global Descriptor Table (GDT)
gdt_start:

; The first entry in the GDT is a mandatory 'null' descriptor

gdt_null:							
	dd	0
	dd	0

; The code segment descriptor
;
; Base = 0, limit = FFFFF
; First flags byte:
;   Bit 0 (Bit 40 in GDT): Access bit (Used with Virtual Memory). Because we are not using virtual memory yet we will set it to 0
;   Bit 1 (Bit 41 in GDT): The readable/writable bit. Its set (for code selector), so we can read and execute data in the segment as code
;   Bit 2 (Bit 42 in GDT): The conforming bit. For now, set to 0 to indicate that only code of the privilege set in bits 45 and 46 can execute.
;   Bit 3 (Bit 43 in GDT): Set to 1 to indicate we have a code segment 
;   Bit 4 (Bit 44 in GDT): Represents this as a "system" or "code/data" descriptor. This is a code selector, so the bit is set to 1.
;   Bits 5-6 (Bits 45-46 in GDT): The privilege level. We are in ring 0, so both bits are 0.
;   Bit 7 (Bit 47 in GDT): Used to indicate the segment is in memory (Used with virtual memory). Set to zero for now, since we are not using virtual memory yet
;
; Second flags nibble: (Granularity)1 (32-bit default)1 (64-bit segment)0 (AVL)0 => 1100b

gdt_code:
	dw	0FFFFh					; Limit (bits 0 - 15)
	dw  0 						; Base (bits 0 - 15)
	db 	0 						; Base (bits 16 - 23)
	db  10011010b				; First flags and type flags
	db	11001111b				; Second flags and bits 16 - 19 of limit
	db 	0 						; Base (bits 24 - 31)
	
; The data segment descriptor
; Base = 0, limit = FFFFF
; First flags nibble:  (Present)1 (Privilege)00 (Descriptor type)1 				 => 1001b
; Type flags nibble: (Code)0 (Expand down)0 (Writable)1 (Accessed)0 			 => 0010b
; Second flags nibble: (Granularity)1 (32-bit default)1 (64-bit segment)0 (AVL)0 => 1100b

gdt_data:
	dw	0FFFFh					; Limit (bits 0 - 15)
	dw  0 						; Base (bits 0 - 15)
	db 	0 						; Base (bits 16 - 23)
	db  10010010b				; First flags and type flags
	db	11001111b				; Second flags and bits 16 - 19 of limit
	db 	0 						; Base (bits 24 - 31)
	
gdt_end:						; This label lets us use the assembler to calculate the size of the GDT below,

; GDT Descriptor

GDT_Descriptor:
	dw	gdt_end - gdt_start - 1 ; Size of the GDT.  It is always one less than the true size.
	dd 	gdt_start				; Start address of the GDT
	
; Useful offsets into the GDT

CODE_SEG	equ	gdt_code - gdt_start
DATA_SEG	equ	gdt_data - gdt_start

