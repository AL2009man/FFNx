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

#include <windows.h>
#include <shlwapi.h>

#include "glyph.h"
#include "image/image.h"
#include "gamepad.h"
#include "log.h"
#include "globals.h"

GlyphRenderer glyphRenderer;

GlyphRenderer::GlyphRenderer()
    : buttonTexture(nullptr), textureHandle(BGFX_INVALID_HANDLE), initialized(false)
{
}

GlyphRenderer::~GlyphRenderer()
{
    shutdown();
}

bool GlyphRenderer::loadButtonTexture(SDL_Gamepad* gamepad)
{
    // Choose sprite sheet based on controller type:
    // PlayStation controllers return LABEL_CROSS for the South button.
    // Xbox and Nintendo Switch controllers return LABEL_A, so they share buttons.png.
    bool isPlayStation = gamepad &&
        SDL_GetGamepadButtonLabel(gamepad, SDL_GAMEPAD_BUTTON_SOUTH) == SDL_GAMEPAD_BUTTON_LABEL_CROSS;

    char path[MAX_PATH] = {};
    strcpy(path, basedir);
    PathAppendA(path, "data");
    PathAppendA(path, "png");
    PathAppendA(path, isPlayStation ? "buttons_ps4.png" : "buttons.png");
    
    if (trace_all || trace_loaders)
        ffnx_trace("Glyph: Loading texture from %s\n", path);
    
    static bx::DefaultAllocator allocator;
    buttonTexture = loadImageContainer(&allocator, path, bimg::TextureFormat::BGRA8);
    
    if (!buttonTexture)
    {
        ffnx_warning("Glyph: Failed to load texture from %s\n", path);
        return false;
    }
    
    // Create BGFX texture
    textureHandle = bgfx::createTexture2D(
        buttonTexture->m_width,
        buttonTexture->m_height,
        false, 1,
        bgfx::TextureFormat::BGRA8,
        BGFX_TEXTURE_NONE,
        bgfx::copy(buttonTexture->m_data, buttonTexture->m_size)
    );
    
    return bgfx::isValid(textureHandle);
}

void GlyphRenderer::unloadButtonTexture()
{
    if (buttonTexture)
    {
        bimg::imageFree(buttonTexture);
        buttonTexture = nullptr;
    }
    
    if (bgfx::isValid(textureHandle))
    {
        bgfx::destroy(textureHandle);
        textureHandle = BGFX_INVALID_HANDLE;
    }
}

bool GlyphRenderer::init(SDL_Gamepad* gamepad)
{
    if (initialized)
        return true;
    
    if (trace_all || trace_loaders)
        ffnx_trace("Glyph: Initializing\n");
    
    if (!loadButtonTexture(gamepad))
    {
        ffnx_error("Glyph: Failed to initialize - texture load failed\n");
        return false;
    }
    
    initialized = true;
    return true;
}

void GlyphRenderer::shutdown()
{
    if (trace_all || trace_loaders)
        ffnx_trace("Glyph: Shutting down\n");
    
    unloadButtonTexture();
    initialized = false;
}

void GlyphRenderer::renderGlyph(float x, float y, ButtonPromptType button, SDL_Gamepad* gamepad, float scale)
{
    if (!isReady())
        return;
    
    // Handle button prompt type and determine SDL button
    SDL_GamepadButton sdlButton = SDL_GAMEPAD_BUTTON_INVALID;
    bool useMapping = false;
    
    switch (button)
    {
        case ButtonPromptType::SQUARE_X:     sdlButton = SDL_GAMEPAD_BUTTON_WEST; break;
        case ButtonPromptType::TRIANGLE_Y:   sdlButton = SDL_GAMEPAD_BUTTON_NORTH; break;
        case ButtonPromptType::CROSS_A:      sdlButton = SDL_GAMEPAD_BUTTON_SOUTH; break;
        case ButtonPromptType::CIRCLE_B:     
            // 0xf6: confirm in swap mode - apply Nintendo-style A/B remap if needed
            sdlButton = SDL_GAMEPAD_BUTTON_EAST;
            useMapping = true;
            break;
        case ButtonPromptType::CIRCLE_B_ALT: 
            // 0xf610: confirm in no-swap mode - always East button, no remap
            sdlButton = SDL_GAMEPAD_BUTTON_EAST;
            useMapping = false;
            break;
        default: return;
    }
    
    // Apply button mapping if this button came from swap-mode script
    if (useMapping)
        sdlButton = ::gamepad.getMappedButton(sdlButton);
    
    // Get UV coords for this button glyph
    float u0, v0, u1, v1;
    getGlyphUV(button, u0, v0, u1, v1);

    // Get label for this face button on this gamepad
    if (gamepad)
    {
        SDL_GamepadButtonLabel label = SDL_GetGamepadButtonLabel(gamepad, sdlButton);
        if (trace_all || trace_gamepad)
            ffnx_trace("Glyph: Render button %d (mapped: %s, label: %d, uv: %.2f,%.2f-%.2f,%.2f) at (%.1f, %.1f)\n", 
                       (int)button, useMapping ? "yes" : "no", (int)label, u0, v0, u1, v1, x, y);
    }
    
    // TODO: render quad at (x, y) with size (scale) sampling [u0,v0]-[u1,v1] from textureHandle
}

void GlyphRenderer::getGlyphUV(ButtonPromptType button, float& u0, float& v0, float& u1, float& v1) const
{
    // Sprite sheet layout: 5 columns, 4 rows
    // Row 0: Cross/A,  Circle/B,   L1/LB, L2/LT, R2/RT
    // Row 1: Square/X, Triangle/Y, R1/RB, Back,  Start
    // Row 2-3: D-pad variants (not used here)
    const float w = 1.0f / 5.0f;
    const float h = 1.0f / 4.0f;

    int col = 0, row = 0;

    switch (button)
    {
        case ButtonPromptType::CROSS_A:      col = 0; row = 0; break;
        case ButtonPromptType::CIRCLE_B:
        case ButtonPromptType::CIRCLE_B_ALT: col = 1; row = 0; break;
        case ButtonPromptType::L1_LB:        col = 2; row = 0; break;
        case ButtonPromptType::L2_LT:        col = 3; row = 0; break;
        case ButtonPromptType::R2_RT:        col = 4; row = 0; break;
        case ButtonPromptType::SQUARE_X:     col = 0; row = 1; break;
        case ButtonPromptType::TRIANGLE_Y:   col = 1; row = 1; break;
        case ButtonPromptType::R1_RB:        col = 2; row = 1; break;
        default: col = 0; row = 0; break;
    }

    u0 = col * w;
    v0 = row * h;
    u1 = u0 + w;
    v1 = v0 + h;
}

SDL_GamepadButtonLabel GlyphRenderer::getConfirmActionLabel(SDL_Gamepad* gamepad) const
{
    if (!gamepad)
        return SDL_GAMEPAD_BUTTON_LABEL_UNKNOWN;
    
    return SDL_GetGamepadButtonLabel(gamepad, ::gamepad.getMappedButton(CONFIRM_BUTTON));
}

SDL_GamepadButtonLabel GlyphRenderer::getCancelActionLabel(SDL_Gamepad* gamepad) const
{
    if (!gamepad)
        return SDL_GAMEPAD_BUTTON_LABEL_UNKNOWN;
    
    return SDL_GetGamepadButtonLabel(gamepad, ::gamepad.getMappedButton(CANCEL_BUTTON));
}

