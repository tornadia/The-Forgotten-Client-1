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
#include "../GUI_Elements/GUI_Label.h"
#include "../GUI_Elements/GUI_StaticImage.h"
#include "../map.h"
#include "../game.h"
#include "../creature.h"
#include "Inventory.h"
#include "itemUI.h"

extern Engine g_engine;
extern Map g_map;
extern Game g_game;

void UTIL_createInventoryPanel()
{
	GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_INVENTORY);
	if(pPanel)
		g_engine.removePanelWindow(pPanel);

	GUI_PanelWindow* newWindow = new GUI_PanelWindow(iRect(0, 0, 172, 170), false, GUI_PANEL_WINDOW_INVENTORY);
	GUI_Button* newButton = new GUI_Button(iRect(124, 81, 43, 20), "Stop");
	newButton->startEvents();
	newWindow->addChild(newButton);
	newButton = new GUI_Button(iRect(124, 105, 43, 20), "Quests");
	newButton->startEvents();
	newWindow->addChild(newButton);
	newButton = new GUI_Button(iRect(124, 127, 43, 20), "Options");
	newButton->startEvents();
	newWindow->addChild(newButton);
	newButton = new GUI_Button(iRect(124, 149, 43, 20), "Help");
	newButton->startEvents();
	newWindow->addChild(newButton);
	GUI_StaticImage* newImage = new GUI_StaticImage(iRect(124, 19, 20, 20), 3, 82, 118);
	newWindow->addChild(newImage);
	newImage = new GUI_StaticImage(iRect(124, 39, 20, 20), 3, 102, 98);
	newWindow->addChild(newImage);
	newImage = new GUI_StaticImage(iRect(124, 59, 20, 20), 3, 122, 98);
	newWindow->addChild(newImage);
	newImage = new GUI_StaticImage(iRect(147, 19, 20, 20), 3, 142, 118);
	newWindow->addChild(newImage);
	newImage = new GUI_StaticImage(iRect(147, 39, 20, 20), 3, 162, 98);
	newWindow->addChild(newImage);
	newImage = new GUI_StaticImage(iRect(147, 59, 20, 20), 3, 202, 98);
	newWindow->addChild(newImage);
	newImage = new GUI_StaticImage(iRect(8, 4, 12, 12), 3, 234, 98);
	newWindow->addChild(newImage);
	newImage = new GUI_StaticImage(iRect(10, 153, 9, 9), 3, 310, 182);
	newWindow->addChild(newImage);
	newImage = new GUI_StaticImage(iRect(8, 128, 34, 21), 3, 315, 32);
	newWindow->addChild(newImage);
	newImage = new GUI_StaticImage(iRect(82, 128, 34, 21), 3, 315, 32);
	newWindow->addChild(newImage);

	GUI_InventoryItem* newInventoryItem = new GUI_InventoryItem(iRect(46, 5, 32, 32), 128, 0, SLOT_HEAD);
	newWindow->addChild(newInventoryItem);
	newInventoryItem = new GUI_InventoryItem(iRect(9, 19, 32, 32), 96, 0, SLOT_NECKLACE);
	newWindow->addChild(newInventoryItem);
	newInventoryItem = new GUI_InventoryItem(iRect(83, 19, 32, 32), 160, 0, SLOT_BACKPACK);
	newWindow->addChild(newInventoryItem);
	newInventoryItem = new GUI_InventoryItem(iRect(46, 42, 32, 32), 96, 32, SLOT_ARMOR);
	newWindow->addChild(newInventoryItem);
	newInventoryItem = new GUI_InventoryItem(iRect(9, 56, 32, 32), 192, 0, SLOT_LEFT);
	newWindow->addChild(newInventoryItem);
	newInventoryItem = new GUI_InventoryItem(iRect(83, 56, 32, 32), 224, 0, SLOT_RIGHT);
	newWindow->addChild(newInventoryItem);
	newInventoryItem = new GUI_InventoryItem(iRect(46, 79, 32, 32), 128, 32, SLOT_LEGS);
	newWindow->addChild(newInventoryItem);
	newInventoryItem = new GUI_InventoryItem(iRect(46, 116, 32, 32), 224, 32, SLOT_FEET);
	newWindow->addChild(newInventoryItem);
	newInventoryItem = new GUI_InventoryItem(iRect(9, 93, 32, 32), 160, 32, SLOT_RING);
	newWindow->addChild(newInventoryItem);
	newInventoryItem = new GUI_InventoryItem(iRect(83, 93, 32, 32), 192, 32, SLOT_AMMO);
	newWindow->addChild(newInventoryItem);
	GUI_Icons* newIcons = new GUI_Icons(iRect(8, 151, 108, 13));
	newWindow->addChild(newIcons);
	/*GUI_Label* newLabel = new GUI_Label(iRect(25, 130, 0, 0), "Atk:", 0, 255, 255, 255);
	newLabel->setAlign(CLIENT_FONT_ALIGN_CENTER);
	newLabel->setFont(CLIENT_FONT_SMALL);
	newWindow->addChild(newLabel);
	newLabel = new GUI_Label(iRect(25, 140, 0, 0), "0", 0, 255, 255, 255);
	newLabel->setAlign(CLIENT_FONT_ALIGN_CENTER);
	newLabel->setFont(CLIENT_FONT_SMALL);
	newWindow->addChild(newLabel);
	newLabel = new GUI_Label(iRect(99, 130, 0, 0), "Def:", 0, 255, 255, 255);
	newLabel->setAlign(CLIENT_FONT_ALIGN_CENTER);
	newLabel->setFont(CLIENT_FONT_SMALL);
	newWindow->addChild(newLabel);
	newLabel = new GUI_Label(iRect(99, 140, 0, 0), "0", 0, 255, 255, 255);
	newLabel->setAlign(CLIENT_FONT_ALIGN_CENTER);
	newLabel->setFont(CLIENT_FONT_SMALL);
	newWindow->addChild(newLabel);*/
	g_engine.addToPanel(newWindow, GUI_PANEL_MAIN);
}

