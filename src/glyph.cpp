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

#include "glyph.h"
#include "gamepad.h"

SDL_GamepadType Glyph::currentControllerType = SDL_GAMEPAD_TYPE_UNKNOWN;

static std::string XboxButtonToLabel(SDL_GamepadButton button)
{
    switch (button)
    {
        case SDL_GAMEPAD_BUTTON_SOUTH:         return "A";
        case SDL_GAMEPAD_BUTTON_EAST:          return "B";
        case SDL_GAMEPAD_BUTTON_WEST:          return "X";
        case SDL_GAMEPAD_BUTTON_NORTH:         return "Y";
        case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: return "LB";
        case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:return "RB";
        case SDL_GAMEPAD_BUTTON_LEFT_STICK:    return "LS";
        case SDL_GAMEPAD_BUTTON_RIGHT_STICK:   return "RS";
        case SDL_GAMEPAD_BUTTON_BACK:          return "View";
        case SDL_GAMEPAD_BUTTON_START:         return "Menu";
        case SDL_GAMEPAD_BUTTON_GUIDE:         return "Xbox";
        case SDL_GAMEPAD_BUTTON_DPAD_UP:       return "DPad Up";
        case SDL_GAMEPAD_BUTTON_DPAD_DOWN:     return "DPad Down";
        case SDL_GAMEPAD_BUTTON_DPAD_LEFT:     return "DPad Left";
        case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:    return "DPad Right";
        default:                               return "Unknown";
    }
}

static std::string PlayStationButtonToLabel(SDL_GamepadButton button)
{
    switch (button)
    {
        case SDL_GAMEPAD_BUTTON_SOUTH:         return "Cross";
        case SDL_GAMEPAD_BUTTON_EAST:          return "Circle";
        case SDL_GAMEPAD_BUTTON_WEST:          return "Square";
        case SDL_GAMEPAD_BUTTON_NORTH:         return "Triangle";
        case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: return "L1";
        case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:return "R1";
        case SDL_GAMEPAD_BUTTON_LEFT_STICK:    return "L3";
        case SDL_GAMEPAD_BUTTON_RIGHT_STICK:   return "R3";
        case SDL_GAMEPAD_BUTTON_BACK:          return "Share";
        case SDL_GAMEPAD_BUTTON_START:         return "Options";
        case SDL_GAMEPAD_BUTTON_GUIDE:         return "PS";
        case SDL_GAMEPAD_BUTTON_DPAD_UP:       return "DPad Up";
        case SDL_GAMEPAD_BUTTON_DPAD_DOWN:     return "DPad Down";
        case SDL_GAMEPAD_BUTTON_DPAD_LEFT:     return "DPad Left";
        case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:    return "DPad Right";
        default:                               return "Unknown";
    }
}

static std::string NintendoButtonToLabel(SDL_GamepadButton button)
{
    switch (button)
    {
        case SDL_GAMEPAD_BUTTON_SOUTH:         return "B";
        case SDL_GAMEPAD_BUTTON_EAST:          return "A";
        case SDL_GAMEPAD_BUTTON_WEST:          return "Y";
        case SDL_GAMEPAD_BUTTON_NORTH:         return "X";
        case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: return "L";
        case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:return "R";
        case SDL_GAMEPAD_BUTTON_LEFT_STICK:    return "LS";
        case SDL_GAMEPAD_BUTTON_RIGHT_STICK:   return "RS";
        case SDL_GAMEPAD_BUTTON_BACK:          return "Minus";
        case SDL_GAMEPAD_BUTTON_START:         return "Plus";
        case SDL_GAMEPAD_BUTTON_GUIDE:         return "Home";
        case SDL_GAMEPAD_BUTTON_DPAD_UP:       return "DPad Up";
        case SDL_GAMEPAD_BUTTON_DPAD_DOWN:     return "DPad Down";
        case SDL_GAMEPAD_BUTTON_DPAD_LEFT:     return "DPad Left";
        case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:    return "DPad Right";
        default:                               return "Unknown";
    }
}

