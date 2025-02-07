; MIPT 2025 ---- Morgachev Dmitri
; Draw centered frame with text in it
;---------------------------------------

.model tiny
.286

.data
    VIDEO_SEG equ 0b800h
    _WIDTH    equ 80
    _HEIGHT   equ 25

    DbgFrameStyle db '1234 6789'

    FrameStyle  db  201, 205, 187
                db  186, ' ', 186
                db  200, 205, 188

    STRING     db 'Mul is awful', 0

.code
org 100h
Start:  mov ax, VIDEO_SEG
        mov es, ax          ; es = VIDEO_SEG

        mov bx, 82h         ; first symbol in cmd args
        call AsciiToInt
        mov dl, al          ; dl = length(al)

        mov dh, 31h       ; 51h = text style
        mov bx, offset FrameStyle
        mov si, offset STRING
        mov cx, 11          ; height = 10
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

        mov  al, dl ; al = length
        mov  ah, cl ; ah = height
        call GetCenteredCorner ; di = offset

        push di    ; saving di

        mov cl, dl ; setting length
        mov ah, dh ; setting style byte
        call DrawLine

    ;MIDDLE LINES
        add bx, 3   ; shifting style symbols
        sub si, 2   ; height -= 2 (top and bottom lines)
    MIDDLE_LINES_LOOP:
        pop di      ; restoring di
        add di, _WIDTH * 2 ; y ++

        push di     ; saving di

        mov cl, dl  ; setting length
        call DrawLine

        dec si                  ; height --
        cmp si, 0               ; while (si != 0)
        jne MIDDLE_LINES_LOOP

    ;LAST LINE
        add bx, 3   ; shifting style symbols
        pop di      ; restoring di
        add di, _WIDTH * 2; di++

        mov cl, dl  ; setting length
        call DrawLine


    ;TEXT
        ; bx is now free to use
        pop bx     ; restoring si to bx draw text
        ; bx = msg

        call Strlen
        ; ax = strlen(msg)

        mov cx, ax ; cx = strlen(msg)
        mov ah, 1  ; height = 1
        call GetCenteredCorner ; di = offset

        mov ah, dh ; ah = style
        call DrawText


        ret
endp
;--------------------------------------------------------------------------------------


;======================================================================================
; Calculate left upper corner position for centered box
; Arg: al - length
;      ah - height
; Ret: di - offset
; Destr: ax, di
;======================================================================================
; x0 = (_WIDTH - length) / 2
; y0 = (_HEIGHT - height) / 2
; offset = 2 * ( y0 * _WIDTH + x0)
GetCenteredCorner proc
        mov di, ax  ; di = height * 100 h + length
        shr di, 8   ; di = height
        xor ah, ah  ; ax = length

        neg ax      ; ax = -length
        neg di      ; di = -height
        add ax, _WIDTH  ; ax = _WIDTH - length
        add di, _HEIGHT ; di = _HEIGHT - height

        shr ax, 1   ; ax = (_WIDTH - length) / 2 = x0
        shr di, 1   ; di = (_HEIGHT - height)/ 2 = y0

        xchg ax, di ; ax = y0, di = x0

    ; We can use
    ; shl ax, 4         ; ax = 16 ax
    ; lea ax, ax + ax*4 ; ax = 80 ax
    ; To do multiplication by _WIDTH = 80
        imul ax, _WIDTH

        add di, ax  ; di = x0 + y0 * _WIDTH
        shl di, 1   ; di = offset
        ret
endp
;--------------------------------------------------------------------------------------


;======================================================================================
; Draw text to video memory
; Args: es - video segment
;       ah - style
;       bx - msg addr
;       cx - length
;       di - offset
; Ret: None
; Destr: ax, bx, cx, di
;======================================================================================
DrawText    proc
    textDraw_loop:
        mov al, byte ptr [bx]
        inc bx
        stosw   ; es:[di] = ax, di += 2
        loop textDraw_loop

        ret
endp

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
;--------------------------------------------------------------------------------------


;======================================================================================
;  Strlen for '\0'-terminated strings
;  Arg: bx - string address
;  Ret: ax - length
;  Destr: ax
;======================================================================================
Strlen proc
        xor ax, ax          ; ax = 0
    STRLEN_LOOP:
        cmp byte ptr [bx], 0         ; if (mem[bx] == 0)
        je STRLEN_LOOP_END  ; return ax
                            ; else
        inc ax              ; ax ++
        inc bx              ; bx ++
        jmp STRLEN_LOOP     ; goto loop
    STRLEN_LOOP_END:
        sub bx, ax          ; bx -= strlen(msg)

        ret
endp
;--------------------------------------------------------------------------------------


;======================================================================================
; Convert string that ends with ' ' to int
; Doesn't support '-'
; Args: bx - string addr
; Ret:  ax - result of conversion
; Destr: ax
;======================================================================================
AsciiToInt  proc
        xor  ax, ax ; ax = 0
        push bx     ; storing bx
    atoi_loop:
        cmp byte ptr [bx], 0Dh ; if (mem[bx] == 0)
        je atoi_loop_end     ; ret ax

        imul ax, 10          ; ax *= 10
        add  ax, [bx]        ; ax = 10*ax + [bx]
        sub  ax, '0'         ; ax = 10*ax + ([bx] - char 0)

        inc  bx              ; bx ++
        jmp atoi_loop
    atoi_loop_end:
        pop  bx              ; restoring bx
        ret
endp
;--------------------------------------------------------------------------------------

; keep this line at the end of code
        end Start
