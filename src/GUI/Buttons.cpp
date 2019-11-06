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

extern Engine g_engine;

void UTIL_createButtonsPanel()
{
	GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_BUTTONS);
	if(pPanel)
		g_engine.removePanelWindow(pPanel);

	GUI_PanelWindow* newWindow = new GUI_PanelWindow(iRect(0, 0, 172, 26), false, GUI_PANEL_WINDOW_BUTTONS, true);
	GUI_Button* newButton = new GUI_Button(iRect(8, 3, 34, 20), "Skills");
	newButton->startEvents();
	newWindow->addChild(newButton);
	newButton = new GUI_Button(iRect(45, 3, 34, 20), "Battle");
	newButton->startEvents();
	newWindow->addChild(newButton);
	newButton = new GUI_Button(iRect(82, 3, 34, 20), "VIP");
	newButton->startEvents();
	newWindow->addChild(newButton);
	newButton = new GUI_Button(iRect(124, 3, 43, 20), "Logout");
	newButton->startEvents();
	newWindow->addChild(newButton);
	g_engine.addToPanel(newWindow, GUI_PANEL_MAIN);
}
