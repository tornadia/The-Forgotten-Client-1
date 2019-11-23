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

#include "GUI_UTIL.h"
#include "../engine.h"
#include "../GUI_Elements/GUI_Panel.h"
#include "../GUI_Elements/GUI_PanelWindow.h"
#include "../GUI_Elements/GUI_Button.h"
#include "../GUI_Elements/GUI_Icon.h"
#include "../GUI_Elements/GUI_StaticImage.h"
#include "../GUI_Elements/GUI_Grouper.h"
#include "../automap.h"
#include "../game.h"
#include "Minimap.h"

#define MINIMAP_ZOOMIN_EVENTID 1000
#define MINIMAP_ZOOMOUT_EVENTID 1001
#define MINIMAP_FLOORUP_EVENTID 1002
#define MINIMAP_FLOORDOWN_EVENTID 1003
#define MINIMAP_SCROLLNORTH_EVENTID 1004
#define MINIMAP_SCROLLSOUTH_EVENTID 1005
#define MINIMAP_SCROLLEAST_EVENTID 1006
#define MINIMAP_SCROLLWEST_EVENTID 1007
#define MINIMAP_SCROLLNORTHWEST_EVENTID 1008
#define MINIMAP_SCROLLNORTHEAST_EVENTID 1009
#define MINIMAP_SCROLLSOUTHWEST_EVENTID 1010
#define MINIMAP_SCROLLSOUTHEAST_EVENTID 1011
#define MINIMAP_CENTER_EVENTID 1012

extern Engine g_engine;
extern Automap g_automap;
extern Game g_game;

void minimap_Events(Uint32 event, Sint32)
{
	switch(event)
	{
		case MINIMAP_ZOOMIN_EVENTID: g_game.minimapZoomIn(); break;
		case MINIMAP_ZOOMOUT_EVENTID: g_game.minimapZoomOut(); break;
		case MINIMAP_FLOORUP_EVENTID: g_game.minimapFloorUp(); break;
		case MINIMAP_FLOORDOWN_EVENTID: g_game.minimapFloorDown(); break;
		case MINIMAP_SCROLLNORTH_EVENTID: g_game.minimapScrollNorth(); break;
		case MINIMAP_SCROLLSOUTH_EVENTID: g_game.minimapScrollSouth(); break;
		case MINIMAP_SCROLLEAST_EVENTID: g_game.minimapScrollEast(); break;
		case MINIMAP_SCROLLWEST_EVENTID: g_game.minimapScrollWest(); break;
		case MINIMAP_SCROLLNORTHWEST_EVENTID: g_game.minimapScrollNorthWest(); break;
		case MINIMAP_SCROLLNORTHEAST_EVENTID: g_game.minimapScrollNorthEast(); break;
		case MINIMAP_SCROLLSOUTHWEST_EVENTID: g_game.minimapScrollSouthWest(); break;
		case MINIMAP_SCROLLSOUTHEAST_EVENTID: g_game.minimapScrollSouthEast(); break;
		case MINIMAP_CENTER_EVENTID: g_game.minimapCenter(); break;
	}
}

