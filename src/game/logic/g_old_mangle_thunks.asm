; ============================================================================
; g_old_mangle_thunks.asm - g.o functions whose MSVC 14.51 mangling differs
; from the Xbox-era compiler (repeated-class-type backref digits 0 vs 1).
; Modern C++ definitions carry the modern mangling; these thunks provide the
; exact binary names by forwarding (jmp preserves the cdecl call).
; ============================================================================

.386
.model flat

EXTERN ?Die_MineDamaged@@YAXPAVEntity@@00HHHPBM0W4hitLocation_t@@@Z : PROC
EXTERN ?Pain_trigger_damage@@YAXPAVEntity@@0HPBMH0W4hitLocation_t@@@Z : PROC
EXTERN ?Die_trigger_damage@@YAXPAVEntity@@00HHHPBM0W4hitLocation_t@@@Z : PROC
EXTERN ?VEH_GetPlayerFollowGoalPosition@@YAPAY02$$CBMPBUEntity@@0@Z : PROC

_TEXT SEGMENT

; ea: 0x0044C6B0 - ?Die_MineDamaged@@YAXPAVEntity@@00HHHQBM1W4hitLocation_t@@@Z
PUBLIC ?Die_MineDamaged@@YAXPAVEntity@@00HHHQBM1W4hitLocation_t@@@Z
?Die_MineDamaged@@YAXPAVEntity@@00HHHQBM1W4hitLocation_t@@@Z PROC
    jmp ?Die_MineDamaged@@YAXPAVEntity@@00HHHPBM0W4hitLocation_t@@@Z
?Die_MineDamaged@@YAXPAVEntity@@00HHHQBM1W4hitLocation_t@@@Z ENDP

; ea: 0x00470B50 - ?Pain_trigger_damage@@YAXPAVEntity@@0HQBMH1W4hitLocation_t@@@Z
PUBLIC ?Pain_trigger_damage@@YAXPAVEntity@@0HQBMH1W4hitLocation_t@@@Z
?Pain_trigger_damage@@YAXPAVEntity@@0HQBMH1W4hitLocation_t@@@Z PROC
    jmp ?Pain_trigger_damage@@YAXPAVEntity@@0HPBMH0W4hitLocation_t@@@Z
?Pain_trigger_damage@@YAXPAVEntity@@0HQBMH1W4hitLocation_t@@@Z ENDP

; ea: 0x00470B90 - ?Die_trigger_damage@@YAXPAVEntity@@00HHHQBM1W4hitLocation_t@@@Z
PUBLIC ?Die_trigger_damage@@YAXPAVEntity@@00HHHQBM1W4hitLocation_t@@@Z
?Die_trigger_damage@@YAXPAVEntity@@00HHHQBM1W4hitLocation_t@@@Z PROC
    jmp ?Die_trigger_damage@@YAXPAVEntity@@00HHHPBM0W4hitLocation_t@@@Z
?Die_trigger_damage@@YAXPAVEntity@@00HHHQBM1W4hitLocation_t@@@Z ENDP

; ea: 0x0044E6A0 - ?VEH_GetPlayerFollowGoalPosition@@YAAAY02$$CBMPBVEntity@@0@Z
PUBLIC ?VEH_GetPlayerFollowGoalPosition@@YAAAY02$$CBMPBVEntity@@0@Z
?VEH_GetPlayerFollowGoalPosition@@YAAAY02$$CBMPBVEntity@@0@Z PROC
    jmp ?VEH_GetPlayerFollowGoalPosition@@YAPAY02$$CBMPBUEntity@@0@Z
?VEH_GetPlayerFollowGoalPosition@@YAAAY02$$CBMPBVEntity@@0@Z ENDP

; ea: 0x004ACEE0 - ??0?$pair@IPBD@std@@QAE@XZ
; MSVC 14.51 emits the templated-ctor form ??$?0IPBD$0A@@?$pair@IPBD@std@@QAE@XZ.
EXTERN ??$?0IPBD$0A@@?$pair@IPBD@std@@QAE@XZ : PROC
PUBLIC ??0?$pair@IPBD@std@@QAE@XZ
??0?$pair@IPBD@std@@QAE@XZ PROC
    jmp ??$?0IPBD$0A@@?$pair@IPBD@std@@QAE@XZ
??0?$pair@IPBD@std@@QAE@XZ ENDP

