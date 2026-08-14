; ============================================================================
; vecdtors.asm - anim.o vector deleting destructors (??_E) 
; Bodies transcribed from IDA disasm (VC7.1 emitted scalar-style bodies; the
; map lists ??_G and ??_E folded at one address).
; ============================================================================

.386
.model flat

; ---- externals ----
EXTERN ?mem_heap_free@@YAXPAX@Z : PROC
EXTERN ?tlMemFree@@YAXPAX@Z : PROC
EXTERN ??1AnimBankManager@@UAE@XZ : PROC
EXTERN ??1?$nalComponent@VnalComponentBase@@VCODNoteData@@VCODNoteTrack@@@@MAE@XZ : PROC
EXTERN ??_7nalBasePoseBlender@@6B@ : DWORD
EXTERN ??_7nalInitList@@6B@ : DWORD
EXTERN ??_7nalInstanceClass@?$nalAnimClass@VnalAnyPose@@@@6B@ : DWORD
EXTERN ??_7nalComponentBase@@6B@ : DWORD

_TEXT SEGMENT

; ea: 0x0055E450 - ??_EnalBasePoseBlender@@MAEPAXI@Z
PUBLIC ??_EnalBasePoseBlender@@MAEPAXI@Z
??_EnalBasePoseBlender@@MAEPAXI@Z PROC
    push ebp
    mov  ebp, esp
    test byte ptr [ebp+0Ch], 1
    push esi
    mov  esi, ecx
    mov  dword ptr [esi], OFFSET ??_7nalBasePoseBlender@@6B@
    jz   short loc_skip_bpb
    push esi
    call ?mem_heap_free@@YAXPAX@Z
    add  esp, 4
loc_skip_bpb:
    mov  eax, esi
    pop  esi
    pop  ebp
    ret  4
??_EnalBasePoseBlender@@MAEPAXI@Z ENDP

; ea: 0x0055E4B0 - ??_EnalInitList@@UAEPAXI@Z
PUBLIC ??_EnalInitList@@UAEPAXI@Z
??_EnalInitList@@UAEPAXI@Z PROC
    push ebp
    mov  ebp, esp
    test byte ptr [ebp+0Ch], 1
    push esi
    mov  esi, ecx
    mov  dword ptr [esi], OFFSET ??_7nalInitList@@6B@
    jz   short loc_skip_nil
    push esi
    call ?mem_heap_free@@YAXPAX@Z
    add  esp, 4
loc_skip_nil:
    mov  eax, esi
    pop  esi
    pop  ebp
    ret  4
??_EnalInitList@@UAEPAXI@Z ENDP

; ea: 0x0055E670 - ??_EnalInstanceClass@?$nalAnimClass@VnalAnyPose@@@@UAEPAXI@Z
PUBLIC ??_EnalInstanceClass@?$nalAnimClass@VnalAnyPose@@@@UAEPAXI@Z
??_EnalInstanceClass@?$nalAnimClass@VnalAnyPose@@@@UAEPAXI@Z PROC
    push ebp
    mov  ebp, esp
    push esi
    mov  esi, ecx
    mov  eax, dword ptr [esi+10h]
    mov  dword ptr [esi], OFFSET ??_7nalInstanceClass@?$nalAnimClass@VnalAnyPose@@@@6B@
    dec  dword ptr [eax+3Ch]
    test byte ptr [ebp+0Ch], 1
    jz   short loc_skip_ic
    push esi
    call ?tlMemFree@@YAXPAX@Z
    add  esp, 4
loc_skip_ic:
    mov  eax, esi
    pop  esi
    pop  ebp
    ret  4
??_EnalInstanceClass@?$nalAnimClass@VnalAnyPose@@@@UAEPAXI@Z ENDP

; ea: 0x0055E840 - ??_EnalComponentBase@@UAEPAXI@Z
PUBLIC ??_EnalComponentBase@@UAEPAXI@Z
??_EnalComponentBase@@UAEPAXI@Z PROC
    push ebp
    mov  ebp, esp
    test byte ptr [ebp+0Ch], 1
    push esi
    mov  esi, ecx
    mov  dword ptr [esi], OFFSET ??_7nalComponentBase@@6B@
    jz   short loc_skip_ncb
    push esi
    call ?mem_heap_free@@YAXPAX@Z
    add  esp, 4
loc_skip_ncb:
    mov  eax, esi
    pop  esi
    pop  ebp
    ret  4
??_EnalComponentBase@@UAEPAXI@Z ENDP

; ea: 0x005602A0 - ??_EAnimBankManager@@UAEPAXI@Z
PUBLIC ??_EAnimBankManager@@UAEPAXI@Z
??_EAnimBankManager@@UAEPAXI@Z PROC
    push ebp
    mov  ebp, esp
    push esi
    mov  esi, ecx
    call ??1AnimBankManager@@UAE@XZ
    test byte ptr [ebp+0Ch], 1
    jz   short loc_skip_abm
    push esi
    call ?mem_heap_free@@YAXPAX@Z
    add  esp, 4
loc_skip_abm:
    mov  eax, esi
    pop  esi
    pop  ebp
    ret  4
??_EAnimBankManager@@UAEPAXI@Z ENDP

; ea: 0x00560590 - ??_E?$nalComponent@VnalComponentBase@@VCODNoteData@@VCODNoteTrack@@@@MAEPAXI@Z
PUBLIC ??_E?$nalComponent@VnalComponentBase@@VCODNoteData@@VCODNoteTrack@@@@MAEPAXI@Z
??_E?$nalComponent@VnalComponentBase@@VCODNoteData@@VCODNoteTrack@@@@MAEPAXI@Z PROC
    push ebp
    mov  ebp, esp
    push esi
    mov  esi, ecx
    call ??1?$nalComponent@VnalComponentBase@@VCODNoteData@@VCODNoteTrack@@@@MAE@XZ
    test byte ptr [ebp+0Ch], 1
    jz   short loc_skip_nc
    push esi
    call ?mem_heap_free@@YAXPAX@Z
    add  esp, 4
loc_skip_nc:
    mov  eax, esi
    pop  esi
    pop  ebp
    ret  4
??_E?$nalComponent@VnalComponentBase@@VCODNoteData@@VCODNoteTrack@@@@MAEPAXI@Z ENDP

_TEXT ENDS
END
