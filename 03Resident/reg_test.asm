.model tiny
.286
.code
org 100h

F12_CODE = 58h

START:
    NEXT:
        mov bx, 0B1B2h
        mov cx, 0C1C2h
        mov ax, 0A1A2h
        mov dx, 0D1D2h
        mov si,  1112h
        mov di, 0E1E2h

        in   al, 60h      ; last keyboard scan-code
        cmp  al, F12_CODE
        jne NEXT

        mov ax, 4c00h ; exit
        int 21h

end START
