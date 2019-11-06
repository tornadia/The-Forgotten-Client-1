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

#ifndef __FILE_ITEM_h_
#define __FILE_ITEM_h_

#include "thing.h"
#include "animator.h"

class ThingType;
class Item : public Thing
{
	public:
		Item(const Position& pos, ThingType* type);

		static Item* createItem(const Position& pos, Uint16 type, Uint16 count, Sint32 phase = AnimationPhase_Automatic);

		virtual Item* getItem() {return this;}
		virtual const Item* getItem() const {return this;}

		virtual bool isItem() {return true;}

		Uint16 getID();
		Uint8 calculateAnimationPhase();
		virtual void render(Sint32 posX, Sint32 posY, bool visible_tile);

		SDL_FORCE_INLINE Sint32 getTopOrder() {return m_topOrder;}
		SDL_FORCE_INLINE Uint8 getAnimationCount() {return m_animCount;}
		SDL_FORCE_INLINE Uint16 getElevation() {return m_elevation;}
		SDL_FORCE_INLINE bool hasElevation() {return m_hasElevation;}
		SDL_FORCE_INLINE ThingType* getThingType() {return m_thingType;}

	protected:
		ThingType* m_thingType;
		Animator* m_animator;
		Animation m_animation;

		Sint32 m_topOrder;
		Uint16 m_elevation;
		Uint8 m_animCount;
		Uint8 m_xPattern;
		Uint8 m_yPattern;
		Uint8 m_zPattern;
		bool m_hasElevation;
};

class ItemNULL : public Item
{
	public:
		ItemNULL(const Position& pos, ThingType* type);

		virtual void render(Sint32 posX, Sint32 posY, bool visible_tile);
};

class Item1X1 : public Item
{
	public:
		Item1X1(const Position& pos, ThingType* type);

		virtual void render(Sint32 posX, Sint32 posY, bool visible_tile);

		Uint32 m_1X1Sprites[ITEM_MAX_CACHED_ANIMATIONS];
};

class Item2X1 : public Item
{
	public:
		Item2X1(const Position& pos, ThingType* type);

		virtual void render(Sint32 posX, Sint32 posY, bool visible_tile);

		Uint32 m_2X1Sprites[ITEM_MAX_CACHED_ANIMATIONS][2];
};

class Item1X2 : public Item
{
	public:
		Item1X2(const Position& pos, ThingType* type);

		virtual void render(Sint32 posX, Sint32 posY, bool visible_tile);

		Uint32 m_1X2Sprites[ITEM_MAX_CACHED_ANIMATIONS][2];
};

class Item2X2 : public Item
{
	public:
		Item2X2(const Position& pos, ThingType* type);

		virtual void render(Sint32 posX, Sint32 posY, bool visible_tile);

		Uint32 m_2X2Sprites[ITEM_MAX_CACHED_ANIMATIONS][4];
};

class Item1X1X2 : public Item
{
	public:
		Item1X1X2(const Position& pos, ThingType* type);

		virtual void render(Sint32 posX, Sint32 posY, bool visible_tile);

		Uint32 m_1X1X2Sprites[ITEM_MAX_CACHED_ANIMATIONS][2];
};

class Item2X1X2 : public Item
{
	public:
		Item2X1X2(const Position& pos, ThingType* type);

		virtual void render(Sint32 posX, Sint32 posY, bool visible_tile);

		Uint32 m_2X1X2Sprites[ITEM_MAX_CACHED_ANIMATIONS][4];
};

class Item1X2X2 : public Item
{
	public:
		Item1X2X2(const Position& pos, ThingType* type);

		virtual void render(Sint32 posX, Sint32 posY, bool visible_tile);

		Uint32 m_1X2X2Sprites[ITEM_MAX_CACHED_ANIMATIONS][4];
};

class Item2X2X2 : public Item
{
	public:
		Item2X2X2(const Position& pos, ThingType* type);

		virtual void render(Sint32 posX, Sint32 posY, bool visible_tile);

		Uint32 m_2X2X2Sprites[ITEM_MAX_CACHED_ANIMATIONS][8];
};

#endif /* __FILE_ITEM_h_ */
