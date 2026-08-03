// ============================================================================
// CDL Base — init and profile reset stubs
// Source: source/cdl_base.cpp
// ea: 0x81E550 (cdl_init), 0x81E560 (cdl_profile_reset)
// ============================================================================

#include <cstring>

// External profile counters (declared in cdl_gjk.cpp)
extern struct cdl_proftimer { float value; unsigned int _pad[3]; }
    cdl_proftimer_closest, cdl_proftimer_support, cdl_proftimer_collide,
    cdl_proftimer_gjk, cdl_proftimer_push_out_sphere,
    cdl_proftimer_test1, cdl_proftimer_test2,
    cdl_proftimer_local_failure, cdl_proftimer_partial_failure,
    cdl_proftimer_full_failure, cdl_proftimer_make_hull;
extern struct cdl_profcounter { int value; unsigned int _pad[3]; }
    cdl_profcounter_collide_calls, cdl_profcounter_gjk_separated,
    cdl_profcounter_gjk_invalid, cdl_profcounter_gjk;

void cdl_init() {
    // No initialization needed (ea: empty function)
}

void cdl_profile_reset() {
    memset(&cdl_proftimer_closest, 0, sizeof(cdl_proftimer_closest));
    memset(&cdl_proftimer_support, 0, sizeof(cdl_proftimer_support));
    memset(&cdl_proftimer_collide,  0, sizeof(cdl_proftimer_collide));
    memset(&cdl_proftimer_gjk,      0, sizeof(cdl_proftimer_gjk));
    memset(&cdl_proftimer_push_out_sphere, 0, sizeof(cdl_proftimer_push_out_sphere));
    memset(&cdl_proftimer_test1,    0, sizeof(cdl_proftimer_test1));
    memset(&cdl_proftimer_test2,    0, sizeof(cdl_proftimer_test2));
    memset(&cdl_proftimer_local_failure,    0, sizeof(cdl_proftimer_local_failure));
    memset(&cdl_proftimer_partial_failure,  0, sizeof(cdl_proftimer_partial_failure));
    memset(&cdl_proftimer_full_failure,     0, sizeof(cdl_proftimer_full_failure));
    memset(&cdl_proftimer_make_hull,        0, sizeof(cdl_proftimer_make_hull));
    cdl_profcounter_collide_calls.value = 0;
    cdl_profcounter_gjk_separated.value = 0;
    cdl_profcounter_gjk_invalid.value = 0;
    cdl_profcounter_gjk.value = 0;
}
