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
#include "../GUI_Elements/GUI_Label.h"
#include "../GUI_Elements/GUI_CheckBox.h"
#include "../GUI_Elements/GUI_Grouper.h"
#include "../GUI_Elements/GUI_Icon.h"
#include "../thingManager.h"
#include "../game.h"
#include "Outfits.h"

#define OUTFITS_TITLE "Select Outfit"
#define OUTFITS_WIDTH 450
#define OUTFITS_HEIGHT 391
#define OUTFITS_CANCEL_EVENTID 1000
#define OUTFITS_OK_EVENTID 1001
#define OUTFITS_LABEL_PREVIEW_TITLE "Preview:"
#define OUTFITS_LABEL_PREVIEW_X 63
#define OUTFITS_LABEL_PREVIEW_Y 32
#define OUTFITS_LABEL_FILTER_TITLE "Filter:"
#define OUTFITS_LABEL_FILTER_X 71
#define OUTFITS_LABEL_FILTER_Y 164
#define OUTFITS_LABEL_CONFIGURE_TITLE "Configure:"
#define OUTFITS_LABEL_CONFIGURE_X 57
#define OUTFITS_LABEL_CONFIGURE_Y 252
#define OUTFITS_LABEL_OUTFIT_X 304
#define OUTFITS_LABEL_OUTFIT_Y 185
#define OUTFITS_LABEL_OUTFIT_EVENTID 1002
#define OUTFITS_LABEL_MOUNT_X 304
#define OUTFITS_LABEL_MOUNT_Y 210
#define OUTFITS_LABEL_MOUNT_EVENTID 1003
#define OUTFITS_GROUPER_OUTFIT_NAME_X 199
#define OUTFITS_GROUPER_OUTFIT_NAME_Y 179
#define OUTFITS_GROUPER_OUTFIT_NAME_W 210
#define OUTFITS_GROUPER_OUTFIT_NAME_H 20
#define OUTFITS_GROUPER_MOUNT_NAME_X 199
#define OUTFITS_GROUPER_MOUNT_NAME_Y 204
#define OUTFITS_GROUPER_MOUNT_NAME_W 210
#define OUTFITS_GROUPER_MOUNT_NAME_H 20
#define OUTFITS_CHECKBOX_MOVEMENT_TEXT "Movement"
#define OUTFITS_CHECKBOX_MOVEMENT_X 16
#define OUTFITS_CHECKBOX_MOVEMENT_Y 47
#define OUTFITS_CHECKBOX_MOVEMENT_W 148
#define OUTFITS_CHECKBOX_MOVEMENT_H 22
#define OUTFITS_CHECKBOX_MOVEMENT_EVENTID 1004
#define OUTFITS_CHECKBOX_OUTFIT_TEXT "Show Outfit"
#define OUTFITS_CHECKBOX_OUTFIT_X 16
#define OUTFITS_CHECKBOX_OUTFIT_Y 74
#define OUTFITS_CHECKBOX_OUTFIT_W 148
#define OUTFITS_CHECKBOX_OUTFIT_H 22
#define OUTFITS_CHECKBOX_OUTFIT_EVENTID 1005
#define OUTFITS_CHECKBOX_FLOOR_TEXT "Show Floor"
#define OUTFITS_CHECKBOX_FLOOR_X 16
#define OUTFITS_CHECKBOX_FLOOR_Y 101
#define OUTFITS_CHECKBOX_FLOOR_W 148
#define OUTFITS_CHECKBOX_FLOOR_H 22
#define OUTFITS_CHECKBOX_FLOOR_EVENTID 1006
#define OUTFITS_CHECKBOX_OUTFITS_TEXT "Only my Outfits"
#define OUTFITS_CHECKBOX_OUTFITS_X 16
#define OUTFITS_CHECKBOX_OUTFITS_Y 179
#define OUTFITS_CHECKBOX_OUTFITS_W 148
#define OUTFITS_CHECKBOX_OUTFITS_H 22
#define OUTFITS_CHECKBOX_OUTFITS_EVENTID 1007
#define OUTFITS_CHECKBOX_MOUNTS_TEXT "Only my Mounts"
#define OUTFITS_CHECKBOX_MOUNTS_X 16
#define OUTFITS_CHECKBOX_MOUNTS_Y 206
#define OUTFITS_CHECKBOX_MOUNTS_W 148
#define OUTFITS_CHECKBOX_MOUNTS_H 22
#define OUTFITS_CHECKBOX_MOUNTS_EVENTID 1008
#define OUTFITS_CHECKBOX_ADDON1_TEXT "Addon 1"
#define OUTFITS_CHECKBOX_ADDON1_X 16
#define OUTFITS_CHECKBOX_ADDON1_Y 267
#define OUTFITS_CHECKBOX_ADDON1_W 148
#define OUTFITS_CHECKBOX_ADDON1_H 22
#define OUTFITS_CHECKBOX_ADDON1_EVENTID 1009
#define OUTFITS_CHECKBOX_ADDON2_TEXT "Addon 2"
#define OUTFITS_CHECKBOX_ADDON2_X 16
#define OUTFITS_CHECKBOX_ADDON2_Y 294
#define OUTFITS_CHECKBOX_ADDON2_W 148
#define OUTFITS_CHECKBOX_ADDON2_H 22
#define OUTFITS_CHECKBOX_ADDON2_EVENTID 1010
#define OUTFITS_CHECKBOX_MOUNT_TEXT "Mount"
#define OUTFITS_CHECKBOX_MOUNT_X 16
#define OUTFITS_CHECKBOX_MOUNT_Y 321
#define OUTFITS_CHECKBOX_MOUNT_W 148
#define OUTFITS_CHECKBOX_MOUNT_H 22
#define OUTFITS_CHECKBOX_MOUNT_EVENTID 1011
#define OUTFITS_GROUPER_OUTFIT_X 174
#define OUTFITS_GROUPER_OUTFIT_Y 29
#define OUTFITS_GROUPER_OUTFIT_W 260
#define OUTFITS_GROUPER_OUTFIT_H 145
#define OUTFITS_GROUPER_OUTFIT_NAME_X 199
#define OUTFITS_GROUPER_OUTFIT_NAME_Y 179
#define OUTFITS_GROUPER_OUTFIT_NAME_W 210
#define OUTFITS_GROUPER_OUTFIT_NAME_H 20
#define OUTFITS_GROUPER_MOUNT_NAME_X 199
#define OUTFITS_GROUPER_MOUNT_NAME_Y 204
#define OUTFITS_GROUPER_MOUNT_NAME_W 210
#define OUTFITS_GROUPER_MOUNT_NAME_H 20
#define OUTFITS_ICON_PREV_OUTFIT_X 174
#define OUTFITS_ICON_PREV_OUTFIT_Y 179
#define OUTFITS_ICON_PREV_OUTFIT_W 20
#define OUTFITS_ICON_PREV_OUTFIT_H 20
#define OUTFITS_ICON_PREV_OUTFIT_EVENTID 1012
#define OUTFITS_ICON_PREV_MOUNT_X 174
#define OUTFITS_ICON_PREV_MOUNT_Y 204
#define OUTFITS_ICON_PREV_MOUNT_W 20
#define OUTFITS_ICON_PREV_MOUNT_H 20
#define OUTFITS_ICON_PREV_MOUNT_EVENTID 1013
#define OUTFITS_ICON_PREV_DIRECTION_X 176
#define OUTFITS_ICON_PREV_DIRECTION_Y 152
#define OUTFITS_ICON_PREV_DIRECTION_W 20
#define OUTFITS_ICON_PREV_DIRECTION_H 20
#define OUTFITS_ICON_PREV_DIRECTION_EVENTID 1014
#define OUTFITS_ICON_NEXT_OUTFIT_X 414
#define OUTFITS_ICON_NEXT_OUTFIT_Y 179
#define OUTFITS_ICON_NEXT_OUTFIT_W 20
#define OUTFITS_ICON_NEXT_OUTFIT_H 20
#define OUTFITS_ICON_NEXT_OUTFIT_EVENTID 1015
#define OUTFITS_ICON_NEXT_MOUNT_X 414
#define OUTFITS_ICON_NEXT_MOUNT_Y 204
#define OUTFITS_ICON_NEXT_MOUNT_W 20
#define OUTFITS_ICON_NEXT_MOUNT_H 20
#define OUTFITS_ICON_NEXT_MOUNT_EVENTID 1016
#define OUTFITS_ICON_NEXT_DIRECTION_X 412
#define OUTFITS_ICON_NEXT_DIRECTION_Y 152
#define OUTFITS_ICON_NEXT_DIRECTION_W 20
#define OUTFITS_ICON_NEXT_DIRECTION_H 20
#define OUTFITS_ICON_NEXT_DIRECTION_EVENTID 1017
#define OUTFITS_OUTFIT_VIEW_X 175
#define OUTFITS_OUTFIT_VIEW_Y 30
#define OUTFITS_OUTFIT_VIEW_W 258
#define OUTFITS_OUTFIT_VIEW_H 143
#define OUTFITS_OUTFIT_VIEW_EVENTID 1018
#define OUTFITS_OUTFIT_COLORS_X 177
#define OUTFITS_OUTFIT_COLORS_Y 229
#define OUTFITS_OUTFIT_COLORS_W 254
#define OUTFITS_OUTFIT_COLORS_H 114