void UTIL_flashQuestsButton()
{

}

void UTIL_flashFollowButton()
{

}

GUI_Icons::GUI_Icons(iRect boxRect, Uint32 internalID)
{
	setRect(boxRect);
	m_internalID = internalID;
}

void GUI_Icons::onMouseMove(Sint32 x, Sint32 y, bool isInsideParent)
{
	if(isInsideParent && m_tRect.isPointInside(x, y))
	{
		Uint32 playerIcons = g_game.getIcons();
		iRect rect = iRect(m_tRect.x1+2, m_tRect.y1+2, 9, 9);
		if(playerIcons & ICON_POISON)
		{
			if(rect.isPointInside(x, y))
			{
				g_engine.showDescription(x, y, "You are poisoned");
				return;
			}
			rect.x1 += 10;
		}
		if(playerIcons & ICON_BURN)
		{
			if(rect.isPointInside(x, y))
			{
				g_engine.showDescription(x, y, "You are burning");
				return;
			}
			rect.x1 += 10;
		}
		if(playerIcons & ICON_ENERGY)
		{
			if(rect.isPointInside(x, y))
			{
				g_engine.showDescription(x, y, "You are electrified");
				return;
			}
			rect.x1 += 10;
		}
		if(playerIcons & ICON_SWORDS)
		{
			if(rect.isPointInside(x, y))
			{
				g_engine.showDescription(x, y, "You may not logout during a fight");
				return;
			}
			rect.x1 += 10;
		}
		if(playerIcons & ICON_DRUNK)
		{
			if(rect.isPointInside(x, y))
			{
				g_engine.showDescription(x, y, "You are drunk");
				return;
			}
			rect.x1 += 10;
		}
		if(playerIcons & ICON_MANASHIELD)
		{
			if(rect.isPointInside(x, y))
			{
				g_engine.showDescription(x, y, "You are protected by a magic shield");
				return;
			}
			rect.x1 += 10;
		}
		if(playerIcons & ICON_HASTE)
		{
			if(rect.isPointInside(x, y))
			{
				g_engine.showDescription(x, y, "You are hasted");
				return;
			}
			rect.x1 += 10;
		}
		if(playerIcons & ICON_PARALYZE)
		{
			if(rect.isPointInside(x, y))
			{
				g_engine.showDescription(x, y, "You are paralysed");
				return;
			}
			rect.x1 += 10;
		}
		if(playerIcons & ICON_DROWNING)
		{
			if(rect.isPointInside(x, y))
			{
				g_engine.showDescription(x, y, "You are drowning");
				return;
			}
			rect.x1 += 10;
		}
		if(playerIcons & ICON_FREEZING)
		{
			if(rect.isPointInside(x, y))
			{
				g_engine.showDescription(x, y, "You are freezing");
				return;
			}
			rect.x1 += 10;
		}
		if(playerIcons & ICON_DAZZLED)
		{
			if(rect.isPointInside(x, y))
			{
				g_engine.showDescription(x, y, "You are dazzled");
				return;
			}
			rect.x1 += 10;
		}
		if(playerIcons & ICON_CURSED)
		{
			if(rect.isPointInside(x, y))
			{
				g_engine.showDescription(x, y, "You are cursed");
				return;
			}
			rect.x1 += 10;
		}
		if(playerIcons & ICON_PARTY_BUFF)
		{
			if(rect.isPointInside(x, y))
			{
				g_engine.showDescription(x, y, "You are strengthened");
				return;
			}
			rect.x1 += 10;
		}
		if(playerIcons & ICON_REDSWORDS)
		{
			if(rect.isPointInside(x, y))
			{
				g_engine.showDescription(x, y, "You may not logout or enter a protection zone");
				return;
			}
			rect.x1 += 10;
		}
		if(playerIcons & ICON_PIGEON)
		{
			if(rect.isPointInside(x, y))
			{
				g_engine.showDescription(x, y, "You are within a protection zone");
				return;
			}
			rect.x1 += 10;
		}
		if(playerIcons & ICON_BLEEDING)
		{
			if(rect.isPointInside(x, y))
			{
				g_engine.showDescription(x, y, "You are bleeding");
				return;
			}
			rect.x1 += 10;
		}

		Creature* localPlayer = g_map.getLocalCreature();
		if(!localPlayer)
			return;

		Uint8 playerSkull = localPlayer->getSkull();
		switch(playerSkull)
		{
			case SKULL_YELLOW:
			{
				if(rect.isPointInside(x, y))
				{
					g_engine.showDescription(x, y, "You may be attacked by other players");
					return;
				}
				rect.x1 += 10;
			}
			break;
			case SKULL_GREEN:
			{
				if(rect.isPointInside(x, y))
				{
					g_engine.showDescription(x, y, "You are a member of a party");
					return;
				}
				rect.x1 += 10;
			}
			break;
			case SKULL_WHITE:
			{
				if(rect.isPointInside(x, y))
				{
					g_engine.showDescription(x, y, "You have attacked an unmarked player");
					return;
				}
				rect.x1 += 10;
			}
			break;
			case SKULL_RED:
			{
				if(rect.isPointInside(x, y))
				{
					g_engine.showDescription(x, y, "You have killed too many unmarked players");
					return;
				}
				rect.x1 += 10;
			}
			break;
			case SKULL_BLACK:
			{
				if(rect.isPointInside(x, y))
				{
					g_engine.showDescription(x, y, "You have excessively killed unmarked players");
					return;
				}
				rect.x1 += 10;
			}
			break;
			case SKULL_ORANGE:
			{
				if(rect.isPointInside(x, y))
				{
					g_engine.showDescription(x, y, "You may suffer revenge from your former victim");
					return;
				}
				rect.x1 += 10;
			}
			break;
			default: break;
		}

		Uint8 playerEmblem = localPlayer->getEmblem();
		if(playerEmblem != GUILDEMBLEM_NONE && playerEmblem <= GUILDEMBLEM_NEUTRAL)
		{
			if(rect.isPointInside(x, y))
			{
				g_engine.showDescription(x, y, "You are involved in a guild war");
				return;
			}
			rect.x1 += 10;
		}
	}
}

