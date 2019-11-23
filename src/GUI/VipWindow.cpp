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
#include "../GUI_Elements/GUI_StaticImage.h"
#include "../GUI_Elements/GUI_Label.h"
#include "../GUI_Elements/GUI_Icon.h"
#include "../GUI_Elements/GUI_Container.h"
#include "../game.h"

#define VIP_TITLE "VIP"
#define VIP_MAXIMINI_EVENTID 1000
#define VIP_CLOSE_EVENTID 1001
#define VIP_CONFIGURE_EVENTID 1002
#define VIP_RESIZE_WIDTH_EVENTID 1003
#define VIP_RESIZE_HEIGHT_EVENTID 1004
#define VIP_EXIT_WINDOW_EVENTID 1005
#define VIP_CONTAINER_EVENTID 1006

extern Engine g_engine;
extern Game g_game;

bool g_haveVIPOpen = false;

void vip_Events(Uint32 event, Sint32 status)
{
	switch(event)
	{
		case VIP_MAXIMINI_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_VIP);
			if(pPanel)
			{
				GUI_Panel* parent = pPanel->getParent();
				if(!parent)
					break;

				iRect& pRect = pPanel->getOriginalRect();
				if(pRect.y2 > 19)
				{
					pPanel->setCachedHeight(pRect.y2);
					pPanel->setSize(pRect.x2, 19);
					parent->checkPanels();

					GUI_Container* pContainer = SDL_static_cast(GUI_Container*, pPanel->getChild(VIP_CONTAINER_EVENTID));
					if(pContainer)
						pContainer->makeInvisible();
				}
				else
				{
					UTIL_ResizePanel(SDL_reinterpret_cast(void*, pPanel), pRect.x2, pPanel->getCachedHeight());

					GUI_Container* pContainer = SDL_static_cast(GUI_Container*, pPanel->getChild(VIP_CONTAINER_EVENTID));
					if(pContainer)
						pContainer->makeVisible();
				}
			}
		}
		break;
		case VIP_CLOSE_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_VIP);
			if(pPanel)
			{
				GUI_Panel* parent = pPanel->getParent();
				if(!parent)
					break;

				parent->removePanel(pPanel);
			}
		}
		break;
		case VIP_CONFIGURE_EVENTID:
		{
			//GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_EXITWARNING);
			//if(pWindow)
				//g_engine.removeWindow(pWindow);
		}
		break;
		case VIP_RESIZE_HEIGHT_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_VIP);
			if(pPanel)
			{
				GUI_Container* pContainer = SDL_static_cast(GUI_Container*, pPanel->getChild(VIP_CONTAINER_EVENTID));
				if(pContainer)
				{
					iRect cRect = pContainer->getRect();
					cRect.y2 = status-19;
					pContainer->setRect(cRect);
				}
			}
		}
		break;
		case VIP_EXIT_WINDOW_EVENTID: {g_engine.setContentWindowHeight(GUI_PANEL_WINDOW_VIP, status); g_haveVIPOpen = false;} break;
	}
}

void UTIL_toggleVipWindow()
{
	GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_VIP);
	if(pPanel)
	{
		g_engine.removePanelWindow(pPanel);
		return;
	}

	GUI_PanelWindow* newWindow = new GUI_PanelWindow(iRect(0, 0, 172, 117), true, GUI_PANEL_WINDOW_VIP, true);
	newWindow->setEventCallback(&vip_Events, VIP_RESIZE_WIDTH_EVENTID, VIP_RESIZE_HEIGHT_EVENTID, VIP_EXIT_WINDOW_EVENTID);
	GUI_StaticImage* newImage = new GUI_StaticImage(iRect(2, 0, GUI_UI_ICON_VIPLIST_W, GUI_UI_ICON_VIPLIST_H), GUI_UI_IMAGE, GUI_UI_ICON_VIPLIST_X, GUI_UI_ICON_VIPLIST_Y);
	newWindow->addChild(newImage);
	GUI_Icon* newIcon = new GUI_Icon(iRect(147, 0, GUI_UI_ICON_MINIMIZE_WINDOW_UP_W, GUI_UI_ICON_MINIMIZE_WINDOW_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_MINIMIZE_WINDOW_UP_X, GUI_UI_ICON_MINIMIZE_WINDOW_UP_Y, GUI_UI_ICON_MINIMIZE_WINDOW_DOWN_X, GUI_UI_ICON_MINIMIZE_WINDOW_DOWN_Y, 0, "Maximise or minimise window");
	newIcon->setButtonEventCallback(&vip_Events, VIP_MAXIMINI_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(159, 0, GUI_UI_ICON_CLOSE_WINDOW_UP_W, GUI_UI_ICON_CLOSE_WINDOW_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_CLOSE_WINDOW_UP_X, GUI_UI_ICON_CLOSE_WINDOW_UP_Y, GUI_UI_ICON_CLOSE_WINDOW_DOWN_X, GUI_UI_ICON_CLOSE_WINDOW_DOWN_Y, 0, "Close this window");
	newIcon->setButtonEventCallback(&vip_Events, VIP_CLOSE_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(131, 0, GUI_UI_ICON_CONFIGURE_WINDOW_UP_W, GUI_UI_ICON_CONFIGURE_WINDOW_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_CONFIGURE_WINDOW_UP_X, GUI_UI_ICON_CONFIGURE_WINDOW_UP_Y, GUI_UI_ICON_CONFIGURE_WINDOW_DOWN_X, GUI_UI_ICON_CONFIGURE_WINDOW_DOWN_Y, 0, "Click here to configure the skills window");
	newIcon->setButtonEventCallback(&vip_Events, VIP_CONFIGURE_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	GUI_Label* newLabel = new GUI_Label(iRect(19, 2, 0, 0), VIP_TITLE, 0, 144, 144, 144);
	newWindow->addChild(newLabel);
	GUI_Container* newContainer = new GUI_Container(iRect(2, 13, 168, 98), newWindow, VIP_CONTAINER_EVENTID);
	newContainer->startEvents();
	newWindow->addChild(newContainer);
	g_engine.addToPanel(newWindow);
	g_haveVIPOpen = true;
}
