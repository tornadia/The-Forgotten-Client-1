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

#ifndef __FILE_GUI_OUTFITS_h_
#define __FILE_GUI_OUTFITS_h_

#include "../GUI_Elements/GUI_Element.h"

class ThingType;
class GUI_Outfit_View : public GUI_Element
{
	public:
		GUI_Outfit_View(iRect boxRect, Uint32 internalID = 0);
		~GUI_Outfit_View() {;}

		void previousDirection();
		void nextDirection();
		void refresh();

		void startMovement();
		void stopMovement();
		void updateMovement();

		Sint32 getOffsetX();
		Sint32 getOffsetY();

		void render();

	protected:
		ThingType* m_ground;
		ThingType* m_outfit;
		ThingType* m_mount;
		Uint32 m_walkStartTime;
		Sint32 m_walkedPixels;
		Uint8 m_direction;
		Uint8 m_currentAnim;
		Uint8 m_currentFrame;
		bool m_showOutfit;
		bool m_showMount;
		bool m_walking;
};

class GUI_Outfit_Colors : public GUI_Element
{
	public:
		GUI_Outfit_Colors(iRect boxRect, Uint32 internalID = 0);
		~GUI_Outfit_Colors() {;}

		void onMouseMove(Sint32 x, Sint32 y, bool isInsideParent);
		void onLMouseDown(Sint32 x, Sint32 y);
		void onLMouseUp(Sint32 x, Sint32 y);

		void render();

	protected:
		Sint32 m_selected;
		Sint32 m_hoverColor[2];
		bool m_Pressed;
};

#endif /* __FILE_GUI_OUTFITS_h_ */
