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

#include "GUI_Container.h"
#include "GUI_ScrollBar.h"
#include "GUI_PanelWindow.h"
#include "../engine.h"

extern Engine g_engine;
extern Uint32 g_frameTime;

GUI_Container::GUI_Container(iRect boxRect, GUI_PanelWindow* parent, Uint32 internalID)
{
	m_parent = parent;
	m_scrollBar = new GUI_VScrollBar(iRect(0, 0, 0, 0), 0, 0);
	setRect(boxRect);
	m_contentSize = 0;
	m_lastPosX = 0;
	m_lastPosY = 0;
	m_maxDisplay = 0;
	m_internalID = internalID;
	m_visible = true;
	m_resetPosition = false;
	m_actElement = NULL;
}

GUI_Container::~GUI_Container()
{
	clearChilds();
	delete m_scrollBar;
}

void GUI_Container::setRect(iRect& NewRect)
{
	iRect nRect = iRect(NewRect.x1+NewRect.x2-12, NewRect.y1, 12, NewRect.y2);
	m_scrollBar->setRect(nRect);
	m_tRect = NewRect;
	m_maxDisplay = (m_tRect.y2+3)/4;
	m_scrollBar->setScrollSize(((m_contentSize+23)/4)-m_maxDisplay);
	m_resetPosition = true;
}

void GUI_Container::setActiveElement(GUI_Element* actElement)
{
	if(m_actElement == actElement)
		return;

	if(m_actElement)
		m_actElement->deActivate();

	m_actElement = actElement;
	if(m_actElement)
	{
        m_actElement->activate();
		for(std::vector<GUI_Element*>::iterator it = m_childs.begin(), end = m_childs.end(); it != end; ++it)
		{
			if((*it) == m_actElement)
			{
				m_childs.erase(it);
				break;
			}
		}
		m_childs.push_back(m_actElement);
	}
}

void GUI_Container::clearChilds()
{
	m_contentSize = 0;
	m_lastPosX = 0;
	m_lastPosY = 0;
	for(std::vector<GUI_Element*>::iterator it = m_childs.begin(), end = m_childs.end(); it != end; ++it)
		delete (*it);
	m_childs.clear();
}

void GUI_Container::addChild(GUI_Element* pChild)
{
	m_childs.push_back(pChild);

	Sint32 posY = pChild->getRect().y1+pChild->getRect().y2;
	if(posY > m_contentSize)
	{
		m_contentSize = posY;
		m_scrollBar->setScrollSize(((m_contentSize+23)/4)-(m_tRect.y2+3)/4);
	}
	m_resetPosition = true;
}

void GUI_Container::removeChild(GUI_Element* pChild)
{
	for(std::vector<GUI_Element*>::iterator it = m_childs.begin(), end = m_childs.end(); it != end; ++it)
	{
		if((*it) == pChild)
		{
			m_childs.erase(it);
			break;
		}
	}
	delete pChild;
	m_resetPosition = true;
}

GUI_Element* GUI_Container::getChild(Uint32 internalID)
{
	for(std::vector<GUI_Element*>::iterator it = m_childs.begin(), end = m_childs.end(); it != end; ++it)
	{
		if((*it)->getInternalID() == internalID)
			return (*it);
	}
	return NULL;
}

void GUI_Container::setAsMaxHeight()
{
	if(m_parent)
		m_parent->setMaxHeight(m_contentSize+40);
}

void* GUI_Container::onAction(Sint32 x, Sint32 y)
{
	if(!m_visible)
		return NULL;

	for(std::vector<GUI_Element*>::reverse_iterator it = m_childs.rbegin(), end = m_childs.rend(); it != end; ++it)
	{
		if((*it)->isEventable() && (*it)->getRect().isPointInside(x, y))
			return (*it)->onAction(x, y);
	}
	return NULL;
}

void GUI_Container::onLMouseDown(Sint32 x, Sint32 y)
{
	if(!m_visible)
		return;

	if(m_scrollBar->getRect().isPointInside(x, y))
	{
		m_scrollBar->onLMouseDown(x, y);
		return;
	}

	std::vector<GUI_Element*> childsBackup = m_childs;
	for(std::vector<GUI_Element*>::reverse_iterator it = childsBackup.rbegin(), end = childsBackup.rend(); it != end; ++it)
	{
		if((*it)->isEventable() && (*it)->getRect().isPointInside(x, y))
		{
			setActiveElement((*it));
			(*it)->onLMouseDown(x, y);
			return;
		}
	}
}

