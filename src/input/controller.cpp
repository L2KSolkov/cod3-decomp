// ea: 0x7E1AA0-0x7E31C9
// controller_xboxr/controller.o input port. The XBGAMEPAD record and all
// edge/button mappings below follow the IDA type and decompiled functions.

#include "input/controller.h"

#include <cstring>
#include <new>

#include "core/mem_heap.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <xinput.h>
#else
using DWORD = unsigned int;
using WORD = unsigned short;
using BYTE = unsigned char;
using SHORT = short;
struct XINPUT_GAMEPAD {
    WORD wButtons;
    BYTE bLeftTrigger;
    BYTE bRightTrigger;
    SHORT sThumbLX;
    SHORT sThumbLY;
    SHORT sThumbRX;
    SHORT sThumbRY;
};
struct XINPUT_STATE { DWORD dwPacketNumber; XINPUT_GAMEPAD Gamepad; };
struct XINPUT_VIBRATION { WORD wLeftMotorSpeed; WORD wRightMotorSpeed; };
#endif

namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char*, ...);
bool Warning(const char*, ...);
}

extern bool g_controllerConnectedErrorShown[];
extern bool Controller_HandleUIXInput(unsigned int port, void* controller_input);

namespace {

#pragma pack(push, 1)
struct XboxInputCapabilities { unsigned char bytes[25]; };
#pragma pack(pop)

// IDA type 8072: 0x58 bytes including the host-aligned tail.
struct XBGAMEPAD {
    unsigned char raw[18];
    WORD last_buttons;
    bool last_analog_buttons[8];
    WORD pressed_buttons;
    bool pressed_analog_buttons[8];
    WORD released_buttons;
    bool released_analog_buttons[8];
    XboxInputCapabilities caps;
    void* hDevice;
    int inserted;
    int removed;
};

static_assert(sizeof(XBGAMEPAD) == 0x58, "XBGAMEPAD layout mismatch");
static_assert(offsetof(XBGAMEPAD, last_buttons) == 0x12,
              "XBGAMEPAD last_buttons offset mismatch");
static_assert(offsetof(XBGAMEPAD, pressed_buttons) == 0x1C,
              "XBGAMEPAD pressed_buttons offset mismatch");
static_assert(offsetof(XBGAMEPAD, released_buttons) == 0x26,
              "XBGAMEPAD released_buttons offset mismatch");
static_assert(offsetof(XBGAMEPAD, hDevice) == 0x4C,
              "XBGAMEPAD hDevice offset mismatch");

static XBGAMEPAD s_gamepads[controller::MAX_CONTROLLERS] = {};
static XINPUT_STATE s_input_states[controller::MAX_CONTROLLERS] = {};
static XINPUT_VIBRATION s_feedback[controller::MAX_CONTROLLERS] = {};
static controller* l_instance = nullptr;
static bool sUixHandled = false;

#ifdef _WIN32
using HostGetState = DWORD (WINAPI*)(DWORD, XINPUT_STATE*);
using HostSetState = DWORD (WINAPI*)(DWORD, XINPUT_VIBRATION*);
static HMODULE s_xinput_module = nullptr;
static HostGetState s_host_get_state = nullptr;
static HostSetState s_host_set_state = nullptr;

static void load_host_xinput()
{
    if (s_xinput_module != nullptr || s_host_get_state != nullptr)
        return;
    const char* modules[] = {"xinput1_4.dll", "xinput1_3.dll", "xinput9_1_0.dll"};
    for (const char* module : modules) {
        s_xinput_module = LoadLibraryA(module);
        if (s_xinput_module != nullptr)
            break;
    }
    if (s_xinput_module == nullptr)
        return;
    s_host_get_state = reinterpret_cast<HostGetState>(
        GetProcAddress(s_xinput_module, "XInputGetState"));
    s_host_set_state = reinterpret_cast<HostSetState>(
        GetProcAddress(s_xinput_module, "XInputSetState"));
}

static DWORD host_get_state(unsigned int port, XINPUT_STATE* state)
{
    load_host_xinput();
    return s_host_get_state != nullptr ? s_host_get_state(port, state) : ERROR_DEVICE_NOT_CONNECTED;
}

static DWORD host_set_state(unsigned int port, XINPUT_VIBRATION* vibration)
{
    load_host_xinput();
    return s_host_set_state != nullptr ? s_host_set_state(port, vibration) : ERROR_DEVICE_NOT_CONNECTED;
}
#else
static DWORD host_get_state(unsigned int, XINPUT_STATE*) { return 1167; }
static DWORD host_set_state(unsigned int, XINPUT_VIBRATION*) { return 1167; }
#endif

static unsigned char digital_level(bool down) { return down ? 0xFF : 0; }

#ifdef _WIN32
// Temporary host keyboard mapping for front-end navigation.
static void merge_host_keyboard(XBGAMEPAD& pad)
{
    auto down = [](int key) {
        return (GetAsyncKeyState(key) & 0x8000) != 0;
    };

    WORD buttons = *reinterpret_cast<const WORD*>(pad.raw);
    if (down(VK_UP) || down('W'))
        buttons |= 0x0001;
    if (down(VK_DOWN) || down('S'))
        buttons |= 0x0002;
    if (down(VK_LEFT) || down('A'))
        buttons |= 0x0004;
    if (down(VK_RIGHT) || down('D'))
        buttons |= 0x0008;
    if (down(VK_TAB))
        buttons |= 0x0020;
    *reinterpret_cast<WORD*>(pad.raw) = buttons;

    if (down(VK_RETURN) || down(VK_SPACE))
        pad.raw[2] = 0xFF;
    if (down(VK_ESCAPE) || down(VK_BACK))
        pad.raw[3] = 0xFF;
}
#endif

// Convert modern XInput into the Xbox byte view consumed by controller.o.
static void copy_host_state(XBGAMEPAD& pad, const XINPUT_STATE& state)
{
    std::memset(pad.raw, 0, sizeof(pad.raw));
    pad.raw[0] = static_cast<unsigned char>(state.Gamepad.wButtons & 0xFF);
    pad.raw[1] = static_cast<unsigned char>(state.Gamepad.wButtons >> 8);
    pad.raw[2] = digital_level((state.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0);
    pad.raw[3] = digital_level((state.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0);
    pad.raw[4] = digital_level((state.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0);
    pad.raw[5] = digital_level((state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0);
    pad.raw[6] = state.Gamepad.bRightTrigger;
    pad.raw[7] = state.Gamepad.bLeftTrigger;
    pad.raw[8] = digital_level((state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0);
    pad.raw[9] = digital_level((state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0);
    pad.raw[11] = static_cast<unsigned char>(state.Gamepad.sThumbLX >> 8);
    pad.raw[13] = static_cast<unsigned char>(state.Gamepad.sThumbLY >> 8);
    pad.raw[15] = static_cast<unsigned char>(state.Gamepad.sThumbRX >> 8);
    pad.raw[17] = static_cast<unsigned char>(state.Gamepad.sThumbRY >> 8);
}

static bool analog_active(const XBGAMEPAD& pad, int index)
{
    return pad.raw[index + 2] > (index >= 6 ? 0x80u : 0x1Eu);
}

static void initialize_pad(XBGAMEPAD& pad, const XINPUT_STATE& state)
{
    std::memset(&pad, 0, sizeof(pad));
    copy_host_state(pad, state);
    pad.last_buttons = *reinterpret_cast<const WORD*>(pad.raw);
    for (int i = 0; i < 8; ++i)
        pad.last_analog_buttons[i] = analog_active(pad, i);
}

static unsigned short compute_motor_speed(float intensity)
{
    if (intensity < 0.0f || intensity > 1.0f) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "controller_xbox.cpp";
        AeAssert::gCurrentLine = 113;
        AeAssert::gCurrentExpr = "intensity >= 0.0f && intensity <= 1.0f";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Please add a descriptive string"))
            __debugbreak();
    }
    return static_cast<unsigned short>(intensity * 65535.0f);
}

static WORD button_bits(controller::ButtonIndex button)
{
    switch (button) {
    case controller::LEFTBUTTON: return 0x0004;
    case controller::DOWNBUTTON: return 0x0002;
    case controller::RIGHTBUTTON: return 0x0008;
    case controller::UPBUTTON: return 0x0001;
    case controller::R3: return 0x0080;
    case controller::L3: return 0x0040;
    case controller::START: return 0x0010;
    case controller::SELECT: return 0x0020;
    default: return 0;
    }
}

static int analog_index(controller::ButtonIndex button)
{
    switch (button) {
    case controller::SQUARE: return 2;
    case controller::X: return 0;
    case controller::CIRCLE: return 1;
    case controller::TRIANGLE: return 3;
    case controller::R1: return 7;
    case controller::L1: return 6;
    case controller::R2: return 4;
    case controller::L2: return 5;
    default: return -1;
    }
}

} // namespace

controller::controller()
    : button_value_fn(nullptr), button_released_fn(nullptr),
      button_released_clear_fn(nullptr), button_pressed_fn(nullptr),
      button_pressed_clear_fn(nullptr), stick_value_fn(nullptr),
      locked_port(0), is_locked(false)
{
    std::memset(accepting_input_from_controller, 1,
                sizeof(accepting_input_from_controller));
    load_host_xinput();
    for (int i = 0; i < MAX_CONTROLLERS; ++i) {
        std::memset(&s_gamepads[i], 0, sizeof(s_gamepads[i]));
        std::memset(&s_input_states[i], 0, sizeof(s_input_states[i]));
        std::memset(&s_feedback[i], 0, sizeof(s_feedback[i]));
        if (host_get_state(i, &s_input_states[i]) == ERROR_SUCCESS) {
            initialize_pad(s_gamepads[i], s_input_states[i]);
            s_gamepads[i].hDevice = reinterpret_cast<void*>(static_cast<uintptr_t>(i + 1));
        }
    }
}

controller* controller::inst()
{
    if (l_instance == nullptr) {
        void* memory = mem_heap_malloc(0x24u);
        if (memory != nullptr)
            l_instance = new (memory) controller();
    }
    return l_instance;
}

void controller::poll()
{
    for (int i = 0; i < MAX_CONTROLLERS; ++i) {
        XINPUT_STATE state = {};
        XBGAMEPAD& pad = s_gamepads[i];
        const bool was_connected = pad.hDevice != nullptr;
        const bool connected = host_get_state(i, &state) == ERROR_SUCCESS;
        pad.inserted = connected && !was_connected;
        pad.removed = !connected && was_connected;
#ifdef _WIN32
        const bool use_host_keyboard = i == 0;
#else
        const bool use_host_keyboard = false;
#endif
        if (!connected && !use_host_keyboard) {
            pad.hDevice = nullptr;
            std::memset(pad.raw, 0, sizeof(pad.raw));
            pad.last_buttons = 0;
            pad.pressed_buttons = 0;
            pad.released_buttons = 0;
            std::memset(pad.last_analog_buttons, 0, sizeof(pad.last_analog_buttons));
            std::memset(pad.pressed_analog_buttons, 0, sizeof(pad.pressed_analog_buttons));
            std::memset(pad.released_analog_buttons, 0, sizeof(pad.released_analog_buttons));
            continue;
        }

        if (connected) {
            pad.hDevice = reinterpret_cast<void*>(static_cast<uintptr_t>(i + 1));
            copy_host_state(pad, state);
        } else {
            std::memset(pad.raw, 0, sizeof(pad.raw));
        }
#ifdef _WIN32
        if (use_host_keyboard)
            merge_host_keyboard(pad);
#endif
        const WORD buttons = *reinterpret_cast<const WORD*>(pad.raw);
        if (connected && !g_controllerConnectedErrorShown[i]
            && Controller_HandleUIXInput(i, &state)) {
            sUixHandled = true;
        } else {
            sUixHandled = false;
            pad.pressed_buttons = buttons & static_cast<WORD>(~pad.last_buttons);
            pad.released_buttons = pad.last_buttons & static_cast<WORD>(~buttons);
        }
        pad.last_buttons = buttons;
        for (int j = 0; j < 8; ++j) {
            const bool active = analog_active(pad, j);
            if (!sUixHandled) {
                pad.pressed_analog_buttons[j] = active && !pad.last_analog_buttons[j];
                pad.released_analog_buttons[j] = !active && pad.last_analog_buttons[j];
            }
            pad.last_analog_buttons[j] = active;
        }
    }
}

bool controller::controller_is_connected(int controller_num)
{
    return s_gamepads[controller_num].hDevice != nullptr;
}

int controller::get_num_controllers() { return num_controllers; }

int controller::button_value(int controller_num, ButtonIndex button)
{
    if (sUixHandled)
        return 0;
    const XBGAMEPAD& pad = s_gamepads[controller_num];
    const int index = analog_index(button);
    if (index >= 0)
        return pad.raw[index == 0 ? 2 : index == 1 ? 3 : index == 2 ? 4 :
                   index == 3 ? 5 : index == 4 ? 6 : index == 5 ? 7 :
                   index == 6 ? 8 : 9];
    return (pad.last_buttons & button_bits(button)) != 0 ? -1 : 0;
}

bool controller::button_pressed(int controller_num, ButtonIndex button)
{
    const XBGAMEPAD& pad = s_gamepads[controller_num];
    const int index = analog_index(button);
    if (index >= 0)
        return pad.pressed_analog_buttons[index];
    return (pad.pressed_buttons & button_bits(button)) != 0;
}

bool controller::button_released(int controller_num, ButtonIndex button)
{
    const XBGAMEPAD& pad = s_gamepads[controller_num];
    const int index = analog_index(button);
    if (index >= 0)
        return pad.released_analog_buttons[index];
    return (pad.released_buttons & button_bits(button)) != 0;
}

bool controller::button_pressed_clear(int controller_num, ButtonIndex button)
{
    XBGAMEPAD& pad = s_gamepads[controller_num];
    const int index = analog_index(button);
    if (index >= 0) {
        if (!pad.pressed_analog_buttons[index])
            return false;
        pad.pressed_analog_buttons[index] = false;
        return true;
    }
    const WORD mask = button_bits(button);
    if ((pad.pressed_buttons & mask) == 0)
        return false;
    pad.pressed_buttons &= static_cast<WORD>(~mask);
    return true;
}

bool controller::button_released_clear(int controller_num, ButtonIndex button)
{
    XBGAMEPAD& pad = s_gamepads[controller_num];
    const int index = analog_index(button);
    if (index >= 0) {
        if (!pad.released_analog_buttons[index])
            return false;
        pad.released_analog_buttons[index] = false;
        return true;
    }
    const WORD mask = button_bits(button);
    if ((pad.released_buttons & mask) == 0)
        return false;
    pad.released_buttons &= static_cast<WORD>(~mask);
    return true;
}

void controller::button_pressed_clear_all(int controller_num)
{
    for (int button = LEFTBUTTON; button < 16; ++button)
        button_pressed_clear(controller_num, static_cast<ButtonIndex>(button));
}

void controller::button_released_clear_all(int controller_num)
{
    for (int button = LEFTBUTTON; button <= SELECT; ++button)
        button_released_clear(controller_num, static_cast<ButtonIndex>(button));
}

void controller::button_pressed_clear_all()
{
    if (is_locked) {
        button_pressed_clear_all(locked_port);
        return;
    }
    for (int i = 0; i < num_controllers; ++i)
        button_pressed_clear_all(i);
}

int controller::button_value(ButtonIndex button, int* controller_num)
{
    if (is_locked) {
        if (controller_num != nullptr)
            *controller_num = locked_port;
        return button_value(locked_port, button);
    }
    int best = 0;
    for (int i = 0; i < num_controllers; ++i) {
        if (!accepting_input_from_controller[i])
            continue;
        const int value = button_value(i, button);
        if (value > best) {
            best = value;
            if (controller_num != nullptr)
                *controller_num = i;
        }
    }
    return best;
}

bool controller::button_pressed(ButtonIndex button, int* controller_num)
{
    if (is_locked) {
        if (controller_num != nullptr)
            *controller_num = locked_port;
        return button_pressed(locked_port, button);
    }
    bool result = false;
    for (int i = 0; i < num_controllers; ++i) {
        if (!accepting_input_from_controller[i])
            continue;
        const bool value = button_pressed(i, button);
        if (value && !result && controller_num != nullptr)
            *controller_num = i;
        result = result || value;
    }
    return result;
}

bool controller::button_pressed_clear(ButtonIndex button, int* controller_num)
{
    if (is_locked) {
        if (controller_num != nullptr)
            *controller_num = locked_port;
        return button_pressed_clear(locked_port, button);
    }
    bool result = false;
    for (int i = 0; i < num_controllers; ++i) {
        if (!accepting_input_from_controller[i])
            continue;
        const bool value = button_pressed_clear(i, button);
        if (value && !result && controller_num != nullptr)
            *controller_num = i;
        result = result || value;
    }
    return result;
}

bool controller::button_released(ButtonIndex button, int* controller_num)
{
    if (is_locked) {
        if (controller_num != nullptr)
            *controller_num = locked_port;
        return button_released(locked_port, button);
    }
    bool result = false;
    for (int i = 0; i < num_controllers; ++i) {
        if (!accepting_input_from_controller[i])
            continue;
        const bool value = button_released(i, button);
        if (value && !result && controller_num != nullptr)
            *controller_num = i;
        result = result || value;
    }
    return result;
}

bool controller::button_released_clear(ButtonIndex button, int* controller_num)
{
    if (is_locked) {
        if (controller_num != nullptr)
            *controller_num = locked_port;
        return button_released_clear(locked_port, button);
    }
    bool result = false;
    for (int i = 0; i < num_controllers; ++i) {
        if (!accepting_input_from_controller[i])
            continue;
        const bool value = button_released_clear(i, button);
        if (value && !result && controller_num != nullptr)
            *controller_num = i;
        result = result || value;
    }
    return result;
}

bool controller::any_button_pressed(int* controller_num)
{
    for (ButtonIndex button = SQUARE; button <= SELECT;
         button = static_cast<ButtonIndex>(button + 1)) {
        if (button_pressed(button, controller_num))
            return true;
    }
    return false;
}

void controller::stick_value(int controller_num, StickIndex stick,
                             int* out_x, int* out_y)
{
    if (sUixHandled) {
        *out_x = 0;
        *out_y = 0;
        return;
    }
    const XBGAMEPAD& pad = s_gamepads[controller_num];
    const int x_offset = stick == LEFTSTICK ? 0x0B : 0x0F;
    const int y_offset = stick == LEFTSTICK ? 0x0D : 0x11;
    *out_x = static_cast<signed char>(pad.raw[x_offset]);
    *out_y = -static_cast<signed char>(pad.raw[y_offset]);
}

void controller::stick_value(int controller_num, StickIndex stick,
                             int& out_x, int& out_y)
{
    stick_value(controller_num, stick, &out_x, &out_y);
}

void controller::stick_value(StickIndex stick, int* out_x, int* out_y,
                             int* controller_num)
{
    if (is_locked) {
        if (controller_num != nullptr)
            *controller_num = locked_port;
        stick_value(locked_port, stick, out_x, out_y);
        return;
    }
    int largest_x = 0;
    int largest_y = 0;
    for (int i = 0; i < num_controllers; ++i) {
        if (!accepting_input_from_controller[i])
            continue;
        int x = 0;
        int y = 0;
        stick_value(i, stick, &x, &y);
        if (x * x > largest_x * largest_x) {
            largest_x = x;
            if (controller_num != nullptr)
                *controller_num = i;
        }
        if (y * y > largest_y * largest_y) {
            largest_y = y;
            if (controller_num != nullptr)
                *controller_num = i;
        }
    }
    *out_x = largest_x;
    *out_y = largest_y;
}

int controller::stick_value_x(int stick_controller, StickIndex stick)
{
    int x = 0;
    int y = 0;
    stick_value(stick_controller, stick, &x, &y);
    return x;
}

int controller::stick_value_y(int stick_controller, StickIndex stick)
{
    int x = 0;
    int y = 0;
    stick_value(stick_controller, stick, &x, &y);
    return y;
}

int controller::stick_value_x(StickIndex stick, int* controller_num)
{
    if (is_locked) {
        if (controller_num != nullptr)
            *controller_num = locked_port;
        return stick_value_x(locked_port, stick);
    }
    int largest = 0;
    for (int i = 0; i < num_controllers; ++i) {
        if (!accepting_input_from_controller[i])
            continue;
        const int value = stick_value_x(i, stick);
        if (value * value > largest * largest) {
            largest = value;
            if (controller_num != nullptr)
                *controller_num = i;
        }
    }
    return largest;
}

int controller::stick_value_y(StickIndex stick, int* controller_num)
{
    if (is_locked) {
        if (controller_num != nullptr)
            *controller_num = locked_port;
        return stick_value_y(locked_port, stick);
    }
    int largest = 0;
    for (int i = 0; i < num_controllers; ++i) {
        if (!accepting_input_from_controller[i])
            continue;
        const int value = stick_value_y(i, stick);
        if (value * value > largest * largest) {
            largest = value;
            if (controller_num != nullptr)
                *controller_num = i;
        }
    }
    return largest;
}

void controller::rumble(int controller_num, RumbleIndex motor, float intensity)
{
    XINPUT_VIBRATION& feedback = s_feedback[controller_num];
    const WORD speed = compute_motor_speed(intensity);
    if (motor == RUMBLE_RIGHT)
        feedback.wRightMotorSpeed = speed;
    else
        feedback.wLeftMotorSpeed = speed;
    if (s_gamepads[controller_num].hDevice == nullptr)
        return;
    const DWORD result = host_set_state(static_cast<unsigned int>(controller_num), &feedback);
    if (result != ERROR_SUCCESS && result != ERROR_DEVICE_NOT_CONNECTED) {
        AeAssert::gCurrentExpr = nullptr;
        AeAssert::gCurrentFile = "controller_xbox.cpp";
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        if (result == 50) {
            AeAssert::gCurrentLine = 936;
            if (!AeAssert::IsIgnored() && AeAssert::Warning("Vibration not supported for device\n"))
                __debugbreak();
        } else {
            AeAssert::gCurrentLine = 940;
            if (!AeAssert::IsIgnored() && AeAssert::Warning("XInputSetState failed, code is: %u\n", result))
                __debugbreak();
        }
    }
}

void controller::stop_all_rumble()
{
    for (int i = 0; i < MAX_CONTROLLERS; ++i) {
        s_feedback[i] = {};
        if (s_gamepads[i].hDevice != nullptr)
            host_set_state(static_cast<unsigned int>(i), &s_feedback[i]);
    }
}

void controller::set_locked_port(int controller_num)
{
    locked_port = controller_num;
    is_locked = true;
}

void controller::unlock_port() { is_locked = false; }

void controller::accept_input_from_all_controllers(bool accept)
{
    for (bool& enabled : accepting_input_from_controller)
        enabled = accept;
}

bool controller::get_is_locked() { return is_locked; }
int controller::get_locked_port() { return locked_port; }