extern Engine g_engine;
extern Game g_game;
extern ThingManager g_thingManager;
extern Uint32 g_frameTime;

std::vector<OutfitDetail> g_outfits;
std::vector<MountDetail> g_mounts;
Uint16 g_outfitLookType;
Uint16 g_outfitMount;
Uint8 g_outfitColors[4];
Uint8 g_outfitAddons;

void outfits_Events(Uint32 event, Sint32 status)
{
	switch(event)
	{
		case OUTFITS_CANCEL_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_OUTFITS)
			{
				g_engine.removeWindow(pWindow);
				g_outfits.clear();
				g_mounts.clear();
				#ifdef HAVE_CXX11_SUPPORT
				g_outfits.shrink_to_fit();
				g_mounts.shrink_to_fit();
				#else
				std::vector<OutfitDetail>(g_outfits).swap(g_outfits);
				std::vector<MountDetail>(g_mounts).swap(g_mounts);
				#endif
			}
		}
		break;
		case OUTFITS_OK_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_OUTFITS)
			{
				g_game.sendSetOutfit(g_outfitLookType, g_outfitColors[0], g_outfitColors[1], g_outfitColors[2], g_outfitColors[3], g_outfitAddons, g_outfitMount);
				g_engine.removeWindow(pWindow);
				g_outfits.clear();
				g_mounts.clear();
				#ifdef HAVE_CXX11_SUPPORT
				g_outfits.shrink_to_fit();
				g_mounts.shrink_to_fit();
				#else
				std::vector<OutfitDetail>(g_outfits).swap(g_outfits);
				std::vector<MountDetail>(g_mounts).swap(g_mounts);
				#endif
			}
		}
		break;
		case OUTFITS_CHECKBOX_MOVEMENT_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_OUTFITS)
			{
				GUI_Outfit_View* pOutfitView = SDL_static_cast(GUI_Outfit_View*, pWindow->getChild(OUTFITS_OUTFIT_VIEW_EVENTID));
				if(pOutfitView)
				{
					if(status == 1)
						pOutfitView->startMovement();
					else
						pOutfitView->stopMovement();
				}
			}
		}
		break;
		case OUTFITS_CHECKBOX_OUTFIT_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_OUTFITS)
			{
				GUI_Outfit_View* pOutfitView = SDL_static_cast(GUI_Outfit_View*, pWindow->getChild(OUTFITS_OUTFIT_VIEW_EVENTID));
				if(pOutfitView)
					pOutfitView->refresh();
			}
		}
		break;
		case OUTFITS_CHECKBOX_ADDON1_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_OUTFITS)
			{
				if(status == 1)
					g_outfitAddons |= 1;
				else
					g_outfitAddons &= ~1;
			}
		}
		break;
		case OUTFITS_CHECKBOX_ADDON2_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_OUTFITS)
			{
				if(status == 1)
					g_outfitAddons |= 2;
				else
					g_outfitAddons &= ~2;
			}
		}
		break;
		case OUTFITS_CHECKBOX_MOUNT_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_OUTFITS)
			{
				GUI_Outfit_View* pOutfitView = SDL_static_cast(GUI_Outfit_View*, pWindow->getChild(OUTFITS_OUTFIT_VIEW_EVENTID));
				if(pOutfitView)
					pOutfitView->refresh();
			}
		}
		break;
		case OUTFITS_ICON_PREV_OUTFIT_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_OUTFITS && g_outfits.size() > 1)
			{
				std::vector<OutfitDetail>::iterator it = g_outfits.begin();
				for(std::vector<OutfitDetail>::iterator end = g_outfits.end(); it != end; ++it)
				{
					if((*it).outfitID == g_outfitLookType)
						break;
				}
				if(it == g_outfits.begin())
					it = g_outfits.end();
				--it;
				g_outfitLookType = (*it).outfitID;

				GUI_Outfit_View* pOutfitView = SDL_static_cast(GUI_Outfit_View*, pWindow->getChild(OUTFITS_OUTFIT_VIEW_EVENTID));
				if(pOutfitView)
					pOutfitView->refresh();

				GUI_Label* pLabel = SDL_static_cast(GUI_Label*, pWindow->getChild(OUTFITS_LABEL_OUTFIT_EVENTID));
				if(pLabel)
					pLabel->setName((*it).outfitName);

				GUI_CheckBox* pCheckBox1 = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(OUTFITS_CHECKBOX_ADDON1_EVENTID));
				GUI_CheckBox* pCheckBox2 = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(OUTFITS_CHECKBOX_ADDON2_EVENTID));
				if(pCheckBox1 && pCheckBox2)
				{
					Uint8 outfitAddons = (*it).outfitAddons;
					if(outfitAddons & 1)//First addon
					{
						if(pCheckBox1->isChecked())
							g_outfitAddons |= 1;
						else
							g_outfitAddons &= ~1;
						pCheckBox1->setColor(180, 180, 180);
						pCheckBox1->startEvents();
					}
					else
					{
						g_outfitAddons &= ~1;
						pCheckBox1->setColor(112, 112, 112);
						pCheckBox1->stopEvents();
					}
					if(outfitAddons & 2)//Second addon
					{
						if(pCheckBox2->isChecked())
							g_outfitAddons |= 2;
						else
							g_outfitAddons &= ~2;
						pCheckBox2->setColor(180, 180, 180);
						pCheckBox2->startEvents();
					}
					else
					{
						g_outfitAddons &= ~2;
						pCheckBox2->setColor(112, 112, 112);
						pCheckBox2->stopEvents();
					}
				}
			}
		}
		break;
		case OUTFITS_ICON_NEXT_OUTFIT_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_OUTFITS && g_outfits.size() > 1)
			{
				std::vector<OutfitDetail>::iterator it = g_outfits.begin();
				for(std::vector<OutfitDetail>::iterator end = g_outfits.end(); it != end; ++it)
				{
					if((*it).outfitID == g_outfitLookType)
						break;
				}
				++it;
				if(it == g_outfits.end())
					it = g_outfits.begin();
				g_outfitLookType = (*it).outfitID;

				GUI_Outfit_View* pOutfitView = SDL_static_cast(GUI_Outfit_View*, pWindow->getChild(OUTFITS_OUTFIT_VIEW_EVENTID));
				if(pOutfitView)
					pOutfitView->refresh();

				GUI_Label* pLabel = SDL_static_cast(GUI_Label*, pWindow->getChild(OUTFITS_LABEL_OUTFIT_EVENTID));
				if(pLabel)
					pLabel->setName((*it).outfitName);

				GUI_CheckBox* pCheckBox1 = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(OUTFITS_CHECKBOX_ADDON1_EVENTID));
				GUI_CheckBox* pCheckBox2 = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(OUTFITS_CHECKBOX_ADDON2_EVENTID));
				if(pCheckBox1 && pCheckBox2)
				{
					Uint8 outfitAddons = (*it).outfitAddons;
					if(outfitAddons & 1)//First addon
					{
						if(pCheckBox1->isChecked())
							g_outfitAddons |= 2;
						else
							g_outfitAddons &= ~2;
						pCheckBox1->setColor(180, 180, 180);
						pCheckBox1->startEvents();
					}
					else
					{
						g_outfitAddons &= ~1;
						pCheckBox1->setColor(112, 112, 112);
						pCheckBox1->stopEvents();
					}
					if(outfitAddons & 2)//Second addon
					{
						if(pCheckBox2->isChecked())
							g_outfitAddons |= 2;
						else
							g_outfitAddons &= ~2;
						pCheckBox2->setColor(180, 180, 180);
						pCheckBox2->startEvents();
					}
					else
					{
						g_outfitAddons &= ~2;
						pCheckBox2->setColor(112, 112, 112);
						pCheckBox2->stopEvents();
					}
				}
			}
		}
		break;
		case OUTFITS_ICON_PREV_MOUNT_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_OUTFITS && g_mounts.size() > 1)
			{
				std::vector<MountDetail>::iterator it = g_mounts.begin();
				for(std::vector<MountDetail>::iterator end = g_mounts.end(); it != end; ++it)
				{
					if((*it).mountID == g_outfitMount)
						break;
				}
				if(it == g_mounts.begin())
					it = g_mounts.end();
				--it;
				g_outfitMount = (*it).mountID;

				GUI_Outfit_View* pOutfitView = SDL_static_cast(GUI_Outfit_View*, pWindow->getChild(OUTFITS_OUTFIT_VIEW_EVENTID));
				if(pOutfitView)
					pOutfitView->refresh();

				GUI_Label* pLabel = SDL_static_cast(GUI_Label*, pWindow->getChild(OUTFITS_LABEL_MOUNT_EVENTID));
				if(pLabel)
					pLabel->setName((*it).mountName);
			}
		}
		break;
		case OUTFITS_ICON_NEXT_MOUNT_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_OUTFITS && g_mounts.size() > 1)
			{
				std::vector<MountDetail>::iterator it = g_mounts.begin();
				for(std::vector<MountDetail>::iterator end = g_mounts.end(); it != end; ++it)
				{
					if((*it).mountID == g_outfitMount)
						break;
				}
				++it;
				if(it == g_mounts.end())
					it = g_mounts.begin();
				g_outfitMount = (*it).mountID;

				GUI_Outfit_View* pOutfitView = SDL_static_cast(GUI_Outfit_View*, pWindow->getChild(OUTFITS_OUTFIT_VIEW_EVENTID));
				if(pOutfitView)
					pOutfitView->refresh();

				GUI_Label* pLabel = SDL_static_cast(GUI_Label*, pWindow->getChild(OUTFITS_LABEL_MOUNT_EVENTID));
				if(pLabel)
					pLabel->setName((*it).mountName);
			}
		}
		break;
		case OUTFITS_ICON_PREV_DIRECTION_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_OUTFITS)
			{
				GUI_Outfit_View* pOutfitView = SDL_static_cast(GUI_Outfit_View*, pWindow->getChild(OUTFITS_OUTFIT_VIEW_EVENTID));
				if(pOutfitView)
					pOutfitView->previousDirection();
			}
		}
		break;
		case OUTFITS_ICON_NEXT_DIRECTION_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_OUTFITS)
			{
				GUI_Outfit_View* pOutfitView = SDL_static_cast(GUI_Outfit_View*, pWindow->getChild(OUTFITS_OUTFIT_VIEW_EVENTID));
				if(pOutfitView)
					pOutfitView->nextDirection();
			}
		}
		break;
	}
}

