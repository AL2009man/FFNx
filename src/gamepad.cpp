/****************************************************************************/
//    Copyright (C) 2009 Aali132                                            //
//    Copyright (C) 2018 quantumpencil                                      //
//    Copyright (C) 2018 Maxime Bacoux                                      //
//    Copyright (C) 2020 myst6re                                            //
//    Copyright (C) 2020 Chris Rizzitello                                   //
//    Copyright (C) 2020 John Pritchard                                     //
//    Copyright (C) 2026 Julian Xhokaxhiu                                   //
//                                                                          //
//    This file is part of FFNx                                             //
//                                                                          //
//    FFNx is free software: you can redistribute it and/or modify          //
//    it under the terms of the GNU General Public License as published by  //
//    the Free Software Foundation, either version 3 of the License         //
//                                                                          //
//    FFNx is distributed in the hope that it will be useful,               //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of        //
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         //
//    GNU General Public License for more details.                          //
/****************************************************************************/

#include <cmath>
#include <SDL3/SDL.h>

#include "log.h"
#include "gamepad.h"

Gamepad gamepad;

Gamepad::Gamepad()
    : cId(-1), deadzoneX(0.05f), deadzoneY(0.02f), sdlGamepad(nullptr), sdlInstanceId(-1), sdlInitialized(false),
      leftStickX(0.0f), leftStickY(0.0f), rightStickX(0.0f), rightStickY(0.0f), leftTrigger(0.0f), rightTrigger(0.0f)
{
}

Gamepad::Gamepad(float dzX, float dzY)
    : cId(-1), deadzoneX(dzX), deadzoneY(dzY), sdlGamepad(nullptr), sdlInstanceId(-1), sdlInitialized(false),
      leftStickX(0.0f), leftStickY(0.0f), rightStickX(0.0f), rightStickY(0.0f), leftTrigger(0.0f), rightTrigger(0.0f)
{
}

Gamepad::~Gamepad()
{
    closeGamepad();
    if (sdlInitialized)
    {
        SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
        sdlInitialized = false;
    }
}

int Gamepad::GetPort() const
{
    return sdlGamepad ? 1 : 0;
}

GamepadInput* Gamepad::GetState()
{
    return &state.Gamepad;
}

const GamepadVibration &Gamepad::GetVibrationState() const
{
    return vibration;
}

bool Gamepad::Gamepad_Init()
{
    if (sdlInitialized)
        return true;

    if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD))
    {
        if (trace_all || trace_gamepad)
            ffnx_trace("Gamepad: SDL_InitSubSystem(SDL_INIT_GAMEPAD) failed: %s\n", SDL_GetError());
        return false;
    }

    SDL_SetGamepadEventsEnabled(true);

    int mapped = SDL_AddGamepadMappingsFromFile("SDL_GameControllerDB.txt");
    if (mapped > 0)
    {
        if (trace_all || trace_gamepad)
            ffnx_trace("Gamepad: loaded %d gamepad mappings from SDL_GameControllerDB\n", mapped);
    }
    else if (mapped < 0)
    {
        if (trace_all || trace_gamepad)
            ffnx_trace("Gamepad: SDL_GameControllerDB mapping files failed: %s\n", SDL_GetError());
    }

    sdlInitialized = true;

    if (trace_all || trace_gamepad)
        ffnx_trace("Gamepad: initialized gamepad subsystem\n");

    return true;
}

void Gamepad::GetDeviceName(SDL_Gamepad *gp, SDL_JoystickID id)
{
    sdlGamepad = gp;
    sdlInstanceId = id;
    cId = 0;
    const char *name = SDL_GetGamepadName(gp);
    controllerName = name ? name : "";
    controllerType = SDL_GetGamepadType(gp);
    if (trace_all || trace_gamepad)
        ffnx_trace("Gamepad connected: %s (%s)\n", controllerName.c_str(), GetTypeGlyphSet());
}

