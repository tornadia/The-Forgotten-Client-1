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

#ifndef __FILE_THINGMANAGER_h_
#define __FILE_THINGMANAGER_h_

#include "defines.h"

class Animator;
enum ThingCategory : Sint32
{
	ThingCategory_Item = 0,
	ThingCategory_Creature = 1,
	ThingCategory_Effect = 2,
	ThingCategory_DistanceEffect = 3,
	ThingCategory_Invalid = 4,
	ThingCategory_Last = ThingCategory_Invalid
};

enum ThingAttribute : Uint64
{
	ThingAttribute_Ground = (1 << 0),
	ThingAttribute_GroundBorder = (1 << 1),
	ThingAttribute_OnBottom = (1 << 2),
	ThingAttribute_OnTop = (1 << 3),
	ThingAttribute_Container = (1 << 4),
	ThingAttribute_Stackable = (1 << 5),
	ThingAttribute_ForceUse = (1 << 6),
	ThingAttribute_MultiUse = (1 << 7),
	ThingAttribute_Writable = (1 << 8),
	ThingAttribute_WritableOnce = (1 << 9),
	ThingAttribute_FluidContainer = (1 << 10),
	ThingAttribute_Splash = (1 << 11),
	ThingAttribute_NotWalkable = (1 << 12),
	ThingAttribute_NotMoveable = (1 << 13),
	ThingAttribute_BlockProjectile = (1 << 14),
	ThingAttribute_NotPathable = (1 << 15),
	ThingAttribute_Pickupable = (1 << 16),
	ThingAttribute_Hangable = (1 << 17),
	ThingAttribute_HookSouth = (1 << 18),
	ThingAttribute_HookEast = (1 << 19),
	ThingAttribute_Rotateable = (1 << 20),
	ThingAttribute_Light = (1 << 21),
	ThingAttribute_DontHide = (1 << 22),
	ThingAttribute_Translucent = (1 << 23),
	ThingAttribute_Displacement = (1 << 24),
	ThingAttribute_Elevation = (1 << 25),
	ThingAttribute_LyingCorpse = (1 << 26),
	ThingAttribute_AnimateAlways = (1 << 27),
	ThingAttribute_MinimapColor = (1 << 28),
	ThingAttribute_LensHelp = (1 << 29),
	ThingAttribute_FullGround = (1 << 30),
	ThingAttribute_LookThrough = (SDL_static_cast(Uint64, 1) << 31),
	ThingAttribute_Cloth = (SDL_static_cast(Uint64, 1) << 32),
	ThingAttribute_Market = (SDL_static_cast(Uint64, 1) << 33),
	ThingAttribute_DefaultAction = (SDL_static_cast(Uint64, 1) << 34),
	ThingAttribute_Wrapable = (SDL_static_cast(Uint64, 1) << 35),
	ThingAttribute_Unwrapable = (SDL_static_cast(Uint64, 1) << 36),
	ThingAttribute_TopEffect = (SDL_static_cast(Uint64, 1) << 37),
	ThingAttribute_FloorChange = (SDL_static_cast(Uint64, 1) << 38),
	ThingAttribute_NoMoveAnimation = (SDL_static_cast(Uint64, 1) << 39),
	ThingAttribute_Chargeable = (SDL_static_cast(Uint64, 1) << 40),
	ThingAttribute_Usable = (SDL_static_cast(Uint64, 1) << 41),
};

enum ThingFrameGroup : Uint8
{
	ThingFrameGroup_Default = 0,
	ThingFrameGroup_Idle = ThingFrameGroup_Default,
	ThingFrameGroup_Moving = 1,
	ThingFrameGroup_Last = 2
};

struct FrameGroup
{
	std::vector<Uint32> m_sprites;
	Animator* m_animator;

	Uint8 m_width;
	Uint8 m_height;
	Uint8 m_realSize;
	Uint8 m_layers;
	Uint8 m_patternX;
	Uint8 m_patternY;
	Uint8 m_patternZ;
	Uint8 m_animCount;
};

struct MarketData
{
	std::string m_name;
	Uint16 m_category;
	Uint16 m_requiredLevel;
	Uint16 m_restrictVocation;
	Uint16 m_showAs;
	Uint16 m_tradeAs;
};

class ThingType
{
	public:
		ThingType();

		void clear();

		bool loadType(Uint16 id, ThingCategory category, SDL_RWops* rwops);
		bool loadAppearance(Sint64 offsetLimit, Uint16& things, SDL_RWops* rwops);
		SDL_FORCE_INLINE bool hasFlag(Uint64 flag) {return (m_flags & flag);}

		Uint32 getSprite(ThingFrameGroup f, Uint8 w, Uint8 h, Uint8 l, Uint8 x, Uint8 y, Uint8 z, Uint8 a);

		FrameGroup m_frameGroup[ThingFrameGroup_Last];
		MarketData m_marketData;
		Uint64 m_flags;
		ThingCategory m_category;

		Uint16 m_groundSpeed;
		Uint16 m_writableSize[2];
		Uint16 m_displacement[2];
		Uint16 m_light[2];
		Uint16 m_elevation;
		Uint16 m_minimapColor;
		Uint16 m_lensHelp;
		Uint16 m_cloth;
		Uint16 m_defaultAction;

		Uint16 m_id;
};

class ThingManager
{
	public:
		ThingManager();
		~ThingManager();

		void clear();
		void unloadDat();

		bool loadDat(const char* filename);
		bool loadAppearances(const char* filename);
		ThingType* getThingType(ThingCategory category, Uint16 id);

		SDL_FORCE_INLINE bool isDatLoaded() {return m_datLoaded;}
		SDL_FORCE_INLINE bool isValidDatId(ThingCategory category, Uint16 id) {return (id > 0 && id < m_things[category].size());}

	protected:
		std::vector<ThingType> m_things[ThingCategory_Last];

		bool m_datLoaded;
};

#endif /* __FILE_THINGMANAGER_h_ */
