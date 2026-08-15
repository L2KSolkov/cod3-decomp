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

_TEXT ENDS
END
