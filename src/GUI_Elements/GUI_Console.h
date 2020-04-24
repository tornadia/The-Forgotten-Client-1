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

#ifndef __FILE_GUI_CONSOLE_h_
#define __FILE_GUI_CONSOLE_h_

#include "GUI_Element.h"

#define MAX_CONSOLE_MESSAGES 1000

struct ConsoleMessage
{
	std::string name;
	std::string message;
	time_t timestamp;
	Uint32 flags;
	Uint32 statementId;
	Uint16 level;
	Uint8 red;
	Uint8 green;
	Uint8 blue;
};

struct ConsoleLine
{
	size_t lineStart;
	size_t lineLength;
	Uint32 startPosition;
	Sint32 selectionStart;
	Sint32 selectionWidth;
	Sint32 messageIndex;
	Uint8 red;
	Uint8 green;
	Uint8 blue;
};

class GUI_VScrollBar;
class GUI_Console : public GUI_Element
{
	public:
		GUI_Console(iRect boxRect, Uint32 internalID = 0);
		~GUI_Console();

		void setRect(iRect& NewRect);
		void setFont(Uint8 font) {m_font = font;}

		void addMessage(Uint32 statementId, time_t timestamp, const std::string name, Uint16 level, const std::string message, Uint8 red, Uint8 green, Uint8 blue, Uint32 flags = 0);
		void setCursor(Uint32 position);
		void moveCursor(Sint32 n);
		void moveCursorVertically(Sint32 n);
		Uint32 getCursorPosition(Sint32 x, Sint32 y);
		ConsoleMessage* getConsoleMessage(Sint32 x, Sint32 y);
		Uint32 getCursor() {return m_cursorPosition;}
		std::string& getCachedText() {return m_sText;}

		void clearConsole();
		void selectAll();
		void setSelection(Uint32 start, Uint32 end);
		void clearSelection();
		bool hasSelection() {return m_selectionEnd != m_selectionStart;}

		void onKeyDown(SDL_Event& event);
		void onLMouseDown(Sint32 x, Sint32 y);
		void onLMouseUp(Sint32 x, Sint32 y);
		void onMouseMove(Sint32 x, Sint32 y, bool isInsideParent);
		void onWheel(Sint32 x, Sint32 y, bool wheelUP);
		void deActivate();
		void update();
		void updateSelection();
		void render();

	protected:
		std::list<ConsoleMessage> m_messages;
		std::vector<ConsoleLine> m_lines;
		std::string m_sText;
		GUI_VScrollBar* m_scrollBar;
		Uint32 m_cursorPosition;
		Uint32 m_selectionReference;
		Uint32 m_selectionStart;
		Uint32 m_selectionEnd;
		Sint32 m_maxDisplay;
		Uint8 m_font;
		bool m_selecting;
		bool m_needUpdate;
		bool m_needUpdateSelection;
		bool m_keepLastScrollPos;
};

#endif /* __FILE_GUI_CONSOLE_h_ */
