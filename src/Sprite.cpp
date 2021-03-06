// GPLv3 License
// Copyright (C) 2019 Danny Angelo Carminati Grein

#include "Sprite.h"
#include "Managers/MouseManager.h"

void CSprite::LoadSprite16(int width, int height, uint16_t *pixels, bool skipHitMask)
{
    assert(Texture == nullptr && "texture should be null");
    Texture = new CGLTexture;
    g_GL_BindTexture16(*Texture, width, height, pixels);
    Init(width, height, pixels, skipHitMask);
}

void CSprite::LoadSprite32(int width, int height, uint32_t *pixels, bool skipHitMask)
{
    assert(Texture == nullptr && "texture should be null");
    Texture = new CGLTexture;
    g_GL_BindTexture32(*Texture, width, height, pixels);
    Init(width, height, pixels, skipHitMask);
}

template <typename T>
void CSprite::Init(int width, int height, T *pixels, bool skipHitMask)
{
    Width = width;
    Height = height;
    if (!skipHitMask)
    {
        BuildHitMask(width, height, pixels);
    }
}

void CSprite::Clear()
{
    if (Texture)
    {
        delete Texture;
        Texture = nullptr;
    }
    Width = 0;
    Height = 0;
    m_HitMap.clear();
}

bool CSprite::Select(int x, int y, bool pixelCheck)
{
    x = g_MouseManager.Position.X - x;
    y = g_MouseManager.Position.Y - y;
    return TestHit(x, y, pixelCheck);
}

bool CSprite::TestHit(int x, int y, bool pixelCheck)
{
    if (x >= 0 && y >= 0 && x < Width && y < Height)
    {
        if (!pixelCheck)
        {
            return true;
        }

        int pos = (y * Width) + x;
        if (pos < (int)m_HitMap.size())
        {
            return m_HitMap[pos];
        }
    }

    return false;
}

template <typename T>
void CSprite::BuildHitMask(int w, int h, T *pixels)
{
    m_HitMap.resize(w * h);
    int pos = 0;
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            m_HitMap[pos] = (pixels[pos] != 0);
            pos++;
        }
    }
}

template void CSprite::BuildHitMask<uint16_t>(int w, int h, uint16_t *pixels);
template void CSprite::BuildHitMask<uint32_t>(int w, int h, uint32_t *pixels);
template void CSprite::Init<uint16_t>(int width, int height, uint16_t *pixels, bool skipHitMask);
template void CSprite::Init<uint32_t>(int width, int height, uint32_t *pixels, bool skipHitMask);
