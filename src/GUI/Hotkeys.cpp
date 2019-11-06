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
#include "../GUI_Elements/GUI_CheckBox.h"
#include "../GUI_Elements/GUI_ListBox.h"

#define HOTKEYS_TITLE "Hotkeys"
#define HOTKEYS_WIDTH 540
#define HOTKEYS_HEIGHT 485
#define HOTKEYS_CANCEL_EVENTID 1000
#define HOTKEYS_OK_EVENTID 1001

#define HOTKEYS_PRESETS_X 18
#define HOTKEYS_PRESETS_Y 30
#define HOTKEYS_PRESETS_W 378
#define HOTKEYS_PRESETS_H 54
#define HOTKEYS_PRESETS_EVENTID 1002

#define HOTKEYS_ADD_TEXT "Add"
#define HOTKEYS_ADD_X 401
#define HOTKEYS_ADD_Y 34
#define HOTKEYS_ADD_W 58
#define HOTKEYS_ADD_H 20
#define HOTKEYS_ADD_EVENTID 1003

#define HOTKEYS_COPY_TEXT "Copy"
#define HOTKEYS_COPY_X 401
#define HOTKEYS_COPY_Y 59
#define HOTKEYS_COPY_W 58
#define HOTKEYS_COPY_H 20
#define HOTKEYS_COPY_EVENTID 1004

#define HOTKEYS_RENAME_TEXT "Rename"
#define HOTKEYS_RENAME_X 464
#define HOTKEYS_RENAME_Y 34
#define HOTKEYS_RENAME_W 58
#define HOTKEYS_RENAME_H 20
#define HOTKEYS_RENAME_EVENTID 1005

#define HOTKEYS_REMOVE_TEXT "Remove"
#define HOTKEYS_REMOVE_X 464
#define HOTKEYS_REMOVE_Y 59
#define HOTKEYS_REMOVE_W 58
#define HOTKEYS_REMOVE_H 20
#define HOTKEYS_REMOVE_EVENTID 1006

#define HOTKEYS_SWITCH_PRESET_TEXT "Auto-Switch Hotkey Preset"
#define HOTKEYS_SWITCH_PRESET_X 18
#define HOTKEYS_SWITCH_PRESET_Y 89
#define HOTKEYS_SWITCH_PRESET_W 504
#define HOTKEYS_SWITCH_PRESET_H 22
#define HOTKEYS_SWITCH_PRESET_EVENTID 1007
#define HOTKEYS_CHAT_MODE_ON_TEXT "Chat Mode On"
#define HOTKEYS_CHAT_MODE_ON_X 18
#define HOTKEYS_CHAT_MODE_ON_Y 116
#define HOTKEYS_CHAT_MODE_ON_W 247
#define HOTKEYS_CHAT_MODE_ON_H 22
#define HOTKEYS_CHAT_MODE_ON_EVENTID 1008
#define HOTKEYS_CHAT_MODE_OFF_TEXT "Chat Mode Off"
#define HOTKEYS_CHAT_MODE_OFF_X 275
#define HOTKEYS_CHAT_MODE_OFF_Y 116
#define HOTKEYS_CHAT_MODE_OFF_W 247
#define HOTKEYS_CHAT_MODE_OFF_H 22
#define HOTKEYS_CHAT_MODE_OFF_EVENTID 1009

extern Engine g_engine;

struct HotkeyTable
{
	const char* hotkeyName;
	ClientHotkeys hotkeyType;
};

