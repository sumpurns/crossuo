/****************************************************************************
**
** RenderObject.cpp
**
** Copyright (C) September 2015 Hotride
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
*****************************************************************************
*/
//---------------------------------------------------------------------------
#include "stdafx.h"
//---------------------------------------------------------------------------
TVector::TVector(double x, double y, double z)
: m_X(x), m_Y(y), m_Z(z)
{
}
//---------------------------------------------------------------------------
void TVector::Link(TVector &v)
{
	m_X = v.X;
	m_Y = v.Y;
	m_Z = v.Z;
}
//---------------------------------------------------------------------------
void TVector::Link(double x, double y, double z)
{
	m_X = x;
	m_Y = y;
	m_Z = z;
}
//---------------------------------------------------------------------------
void TVector::Add(TVector &v)
{
	m_X += v.X;
	m_Y += v.Y;
	m_Z += v.Z;
}
//---------------------------------------------------------------------------
void TVector::Add(double x, double y, double z)
{
	m_X += x;
	m_Y += y;
	m_Z += z;
}
//---------------------------------------------------------------------------
void TVector::Merge(TVector &v)
{
	double newX = m_Y * v.Z - m_Z * v.Y;
	double newY = m_Z * v.X - m_X * v.Z;
	double newZ = m_X * v.Y - m_Y * v.X;

	m_X = newX;
	m_Y = newY;
	m_Z = newZ;
}
//---------------------------------------------------------------------------
void TVector::Merge(double x, double y, double z)
{
	double newX = m_Y * z - m_Z * y;
	double newY = m_Z * x - m_X * z;
	double newZ = m_X * y - m_Y * x;

	m_X = newX;
	m_Y = newY;
	m_Z = newZ;
}
//---------------------------------------------------------------------------
void TVector::Normalize()
{
	if (!m_X && !m_Y && !m_Z)
			return;

	double result = m_X * m_X + m_Y * m_Y + m_Z * m_Z;

	//if (result)
	{
		result = sqrtl(result);
		
		m_X /= result;
		m_Y /= result;
		m_Z /= result;
	}
}
//---------------------------------------------------------------------------
//------------------------------TMapObject----------------------------------
//---------------------------------------------------------------------------
TMapObject::TMapObject(RENDER_OBJECT_TYPE type, DWORD serial, WORD graphic, WORD color, short x, short y, char z)
: TRenderWorldObject(type, serial, graphic, color, x, y, z)
{
}
//---------------------------------------------------------------------------
TMapObject::~TMapObject()
{
#if UO_DEBUG_INFO!=0
	if (m_RenderType == ROT_LAND_OBJECT)
		g_LandObjectsCount--;
	else if (m_RenderType == ROT_STATIC_OBJECT)
		g_StaticsObjectsCount--;
#endif //UO_DEBUG_INFO!=0
}
//---------------------------------------------------------------------------
//---------------------------------TLandObject------------------------------
//---------------------------------------------------------------------------
TLandObject::TLandObject(DWORD serial, WORD graphic, WORD color, short x, short y, char z)
: TMapObject(ROT_LAND_OBJECT, serial, graphic, color, x, y, z), m_MinZ(z),
m_ListCreated(false)
{
	LAND_TILES &tile = UO->m_LandData[graphic / 32].Tiles[graphic % 32];

	m_IsStretched = (!tile.TexID && ::IsWet(tile.Flags));

	memset(&m_Rect, 0, sizeof(RECT));
	memset(&m_Normals[0], 0, sizeof(m_Normals));

#if UO_DEBUG_INFO!=0
	g_LandObjectsCount++;
#endif //UO_DEBUG_INFO!=0
}
//---------------------------------------------------------------------------
void TLandObject::UpdateZ(char zTop, char zRight, char zBottom)
{
	if (m_IsStretched)
	{
		//�������� ������� �������� Z-����������
		m_Serial = ((m_Z + zTop + zRight + zBottom) / 4);

		//�������� ��� �������
		m_Rect.left = m_Z * 4;
		m_Rect.top = zTop * 4;
		m_Rect.right = zRight * 4;
		m_Rect.bottom = zBottom * 4;

		//����������� Z-���������� �� ����
		m_MinZ = m_Z;

		if (zTop < m_MinZ)
			m_MinZ = zTop;

		if (zRight < m_MinZ)
			m_MinZ = zRight;

		if (zBottom < m_MinZ)
			m_MinZ = zBottom;
	}
}
//---------------------------------------------------------------------------
int TLandObject::Draw(bool &mode, int &drawX, int &drawY, DWORD &ticks)
{
	if (m_Z <= g_MaxGroundZ)
	{
		if (mode)
		{
			WORD objColor = 0;

			if (this == g_SelectedObject)
				objColor = SELECT_LAND_COLOR;

#if UO_DEBUG_INFO!=0
g_RenderedObjectsCountInGameWindow++;
#endif
			glEnable(GL_DEPTH_TEST);

			if (!m_IsStretched)
				UO->DrawLandArt(m_Graphic, objColor, drawX, drawY, m_Z);
			else
			{
				char minZ = m_MinZ + 3;

				if (minZ > m_Z)
					minZ = m_Z;

				g_ZBuffer = minZ;

				UO->DrawLandTexture(m_Graphic, objColor, drawX, drawY, m_Rect, m_Normals);
			}

			glDisable(GL_DEPTH_TEST);
		}
		else
		{
			if (!m_IsStretched)
			{
				if (UO->LandPixelsInXY(m_Graphic, drawX, drawY, m_Z))
				{
					g_LastObjectType = SOT_LAND_OBJECT;
					g_LastSelectedObject = 2;
					g_SelectedObject = this;
				}
			}
			else
			{
				if (UO->LandTexturePixelsInXY(drawX, drawY, m_Rect))
				{
					g_LastObjectType = SOT_LAND_OBJECT;
					g_LastSelectedObject = 2;
					g_SelectedObject = this;
				}
			}
		}
	}

	return 0;
}
//---------------------------------------------------------------------------
//------------------------------TRenderStaticObject-------------------------
//---------------------------------------------------------------------------
TRenderStaticObject::TRenderStaticObject(RENDER_OBJECT_TYPE renderType, DWORD serial, WORD graphic, WORD color, short x, short y, char z)
: TMapObject(renderType, serial, graphic, color, x, y, z), m_FoliageTransparentIndex(-1),
m_TiledataPtr(&UO->m_StaticData[graphic / 32].Tiles[graphic % 32])
{
	if (m_TiledataPtr->Height > 5)
		m_CanBeTransparent = 1;
	else if (IsRoof() || (IsSurface() && IsBackground()) || IsWall())
		m_CanBeTransparent = 1;
	else if (m_TiledataPtr->Height == 5 && IsSurface() && !IsBackground())
		m_CanBeTransparent = 1;
	else
		m_CanBeTransparent = 0;

	if (renderType == ROT_GAME_OBJECT)
	{
		m_TextControl = new TTextContainer(3);

		if (IsSurface() || (IsBackground() && IsUnknown2()))
			m_CanBeTransparent |= 0x10;
	}
	else
	{
		m_TextControl = new TTextContainer(1);

		if (IsSurface() || (IsBackground() && IsUnknown2()) || IsRoof())
			m_CanBeTransparent |= 0x10;
	}
}
//---------------------------------------------------------------------------
TRenderStaticObject::~TRenderStaticObject()
{
	if (m_TextControl != NULL)
	{
		delete m_TextControl;
		m_TextControl = NULL;
	}
}
//---------------------------------------------------------------------------
void TRenderStaticObject::AddText(TTextData *msg)
{
	if (m_TextControl != NULL)
	{
		m_TextControl->Add(msg);

		UO->AddJournalMessage(msg, "You see: ");
	}
}
//---------------------------------------------------------------------------
int TRenderStaticObject::GetTextOffsetX(TTextData *text)
{
	return text->m_Texture.Width / 2;
}
//---------------------------------------------------------------------------
int TRenderStaticObject::GetTextOffsetY(TTextData *text)
{
	int offset = 0;

	TTextData *td = m_TextControl->m_Head;

	while (td != NULL)
	{
		offset += td->m_Texture.Height;
		
		if (text == td)
			break;

		td = td->m_Prev;
	}

	return offset;
}
//---------------------------------------------------------------------------
bool TRenderStaticObject::TextCanBeTransparent(TRenderTextObject *text)
{
	bool result = true;

	for (TTextData *td = m_TextControl->m_Head; td != NULL; td = td->m_Prev)
	{
		if (text == td)
		{
			result = false;
			break;
		}
	}

	return result;
}
//---------------------------------------------------------------------------
bool TRenderStaticObject::TranparentTest(int &playerZPlus5)
{
	bool result = true;

	if (m_Z <= playerZPlus5 - m_TiledataPtr->Height)
		result = false;
	else if (playerZPlus5 < m_Z && !(m_CanBeTransparent & 0xF))
		result = false;

	return result;
}
//---------------------------------------------------------------------------
//-------------------------------TStaticObject------------------------------
//---------------------------------------------------------------------------
TStaticObject::TStaticObject(DWORD serial, WORD graphic, WORD color, short x, short y, char z)
: TRenderStaticObject(ROT_STATIC_OBJECT, serial, graphic, color, x, y, z)
{
	m_Graphic += 0x4000;
	
	if (IsWet())
		m_RenderQueueIndex = 1;
	else if (IsBackground())
		m_RenderQueueIndex = 3 - (int)IsSurface();
	else if (IsSurface())
		m_RenderQueueIndex = 4;
	else
		m_RenderQueueIndex = 6;

	m_TextControl->MaxSize = 1;

#if UO_DEBUG_INFO!=0
	g_StaticsObjectsCount++;
#endif //UO_DEBUG_INFO!=0
}
//---------------------------------------------------------------------------
int TStaticObject::Draw(bool &mode, int &drawX, int &drawY, DWORD &ticks)
{
	if (g_NoDrawRoof && IsRoof())
		return 0;

	WORD objGraphic = m_Graphic - 0x4000;

	if (mode)
	{
#if UO_DEBUG_INFO!=0
		g_RenderedObjectsCountInGameWindow++;
#endif

		WORD objColor = m_Color;

		if (this == g_SelectedObject)
			objColor = SELECT_STATIC_COLOR;

		if (IsFoliage())
		{
			if (!g_GrayedPixels)
			{
				if (m_FoliageTransparentIndex == g_FoliageIndex)
				{
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					glColor4f(1.0f, 1.0f, 1.0f, 0.3f);

					UO->DrawStaticArtAnimated(objGraphic, objColor, drawX, drawY, m_Z);
					
					glDisable(GL_BLEND);
				}
				else
					UO->DrawStaticArtAnimated(objGraphic, objColor, drawX, drawY, m_Z);
			}
		}
		else
		{
			if (m_CanBeTransparent & 0x10)
				glEnable(GL_DEPTH_TEST);

			if (g_UseCircleTrans)
				UO->DrawStaticArtAnimatedTransparent(objGraphic, objColor, drawX, drawY, m_Z);
			else
				UO->DrawStaticArtAnimated(objGraphic, objColor, drawX, drawY, m_Z);

			glDisable(GL_DEPTH_TEST);
		}

		if (IsLightSource() && GameScreen->UseLight)
			GameScreen->AddLight(this, this, drawX, drawY - (m_Z * 4));
	}
	else
	{
		if (IsFoliage())
		{
			if (!g_GrayedPixels)
			{
				if (m_FoliageTransparentIndex != g_FoliageIndex)
				{
					if (UO->StaticPixelsInXYAnimated(objGraphic, drawX, drawY, m_Z))
					{
						g_LastObjectType = SOT_STATIC_OBJECT;
						g_LastSelectedObject = 3;
						g_SelectedObject = this;
					}
				}
			}
		}
		else if (UO->StaticPixelsInXYAnimated(objGraphic, drawX, drawY, m_Z) && !g_UseCircleTrans)
		{
			g_LastObjectType = SOT_STATIC_OBJECT;
			g_LastSelectedObject = 3;
			g_SelectedObject = this;
		}
	}

	return 0;
}
//---------------------------------------------------------------------------