void UTIL_createOutfitWindow(Uint16 lookType, Uint8 lookHead, Uint8 lookBody, Uint8 lookLegs, Uint8 lookFeet, Uint8 lookAddons, Uint16 lookMount, std::vector<OutfitDetail>& outfits, std::vector<MountDetail>& mounts)
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_OUTFITS);
	if(pWindow)
		g_engine.removeWindow(pWindow);

	if(lookType == 0)
	{
		if(outfits.empty())
			return;
		lookType = (*outfits.begin()).outfitID;
	}

	std::string outfitName = "Your Character";
	std::string mountName = "No Mount";
	Uint8 outfitAddons = 0;
	bool foundOutfit = false;
	bool foundMount = false;
	for(std::vector<OutfitDetail>::iterator it = outfits.begin(), end = outfits.end(); it != end; ++it)
	{
		OutfitDetail& outfit = (*it);
		if(outfit.outfitID == lookType)
		{
			outfitName = outfit.outfitName;
			outfitAddons = outfit.outfitAddons;
			foundOutfit = true;
			break;
		}
	}

	if(lookMount != 0)
	{
		for(std::vector<MountDetail>::iterator it = mounts.begin(), end = mounts.end(); it != end; ++it)
		{
			MountDetail& mount = (*it);
			if(mount.mountID == lookMount)
			{
				mountName = mount.mountName;
				foundMount = true;
				break;
			}
		}

		if(!foundMount)
		{
			MountDetail newMount;
			newMount.mountID = lookMount;
			newMount.mountName = "Your Mount";
			mounts.push_back(newMount);
			mountName = "Tempest";
			foundMount = true;
		}
	}

	if(!foundOutfit)
	{
		OutfitDetail newOutfit;
		newOutfit.outfitID = lookType;
		newOutfit.outfitName = "Your Character";
		newOutfit.outfitAddons = 0;
		outfits.push_back(newOutfit);
	}

	g_outfits = outfits;
	g_mounts = mounts;
	g_outfitLookType = lookType;
	g_outfitMount = lookMount;
	g_outfitColors[0] = lookHead;
	g_outfitColors[1] = lookBody;
	g_outfitColors[2] = lookLegs;
	g_outfitColors[3] = lookFeet;
	g_outfitAddons = lookAddons;

	GUI_Window* newWindow = new GUI_Window(iRect(0, 0, OUTFITS_WIDTH, OUTFITS_HEIGHT), OUTFITS_TITLE, GUI_WINDOW_OUTFITS);
	GUI_Outfit_View* newView = new GUI_Outfit_View(iRect(OUTFITS_OUTFIT_VIEW_X, OUTFITS_OUTFIT_VIEW_Y, OUTFITS_OUTFIT_VIEW_W, OUTFITS_OUTFIT_VIEW_H), OUTFITS_OUTFIT_VIEW_EVENTID);
	newWindow->addChild(newView);
	GUI_Outfit_Colors* newColors = new GUI_Outfit_Colors(iRect(OUTFITS_OUTFIT_COLORS_X, OUTFITS_OUTFIT_COLORS_Y, OUTFITS_OUTFIT_COLORS_W, OUTFITS_OUTFIT_COLORS_H));
	newColors->startEvents();
	newWindow->addChild(newColors);
	GUI_Icon* newIcon = new GUI_Icon(iRect(OUTFITS_ICON_PREV_OUTFIT_X, OUTFITS_ICON_PREV_OUTFIT_Y, OUTFITS_ICON_PREV_OUTFIT_W, OUTFITS_ICON_PREV_OUTFIT_H), 3, 240, 232, 280, 232);
	newIcon->setButtonEventCallback(&outfits_Events, OUTFITS_ICON_PREV_OUTFIT_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(OUTFITS_ICON_PREV_MOUNT_X, OUTFITS_ICON_PREV_MOUNT_Y, OUTFITS_ICON_PREV_MOUNT_W, OUTFITS_ICON_PREV_MOUNT_H), 3, 240, 232, 280, 232);
	newIcon->setButtonEventCallback(&outfits_Events, OUTFITS_ICON_PREV_MOUNT_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(OUTFITS_ICON_PREV_DIRECTION_X, OUTFITS_ICON_PREV_DIRECTION_Y, OUTFITS_ICON_PREV_DIRECTION_W, OUTFITS_ICON_PREV_DIRECTION_H), 3, 352, 438, 392, 438);
	newIcon->setButtonEventCallback(&outfits_Events, OUTFITS_ICON_PREV_DIRECTION_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(OUTFITS_ICON_NEXT_OUTFIT_X, OUTFITS_ICON_NEXT_OUTFIT_Y, OUTFITS_ICON_NEXT_OUTFIT_W, OUTFITS_ICON_NEXT_OUTFIT_H), 3, 260, 232, 300, 232);
	newIcon->setButtonEventCallback(&outfits_Events, OUTFITS_ICON_NEXT_OUTFIT_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(OUTFITS_ICON_NEXT_MOUNT_X, OUTFITS_ICON_NEXT_MOUNT_Y, OUTFITS_ICON_NEXT_MOUNT_W, OUTFITS_ICON_NEXT_MOUNT_H), 3, 260, 232, 300, 232);
	newIcon->setButtonEventCallback(&outfits_Events, OUTFITS_ICON_NEXT_MOUNT_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(OUTFITS_ICON_NEXT_DIRECTION_X, OUTFITS_ICON_NEXT_DIRECTION_Y, OUTFITS_ICON_NEXT_DIRECTION_W, OUTFITS_ICON_NEXT_DIRECTION_H), 3, 372, 438, 329, 12);
	newIcon->setButtonEventCallback(&outfits_Events, OUTFITS_ICON_NEXT_DIRECTION_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	GUI_Grouper* newGrouper = new GUI_Grouper(iRect(OUTFITS_GROUPER_OUTFIT_X, OUTFITS_GROUPER_OUTFIT_Y, OUTFITS_GROUPER_OUTFIT_W, OUTFITS_GROUPER_OUTFIT_H));
	newWindow->addChild(newGrouper);
	newGrouper = new GUI_Grouper(iRect(OUTFITS_GROUPER_OUTFIT_NAME_X, OUTFITS_GROUPER_OUTFIT_NAME_Y, OUTFITS_GROUPER_OUTFIT_NAME_W, OUTFITS_GROUPER_OUTFIT_NAME_H));
	newWindow->addChild(newGrouper);
	newGrouper = new GUI_Grouper(iRect(OUTFITS_GROUPER_MOUNT_NAME_X, OUTFITS_GROUPER_MOUNT_NAME_Y, OUTFITS_GROUPER_MOUNT_NAME_W, OUTFITS_GROUPER_MOUNT_NAME_H));
	newWindow->addChild(newGrouper);
	GUI_CheckBox* newCheckBox = new GUI_CheckBox(iRect(OUTFITS_CHECKBOX_MOVEMENT_X, OUTFITS_CHECKBOX_MOVEMENT_Y, OUTFITS_CHECKBOX_MOVEMENT_W, OUTFITS_CHECKBOX_MOVEMENT_H), OUTFITS_CHECKBOX_MOVEMENT_TEXT, false, OUTFITS_CHECKBOX_MOVEMENT_EVENTID);
	newCheckBox->setBoxEventCallback(&outfits_Events, OUTFITS_CHECKBOX_MOVEMENT_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);
	newCheckBox = new GUI_CheckBox(iRect(OUTFITS_CHECKBOX_OUTFIT_X, OUTFITS_CHECKBOX_OUTFIT_Y, OUTFITS_CHECKBOX_OUTFIT_W, OUTFITS_CHECKBOX_OUTFIT_H), OUTFITS_CHECKBOX_OUTFIT_TEXT, true, OUTFITS_CHECKBOX_OUTFIT_EVENTID, 112, 112, 112);
	newCheckBox->setBoxEventCallback(&outfits_Events, OUTFITS_CHECKBOX_OUTFIT_EVENTID);
	newCheckBox->stopEvents();
	newWindow->addChild(newCheckBox);
	newCheckBox = new GUI_CheckBox(iRect(OUTFITS_CHECKBOX_FLOOR_X, OUTFITS_CHECKBOX_FLOOR_Y, OUTFITS_CHECKBOX_FLOOR_W, OUTFITS_CHECKBOX_FLOOR_H), OUTFITS_CHECKBOX_FLOOR_TEXT, true, OUTFITS_CHECKBOX_FLOOR_EVENTID, 112, 112, 112);
	newCheckBox->setBoxEventCallback(&outfits_Events, OUTFITS_CHECKBOX_FLOOR_EVENTID);
	newCheckBox->stopEvents();
	newWindow->addChild(newCheckBox);
	newCheckBox = new GUI_CheckBox(iRect(OUTFITS_CHECKBOX_OUTFITS_X, OUTFITS_CHECKBOX_OUTFITS_Y, OUTFITS_CHECKBOX_OUTFITS_W, OUTFITS_CHECKBOX_OUTFITS_H), OUTFITS_CHECKBOX_OUTFITS_TEXT, true, OUTFITS_CHECKBOX_OUTFITS_EVENTID, 112, 112, 112);
	newCheckBox->setBoxEventCallback(&outfits_Events, OUTFITS_CHECKBOX_OUTFITS_EVENTID);
	newCheckBox->stopEvents();
	newWindow->addChild(newCheckBox);
	newCheckBox = new GUI_CheckBox(iRect(OUTFITS_CHECKBOX_MOUNTS_X, OUTFITS_CHECKBOX_MOUNTS_Y, OUTFITS_CHECKBOX_MOUNTS_W, OUTFITS_CHECKBOX_MOUNTS_H), OUTFITS_CHECKBOX_MOUNTS_TEXT, true, OUTFITS_CHECKBOX_MOUNTS_EVENTID, 112, 112, 112);
	newCheckBox->setBoxEventCallback(&outfits_Events, OUTFITS_CHECKBOX_MOUNTS_EVENTID);
	newCheckBox->stopEvents();
	newWindow->addChild(newCheckBox);
	newCheckBox = new GUI_CheckBox(iRect(OUTFITS_CHECKBOX_ADDON1_X, OUTFITS_CHECKBOX_ADDON1_Y, OUTFITS_CHECKBOX_ADDON1_W, OUTFITS_CHECKBOX_ADDON1_H), OUTFITS_CHECKBOX_ADDON1_TEXT, (g_outfitAddons & 1), OUTFITS_CHECKBOX_ADDON1_EVENTID);
	newCheckBox->setBoxEventCallback(&outfits_Events, OUTFITS_CHECKBOX_ADDON1_EVENTID);
	if(outfitAddons & 1)//First addon
		newCheckBox->startEvents();
	else
	{
		g_outfitAddons &= ~1;
		newCheckBox->setColor(112, 112, 112);
		newCheckBox->stopEvents();
	}
	newWindow->addChild(newCheckBox);
	newCheckBox = new GUI_CheckBox(iRect(OUTFITS_CHECKBOX_ADDON2_X, OUTFITS_CHECKBOX_ADDON2_Y, OUTFITS_CHECKBOX_ADDON2_W, OUTFITS_CHECKBOX_ADDON2_H), OUTFITS_CHECKBOX_ADDON2_TEXT, (g_outfitAddons & 2), OUTFITS_CHECKBOX_ADDON2_EVENTID);
	newCheckBox->setBoxEventCallback(&outfits_Events, OUTFITS_CHECKBOX_ADDON2_EVENTID);
	if(outfitAddons & 2)//Second addon
		newCheckBox->startEvents();
	else
	{
		g_outfitAddons &= ~2;
		newCheckBox->setColor(112, 112, 112);
		newCheckBox->stopEvents();
	}
	newWindow->addChild(newCheckBox);
	newCheckBox = new GUI_CheckBox(iRect(OUTFITS_CHECKBOX_MOUNT_X, OUTFITS_CHECKBOX_MOUNT_Y, OUTFITS_CHECKBOX_MOUNT_W, OUTFITS_CHECKBOX_MOUNT_H), OUTFITS_CHECKBOX_MOUNT_TEXT, true, OUTFITS_CHECKBOX_MOUNT_EVENTID);
	newCheckBox->setBoxEventCallback(&outfits_Events, OUTFITS_CHECKBOX_MOUNT_EVENTID);
	if(!mounts.empty())
		newCheckBox->startEvents();
	else
	{
		newCheckBox->setColor(112, 112, 112);
		newCheckBox->stopEvents();
	}
	newWindow->addChild(newCheckBox);
	GUI_Label* newLabel = new GUI_Label(iRect(OUTFITS_LABEL_PREVIEW_X, OUTFITS_LABEL_PREVIEW_Y, 0, 0), OUTFITS_LABEL_PREVIEW_TITLE);
	newWindow->addChild(newLabel);
	newLabel = new GUI_Label(iRect(OUTFITS_LABEL_FILTER_X, OUTFITS_LABEL_FILTER_Y, 0, 0), OUTFITS_LABEL_FILTER_TITLE);
	newWindow->addChild(newLabel);
	newLabel = new GUI_Label(iRect(OUTFITS_LABEL_CONFIGURE_X, OUTFITS_LABEL_CONFIGURE_Y, 0, 0), OUTFITS_LABEL_CONFIGURE_TITLE);
	newWindow->addChild(newLabel);
	newLabel = new GUI_Label(iRect(OUTFITS_LABEL_OUTFIT_X, OUTFITS_LABEL_OUTFIT_Y, 0, 0), outfitName, OUTFITS_LABEL_OUTFIT_EVENTID);
	newLabel->setAlign(CLIENT_FONT_ALIGN_CENTER);
	newWindow->addChild(newLabel);
	newLabel = new GUI_Label(iRect(OUTFITS_LABEL_MOUNT_X, OUTFITS_LABEL_MOUNT_Y, 0, 0), mountName, OUTFITS_LABEL_MOUNT_EVENTID);
	newLabel->setAlign(CLIENT_FONT_ALIGN_CENTER);
	newWindow->addChild(newLabel);
	GUI_Button* newButton = new GUI_Button(iRect(OUTFITS_WIDTH-56, OUTFITS_HEIGHT-30, 43, 20), "Cancel", CLIENT_GUI_ESCAPE_TRIGGER);
	newButton->setButtonEventCallback(&outfits_Events, OUTFITS_CANCEL_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	newButton = new GUI_Button(iRect(OUTFITS_WIDTH-109, OUTFITS_HEIGHT-30, 43, 20), "Ok", CLIENT_GUI_ENTER_TRIGGER);
	newButton->setButtonEventCallback(&outfits_Events, OUTFITS_OK_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	GUI_Separator* newSeparator = new GUI_Separator(iRect(13, OUTFITS_HEIGHT-40, OUTFITS_WIDTH-26, 2));
	newWindow->addChild(newSeparator);
	g_engine.addWindow(newWindow);
}

GUI_Outfit_View::GUI_Outfit_View(iRect boxRect, Uint32 internalID)
{
	setRect(boxRect);
	m_internalID = internalID;
	m_ground = g_thingManager.getThingType(ThingCategory_Item, 429);
	m_outfit = g_thingManager.getThingType(ThingCategory_Creature, g_outfitLookType);
	m_mount = (g_outfitMount == 0 ? NULL : g_thingManager.getThingType(ThingCategory_Creature, g_outfitMount));
	m_walkStartTime = 0;
	m_walkedPixels = 0;
	m_direction = DIRECTION_SOUTH;
	m_currentAnim = 0;
	m_currentFrame = ThingFrameGroup_Idle;
	m_showOutfit = true;
	m_showMount = true;
	m_walking = false;
}

void GUI_Outfit_View::previousDirection()
{
	if(m_direction == DIRECTION_WEST)
		m_direction = DIRECTION_NORTH;
	else
		++m_direction;
}

void GUI_Outfit_View::nextDirection()
{
	if(m_direction == DIRECTION_NORTH)
		m_direction = DIRECTION_WEST;
	else
		--m_direction;
}

void GUI_Outfit_View::refresh()
{
	m_outfit = g_thingManager.getThingType(ThingCategory_Creature, g_outfitLookType);
	m_mount = (g_outfitMount == 0 ? NULL : g_thingManager.getThingType(ThingCategory_Creature, g_outfitMount));

	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_OUTFITS);
	if(pWindow)
	{
		GUI_CheckBox* pCheckBox = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(OUTFITS_CHECKBOX_OUTFIT_EVENTID));
		if(pCheckBox)
			m_showOutfit = pCheckBox->isChecked();

		pCheckBox = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(OUTFITS_CHECKBOX_MOUNT_EVENTID));
		if(pCheckBox)
			m_showMount = pCheckBox->isChecked();
	}
}

