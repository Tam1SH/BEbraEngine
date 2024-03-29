﻿
#include "platform_window.hpp"
export module Input;

namespace BEbraEngine {
    
    export enum struct KeyCode {
        
        KEY_0 = SDL_SCANCODE_0,
        KEY_1 = SDL_SCANCODE_1,
        KEY_2 = SDL_SCANCODE_2,
        KEY_3 = SDL_SCANCODE_3,
        KEY_4 = SDL_SCANCODE_4,
        KEY_5 = SDL_SCANCODE_5,
        KEY_6 = SDL_SCANCODE_6,
        KEY_7 = SDL_SCANCODE_7,
        KEY_8 = SDL_SCANCODE_8,
        KEY_9 = SDL_SCANCODE_9,

        KEY_MINUS = SDL_SCANCODE_MINUS,
        KEY_EQUAL = SDL_SCANCODE_EQUALS,
        KEY_BACKSPACE = SDL_SCANCODE_BACKSPACE,
        KEY_Q = SDL_SCANCODE_Q,
        KEY_W = SDL_SCANCODE_W,
        KEY_E = SDL_SCANCODE_E,
        KEY_R = SDL_SCANCODE_R,
        KEY_T = SDL_SCANCODE_T,
        KEY_Y = SDL_SCANCODE_Y,
        KEY_U = SDL_SCANCODE_U,
        KEY_I = SDL_SCANCODE_I,
        KEY_O = SDL_SCANCODE_O,
        KEY_P = SDL_SCANCODE_P,
        KEY_LEFT_BRACKET = SDL_SCANCODE_LEFTBRACKET,
        KEY_RIGHT_BRACKET = SDL_SCANCODE_RIGHTBRACKET,
        KEY_A = SDL_SCANCODE_A,
        KEY_S = SDL_SCANCODE_S,
        KEY_D = SDL_SCANCODE_D,
        KEY_F = SDL_SCANCODE_F,
        KEY_G = SDL_SCANCODE_G,
        KEY_H = SDL_SCANCODE_H,
        KEY_J = SDL_SCANCODE_J,
        KEY_K = SDL_SCANCODE_K,
        KEY_L = SDL_SCANCODE_L,
        KEY_SEMICOLON = SDL_SCANCODE_SEMICOLON,
        KEY_BACKSLASH = SDL_SCANCODE_BACKSLASH,
        KEY_Z = SDL_SCANCODE_Z,
        KEY_X = SDL_SCANCODE_X,
        KEY_C = SDL_SCANCODE_C,
        KEY_V = SDL_SCANCODE_V,
        KEY_B = SDL_SCANCODE_B,
        KEY_N = SDL_SCANCODE_N,
        KEY_M = SDL_SCANCODE_M,
        KEY_COMMA = SDL_SCANCODE_COMMA,
        KEY_PERIOD = SDL_SCANCODE_PERIOD,
        KEY_SLASH = SDL_SCANCODE_SLASH,
        KEY_CAPS_LOCK = SDL_SCANCODE_CAPSLOCK,
        KEY_TAB = SDL_SCANCODE_TAB,
        KEY_LEFT_SHIFT = SDL_SCANCODE_LSHIFT,
        KEY_ENTER = SDL_SCANCODE_KP_ENTER,
        KEY_LEFT_CONTROL = SDL_SCANCODE_LCTRL,
        KEY_RIGHT_CONTROL = SDL_SCANCODE_RCTRL,
        KEY_LEFT_ALT = SDL_SCANCODE_LALT,
        KEY_RIGHT_ALT = SDL_SCANCODE_RALT,
        KEY_F1 = SDL_SCANCODE_F1,
        KEY_F2 = SDL_SCANCODE_F2,
        KEY_F3 = SDL_SCANCODE_F3,
        KEY_F4 = SDL_SCANCODE_F4,
        KEY_F5 = SDL_SCANCODE_F5,
        KEY_F6 = SDL_SCANCODE_F6,
        KEY_F7 = SDL_SCANCODE_F7,
        KEY_F8 = SDL_SCANCODE_F8,
        KEY_F9 = SDL_SCANCODE_F9,
        KEY_F10 = SDL_SCANCODE_F10,
        KEY_F11 = SDL_SCANCODE_F11,
        KEY_F12 = SDL_SCANCODE_F12,
        KEY_SPACE = SDL_SCANCODE_SPACE,
        KEY_RIGHT_SHIFT = SDL_SCANCODE_RSHIFT
    };
    

    export struct Input {
        struct InputState {
            float x{}, y{};
            float scrollX{}, scrollY{};

        };
    public:
        friend struct Window;


        static void setKeyBoardState(const Uint8* state)
        {
            Input::state = state;
        }

        static float getScrollX()
        {
            return *Input::scrollX;
        }

        static float getScrollY()
        {
            return *Input::scrollY;
        }

        static float getX()
        {
            return *Input::mouseX;
        }

        static float getY()
        {
            return *Input::mouseY;
        }

        static float getOffsetX()
        {
            return *Input::mouseX;
        }

        static float getOffsetY()
        {
            return *Input::mouseY;
        }

        static bool isKeyPressed(KeyCode key)
        {
            if (Input::state[(int)key])
                return true;
            return false;
        }

        static bool isKeyReleased(KeyCode key)
        {
            if (Input::state[(int)key])
                return true;
            return false;
        }

        static void setWindow(WindowHandle* window)
        {
            Input::state = SDL_GetKeyboardState(NULL);
            Input::oldState = SDL_GetKeyboardState(NULL);
            Input::window = window;
        }

        Input() {}

        ~Input() {}
    private:

        static void update(InputState& state)
        {
            *scrollX = state.scrollX;
            *scrollY = state.scrollY;

            *mouseX = state.x;
            *mouseY = state.y;
        }

        static float* mouseX;

        static float* mouseY;

        static float* lastX;
        
        static float* lastY;

        static const Uint8* state;

        static const Uint8* oldState;

        static float* scrollX;

        static float* scrollY;

        static WindowHandle* window;

    };

    float* Input::mouseX = new float();

    float* Input::lastX = new float();

    float* Input::lastY = new float();

    const uint8_t* Input::state;

    const uint8_t* Input::oldState;

    float* Input::mouseY = new float();

    float* Input::scrollX = new float();

    float* Input::scrollY = new float();

    SDL_Window* Input::window;

 

}