; ea: 0x004B0090 - ??_F?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@QAEXXZ
; Default-constructor closure: forwards to the default ctor.
EXTERN ??0?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@QAE@XZ : PROC
PUBLIC ??_F?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@QAEXXZ
??_F?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@QAEXXZ PROC
    jmp ??0?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@QAE@XZ
??_F?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@QAEXXZ ENDP

; ============================================================================
; Vector-deleting destructors (??_E) forwarding to the scalar form (??_G).
; The release binary's ??_E bodies are equivalent (single-object free).
; ============================================================================

; ea: 0x004A52F0 - ??_ETask@@UAEPAXI@Z
EXTERN ??_GTask@@UAEPAXI@Z : PROC
PUBLIC ??_ETask@@UAEPAXI@Z
??_ETask@@UAEPAXI@Z PROC
    jmp ??_GTask@@UAEPAXI@Z
??_ETask@@UAEPAXI@Z ENDP

; ea: 0x004AF160 - ??_ETaskFunctor@@UAEPAXI@Z
EXTERN ??_GTaskFunctor@@UAEPAXI@Z : PROC
PUBLIC ??_ETaskFunctor@@UAEPAXI@Z
??_ETaskFunctor@@UAEPAXI@Z PROC
    jmp ??_GTaskFunctor@@UAEPAXI@Z
??_ETaskFunctor@@UAEPAXI@Z ENDP

; ea: 0x004A5980 - ??_EWaitTilOutput@@UAEPAXI@Z
EXTERN ??_GWaitTilOutput@@UAEPAXI@Z : PROC
PUBLIC ??_EWaitTilOutput@@UAEPAXI@Z
??_EWaitTilOutput@@UAEPAXI@Z PROC
    jmp ??_GWaitTilOutput@@UAEPAXI@Z
??_EWaitTilOutput@@UAEPAXI@Z ENDP

; ea: 0x004B0D60 - ??_EEntityDeathTask@@UAEPAXI@Z
EXTERN ??_GEntityDeathTask@@UAEPAXI@Z : PROC
PUBLIC ??_EEntityDeathTask@@UAEPAXI@Z
??_EEntityDeathTask@@UAEPAXI@Z PROC
    jmp ??_GEntityDeathTask@@UAEPAXI@Z
??_EEntityDeathTask@@UAEPAXI@Z ENDP

; ea: 0x004B1560 - ??_E?$TaskFunctor1@VAnimationUpdateTask@@M@@UAEPAXI@Z
EXTERN ??_G?$TaskFunctor1@VAnimationUpdateTask@@M@@UAEPAXI@Z : PROC
PUBLIC ??_E?$TaskFunctor1@VAnimationUpdateTask@@M@@UAEPAXI@Z
??_E?$TaskFunctor1@VAnimationUpdateTask@@M@@UAEPAXI@Z PROC
    jmp ??_G?$TaskFunctor1@VAnimationUpdateTask@@M@@UAEPAXI@Z
??_E?$TaskFunctor1@VAnimationUpdateTask@@M@@UAEPAXI@Z ENDP

; ea: 0x004B15E0 - ??_E?$TaskFunctor1@VXAnimUpdateTask@@M@@UAEPAXI@Z
EXTERN ??_G?$TaskFunctor1@VXAnimUpdateTask@@M@@UAEPAXI@Z : PROC
PUBLIC ??_E?$TaskFunctor1@VXAnimUpdateTask@@M@@UAEPAXI@Z
??_E?$TaskFunctor1@VXAnimUpdateTask@@M@@UAEPAXI@Z PROC
    jmp ??_G?$TaskFunctor1@VXAnimUpdateTask@@M@@UAEPAXI@Z
??_E?$TaskFunctor1@VXAnimUpdateTask@@M@@UAEPAXI@Z ENDP

; ea: 0x004B1F60 - ??_E?$WaitTilOutputInst1@Vstring@Broc@@@@UAEPAXI@Z
EXTERN ??_G?$WaitTilOutputInst1@Vstring@Broc@@@@UAEPAXI@Z : PROC
PUBLIC ??_E?$WaitTilOutputInst1@Vstring@Broc@@@@UAEPAXI@Z
??_E?$WaitTilOutputInst1@Vstring@Broc@@@@UAEPAXI@Z PROC
    jmp ??_G?$WaitTilOutputInst1@Vstring@Broc@@@@UAEPAXI@Z
??_E?$WaitTilOutputInst1@Vstring@Broc@@@@UAEPAXI@Z ENDP