void Gamepad::handleSDLEvents()
{
    // Pump OS events into the SDL queue once, then peek only at gamepad events.
    // This avoids consuming unrelated events (window, keyboard, mouse) that the
    // rest of the engine still needs to process.
    SDL_PumpEvents();

    SDL_Event events[8];
    int count;
    while ((count = SDL_PeepEvents(events, SDL_arraysize(events), SDL_GETEVENT,
                                   SDL_EVENT_GAMEPAD_AXIS_MOTION,
                                   SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED)) > 0)
    {
        for (int i = 0; i < count; ++i)
        {
            const SDL_Event &event = events[i];
            switch (event.type)
            {
                case SDL_EVENT_GAMEPAD_ADDED:
                    if (!sdlGamepad)
                    {
                        SDL_JoystickID id = event.gdevice.which;
                        SDL_Gamepad *gp = SDL_OpenGamepad(id);
                        if (gp)
                            GetDeviceName(gp, id);
                    }
                    break;

                case SDL_EVENT_GAMEPAD_REMOVED:
                    if (sdlGamepad && event.gdevice.which == sdlInstanceId)
                    {
                        if (trace_all || trace_gamepad)
                            ffnx_trace("Gamepad disconnected: %s\n", controllerName.empty() ? "unknown" : controllerName.c_str());
                        closeGamepad();
                    }
                    break;

                case SDL_EVENT_GAMEPAD_REMAPPED:
                    if (sdlGamepad && event.gdevice.which == sdlInstanceId)
                    {
                        if (trace_all || trace_gamepad)
                            ffnx_trace("Gamepad remapped: %s\n", controllerName.c_str());
                    }
                    break;

                default:
                    break;
            }
        }
    }
}

bool Gamepad::openGamepad()
{
    if (!Gamepad_Init())
        return false;

    int count = 0;
    SDL_JoystickID *ids = SDL_GetGamepads(&count);

    if (!ids || count == 0)
    {
        if (trace_all || trace_gamepad)
            ffnx_trace("Gamepad: no gamepads, count=%d\n", count);
        SDL_free(ids);
        return false;
    }

    for (int i = 0; i < count; ++i)
    {
        SDL_Gamepad *gp = SDL_OpenGamepad(ids[i]);
        if (!gp)
        {
            if (trace_all || trace_gamepad)
                ffnx_trace("Gamepad: SDL_OpenGamepad failed: %s\n", SDL_GetError());
            continue;
        }

        GetDeviceName(gp, ids[i]);
        break;
    }

    SDL_free(ids);
    return sdlGamepad != nullptr;
}

void Gamepad::closeGamepad()
{
    if (sdlGamepad)
    {
        SDL_CloseGamepad(sdlGamepad);
        sdlGamepad = nullptr;
    }

    sdlInstanceId = -1;
    cId = -1;
    controllerName.clear();
    controllerType = SDL_GAMEPAD_TYPE_UNKNOWN;
    ZeroMemory(&state, sizeof(state));
    leftStickX = leftStickY = rightStickX = rightStickY = 0.0f;
    leftTrigger = rightTrigger = 0.0f;
}

SDL_GamepadType Gamepad::GetType() const
{
    return controllerType;
}

const char* Gamepad::GetTypeGlyphSet() const
{
    switch (controllerType)
    {
        case SDL_GAMEPAD_TYPE_XBOX360:
        case SDL_GAMEPAD_TYPE_XBOXONE:
            return "Xbox";
        case SDL_GAMEPAD_TYPE_PS3:
        case SDL_GAMEPAD_TYPE_PS4:
        case SDL_GAMEPAD_TYPE_PS5:
            return "PlayStation";
        case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO:
        case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_LEFT:
        case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT:
        case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR:
            return "Nintendo";
        default:
            return "Generic";
    }
}

SDL_GamepadButtonLabel Gamepad::GetButtonLabel(SDL_GamepadButton button) const
{
    if (sdlGamepad)
        return SDL_GetGamepadButtonLabel(sdlGamepad, button);
    SDL_GamepadType fallback = (controllerType != SDL_GAMEPAD_TYPE_UNKNOWN)
        ? controllerType : SDL_GAMEPAD_TYPE_STANDARD;
    return SDL_GetGamepadButtonLabelForType(fallback, button);
}

const char* Gamepad::GetButtonLabelName(SDL_GamepadButtonLabel label) const
{
    switch (label)
    {
        case SDL_GAMEPAD_BUTTON_LABEL_A:        return "A";
        case SDL_GAMEPAD_BUTTON_LABEL_B:        return "B";
        case SDL_GAMEPAD_BUTTON_LABEL_X:        return "X";
        case SDL_GAMEPAD_BUTTON_LABEL_Y:        return "Y";
        case SDL_GAMEPAD_BUTTON_LABEL_CROSS:    return "Cross";
        case SDL_GAMEPAD_BUTTON_LABEL_CIRCLE:   return "Circle";
        case SDL_GAMEPAD_BUTTON_LABEL_SQUARE:   return "Square";
        case SDL_GAMEPAD_BUTTON_LABEL_TRIANGLE: return "Triangle";
        default:                                return "Unknown";
    }
}