void GUI_Container::onLMouseUp(Sint32 x, Sint32 y)
{
	if(!m_visible)
		return;

	m_scrollBar->onLMouseUp(x, y);
	for(std::vector<GUI_Element*>::iterator it = m_childs.begin(), end = m_childs.end(); it != end; ++it)
		(*it)->onLMouseUp(x, y);
}

void GUI_Container::onRMouseDown(Sint32 x, Sint32 y)
{
	if(!m_visible)
		return;

	if(m_scrollBar->getRect().isPointInside(x, y))
	{
		m_scrollBar->onRMouseDown(x, y);
		return;
	}

	std::vector<GUI_Element*> childsBackup = m_childs;
	for(std::vector<GUI_Element*>::reverse_iterator it = childsBackup.rbegin(), end = childsBackup.rend(); it != end; ++it)
	{
		if((*it)->isEventable() && (*it)->getRect().isPointInside(x, y))
		{
			setActiveElement((*it));
			(*it)->onRMouseDown(x, y);
			return;
		}
	}
}

void GUI_Container::onRMouseUp(Sint32 x, Sint32 y)
{
	if(!m_visible)
		return;

	m_scrollBar->onRMouseUp(x, y);
	for(std::vector<GUI_Element*>::iterator it = m_childs.begin(), end = m_childs.end(); it != end; ++it)
		(*it)->onRMouseUp(x, y);
}

void GUI_Container::onMouseMove(Sint32 x, Sint32 y, bool isInsideParent)
{
	if(!m_visible)
		return;

	m_scrollBar->onMouseMove(x, y, isInsideParent);
	if(isInsideParent)
		isInsideParent = isInsideRect(x, y);
	for(std::vector<GUI_Element*>::iterator it = m_childs.begin(), end = m_childs.end(); it != end; ++it)
		(*it)->onMouseMove(x, y, isInsideParent);
}

void GUI_Container::onWheel(Sint32 x, Sint32 y, bool wheelUP)
{
	if(!m_visible)
		return;

	(void)x;
	(void)y;
	/*
	It breaks scrolling by mouse so it is disabled untill there's reason to enable it
	std::vector<GUI_Element*> childsBackup = m_childs;
	for(std::vector<GUI_Element*>::reverse_iterator it = childsBackup.rbegin(), end = childsBackup.rend(); it != end; ++it)
	{
		if((*it)->isEventable() && (*it)->getRect().isPointInside(x, y))
		{
			(*it)->onWheel(x, y, wheelUP);
			return;
		}
	}*/

	if(wheelUP)
		m_scrollBar->setScrollPos(m_scrollBar->getScrollPos()-1);
	else
		m_scrollBar->setScrollPos(m_scrollBar->getScrollPos()+1);
}

void GUI_Container::render()
{
	if(!m_visible)
		return;

	Surface* renderer = g_engine.getRender();
	renderer->setClipRect(m_tRect.x1, m_tRect.y1, m_tRect.x2-12, m_tRect.y2);

	Sint32 posY = m_tRect.y1-(m_scrollBar->getScrollPos()*4)+10;
	if(m_resetPosition || posY != m_lastPosY || m_tRect.x1 != m_lastPosX)
	{
		m_drawns.clear();
		for(std::vector<GUI_Element*>::iterator it = m_childs.begin(), end = m_childs.end(); it != end; ++it)
		{
			iRect childRect = (*it)->getRect();
			iRect newChildRect;

			newChildRect.x1 = m_tRect.x1+(childRect.x1-m_lastPosX);
			newChildRect.y1 = posY+(childRect.y1-m_lastPosY);
			newChildRect.x2 = childRect.x2;
			newChildRect.y2 = childRect.y2;
			(*it)->setRect(newChildRect);

			Sint32 npy = newChildRect.y1+newChildRect.y2;
			if((newChildRect.y1 >= m_tRect.y1 && newChildRect.y1 <= m_tRect.y1+m_tRect.y2) || (npy >= m_tRect.y1 && npy <= m_tRect.y1+m_tRect.y2))
				m_drawns.push_back((*it));
		}
		m_lastPosX = m_tRect.x1;
		m_lastPosY = posY;
		m_resetPosition = false;
	}
	for(std::vector<GUI_Element*>::iterator it = m_drawns.begin(), end = m_drawns.end(); it != end; ++it)
		(*it)->render();

	renderer->disableClipRect();
	m_scrollBar->render();
}