; ea: 0x004B1FF0 - ??_E?$WaitTilOutputInst2@Vstring@Broc@@V12@@@UAEPAXI@Z
EXTERN ??_G?$WaitTilOutputInst2@Vstring@Broc@@V12@@@UAEPAXI@Z : PROC
PUBLIC ??_E?$WaitTilOutputInst2@Vstring@Broc@@V12@@@UAEPAXI@Z
??_E?$WaitTilOutputInst2@Vstring@Broc@@V12@@@UAEPAXI@Z PROC
    jmp ??_G?$WaitTilOutputInst2@Vstring@Broc@@V12@@@UAEPAXI@Z
??_E?$WaitTilOutputInst2@Vstring@Broc@@V12@@@UAEPAXI@Z ENDP

; ea: 0x004B2940 - ??_E?$WaitTilOutputInst1@I@@UAEPAXI@Z
EXTERN ??_G?$WaitTilOutputInst1@I@@UAEPAXI@Z : PROC
PUBLIC ??_E?$WaitTilOutputInst1@I@@UAEPAXI@Z
??_E?$WaitTilOutputInst1@I@@UAEPAXI@Z PROC
    jmp ??_G?$WaitTilOutputInst1@I@@UAEPAXI@Z
??_E?$WaitTilOutputInst1@I@@UAEPAXI@Z ENDP

; ea: 0x004B2A70 - ??_E?$WaitTilOutputInst1@Ventity@Broc@@@@UAEPAXI@Z
EXTERN ??_G?$WaitTilOutputInst1@Ventity@Broc@@@@UAEPAXI@Z : PROC
PUBLIC ??_E?$WaitTilOutputInst1@Ventity@Broc@@@@UAEPAXI@Z
??_E?$WaitTilOutputInst1@Ventity@Broc@@@@UAEPAXI@Z PROC
    jmp ??_G?$WaitTilOutputInst1@Ventity@Broc@@@@UAEPAXI@Z
??_E?$WaitTilOutputInst1@Ventity@Broc@@@@UAEPAXI@Z ENDP

; ea: 0x004B2BB0 - ??_E?$WaitTilOutputInst2@HVentity@Broc@@@@UAEPAXI@Z
EXTERN ??_G?$WaitTilOutputInst2@HVentity@Broc@@@@UAEPAXI@Z : PROC
PUBLIC ??_E?$WaitTilOutputInst2@HVentity@Broc@@@@UAEPAXI@Z
??_E?$WaitTilOutputInst2@HVentity@Broc@@@@UAEPAXI@Z PROC
    jmp ??_G?$WaitTilOutputInst2@HVentity@Broc@@@@UAEPAXI@Z
??_E?$WaitTilOutputInst2@HVentity@Broc@@@@UAEPAXI@Z ENDP

; ea: 0x004A97A0 - ??_H@YGXPAXIHP6EPAX0@Z@Z (vector constructor iterator)
EXTERN ?vector_ctor_iterator_helper@@YGXPAXIHP6EPAX0@Z@Z : PROC
PUBLIC ??_H@YGXPAXIHP6EPAX0@Z@Z
??_H@YGXPAXIHP6EPAX0@Z@Z PROC
    jmp ?vector_ctor_iterator_helper@@YGXPAXIHP6EPAX0@Z@Z
??_H@YGXPAXIHP6EPAX0@Z@Z ENDP

; ea: 0x004AEBA0 - ??_9@$B3AE (vcall thunk: virtual at vftable+4)
PUBLIC ??_9@$B3AE
??_9@$B3AE PROC
    mov eax, [ecx]
    jmp dword ptr [eax+4]
??_9@$B3AE ENDP

; ea: 0x004B1920 - ??0const_iterator@?$reserved_dlist@VPakFile@@@@QAE@ABV1@@Z
; Old-compiler backref digit (1) vs MSVC 14.51 (01).
EXTERN ??0const_iterator@?$reserved_dlist@VPakFile@@@@QAE@ABV01@@Z : PROC
PUBLIC ??0const_iterator@?$reserved_dlist@VPakFile@@@@QAE@ABV1@@Z
??0const_iterator@?$reserved_dlist@VPakFile@@@@QAE@ABV1@@Z PROC
    jmp ??0const_iterator@?$reserved_dlist@VPakFile@@@@QAE@ABV01@@Z
??0const_iterator@?$reserved_dlist@VPakFile@@@@QAE@ABV1@@Z ENDP

_TEXT ENDS
END
