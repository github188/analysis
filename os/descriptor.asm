; 描述符
; usage: Descriptor Base, Limit, Attr
;        Base: dd
;        Limit: dd (low 20 bits available)
;        Attr: dw (lower 4 bits or higher byte are always 0)

%macro DESCRIPTOR 3
    dw      %2 & 0FFFFh ; 段界限1
    dw      %1 & 0FFFFh ; 段基址1
    db      (%1 >> 16) & 0FFh ; 段基址2
    dw      ((%2 >> 8) & 0F00h) | (%3 & 0F0FFh) ; 属性1 + 段界限2 + 属性2
    db      (%1 >> 24) & 0FFh ; 段基址3
%endmacro
