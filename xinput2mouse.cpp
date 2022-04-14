#include <Windows.h>
#include <Xinput.h>
#include <iostream>

using namespace std;

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "XInput.lib")

#define XINPUT_GAMEPAD_THUMB_MAX 32767
#define CALC_MAGNITUDE(X, Y) sqrt(X* X + Y * Y)
#define SCROLL_SPEED_UNIT 120.0
#define MOUSE_MOVE_SPEED_UNIT 50.0

// judge left thumb pulled by compare dead zone
bool JudgeThumbLPulled(XINPUT_STATE state) {
    return state.Gamepad.sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ||
           state.Gamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
}

// judge right thumb pulled by compare dead zone
bool JudgeThumbRPulled(XINPUT_STATE state) {
    return CALC_MAGNITUDE(state.Gamepad.sThumbRX, state.Gamepad.sThumbRY) >
           XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
}

void Run() {
    // find first xinput controller
    DWORD dwResult;
    DWORD xinputIdx = 0;
    XINPUT_STATE state, lastState;
    for (; xinputIdx < XUSER_MAX_COUNT; xinputIdx++) {
        ZeroMemory(&state, sizeof(XINPUT_STATE));
        dwResult = XInputGetState(xinputIdx, &state);
        if (dwResult == ERROR_SUCCESS) {
            break;
        }
    }

    int sx = 0, sy = 0, mx = 0, my = 0;
    // poll from first controller
    while (true) {
        Sleep(50);
        lastState = state;
        ZeroMemory(&state, sizeof(XINPUT_STATE));
        dwResult = XInputGetState(xinputIdx, &state);
        // check if controller is still available
        if (dwResult != ERROR_SUCCESS) {
            if (dwResult == ERROR_DEVICE_NOT_CONNECTED)
                continue;
            else
                break;
        }
        // check if there are any changes
        if (state.dwPacketNumber != lastState.dwPacketNumber) {
            // changeBtns = pressedBtns + releasedBtns
            WORD changeBtns =
                state.Gamepad.wButtons ^ lastState.Gamepad.wButtons;
            // check if button has been changed
            if (changeBtns != 0) {
                WORD pressedBtns = changeBtns & state.Gamepad.wButtons;

                if (XINPUT_GAMEPAD_DPAD_UP & changeBtns)
                    if (XINPUT_GAMEPAD_DPAD_UP & pressedBtns)
                        keybd_event(VK_PRIOR, 0, 0, 0);
                    else
                        keybd_event(VK_PRIOR, 0, KEYEVENTF_KEYUP, 0);
                if (XINPUT_GAMEPAD_DPAD_DOWN & changeBtns)
                    if (XINPUT_GAMEPAD_DPAD_DOWN & pressedBtns)
                        keybd_event(VK_NEXT, 0, 0, 0);
                    else
                        keybd_event(VK_NEXT, 0, KEYEVENTF_KEYUP, 0);
                if (XINPUT_GAMEPAD_DPAD_LEFT & changeBtns)
                    if (XINPUT_GAMEPAD_DPAD_LEFT & pressedBtns)
                        keybd_event(VK_HOME, 0, 0, 0);
                    else
                        keybd_event(VK_HOME, 0, KEYEVENTF_KEYUP, 0);
                if (XINPUT_GAMEPAD_DPAD_RIGHT & changeBtns)
                    if (XINPUT_GAMEPAD_DPAD_RIGHT & pressedBtns)
                        keybd_event(VK_END, 0, 0, 0);
                    else
                        keybd_event(VK_END, 0, KEYEVENTF_KEYUP, 0);

                if (XINPUT_GAMEPAD_LEFT_SHOULDER & changeBtns)
                    if (XINPUT_GAMEPAD_LEFT_SHOULDER & pressedBtns)
                        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                    else
                        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                if (XINPUT_GAMEPAD_RIGHT_SHOULDER & changeBtns)
                    if (XINPUT_GAMEPAD_RIGHT_SHOULDER & pressedBtns)
                        mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
                    else
                        mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
            }

            // handle left thumb moved
            sx = sy = 0;
            SHORT lx = state.Gamepad.sThumbLX;
            SHORT ly = state.Gamepad.sThumbLY;
            if (lx > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
                sx = lx - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
            if (lx < 0 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
                sx = lx + XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
            if (ly > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
                sy = ly - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
            if (ly < 0 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
                sy = ly + XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;

            // handle right thumb moved
            mx = my = 0;
            SHORT rx = state.Gamepad.sThumbRX;
            SHORT ry = state.Gamepad.sThumbRY;
            if (rx > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
                mx = rx - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
            if (rx < 0 - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
                mx = rx + XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
            if (ry > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
                my = ry - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
            if (ry < 0 - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
                my = ry + XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
        }

        // printf("sx: %6d sy:%6d mx:%6d my:%6d\n", sx, sy, mx, my);
        // scroll cursor
        if (sx != 0 || sy != 0) {
            mouse_event(MOUSEEVENTF_HWHEEL, 0, 0,
                        sx * (SCROLL_SPEED_UNIT / XINPUT_GAMEPAD_THUMB_MAX), 0);
            mouse_event(MOUSEEVENTF_WHEEL, 0, 0,
                        sy * (SCROLL_SPEED_UNIT / XINPUT_GAMEPAD_THUMB_MAX), 0);
        }
        // move cursor relatively
        if (mx != 0 || my != 0) {
            mouse_event(
                MOUSEEVENTF_MOVE,
                mx * (MOUSE_MOVE_SPEED_UNIT / XINPUT_GAMEPAD_THUMB_MAX),
                -1 * my * (MOUSE_MOVE_SPEED_UNIT / XINPUT_GAMEPAD_THUMB_MAX), 0,
                0);
        }
    }
}

int main() {
    // cout << "xinput2mouse" << endl;
    Run();

    return 0;
}