const char* Gamepad::GetButtonPromptLabel(const char* actionToken) const
{
    if (!actionToken) return "Unknown";

    // --- Face buttons: resolved per-controller via SDL_GetGamepadButtonLabel ---
    struct FaceAction { const char* token; SDL_GamepadButton button; };
    static const FaceAction faceActions[] = {
        { "Confirm", SDL_GAMEPAD_BUTTON_SOUTH },
        { "Cancel",  SDL_GAMEPAD_BUTTON_EAST  },
        { "Menu",    SDL_GAMEPAD_BUTTON_NORTH },
        { "Assist",  SDL_GAMEPAD_BUTTON_WEST  },
        // Raw face-button names (pass-through queries)
        { "South",   SDL_GAMEPAD_BUTTON_SOUTH },
        { "East",    SDL_GAMEPAD_BUTTON_EAST  },
        { "North",   SDL_GAMEPAD_BUTTON_NORTH },
        { "West",    SDL_GAMEPAD_BUTTON_WEST  },
    };
    for (const auto& fa : faceActions)
    {
        if (SDL_strcasecmp(actionToken, fa.token) == 0)
            return GetButtonLabelName(GetButtonLabel(fa.button));
    }

    // --- Non-face buttons: derive platform from what SDL actually reports
    SDL_GamepadButtonLabel southLabel = GetButtonLabel(SDL_GAMEPAD_BUTTON_SOUTH);
    const bool isPS     = (southLabel == SDL_GAMEPAD_BUTTON_LABEL_CROSS);
    const bool isSwitch = (southLabel == SDL_GAMEPAD_BUTTON_LABEL_B);

    // Shoulders
    if (SDL_strcasecmp(actionToken, "L1") == 0 || SDL_strcasecmp(actionToken, "LeftShoulder") == 0 || SDL_strcasecmp(actionToken, "LB") == 0)
        return isSwitch ? "L"  : (isPS ? "L1" : "LB");
    if (SDL_strcasecmp(actionToken, "R1") == 0 || SDL_strcasecmp(actionToken, "RightShoulder") == 0 || SDL_strcasecmp(actionToken, "RB") == 0)
        return isSwitch ? "R"  : (isPS ? "R1" : "RB");

    // Triggers
    if (SDL_strcasecmp(actionToken, "L2") == 0 || SDL_strcasecmp(actionToken, "LeftTrigger") == 0 || SDL_strcasecmp(actionToken, "LT") == 0)
        return isSwitch ? "ZL" : (isPS ? "L2" : "LT");
    if (SDL_strcasecmp(actionToken, "R2") == 0 || SDL_strcasecmp(actionToken, "RightTrigger") == 0 || SDL_strcasecmp(actionToken, "RT") == 0)
        return isSwitch ? "ZR" : (isPS ? "R2" : "RT");

    // Stick clicks
    if (SDL_strcasecmp(actionToken, "L3") == 0 || SDL_strcasecmp(actionToken, "LeftThumb") == 0 || SDL_strcasecmp(actionToken, "LeftStickClick") == 0)
        return "L3";
    if (SDL_strcasecmp(actionToken, "R3") == 0 || SDL_strcasecmp(actionToken, "RightThumb") == 0 || SDL_strcasecmp(actionToken, "RightStickClick") == 0)
        return "R3";

    // Start / Select / Guide — names vary by platform
    if (SDL_strcasecmp(actionToken, "Start") == 0 || SDL_strcasecmp(actionToken, "Options") == 0 || SDL_strcasecmp(actionToken, "Plus") == 0)
        return isPS ? "Options" : (isSwitch ? "Plus"  : "Start");
    if (SDL_strcasecmp(actionToken, "Select") == 0 || SDL_strcasecmp(actionToken, "Back") == 0 || SDL_strcasecmp(actionToken, "Share") == 0 || SDL_strcasecmp(actionToken, "Minus") == 0)
        return isPS ? "Share"   : (isSwitch ? "Minus" : "Back");
    if (SDL_strcasecmp(actionToken, "Guide") == 0 || SDL_strcasecmp(actionToken, "PS") == 0 || SDL_strcasecmp(actionToken, "Home") == 0)
        return isPS ? "PS"      : (isSwitch ? "Home"  : "Guide");

    // DPad
    if (SDL_strcasecmp(actionToken, "Up")    == 0 || SDL_strcasecmp(actionToken, "DPadUp")    == 0) return "DPadUp";
    if (SDL_strcasecmp(actionToken, "Down")  == 0 || SDL_strcasecmp(actionToken, "DPadDown")  == 0) return "DPadDown";
    if (SDL_strcasecmp(actionToken, "Left")  == 0 || SDL_strcasecmp(actionToken, "DPadLeft")  == 0) return "DPadLeft";
    if (SDL_strcasecmp(actionToken, "Right") == 0 || SDL_strcasecmp(actionToken, "DPadRight") == 0) return "DPadRight";
    if (SDL_strcasecmp(actionToken, "Directional Buttons") == 0 || SDL_strcasecmp(actionToken, "DPad") == 0)
        return "DPad";

    // Analog stick directions / whole sticks
    if (SDL_strcasecmp(actionToken, "LeftStick")  == 0) return "LeftStick";
    if (SDL_strcasecmp(actionToken, "RightStick") == 0) return "RightStick";

    // Unrecognized token: pass through as-is so mods can use custom tokens.
    return actionToken;
}

