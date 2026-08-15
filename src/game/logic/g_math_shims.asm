; ============================================================================
; g_math_shims.asm - g.o C math wrappers (_cosf/_sinf/_sqrtf/_fabsf/_powf/
; _tanf/_ceilf @ 0x4A5070-0x4A50F0). MSVC 14.51 decorates an extern "C"
; identifier already starting with '_' to '__', so these are provided here.
; Bodies forward to the CRT double-precision functions (float args promoted).
; ============================================================================

.386
.model flat

EXTERN _cos : PROC
EXTERN _sin : PROC
EXTERN _tan : PROC
EXTERN _sqrt : PROC
EXTERN _pow : PROC
EXTERN _ceil : PROC

_TEXT SEGMENT

; ea: 0x004A5090 - _cosf
PUBLIC _cosf
_cosf PROC
    push ebp
    mov  ebp, esp
    sub  esp, 8
    fld  dword ptr [ebp+8]
    fstp qword ptr [esp]
    call _cos
    add  esp, 8
    pop  ebp
    retn 4
_cosf ENDP

; ea: 0x004A50D0 - _sinf
PUBLIC _sinf
_sinf PROC
    push ebp
    mov  ebp, esp
    sub  esp, 8
    fld  dword ptr [ebp+8]
    fstp qword ptr [esp]
    call _sin
    add  esp, 8
    pop  ebp
    retn 4
_sinf ENDP

; ea: 0x004A50F0 - _tanf
PUBLIC _tanf
_tanf PROC
    push ebp
    mov  ebp, esp
    sub  esp, 8
    fld  dword ptr [ebp+8]
    fstp qword ptr [esp]
    call _tan
    add  esp, 8
    pop  ebp
    retn 4
_tanf ENDP

; ea: 0x004A50E0 - _sqrtf
PUBLIC _sqrtf
_sqrtf PROC
    push ebp
    mov  ebp, esp
    sub  esp, 8
    fld  dword ptr [ebp+8]
    fstp qword ptr [esp]
    call _sqrt
    add  esp, 8
    pop  ebp
    retn 4
_sqrtf ENDP

; ea: 0x004A50A0 - _fabsf (|x| via fabs instruction)
PUBLIC _fabsf
_fabsf PROC
    push ebp
    mov  ebp, esp
    fld  dword ptr [ebp+8]
    fabs
    pop  ebp
    retn 4
_fabsf ENDP

; ea: 0x004A50B0 - _powf
PUBLIC _powf
_powf PROC
    push ebp
    mov  ebp, esp
    sub  esp, 16
    fld  dword ptr [ebp+0Ch]
    fstp qword ptr [esp+8]
    fld  dword ptr [ebp+8]
    fstp qword ptr [esp]
    call _pow
    add  esp, 16
    pop  ebp
    retn 8
_powf ENDP

; ea: 0x004A5070 - _ceilf
PUBLIC _ceilf
_ceilf PROC
    push ebp
    mov  ebp, esp
    sub  esp, 8
    fld  dword ptr [ebp+8]
    fstp qword ptr [esp]
    call _ceil
    add  esp, 8
    pop  ebp
    retn 4
_ceilf ENDP

_TEXT ENDS
END
