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

#ifndef __FILE_STRUCTURES_h_
#define __FILE_STRUCTURES_h_

#include "defines.h"

struct Hotkey_Text
{
	Uint8 type;
	bool sendAutomatically;
	std::string* text;
};

struct Hotkey_Item
{
	Uint8 type;
	Uint8 usageType;
	Uint16 itemId;
	Uint8 itemSubtype;
};

typedef union HotkeyAction
{
	Uint8 type;
	Hotkey_Text text;
	Hotkey_Item item;
} HotkeyAction;

struct HotkeyUsage
{
	HotkeyAction action;
	SDL_Keycode keycode;
	ClientHotkeys hotkey;
	Uint16 modifers;
	Uint8 keyid;
};

struct ClientActionData
{
	ClientActionData() : creatureId(0), itemId(0), posX(0xFFFF), posY(0), posZ(0), posStack(0) {}

	Uint32 creatureId;
	Uint16 itemId;
	Uint16 posX;
	Uint16 posY;
	Uint8 posZ;
	Uint8 posStack;
};

struct LightMap
{
	LightMap() : r(0), g(0), b(0) {}

	Sint32 r;
	Sint32 g;
	Sint32 b;
};

struct ContextMenuChild
{
	std::string text;
	std::string shortcut;
	Uint32 eventId;
	Uint8 childStyle;
};

struct WorldDetail
{
	std::string worldName;
	std::string worldIp;
	Uint16 worldPort;
	bool previewState;
};

struct CharacterDetail
{
	std::string name;
	std::string vocName;
	std::string worldName;
	std::string worldIp;
	Uint32 health;
	Uint32 healthMax;
	Uint32 mana;
	Uint32 manaMax;
	Uint16 level;
	Uint16 lookType;
	Uint16 worldPort;
	Uint8 lookHead;
	Uint8 lookBody;
	Uint8 lookLegs;
	Uint8 lookFeet;
	Uint8 lookAddons;
	bool previewState;
};

struct ChannelDetail
{
	std::string channelName;
	Uint16 channelId;
};

struct OutfitDetail
{
	std::string outfitName;
	Uint16 outfitID;
	Uint8 outfitAddons;
};

struct MountDetail
{
	std::string mountName;
	Uint16 mountID;
};

struct QuestLogDetail
{
	std::string questName;
	Uint16 questId;
	bool questCompleted;
};

struct QuestLineDetail
{
	std::string missionName;
	std::string missionDescription;
};

struct RewardCollection
{
	Uint16 itemId;
	Uint8 itemCount;
};

struct RequestItems
{
	Uint16 itemId;
	Uint8 itemCount;
};

struct InspectObject_NormalObject
{
	Uint8 type;
	Uint16 posX;
	Uint16 posY;
	Uint8 posZ;
};

struct InspectObject_NPCTrade
{
	Uint8 type;
	Uint16 itemId;
	Uint8 itemCount;
};

struct InspectObject_Cyclopedia
{
	Uint8 type;
	Uint16 itemId;
};

typedef union InspectObjectData
{
	Uint8 type;
	InspectObject_NormalObject normalObject;
	InspectObject_NPCTrade npcTrade;
	InspectObject_Cyclopedia cyclopedia;
} InspectObjectData;

struct LootContainer_AssignContainer
{
	Uint8 action;
	Uint8 objectCategory;
	Uint16 posX;
	Uint16 posY;
	Uint8 posZ;
	Uint16 itemId;
	Uint8 itemCount;
};

struct LootContainer_ClearContainer
{
	Uint8 action;
	Uint8 objectCategory;
};

struct LootContainer_SetUseFallback
{
	Uint8 action;
	bool useFallback;
};

typedef union LootContainerData
{
	Uint8 action;
	LootContainer_AssignContainer assignContainer;
	LootContainer_ClearContainer clearContainer;
	LootContainer_SetUseFallback setUseFallback;
} LootContainerData;

#endif /* __FILE_STRUCTURES_h_ */