bool Gamepad::CheckConnection()
{
    if (!Gamepad_Init())
        return false;

    handleSDLEvents();

    if (sdlGamepad)
        return true;

    if (!SDL_HasGamepad())
        return false;

    return openGamepad();
}

bool Gamepad::Refresh()
{
    if (!Gamepad_Init())
        return false;

    handleSDLEvents();

    if (!sdlGamepad)
    {
        if (!openGamepad())
            return false;
    }

    auto normAxis = [](Sint16 v) -> float { return fmaxf(-1.0f, (float)v / 32767.0f); };
    auto applyDeadzone = [](float v, float dz) -> float
    {
        if (fabsf(v) < dz) return 0.0f;
        float sign = (v < 0.0f) ? -1.0f : 1.0f;
        return ((fabsf(v) - dz) / (1.0f - dz)) * sign;
    };

    leftStickX  =  applyDeadzone(normAxis(SDL_GetGamepadAxis(sdlGamepad, SDL_GAMEPAD_AXIS_LEFTX)),  deadzoneX);
    leftStickY  = -applyDeadzone(normAxis(SDL_GetGamepadAxis(sdlGamepad, SDL_GAMEPAD_AXIS_LEFTY)),  deadzoneY);
    rightStickX =  applyDeadzone(normAxis(SDL_GetGamepadAxis(sdlGamepad, SDL_GAMEPAD_AXIS_RIGHTX)), deadzoneX);
    rightStickY = -applyDeadzone(normAxis(SDL_GetGamepadAxis(sdlGamepad, SDL_GAMEPAD_AXIS_RIGHTY)), deadzoneY);

    leftTrigger  = SDL_clamp((float)SDL_GetGamepadAxis(sdlGamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER)  / 32767.0f, 0.0f, 1.0f);
    rightTrigger = SDL_clamp((float)SDL_GetGamepadAxis(sdlGamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) / 32767.0f, 0.0f, 1.0f);

    static const WORD buttonMasks[] = {
        GAMEPAD_BUTTON_DPAD_UP, GAMEPAD_BUTTON_DPAD_DOWN, GAMEPAD_BUTTON_DPAD_LEFT,  GAMEPAD_BUTTON_DPAD_RIGHT,
        GAMEPAD_BUTTON_START,   GAMEPAD_BUTTON_BACK,      GAMEPAD_BUTTON_LEFT_THUMB, GAMEPAD_BUTTON_RIGHT_THUMB,
        GAMEPAD_BUTTON_LEFT_SHOULDER, GAMEPAD_BUTTON_RIGHT_SHOULDER,
        GAMEPAD_BUTTON_A, GAMEPAD_BUTTON_B, GAMEPAD_BUTTON_X, GAMEPAD_BUTTON_Y
    };
    static const SDL_GamepadButton sdlButtons[] = {
        SDL_GAMEPAD_BUTTON_DPAD_UP,    SDL_GAMEPAD_BUTTON_DPAD_DOWN,  SDL_GAMEPAD_BUTTON_DPAD_LEFT,  SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
        SDL_GAMEPAD_BUTTON_START,      SDL_GAMEPAD_BUTTON_BACK,       SDL_GAMEPAD_BUTTON_LEFT_STICK, SDL_GAMEPAD_BUTTON_RIGHT_STICK,
        SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,
        SDL_GAMEPAD_BUTTON_SOUTH, SDL_GAMEPAD_BUTTON_EAST, SDL_GAMEPAD_BUTTON_WEST, SDL_GAMEPAD_BUTTON_NORTH
    };
    static_assert(sizeof(buttonMasks) == sizeof(sdlButtons) / sizeof(sdlButtons[0]) * sizeof(buttonMasks[0]),
        "buttonMasks and sdlButtons must have the same number of entries");

    WORD buttons = 0;
    for (int i = 0; i < (int)(sizeof(buttonMasks) / sizeof(buttonMasks[0])); i++)
    {
        if (SDL_GetGamepadButton(sdlGamepad, sdlButtons[i]))
            buttons |= buttonMasks[i];
    }
    if (SDL_GetGamepadButton(sdlGamepad, SDL_GAMEPAD_BUTTON_GUIDE))
        buttons |= GAMEPAD_BUTTON_GUIDE;

    ZeroMemory(&state, sizeof(GamepadState));
    state.Gamepad.sThumbLX      = SDL_GetGamepadAxis(sdlGamepad, SDL_GAMEPAD_AXIS_LEFTX);
    state.Gamepad.sThumbLY      = SDL_GetGamepadAxis(sdlGamepad, SDL_GAMEPAD_AXIS_LEFTY);
    state.Gamepad.sThumbRX      = SDL_GetGamepadAxis(sdlGamepad, SDL_GAMEPAD_AXIS_RIGHTX);
    state.Gamepad.sThumbRY      = SDL_GetGamepadAxis(sdlGamepad, SDL_GAMEPAD_AXIS_RIGHTY);
    state.Gamepad.bLeftTrigger  = (BYTE)(leftTrigger  * 255.0f);
    state.Gamepad.bRightTrigger = (BYTE)(rightTrigger * 255.0f);
    state.Gamepad.wButtons      = buttons;

    return true;
}

