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
#include "../GUI_Elements/GUI_ContextMenu.h"
#include "../game.h"
#include "../map.h"
#include "../creature.h"
#include "BattleWindow.h"

#define BATTLE_TITLE "Battle List"
#define BATTLE_MAXIMINI_EVENTID 1000
#define BATTLE_CLOSE_EVENTID 1001
#define BATTLE_CONFIGURE_EVENTID 1002
#define BATTLE_RESIZE_WIDTH_EVENTID 1003
#define BATTLE_RESIZE_HEIGHT_EVENTID 1004
#define BATTLE_EXIT_WINDOW_EVENTID 1005
#define BATTLE_CONTAINER_EVENTID 1006

bool g_recreateBattleWindow = false;
std::vector<Creature*> g_battleCreatures;

extern Engine g_engine;
extern Game g_game;
extern Map g_map;

void battle_Events(Uint32 event, Sint32 status)
{
	switch(event)
	{
		case BATTLE_MAXIMINI_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_BATTLE);
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

					GUI_Container* pContainer = SDL_static_cast(GUI_Container*, pPanel->getChild(BATTLE_CONTAINER_EVENTID));
					if(pContainer)
						pContainer->makeInvisible();
				}
				else
				{
					UTIL_ResizePanel(SDL_reinterpret_cast(void*, pPanel), pRect.x2, pPanel->getCachedHeight());

					GUI_Container* pContainer = SDL_static_cast(GUI_Container*, pPanel->getChild(BATTLE_CONTAINER_EVENTID));
					if(pContainer)
						pContainer->makeVisible();
				}
			}
		}
		break;
		case BATTLE_CLOSE_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_BATTLE);
			if(pPanel)
			{
				GUI_Panel* parent = pPanel->getParent();
				if(!parent)
					break;

				parent->removePanel(pPanel);
			}
		}
		break;
		case BATTLE_CONFIGURE_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_BATTLE);
			if(pPanel)
			{
				GUI_Icon* pIcon = SDL_static_cast(GUI_Icon*, pPanel->getChild(BATTLE_CONTAINER_EVENTID));
				if(pIcon)
				{
					iRect& iconRect = pIcon->getRect();
					UTIL_createBattlePopupMenu(NULL, iconRect.x1, iconRect.y1+12);
				}
			}
		}
		break;
		case BATTLE_RESIZE_HEIGHT_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_BATTLE);
			if(pPanel)
			{
				GUI_Container* pContainer = SDL_static_cast(GUI_Container*, pPanel->getChild(BATTLE_CONTAINER_EVENTID));
				if(pContainer)
				{
					iRect cRect = pContainer->getRect();
					cRect.y2 = status-19;
					pContainer->setRect(cRect);
				}
			}
		}
		break;
		case BATTLE_EXIT_WINDOW_EVENTID: g_engine.setContentWindowHeight(GUI_PANEL_WINDOW_BATTLE, status); break;
	}
}

struct AscByTime {bool operator()(Creature*, Creature*) {return false;}};
struct DescByTime {bool operator()(Creature*, Creature*) {return false;}};
struct AscByDistance {bool operator()(Creature* a, Creature* b) {return UTIL_max<Sint32>(Position::getDistanceX(a->getCurrentPosition(), g_map.getCentralPosition()), Position::getDistanceY(a->getCurrentPosition(), g_map.getCentralPosition())) < UTIL_max<Sint32>(Position::getDistanceX(b->getCurrentPosition(), g_map.getCentralPosition()), Position::getDistanceY(b->getCurrentPosition(), g_map.getCentralPosition()));}};
struct DescByDistance {bool operator()(Creature* a, Creature* b) {return UTIL_max<Sint32>(Position::getDistanceX(a->getCurrentPosition(), g_map.getCentralPosition()), Position::getDistanceY(a->getCurrentPosition(), g_map.getCentralPosition())) > UTIL_max<Sint32>(Position::getDistanceX(b->getCurrentPosition(), g_map.getCentralPosition()), Position::getDistanceY(b->getCurrentPosition(), g_map.getCentralPosition()));}};
struct AscByHP {bool operator()(Creature* a, Creature* b) {return a->getHealth() < b->getHealth();}};
struct DescByHP {bool operator()(Creature* a, Creature* b) {return a->getHealth() > b->getHealth();}};
struct AscByName {bool operator()(Creature* a, Creature* b) {return a->getName() < b->getName();}};
struct DescByName {bool operator()(Creature* a, Creature* b) {return a->getName() > b->getName();}};

void UTIL_sortBattleWindow(SortMethods method)
{
	switch(method)
	{
		case Sort_Ascending_Time: std::sort(g_battleCreatures.begin(), g_battleCreatures.end(), AscByTime()); break;
		case Sort_Descending_Time: std::sort(g_battleCreatures.begin(), g_battleCreatures.end(), DescByTime()); break;
		case Sort_Ascending_Distance: std::sort(g_battleCreatures.begin(), g_battleCreatures.end(), AscByDistance()); break;
		case Sort_Descending_Distance: std::sort(g_battleCreatures.begin(), g_battleCreatures.end(), DescByDistance()); break;
		case Sort_Ascending_HP: std::sort(g_battleCreatures.begin(), g_battleCreatures.end(), AscByHP()); break;
		case Sort_Descending_HP: std::sort(g_battleCreatures.begin(), g_battleCreatures.end(), DescByHP()); break;
		case Sort_Ascending_Name: std::sort(g_battleCreatures.begin(), g_battleCreatures.end(), AscByName()); break;
		default: std::sort(g_battleCreatures.begin(), g_battleCreatures.end(), DescByName()); break;
	}
	g_recreateBattleWindow = true;
}

