    org 07c00h

    mov ax, cs
    mov ds, ax
    mov es, ax
    call disp_str
    jmp $

disp_str:
    mov ax, boot_msg
    mov bp, ax
    mov cx, 15
    mov ax, 01301h
    mov bx, 000ch
    mov dl, 0
    int 10h
    ret

boot_msg:
    db "hello, os world"

    times 510 - ($ - $$) db 0
    dw 0xaa55
