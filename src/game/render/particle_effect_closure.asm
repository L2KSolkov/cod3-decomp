; ============================================================================
; particle_effect_closure.asm - render.o ParticleEffect default constructor
; closure (??_FParticleEffect@@QAEXXZ @ 0x6EBED0). MSVC 14.51 emits ??_E but
; not the legacy ??_F closure for new[]; transcribed from IDA disasm.
; ============================================================================

.686
.xmm
.model flat

_TEXT SEGMENT

; ea: 0x006EBED0 - ??_FParticleEffect@@QAEXXZ
PUBLIC ??_FParticleEffect@@QAEXXZ
??_FParticleEffect@@QAEXXZ PROC
    movss       xmm0, dword ptr [__real_bf800000]
    xor         eax, eax
    mov         dword ptr [ecx+28h], eax   ; mDObjHandle
    mov         dword ptr [ecx+2Ch], eax   ; mEntHandle
    mov         word ptr [ecx+32h], ax     ; mFlags = 0
    or          edx, 0FFFFFFFFh
    mov         word ptr [ecx+18h], dx     ; mIndex.first = -1
    mov         word ptr [ecx+1Ah], dx     ; mIndex.second = -1
    mov         dword ptr [ecx+1Ch], eax   ; mEffect
    mov         dword ptr [ecx+28h], eax   ; mDObjHandle
    mov         dword ptr [ecx+2Ch], eax   ; mEntHandle
    mov         dword ptr [ecx+24h], eax   ; mPoPtr
    mov         dword ptr [ecx+34h], edx   ; mPakId = PAK_ID_INVALID
    mov         word ptr [ecx+32h], ax     ; mFlags = 0
    or          byte ptr [ecx+32h], 10h    ; mFlags |= 0x10
    mov         dword ptr [ecx+38h], eax   ; mRaycastData
    movss       dword ptr [ecx+0Ch], xmm0  ; cached_pos[3] = -1.0f
    mov         dword ptr [ecx+14h], eax   ; culled = 0
    retn
??_FParticleEffect@@QAEXXZ ENDP

__real_bf800000 DD 0BF800000h

_TEXT ENDS
END
