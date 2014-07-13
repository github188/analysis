%include "descriptor.asm"


org 07c00h
    jmp LABEL_BEGIN


[SECTION .gdt]
; GDT {{
; 空描述符
label_gdt_null: DESCRIPTOR 0, 0, 0
; 非一致代码段
label_desc_code32: DESCRIPTOR 0, SEG_CODE32_LEN - 1, DA_C + DA_32
; 显存数据段
label_desc_video: DESCRIPTOR 0B8000h, 0FFFFh, DA_DRW
; }} GDT

GDT_LEN     equ     $ - label_gdt_null ; GDT长度
gdt_ptr     dw      GDT_LEN - 1 ; GDT界限
            dd      0           ; GDT基址
; end of section .gdt


; GDT选择子
SELECTOR_CODE32     equ     label_desc_code32 - label_gdt_null
SELECTOR_VIDEO      equ     label_desc_video - label_gdt_null


[BITS 16]
[SECTION .s16]
LABEL_BEGIN:
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0100h

    ; 初始化32位代码段描述符
    xor eax, eax
    mov ax, cs
    shl eax, 4
    add eax, label_desc_code32
    mov word [label_desc_code32 + 2], ax
    mov eax, 16
    mov byte [label_desc_code32 + 4], al
    mov byte [label_desc_code32 + 7], ah

    ; 准备加载gdtr
    xor eax, eax
    mov ax, ds
    shl eax, 4
    add eax, label_gdt_null
    mov dword [gdt_ptr + 2], eax

    ; 加载gdtr
    lgdt [gdt_ptr]

    ; 关中断
    cli

    ; 打开a20地址线
    ;push ax
    ;mov ax, 02401h ; 如果是0x2400则关闭A20地址线
    ;int 15h
    ;pop ax
    in al, 92h
    or al, 00000010b
    out 92h, al

    ; 开启保护模式
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; 跳转到32位代码段
    jmp dword SELECTOR_CODE32:0
; END OF LABEL_BEGIN


[BITS 32]
[SECTION .s32]
LABEL_SEG_CODE32:
    mov ax, SELECTOR_VIDEO
    mov gs, ax
    mov edi, (80 * 11 + 79) * 2 ; 11行79列
    mov ah, 0Ch
    mov al, 'p'
    mov [gs:edi], ax
    jmp $
SEG_CODE32_LEN      equ     $ - LABEL_SEG_CODE32
; end of LABEL_SEG_CODE32
