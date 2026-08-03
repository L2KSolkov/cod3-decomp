// ============================================================================
// AE Controller — controller init/poll wrappers for assertion UI
// ea: 0x7BF240 (InitController), 0x7BF250 (PollController)
// ============================================================================

// Global — handle for assertion UI controller input
static void* s_handleForAssert = nullptr;

// XInput state stubs (will be replaced when controller_xboxr is ported)
namespace {
    struct XINPUT_STATE {
        struct {
            unsigned char bAnalogButtons[8];
        } Gamepad;
    };
    static XINPUT_STATE xiCurrState = {};
    static XINPUT_STATE xiPrevState = {};
    static bool bPrevState = false;
}

void InitController(void* handle) {
    s_handleForAssert = handle;
}

unsigned int PollController(int whichButtons, int* pnWhichButtonHit) {
    // stub — always returns "no button pressed" (2)
    // Full implementation at ea:0x7BF250 requires controller::inst() and XInput
    *pnWhichButtonHit = 0;
    return 2;
}
