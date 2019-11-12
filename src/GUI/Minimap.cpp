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
#include "../GUI_Elements/GUI_Panel.h"
#include "../GUI_Elements/GUI_PanelWindow.h"
#include "../GUI_Elements/GUI_Button.h"
#include "../GUI_Elements/GUI_Icon.h"
#include "../GUI_Elements/GUI_StaticImage.h"
#include "../GUI_Elements/GUI_Grouper.h"
#include "../automap.h"
#include "Minimap.h"

extern Engine g_engine;
extern Automap g_automap;

void UTIL_createMinimapPanel()
{
	GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_MINIMAP);
	if(pPanel)
		g_engine.removePanelWindow(pPanel);

	GUI_PanelWindow* newWindow = new GUI_PanelWindow(iRect(0, 0, 172, 117), false, GUI_PANEL_WINDOW_MINIMAP);
	GUI_Grouper* newGrouper = new GUI_Grouper(iRect(8, 4, 108, 108));
	newWindow->addChild(newGrouper);
	GUI_Minimap* newMinimap = new GUI_Minimap(iRect(9, 5, 106, 106));
	newWindow->addChild(newMinimap);
	GUI_Button* newButton = new GUI_Button(iRect(124, 92, 43, 20), "Centre");
	newButton->startEvents();
	newWindow->addChild(newButton);
	GUI_StaticImage* newImage = new GUI_StaticImage(iRect(130, 10, 31, 31), 3, 380, 65);
	newWindow->addChild(newImage);
	newImage = new GUI_StaticImage(iRect(124, 4, 43, 43), 3, 131, 138);
	newWindow->addChild(newImage);
	newImage = new GUI_StaticImage(iRect(124, 71, 20, 20), 3, 2, 98);
	newWindow->addChild(newImage);
	newImage = new GUI_StaticImage(iRect(124, 50, 20, 20), 3, 22, 98);
	newWindow->addChild(newImage);
	newImage = new GUI_StaticImage(iRect(147, 50, 20, 20), 3, 42, 98);
	newWindow->addChild(newImage);
	newImage = new GUI_StaticImage(iRect(147, 71, 20, 20), 3, 62, 98);
	newWindow->addChild(newImage);
	GUI_Icon* newIcon = new GUI_Icon(iRect(139, 4, 13, 7), 3, 146, 138, 486, 96);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(139, 40, 13, 7), 3, 146, 174, 499, 96);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(124, 19, 7, 13), 3, 131, 153, 504, 78);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(160, 19, 7, 13), 3, 167, 153, 504, 65);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(126, 6, 12, 12), 3, 133, 140, 450, 96);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(153, 6, 12, 12), 3, 160, 140, 438, 96);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(126, 33, 12, 12), 3, 133, 167, 474, 96);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(153, 33, 12, 12), 3, 160, 167, 462, 96);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	g_engine.addToPanel(newWindow, GUI_PANEL_MAIN);
}

GUI_Minimap::GUI_Minimap(iRect boxRect, Uint32 internalID)
{
	setRect(boxRect);
	m_internalID = internalID;
}

void GUI_Minimap::render()
{
	g_automap.render(m_tRect.x1, m_tRect.y1, m_tRect.x2, m_tRect.y2);
}
