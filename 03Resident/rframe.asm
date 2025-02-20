
.code
;! WARNING:
;! THIS IS NOT .DATA, so your code may fall through
    FrameStyle:
    ; first style (double lines)
                db  201, 205, 187
                db  186, ' ', 186
                db  200, 205, 188
    STYLE_ATTR = 15h
    FRAME_TIME = 16000 ; in mcs

    FRAME_WIDTH = 11
    FRAME_HEIGHT = 8
    FRAME_X = 80 - FRAME_WIDTH
    FRAME_Y = 0
    FRAME_POS = (FRAME_Y * _WIDTH + FRAME_X) * 2


;======================================================================================
; Args: ss:bx - addr of first register
; Destr: all
;======================================================================================
Draw proc
        mov  ax, VIDEO_SEG
        mov  es, ax
        cld

        mov  si, offset FrameStyle
        xor  cx, cx
        mov  ah, STYLE_ATTR
        mov  dl, FRAME_WIDTH
        mov  dh, FRAME_HEIGHT
        mov  di, FRAME_POS

        call DrawFrameBorders

        call DrawRegisters
        ret
endp
;--------------------------------------------------------------------------------------


;======================================================================================
; Draws state of registers
; Args:  ss:bp - addr of first register in memory
;        order of registers: (ax cx dx bx sp bp si di)
;                        bx - 0  2  4  6  8  10 12 14
; Ret:
; Destr: ax, bx, cx, dx, di, si
;======================================================================================
text_to_draw db 0, 0, 0, 0
reg_count = 6
reg_order:
    db   0,  'ax '
    db   6,  'bx '
    db   2,  'cx '
    db   4,  'dx '
    db  12,  'si '
    db  14,  'di '

DrawRegisters proc

        mov  di, FRAME_POS + _WIDTH*2 + 4
        xor  bx, bx
    @@RegDrawLoop:
        mov  dx, bp

        mov  al, byte ptr offset reg_order[bx]
        xor  ah, ah                             ; ax = al
        sub  bp, ax
        mov  ax, word ptr ss:[bp]           ; ax = reg value, using stack segment

        mov  bp, dx
    ; Converting number to string
        mov  cx, 4
        mov  si, offset text_to_draw

        mov  dx, bx                      ; saving bx
        call HexToString
        mov  bx, dx                     ; restoring bx

    ; Printing '<reg_name> '
        mov  ah, STYLE_ATTR
        lea  si, reg_order[bx + 1]      ; string address
        mov  cx, 3                      ; strlen
        call DrawText                   ; register name

    ; Printing value of register
        mov  si, offset text_to_draw
        mov  cx, 4
        call DrawText                   ; register value

    ; Adjusting offset
        sub  di, 7*2                    ; shift from DrawText calls
        add  di, _WIDTH * 2             ; moving to next line

    ; Looping
        add  bx, 4                      ; 4 bytes for each line in table
        cmp  bx, reg_count * 4          ; if bx < reg_count
        jb   @@RegDrawLoop              ; continue

        ret
endp
;--------------------------------------------------------------------------------------


;======================================================================================
; Draw frame borders and fill background
; Args: ah - style attribute
;       es: video segment
;       di: start offset
;       dl: frame width
;    ds:si: style symbols array
;       dh: frame height
; Ret: cx = 0, dh = 0
; Destr: al, cx, dh, di
;======================================================================================
DrawFrameBorders        proc
        ;FIRST LINE

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
        call DrawLine ;

        sub si, 3   ; correcting si (because DrawLine adds 3 to si)

        dec dh                  ; height --
        ;cmp dh, 0               ; while (dh != 0)
        jnz MIDDLE_LINES_LOOP

    ;LAST LINE

        add si, 3   ; correcting si again

        mov cx, _WIDTH
        sub cl, dl      ; cl = _WIDTH - length
        shl cx, 1       ; cl = 2*(_WIDTH - length)

        add di, cx      ; shifting di to next line

        mov cl, dl      ; setting length
        call DrawLine

        sub si, 9       ; si is not destroyed
        ret
endp
;--------------------------------------------------------------------------------------


;======================================================================================
; Convert hex number to string
; Args: ax - number
;       cx - length of string
;    ds:si - string ptr
;
; Ret:   si - string ptr with converted number
; Destr: ax, bx, cx
;======================================================================================
HexToString proc
        add si, cx  ; digits are filled from the end
        dec si

    @@loop_begin:
        mov bx, ax
        and bx, 0Fh  ; one digit

        cmp bx, 9
        ja  @@CONV_LETTER        ; if (bx > 9)

        add bx, '0'              ; converting numerical digit to char
        jmp @@CHAR_MOVE

    @@CONV_LETTER:

        add bx, 'A' - 10

    @@CHAR_MOVE:
        mov byte ptr [si], bl
        shr ax, 4   ; ax /= 16
        dec si
        loop @@loop_begin

        inc si  ; correcting si

        ret
endp
;--------------------------------------------------------------------------------------

;======================================================================================
; Draws text in center of the screen
; Args: dl - text length
;    ds:si - text addr
;       ah - style attribute
; Ret: cx = 0
; Destr: al, cx, dx, si, di
;======================================================================================
; DrawCenteredText       proc
;         mov dh, 1  ; height = 1
;         call GetCenteredCorner ; di = offset
;
;         mov cl, dl ; cl = strlen
;         xor ch, ch ;
;         call DrawText
;
;         ret
; endp
;--------------------------------------------------------------------------------------



;======================================================================================
; Draw cx chars to video memory
; Args: es - video segment
;       ah - style
;       cx - length
;       di - offset
;       si - msg addr
; Ret: cx = 0
; Destr: al, cx, di, si
;======================================================================================
DrawText    proc
    test cx, cx   ;cmp cx, 0
    jz @@DrawText_End

    @@textDraw_loop:
        lodsb   ; al = ds:[si++]
        stosw   ; es:[di] = ax, di += 2
        loop @@textDraw_loop

        ret

    @@DrawText_End:
endp

;--------------------------------------------------------------------------------------


;======================================================================================
;   Draws line using given style
;   Args: es : videoSegment address
;         ah : text style
;         cx : length
;         di : starting position (offset)
;      ds:si : style symbols address (char[3])
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

