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

#ifndef __FILE_GUI_SCROLLBAR_h_
#define __FILE_GUI_SCROLLBAR_h_

#include "GUI_Element.h"

class GUI_HScrollBar : public GUI_Element
{
	public:
		GUI_HScrollBar(iRect boxRect, Sint32 sSize, Sint32 sPos, Uint32 internalID = 0);
		
		SDL_FORCE_INLINE Sint32 getScrollSize() {return m_scrollSize;}
		SDL_FORCE_INLINE Sint32 getScrollPos() {return m_scrollPos[1];}

		void setRect(iRect& NewRect);
		void setBarEventCallback(void(*eventHandlerFunction)(Uint32, Sint32), Uint32 mEvent);

		void setScrollSize(Sint32 sSize);
		void setScrollPos(Sint32 sPos);

		void onLMouseDown(Sint32 x, Sint32 y);
		void onLMouseUp(Sint32 x, Sint32 y);
		void onMouseMove(Sint32 x, Sint32 y, bool isInsideParent);
		void onWheel(Sint32 x, Sint32 y, bool wheelUP);
		void render();

	protected:
		void (*m_eventHandlerFunction)(Uint32, Sint32);
		float m_scrollScale;
		float m_scrollInvScale;
		Uint32 m_evtParam;
		Uint32 m_lastUpdate;
		Sint32 m_mouseClickX;
		Sint32 m_scrollPosSize;
		Sint32 m_scrollPos[2];
		Sint32 m_scrollSize;
		Uint8 m_buttonDown;
		bool m_haveHandle;
};

class GUI_VScrollBar : public GUI_Element
{
	public:
		GUI_VScrollBar(iRect boxRect, Sint32 sSize, Sint32 sPos, Uint32 internalID = 0);

		SDL_FORCE_INLINE Sint32 getScrollSize() {return m_scrollSize;}
		SDL_FORCE_INLINE Sint32 getScrollPos() {return m_scrollPos[1];}

		void setRect(iRect& NewRect);
		void setBarEventCallback(void(*eventHandlerFunction)(Uint32, Sint32), Uint32 mEvent);

		void setScrollSize(Sint32 sSize);
		void setScrollPos(Sint32 sPos);

		void onLMouseDown(Sint32 x, Sint32 y);
		void onLMouseUp(Sint32 x, Sint32 y);
		void onMouseMove(Sint32 x, Sint32 y, bool isInsideParent);
		void onWheel(Sint32 x, Sint32 y, bool wheelUP);
		void render();

	protected:
		void (*m_eventHandlerFunction)(Uint32, Sint32);
		float m_scrollScale;
		float m_scrollInvScale;
		Uint32 m_evtParam;
		Uint32 m_lastUpdate;
		Sint32 m_mouseClickY;
		Sint32 m_scrollPosSize;
		Sint32 m_scrollPos[2];
		Sint32 m_scrollSize;
		Uint8 m_buttonDown;
		bool m_haveHandle;
};

#endif /* __FILE_GUI_SCROLLBAR_h_ */
