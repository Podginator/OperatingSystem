; Test_A20_Enabled. Check the status of the A20 line.
;
; Returns: 0 in ax if the A20 line is disabled (memory wraps around)
;          1 in ax if the A20 line is enabled (memory does not wrap around)
 
Test_A20_Enabled:
    pushf
    push 	ds
    push 	es
    push 	di
    push 	si
 
    xor 	ax, ax 					; Set DS:SI to point to FFFF:0510 and ES:DI to point to 0000:0500.
    mov 	es, ax
    not 	ax 
    mov 	ds, ax

    mov 	di, 0500h
    mov 	si, 0510h
 
    mov 	al, byte [es:di]		; Save the current contents of 0000:0500		
    push 	ax
    mov 	al, byte [ds:si]		; Save the current contents of FFFF:0510
    push 	ax
 
    mov 	byte [es:di], 0			; Now write 00 to 0000:0500 and FF to FFFF:0510
    mov 	byte [ds:si], 0FFh
    cmp 	byte [es:di], 0FFh		; If 0000:0500 contains FF, then memory addressing is wrapping round which means that
									; the A20 line is NOT enabled.
 
    pop 	ax
    mov	 	byte [ds:si], al		; Restore the original values of the memory locations
    pop 	ax
    mov	    byte [es:di], al
 
    mov	    ax, 0
    je 		Test_A20_Exit			; Return 0 if the line is NOT enabled. 
 
    mov 	ax, 1					; otherwise return 1.
 
Test_A20_Exit:
    pop 	si
    pop	 	di
    pop	    es
    pop 	ds
    popf
    ret	
	
; The original method used to enable the A20 line involved using some 
; hardware IO using the Keyboard Controller chip (8042 chip).
;
; Before each command is sent to the chip (via I/O port 64h, we need to ensure
; that the keyboard input buffer is empty).
;
; If you want to know more about this, see http://wiki.osdev.org/%228042%22_PS/2_Controller

	
Enable_A20_Using_Kbd_Controller:
	call    A20Wait_Input					
	mov     al, 0ADh				; Disable keyboard
	out     64h, al

	call    A20Wait_Input
	mov     al, 0D0h				; Issue command to say we are going to read from input port
	out     64h ,al

	call    A20Wait_Output
	in      al, 60h					; Do the read and save the result
	push    eax

	call    A20Wait_Input
	mov     al,0D1h					; Issue command to say we are going to write to output port
	out     64h,al

	call    A20Wait_Input
	pop     eax						; Retrieve byte that we read
	or      al, 2					; Set bit to enable A20 line
	out     60h,al					; and output it

	call    A20Wait_Input			; Enable keyboard
	mov     al,0AEh
	out     64h,al

	call    A20Wait_Input
	ret

A20Wait_Input:						; Wait for input buffer to be empty (wait for bit 1 to be 0). This must be empty before any attempt
	in      al,64h					; is made to write to output ports 60h or 64h
	test    al,2
	jnz     A20Wait_Input
	ret

A20Wait_Output:						; Wait for output buffer to be full (must be set before attempting to read from input port 60h)
	in      al,64h
	test    al,1
	jz      A20Wait_Output
	ret	

; Enable the A20 line. We attempt this using various mechanisms in the following order:
;
;	1.  Attempt to use the BIOS function
;	2.	Attempt using the keyboard controller`
;	3.	Attempt using the Fast A20 gate
;

Enable_A20:
	call 	Test_A20_Enabled		; First see if it is already enabled.  If so, there is nothing to do.
	mov		dx, 1					; Indicate that A20 is already enabled
	cmp		ax, 1
	je	 	A20_Enabled
	
	mov 	ax, 2401h				; Try the BIOS function that enables the A20 line first (note that it is possible it is not supported)
	int 	15h						
	
	call 	Test_A20_Enabled		; Now see if A20 is enabled.  
	mov		dx, 2
	cmp		ax, 1
	je	 	A20_Enabled
	
	Call	Enable_A20_Using_Kbd_Controller	; See if we can enable A20 using the keyboard controller`
	call 	Test_A20_Enabled		; Now see if A20 is enabled.  
	mov		dx, 3
	cmp		ax, 1
	je	 	A20_Enabled
	
	in 		al, 92h					; Finally see if we can enable A20 using the Fast A20 gate (We try this method last since 
	test 	al, 2					; it could crash if it fails).
	jnz 	FastA20_Exit
	or 		al, 2
	and 	al, 0FEh
	out 	92h, al

FastA20_Exit:
	call 	Test_A20_Enabled		; And see if A20 is enabled
	mov		dx, 4
	cmp		ax, 1
	je	 	A20_Enabled
	mov		dx, 0
	
A20_Enabled:	
	ret
	