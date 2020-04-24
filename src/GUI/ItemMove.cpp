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

#include "GUI_UTIL.h"
#include "../engine.h"
#include "../game.h"
#include "../GUI_Elements/GUI_Window.h"
#include "../GUI_Elements/GUI_Button.h"
#include "../GUI_Elements/GUI_Separator.h"
#include "../GUI_Elements/GUI_ScrollBar.h"
#include "../GUI_Elements/GUI_Label.h"
#include "ItemMove.h"

#define ITEM_MOVE_TITLE "Move Items"
#define ITEM_MOVE_WIDTH 245
#define ITEM_MOVE_HEIGHT 130
#define ITEM_MOVE_CANCEL_EVENTID 1000
#define ITEM_MOVE_OK_EVENTID 1001
#define ITEM_MOVE_ACTIVATE_EVENTID 1002
#define ITEM_MOVE_LABEL_TITLE "Set or type the number of\nitems you want to move."
#define ITEM_MOVE_LABEL_X 60
#define ITEM_MOVE_LABEL_Y 32
#define ITEM_MOVE_SCROLLBAR_X 60
#define ITEM_MOVE_SCROLLBAR_Y 60
#define ITEM_MOVE_SCROLLBAR_W 169
#define ITEM_MOVE_SCROLLBAR_H 12
#define ITEM_MOVE_SCROLLBAR_EVENTID 1003
#define ITEM_MOVE_SCROLLBAR_SET_EVENTID 1004
#define ITEM_MOVE_ITEM_X 17
#define ITEM_MOVE_ITEM_Y 30
#define ITEM_MOVE_ITEM_W 32
#define ITEM_MOVE_ITEM_H 32
#define ITEM_MOVE_ITEM_EVENTID 1005

extern Engine g_engine;
extern Game g_game;
extern Uint32 g_frameTime;

void item_move_Events(Uint32 event, Sint32 status)
{
	switch(event)
	{
		case ITEM_MOVE_CANCEL_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_ITEMMOVE)
				g_engine.removeWindow(pWindow);
		}
		break;
		case ITEM_MOVE_OK_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_ITEMMOVE)
			{
				Uint16 itemCount = 1;
				GUI_HScrollBar* pHScrollBar = SDL_static_cast(GUI_HScrollBar*, pWindow->getChild(ITEM_MOVE_SCROLLBAR_EVENTID));
				if(pHScrollBar)
					itemCount = SDL_static_cast(Uint16, pHScrollBar->getScrollPos()) + 1;

				ClientActionData& actionDataFirst = g_engine.getActionData(CLIENT_ACTION_FIRST);
				ClientActionData& actionDataSecond = g_engine.getActionData(CLIENT_ACTION_SECOND);
				g_game.sendMove(Position(actionDataFirst.posX, actionDataFirst.posY, actionDataFirst.posZ), actionDataFirst.itemId, actionDataFirst.posStack, Position(actionDataSecond.posX, actionDataSecond.posY, actionDataSecond.posZ), itemCount);

				g_engine.removeWindow(pWindow);
			}
		}
		break;
		case ITEM_MOVE_ACTIVATE_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_ITEMMOVE)
			{
				GUI_ItemMove* pItemMove = SDL_static_cast(GUI_ItemMove*, pWindow->getChild(ITEM_MOVE_ITEM_EVENTID));
				if(pItemMove)
					pWindow->setActiveElement(pItemMove);
			}
		}
		break;
		case ITEM_MOVE_SCROLLBAR_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_ITEMMOVE)
			{
				GUI_ItemMove* pItemMove = SDL_static_cast(GUI_ItemMove*, pWindow->getChild(ITEM_MOVE_ITEM_EVENTID));
				if(pItemMove)
					pItemMove->setItemCount(SDL_static_cast(Uint16, status) + 1);
			}
		}
		break;
		case ITEM_MOVE_SCROLLBAR_SET_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_ITEMMOVE)
			{
				GUI_HScrollBar* pHScrollBar = SDL_static_cast(GUI_HScrollBar*, pWindow->getChild(ITEM_MOVE_SCROLLBAR_EVENTID));
				if(pHScrollBar)
					pHScrollBar->setScrollPos(status);
			}
		}
		break;
	}
}

