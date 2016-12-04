
; 32-bit Console Output Routines

BITS 32

%define	VIDEO_MEMORY	0B8000h			; Base of video memory
%define	COLS			80				; Width of screen
%define ROWS			25				; Height of screen

cursorX db 0							; Current x/y location
cursorY db 0

screenAttribute 	db 0Fh				; The default background attribute (white on black)

;  Write a character to the current position on the screen
;
;  On input: BL = Character to display
;			 BH = Attribute to use

Console_WriteChar_32:
	pusha								; Save registers
	cmp		bl, 0Dh						; Is it a carriage return?
	je		BeginningOfLine				; Yes, just reset to beginning of line
	cmp		bl, 0x0A					; Is it a newline character?
	je		NextRow						; Go to next row
	
	mov		edi, VIDEO_MEMORY			; Get pointer to video memory

	mov		ecx, COLS * 2				; Each character occupies 2 bytes, so it is 2 * COLS bytes per line
	movzx	eax, byte [cursorY]			; Get current y position
	mul		ecx							; Multiply y * COLS * 2 to get to start of current line

	movzx	ecx, byte [cursorX]			; Multiply x position by 2 because it is 2 bytes per char
	add		ecx, ecx
	add		eax, ecx					; And add together to get current position in video memory


	mov		word [edi + eax], bx		; Write character and attribute to video display

	; Update next position      

	inc		byte [cursorX]				; Go to next character
	cmp		byte [cursorX], COLS		; Are we at the end of the line?
	je		NextRow						; Yes, go to beginning of next row
	jmp		Done			

NextRow:
	cmp		byte [cursorY], ROWS - 1	; Check to see if we are on the bottom line of the screen
	jne 	NoScroll					; If not, we can just move to the next row
	
	; Scroll the screen up one line
	mov		esi, edi					; EDI already points at the base of video memory
	add		esi, COLS * 2				; Set ESI to point to the beginning of the second line on the screen
	mov		ecx, (ROWS - 1) * COLS		; How many characters to move
	cld
	rep		movsw						; Scroll up
	
	mov		edi, VIDEO_MEMORY			; Now, clear the bottom line
	add		edi, (ROWS - 1) * COLS * 2	; Set EDI to point to beginning of last line
	mov		ecx, COLS
	mov		ah, byte [screenAttribute]
	mov		al, ' '	
	rep		stosw
	jmp		BeginningOfLine
	
NoScroll:	
	inc		byte [cursorY]					; Go to next row

BeginningOfLine:	
	mov		byte [cursorX], 0				; Go back to col 0

Done:
	mov		bh, byte [cursorY]			; Get current position
	mov		bl, byte [cursorX]
	call	MoveCursor					; Update cursor position
	popa								; restore registers and return
	ret

;	Console_Write_32
; - Prints a null terminated string
;
;	On input: ESI = Address of string to print
;             BH  = Attribute to use

Console_Write_32:
	pusha								; Save registers

StringLoop:
	mov		bl, byte [esi]				; Get next character
	cmp		bl, 0						; Is it 0 (Null terminator)?
	je		StringDone			

	call	Console_WriteChar_32

	inc		esi							; go to next character
	jmp		StringLoop

StringDone:
	popa								; restore registers, and return
	ret

; 	Console_ClearScreen_32	
;
;	Clear screen to spaces and set cursor position to top left hand corner
;
;	On input:  BH = Attribute for each space on the screen

Console_ClearScreen_32:
	pusha
	mov		byte [screenAttribute], bh		; Save the default attribute
	cld										; Fill 2000 characters with a space and the appropriate attribute
	mov		edi, VIDEO_MEMORY
	mov		ecx, ROWS * COLS
	mov		ah, bh
	mov		al, ' '	
	rep		stosw

	mov		byte [cursorX], 0
	mov		byte [cursorY], 0
	xor		bx, bx
	call	MoveCursor						; Update cursor position
	popa
	ret
	

;	MoveCursor.  Update hardware cursor position
;  
;	On input:  BH = Y Position
;			   BL = X position

MoveCursor:
	pusha								; Save registers 

	mov		ecx, COLS					; Multiply Y position by COLS
	movzx	eax, bh				
	mul		ecx	
	movzx	ecx, bl						; Now add X position
	add		eax, ecx			
	mov		ebx, eax

	; Output low byte to VGA register 

	mov		al, 0x0f
	mov		dx, 0x03D4
	out		dx, al

	mov		al, bl
	mov		dx, 0x03D5
	out		dx, al			

	; Output high byte to VGA register 

	xor		eax, eax
	mov		al, 0x0e
	mov		dx, 0x03D4
	out		dx, al

	mov		al, bh
	mov		dx, 0x03D5
	out		dx, al			

	popa
	ret

