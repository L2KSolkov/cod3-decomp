; ============================================================================
; placement_delete.asm - g.o global placement delete (??3@YAXPAX0@Z @
; 0x004489F0). MSVC 14.51 emits this only with exceptions enabled; the
; project compiles with /EHs-c- so it is provided here (empty body per IDA).
; ============================================================================

.386
.model flat

_TEXT SEGMENT

; ea: 0x004489F0 - ??3@YAXPAX0@Z
PUBLIC ??3@YAXPAX0@Z
??3@YAXPAX0@Z PROC
    retn
??3@YAXPAX0@Z ENDP

_TEXT ENDS
END
