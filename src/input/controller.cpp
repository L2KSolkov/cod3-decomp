// ============================================================================
// Controller — Xbox gamepad input abstraction
// Source: controller_xbox.o + controller.o (25 funcs)
// ea: 0x7E1AA0-0x7E31A0
// Maps Xbox XInput to Win32 XInput / keyboard fallback.
// ============================================================================

#include <cstring>

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #include <xinput.h>
#else
  #include <cstdint>
  #include <cstring>
  typedef uint32_t DWORD;
  typedef uint16_t WORD;
  typedef int16_t  SHORT;
  typedef uint8_t  BYTE;
  struct XINPUT_STATE { DWORD dwPacketNumber; struct { WORD wButtons; BYTE bLeftTrigger; BYTE bRightTrigger; SHORT sThumbLX, sThumbLY, sThumbRX, sThumbRY; } Gamepad; };
  struct XINPUT_VIBRATION { WORD wLeftMotorSpeed; WORD wRightMotorSpeed; };
  static DWORD XInputGetState(DWORD, XINPUT_STATE*) { return 1; }
  static DWORD XInputSetState(DWORD, XINPUT_VIBRATION*) { return 1; }
#endif

class controller {
public:
    // Values match the binary's controller::ButtonIndex (verified against the
    // button_value switch at 0x7E2050 and kPadAliasButtonIndexDesc order).
    enum ButtonIndex {
        LEFTBUTTON  = 0,
        DOWNBUTTON  = 1,
        RIGHTBUTTON = 2,
        UPBUTTON    = 3,
        SQUARE      = 4,    // X button
        X           = 5,    // A button
        CIRCLE      = 6,    // B button
        TRIANGLE    = 7,    // Y button
        R1          = 8,    // right trigger
        L1          = 9,    // left trigger
        R2          = 10,   // black
        L2          = 11,   // white
        R3          = 12,   // right stick click
        L3          = 13,   // left stick click
        START       = 14,   // start button
        SELECT      = 15,   // back button
    };

    enum StickIndex {
        LEFTSTICK  = 0,
        RIGHTSTICK = 1,
    };

    enum RumbleIndex {
        RUMBLE_LEFT  = 0,
        RUMBLE_RIGHT = 1,
    };

    static const int MAX_CONTROLLERS = 4;

    controller();
    ~controller();

    static controller* inst();

    void poll();
    bool controller_is_connected(int index);

    int  button_value(int index, ButtonIndex btn);
    bool button_pressed(int index, ButtonIndex btn);
    bool button_pressed_clear(int index, ButtonIndex btn);
    void button_pressed_clear_all(int index);
    bool button_released(int index, ButtonIndex btn);
    bool button_released_clear(int index, ButtonIndex btn);
    bool any_button_pressed(int index);
    void stick_value(int index, StickIndex stick, int& outX, int& outY);

    void rumble(int index, RumbleIndex motor, float speed);
    void stop_all_rumble();

    int  locked_port;
    bool is_locked;

private:
    struct PadState {
        bool      connected;
        WORD      lastButtons;
        WORD      curButtons;
        BYTE      analogButtons[8];
        SHORT     thumbLX, thumbLY;
        SHORT     thumbRX, thumbRY;
        XINPUT_STATE rawState;
        DWORD     hDevice;
    };

    static PadState s_pads[MAX_CONTROLLERS];
    static bool     s_uixHandled;
    static bool     s_initialized;
    static controller* s_instance;

    void refresh_device_list();
    static WORD compute_motor_speed(float speed);
};

// ============================================================================
// Implementation
// ============================================================================

controller* controller::s_instance = nullptr;
controller::PadState controller::s_pads[MAX_CONTROLLERS] = {};
bool controller::s_uixHandled = false;
bool controller::s_initialized = false;

controller::controller()
    : locked_port(0)
    , is_locked(false)
{
    if (s_initialized) return;
    s_initialized = true;
    s_instance = this;

    for (int i = 0; i < MAX_CONTROLLERS; ++i) {
        s_pads[i].connected = false;
        s_pads[i].lastButtons = 0;
        s_pads[i].curButtons = 0;
        memset(s_pads[i].analogButtons, 0, sizeof(s_pads[i].analogButtons));
        s_pads[i].thumbLX = 0; s_pads[i].thumbLY = 0;
        s_pads[i].thumbRX = 0; s_pads[i].thumbRY = 0;
    }

    refresh_device_list();
}

controller::~controller() {}

controller* controller::inst() { return s_instance; }

void controller::refresh_device_list() {
    for (int i = 0; i < MAX_CONTROLLERS; ++i) {
        XINPUT_STATE state;
        DWORD result = XInputGetState(i, &state);

        s_pads[i].connected = (result == 0); // ERROR_SUCCESS
        if (s_pads[i].connected) {
            s_pads[i].curButtons = state.Gamepad.wButtons;
            s_pads[i].thumbLX = state.Gamepad.sThumbLX;
            s_pads[i].thumbLY = state.Gamepad.sThumbLY;
            s_pads[i].thumbRX = state.Gamepad.sThumbRX;
            s_pads[i].thumbRY = state.Gamepad.sThumbRY;

            // Analog button thresholds (matching Xbox behavior)
            bool isTrigger = false;
            for (int j = 0; j < 8; ++j) {
                BYTE val = *(&state.Gamepad.bLeftTrigger + j);
                BYTE threshold = (j == 6 || j == 7) ? 0x80 : 0x1E;
                s_pads[i].analogButtons[j] = val;
            }
        } else {
            memset(&s_pads[i].curButtons, 0, sizeof(WORD) + 8 + 4 * sizeof(SHORT));
        }
    }
}