void GUI_Outfit_View::startMovement()
{
	m_walking = true;
	m_currentAnim = 0;
	if(m_outfit->hasFlag(ThingAttribute_NoMoveAnimation))
		m_currentFrame = ThingFrameGroup_Idle;
	else
		m_currentFrame = ThingFrameGroup_Moving;
	m_walkStartTime = g_frameTime;
}

void GUI_Outfit_View::stopMovement()
{
	m_walking = false;
	m_currentAnim = 0;
	m_currentFrame = ThingFrameGroup_Idle;
}

void GUI_Outfit_View::updateMovement()
{
	if(m_walking)
	{
		float ticks = 500.f;
		float walkTicks = (ticks*0.03125f);
		m_walkedPixels = SDL_static_cast(Sint32, (g_frameTime - m_walkStartTime) / walkTicks);
		if(m_currentFrame == ThingFrameGroup_Idle)//We probably don't have framegroups
		{
			Uint8 animCount;
			if(m_mount)
				animCount = UTIL_max<Uint8>(0, (m_mount->m_frameGroup[m_currentFrame].m_animCount-1));
			else
				animCount = UTIL_max<Uint8>(0, (m_outfit->m_frameGroup[m_currentFrame].m_animCount-1));
			float footTicks = ticks/SDL_static_cast(float, animCount);
			m_currentAnim = UTIL_safeMod<Uint8>(SDL_static_cast(Uint8, (g_frameTime - m_walkStartTime) / footTicks), animCount)+1;
		}
		else
		{
			Uint8 animCount;
			if(m_mount)
				animCount = UTIL_max<Uint8>(0, m_mount->m_frameGroup[m_currentFrame].m_animCount);
			else
				animCount = UTIL_max<Uint8>(0, m_outfit->m_frameGroup[m_currentFrame].m_animCount);
			float footTicks = ticks/SDL_static_cast(float, animCount);
			m_currentAnim = UTIL_safeMod<Uint8>(SDL_static_cast(Uint8, (g_frameTime - m_walkStartTime) / footTicks), animCount);
		}
	}
}

