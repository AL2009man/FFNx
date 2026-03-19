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

    if ((SDL_WasInit(SDL_INIT_GAMEPAD) & SDL_INIT_GAMEPAD) == 0)
    {
        if (SDL_Init(SDL_INIT_GAMEPAD) != 0)
        {
            if (trace_all || trace_gamepad)
                ffnx_trace("Gamepad: SDL_Init(SDL_INIT_GAMEPAD) failed: %s\n", SDL_GetError());
            return false;
        }
    }

    SDL_SetGamepadEventsEnabled(true);

    int mapped = SDL_AddGamepadMappingsFromFile("SDL_GameControllerDB.txt");
    if (mapped > 0)
    {
        if (trace_all || trace_gamepad)
            ffnx_trace("loaded %d gamepad mappings from SDL_GameControllerDB\n", mapped);
    }
    else if (mapped < 0)
    {
        if (trace_all || trace_gamepad)
            ffnx_trace("SDL_GameControllerDB mapping files failed: %s\n", SDL_GetError());
    }

    sdlInitialized = true;

    if (trace_all || trace_gamepad)
        ffnx_trace("Gamepad: initialized gamepad subsystem\n");

    return true;
}

void Gamepad::handleSDLEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_EVENT_GAMEPAD_ADDED:
                if (!sdlGamepad)
                {
                    SDL_JoystickID id = event.gdevice.which;
                    SDL_Gamepad *gp = SDL_OpenGamepad(id);
                    if (gp)
                    {
                        sdlGamepad = gp;
                        sdlInstanceId = id;
                        cId = 0;
                        if (trace_all || trace_gamepad)
                            ffnx_trace("Gamepad:handleSDLEvents: added gamepad %lld\n", (long long)id);
                    }
                }
                break;

            case SDL_EVENT_GAMEPAD_REMOVED:
                if (sdlGamepad && event.gdevice.which == sdlInstanceId)
                {
                    if (trace_all || trace_gamepad)
                        ffnx_trace("Gamepad:handleSDLEvents: removed gamepad %lld\n", (long long)event.gdevice.which);
                    closeGamepad();
                }
                break;

            default:
                break;
        }
    }
}

