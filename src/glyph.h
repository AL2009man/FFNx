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

#pragma once

#include <SDL3/SDL.h>
#include <string>

/**
 * Glyph style enumeration for different controller button naming conventions
 */
enum class GlyphStyle
{
    Auto,          // Auto-detect from controller type
    Xbox,          // Xbox-style: A/B/X/Y, LB/RB, View/Menu
    PlayStation,   // PlayStation-style: Cross/Circle/Square/Triangle, L1/R1, Share/Options
    Nintendo       // Nintendo-style: B/A/Y/X, L/R, Minus/Plus
};

/**
 * Glyph management system for controller button naming and localization
 * Provides clean interface for getting button labels in different styles
 */
class Glyph
{
public:
    /**
     * Get button label in specified style
     * \param style GlyphStyle enum (Xbox, PlayStation, Nintendo, or Auto)
     * \param button SDL_GamepadButton enum
     * \return Button label as string (e.g., "A", "Cross", "B")
     */
    static std::string GetButtonLabel(GlyphStyle style, SDL_GamepadButton button);

    /**
     * Get button label in specified style from legacy button mask
     * \param style GlyphStyle enum
     * \param legacyButtonMask GAMEPAD_BUTTON_* constant
     * \return Button label as string
     */
    static std::string GetButtonLabelFromMask(GlyphStyle style, WORD legacyButtonMask);

    /**
     * Get button label in Xbox style
     * \param button SDL_GamepadButton enum
     * \return Xbox button label
     */
    static std::string GetXboxLabel(SDL_GamepadButton button);

    /**
     * Get button label in PlayStation style
     * \param button SDL_GamepadButton enum
     * \return PlayStation button label
     */
    static std::string GetPlayStationLabel(SDL_GamepadButton button);

    /**
     * Get button label in Nintendo style
     * \param button SDL_GamepadButton enum
     * \return Nintendo button label
     */
    static std::string GetNintendoLabel(SDL_GamepadButton button);

    /**
     * Get button label in Xbox style from legacy mask
     * \param legacyButtonMask GAMEPAD_BUTTON_* constant
     * \return Xbox button label
     */
    static std::string GetXboxLabelFromMask(WORD legacyButtonMask);

    /**
     * Get button label in PlayStation style from legacy mask
     * \param legacyButtonMask GAMEPAD_BUTTON_* constant
     * \return PlayStation button label
     */
    static std::string GetPlayStationLabelFromMask(WORD legacyButtonMask);

    /**
     * Get button label in Nintendo style from legacy mask
     * \param legacyButtonMask GAMEPAD_BUTTON_* constant
     * \return Nintendo button label
     */
    static std::string GetNintendoLabelFromMask(WORD legacyButtonMask);

    /**
     * Set the controller type for auto-detection
     * \param type SDL_GamepadType for determining native button labels
     */
    static void SetControllerType(SDL_GamepadType type);

    /**
     * Get the currently set controller type
     * \return SDL_GamepadType
     */
    static SDL_GamepadType GetControllerType();

private:
    static SDL_GamepadType currentControllerType;
};
