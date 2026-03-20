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

#include "api.h"
#include "gamepad.h"
#include "glyph.h"

thread_local static std::string nxApiBufferString;

FFNX_API void __stdcall nxRegisterMouseListener(MouseListener* listener)
{
    mouseListeners.push_back(listener);
}

FFNX_API void __stdcall nxRegisterKeyListener(KeyListener* listener)
{
    keyListeners.push_back(listener);
}

FFNX_API const char* __stdcall nxGetXboxButtonPrompt(int button)
{
    nxApiBufferString = Glyph::GetXboxLabel((SDL_GamepadButton)button);
    return nxApiBufferString.c_str();
}

FFNX_API const char* __stdcall nxGetPlayStationButtonPrompt(int button)
{
    nxApiBufferString = Glyph::GetPlayStationLabel((SDL_GamepadButton)button);
    return nxApiBufferString.c_str();
}

FFNX_API const char* __stdcall nxGetNintendoButtonPrompt(int button)
{
    nxApiBufferString = Glyph::GetNintendoLabel((SDL_GamepadButton)button);
    return nxApiBufferString.c_str();
}

FFNX_API const char* __stdcall nxGetXboxButtonPromptFromMask(unsigned short legacyButtonMask)
{
    nxApiBufferString = Glyph::GetXboxLabelFromMask(legacyButtonMask);
    return nxApiBufferString.c_str();
}

FFNX_API const char* __stdcall nxGetPlayStationButtonPromptFromMask(unsigned short legacyButtonMask)
{
    nxApiBufferString = Glyph::GetPlayStationLabelFromMask(legacyButtonMask);
    return nxApiBufferString.c_str();
}

FFNX_API const char* __stdcall nxGetNintendoButtonPromptFromMask(unsigned short legacyButtonMask)
{
    nxApiBufferString = Glyph::GetNintendoLabelFromMask(legacyButtonMask);
    return nxApiBufferString.c_str();
}

FFNX_API const char* __stdcall nxGetButtonPromptAuto(int button)
{
    nxApiBufferString = Glyph::GetButtonLabel(GlyphStyle::Auto, (SDL_GamepadButton)button);
    return nxApiBufferString.c_str();
}

FFNX_API const char* __stdcall nxGetButtonPromptAutoFromMask(unsigned short legacyButtonMask)
{
    nxApiBufferString = Glyph::GetButtonLabelFromMask(GlyphStyle::Auto, legacyButtonMask);
    return nxApiBufferString.c_str();
}

FFNX_API int __stdcall nxGetGamepadType(void)
{
    return (int)Glyph::GetControllerType();
}