void controller::poll() {
    for (int i = 0; i < MAX_CONTROLLERS; ++i) {
        s_pads[i].lastButtons = s_pads[i].curButtons;
    }
    refresh_device_list();
}

bool controller::controller_is_connected(int index) {
    return s_pads[index].connected;
}

// Button mapping: Xbox XInput to PS2-style button indices
static WORD btnToXInput(controller::ButtonIndex btn) {
    switch (btn) {
        case controller::LEFTBUTTON:   return 0x0004; // XINPUT_GAMEPAD_DPAD_LEFT
        case controller::DOWNBUTTON:   return 0x0002; // XINPUT_GAMEPAD_DPAD_DOWN
        case controller::RIGHTBUTTON:  return 0x0008; // XINPUT_GAMEPAD_DPAD_RIGHT
        case controller::UPBUTTON:     return 0x0001; // XINPUT_GAMEPAD_DPAD_UP
        case controller::START:        return 0x0010; // XINPUT_GAMEPAD_START
        case controller::SELECT:       return 0x0020; // XINPUT_GAMEPAD_BACK
        case controller::L1:           return 0x0100; // XINPUT_GAMEPAD_LEFT_SHOULDER
        case controller::R1:           return 0x0200; // XINPUT_GAMEPAD_RIGHT_SHOULDER
        case controller::L3:           return 0x0040; // XINPUT_GAMEPAD_LEFT_THUMB
        case controller::R3:           return 0x0080; // XINPUT_GAMEPAD_RIGHT_THUMB
        default: return 0;
    }
}

int controller::button_value(int index, ButtonIndex btn) {
    if (s_uixHandled || !s_pads[index].connected) return 0;

    switch (btn) {
        case SQUARE:    return s_pads[index].analogButtons[2];  // X
        case X:         return s_pads[index].analogButtons[0];  // A
        case CIRCLE:    return s_pads[index].analogButtons[1];  // B
        case TRIANGLE:  return s_pads[index].analogButtons[3];  // Y
        case L2:        return s_pads[index].analogButtons[5];  // left trigger
        case R2:        return s_pads[index].analogButtons[4];  // right trigger
        case L1:        return s_pads[index].analogButtons[6];  // left shoulder analog
        case R1:        return s_pads[index].analogButtons[7];  // right shoulder analog
        default: {
            WORD mask = btnToXInput(btn);
            return (s_pads[index].curButtons & mask) ? 1 : 0;
        }
    }
}

bool controller::button_pressed(int index, ButtonIndex btn) {
    WORD mask = btnToXInput(btn);
    bool now  = (s_pads[index].curButtons & mask) != 0;
    bool prev = (s_pads[index].lastButtons & mask) != 0;
    return now && !prev;
}

bool controller::button_pressed_clear(int index, ButtonIndex btn) {
    WORD mask = btnToXInput(btn);
    bool now = (s_pads[index].curButtons & mask) != 0;
    bool prev = (s_pads[index].lastButtons & mask) != 0;
    if (now && !prev) {
        s_pads[index].lastButtons |= mask; // mark as seen
        return true;
    }
    return false;
}

void controller::button_pressed_clear_all(int index) {
    for (int b = LEFTBUTTON; b < 16; ++b)
        button_pressed_clear(index, (ButtonIndex)b);
}

bool controller::button_released(int index, ButtonIndex btn) {
    WORD mask = btnToXInput(btn);
    bool now  = (s_pads[index].curButtons & mask) != 0;
    bool prev = (s_pads[index].lastButtons & mask) != 0;
    return !now && prev;
}

bool controller::button_released_clear(int index, ButtonIndex btn) {
    WORD mask = btnToXInput(btn);
    bool now  = (s_pads[index].curButtons & mask) != 0;
    bool prev = (s_pads[index].lastButtons & mask) != 0;
    if (!now && prev) {
        s_pads[index].lastButtons &= ~mask;
        return true;
    }
    return false;
}

bool controller::any_button_pressed(int index) {
    return s_pads[index].curButtons != s_pads[index].lastButtons;
}

void controller::stick_value(int index, StickIndex stick, int& outX, int& outY) {
    if (s_uixHandled || !s_pads[index].connected) {
        outX = 0; outY = 0;
        return;
    }

    if (stick == LEFTSTICK) {
        outX = s_pads[index].thumbLX;
        outY = -s_pads[index].thumbLY; // invert Y
    } else {
        outX = s_pads[index].thumbRX;
        outY = -s_pads[index].thumbRY;
    }
}

WORD controller::compute_motor_speed(float speed) {
    if (speed < 0.0f) speed = 0.0f;
    if (speed > 1.0f) speed = 1.0f;
    return (WORD)(speed * 65535.0f);
}

void controller::rumble(int index, RumbleIndex motor, float speed) {
    // XInputSetState with per-motor speed
    WORD leftSpeed  = (motor == RUMBLE_LEFT)  ? compute_motor_speed(speed) : 0;
    WORD rightSpeed = (motor == RUMBLE_RIGHT) ? compute_motor_speed(speed) : 0;

    XINPUT_VIBRATION vib;
    vib.wLeftMotorSpeed  = leftSpeed;
    vib.wRightMotorSpeed = rightSpeed;
    XInputSetState(index, &vib);
}

void controller::stop_all_rumble() {
    for (int i = 0; i < MAX_CONTROLLERS; ++i) {
        XINPUT_VIBRATION vib = {0, 0};
        XInputSetState(i, &vib);
    }
}
