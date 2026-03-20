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

#include "globals.h"
#include "input.h"
#include <SDL3/SDL.h>

FFNX_API void __stdcall nxRegisterMouseListener(MouseListener* listener);
FFNX_API void __stdcall nxRegisterKeyListener(KeyListener* listener);


/**
 * Get button label in Xbox style (A/B/X/Y, LB/RB, View/Menu, etc.)
 * \param button SDL_GamepadButton button constant
 * \return Localized button name as string
 */
FFNX_API const char* __stdcall nxGetXboxButtonPrompt(int button);

/**
 * Get button label in PlayStation style (Cross/Circle/Square/Triangle, L1/R1, Share/Options, etc.)
 * \param button SDL_GamepadButton button constant
 * \return Localized button name as string
 */
FFNX_API const char* __stdcall nxGetPlayStationButtonPrompt(int button);

/**
 * Get button label in Nintendo style (B/A/Y/X, L/R, Minus/Plus, etc.)
 * \param button SDL_GamepadButton button constant
 * \return Localized button name as string
 */
FFNX_API const char* __stdcall nxGetNintendoButtonPrompt(int button);

/**
 * Get button label in Xbox style using legacy XInput button mask
 * \param legacyButtonMask GAMEPAD_BUTTON_* constant (e.g., GAMEPAD_BUTTON_A, GAMEPAD_BUTTON_X)
 * \return Localized button name as string
 */
FFNX_API const char* __stdcall nxGetXboxButtonPromptFromMask(unsigned short legacyButtonMask);

/**
 * Get button label in PlayStation style using legacy XInput button mask
 * \param legacyButtonMask GAMEPAD_BUTTON_* constant
 * \return Localized button name as string
 */
FFNX_API const char* __stdcall nxGetPlayStationButtonPromptFromMask(unsigned short legacyButtonMask);

/**
 * Get button label in Nintendo style using legacy XInput button mask
 * \param legacyButtonMask GAMEPAD_BUTTON_* constant
 * \return Localized button name as string
 */
FFNX_API const char* __stdcall nxGetNintendoButtonPromptFromMask(unsigned short legacyButtonMask);

/**
 * Auto-detect controller type and get button label in native style
 * \param button SDL_GamepadButton button constant
 * \return Localized button name in detected controller's native style
 */
FFNX_API const char* __stdcall nxGetButtonPromptAuto(int button);

/**
 * Auto-detect controller type and get button label in native style using legacy mask
 * \param legacyButtonMask GAMEPAD_BUTTON_* constant
 * \return Localized button name in detected controller's native style
 */
FFNX_API const char* __stdcall nxGetButtonPromptAutoFromMask(unsigned short legacyButtonMask);

/**
 * Get the currently connected controller type (SDL_GamepadType enum value)
 * \return SDL_GAMEPAD_TYPE_* constant, or SDL_GAMEPAD_TYPE_UNKNOWN if no controller
 */
FFNX_API int __stdcall nxGetGamepadType(void);
