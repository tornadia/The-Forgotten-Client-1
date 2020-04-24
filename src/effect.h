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

#ifndef __FILE_EFFECT_h_
#define __FILE_EFFECT_h_

#include "position.h"
#include "animator.h"

class ThingType;
class Effect
{
	public:
		static std::vector<Effect*> effects;
		static Uint32 effectCount;

		Effect(const Position& pos, Uint16 delay, ThingType* type);
		~Effect();

		static Effect* createEffect(const Position& pos, Uint16 delay, Uint16 type);

		Uint16 getID();
		SDL_FORCE_INLINE const Position& getPos() {return m_position;}
		bool canBeDeleted();
		bool isDelayed();
		bool isTopEffect() {return m_topEffect;}

		static void update();
		virtual void render(Sint32 posX, Sint32 posY, bool visible_tile);

		Uint8 calculateAnimationPhase();

	protected:
		ThingType* m_thingType;
		Animator* m_animator;
		Animation m_animation;
		Position m_position;

		Uint32 m_startTime;
		Uint8 m_currentAnim;
		Uint8 m_animCount;
		Uint8 m_xPattern;
		Uint8 m_yPattern;
		Uint8 m_zPattern;
		bool m_topEffect;
};

class EffectNULL : public Effect
{
	public:
		EffectNULL(const Position& pos, Uint16 delay, ThingType* type);

		virtual void render(Sint32 posX, Sint32 posY, bool visible_tile);
};

class Effect1X1 : public Effect
{
	public:
		Effect1X1(const Position& pos, Uint16 delay, ThingType* type);

		virtual void render(Sint32 posX, Sint32 posY, bool visible_tile);

		Uint32 m_1X1Sprites[EFFECT_MAX_CACHED_ANIMATIONS];
};

class Effect2X1 : public Effect
{
	public:
		Effect2X1(const Position& pos, Uint16 delay, ThingType* type);

		virtual void render(Sint32 posX, Sint32 posY, bool visible_tile);

		Uint32 m_2X1Sprites[EFFECT_MAX_CACHED_ANIMATIONS][2];
};

class Effect1X2 : public Effect
{
	public:
		Effect1X2(const Position& pos, Uint16 delay, ThingType* type);

		virtual void render(Sint32 posX, Sint32 posY, bool visible_tile);

		Uint32 m_1X2Sprites[EFFECT_MAX_CACHED_ANIMATIONS][2];
};

class Effect2X2 : public Effect
{
	public:
		Effect2X2(const Position& pos, Uint16 delay, ThingType* type);

		virtual void render(Sint32 posX, Sint32 posY, bool visible_tile);

		Uint32 m_2X2Sprites[EFFECT_MAX_CACHED_ANIMATIONS][4];
};

#endif /* __FILE_EFFECT_h_ */
