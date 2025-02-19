.model tiny
.286

LOCALS @@

.code
org 100h

;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
VIDEO_SEG = 0b800h  ; text mode video memory segment
_WIDTH    = 80
_HEIGHT   = 25
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

START:

;====================================================================
    call main
    mov  ax, 3100h      ; DOS Fn 31h = terminate but stay resident
                        ; al - exit code, dx * 16 = reserved memory
    mov  dx, offset __EndOfInterrupts__
    add  dx, 15
    shr  dx, 4          ; dx = EOI / 16 rounded up
    int  21h            ; TSR exit
;====================================================================


;=========================================================================
; Custom keyboard (09h) interrupt
; Args  : none
; Return: none
; Destr : none
;=========================================================================
Int09NEW proc
        push ax bx es

        mov ax, VIDEO_SEG
        mov es, ax      ; es = VIDEO_SEG

        mov ah, 15h     ; style
        mov bx, (_WIDTH * 10 + _WIDTH / 2) * 2 ; pos

        in al, 60h      ; last keyboard scan-code
        mov es:[bx], ax ; drawing this scan-code

    ; Telling kb that interrupt was handled
        in al, 61h      ; Reading state of PPI port B
        mov ah, al      ;
        or al, 80h      ; 80h = 0b1000 0000 : activating high bit
        out 61h, al     ; Disabling keyboard    ->
        mov al, ah      ;                           kb signal is handled
        out 61h, al     ; Enabling keyboard     ->

    ; Signal for interrupt controller
        mov al, 20h     ; EOI
        out 20h, al     ; 20h - Interrupt controller

        pop es bx ax
        iret
endp
;--------------------------------------------------------------------------


; This label MUST be after all Interrupts code
; All the code after this label will be freed after termination
__EndOfInterrupts__:

main    proc
        xor ax, ax
        mov es, ax      ; es = 0
        mov bx, 09h * 4 ; bx = addr(int 09h)

        int 09h

    ; Replacing int 09h with new one
        cli

        mov word ptr es:[bx], offset Int09NEW    ; offset address
        push cs
        pop ax                          ; ax = cs
        mov es:[bx+2], ax               ; segment address = cs

        sti

        int 09h

        ret

endp


; End of compilation
end START