void UTIL_recreateBattleWindow(GUI_Container* container)
{
	if(!container)
	{
		GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_BATTLE);
		if(pPanel)
			container = SDL_static_cast(GUI_Container*, pPanel->getChild(BATTLE_CONTAINER_EVENTID));
		if(!container)
			return;
	}
	container->clearChilds();
	Sint32 PosY = 0;
	size_t creatures = g_battleCreatures.size();
	for(size_t i = 0; i < creatures; ++i)
	{
		GUI_BattleCreature* newBattleCreature = new GUI_BattleCreature(iRect(5, PosY, 145, 20), i);
		container->addChild(newBattleCreature);
		PosY += 23;
	}
}

void UTIL_flashBattleWindow()
{
	//16 blinks: 255, 160, 0
}

void UTIL_createBattleWindow()
{
	GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_BATTLE);
	if(pPanel)
		g_engine.removePanelWindow(pPanel);

	GUI_PanelWindow* newWindow = new GUI_PanelWindow(iRect(0, 0, 172, 117), true, GUI_PANEL_WINDOW_BATTLE, true);
	newWindow->setEventCallback(&battle_Events, BATTLE_RESIZE_WIDTH_EVENTID, BATTLE_RESIZE_HEIGHT_EVENTID, BATTLE_EXIT_WINDOW_EVENTID);
	GUI_BattleOptimizer* newBattleOptimizer = new GUI_BattleOptimizer(iRect(19, 2, 0, 0));
	newWindow->addChild(newBattleOptimizer);
	GUI_StaticImage* newImage = new GUI_StaticImage(iRect(2, 0, 12, 12), 3, 289, 60);
	newWindow->addChild(newImage);
	GUI_Icon* newIcon = new GUI_Icon(iRect(147, 0, 12, 12), 3, 234, 98, 234, 110, 0, "Maximise or minimise window");
	newIcon->setButtonEventCallback(&battle_Events, BATTLE_MAXIMINI_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(159, 0, 12, 12), 3, 222, 98, 222, 110, 0, "Close this window");
	newIcon->setButtonEventCallback(&battle_Events, BATTLE_CLOSE_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(131, 0, 12, 12), 3, 208, 260, 220, 260, 0, "Click here to configure the battle list");
	newIcon->setButtonEventCallback(&battle_Events, BATTLE_CONFIGURE_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	GUI_Label* newLabel = new GUI_Label(iRect(19, 2, 0, 0), BATTLE_TITLE, 0, 144, 144, 144);
	newWindow->addChild(newLabel);
	GUI_Container* newContainer = new GUI_Container(iRect(2, 13, 168, 98), newWindow, BATTLE_CONTAINER_EVENTID);
	UTIL_recreateBattleWindow(newContainer);
	newContainer->startEvents();
	newWindow->addChild(newContainer);
	g_engine.addToPanel(newWindow);
	g_recreateBattleWindow = false;
}

void UTIL_createBattlePopupMenu(void* data, Sint32 x, Sint32 y)
{
	Creature* creature = SDL_reinterpret_cast(Creature*, data);
	GUI_ContextMenu* newMenu = g_engine.createThingContextMenu(creature, NULL, NULL);
	newMenu->addSeparator();
	newMenu->addContextMenu(CONTEXTMENU_STYLE_CHECKED, 0, "Sort Ascending by Display Time", "");
	newMenu->addContextMenu(CONTEXTMENU_STYLE_UNCHECKED, 0, "Sort Descending by Display Time", "");
	newMenu->addContextMenu(CONTEXTMENU_STYLE_UNCHECKED, 0, "Sort Ascending by Distance", "");
	newMenu->addContextMenu(CONTEXTMENU_STYLE_UNCHECKED, 0, "Sort Descending by Distance", "");
	newMenu->addContextMenu(CONTEXTMENU_STYLE_UNCHECKED, 0, "Sort Ascending by Hit Points", "");
	newMenu->addContextMenu(CONTEXTMENU_STYLE_UNCHECKED, 0, "Sort Descending by Hit Points", "");
	newMenu->addContextMenu(CONTEXTMENU_STYLE_UNCHECKED, 0, "Sort Ascending by Name", "");
	newMenu->addContextMenu(CONTEXTMENU_STYLE_UNCHECKED, 0, "Sort Descending by Name", "");
	newMenu->setEventCallback(&battle_Events);
	g_engine.showContextMenu(newMenu, x, y);
}

GUI_BattleOptimizer::GUI_BattleOptimizer(iRect boxRect, Uint32 internalID)
{
	setRect(boxRect);
	m_internalID = internalID;
}

void GUI_BattleOptimizer::render()
{
	if(g_recreateBattleWindow)
	{
		UTIL_recreateBattleWindow(NULL);
		g_recreateBattleWindow = false;
	}
}

GUI_BattleCreature::GUI_BattleCreature(iRect boxRect, size_t index, Uint32 internalID)
{
	setRect(boxRect);
	m_internalID = internalID;
	m_index = index;
}

void GUI_BattleCreature::render()
{
	g_engine.getRender()->fillRectangle(m_tRect.x1, m_tRect.y1, m_tRect.x2, m_tRect.y2, 176, 176, 176, 255);
}
