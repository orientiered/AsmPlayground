.model tiny
.286

LOCALS @@

.code
org 100h

;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
VIDEO_SEG = 0b800h  ; text mode video memory segment
_WIDTH    = 80
_HEIGHT   = 25
F11_CODE  = 57h
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

START:

;====================================================================
    call main
    mov  ax, 3100h      ; DOS Fn 31h = terminate but stay resident
                        ; al - exit code, dx * 16 = reserved memory
    mov  dx, offset __EndOfInterrupts__
    add  dx, 16
    shr  dx, 4          ; dx = EOI / 16 rounded up
    int  21h            ; TSR exit
;====================================================================

;=========================================================================
; Custom keyboard (09h) interrupt
; Args  : none
; Return: none
; Destr : none
;=========================================================================
KB_INT proc
        push ax
        in   al, 60h      ; last keyboard scan-code
        cmp  al, F11_CODE
        pop  ax
        jne  @@OLD_HANDLER ; if pressed key != F11 continue

    ; Saving registers
        pusha
        ; push ax cx dx bx sp bp si di
        push ds
        push es


    ; ds and es should be equal to cs for correct behavior
        push cs
        pop  ds
        push cs
        pop  es

        mov  bp, sp
        add  bp, (10-1)*2 ; addr of first register
        call Draw


    ; Restoring registers
        pop es
        pop ds
        popa

    ; Calling old interrupt handler
    @@OLD_HANDLER:
        db 0eah              ; far jump to old interrupt handler
        OLD_KB_INT_OFS dw 0  ; addr
        OLD_KB_INT_SEG dw 0  ; segment
        ; iret
endp
;--------------------------------------------------------------------------
db DRAW_ACTIVE 0       ; current mode
;~~~~~~~~~~~~~~~~~~~~~~~~~
INCLUDE rframe.asm
;~~~~~~~~~~~~~~~~~~~~~~~~~

; This label MUST be after all Interrupts code
; All the code after this label will be freed after termination
__EndOfInterrupts__:

;=========================================================================
; Resident initialization: saving old interrupts addresses and moving new
;=========================================================================
main    proc
        xor ax, ax
        mov es, ax      ; es = 0
        mov bx, 09h * 4 ; bx = addr(int 09h)

        ; int 09h

    ; Replacing int 09h with new one
        mov ax, es:[bx]     ; old kb int offset
        mov OLD_KB_INT_OFS, ax
        mov ax, es:[bx+2]   ; old kb int segment
        mov OLD_KB_INT_SEG, ax

        cli                 ; disabling interrupts

        mov word ptr es:[bx], offset KB_INT    ; offset address
        push cs
        pop ax                          ; ax = cs
        mov es:[bx+2], ax               ; segment address = cs

        sti                 ; enabling interrupts

        ; int 09h

        ret

endp
;--------------------------------------------------------------------------





; End of compilation
end START
