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
CTRL_CODE = 1dh
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
CTRL_PRESSED db 0
F11_PRESSED  db 0
DRAW_ACTIVE  db 0             ; current mode
KB_INT proc
        push ax
        in   al, 60h      ; last keyboard scan-code

        mov  ah, al
        and  ah, 80h      ; press/release bit
        shr  ah, 7        ; moving to low bit
        dec  ah           ; ah = 0 if released, FF if pressed

        and  al, not 80h  ; raw scan-code


    ; Checking if ctrl was pressed
        cmp  al, CTRL_CODE
        jne  @@CHECK_F11_PRESS
        mov  byte ptr cs:CTRL_PRESSED, ah

        jmp  @@CHECK_KEYBIND

    @@CHECK_F11_PRESS:
        cmp  al, F11_CODE
        jne  @@OLD_HANDLER ; if pressed key != F11 continue

        mov byte ptr cs:F11_PRESSED,  ah

    @@CHECK_KEYBIND:
    ; CTRL_PRESSED and F11_PRESSED
        mov  al, cs:CTRL_PRESSED
        test al, cs:F11_PRESSED
        jz   @@OLD_HANDLER

        ; mov  cs:CTRL_PRESSED, 0
        ; mov  cs:F11_PRESSED,  0

    ; Inverting DRAW_ACTIVE on Ctrl+F11 press
        not byte ptr cs:DRAW_ACTIVE
        push cx si di es ds
        call SaveRestoreBackground
        pop  ds es di si cx

    ; Calling old interrupt handler
    @@OLD_HANDLER:
        pop ax               ; restoring ax

        db 0eah              ; far jump to old interrupt handler
        OLD_KB_INT_OFS dw 0  ; addr
        OLD_KB_INT_SEG dw 0  ; segment
        ; iret
endp
;--------------------------------------------------------------------------

;=========================================================================
; Custom timer (08h) interrupt
; Args  : none
; Return: none
; Destr : none
;=========================================================================
TM_INT  proc
    ; Skipping interrupt if it is deactivated
        cmp cs:DRAW_ACTIVE, 0
        je @@OLD_HANDLER

        ; stack: flags cs ip
    ; Saving registers
        pusha
        ; push ax cx dx bx sp bp si di
        push ss
        push ds
        push es


    ; ds and es should be equal to cs for correct behavior
        push cs
        pop  ds
        push cs
        pop  es

        mov  bp, sp
        add  bp, (11-1)*2 ; addr of first register
        call Draw


    ; Restoring registers
        pop es
        pop ds
        pop ss
        popa

        ; Calling old interrupt handler
    @@OLD_HANDLER:
        db 0eah              ; far jump to old interrupt handler
        OLD_TM_INT_OFS dw 0  ; addr
        OLD_TM_INT_SEG dw 0  ; segment
        ; iret
endp
;--------------------------------------------------------------------------

;~~~~~~~~~~~~~~~~~~~~~~~~~
INCLUDE rframe.asm
;~~~~~~~~~~~~~~~~~~~~~~~~~

;=========================================================================
; Save background of the frame if DRAW_ACTIVE == 1
; Restore background to the screen otherwise
; Args:
; Ret: none
; Destr: ax, cx, si, di, es, ds
;=========================================================================
BKG_BUFFER dw FRAME_HEIGHT*FRAME_WIDTH dup (0)
SaveRestoreBackground proc
    ; Default code copies data from BKG_BUFFER to screen (DRAW_ACTIVE = 0)
    ; When DRAW_ACTIVE = 0 we swap registers in commands
        cmp cs:DRAW_ACTIVE, 0
        jne @@SCREEN_TO_BUFFER
        mov  word ptr cs:@@SEGM_INIT+1, 1FC0h ; mov es, pop ds
        mov  word ptr cs:@@REGISTER_INIT, 9090h  ; nop nop
        mov  byte ptr cs:@@ADD_CMD+1, 0C7h ; add di

        jmp @@COPY_STAGE
    @@SCREEN_TO_BUFFER:
        mov  word ptr cs:@@SEGM_INIT+1, 07D8h ; mov ds, pop es
        mov  word ptr cs:@@REGISTER_INIT, 0FE87h ; xchg di, si
        mov  byte ptr cs:@@ADD_CMD+1, 0C6h ; add si


    ; 03EB				 @@SEGM_INIT:
    ; 03EB  8E C0			 mov es, ax	   ; es	= VIDEO_SEG
    ; 03ED  8E D8			 mov ds, ax
    ; 03EF  1F			     pop  ds	   ; ds	= cs
    ; 03F0  07			     pop  es

    ; 0405  90			     nop
    ; 0406  87 FE			 xchg di, si

    ; 041C  81 C7 008A		 add di, (_WIDTH - FRAME_WIDTH)*2
    ; 0420  81 C6 008A		 add si, (_WIDTH - FRAME_WIDTH)*2
    @@COPY_STAGE:

        mov  ax, VIDEO_SEG
        push cs
    @@SEGM_INIT:
        mov  es, ax    ; es = VIDEO_SEG
        pop  ds       ; ds = cs


        mov  cx, FRAME_HEIGHT
        mov  di, FRAME_POS
        mov  si, offset BKG_BUFFER

    @@REGISTER_INIT:
        nop
        nop

    @@row_loop:
        mov ax, cx          ; saving cx

        mov cx, FRAME_WIDTH
        rep movsw     ; mov es:[di+=2], ds:[si+=2]
    @@ADD_CMD:
        add di, (_WIDTH - FRAME_WIDTH)*2    ;correcting pointer in video memory

        mov cx, ax          ; outer loop counter
        loop @@row_loop

        ret
endp
;--------------------------------------------------------------------------


; This label MUST be after all Interrupts code
; All the code after this label will be freed after termination
__EndOfInterrupts__:

;=========================================================================
; Resident initialization: saving old interrupts addresses and moving new
;=========================================================================
main    proc
        xor ax, ax
        mov es, ax      ; es = 0


        mov bx, 08h * 4 ; bx = addr(int 08h)

    ; int 08h and int 09h
    ; KB = keyboard, TM = timer
    ; es:  bx    bx+2    bx+4    bx+6
    ;    TM_OFS TM_SEG  KB_OFS  TM_OFS
    ; Replacing ints with new ones
        cli                 ; disabling interrupts
        mov ax, es:[bx]     ; old tm int offset
        mov OLD_TM_INT_OFS, ax
        mov ax, es:[bx+2]   ; old tm int segment
        mov OLD_TM_INT_SEG, ax

        mov ax, es:[bx+4]   ; old kb int offset
        mov OLD_KB_INT_OFS, ax
        mov ax, es:[bx+6]   ; old kb int segment
        mov OLD_KB_INT_SEG, ax


        mov word ptr es:[bx],     offset TM_INT    ; TM offset address
        mov word ptr es:[bx + 4], offset KB_INT    ; KB offset address

        push cs
        pop ax                          ; ax = cs
        mov word ptr es:[bx + 2], ax             ; TM segment address = cs
        mov word ptr es:[bx + 6], ax             ; KB segment address = cs

        sti                 ; enabling interrupts

        ; int 09h

        ret

endp
;--------------------------------------------------------------------------





; End of compilation
end START
