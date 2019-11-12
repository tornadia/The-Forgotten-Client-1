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

#ifndef __FILE_GUI_PANEL_h_
#define __FILE_GUI_PANEL_h_

#include "../defines.h"

enum GUIPanelID
{
	GUI_PANEL_MAIN,//Always
	GUI_PANEL_RIGHT1,//Always
	GUI_PANEL_RIGHT2,//Optional
	GUI_PANEL_RIGHT3,//Optional
	GUI_PANEL_RIGHT4,//Optional
	GUI_PANEL_LEFT1,//Optional
	GUI_PANEL_LEFT2,//Optional
	GUI_PANEL_LEFT3,//Optional
	GUI_PANEL_LEFT4,//Optional
	GUI_PANEL_RANDOM = -1
};

class GUI_PanelWindow;
class GUI_Panel
{
	public:
		GUI_Panel(iRect boxRect, Sint32 internalId = 0);
		~GUI_Panel();

		void setRect(iRect& NewRect);
		iRect& getRect() {return m_tRect;}
		bool isInsideRect(Sint32 x, Sint32 y) {return m_tRect.isPointInside(x, y);}
		Sint32 getFreeHeight();

		void clearPanels();
		void resizePanel(GUI_PanelWindow* pPanel, Sint32 x, Sint32 y);
		void checkPanels();
		void checkPanel(GUI_PanelWindow* pPanel, Sint32 x, Sint32 y);
		bool tryFreeHeight(GUI_PanelWindow* pPanel);
		void addPanel(GUI_PanelWindow* pPanel);
		void removePanel(GUI_PanelWindow* pPanel, bool deletePanel = true);
		GUI_PanelWindow* getPanel(Uint32 internalID);

		void setActPanel(GUI_PanelWindow* panel) {m_actPanel = panel;}
		void setInternalID(Sint32 internalId) {m_internalID = internalId;}
		Sint32 getInternalID() {return m_internalID;}

		void* onAction(Sint32 x, Sint32 y);
		void onLMouseDown(Sint32 x, Sint32 y);
		void onLMouseUp(Sint32 x, Sint32 y);
		void onRMouseDown(Sint32 x, Sint32 y);
		void onRMouseUp(Sint32 x, Sint32 y);
		void onWheel(Sint32 x, Sint32 y, bool wheelUP);
		void onMouseMove(Sint32 x, Sint32 y, bool isInsideParent);

		void render();

	protected:
		std::vector<GUI_PanelWindow*> m_panels;
		GUI_PanelWindow* m_actPanel;
		iRect m_tRect;
		Sint32 m_lastPosY;
		Sint32 m_freeHeight;
		Sint32 m_internalID;
};

#endif /* __FILE_GUI_PANEL_h_ */
