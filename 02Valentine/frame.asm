; Draw frame with text in it
;---------------------------------------

.model tiny


.data
    VIDEO_SEG equ 0b800h
    _WIDTH    equ 80
    _HEIGHT   equ 25

    FrameStyle db '1234 6789'
    STRING     db 'Amogus'

.code
org 100h
Start:  mov ax, VIDEO_SEG
        mov es, ax          ; es = VIDEO_SEG

        mov dx, 3130h         ; 51h = text style, 30h = lengths
        mov bx, offset FrameStyle
        mov cx, 10          ; height = 10
        call DrawFrame

        mov ax, 4C00h       ; DOS Fn 4Ch = exit(al)
        int 21h             ; syscall exit


;======================================================================================
;   Draw centered frame with text in it
;   Args: es : videoSegment address
;         bx : style symbols array addr(char[9])
;         si : null-terminated text addr
;         cx : height
;         dl : width
;         dh : text style
;  Return: None
;  Destr: ax, bx, cx, si, di,
;======================================================================================
DrawFrame proc

        push si     ; storing si as we don't need it now

    ;FIRST LINE
        mov  si,  cx; si  = cx (height)
        ; x_0 = (_WIDTH - dx) / 2 * 2
        ; y_0 = (_HEIGHT - height) / 2
        ; offset = (y_0  + y) * _WIDTH + x_0 = ((_HEIGHT - height) / 2 + y + 1) * _WIDTH - dx
        mov  di, _WIDTH * 2
        push di    ; saving di

        mov cl, dl
        mov ah, dh
        call DrawLine

    ;MIDDLE LINES
        add bx, 3   ; shifting style symbols
        sub si, 2   ; height -= 2 (top and bottom lines)
    MIDDLE_LINES_LOOP:
        pop di      ; restoring di
        add di, _WIDTH * 2 ; y ++

        push di     ; saving di

        mov cl, dl
        call DrawLine

        dec si                  ; height --
        cmp si, 0               ; while (si != 0)
        jne MIDDLE_LINES_LOOP

    ;LAST LINE
        add bx, 3   ; shifting style symbols
        pop di      ; restoring di
        add di, _WIDTH * 2; di++

        mov cl, dl
        call DrawLine


        ; TEXT
        pop si     ; restoring si to draw text

        ret
endp
;--------------------------------------------------------------------------------------


;======================================================================================
; Calculate left upper corner position for centered box
; Arg:

;======================================================================================

;--------------------------------------------------------------------------------------



;======================================================================================
;   Draws line using given style
;   Args: es : videoSegment address
;         ah : text style
;         di : starting position (offset)
;         bx : style symbols address (char[3])
;         cx : length
;  Return: None
;  Destr:  al, di, cx
;=======================================================================================
DrawLine proc

    ;Line start
        mov al, [bx]      ; al = symbols[0]
        mov es:[di], ax   ; vmem[di] = ax
        add di, 2         ; di += 2

    ;Preparing for loop
        sub cx, 2         ; cx -= 2 (corners)

    ;Main part of line
        mov al, [bx + 1]    ; al = symbols[1]
    LINE_DRAW_LOOP:
        mov es:[di], ax   ; vmem[di] = ax
        add di, 2         ; di += 2
        loop LINE_DRAW_LOOP

    ;Line end
        mov al, [bx + 2]    ; al = symbols[2]
        mov es:[di], ax     ; vmem[di] = ax

        ret
endp
;======================================================================================




; keep this line at the end of code
        end Start