void UTIL_createMinimapPanel()
{
	GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_MINIMAP);
	if(pPanel)
		g_engine.removePanelWindow(pPanel);

	GUI_PanelWindow* newWindow = new GUI_PanelWindow(iRect(0, 0, 172, 117), false, GUI_PANEL_WINDOW_MINIMAP);
	GUI_Grouper* newGrouper = new GUI_Grouper(iRect(5, 1, 114, 114));
	newWindow->addChild(newGrouper);
	GUI_Minimap* newMinimap = new GUI_Minimap(iRect(6, 2, 112, 112));
	newMinimap->startEvents();
	newWindow->addChild(newMinimap);
	GUI_Button* newButton = new GUI_Button(iRect(124, 92, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Centre");
	newButton->setButtonEventCallback(&minimap_Events, MINIMAP_CENTER_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	GUI_StaticImage* newImage = new GUI_StaticImage(iRect(130, 10, 31, 31), GUI_UI_IMAGE, GUI_UI_ICON_MAP_TIME_X, GUI_UI_ICON_MAP_TIME_Y);
	newWindow->addChild(newImage);
	newImage = new GUI_StaticImage(iRect(124, 4, GUI_UI_ICON_MAP_ROSE_W, GUI_UI_ICON_MAP_ROSE_H), GUI_UI_IMAGE, GUI_UI_ICON_MAP_ROSE_X, GUI_UI_ICON_MAP_ROSE_Y);
	newWindow->addChild(newImage);
	GUI_Icon* newIcon = new GUI_Icon(iRect(139, 4, GUI_UI_ICON_MAP_ROSEN_UP_W, GUI_UI_ICON_MAP_ROSEN_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_MAP_ROSEN_UP_X, GUI_UI_ICON_MAP_ROSEN_UP_Y, GUI_UI_ICON_MAP_ROSEN_DOWN_X, GUI_UI_ICON_MAP_ROSEN_DOWN_Y);
	newIcon->setButtonEventCallback(&minimap_Events, MINIMAP_SCROLLNORTH_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(139, 40, GUI_UI_ICON_MAP_ROSES_UP_W, GUI_UI_ICON_MAP_ROSES_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_MAP_ROSES_UP_X, GUI_UI_ICON_MAP_ROSES_UP_Y, GUI_UI_ICON_MAP_ROSES_DOWN_X, GUI_UI_ICON_MAP_ROSES_DOWN_Y);
	newIcon->setButtonEventCallback(&minimap_Events, MINIMAP_SCROLLSOUTH_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(124, 19, GUI_UI_ICON_MAP_ROSEW_UP_W, GUI_UI_ICON_MAP_ROSEW_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_MAP_ROSEW_UP_X, GUI_UI_ICON_MAP_ROSEW_UP_Y, GUI_UI_ICON_MAP_ROSEW_DOWN_X, GUI_UI_ICON_MAP_ROSEW_DOWN_Y);
	newIcon->setButtonEventCallback(&minimap_Events, MINIMAP_SCROLLWEST_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(160, 19, GUI_UI_ICON_MAP_ROSEE_UP_W, GUI_UI_ICON_MAP_ROSEE_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_MAP_ROSEE_UP_X, GUI_UI_ICON_MAP_ROSEE_UP_Y, GUI_UI_ICON_MAP_ROSEE_DOWN_X, GUI_UI_ICON_MAP_ROSEE_DOWN_Y);
	newIcon->setButtonEventCallback(&minimap_Events, MINIMAP_SCROLLEAST_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(126, 6, GUI_UI_ICON_MAP_ROSENW_UP_W, GUI_UI_ICON_MAP_ROSENW_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_MAP_ROSENW_UP_X, GUI_UI_ICON_MAP_ROSENW_UP_Y, GUI_UI_ICON_MAP_ROSENW_DOWN_X, GUI_UI_ICON_MAP_ROSENW_DOWN_Y);
	newIcon->setButtonEventCallback(&minimap_Events, MINIMAP_SCROLLNORTHWEST_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(153, 6, GUI_UI_ICON_MAP_ROSENE_UP_W, GUI_UI_ICON_MAP_ROSENE_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_MAP_ROSENE_UP_X, GUI_UI_ICON_MAP_ROSENE_UP_Y, GUI_UI_ICON_MAP_ROSENE_DOWN_X, GUI_UI_ICON_MAP_ROSENE_DOWN_Y);
	newIcon->setButtonEventCallback(&minimap_Events, MINIMAP_SCROLLNORTHEAST_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(126, 33, GUI_UI_ICON_MAP_ROSESW_UP_W, GUI_UI_ICON_MAP_ROSESW_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_MAP_ROSESW_UP_X, GUI_UI_ICON_MAP_ROSESW_UP_Y, GUI_UI_ICON_MAP_ROSESW_DOWN_X, GUI_UI_ICON_MAP_ROSESW_DOWN_Y);
	newIcon->setButtonEventCallback(&minimap_Events, MINIMAP_SCROLLSOUTHWEST_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(153, 33, GUI_UI_ICON_MAP_ROSESE_UP_W, GUI_UI_ICON_MAP_ROSESE_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_MAP_ROSESE_UP_X, GUI_UI_ICON_MAP_ROSESE_UP_Y, GUI_UI_ICON_MAP_ROSESE_DOWN_X, GUI_UI_ICON_MAP_ROSESE_DOWN_Y);
	newIcon->setButtonEventCallback(&minimap_Events, MINIMAP_SCROLLSOUTHEAST_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(124, 71, GUI_UI_ICON_MAP_ZOOMIN_UP_W, GUI_UI_ICON_MAP_ZOOMIN_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_MAP_ZOOMIN_UP_X, GUI_UI_ICON_MAP_ZOOMIN_UP_Y, GUI_UI_ICON_MAP_ZOOMIN_DOWN_X, GUI_UI_ICON_MAP_ZOOMIN_DOWN_Y);
	newIcon->setButtonEventCallback(&minimap_Events, MINIMAP_ZOOMIN_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(124, 50, GUI_UI_ICON_MAP_ZOOMOUT_UP_W, GUI_UI_ICON_MAP_ZOOMOUT_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_MAP_ZOOMOUT_UP_X, GUI_UI_ICON_MAP_ZOOMOUT_UP_Y, GUI_UI_ICON_MAP_ZOOMOUT_DOWN_X, GUI_UI_ICON_MAP_ZOOMOUT_DOWN_Y);
	newIcon->setButtonEventCallback(&minimap_Events, MINIMAP_ZOOMOUT_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(147, 50, GUI_UI_ICON_MAP_MOVEUP_UP_W, GUI_UI_ICON_MAP_MOVEUP_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_MAP_MOVEUP_UP_X, GUI_UI_ICON_MAP_MOVEUP_UP_Y, GUI_UI_ICON_MAP_MOVEUP_DOWN_X, GUI_UI_ICON_MAP_MOVEUP_DOWN_Y);
	newIcon->setButtonEventCallback(&minimap_Events, MINIMAP_FLOORUP_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(147, 71, GUI_UI_ICON_MAP_MOVEDOWN_UP_W, GUI_UI_ICON_MAP_MOVEDOWN_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_MAP_MOVEDOWN_UP_X, GUI_UI_ICON_MAP_MOVEDOWN_UP_Y, GUI_UI_ICON_MAP_MOVEDOWN_DOWN_X, GUI_UI_ICON_MAP_MOVEDOWN_DOWN_Y);
	newIcon->setButtonEventCallback(&minimap_Events, MINIMAP_FLOORDOWN_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	g_engine.addToPanel(newWindow, GUI_PANEL_MAIN);
}

GUI_Minimap::GUI_Minimap(iRect boxRect, Uint32 internalID)
{
	setRect(boxRect);
	m_internalID = internalID;
	m_bMouseDragging = false;
	m_bMouseAutowalk = false;
}

void GUI_Minimap::onLMouseDown(Sint32 x, Sint32 y)
{
	m_bMouseAutowalk = true;
	m_bMouseDragging = true;
	m_mouseEvent.x1 = x;
	m_mouseEvent.y1 = y;
}

void GUI_Minimap::onLMouseUp(Sint32 x, Sint32 y)
{
	m_bMouseDragging = false;
	if(m_bMouseAutowalk && m_tRect.isPointInside(x, y))
	{
		MapMark* mapMark;
		Position autoWalkPosition = g_automap.getMapDetail(x-m_tRect.x1, y-m_tRect.y1, m_tRect.x2, m_tRect.y2, mapMark);
		g_game.startAutoWalk(autoWalkPosition);
	}
}

void GUI_Minimap::onWheel(Sint32, Sint32, bool wheelUP)
{
	if(wheelUP)
		g_game.minimapZoomIn();
	else
		g_game.minimapZoomOut();
}

void GUI_Minimap::onMouseMove(Sint32 x, Sint32 y, bool)
{
	if(m_bMouseDragging)
	{
		Sint32 diffX = (x-m_mouseEvent.x1)/g_automap.getZoom();
		Sint32 diffY = (y-m_mouseEvent.y1)/g_automap.getZoom();
		if(diffX != 0 || diffY != 0)
		{
			Position& pos = g_automap.getPosition();
			g_automap.setPosition(Position(SDL_static_cast(Uint16, SDL_static_cast(Sint32, pos.x)-diffX), SDL_static_cast(Uint16, SDL_static_cast(Sint32, pos.y)-diffY), pos.z));

			m_mouseEvent.x1 = x;
			m_mouseEvent.y1 = y;

			//When we move the minimap let's disable autowalk click
			m_bMouseAutowalk = false;
		}
	}
}

void GUI_Minimap::render()
{
	g_automap.render(m_tRect.x1, m_tRect.y1, m_tRect.x2, m_tRect.y2);
}
