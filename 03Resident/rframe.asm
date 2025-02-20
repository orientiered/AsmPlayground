
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

    FRAME_WIDTH = 16
    FRAME_HEIGHT = 10
    FRAME_X = 80 - FRAME_WIDTH
    FRAME_Y = 0
    FRAME_POS = (FRAME_Y * _WIDTH + FRAME_X) * 2


;======================================================================================
;
;======================================================================================
DrawRegisters proc
        mov ax, VIDEO_SEG
        mov es, ax
        cld

        mov si, offset FrameStyle
        xor cx, cx
        xor bx, bx
        mov ah, STYLE_ATTR
        mov dl, FRAME_WIDTH
        mov dh, FRAME_HEIGHT
        mov di, FRAME_POS

        call DrawFrameBorders

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
; DrawText    proc
;     test cx, cx   ;cmp cx, 0
;     je DrawText_End
;
;     textDraw_loop:
;         lodsb   ; al = ds:[si++]
;         stosw   ; es:[di] = ax, di += 2
;         loop textDraw_loop
;
;         ret
;
;     DrawText_End:
; endp

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

