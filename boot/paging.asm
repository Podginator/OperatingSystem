;	Setup paging for bootloader

bits	32

; Address of page directory table
%define		PAGE_DIR			9C000h

;Address of  0th page table. Address must be 4KB aligned
%define		PAGE_TABLE_0		9D000h

; Address of 768th page table. Address must be 4KB aligned
%define		PAGE_TABLE_768		9E000h

; Each page table has 1024 entries
%define		PAGE_TABLE_ENTRIES	1024

; Attributes (page is present;page is writable; supervisor mode)
%define		PRIV				3

; Enable Paging

EnablePaging:
	pusha										; Save stack frame

	; Set page table to reference first 4MB of memory

	mov		eax, PAGE_TABLE_0					; Address of first page table
	mov		ebx, 0 | PRIV						; Starting physical address of page.  First one is 0, next is 4K, etc
	mov		ecx, PAGE_TABLE_ENTRIES				; for every page in table...

EnablePaging_Loop1:
	mov		dword [eax], ebx					; Write the entry
	add		eax, 4								; Go to next page entry in table (Each entry is 4 bytes)
	add		ebx, 4096							; Add 4K to the physical entry
	loop	EnablePaging_Loop1					; Go to next entry

	;	Map the 768th table to physical addr 1MB
	;	The 768th table starts the 3gb virtual address
 
	mov		eax, PAGE_TABLE_768				; First page table
	mov		ebx, 100000h | PRIV			; Starting physical address of page (where our kernel is physically loaded)
	mov		ecx, PAGE_TABLE_ENTRIES			; For every page in table...

EnablePaging_Loop2:
	mov		dword [eax], ebx				; Write the entry
	add		eax, 4							; Go to next page entry in table (Each entry is 4 bytes)
	add		ebx, 4096						; Next page points to next 4K of physical memory
	loop	EnablePaging_Loop2				; Go to next entry
	
	; Set up the entries in the directory table.  We are only interested in the 0th and 768th entry (i.e. the first
	; 4MB of address space and the space at 3GB)

	mov		eax, PAGE_TABLE_0 | PRIV			; 1st table is directory entry 0
	mov		dword [PAGE_DIR], eax

	mov		eax, PAGE_TABLE_768 | PRIV			; 768th entry in directory table
	mov		dword [PAGE_DIR + (768 * 4)], eax

	; Install directory table

	mov		eax, PAGE_DIR
	mov		cr3, eax

	; Enable paging

	mov		eax, cr0
	or		eax, 80000000h
	mov		cr0, eax

	popa
	ret

