; MIPT 2025 ---- Morgachev Dmitri
; Draw centered frame with text in it
; Input: 2 optional arguments
;       - width  >= 3
;       - height >= 4
;       Default: 25*11
;---------------------------------------

.model tiny
.286

.data
    VIDEO_SEG equ 0b800h
    _WIDTH    equ 80
    _HEIGHT   equ 25
    DFLT_STYLE = 11111101b

    DFLT_LENGTH = 25
    DFLT_HEIGHT = 11

    DbgFrameStyle db '1234 6789'

    FrameStyle:
    ; first style (double lines)
                db  201, 205, 187
                db  186, ' ', 186
                db  200, 205, 188
    ; second style (lines)
                db  218, 196, 191
                db  179, ' ', 179
                db  192, 196, 217
    ; third style (hearts)
                db  3, 3, 3
                db 3, ' ', 3
                db 3, 3, 3
    ; fourth style (arrows)
                db 4, 16, 4
                db 30, ' ', 31
                db 4, 17, 4

    STRING     db 'Lorem ipsum', 0

.code
org 100h
Start:  mov ax, VIDEO_SEG
        mov es, ax          ; es = VIDEO_SEG

        call ParseCmd       ; parsing cmd args
        ; result is stored according to DrawFrame argument list

        call DrawFrame

        mov ax, 4C00h       ; DOS Fn 4Ch = exit(al)
        int 21h             ; syscall exit



;======================================================================================
; Parse positional arguments in command line
; Args:
; Ret:   dl - first arg(length), dh - second arg(height)
;        ah - style byte
;     ds:si - frame style addr
;     ds:di - string addr to draw in frame
; Destr: ax, dx, si, di
;======================================================================================
ParseCmd    proc
        mov di, 81h         ; first symbol in cmd args
        mov cx, ds:[di-1]   ; storing length of cmd line
        add cx, 81h         ; ds:cx = end of cmd line

    ;First arg: length
        call AsciiToInt
        mov dl, al          ; dl = length    (al)
        cmp ax, 2           ; if (ax <= 2)
        ja  skip_default_length
        mov dl, DFLT_LENGTH ; setting default length
    skip_default_length:

    ;Second arg: height
        call AsciiToInt
        mov dh, al          ; dh = height
        cmp ax, 3           ; if (ax <= 3)
        ja skip_default_height
        mov dh, DFLT_HEIGHT ; setting default length
    skip_default_height:

    ; Third arg: style byte
        call AtoiHex
        mov bh, al         ; temporarily storing style byte in bh

    ; Fourth arg: frame style
        call AsciiToInt
        sub ax, 1

        mov si, ax  ;si = ax
        shl si, 3   ;si = 8*ax
        add si, ax  ;si = 9*ax
        add si, offset FrameStyle   ; si = Framestyle + 9*ax

    ;Setting other DrawFrame params
        mov ah, bh       ; ah = text style
        mov di, offset STRING
    parseCmd_end:

        ret
endp
;--------------------------------------------------------------------------------------


;======================================================================================
;   Draw centered frame with text in it
;   Args: es : videoSegment address
;      ds:si : style symbols array addr(char[9])
;      ds:di : null-terminated text addr
;         ah : style byte
;         dl : width
;         dh : height
;  Return: None
;  Destr: ax, bx, cx, si, di,
;======================================================================================
DrawFrame proc
        push di ; saving text addr
    ;FIRST LINE
        call GetCenteredCorner ; di = offset

        mov cl, dl ; setting length
        xor ch, ch ; zeroing high byte
        call DrawLine

    ;MIDDLE LINES
        sub dh, 2   ; height -= 2 (top and bottom lines)
    MIDDLE_LINES_LOOP:

        mov cx, _WIDTH
        sub cl, dl      ; cl = _WIDTH - length
        shl cx, 1       ; cl = 2*(_WIDTH - length)

        add di, cx      ; shifting di to next line

        mov cl, dl  ; setting length
        call DrawLine
        sub si, 3   ; correcting si

        dec dh                  ; height --
        cmp dh, 0               ; while (dh != 0)
        jne MIDDLE_LINES_LOOP

    ;LAST LINE

        add si, 3   ; correcting si again

        mov cx, _WIDTH
        sub cl, dl      ; cl = _WIDTH - length
        shl cx, 1       ; cl = 2*(_WIDTH - length)

        add di, cx      ; shifting di to next line

        mov cl, dl  ; setting length
        call DrawLine

    ;TEXT
        mov bh, ah ; saving style byte
        pop si     ; restoring text addr
        mov di, si ; strlen (ds:di (text addr) )
        call Strlen
        ; cx = strlen(msg)

        mov dl, cl ;
        mov dh, 1  ; height = 1
        call GetCenteredCorner ; di = offset

        mov cl, dl ; cl = strlen
        xor ch, ch ;
        mov ah, bh ; restoring style byte
        call DrawText


        ret
endp
;--------------------------------------------------------------------------------------


;======================================================================================
; Calculate left upper corner position for centered box
; Arg: dl - length
;      dh - height
; Ret: di - offset
; Destr: cx, di
;======================================================================================
; x0 = (_WIDTH - length) / 2
; y0 = (_HEIGHT - height) / 2
; offset = 2 * ( y0 * _WIDTH + x0)
GetCenteredCorner proc
        mov cx, dx  ; saving dx

        mov di, dx  ; di = height * 100 h + length
        shr di, 8   ; di = height
        xor dh, dh  ; dx = length

        neg dx      ; dx = -length
        neg di      ; di = -height
        add dx, _WIDTH  ; dx = _WIDTH - length
        add di, _HEIGHT ; di = _HEIGHT - height

        shr dx, 1   ; dx = (_WIDTH - length) / 2 = x0
        shr di, 1   ; di = (_HEIGHT - height)/ 2 = y0

        xchg dx, di ; dx = y0, di = x0

    ; We can use
    ; shl dx, 4         ; dx = 16 dx
    ; lea dx, dx + dx*4 ; dx = 80 dx
    ; To do multiplication by _WIDTH = 80
        imul dx, _WIDTH

        add di, dx  ; di = x0 + y0 * _WIDTH
        shl di, 1   ; di = offset

        mov dx, cx  ; restoring dx
        ret
