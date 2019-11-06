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

#include "GUI_Button.h"
#include "../engine.h"

extern Engine g_engine;

GUI_Button::GUI_Button(iRect boxRect, const std::string labelName, Uint32 internalID)
{
	m_sx[0] = m_sx[1] = 0;
	m_sy[0] = m_sy[1] = 0;

	setRect(boxRect);
	PERFORM_MOVE(m_Label, labelName);
	m_startX = g_engine.calculateFontWidth(CLIENT_FONT_SMALL, m_Label)/2;
	m_Pressed = 0;
	m_evtParam = 0;
	m_eventHandlerFunction = NULL;
	m_internalID = internalID;
}

void GUI_Button::setRect(iRect& NewRect)
{
	switch(NewRect.x2)
	{
		case 34:
			m_sx[0] = 174;
			m_sx[1] = 174;
			m_sy[0] = 138;
			m_sy[1] = 158;
			break;
		case 43:
			m_sx[0] = 2;
			m_sx[1] = 2;
			m_sy[0] = 138;
			m_sy[1] = 158;
			break;
		case 58:
			m_sx[0] = 236;
			m_sx[1] = 294;
			m_sy[0] = 252;
			m_sy[1] = 252;
			break;
		case 75:
			m_sx[0] = 0;
			m_sx[1] = 0;
			m_sy[0] = 452;
			m_sy[1] = 472;
			break;
		case 86:
			m_sx[0] = 45;
			m_sx[1] = 45;
			m_sy[0] = 138;
			m_sy[1] = 158;
			break;
	}
	m_tRect = NewRect;
}

void GUI_Button::setButtonEventCallback(void (*eventHandlerFunction)(Uint32,Sint32), Uint32 mEvent)
{
	m_evtParam = mEvent;
	m_eventHandlerFunction = eventHandlerFunction;
}

void GUI_Button::onMouseMove(Sint32 x, Sint32 y, bool)
{
	if(m_Pressed > 0)
	{
		bool inside = m_tRect.isPointInside(x, y);
		if(m_Pressed == 1 && !inside)
			m_Pressed = 2;
		else if(m_Pressed == 2 && inside)
			m_Pressed = 1;
	}
}

void GUI_Button::onLMouseDown(Sint32, Sint32)
{
	m_Pressed = 1;
}

void GUI_Button::onLMouseUp(Sint32 x, Sint32 y)
{
	if(m_Pressed > 0)
	{
		m_Pressed = 0;
		if(m_tRect.isPointInside(x, y))
		{
			if(m_eventHandlerFunction)
				UTIL_SafeEventHandler(m_eventHandlerFunction, m_evtParam, 1);
		}
	}
}

void GUI_Button::render()
{
	Surface* renderer = g_engine.getRender();
	renderer->drawPicture(3, m_sx[(m_Pressed == 1 ? 1 : 0)], m_sy[(m_Pressed == 1 ? 1 : 0)], m_tRect.x1, m_tRect.y1, m_tRect.x2, m_tRect.y2);
	g_engine.drawFont(CLIENT_FONT_SMALL, m_tRect.x1+(m_tRect.x2/2)+(m_Pressed == 1 ? 1 : 0)-m_startX, m_tRect.y1+(m_Pressed == 1 ? 7 : 6), m_Label, 255, 255, 255, CLIENT_FONT_ALIGN_LEFT);
}