bool Gamepad::Vibrate(WORD wLeftMotorSpeed, WORD wRightMotorSpeed)
{
    vibration.wLeftMotorSpeed = wLeftMotorSpeed;
    vibration.wRightMotorSpeed = wRightMotorSpeed;

    if (!Gamepad_Init())
        return false;

    if (!sdlGamepad && !openGamepad())
        return false;

    // SDL requires a duration; calling each frame keeps rumble alive.
    if (!SDL_RumbleGamepad(sdlGamepad, wLeftMotorSpeed, wRightMotorSpeed, 100))
    {
        closeGamepad();
        return false;
    }

    return true;
}

bool Gamepad::IsPressed(WORD button) const
{
    return (state.Gamepad.wButtons & button) != 0;
}

bool Gamepad::IsIdle() const
{
    return !(leftStickY > 0.5f  || IsPressed(GAMEPAD_BUTTON_DPAD_UP))    &&
           !(leftStickY < -0.5f || IsPressed(GAMEPAD_BUTTON_DPAD_DOWN))  &&
           !(leftStickX < -0.5f || IsPressed(GAMEPAD_BUTTON_DPAD_LEFT))  &&
           !(leftStickX > 0.5f  || IsPressed(GAMEPAD_BUTTON_DPAD_RIGHT)) &&
           !IsPressed(GAMEPAD_BUTTON_X)              &&
           !IsPressed(GAMEPAD_BUTTON_A)              &&
           !IsPressed(GAMEPAD_BUTTON_B)              &&
           !IsPressed(GAMEPAD_BUTTON_Y)              &&
           !IsPressed(GAMEPAD_BUTTON_LEFT_SHOULDER)  &&
           !IsPressed(GAMEPAD_BUTTON_RIGHT_SHOULDER) &&
           !(leftTrigger  > 0.85f)                  &&
           !(rightTrigger > 0.85f)                  &&
           !IsPressed(GAMEPAD_BUTTON_BACK)           &&
           !IsPressed(GAMEPAD_BUTTON_START)          &&
           !IsPressed(GAMEPAD_BUTTON_LEFT_THUMB)     &&
           !IsPressed(GAMEPAD_BUTTON_RIGHT_THUMB)    &&
           !IsPressed(GAMEPAD_BUTTON_GUIDE);
}
