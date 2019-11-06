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

#include "GUI_Icon.h"
#include "../engine.h"

extern Engine g_engine;

GUI_Icon::GUI_Icon(iRect boxRect, Uint16 picture, Sint32 pictureX, Sint32 pictureY, Sint32 cPictureX, Sint32 cPictureY, Uint32 internalID, const std::string description)
{
	m_picture = picture;
	m_sx[0] = pictureX;
	m_sy[0] = pictureY;
	m_sx[1] = cPictureX;
	m_sy[1] = cPictureY;

	setRect(boxRect);
	PERFORM_MOVE(m_Description, description);
	m_Pressed = 0;
	m_evtParam = 0;
	m_eventHandlerFunction = NULL;
	m_internalID = internalID;
}

void GUI_Icon::setButtonEventCallback(void (*eventHandlerFunction)(Uint32,Sint32), Uint32 mEvent)
{
	m_evtParam = mEvent;
	m_eventHandlerFunction = eventHandlerFunction;
}

void GUI_Icon::setData(Uint16 picture, Sint32 pictureX, Sint32 pictureY, Sint32 cPictureX, Sint32 cPictureY)
{
	m_picture = picture;
	m_sx[0] = pictureX;
	m_sy[0] = pictureY;
	m_sx[1] = cPictureX;
	m_sy[1] = cPictureY;
}

void GUI_Icon::onMouseMove(Sint32 x, Sint32 y, bool)
{
	bool inside = m_tRect.isPointInside(x, y);
	if(m_Pressed > 0)
	{
		if(m_Pressed == 1 && !inside)
			m_Pressed = 2;
		else if(m_Pressed == 2 && inside)
			m_Pressed = 1;
	}
	if(!m_Description.empty() && inside)
		g_engine.showDescription(x, y, m_Description);
}

void GUI_Icon::onLMouseDown(Sint32, Sint32)
{
	m_Pressed = 1;
}

void GUI_Icon::onLMouseUp(Sint32 x, Sint32 y)
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

void GUI_Icon::render()
{
	Surface* renderer = g_engine.getRender();
	renderer->drawPicture(m_picture, m_sx[(m_Pressed == 1 ? 1 : 0)], m_sy[(m_Pressed == 1 ? 1 : 0)], m_tRect.x1, m_tRect.y1, m_tRect.x2, m_tRect.y2);
}
