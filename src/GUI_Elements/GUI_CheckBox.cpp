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

#include "GUI_CheckBox.h"
#include "../engine.h"

extern Engine g_engine;

GUI_CheckBox::GUI_CheckBox(iRect boxRect, const std::string labelName, bool checked, Uint32 internalID, Uint8 red, Uint8 green, Uint8 blue)
{
	setRect(boxRect);
	PERFORM_MOVE(m_Label, labelName);
	m_Checked = checked;
	m_evtParam = 0;
	m_eventHandlerFunction = NULL;
	m_red = red;
	m_green = green;
	m_blue = blue;
	m_internalID = internalID;
}

void GUI_CheckBox::setBoxEventCallback(void (*eventHandlerFunction)(Uint32,Sint32), Uint32 mEvent)
{
	m_evtParam = mEvent;
	m_eventHandlerFunction = eventHandlerFunction;
}

void GUI_CheckBox::setName(const std::string labelName)
{
	PERFORM_MOVE(m_Label, labelName);
}

void GUI_CheckBox::setChecked(bool check)
{
    m_Checked = check;
    if(m_eventHandlerFunction)
		UTIL_SafeEventHandler(m_eventHandlerFunction, m_evtParam, (m_Checked ? 1 : 0));
}

void GUI_CheckBox::onLMouseUp(Sint32 x, Sint32 y)
{
	if(m_tRect.isPointInside(x, y))
		setChecked(!m_Checked);
}

void GUI_CheckBox::render()
{
	Surface* renderer = g_engine.getRender();
	renderer->drawPictureRepeat(3, 0, 96, 96, 1, m_tRect.x1, m_tRect.y1, m_tRect.x2, 1);
	renderer->drawPictureRepeat(3, 0, 98, 1, 96, m_tRect.x1, m_tRect.y1+1, 1, m_tRect.y2-1);
	renderer->drawPictureRepeat(3, 2, 197, 96, 1, m_tRect.x1+1, m_tRect.y1+m_tRect.y2-1, m_tRect.x2-1, 1);
	renderer->drawPictureRepeat(3, 276, 0, 1, 96, m_tRect.x1+m_tRect.x2-1, m_tRect.y1+1, 1, m_tRect.y2-2);
	renderer->drawPicture(3, 308, (m_Checked ? 108 : 96), m_tRect.x1+6, m_tRect.y1+5, 12, 12);
	g_engine.drawFont(CLIENT_FONT_NONOUTLINED, m_tRect.x1+22, m_tRect.y1+6, m_Label, m_red, m_green, m_blue, CLIENT_FONT_ALIGN_LEFT);
}
