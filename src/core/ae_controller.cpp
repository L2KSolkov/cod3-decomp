// ea: 0x7BF240 (InitController), 0x7BF250 (PollController)

#include "input/controller.h"
#include <stdint.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <xinput.h>
#else
using DWORD = unsigned int;
using BYTE = unsigned char;
using WORD = unsigned short;
using SHORT = short;
struct XINPUT_GAMEPAD { WORD wButtons; BYTE bLeftTrigger; BYTE bRightTrigger; SHORT sThumbLX, sThumbLY, sThumbRX, sThumbRY; };
struct XINPUT_STATE { DWORD dwPacketNumber; XINPUT_GAMEPAD Gamepad; };
#endif

namespace {
static void* s_handleForAssert = nullptr;
static XINPUT_STATE xiCurrState = {};
static XINPUT_STATE xiPrevState = {};
static bool bPrevState = false;
static int dword_10DDAE4 = 0;
static int16_t word_10DDAE8 = 0;
static int dword_10DDAFC = 0;
static int16_t word_10DDB00 = 0;

#ifdef _WIN32
using GetStateFn = DWORD (WINAPI*)(DWORD, XINPUT_STATE*);
static HMODULE s_xinput = nullptr;
static GetStateFn s_get_state = nullptr;

static DWORD get_state(int handle, XINPUT_STATE* state)
{
    if (s_get_state == nullptr && s_xinput == nullptr) {
        const char* modules[] = {"xinput1_4.dll", "xinput1_3.dll", "xinput9_1_0.dll"};
        for (const char* module : modules) {
            s_xinput = LoadLibraryA(module);
            if (s_xinput != nullptr)
                break;
        }
        if (s_xinput != nullptr)
            s_get_state = reinterpret_cast<GetStateFn>(GetProcAddress(s_xinput, "XInputGetState"));
    }
    if (s_get_state == nullptr)
        return ERROR_DEVICE_NOT_CONNECTED;
    const unsigned int port = static_cast<unsigned int>(handle);
    return s_get_state(port, state);
}
#else
static DWORD get_state(int, XINPUT_STATE*) { return 1167; }
#endif
}

void InitController(int handle)
{
    s_handleForAssert = reinterpret_cast<void*>(static_cast<uintptr_t>(handle));
}

int PollController(int whichButtons, int* whichButtonHit)
{
    controller* pad = controller::inst();
    pad->stop_all_rumble();
    if (s_handleForAssert == 0) {
        pad->poll();
        return 2;
    }

    const DWORD result = get_state(static_cast<int>(reinterpret_cast<uintptr_t>(s_handleForAssert)),
                                   &xiCurrState);
    if (result != 0)
        return result;
    if (bPrevState) {
        if ((whichButtons & 4) != 0
            && xiCurrState.Gamepad.bLeftTrigger > 0x1E
            && xiPrevState.Gamepad.bLeftTrigger < 0x1E) {
            *whichButtonHit = 4;
            bPrevState = false;
            return 1;
        }
        if ((whichButtons & 1) != 0
            && static_cast<BYTE>(xiCurrState.Gamepad.sThumbLX) > 0x1E
            && static_cast<BYTE>(xiPrevState.Gamepad.sThumbLX) < 0x1E) {
            bPrevState = false;
            *whichButtonHit = 1;
            return 1;
        }
        if ((whichButtons & 2) != 0
            && xiCurrState.Gamepad.bRightTrigger > 0x1E
            && xiPrevState.Gamepad.bRightTrigger < 0x1E) {
            bPrevState = false;
            *whichButtonHit = 2;
            return 1;
        }
        if ((whichButtons & 8) != 0
            && static_cast<BYTE>(xiCurrState.Gamepad.sThumbLX >> 8) > 0x1E
            && static_cast<BYTE>(xiPrevState.Gamepad.sThumbLX >> 8) < 0x1E) {
            *whichButtonHit = 8;
            bPrevState = false;
            return 1;
        }
        if ((whichButtons & 0x10) != 0
            && static_cast<BYTE>(xiCurrState.Gamepad.sThumbLY >> 8) > 0x1E
            && static_cast<BYTE>(xiPrevState.Gamepad.sThumbLY >> 8) < 0x1E) {
            bPrevState = false;
            *whichButtonHit = 16;
            return 1;
        }
    }
    xiPrevState = xiCurrState;
    dword_10DDAE4 = dword_10DDAFC;
    word_10DDAE8 = word_10DDB00;
    bPrevState = true;
    return 2;
}