void UTIL_createItemMove()
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_ITEMMOVE);
	if(pWindow)
		g_engine.removeWindow(pWindow);

	ClientActionData& actionDataFirst = g_engine.getActionData(CLIENT_ACTION_FIRST);
	ClientActionData& actionDataSecond = g_engine.getActionData(CLIENT_ACTION_SECOND);

	GUI_Window* newWindow = new GUI_Window(iRect(0, 0, ITEM_MOVE_WIDTH, ITEM_MOVE_HEIGHT), ITEM_MOVE_TITLE, GUI_WINDOW_ITEMMOVE);
	GUI_ItemMove* newItemMove = new GUI_ItemMove(iRect(ITEM_MOVE_ITEM_X, ITEM_MOVE_ITEM_Y, ITEM_MOVE_ITEM_W, ITEM_MOVE_ITEM_H), actionDataFirst.itemId, actionDataSecond.itemId, ITEM_MOVE_ITEM_EVENTID);
	newWindow->addChild(newItemMove);
	GUI_HScrollBar* newHScrollBar = new GUI_HScrollBar(iRect(ITEM_MOVE_SCROLLBAR_X, ITEM_MOVE_SCROLLBAR_Y, ITEM_MOVE_SCROLLBAR_W, ITEM_MOVE_SCROLLBAR_H), SDL_static_cast(Sint32, actionDataSecond.itemId) - 1, SDL_static_cast(Sint32, actionDataSecond.itemId) - 1, ITEM_MOVE_SCROLLBAR_EVENTID);
	newHScrollBar->setBarEventCallback(&item_move_Events, ITEM_MOVE_SCROLLBAR_EVENTID);
	newHScrollBar->startEvents();
	newWindow->addChild(newHScrollBar);
	GUI_Label* newLabel = new GUI_Label(iRect(ITEM_MOVE_LABEL_X, ITEM_MOVE_LABEL_Y, 0, 0), ITEM_MOVE_LABEL_TITLE);
	newWindow->addChild(newLabel);
	GUI_Button* newButton = new GUI_Button(iRect(ITEM_MOVE_WIDTH - 56, ITEM_MOVE_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Cancel", CLIENT_GUI_ESCAPE_TRIGGER);
	newButton->setButtonEventCallback(&item_move_Events, ITEM_MOVE_CANCEL_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	newButton = new GUI_Button(iRect(ITEM_MOVE_WIDTH - 109, ITEM_MOVE_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Ok", CLIENT_GUI_ENTER_TRIGGER);
	newButton->setButtonEventCallback(&item_move_Events, ITEM_MOVE_OK_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	GUI_Separator* newSeparator = new GUI_Separator(iRect(13, ITEM_MOVE_HEIGHT - 40, ITEM_MOVE_WIDTH - 26, 2));
	newWindow->addChild(newSeparator);
	g_engine.addWindow(newWindow, true);
}

GUI_ItemMove::GUI_ItemMove(iRect boxRect, Uint16 itemId, Uint16 itemCount, Uint32 internalID)
{
	setRect(boxRect);
	m_internalID = internalID;
	m_item = ItemUI::createItemUI(itemId, 1);
	if(m_item)
		m_item->setSubtype(itemCount, true);

	m_lastKey = g_frameTime;
	m_calculate = 0;
}

GUI_ItemMove::~GUI_ItemMove()
{
	if(m_item)
		delete m_item;
}

void GUI_ItemMove::onKeyDown(SDL_Event& event)
{
	if(event.key.keysym.mod == KMOD_NONE)
	{
		switch(event.key.keysym.sym)
		{
			case SDLK_0:
			case SDLK_1:
			case SDLK_2:
			case SDLK_3:
			case SDLK_4:
			case SDLK_5:
			case SDLK_6:
			case SDLK_7:
			case SDLK_8:
			case SDLK_9:
			{
				if(g_frameTime - m_lastKey >= 500)
					m_calculate = 0;
				else
					m_calculate *= 10;

				m_lastKey = g_frameTime;
				m_calculate += SDL_static_cast(Uint32, event.key.keysym.sym - SDLK_0);
				if(m_calculate >= 100000)//Make sure we don't overflow our value
					m_calculate = 99990;

				UTIL_SafeEventHandler(&item_move_Events, ITEM_MOVE_SCROLLBAR_SET_EVENTID, SDL_static_cast(Sint32, m_calculate) - 1);
			}
			break;
		}
	}
}

void GUI_ItemMove::setItemCount(Uint16 itemCount)
{
	if(m_item)
		m_item->setSubtype(itemCount, true);
}

void GUI_ItemMove::render()
{
	if(!isActive())
		UTIL_SafeEventHandler(&item_move_Events, ITEM_MOVE_ACTIVATE_EVENTID, 1);

	g_engine.getRender()->drawPicture(GUI_UI_IMAGE, GUI_UI_INVENTORY_EMPTY_X, GUI_UI_INVENTORY_EMPTY_Y, m_tRect.x1 - 1, m_tRect.y1 - 1, m_tRect.x2 + 2, m_tRect.y2 + 2);
	if(m_item)
		m_item->render(m_tRect.x1, m_tRect.y1, m_tRect.y2);
}