static bool SDLButtonFromLegacyMask(WORD legacyMask, SDL_GamepadButton &outSDLButton)
{
    switch (legacyMask)
    {
        case GAMEPAD_BUTTON_A:              outSDLButton = SDL_GAMEPAD_BUTTON_SOUTH; return true;
        case GAMEPAD_BUTTON_B:              outSDLButton = SDL_GAMEPAD_BUTTON_EAST; return true;
        case GAMEPAD_BUTTON_X:              outSDLButton = SDL_GAMEPAD_BUTTON_WEST; return true;
        case GAMEPAD_BUTTON_Y:              outSDLButton = SDL_GAMEPAD_BUTTON_NORTH; return true;
        case GAMEPAD_BUTTON_LEFT_SHOULDER:  outSDLButton = SDL_GAMEPAD_BUTTON_LEFT_SHOULDER; return true;
        case GAMEPAD_BUTTON_RIGHT_SHOULDER: outSDLButton = SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER; return true;
        case GAMEPAD_BUTTON_LEFT_THUMB:     outSDLButton = SDL_GAMEPAD_BUTTON_LEFT_STICK; return true;
        case GAMEPAD_BUTTON_RIGHT_THUMB:    outSDLButton = SDL_GAMEPAD_BUTTON_RIGHT_STICK; return true;
        case GAMEPAD_BUTTON_BACK:           outSDLButton = SDL_GAMEPAD_BUTTON_BACK; return true;
        case GAMEPAD_BUTTON_START:          outSDLButton = SDL_GAMEPAD_BUTTON_START; return true;
        case GAMEPAD_BUTTON_DPAD_UP:        outSDLButton = SDL_GAMEPAD_BUTTON_DPAD_UP; return true;
        case GAMEPAD_BUTTON_DPAD_DOWN:      outSDLButton = SDL_GAMEPAD_BUTTON_DPAD_DOWN; return true;
        case GAMEPAD_BUTTON_DPAD_LEFT:      outSDLButton = SDL_GAMEPAD_BUTTON_DPAD_LEFT; return true;
        case GAMEPAD_BUTTON_DPAD_RIGHT:     outSDLButton = SDL_GAMEPAD_BUTTON_DPAD_RIGHT; return true;
        case GAMEPAD_BUTTON_GUIDE:          outSDLButton = SDL_GAMEPAD_BUTTON_GUIDE; return true;
        default:
            return false;
    }
}

std::string Glyph::GetButtonLabel(GlyphStyle style, SDL_GamepadButton button)
{
    GlyphStyle resolvedStyle = style;

    // Auto-detect style from controller type
    if (style == GlyphStyle::Auto)
    {
        SDL_GamepadType type = currentControllerType;
        if (type == SDL_GAMEPAD_TYPE_PS3 || type == SDL_GAMEPAD_TYPE_PS4 || type == SDL_GAMEPAD_TYPE_PS5)
            resolvedStyle = GlyphStyle::PlayStation;
        else if (type == SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO || type == SDL_GAMEPAD_TYPE_GAMECUBE ||
                 type == SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR || type == SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_LEFT ||
                 type == SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT)
            resolvedStyle = GlyphStyle::Nintendo;
        else
            resolvedStyle = GlyphStyle::Xbox;  // Default to Xbox for unknown types
    }

    // Return label based on resolved style
    switch (resolvedStyle)
    {
        case GlyphStyle::PlayStation:
            return PlayStationButtonToLabel(button);
        case GlyphStyle::Nintendo:
            return NintendoButtonToLabel(button);
        case GlyphStyle::Xbox:
        case GlyphStyle::Auto:  // Already resolved, fallback not reached
        default:
            return XboxButtonToLabel(button);
    }
}

std::string Glyph::GetButtonLabelFromMask(GlyphStyle style, WORD legacyButtonMask)
{
    SDL_GamepadButton sdlButton = SDL_GAMEPAD_BUTTON_INVALID;
    if (SDLButtonFromLegacyMask(legacyButtonMask, sdlButton))
        return GetButtonLabel(style, sdlButton);

    return "Unknown";
}

std::string Glyph::GetXboxLabel(SDL_GamepadButton button)
{
    return XboxButtonToLabel(button);
}

std::string Glyph::GetPlayStationLabel(SDL_GamepadButton button)
{
    return PlayStationButtonToLabel(button);
}

std::string Glyph::GetNintendoLabel(SDL_GamepadButton button)
{
    return NintendoButtonToLabel(button);
}

std::string Glyph::GetXboxLabelFromMask(WORD legacyButtonMask)
{
    return GetButtonLabelFromMask(GlyphStyle::Xbox, legacyButtonMask);
}

std::string Glyph::GetPlayStationLabelFromMask(WORD legacyButtonMask)
{
    return GetButtonLabelFromMask(GlyphStyle::PlayStation, legacyButtonMask);
}

std::string Glyph::GetNintendoLabelFromMask(WORD legacyButtonMask)
{
    return GetButtonLabelFromMask(GlyphStyle::Nintendo, legacyButtonMask);
}

void Glyph::SetControllerType(SDL_GamepadType type)
{
    currentControllerType = type;
}

SDL_GamepadType Glyph::GetControllerType()
{
    return currentControllerType;
}
