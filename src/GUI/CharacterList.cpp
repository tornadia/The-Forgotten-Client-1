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
#include "../GUI_Elements/GUI_Window.h"
#include "../GUI_Elements/GUI_Button.h"
#include "../GUI_Elements/GUI_Separator.h"
#include "../GUI_Elements/GUI_ListBox.h"
#include "../GUI_Elements/GUI_Label.h"
#include "../GUI_Elements/GUI_CharacterView.h"

#define CHARACTERLIST_TITLE "Select Character"
#define CHARACTERLIST_WIDTH 236
#define CHARACTERLIST_HEIGHT1 294
#define CHARACTERLIST_HEIGHT2 384
#define CHARACTERLIST_CANCEL_EVENTID 1000
#define CHARACTERLIST_OK_EVENTID 1001
#define CHARACTERLIST_CHARACTERS_EVENTID 1002

extern Engine g_engine;

void characterlist_Events(Uint32 event, Sint32)
{
	switch(event)
	{
		case CHARACTERLIST_CANCEL_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_CHARACTERLIST)
			{
				g_engine.removeWindow(pWindow);
				g_engine.releaseConnection();
			}
		}
		break;
		case CHARACTERLIST_OK_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_CHARACTERLIST)
			{
				g_engine.removeWindow(pWindow);

				UTIL_messageBox("Connecting", "Connecting to the game world. Please wait.");
				g_engine.issueNewConnection(true);
			}
		}
		break;
		case CHARACTERLIST_CHARACTERS_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_CHARACTERLIST)
			{
				GUI_ListBox* pListBox = SDL_static_cast(GUI_ListBox*, pWindow->getChild(CHARACTERLIST_CHARACTERS_EVENTID));
				if(pListBox)
					g_engine.setCharacterSelectId(pListBox->getSelect());
			}
		}
		break;
	}
}

void UTIL_createCharacterList()
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_CHARACTERLIST);
	if(pWindow)
		g_engine.removeWindow(pWindow);

	std::vector<CharacterDetail>& accountCharList = g_engine.getAccountCharList();
	Uint32 accountPremDays = g_engine.getAccountPremDays();
	Sint32 lastSelectId = g_engine.getCharacterSelectId();
	Uint8 accountStatus = g_engine.getAccountStatus();
	Uint8 accountSubstatus = g_engine.getAccountSubstatus();
	bool newCharacterList = g_engine.getAccountNewCharList();

	GUI_Window* newWindow = new GUI_Window(iRect(0, 0, CHARACTERLIST_WIDTH, (newCharacterList ? CHARACTERLIST_HEIGHT2 : CHARACTERLIST_HEIGHT1)), CHARACTERLIST_TITLE, GUI_WINDOW_CHARACTERLIST);
	GUI_Label* newLabel = new GUI_Label(iRect(18, 34, 0, 0), "Select Character:");
	newWindow->addChild(newLabel);
	newLabel = new GUI_Label(iRect(18, (newCharacterList ? 298 : 208), 0, 0), "Account Status:");
	newWindow->addChild(newLabel);
	if(accountStatus == AccountStatus_Frozen)
	{
		newLabel = new GUI_Label(iRect(18, (newCharacterList ? 312 : 222), 0, 0), "Frozen Account", 0, 248, 96, 96);
		newWindow->addChild(newLabel);
	}
	else if(accountStatus == AccountStatus_Suspended)
	{
		newLabel = new GUI_Label(iRect(18, (newCharacterList ? 312 : 222), 0, 0), "Suspended Account", 0, 248, 96, 96);
		newWindow->addChild(newLabel);
	}
	else if(accountSubstatus == SubscriptionStatus_Premium)
	{
		if(accountPremDays == 0)
		{
			newLabel = new GUI_Label(iRect(18, (newCharacterList ? 312 : 222), 0, 0), "Gratis Premium Account");
			newWindow->addChild(newLabel);
		}
		else
		{
			SDL_snprintf(g_buffer, sizeof(g_buffer), "Premium Account (%u day%s left)", accountPremDays, (accountPremDays > 1 ? "s" : ""));
			newLabel = new GUI_Label(iRect(18, (newCharacterList ? 312 : 222), 0, 0), g_buffer);
			newWindow->addChild(newLabel);
			if(accountPremDays <= 5)
			{
				//Overwrite the days left part as red
				SDL_snprintf(g_buffer, sizeof(g_buffer), "%u day%s left", accountPremDays, (accountPremDays > 1 ? "s" : ""));
				newLabel = new GUI_Label(iRect(126, (newCharacterList ? 312 : 222), 0, 0), g_buffer, 0, 248, 96, 96);
				newWindow->addChild(newLabel);
			}
		}
	}
	else
	{
		newLabel = new GUI_Label(iRect(18, (newCharacterList ? 312 : 222), 0, 0), "Free Account");
		newWindow->addChild(newLabel);
	}
	GUI_ListBox* newListBox = new GUI_ListBox(iRect(18, 47, 200, 146), CHARACTERLIST_CHARACTERS_EVENTID);
	for(std::vector<CharacterDetail>::iterator it = accountCharList.begin(), end = accountCharList.end(); it != end; ++it)
	{
		CharacterDetail& character = (*it);
		SDL_snprintf(g_buffer, sizeof(g_buffer), "%s (%s)", character.name.c_str(), character.worldName.c_str());
		newListBox->add(g_buffer);
	}
	newListBox->setSelect(lastSelectId);
	newListBox->setEventCallback(&characterlist_Events, CHARACTERLIST_CHARACTERS_EVENTID);
	newListBox->startEvents();
	newWindow->addChild(newListBox);
	if(newCharacterList)
	{
		GUI_CharacterView* newCharacterView = new GUI_CharacterView(iRect(18, 200, 200, 82));
		newWindow->addChild(newCharacterView);
	}
	GUI_Button* newButton = new GUI_Button(iRect(180, (newCharacterList ? 355 : 265), 43, 20), "Cancel", CLIENT_GUI_ESCAPE_TRIGGER);
	newButton->setButtonEventCallback(&characterlist_Events, CHARACTERLIST_CANCEL_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	newButton = new GUI_Button(iRect(127, (newCharacterList ? 355 : 265), 43, 20), "Ok", CLIENT_GUI_ENTER_TRIGGER);
	newButton->setButtonEventCallback(&characterlist_Events, CHARACTERLIST_OK_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	GUI_Separator* newSeparator = new GUI_Separator(iRect(13, (newCharacterList ? 345 : 255), 210, 2));
	newWindow->addChild(newSeparator);
	g_engine.addWindow(newWindow);
}
