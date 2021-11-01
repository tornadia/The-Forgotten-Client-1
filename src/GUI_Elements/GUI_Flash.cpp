/*
  The Forgotten Client
  Copyright (C) 2020 Saiyans King

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

#include "GUI_Flash.h"
#include "../engine.h"

extern Engine g_engine;
extern Uint32 g_frameTime;

GUI_Flash::GUI_Flash(iRect boxRect, Uint32 internalID)
{
	setRect(boxRect);
	m_startFlash = g_frameTime;
	m_lastControlFlash = g_frameTime;
	m_internalID = internalID;
}

void GUI_Flash::reset()
{
	m_startFlash = g_frameTime;
	m_lastControlFlash = g_frameTime;
	m_controlFlash = true;
}

void GUI_Flash::setEndEventCallback(void (*eventHandlerFunction)(Uint32, Sint32), Uint32 mEvent)
{
	m_evtParam = mEvent;
	m_eventHandlerFunction = eventHandlerFunction;
}

void GUI_Flash::render()
{
	if((g_frameTime - m_startFlash) < 8000)
	{
		if((g_frameTime - m_lastControlFlash) >= 500)
		{
			m_controlFlash = !m_controlFlash;
			m_lastControlFlash = g_frameTime;
		}
		if(!m_controlFlash)
			return;
	}
	else
	{
		if(m_eventHandlerFunction)
			UTIL_SafeEventHandler((void*)m_eventHandlerFunction, m_evtParam, 1);
		return;
	}

	auto& renderer = g_engine.getRender();
	renderer->drawRectangle(m_tRect.x1, m_tRect.y1, m_tRect.x2, m_tRect.y2, 1, 255, 160, 0, 255);
}
