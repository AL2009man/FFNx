/****************************************************************************/
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
#include <cstdint>
#include <bgfx/bgfx.h>
#include <bimg/bimg.h>

// Button prompt byte codes from field scripts
enum class ButtonPromptType : uint8_t
{
    SQUARE_X = 0xF8,
    TRIANGLE_Y = 0xF7,
    CROSS_A = 0xF9,
    CIRCLE_B = 0xF6,       // Confirm button (swapped mode)
    CIRCLE_B_ALT = 0x10,   // Confirm button (no-swap mode) - 0xf610
    L1_LB = 0x11,
    R1_RB = 0x12,
    L2_LT = 0x13,
    R2_RT = 0x14,
};

// Glyph rendering system for button prompts
class GlyphRenderer
{
private:
    bimg::ImageContainer* buttonTexture;
    bgfx::TextureHandle textureHandle;
    bool initialized;
    
    bool loadButtonTexture(SDL_Gamepad* gamepad);
    void unloadButtonTexture();
    
public:
    GlyphRenderer();
    ~GlyphRenderer();
    
    bool init(SDL_Gamepad* gamepad);
    void shutdown();
    
    void renderGlyph(float x, float y, ButtonPromptType button, SDL_Gamepad* gamepad, float scale = 1.0f);
    void getGlyphUV(ButtonPromptType button, float& u0, float& v0, float& u1, float& v1) const;

    // Query the correctly remapped button label for confirm/cancel actions
    SDL_GamepadButtonLabel getConfirmActionLabel(SDL_Gamepad* gamepad) const;
    SDL_GamepadButtonLabel getCancelActionLabel(SDL_Gamepad* gamepad) const;

    bool isReady() const { return initialized && bgfx::isValid(textureHandle); }
};

extern GlyphRenderer glyphRenderer;