Sint32 GUI_Outfit_View::getOffsetX()
{
	if(!m_walking) return 0;
	switch(m_direction)
	{
		case DIRECTION_NORTH:
			return 0;
		case DIRECTION_EAST:
			return (m_walkedPixels%32);
		case DIRECTION_SOUTH:
			return 0;
		case DIRECTION_WEST:
			return -(m_walkedPixels%32);
	}
	return 0;
}

Sint32 GUI_Outfit_View::getOffsetY()
{
	if(!m_walking) return 0;
	switch(m_direction)
	{
		case DIRECTION_NORTH:
			return -(m_walkedPixels%32);
		case DIRECTION_EAST:
			return 0;
		case DIRECTION_SOUTH:
			return (m_walkedPixels%32);
		case DIRECTION_WEST:
			return 0;
	}
	return 0;
}

void GUI_Outfit_View::render()
{
	Uint32 outfitColor = (g_outfitColors[3] << 24) | (g_outfitColors[2] << 16) | (g_outfitColors[1] << 8) | (g_outfitColors[0]);
	updateMovement();

	Surface* renderer = g_engine.getRender();
	renderer->beginGameScene();
	Sint32 startX = -getOffsetX();
	Sint32 startY = -getOffsetY();
	Sint32 posXc;
	Sint32 posYc = -32+startY;
	for(Uint8 y = 0; y < 5; ++y)
	{
		posXc = -32+startX;
		for(Uint8 x = 0; x < 7; ++x)
		{
			Uint8 pDiff = SDL_static_cast(Uint8, (m_walkedPixels/32));
			Uint8 xPattern = UTIL_safeMod<Uint8>((m_direction == DIRECTION_EAST ? (x+pDiff) : m_direction == DIRECTION_WEST ? (x-pDiff) : x), m_ground->m_frameGroup[ThingFrameGroup_Default].m_patternX);
			Uint8 yPattern = UTIL_safeMod<Uint8>((m_direction == DIRECTION_SOUTH ? (y+pDiff) : m_direction == DIRECTION_NORTH ? (y-pDiff) : y), m_ground->m_frameGroup[ThingFrameGroup_Default].m_patternY);
			Uint32 sprite = m_ground->getSprite(ThingFrameGroup_Default, 0, 0, 0, xPattern, yPattern, 0, 0);
			if(sprite != 0)
				renderer->drawSprite(sprite, posXc, posYc);
			posXc += 32;
		}
		posYc += 32;
	}
	Uint8 zPattern = 0;
	Sint32 xDiff = (!m_showOutfit ? 0 : (m_outfit->m_frameGroup[m_currentFrame].m_realSize-32)/2);
	startX = 64+xDiff;
	startY = 32;
	if(m_showOutfit)
	{
		startX -= m_outfit->m_displacement[0];
		startY -= m_outfit->m_displacement[1];
	}
	if(m_mount && m_showMount)
	{
		Uint8 animationFrame;
		if((false && m_currentFrame == ThingFrameGroup_Idle) || m_mount->hasFlag(ThingAttribute_AnimateAlways))
		{
			Sint32 ticks = (1000 / m_mount->m_frameGroup[m_currentFrame].m_animCount);
			animationFrame = UTIL_safeMod<Uint8>(SDL_static_cast(Uint8, (g_frameTime / ticks)), m_mount->m_frameGroup[m_currentFrame].m_animCount);
		}
		else
			animationFrame = m_currentAnim;

		startX -= m_mount->m_displacement[0];
		startY -= m_mount->m_displacement[1];
		posYc = startY;
		for(Uint8 cy = 0; cy < m_mount->m_frameGroup[m_currentFrame].m_height; ++cy)
		{
			posXc = startX;
			for(Uint8 cx = 0; cx < m_mount->m_frameGroup[m_currentFrame].m_width; ++cx)
			{
				Uint32 sprite = m_mount->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), cx, cy, 0, m_direction, 0, 0, animationFrame);
				if(sprite != 0)
					renderer->drawSprite(sprite, posXc, posYc);
				posXc -= 32;
			}
			posYc -= 32;
		}
		zPattern = UTIL_min<Uint8>(1, m_outfit->m_frameGroup[m_currentFrame].m_patternZ-1);
	}
	if(m_showOutfit)
	{
		Uint8 animationFrame;
		if((false && m_currentFrame == ThingFrameGroup_Idle) || m_outfit->hasFlag(ThingAttribute_AnimateAlways))
		{
			Sint32 ticks = (1000 / m_outfit->m_frameGroup[m_currentFrame].m_animCount);
			animationFrame = UTIL_safeMod<Uint8>(SDL_static_cast(Uint8, (g_frameTime / ticks)), m_outfit->m_frameGroup[m_currentFrame].m_animCount);
		}
		else
			animationFrame = m_currentAnim;
		if(m_outfit->m_frameGroup[m_currentFrame].m_layers > 1)
		{
			posYc = startY;
			for(Uint8 cy = 0; cy < m_outfit->m_frameGroup[m_currentFrame].m_height; ++cy)
			{
				posXc = startX;
				for(Uint8 cx = 0; cx < m_outfit->m_frameGroup[m_currentFrame].m_width; ++cx)
				{
					Uint32 sprite = m_outfit->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), cx, cy, 0, m_direction, 0, zPattern, animationFrame);
					Uint32 spriteMask = m_outfit->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), cx, cy, 1, m_direction, 0, zPattern, animationFrame);
					if(sprite != 0)
					{
						if(spriteMask != 0)
							renderer->drawSpriteMask(sprite, spriteMask, posXc, posYc, outfitColor);
						else
							renderer->drawSprite(sprite, posXc, posYc);
					}
					if(m_outfit->m_frameGroup[m_currentFrame].m_patternY > 1)
					{
						if(g_outfitAddons & 1)//First addon
						{
							sprite = m_outfit->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), cx, cy, 0, m_direction, 1, zPattern, animationFrame);
							spriteMask = m_outfit->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), cx, cy, 1, m_direction, 1, zPattern, animationFrame);
							if(sprite != 0)
							{
								if(spriteMask != 0)
									renderer->drawSpriteMask(sprite, spriteMask, posXc, posYc, outfitColor);
								else
									renderer->drawSprite(sprite, posXc, posYc);
							}
						}
						if(g_outfitAddons & 2)//Second addon
						{
							sprite = m_outfit->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), cx, cy, 0, m_direction, 2, zPattern, animationFrame);
							spriteMask = m_outfit->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), cx, cy, 1, m_direction, 2, zPattern, animationFrame);
							if(sprite != 0)
							{
								if(spriteMask != 0)
									renderer->drawSpriteMask(sprite, spriteMask, posXc, posYc, outfitColor);
								else
									renderer->drawSprite(sprite, posXc, posYc);
							}
						}
					}
					posXc -= 32;
				}
				posYc -= 32;
			}
		}
		else
		{
			posYc = startY;
			for(Uint8 cy = 0; cy < m_outfit->m_frameGroup[m_currentFrame].m_height; ++cy)
			{
				posXc = startX;
				for(Uint8 cx = 0; cx < m_outfit->m_frameGroup[m_currentFrame].m_width; ++cx)
				{
					Uint32 sprite = m_outfit->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), cx, cy, 0, m_direction, 0, zPattern, animationFrame);
					if(sprite != 0)
						renderer->drawSprite(sprite, posXc, posYc);
					if(m_outfit->m_frameGroup[m_currentFrame].m_patternY > 1)
					{
						if(g_outfitAddons & 1)//First addon
						{
							sprite = m_outfit->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), cx, cy, 0, m_direction, 1, zPattern, animationFrame);
							if(sprite != 0)
								renderer->drawSprite(sprite, posXc, posYc);
						}
						if(g_outfitAddons & 2)//Second addon
						{
							sprite = m_outfit->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), cx, cy, 0, m_direction, 2, zPattern, animationFrame);
							if(sprite != 0)
								renderer->drawSprite(sprite, posXc, posYc);
						}
					}
					posXc -= 32;
				}
				posYc -= 32;
			}
		}
	}
	renderer->endGameScene();
	renderer->drawGameScene(16, 0, 128, 72, m_tRect.x1, m_tRect.y1, m_tRect.x2, m_tRect.y2);
}