HotkeyTable hotkeyTable[] =
{
	{"Movement:\x0E\xC0\xC0\xC0 Go East", CLIENT_HOTKEY_MOVEMENT_GOEAST},
	{"Movement:\x0E\xC0\xC0\xC0 Go North", CLIENT_HOTKEY_MOVEMENT_GONORTH},
	{"Movement:\x0E\xC0\xC0\xC0 Go North-East", CLIENT_HOTKEY_MOVEMENT_GONORTHEAST},
	{"Movement:\x0E\xC0\xC0\xC0 Go North-West", CLIENT_HOTKEY_MOVEMENT_GONORTHWEST},
	{"Movement:\x0E\xC0\xC0\xC0 Go South", CLIENT_HOTKEY_MOVEMENT_GOSOUTH},
	{"Movement:\x0E\xC0\xC0\xC0 Go South-East", CLIENT_HOTKEY_MOVEMENT_GOSOUTHEAST},
	{"Movement:\x0E\xC0\xC0\xC0 Go South-West", CLIENT_HOTKEY_MOVEMENT_GOSOUTHWEST},
	{"Movement:\x0E\xC0\xC0\xC0 Go West", CLIENT_HOTKEY_MOVEMENT_GOWEST},
	{"Movement:\x0E\xC0\xC0\xC0 Turn East", CLIENT_HOTKEY_MOVEMENT_TURNEAST},
	{"Movement:\x0E\xC0\xC0\xC0 Turn North", CLIENT_HOTKEY_MOVEMENT_TURNNORTH},
	{"Movement:\x0E\xC0\xC0\xC0 Turn South", CLIENT_HOTKEY_MOVEMENT_TURNSOUTH},
	{"Movement:\x0E\xC0\xC0\xC0 Turn West", CLIENT_HOTKEY_MOVEMENT_TURNWEST},
	{"Movement:\x0E\xC0\xC0\xC0 Mount/Dismount", CLIENT_HOTKEY_MOVEMENT_MOUNT},
	{"Movement:\x0E\xC0\xC0\xC0 Stop All Actions", CLIENT_HOTKEY_MOVEMENT_STOPACTIONS},
	{"Dialogs:\x0E\xC0\xC0\xC0 Open Bugreport", CLIENT_HOTKEY_DIALOGS_OPENBUGREPORTS},
	{"Dialogs:\x0E\xC0\xC0\xC0 Open Ignore List", CLIENT_HOTKEY_DIALOGS_OPENIGNORELIST},
	{"Dialogs:\x0E\xC0\xC0\xC0 Open Options", CLIENT_HOTKEY_DIALOGS_OPENOPTIONS},
	{"Dialogs:\x0E\xC0\xC0\xC0 Open Hotkeys", CLIENT_HOTKEY_DIALOGS_OPENHOTKEYS},
	{"Dialogs:\x0E\xC0\xC0\xC0 Open Questlog", CLIENT_HOTKEY_DIALOGS_OPENQUESTLOG},
	{"Dialogs:\x0E\xC0\xC0\xC0 Open Terminal", CLIENT_HOTKEY_DIALOGS_OPENTERMINAL},
	{"Windows:\x0E\xC0\xC0\xC0 Show/Hide VIP List", CLIENT_HOTKEY_WINDOWS_OPENVIPWINDOW},
	{"Windows:\x0E\xC0\xC0\xC0 Show/Hide Battle List", CLIENT_HOTKEY_WINDOWS_OPENBATTLEWINDOW},
	{"Windows:\x0E\xC0\xC0\xC0 Show/Hide Skills Window", CLIENT_HOTKEY_WINDOWS_OPENSKILLSWINDOW},
	{"Chat Channel:\x0E\xC0\xC0\xC0 Close Current Channel", CLIENT_HOTKEY_CHAT_CLOSECHANNEL},
	{"Chat Channel:\x0E\xC0\xC0\xC0 Next Channel", CLIENT_HOTKEY_CHAT_NEXTCHANNEL},
	{"Chat Channel:\x0E\xC0\xC0\xC0 Open Channel List", CLIENT_HOTKEY_CHAT_OPENCHANNELLIST},
	{"Chat Channel:\x0E\xC0\xC0\xC0 Open Help Channel", CLIENT_HOTKEY_CHAT_OPENHELPCHANNEL},
	{"Chat Channel:\x0E\xC0\xC0\xC0 Open NPC Channel", CLIENT_HOTKEY_CHAT_OPENNPCCHANNEL},
	{"Chat Channel:\x0E\xC0\xC0\xC0 Previous Channel", CLIENT_HOTKEY_CHAT_PREVIOUSCHANNEL},
	{"Chat Channel:\x0E\xC0\xC0\xC0 Show Default Channel", CLIENT_HOTKEY_CHAT_DEFAULTCHANNEL},
	{"Chat Mode:\x0E\xC0\xC0\xC0 Toggle Chat[On/Off]", CLIENT_HOTKEY_CHAT_TOGGLECHAT},
	{"Minimap:\x0E\xC0\xC0\xC0 Center", CLIENT_HOTKEY_MINIMAP_CENTER},
	{"Minimap:\x0E\xC0\xC0\xC0 One Floor Down", CLIENT_HOTKEY_MINIMAP_FLOORDOWN},
	{"Minimap:\x0E\xC0\xC0\xC0 One Floor Up", CLIENT_HOTKEY_MINIMAP_FLOORUP},
	{"Minimap:\x0E\xC0\xC0\xC0 Scroll East", CLIENT_HOTKEY_MINIMAP_SCROLLEAST},
	{"Minimap:\x0E\xC0\xC0\xC0 Scroll North", CLIENT_HOTKEY_MINIMAP_SCROLLNORTH},
	{"Minimap:\x0E\xC0\xC0\xC0 Scroll South", CLIENT_HOTKEY_MINIMAP_SCROLLSOUTH},
	{"Minimap:\x0E\xC0\xC0\xC0 Scroll West", CLIENT_HOTKEY_MINIMAP_SCROLLWEST},
	{"Minimap:\x0E\xC0\xC0\xC0 Zoom In", CLIENT_HOTKEY_MINIMAP_ZOOMIN},
	{"Minimap:\x0E\xC0\xC0\xC0 Zoom Out", CLIENT_HOTKEY_MINIMAP_ZOOMOUT},
	{"UI:\x0E\xC0\xC0\xC0 Show/Hide Creature Info", CLIENT_HOTKEY_UI_TOGGLECREATUREINFO},
	{"UI:\x0E\xC0\xC0\xC0 Show/Hide FPS/Ping indicator", CLIENT_HOTKEY_UI_TOGGLEFPSINDICATOR},
	{"UI:\x0E\xC0\xC0\xC0 Toggle Fullscreen", CLIENT_HOTKEY_UI_TOGGLEFULLSCREEN},
	{"Combat:\x0E\xC0\xC0\xC0 Set to Offensive", CLIENT_HOTKEY_COMBAT_SETOFFENSIVE},
	{"Combat:\x0E\xC0\xC0\xC0 Set to Balanced", CLIENT_HOTKEY_COMBAT_SETBALANCED},
	{"Combat:\x0E\xC0\xC0\xC0 Set to Defensive", CLIENT_HOTKEY_COMBAT_SETDEFENSIVE},
	{"Combat:\x0E\xC0\xC0\xC0 Toggle Chase Mode", CLIENT_HOTKEY_COMBAT_TOGGLECHASEMODE},
	{"Combat:\x0E\xC0\xC0\xC0 Toggle Secure Mode", CLIENT_HOTKEY_COMBAT_TOGGLESECUREMODE},
	{"PvP Mode:\x0E\xC0\xC0\xC0 Set to Dove", CLIENT_HOTKEY_PVPMODE_SETDOVE},
	{"PvP Mode:\x0E\xC0\xC0\xC0 Set to Red Fist", CLIENT_HOTKEY_PVPMODE_SETREDFIST},
	{"PvP Mode:\x0E\xC0\xC0\xC0 Set to White Hand", CLIENT_HOTKEY_PVPMODE_SETWHITEHAND},
	{"PvP Mode:\x0E\xC0\xC0\xC0 Set to Yellow Hand", CLIENT_HOTKEY_PVPMODE_SETYELLOWHAND},
	{"Misc.:\x0E\xC0\xC0\xC0 Activate Lenshelp", CLIENT_HOTKEY_MISC_LENSHELP},
	{"Misc.:\x0E\xC0\xC0\xC0 Change Character", CLIENT_HOTKEY_MISC_CHANGECHARACTER},
	{"Misc.:\x0E\xC0\xC0\xC0 Change Outfit", CLIENT_HOTKEY_MISC_CHANGEOUTFIT},
	{"Misc.:\x0E\xC0\xC0\xC0 Logout", CLIENT_HOTKEY_MISC_LOGOUT},
	{"Misc.:\x0E\xC0\xC0\xC0 Next Hotkey Preset", CLIENT_HOTKEY_MISC_NEXTPRESET},
	{"Misc.:\x0E\xC0\xC0\xC0 Previous Hotkey Preset", CLIENT_HOTKEY_MISC_PREVIOUSPRESET},
	{"Misc.:\x0E\xC0\xC0\xC0 Take Screenshot", CLIENT_HOTKEY_MISC_TAKESCREENSHOT}
};

