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

#ifndef __FILE_GUI_UTIL_h_
#define __FILE_GUI_UTIL_h_

#include "../defines.h"

void UTIL_createMainWindow();
void UTIL_createEnterGame();
void UTIL_protocolOptions();
void UTIL_options();
void UTIL_info();
void UTIL_exitWarning();

void UTIL_generalOptions();
void UTIL_graphicsOptions();
void UTIL_advancedGraphicsOptions();
void UTIL_consoleOptions();
void UTIL_hotkeyOptions();

void UTIL_messageBox(const std::string& title, const std::string& message);
void UTIL_createMotd();
void UTIL_createCharacterList();
void UTIL_createQuestLog(Uint32 questId, std::vector<QuestLogDetail>& questLogElements);
void UTIL_createQuestine(Uint16 questId, std::vector<QuestLineDetail>& questLineElements);

void UTIL_createMinimapPanel();
void UTIL_createHealthPanel();
void UTIL_createInventoryPanel();
void UTIL_createButtonsPanel();
void UTIL_createSkillsWindow();
void UTIL_createSkillsPopupMenu(Sint32 x, Sint32 y);
void UTIL_createBattleWindow();
void UTIL_createBattlePopupMenu(void* data, Sint32 x, Sint32 y);
void UTIL_createVipWindow();
void UTIL_createContainerWindow(Uint8 index);

void UTIL_updateHealthPanel();
void UTIL_updateSkillsWindowStats();
void UTIL_updateSkillsWindowSkills();
void UTIL_sortBattleWindow(SortMethods method);

void UTIL_flashQuestsButton();
void UTIL_flashBattleWindow();
void UTIL_flashHealthPanel();
void UTIL_flashFollowButton();

void UTIL_deathWindow(Uint8 deathType, Uint8 penalty);
void UTIL_createChannels(std::vector<ChannelDetail>& channels);
void UTIL_createOutfitWindow(Uint16 lookType, Uint8 lookHead, Uint8 lookBody, Uint8 lookLegs, Uint8 lookFeet, Uint8 lookAddons, Uint16 lookMount, std::vector<OutfitDetail>& outfits, std::vector<MountDetail>& mounts);
void UTIL_createTutorialHint(Uint32 hintId);

#endif /* __FILE_GUI_UTIL_h_ */