GUI_Outfit_Colors::GUI_Outfit_Colors(iRect boxRect, Uint32 internalID)
{
	setRect(boxRect);
	m_internalID = internalID;
	m_selected = 0;
	m_hoverColor[0] = m_hoverColor[1] = -1;
	m_Pressed = false;
}

void GUI_Outfit_Colors::onMouseMove(Sint32 x, Sint32 y, bool)
{
	if(!m_Pressed)
		return;

	iRect rect = iRect(m_tRect.x1+4, m_tRect.y1+20, 247, 91);
	if(rect.isPointInside(x, y))
	{
		Sint32 xFactor = (x-m_tRect.x1-4)/13;
		Sint32 yFactor = (y-m_tRect.y1-20)/13;
		Sint32 cFactor = yFactor*19+xFactor;
		if(m_hoverColor[1] == cFactor)
			m_hoverColor[0] = cFactor;
		else
			m_hoverColor[0] = -1;
		return;
	}
}

void GUI_Outfit_Colors::onLMouseDown(Sint32 x, Sint32 y)
{
	m_Pressed = true;

	iRect rect = iRect(m_tRect.x1+4, m_tRect.y1+20, 247, 91);
	if(rect.isPointInside(x, y))
	{
		Sint32 xFactor = (x-m_tRect.x1-4)/13;
		Sint32 yFactor = (y-m_tRect.y1-20)/13;
		Sint32 cFactor = yFactor*19+xFactor;
		m_hoverColor[0] = m_hoverColor[1] = cFactor;
		return;
	}
	rect = iRect(m_tRect.x1, m_tRect.y1, 63, 18);
	if(rect.isPointInside(x, y))
	{
		m_selected = 0;
		return;
	}
	rect = iRect(m_tRect.x1+63, m_tRect.y1, 63, 18);
	if(rect.isPointInside(x, y))
	{
		m_selected = 1;
		return;
	}
	rect = iRect(m_tRect.x1+126, m_tRect.y1, 63, 18);
	if(rect.isPointInside(x, y))
	{
		m_selected = 2;
		return;
	}
	rect = iRect(m_tRect.x1+189, m_tRect.y1, 64, 18);
	if(rect.isPointInside(x, y))
	{
		m_selected = 3;
		return;
	}
}

