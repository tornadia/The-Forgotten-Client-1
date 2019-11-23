/*
  Tibia CLient
  Copyright (C) 2019 Saiyans King

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "GUI_Label.h"
#include "../engine.h"

extern Engine g_engine;

GUI_Label::GUI_Label(iRect boxRect, const std::string labelName, Uint32 internalID, Uint8 red, Uint8 green, Uint8 blue)
{
	setRect(boxRect);
	m_Label = std::move(labelName);
	m_font = CLIENT_FONT_NONOUTLINED;
	m_red = red;
	m_green = green;
	m_blue = blue;
	m_internalID = internalID;
	m_align = CLIENT_FONT_ALIGN_LEFT;
}

void GUI_Label::setName(const std::string labelName)
{
	m_Label = std::move(labelName);
}

void GUI_Label::render()
{
	g_engine.drawFont(m_font, m_tRect.x1, m_tRect.y1, m_Label, m_red, m_green, m_blue, m_align);
}
