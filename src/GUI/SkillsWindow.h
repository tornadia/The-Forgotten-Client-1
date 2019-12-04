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

#ifndef __FILE_GUI_SKILLSWINDOW_h_
#define __FILE_GUI_SKILLSWINDOW_h_

#include "../GUI_Elements/GUI_Element.h"
#include "../GUI_Elements/GUI_Container.h"

class GUI_SkillsContainer : public GUI_Container
{
	public:
		GUI_SkillsContainer(iRect boxRect, GUI_PanelWindow* parent, Uint32 internalID = 0);

		void onRMouseDown(Sint32 x, Sint32 y);
		void onRMouseUp(Sint32 x, Sint32 y);

	protected:
		bool m_rmouse;
};

class GUI_SkillSeparator : public GUI_Element
{
	public:
		GUI_SkillSeparator(iRect boxRect, Uint32 internalID = 0);

		void render();
};

class GUI_SkillBar : public GUI_Element
{
	public:
		GUI_SkillBar(iRect boxRect, Uint32 internalID = 0);

		void setPercent(Uint8 percent);
		void setColor(Uint8 red, Uint8 green, Uint8 blue) {m_red = red; m_green = green; m_blue = blue;}
		void render();

	protected:
		Sint32 m_percent;
		Uint8 m_red;
		Uint8 m_green;
		Uint8 m_blue;
};

class GUI_SkillDescription : public GUI_Element
{
	public:
		GUI_SkillDescription(iRect boxRect, Uint32 internalID = 0);
		
		void setName(bool value, const std::string labelName);
		void setColor(bool value, Uint8 red, Uint8 green, Uint8 blue);
		void setAlign(bool value, Uint8 align);
		void setFont(bool value, Uint8 font);
		void onMouseMove(Sint32 x, Sint32 y, bool isInsideParent);
		void render();

	protected:
		struct
		{
			std::string m_Label;
			Sint32 m_align;
			Uint8 m_font;
			Uint8 m_red;
			Uint8 m_green;
			Uint8 m_blue;
		} skillName;

		struct
		{
			std::string m_Label;
			Uint32 m_nameLen;
			Sint32 m_align;
			Uint8 m_font;
			Uint8 m_red;
			Uint8 m_green;
			Uint8 m_blue;
		} skillValue;
};

#endif /* __FILE_GUI_SKILLSWINDOW_h_ */