bool Gamepad::openFirstAvailableGamepad()
{
    if (!Gamepad_Init())
        return false;

    int count = 0;
    SDL_JoystickID *ids = SDL_GetGamepads(&count);

    if (!ids || count == 0)
    {
        if (trace_all || trace_gamepad)
            ffnx_trace("Gamepad:openFirstAvailableGamepad: no gamepads, count=%d\n", count);
        SDL_free(ids);
        return false;
    }

    if (trace_all || trace_gamepad)
        ffnx_trace("Gamepad:openFirstAvailableGamepad: found %d ids\n", count);

    for (int i = 0; i < count; ++i)
    {
        if (!SDL_IsGamepad(ids[i]))
        {
            if (trace_all || trace_gamepad)
                ffnx_trace("Gamepad:openFirstAvailableGamepad: id %lld is not a gamepad\n", (long long)ids[i]);
            continue;
        }

        SDL_Gamepad *gp = SDL_OpenGamepad(ids[i]);
        if (!gp)
        {
            if (trace_all || trace_gamepad)
                ffnx_trace("Gamepad:openFirstAvailableGamepad: SDL_OpenGamepad(%lld) failed: %s\n", (long long)ids[i], SDL_GetError());
            continue;
        }

        sdlGamepad = gp;
        sdlInstanceId = ids[i];
        cId = 0;

        if (trace_all || trace_gamepad)
            ffnx_trace("Gamepad:openFirstAvailableGamepad: opened gamepad %lld\n", (long long)ids[i]);

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
    ZeroMemory(&state, sizeof(state));
    leftStickX = leftStickY = rightStickX = rightStickY = 0.0f;
    leftTrigger = rightTrigger = 0.0f;
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

    return openFirstAvailableGamepad();
}

bool Gamepad::Refresh()
{
    if (!Gamepad_Init())
        return false;

    handleSDLEvents();

    if (!sdlGamepad)
    {
        if (!openFirstAvailableGamepad())
            return false;
    }

    SDL_UpdateGamepads();

    Sint16 rawLX = SDL_GetGamepadAxis(sdlGamepad, SDL_GAMEPAD_AXIS_LEFTX);
    Sint16 rawLY = SDL_GetGamepadAxis(sdlGamepad, SDL_GAMEPAD_AXIS_LEFTY);
    Sint16 rawRX = SDL_GetGamepadAxis(sdlGamepad, SDL_GAMEPAD_AXIS_RIGHTX);
    Sint16 rawRY = SDL_GetGamepadAxis(sdlGamepad, SDL_GAMEPAD_AXIS_RIGHTY);
    Sint16 rawLT = SDL_GetGamepadAxis(sdlGamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
    Sint16 rawRT = SDL_GetGamepadAxis(sdlGamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);

    auto normAxis = [](Sint16 v) -> float { return fmaxf(-1.0f, (float)v / 32767.0f); };
    auto applyDeadzone = [&](float v, float dz) -> float
    {
        if (fabsf(v) < dz)
            return 0.0f;
        float sign = (v < 0.0f) ? -1.0f : 1.0f;
        float absVal = fabsf(v);
        float out = (absVal - dz) * sign;
        if (dz > 0.0f) out *= 1.0f / (1.0f - dz);
        return out;
    };

    float normLX = normAxis(rawLX);
    float normLY = normAxis(rawLY);
    float normRX = normAxis(rawRX);
    float normRY = normAxis(rawRY);

    leftStickX = applyDeadzone(normLX, deadzoneX);
    leftStickY = -applyDeadzone(normLY, deadzoneY);
    rightStickX = applyDeadzone(normRX, deadzoneX);
    rightStickY = -applyDeadzone(normRY, deadzoneY);

    leftTrigger = (float)rawLT / 32767.0f;
    rightTrigger = (float)rawRT / 32767.0f;

    float leftTriggerClamped = leftTrigger < 0.0f ? 0.0f : (leftTrigger > 1.0f ? 1.0f : leftTrigger);
    float rightTriggerClamped = rightTrigger < 0.0f ? 0.0f : (rightTrigger > 1.0f ? 1.0f : rightTrigger);

    ZeroMemory(&state, sizeof(GamepadState));
    state.Gamepad.sThumbLX = rawLX;
    state.Gamepad.sThumbLY = rawLY;
    state.Gamepad.sThumbRX = rawRX;
    state.Gamepad.sThumbRY = rawRY;
    state.Gamepad.bLeftTrigger = (BYTE) (leftTriggerClamped * 255.0f);
    state.Gamepad.bRightTrigger = (BYTE) (rightTriggerClamped * 255.0f);

    WORD buttons = 0;
    const WORD buttonMasks[] = {
        GAMEPAD_BUTTON_DPAD_UP, GAMEPAD_BUTTON_DPAD_DOWN, GAMEPAD_BUTTON_DPAD_LEFT, GAMEPAD_BUTTON_DPAD_RIGHT,
        GAMEPAD_BUTTON_START, GAMEPAD_BUTTON_BACK, GAMEPAD_BUTTON_LEFT_THUMB, GAMEPAD_BUTTON_RIGHT_THUMB,
        GAMEPAD_BUTTON_LEFT_SHOULDER, GAMEPAD_BUTTON_RIGHT_SHOULDER, GAMEPAD_BUTTON_A, GAMEPAD_BUTTON_B,
        GAMEPAD_BUTTON_X, GAMEPAD_BUTTON_Y
    };

    const SDL_GamepadButton sdlButtons[] = {
        SDL_GAMEPAD_BUTTON_DPAD_UP, SDL_GAMEPAD_BUTTON_DPAD_DOWN, SDL_GAMEPAD_BUTTON_DPAD_LEFT, SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
        SDL_GAMEPAD_BUTTON_START, SDL_GAMEPAD_BUTTON_BACK, SDL_GAMEPAD_BUTTON_LEFT_STICK, SDL_GAMEPAD_BUTTON_RIGHT_STICK,
        SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, SDL_GAMEPAD_BUTTON_SOUTH, SDL_GAMEPAD_BUTTON_EAST,
        SDL_GAMEPAD_BUTTON_WEST, SDL_GAMEPAD_BUTTON_NORTH
    };

    const int buttonCount = sizeof(buttonMasks) / sizeof(buttonMasks[0]);
    for (int i = 0; i < buttonCount; i++)
    {
        if (SDL_GetGamepadButton(sdlGamepad, sdlButtons[i]))
            buttons |= buttonMasks[i];
    }

    if (SDL_GetGamepadButton(sdlGamepad, SDL_GAMEPAD_BUTTON_GUIDE))
        buttons |= 0x400;

    state.Gamepad.wButtons = buttons;

    return true;
}

bool Gamepad::Vibrate(WORD wLeftMotorSpeed, WORD wRightMotorSpeed)
{
    vibration.wLeftMotorSpeed = wLeftMotorSpeed;
    vibration.wRightMotorSpeed = wRightMotorSpeed;

    if (!Gamepad_Init())
        return false;

    if (!sdlGamepad && !openFirstAvailableGamepad())
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
  return  !(gamepad.leftStickY > 0.5f || gamepad.IsPressed(GAMEPAD_BUTTON_DPAD_UP)) &&
          !(gamepad.leftStickY < -0.5f || gamepad.IsPressed(GAMEPAD_BUTTON_DPAD_DOWN)) &&
          !(gamepad.leftStickX < -0.5f || gamepad.IsPressed(GAMEPAD_BUTTON_DPAD_LEFT)) &&
          !(gamepad.leftStickX > 0.5f || gamepad.IsPressed(GAMEPAD_BUTTON_DPAD_RIGHT)) &&
          !gamepad.IsPressed(GAMEPAD_BUTTON_X) &&
          !gamepad.IsPressed(GAMEPAD_BUTTON_A) &&
          !gamepad.IsPressed(GAMEPAD_BUTTON_B) &&
          !gamepad.IsPressed(GAMEPAD_BUTTON_Y) &&
          !gamepad.IsPressed(GAMEPAD_BUTTON_LEFT_SHOULDER)&&
          !gamepad.IsPressed(GAMEPAD_BUTTON_RIGHT_SHOULDER) &&
          !(gamepad.leftTrigger > 0.85f) &&
          !(gamepad.rightTrigger > 0.85f) &&
          !gamepad.IsPressed(GAMEPAD_BUTTON_BACK) &&
          !gamepad.IsPressed(GAMEPAD_BUTTON_START) &&
          !gamepad.IsPressed(GAMEPAD_BUTTON_START) &&
          !gamepad.IsPressed(GAMEPAD_BUTTON_LEFT_THUMB) &&
          !gamepad.IsPressed(GAMEPAD_BUTTON_RIGHT_THUMB) &&
          !gamepad.IsPressed(0x400);
}
