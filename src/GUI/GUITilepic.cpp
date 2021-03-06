﻿// MIT License
// Copyright (C) August 2016 Hotride

#include "GUITilepic.h"
#include "../api/uodata.h"
#include "../CrossUO.h"
#include "../Point.h"

CGUITilepic::CGUITilepic(uint16_t graphic, uint16_t color, int x, int y)
    : CGUIDrawObject(GOT_TILEPIC, 0, graphic, color, x, y)
{
}

CGUITilepic::~CGUITilepic()
{
}

CSize CGUITilepic::GetSize()
{
    DEBUG_TRACE_FUNCTION;
    CSize size;
    auto spr = g_Game.ExecuteStaticArt(Graphic);
    if (spr != nullptr)
    {
        size.Width = spr->Width;
        size.Height = spr->Height;
    }
    return size;
}

void CGUITilepic::PrepareTextures()
{
    DEBUG_TRACE_FUNCTION;
    g_Game.ExecuteStaticArt(Graphic);
}

void CGUITilepic::Draw(bool checktrans)
{
    DEBUG_TRACE_FUNCTION;
    auto spr = g_Game.ExecuteStaticArt(Graphic);
    if (spr != nullptr && spr->Texture)
    {
        SetShaderMode();
        spr->Texture->Draw(m_X, m_Y, checktrans);
    }
}

bool CGUITilepic::Select()
{
    DEBUG_TRACE_FUNCTION;
    auto spr = g_Index.m_Static[Graphic].Sprite;
    if (spr != nullptr)
    {
        return spr->Select(m_X, m_Y, !BoundingBoxCheck);
    }
    return false;
}