void hotkey_Events(Uint32 event, Sint32)
{
	switch(event)
	{
		case HOTKEYS_CANCEL_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_KEYBOARD)
				g_engine.removeWindow(pWindow);
		}
		break;
		case HOTKEYS_OK_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_KEYBOARD)
				g_engine.removeWindow(pWindow);
		}
		break;
	}
}

void UTIL_hotkeyOptions()
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_KEYBOARD);
	if(pWindow)
		g_engine.removeWindow(pWindow);

	GUI_Window* newWindow = new GUI_Window(iRect(0, 0, HOTKEYS_WIDTH, HOTKEYS_HEIGHT), HOTKEYS_TITLE, GUI_WINDOW_KEYBOARD);

	GUI_ListBox* newListBox = new GUI_ListBox(iRect(HOTKEYS_PRESETS_X, HOTKEYS_PRESETS_Y, HOTKEYS_PRESETS_W, HOTKEYS_PRESETS_H), HOTKEYS_PRESETS_EVENTID);
	newListBox->add("Test0");
	newListBox->add("Test1");
	newListBox->add("Test2");
	newListBox->add("Test3");
	newListBox->add("Test4");
	newListBox->add("Test5");
	newListBox->startEvents();
	newWindow->addChild(newListBox);

	GUI_CheckBox* newCheckBox = new GUI_CheckBox(iRect(HOTKEYS_SWITCH_PRESET_X, HOTKEYS_SWITCH_PRESET_Y, HOTKEYS_SWITCH_PRESET_W, HOTKEYS_SWITCH_PRESET_H), HOTKEYS_SWITCH_PRESET_TEXT, false, HOTKEYS_SWITCH_PRESET_EVENTID);
	newCheckBox->setBoxEventCallback(&hotkey_Events, HOTKEYS_SWITCH_PRESET_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);
	newCheckBox = new GUI_CheckBox(iRect(HOTKEYS_CHAT_MODE_ON_X, HOTKEYS_CHAT_MODE_ON_Y, HOTKEYS_CHAT_MODE_ON_W, HOTKEYS_CHAT_MODE_ON_H), HOTKEYS_CHAT_MODE_ON_TEXT, false, HOTKEYS_CHAT_MODE_ON_EVENTID);
	newCheckBox->setBoxEventCallback(&hotkey_Events, HOTKEYS_CHAT_MODE_ON_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);
	newCheckBox = new GUI_CheckBox(iRect(HOTKEYS_CHAT_MODE_OFF_X, HOTKEYS_CHAT_MODE_OFF_Y, HOTKEYS_CHAT_MODE_OFF_W, HOTKEYS_CHAT_MODE_OFF_H), HOTKEYS_CHAT_MODE_OFF_TEXT, false, HOTKEYS_CHAT_MODE_OFF_EVENTID);
	newCheckBox->setBoxEventCallback(&hotkey_Events, HOTKEYS_CHAT_MODE_OFF_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);

	GUI_Button* newButton = new GUI_Button(iRect(HOTKEYS_WIDTH-56, HOTKEYS_HEIGHT-30, 43, 20), "Cancel", CLIENT_GUI_ESCAPE_TRIGGER);
	newButton->setButtonEventCallback(&hotkey_Events, HOTKEYS_CANCEL_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	newButton = new GUI_Button(iRect(HOTKEYS_WIDTH-109, HOTKEYS_HEIGHT-30, 43, 20), "Ok", CLIENT_GUI_ENTER_TRIGGER);
	newButton->setButtonEventCallback(&hotkey_Events, HOTKEYS_OK_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	newButton = new GUI_Button(iRect(HOTKEYS_ADD_X, HOTKEYS_ADD_Y, HOTKEYS_ADD_W, HOTKEYS_ADD_H), HOTKEYS_ADD_TEXT, HOTKEYS_ADD_EVENTID);
	newButton->setButtonEventCallback(&hotkey_Events, HOTKEYS_ADD_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	newButton = new GUI_Button(iRect(HOTKEYS_COPY_X, HOTKEYS_COPY_Y, HOTKEYS_COPY_W, HOTKEYS_COPY_H), HOTKEYS_COPY_TEXT, HOTKEYS_COPY_EVENTID);
	newButton->setButtonEventCallback(&hotkey_Events, HOTKEYS_COPY_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	newButton = new GUI_Button(iRect(HOTKEYS_RENAME_X, HOTKEYS_RENAME_Y, HOTKEYS_RENAME_W, HOTKEYS_RENAME_H), HOTKEYS_RENAME_TEXT, HOTKEYS_RENAME_EVENTID);
	newButton->setButtonEventCallback(&hotkey_Events, HOTKEYS_RENAME_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	newButton = new GUI_Button(iRect(HOTKEYS_REMOVE_X, HOTKEYS_REMOVE_Y, HOTKEYS_REMOVE_W, HOTKEYS_REMOVE_H), HOTKEYS_REMOVE_TEXT, HOTKEYS_REMOVE_EVENTID);
	newButton->setButtonEventCallback(&hotkey_Events, HOTKEYS_REMOVE_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	GUI_Separator* newSeparator = new GUI_Separator(iRect(13, HOTKEYS_HEIGHT-40, HOTKEYS_WIDTH-26, 2));
	newWindow->addChild(newSeparator);
	g_engine.addWindow(newWindow, true);
}