void GUI_Icons::render()
{
	Surface* renderer = g_engine.getRender();
	renderer->drawPicture(3, 98, 240, m_tRect.x1, m_tRect.y1, m_tRect.x2, m_tRect.y2);
	
	Sint32 posX = m_tRect.x1+2;
	Sint32 posY = m_tRect.y1+2;
	Uint32 playerIcons = g_game.getIcons();
	if(playerIcons & ICON_POISON)
	{
		renderer->drawPicture(3, 279, 32, posX, posY, 9, 9);
		posX += 10;
	}
	if(playerIcons & ICON_BURN)
	{
		renderer->drawPicture(3, 288, 32, posX, posY, 9, 9);
		posX += 10;
	}
	if(playerIcons & ICON_ENERGY)
	{
		renderer->drawPicture(3, 297, 32, posX, posY, 9, 9);
		posX += 10;
	}
	if(playerIcons & ICON_SWORDS)
	{
		renderer->drawPicture(3, 306, 32, posX, posY, 9, 9);
		posX += 10;
	}
	if(playerIcons & ICON_DRUNK)
	{
		renderer->drawPicture(3, 279, 41, posX, posY, 9, 9);
		posX += 10;
	}
	if(playerIcons & ICON_MANASHIELD)
	{
		renderer->drawPicture(3, 288, 41, posX, posY, 9, 9);
		posX += 10;
	}
	if(playerIcons & ICON_HASTE)
	{
		renderer->drawPicture(3, 297, 41, posX, posY, 9, 9);
		posX += 10;
	}
	if(playerIcons & ICON_PARALYZE)
	{
		renderer->drawPicture(3, 306, 41, posX, posY, 9, 9);
		posX += 10;
	}
	if(playerIcons & ICON_DROWNING)
	{
		renderer->drawPicture(3, 279, 59, posX, posY, 9, 9);
		posX += 10;
	}
	if(playerIcons & ICON_FREEZING)
	{
		renderer->drawPicture(3, 279, 68, posX, posY, 9, 9);
		posX += 10;
	}
	if(playerIcons & ICON_DAZZLED)
	{
		renderer->drawPicture(3, 279, 77, posX, posY, 9, 9);
		posX += 10;
	}
	if(playerIcons & ICON_CURSED)
	{
		renderer->drawPicture(3, 279, 86, posX, posY, 9, 9);
		posX += 10;
	}
	if(playerIcons & ICON_PARTY_BUFF)
	{
		renderer->drawPicture(3, 307, 148, posX, posY, 9, 9);
		posX += 10;
	}
	if(playerIcons & ICON_REDSWORDS)
	{
		renderer->drawPicture(3, 310, 191, posX, posY, 9, 9);
		posX += 10;
	}
	if(playerIcons & ICON_PIGEON)
	{
		renderer->drawPicture(3, 310, 182, posX, posY, 9, 9);
		posX += 10;
	}
	if(playerIcons & ICON_BLEEDING)
	{
		renderer->drawPicture(3, 322, 0, posX, posY, 9, 9);
		posX += 10;
	}

	Creature* localPlayer = g_map.getLocalCreature();
	if(!localPlayer)
		return;

	Uint8 playerSkull = localPlayer->getSkull();
	switch(playerSkull)
	{
		case SKULL_YELLOW:
		{
			renderer->drawPicture(3, 288, 50, posX, posY, 9, 9);
			posX += 10;
		}
		break;
		case SKULL_GREEN:
		{
			renderer->drawPicture(3, 279, 50, posX, posY, 9, 9);
			posX += 10;
		}
		break;
		case SKULL_WHITE:
		{
			renderer->drawPicture(3, 297, 50, posX, posY, 9, 9);
			posX += 10;
		}
		break;
		case SKULL_RED:
		{
			renderer->drawPicture(3, 306, 50, posX, posY, 9, 9);
			posX += 10;
		}
		break;
		case SKULL_BLACK:
		{
			renderer->drawPicture(3, 342, 200, posX, posY, 9, 9);
			posX += 10;
		}
		break;
		case SKULL_ORANGE:
		{
			renderer->drawPicture(3, 242, 218, posX, posY, 9, 9);
			posX += 10;
		}
		break;
		default: break;
	}

	Uint8 playerEmblem = localPlayer->getEmblem();
	if(playerEmblem != GUILDEMBLEM_NONE && playerEmblem <= GUILDEMBLEM_NEUTRAL)
	{
		renderer->drawPicture(3, 251, 218, posX, posY, 9, 9);
		posX += 10;
	}
}

GUI_InventoryItem::GUI_InventoryItem(iRect boxRect, Sint32 skinX, Sint32 skinY, Uint8 slot, Uint32 internalID)
{
	setRect(boxRect);
	m_skinX = skinX;
	m_skinY = skinY;
	m_slot = slot;
	m_internalID = internalID;
}

void GUI_InventoryItem::render()
{
	Surface* renderer = g_engine.getRender();
	renderer->drawPicture(3, 186, 64, m_tRect.x1-1, m_tRect.y1-1, m_tRect.x2+2, m_tRect.y2+2);

	ItemUI* item = g_game.getInventoryItem(m_slot);
	if(item)
		item->render(m_tRect.x1, m_tRect.y1, m_tRect.y2);
	else
		renderer->drawPicture(3, m_skinX, m_skinY, m_tRect.x1, m_tRect.y1, m_tRect.x2, m_tRect.y2);
}