void GUI_Outfit_Colors::onLMouseUp(Sint32 x, Sint32 y)
{
	if(!m_Pressed)
		return;
	m_Pressed = false;

	iRect rect = iRect(m_tRect.x1+4, m_tRect.y1+20, 247, 91);
	if(rect.isPointInside(x, y))
	{
		Sint32 xFactor = (x-m_tRect.x1-4)/13;
		Sint32 yFactor = (y-m_tRect.y1-20)/13;
		Sint32 cFactor = yFactor*19+xFactor;
		if(m_hoverColor[0] == cFactor)
			g_outfitColors[m_selected] = SDL_static_cast(Uint8, cFactor);
	}
	m_hoverColor[0] = m_hoverColor[1] = -1;
}

void GUI_Outfit_Colors::render()
{
	Surface* renderer = g_engine.getRender();
	renderer->drawPicture(3, 99, 198, m_tRect.x1+m_tRect.x2-2, m_tRect.y1+16, 2, 2);
	renderer->drawPicture(3, 99, 198, m_tRect.x1, m_tRect.y1+m_tRect.y2-2, 2, 2);
	renderer->drawPictureRepeat(3, 2, 198, 96, 2, m_tRect.x1, m_tRect.y1+16, m_tRect.x2-2, 2);
	renderer->drawPictureRepeat(3, 256, 0, 2, 96, m_tRect.x1, m_tRect.y1+18, 2, m_tRect.y2-20);
	renderer->drawPictureRepeat(3, 0, 96, 96, 2, m_tRect.x1+2, m_tRect.y1+m_tRect.y2-2, m_tRect.x2-2, 2);
	renderer->drawPictureRepeat(3, 0, 98, 2, 96, m_tRect.x1+m_tRect.x2-2, m_tRect.y1+18, 2, m_tRect.y2-20);

	renderer->drawPicture(3, 210, 200, m_tRect.x1, m_tRect.y1, 62, 18);
	renderer->drawPicture(3, 304, 200, m_tRect.x1+62, m_tRect.y1, 2, 18);
	renderer->drawPicture(3, 211, 200, m_tRect.x1+64, m_tRect.y1, 61, 18);
	renderer->drawPicture(3, 304, 200, m_tRect.x1+125, m_tRect.y1, 2, 18);
	renderer->drawPicture(3, 211, 200, m_tRect.x1+127, m_tRect.y1, 61, 18);
	renderer->drawPicture(3, 304, 200, m_tRect.x1+188, m_tRect.y1, 2, 18);
	renderer->drawPicture(3, 211, 200, m_tRect.x1+190, m_tRect.y1, 61, 18);
	renderer->drawPicture(3, 304, 200, m_tRect.x1+251, m_tRect.y1, 2, 18);
	switch(m_selected)
	{
		case 1:
		{
			renderer->drawPicture(3, 114, 200, m_tRect.x1+63, m_tRect.y1, 62, 18);
			renderer->drawPicture(3, 208, 200, m_tRect.x1+125, m_tRect.y1, 2, 18);
		}
		break;
		case 2:
		{
			renderer->drawPicture(3, 114, 200, m_tRect.x1+126, m_tRect.y1, 62, 18);
			renderer->drawPicture(3, 208, 200, m_tRect.x1+188, m_tRect.y1, 2, 18);
		}
		break;
		case 3:
		{
			renderer->drawPicture(3, 114, 200, m_tRect.x1+189, m_tRect.y1, 62, 18);
			renderer->drawPicture(3, 208, 200, m_tRect.x1+251, m_tRect.y1, 2, 18);
		}
		break;
		default:
		{
			renderer->drawPicture(3, 114, 200, m_tRect.x1, m_tRect.y1, 62, 18);
			renderer->drawPicture(3, 208, 200, m_tRect.x1+62, m_tRect.y1, 2, 18);
		}
		break;
	}
	g_engine.drawFont(CLIENT_FONT_SMALL, m_tRect.x1+22, m_tRect.y1+5, "Head", 255, 255, 255, CLIENT_FONT_ALIGN_LEFT);
	g_engine.drawFont(CLIENT_FONT_SMALL, m_tRect.x1+78, m_tRect.y1+5, "Primary", 255, 255, 255, CLIENT_FONT_ALIGN_LEFT);
	g_engine.drawFont(CLIENT_FONT_SMALL, m_tRect.x1+136, m_tRect.y1+5, "Secondary", 255, 255, 255, CLIENT_FONT_ALIGN_LEFT);
	g_engine.drawFont(CLIENT_FONT_SMALL, m_tRect.x1+208, m_tRect.y1+5, "Detail", 255, 255, 255, CLIENT_FONT_ALIGN_LEFT);

	Uint8 hoverC = SDL_static_cast(Uint8, m_hoverColor[0]);
	Uint8 c = 0;
	Sint32 posY = m_tRect.y1+20;
	for(Sint32 i = 0; i < 7; ++i)
	{
		Sint32 posX = m_tRect.x1+4;
		for(Sint32 j = 0; j < 19; ++j)
		{
			Uint8 red, green, blue;
			getOutfitColorRGB(c, red, green, blue);
			renderer->fillRectangle(posX+2, posY+2, 8, 8, red, green, blue, 255);
			if(g_outfitColors[m_selected] == c)
			{
				renderer->drawPicture(3, 222, 134, posX, posY, 12, 12);
				renderer->drawRectangle(posX-1, posY-1, 14, 14, 255, 255, 255, 255);
			}
			else
				renderer->drawPicture(3, 222, (hoverC == c ? 134 : 122), posX, posY, 12, 12);
			++c;
			posX += 13;
		}
		posY += 13;
	}
}
