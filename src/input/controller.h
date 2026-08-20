// Controller ABI and input surface reconstructed from controller_xboxr/controller.o.
#pragma once

#include <stddef.h>

class controller {
public:
    enum ButtonIndex {
        LEFTBUTTON = 0,
        DOWNBUTTON = 1,
        RIGHTBUTTON = 2,
        UPBUTTON = 3,
        SQUARE = 4,
        X = 5,
        CIRCLE = 6,
        TRIANGLE = 7,
        R1 = 8,
        L1 = 9,
        R2 = 10,
        L2 = 11,
        R3 = 12,
        L3 = 13,
        START = 14,
        SELECT = 15,
    };

    enum StickIndex {
        LEFTSTICK = 0,
        RIGHTSTICK = 1,
    };

    enum RumbleIndex {
        RUMBLE_LEFT = 0,
        RUMBLE_RIGHT = 1,
        RUMBLE_COUNT = 2,
    };

    static const int MAX_CONTROLLERS = 4;
    static int num_controllers;

    controller();
    static controller* inst();

    void poll();
    bool controller_is_connected(int controller_num);
    int get_num_controllers();

    int button_value(int controller_num, ButtonIndex button);
    bool button_pressed(int controller_num, ButtonIndex button);
    bool button_pressed_clear(int controller_num, ButtonIndex button);
    void button_pressed_clear_all(int controller_num);
    void button_pressed_clear_all();
    bool button_released(int controller_num, ButtonIndex button);
    bool button_released_clear(int controller_num, ButtonIndex button);
    void button_released_clear_all(int controller_num);

    int button_value(ButtonIndex button, int* controller_num);
    bool button_pressed(ButtonIndex button, int* controller_num);
    bool button_pressed_clear(ButtonIndex button, int* controller_num);
    bool button_released(ButtonIndex button, int* controller_num);
    bool button_released_clear(ButtonIndex button, int* controller_num);
    bool any_button_pressed(int* controller_num);

    void stick_value(int controller_num, StickIndex stick, int* out_x, int* out_y);
    void stick_value(int controller_num, StickIndex stick, int& out_x, int& out_y);
    void stick_value(StickIndex stick, int* out_x, int* out_y, int* controller_num);
    int stick_value_x(int controller_num, StickIndex stick);
    int stick_value_y(int controller_num, StickIndex stick);
    int stick_value_x(StickIndex stick, int* controller_num);
    int stick_value_y(StickIndex stick, int* controller_num);

    void rumble(int controller_num, RumbleIndex motor, float intensity);
    void stop_all_rumble();

    void set_locked_port(int controller_num);
    void unlock_port();
    void accept_input_from_all_controllers(bool accept);
    bool get_is_locked();
    int get_locked_port();

    // These six callbacks are part of the verified 0x24-byte controller ABI.
    void (*button_value_fn)(int*);
    void (*button_released_fn)(int*);
    void (*button_released_clear_fn)(int*);
    void (*button_pressed_fn)(int*);
    void (*button_pressed_clear_fn)(int*);
    void (*stick_value_fn)(int*, int*);
    int locked_port;
    bool is_locked;
    bool accepting_input_from_controller[MAX_CONTROLLERS];
};

static_assert(offsetof(controller, locked_port) == 0x18,
              "controller::locked_port offset mismatch");
static_assert(offsetof(controller, is_locked) == 0x1C,
              "controller::is_locked offset mismatch");
static_assert(offsetof(controller, accepting_input_from_controller) == 0x1D,
              "controller accepting-input offset mismatch");
static_assert(sizeof(controller) == 0x24, "controller size mismatch");