endp
;--------------------------------------------------------------------------------------


;======================================================================================
; Draw text to video memory
; Args: es - video segment
;       ah - style
;       cx - length
;       di - offset
;       si - msg addr
; Ret: None
; Destr: ax, cx, di, si
;======================================================================================
DrawText    proc
    textDraw_loop:
        lodsb   ; al = ds:[si++]
        stosw   ; es:[di] = ax, di += 2
        loop textDraw_loop

        ret
endp

;--------------------------------------------------------------------------------------


;======================================================================================
;   Draws line using given style
;   Args: es : videoSegment address
;         ah : text style
;         cx : length
;         di : starting position (offset)
;         si : style symbols address (char[3])
;  Return: si = si + 3 (moves style symbols array)
;          di = di + length*2
;  Destr:  al, cx, di, si
;=======================================================================================
DrawLine proc

    ;Line start
        lodsb             ; al = ds:[si++]
        stosw             ; mov es:[di+=2], ax

    ;Preparing for loop
        sub cx, 2         ; cx -= 2 (corners)

    ;Main part of line
        lodsb             ; al = ds:[si++]
        rep stosw         ;loop mov es:[di+=2], ax

    ;Line end
        lodsb             ; al = ds:[si++]
        stosw             ; mov es:[di+=2], ax

        ret
endp
;--------------------------------------------------------------------------------------


;======================================================================================
;  Strlen for '\0'-terminated strings
;  Arg: ds:di - string address
;  Ret: cx - length
;  Destr: ax, cx, di
;======================================================================================
Strlen proc
    ;scasb uses es:di, to we need to copy ds to es
        push es
        push ds
        pop  es ; es = ds

    ;Preparing registers
        xor ax, ax          ; ax = 0
        xor cx, cx          ; cx = 0
        dec cx              ; cx = FFFF

        repne scasb         ; searching for 0

        neg cx              ; cx = len + 1
        dec cx              ; cx = len

    ;Restoring arguments
        pop es
        ret
endp
;--------------------------------------------------------------------------------------


;======================================================================================
; Convert string to number
; base 10
; Skips all spaces
; Stops when reaches first non-digit char
; Args: ds:di - string addr
; Ret:  ax - result of conversion
;       ds:di - first position after number
; Destr: ax, di
;======================================================================================
AsciiToInt  proc
        xor  ax, ax ; ax = 0
        push cx     ; storing cx

        call SkipSpaces
        xor cx, cx  ; cx = 0

        xchg di, si ; lodsb works with ds:si

    atoi_loop:
        lodsb                   ; al = ds:si++
        sub al, '0'             ; al = mem[si] - char 0

        cmp al, 9             ; if ( cx - 9 > 0 ) <=> !isdigit(mem[bx])
        ja  atoi_loop_end     ; ret ax, bx

        imul cx, 10          ; cx *= 10
        add  cx, ax          ; ax += mem[bx] - char 0

        jmp atoi_loop
    atoi_loop_end:

        xchg di, si
        mov ax, cx  ; result is in ax
        pop cx      ; restoring cx
        ret
endp
;--------------------------------------------------------------------------------------

;======================================================================================
; Convert string to number
; base 16
; Skips all spaces
; Stops when reaches first non-digit and non base-16 letter char
; Args: ds:di - string addr
; Ret:  ax - result of conversion
;       ds:di - first position after number
; Destr: ax, di, si
;======================================================================================
AtoiHex  proc
        xor  ax, ax ; ax = 0
        push cx     ; storing cx

        call SkipSpaces
        xor cx, cx  ; cx = 0

        xchg di, si ; lodsb works with ds:si

    atoi_hex_loop:
        lodsb                   ; al = ds:si++

    ; checking whether char is digit
        sub al, '0'             ; al = mem[si] - char 0

        cmp al, 9             ; if ( cx - 9 <= 0 ) <=> isdigit(mem[si])
        jbe  atoi_hex_calc     ; ret ax,

    ; else trying letters
        sub al, 'a' - '0'     ; shifting to letters
        cmp al, 5             ; F-10 = 5
        ja atoi_hex_loop_end
        add ax, 10            ; compensating subtraction of 'a'

    atoi_hex_calc:
        imul cx, 10h         ; cx *= 16
        add  cx, ax          ; cx += digit

        jmp atoi_loop
    atoi_hex_loop_end:

        xchg di, si
        mov ax, cx  ; result is in ax
        pop cx      ; restoring cx
        ret
endp
;--------------------------------------------------------------------------------------


;======================================================================================
; Skips space characters in the string (currently only ' ')
; Args: ds:di - string addr
; Ret:  di - moved addr to first non-space char
; Destr: ax, cx, di
;======================================================================================
SkipSpaces      proc
    ;scasb uses es:di, to we need to copy ds to es
        ; push cx ; saving cx because it is destor
        push es
        push ds
        pop  es ; es = ds

        mov al, ' ' ; search symbol for scasb

        xor cx, cx  ; cx = 0
        dec cx      ; cx = FFFFh
        repe scasb  ; skipping while es:[di] == ' '

        dec di      ; scasb will increase di even if es:[di] == al, so dec is needed

        pop es ; restoring es
        ret
endp
;--------------------------------------------------------------------------------------

; keep this line at the end of code
        end Start
