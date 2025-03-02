; Exploits:
; FIRST issue:
;   1. Write any 20 symbols, then 4 and alt + 1
;   2. This will overwrite main return address to call into AccessGranted instruction
;
;
;

.model tiny
.286

locals @@

.code
org 100h

;~~~~~~
.data
BUFFER_LEN = 20
PASSWORD db 'memorabilia'
PWD_LEN   = $ - PASSWORD
PASSWORD_CORRECT   db 'Correct. Access granted', 0Dh, 0Ah, '$'
PASSWORD_INCORRECT db 'Wrong password. Try again', 0Dh, 0Ah, '$'
;~~~~~~
.code

Start:

    call Main
    mov ax, 4C00h       ;exit
    int 21h


;=========================================================================
;
;=========================================================================
Main proc
    sub  sp, BUFFER_LEN  ; allocating string buffer
    mov  di, sp          ; ss:di = buffer start position
    call GetLine

    xor  al, al
    mov  di, sp
    mov  cx, PWD_LEN
    mov  si, offset PASSWORD
    call CheckPassword

    call PrintOutput

    add sp, BUFFER_LEN  ; deallocating buffer
    ret
endp
;--------------------------------------------------------------------------

;=========================================================================
; Scans user input from console until CR
; Args:  ss:di - buffer start position
; Ret:   es = ss
; Destr: ax, di, es
;=========================================================================
GetLine proc
        mov  ax, ss
        mov  es, ax  ;es = ss

        mov  ah, 01h ; DOS Fn 01h = keyboard input -> al
    @@ScanLoop:
        int  21h      ; al = kybd input

        stosb         ; es:[di++] = al

        cmp  al, 0Dh  ; CR
        jne @@ScanLoop

        ret

endp
;--------------------------------------------------------------------------

;=========================================================================
; Prints result of password check
; Args: al - true if password is correct
; Ret: none
; Destr: ax, dx
;=========================================================================
PrintOutput proc
        test al, al ; cmp al, 0
        jz   @@incorrect_password
        call  AccessGranted
        ret
    @@incorrect_password:
        call  WrongPassword
        ret


endp
;--------------------------------------------------------------------------

;=========================================================================
; Executed on correct password
; Args: none
; Ret: none
; Destr: ax, dx
;=========================================================================
AccessGranted proc
        mov  dx, offset PASSWORD_CORRECT
        mov  ah, 09h
        int  21h

        ret
endp
;--------------------------------------------------------------------------

;=========================================================================
; Executed on incorrect password
; Args: none
; Ret: none
; Destr: ax, dx
;=========================================================================
WrongPassword proc
        mov  dx, offset PASSWORD_INCORRECT
        mov  ah, 09h
        int  21h

        ret
endp
;--------------------------------------------------------------------------

;=========================================================================
; Validates password
; Args: ds:si - string b
;       es:di - string a
;       cx - password length
;       expects al to be 0
; Ret:  al - non-zero value if password is correct
; Destr:
;=========================================================================
CheckPassword proc
        repe cmpsb      ; comparing strings
        test cx, cx     ; if cx == 0
        jnz @@notequal
        inc al          ; al = 1 ( strings are equal)
    @@notequal:


        ret
endp
;--------------------------------------------------------------------------

end Start

