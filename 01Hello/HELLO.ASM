.model tiny

.data
EOL 	equ 0dh, 0ah   			; \r\n macro
MSG_STR db 'Hello world', EOL, '$'	; Message to output

.code			; PSP shift
org 100h

START:
	mov ah, 09h		; DOS Func 09h = puts(dx)
	mov dx, offset MSG_STR  ; dx = &MSG_STR
	int 21h			; syscall puts(MSG_STR)

	mov ax, 4c00h		; DOS Func 4ch = exit(al)
	int 21h			; syscall exit(0)

end START

First DOS Asm program
