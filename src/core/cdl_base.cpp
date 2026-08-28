// ============================================================================
// CDL Base — init and profile reset stubs
// Source: source/cdl_base.cpp
// ea: 0x81E550 (cdl_init), 0x81E560 (cdl_profile_reset)
// ============================================================================

#include <cstring>

// External profile counters (declared in cdl_gjk.cpp)
extern struct cdl_proftimer { unsigned __int64 stamp; unsigned __int64 value; }
    cdl_proftimer_closest, cdl_proftimer_support, cdl_proftimer_collide,
    cdl_proftimer_gjk, cdl_proftimer_push_out_sphere,
    cdl_proftimer_test1, cdl_proftimer_test2,
    cdl_proftimer_local_failure, cdl_proftimer_partial_failure,
    cdl_proftimer_full_failure, cdl_proftimer_make_hull;
extern struct cdl_profcounter { unsigned __int64 value; }
    cdl_profcounter_collide_calls, cdl_profcounter_gjk_separated,
    cdl_profcounter_gjk_invalid, cdl_profcounter_gjk;

// ea: 0x81E550
void cdl_init() {
    // No initialization needed (ea: empty function)
}

// ea: 0x81E560
void cdl_profile_reset() {
    cdl_proftimer_closest.value = 0;
    cdl_proftimer_support.value = 0;
    cdl_proftimer_collide.value = 0;
    cdl_proftimer_gjk.value = 0;
    cdl_proftimer_push_out_sphere.value = 0;
    cdl_proftimer_test1.value = 0;
    cdl_proftimer_test2.value = 0;
    cdl_proftimer_local_failure.value = 0;
    cdl_proftimer_partial_failure.value = 0;
    cdl_proftimer_full_failure.value = 0;
    cdl_proftimer_make_hull.value = 0;
    cdl_profcounter_collide_calls.value = 0;
    cdl_profcounter_gjk_separated.value = 0;
    cdl_profcounter_gjk_invalid.value = 0;
    cdl_profcounter_gjk.value = 0;
}
