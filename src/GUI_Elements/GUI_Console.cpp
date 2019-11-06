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

#include "GUI_Console.h"
#include "GUI_ScrollBar.h"
#include "../engine.h"

extern Engine g_engine;
extern Uint32 g_frameTime;

GUI_Console::GUI_Console(iRect boxRect, Uint32 internalID)
{
	m_scrollBar = new GUI_VScrollBar(iRect(0, 0, 0, 0), 0, 0);
	setRect(boxRect);
	m_internalID = internalID;
	m_cursorPosition = 0;
	m_selectionReference = 0;
	m_selectionStart = 0;
	m_selectionEnd = 0;
	m_maxDisplay = 0;
	m_font = CLIENT_FONT_NONOUTLINED;
	m_selecting = false;
	m_needUpdate = true;
	m_needUpdateSelection = false;
	m_keepLastScrollPos = false;
}

GUI_Console::~GUI_Console()
{
	delete m_scrollBar;
}

void GUI_Console::setRect(iRect& NewRect)
{
	if(m_tRect == NewRect)
		return;

	iRect nRect = iRect(NewRect.x1+NewRect.x2-13, NewRect.y1+1, 12, NewRect.y2-2);
	m_scrollBar->setRect(nRect);
	m_tRect = NewRect;
	m_maxDisplay = (m_tRect.y2-4)/14;
	m_needUpdate = true;
	m_keepLastScrollPos = (m_scrollBar->getScrollPos() == m_scrollBar->getScrollSize());
}

void GUI_Console::setSelection(Uint32 start, Uint32 end)
{
	if(start == m_selectionStart && end == m_selectionEnd)
		return;

	if(start > end)
		std::swap(start, end);

	m_selectionStart = UTIL_max<Uint32>(0, UTIL_min<Uint32>(SDL_static_cast(Uint32, m_sText.length()), start));
	m_selectionEnd = UTIL_max<Uint32>(0, UTIL_min<Uint32>(SDL_static_cast(Uint32, m_sText.length()), end));
	m_needUpdateSelection = true;
}

void GUI_Console::clearSelection()
{
	if(!hasSelection())
		return;

	m_selectionStart = 0;
	m_selectionEnd = 0;
	m_needUpdateSelection = true;
}

void GUI_Console::onKeyDown(SDL_Event event)
{
	switch(event.key.keysym.sym)
	{
		case SDLK_LEFT:
		{
			if(event.key.keysym.mod == KMOD_SHIFT)
			{
				if(!hasSelection())
					m_selectionReference = m_cursorPosition;

				moveCursor(-1);
				setSelection(m_selectionReference, m_cursorPosition);
			}
			else
			{
				clearSelection();
				moveCursor(-1);
			}
		}
		break;
		case SDLK_RIGHT:
		{
			if(event.key.keysym.mod == KMOD_SHIFT)
			{
				if(!hasSelection())
					m_selectionReference = m_cursorPosition;

				moveCursor(1);
				setSelection(m_selectionReference, m_cursorPosition);
			}
			else
			{
				clearSelection();
				moveCursor(1);
			}
		}
		break;
		case SDLK_HOME:
		{
			if(event.key.keysym.mod == KMOD_SHIFT)
			{
				if(m_cursorPosition != 0)
				{
					setSelection(m_cursorPosition, 0);
					setCursor(0);
				}
			}
			else
			{
				if(m_cursorPosition != 0)
				{
					clearSelection();
					setCursor(0);
				}
			}
		}
		break;
		case SDLK_END:
		{
			if(event.key.keysym.mod == KMOD_SHIFT)
			{
				if(m_cursorPosition != SDL_static_cast(Uint32, m_sText.length()))
				{
					setSelection(m_cursorPosition, SDL_static_cast(Uint32, m_sText.length()));
					setCursor(SDL_static_cast(Uint32, m_sText.length()));
				}
			}
			else
			{
				if(m_cursorPosition != SDL_static_cast(Uint32, m_sText.length()))
				{
					clearSelection();
					setCursor(SDL_static_cast(Uint32, m_sText.length()));
				}
			}
		}
		break;
		case SDLK_UP:
		{
			if(event.key.keysym.mod == KMOD_SHIFT)
			{
				if(m_cursorPosition != 0)
				{
					if(!hasSelection())
						m_selectionReference = m_cursorPosition;

					moveCursorVertically(-1);
					setSelection(m_selectionReference, m_cursorPosition);
				}
			}
			else
			{
				if(m_cursorPosition != 0)
				{
					clearSelection();
					moveCursorVertically(-1);
				}
			}
		}
		break;
		case SDLK_DOWN:
		{
			if(event.key.keysym.mod == KMOD_SHIFT)
			{
				if(m_cursorPosition != SDL_static_cast(Uint32, m_sText.length()))
				{
					if(!hasSelection())
						m_selectionReference = m_cursorPosition;

					moveCursorVertically(1);
					setSelection(m_selectionReference, m_cursorPosition);
				}
			}
			else
			{
				if(m_cursorPosition != SDL_static_cast(Uint32, m_sText.length()))
				{
					clearSelection();
					moveCursorVertically(1);
				}
			}
		}
		break;
		case SDLK_a:
		{
			if(event.key.keysym.mod == KMOD_CTRL)
			{
				m_selectionReference = 0;
				setSelection(m_selectionReference, SDL_static_cast(Uint32, m_sText.length()));
				setCursor(SDL_static_cast(Uint32, m_sText.length()));
			}
		}
		break;
		case SDLK_x:
		{
			if(event.key.keysym.mod == KMOD_CTRL)
			{
				if(hasSelection())
					UTIL_SetClipboardTextLatin1(m_sText.substr(SDL_static_cast(size_t, m_selectionStart), SDL_static_cast(size_t, m_selectionEnd-m_selectionStart)).c_str());
			}
		}
		break;
		case SDLK_c:
		{
			if(event.key.keysym.mod == KMOD_CTRL)
			{
				if(hasSelection())
					UTIL_SetClipboardTextLatin1(m_sText.substr(SDL_static_cast(size_t, m_selectionStart), SDL_static_cast(size_t, m_selectionEnd-m_selectionStart)).c_str());
			}
		}
		break;
	}
}

void GUI_Console::onLMouseDown(Sint32 x, Sint32 y)
{
	if(m_scrollBar->getRect().isPointInside(x, y))
	{
		m_scrollBar->onLMouseDown(x, y);
		return;
	}
	m_selecting = true;
	m_cursorPosition = getCursorPosition(x, y);

	m_selectionReference = m_cursorPosition;
	setSelection(m_selectionReference, m_cursorPosition);
}

void GUI_Console::onLMouseUp(Sint32 x, Sint32 y)
{
	m_scrollBar->onLMouseUp(x, y);
	m_selecting = false;
}

void GUI_Console::onMouseMove(Sint32 x, Sint32 y, bool isInsideParent)
{
	m_scrollBar->onMouseMove(x, y, isInsideParent);
	if(m_selecting)
	{
		m_cursorPosition = getCursorPosition(x, y);
		setSelection(m_selectionReference, m_cursorPosition);
	}
}

void GUI_Console::onWheel(Sint32, Sint32, bool wheelUP)
{
	if(wheelUP)
		m_scrollBar->setScrollPos(m_scrollBar->getScrollPos()-1);
	else
		m_scrollBar->setScrollPos(m_scrollBar->getScrollPos()+1);
}

void GUI_Console::deActivate()
{
	m_bActive = false;
	clearSelection();
}

void GUI_Console::update()
{
	m_sText.clear();
	m_lines.clear();
	
	bool firstMessage = true;
	Uint32 allowWidth = SDL_static_cast(Uint32, m_tRect.x2-28);
	size_t start = 0;
	size_t i = 0;
	Sint32 index = 0;
	for(std::list<ConsoleMessage>::iterator it = m_messages.begin(), end = m_messages.end(); it != end; ++it, ++index)
	{
		ConsoleMessage& cMessage = (*it);
		Sint32 len;
		if(g_engine.hasShowTimestamps())
		{
			std::string timeStr = UTIL_formatDate("%H:%M", cMessage.timestamp);
			if(!cMessage.name.empty())
			{
				if(g_engine.hasShowLevels() && cMessage.level > 0)
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s %s[%u]: %s", timeStr.c_str(), cMessage.name.c_str(), SDL_static_cast(Uint32, cMessage.level), cMessage.message.c_str());
				else
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s %s: %s", timeStr.c_str(), cMessage.name.c_str(), cMessage.message.c_str());
			}
			else
				len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s %s", timeStr.c_str(), cMessage.message.c_str());
		}
		else
		{
			if(!cMessage.name.empty())
			{
				if(g_engine.hasShowLevels() && cMessage.level > 0)
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s[%u]: %s", cMessage.name.c_str(), SDL_static_cast(Uint32, cMessage.level), cMessage.message.c_str());
				else
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s: %s", cMessage.name.c_str(), cMessage.message.c_str());
			}
			else
				len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", cMessage.message.c_str());
		}
		if(firstMessage)
		{
			m_sText.append(std::string(g_buffer, SDL_static_cast(size_t, len)));
			firstMessage = false;
		}
		else
		{
			m_sText.append(1, '\n');
			m_sText.append(std::string(g_buffer, SDL_static_cast(size_t, len)));
			start = i+1;//Skip newline because we already handled it in loop
			i = start;
		}

		bool firstLine = true;
		size_t goodPos = 0;
		size_t strLen = m_sText.length();
		while(i < strLen)
		{
			ConsoleLine newLine;
			newLine.startPosition = (firstLine ? 4 : 14);
			newLine.selectionWidth = 0;
			newLine.messageIndex = index;
			newLine.red = cMessage.red;
			newLine.green = cMessage.green;
			newLine.blue = cMessage.blue;
			if(m_sText[i] == '\n')
			{
				newLine.lineStart = start;
				newLine.lineLength = i-start+1;
				m_lines.push_back(newLine);
				firstLine = false;
				start = i+1;
				i = start;
				goodPos = 0;
				continue;
			}

			Uint32 width = g_engine.calculateFontWidth(m_font, m_sText, start, i-start+1);
			if(width >= allowWidth)
			{
				if(goodPos != 0)
				{
					newLine.lineStart = start;
					newLine.lineLength = goodPos-start+1;
					m_lines.push_back(newLine);
					firstLine = false;
					start = goodPos+1;
					goodPos = 0;
					continue;
				}
				else
				{
					newLine.lineStart = start;
					newLine.lineLength = i-start;
					m_lines.push_back(newLine);
					firstLine = false;
					start = i;
				}
			}
			else
			{
				if(m_sText[i] == ' ')
					goodPos = i;
			}
			++i;
		}
		if(i > start)
		{
			ConsoleLine newLine;
			newLine.lineStart = start;
			newLine.lineLength = i-start+1;
			newLine.startPosition = (firstLine ? 4 : 14);
			newLine.selectionWidth = 0;
			newLine.messageIndex = index;
			newLine.red = cMessage.red;
			newLine.green = cMessage.green;
			newLine.blue = cMessage.blue;
			m_lines.push_back(newLine);
		}
		else if(m_sText[i-1] == '\n')
		{
			ConsoleLine newLine;
			newLine.lineStart = i;
			newLine.lineLength = 1;
			newLine.startPosition = (firstLine ? 4 : 14);
			newLine.selectionWidth = 0;
			newLine.messageIndex = index;
			newLine.red = cMessage.red;
			newLine.green = cMessage.green;
			newLine.blue = cMessage.blue;
			m_lines.push_back(newLine);
		}
	}

	m_scrollBar->setScrollSize(SDL_static_cast(Sint32, m_lines.size())-m_maxDisplay);
	if(hasSelection())
		m_needUpdateSelection = true;
}

void GUI_Console::updateSelection()
{
	Sint32 vectorPos = 0, cursorPos = 0;
	for(std::vector<ConsoleLine>::iterator it = m_lines.begin(), end = m_lines.end(); it != end; ++it)
	{
		ConsoleLine& currentLine = (*it);
		Uint32 selectionStart = UTIL_max<Uint32>(SDL_static_cast(Uint32, currentLine.lineStart), m_selectionStart);
		Uint32 selectionLen = UTIL_min<Uint32>(SDL_static_cast(Uint32, currentLine.lineStart+currentLine.lineLength), m_selectionEnd)-selectionStart;
		if(selectionLen <= SDL_static_cast(Uint32, currentLine.lineLength))
		{
			Uint32 selectionWidth = g_engine.calculateFontWidth(m_font, m_sText, selectionStart, selectionLen);
			Uint32 positionOfSelection = currentLine.startPosition + g_engine.calculateFontWidth(m_font, m_sText, currentLine.lineStart, selectionStart-currentLine.lineStart);
			currentLine.selectionStart = SDL_static_cast(Sint32, positionOfSelection);
			currentLine.selectionWidth = SDL_static_cast(Sint32, selectionWidth);
		}
		else
			currentLine.selectionWidth = 0;

		size_t position = SDL_static_cast(size_t, m_cursorPosition)-currentLine.lineStart;
		if(position < currentLine.lineLength)
			cursorPos = vectorPos;

		++vectorPos;
	}

	//Keep us in the cursor sight
	Sint32 actualPos = m_scrollBar->getScrollPos();
	if(actualPos > cursorPos)
		m_scrollBar->setScrollPos(cursorPos);
	else if(actualPos < cursorPos-m_maxDisplay+1)
		m_scrollBar->setScrollPos(cursorPos-m_maxDisplay+1);
}

void GUI_Console::render()
{
	if(m_needUpdate)
	{
		update();
		m_needUpdate = false;
	}
	if(m_needUpdateSelection)
	{
		updateSelection();
		m_needUpdateSelection = false;
	}
	if(m_keepLastScrollPos)
	{
		m_scrollBar->setScrollPos(m_scrollBar->getScrollSize());
		m_keepLastScrollPos = false;
	}

	Surface* renderer = g_engine.getRender();
	m_scrollBar->render();

	Sint32 count = m_scrollBar->getScrollPos();
	std::vector<ConsoleLine>::iterator it = m_lines.begin();
	std::advance(it, count);

	Sint32 messagesVisible = UTIL_min<Sint32>(m_maxDisplay, SDL_static_cast(Sint32, m_lines.size()));
	Sint32 posY = m_tRect.y1+m_tRect.y2-(messagesVisible*14);
	Sint32 endY = m_tRect.y1+m_tRect.y2-14;
	for(std::vector<ConsoleLine>::iterator end = m_lines.end(); it != end; ++it)
	{
		if(posY > endY)
			break;

		ConsoleLine& currentLine = (*it);
		if(currentLine.selectionWidth != 0)
			renderer->fillRectangle(m_tRect.x1+currentLine.selectionStart, posY-1, currentLine.selectionWidth, 14, 128, 128, 128, 255);

		g_engine.drawFont(m_font, m_tRect.x1+currentLine.startPosition, posY, m_sText, currentLine.red, currentLine.green, currentLine.blue, CLIENT_FONT_ALIGN_LEFT, currentLine.lineStart, currentLine.lineLength);
		posY += 14;
	}
}

void GUI_Console::addMessage(Uint32 statementId, time_t timestamp, const std::string name, Uint16 level, const std::string message, Uint8 red, Uint8 green, Uint8 blue, Uint32 flags)
{
	ConsoleMessage newMessage;
	newMessage.flags = flags;
	newMessage.statementId = statementId;
	newMessage.timestamp = timestamp;
	PERFORM_MOVE(newMessage.name, name);
	newMessage.level = level;
	PERFORM_MOVE(newMessage.message, message);
	newMessage.red = red;
	newMessage.green = green;
	newMessage.blue = blue;
	m_messages.push_back(newMessage);
	if(m_messages.size() > MAX_CONSOLE_MESSAGES)
		m_messages.pop_front();
	m_needUpdate = true;
	m_keepLastScrollPos = (m_scrollBar->getScrollPos() == m_scrollBar->getScrollSize());
}

void GUI_Console::setCursor(Uint32 position)
{
	m_cursorPosition = UTIL_max<Uint32>(0, UTIL_min<Uint32>(SDL_static_cast(Uint32, m_sText.length()), position));
	m_needUpdateSelection = true;
}

void GUI_Console::moveCursor(Sint32 n)
{
	Sint32 pos = UTIL_max<Sint32>(0, SDL_static_cast(Sint32, m_cursorPosition)+n);
	setCursor(SDL_static_cast(Uint32, pos));
}

void GUI_Console::moveCursorVertically(Sint32 n)
{
	if(m_lines.size() < 2)
		return;

	size_t positionInLine = 0;
	std::vector<ConsoleLine>::iterator it = m_lines.begin(), end = m_lines.end();
	for(; it != end; ++it)
	{
		ConsoleLine& currentLine = (*it);
		size_t position = SDL_static_cast(size_t, m_cursorPosition)-currentLine.lineStart;
		if(position < currentLine.lineLength)
		{
			positionInLine = position;
			break;
		}
	}
	if(n > 0)
	{
		for(Sint32 i = 0; i < n; ++i)
		{
			if(++it == end)
			{
				--it;
				break;
			}
		}
		if((*it).lineLength <= positionInLine)
			positionInLine = (*it).lineLength-1;
		setCursor(SDL_static_cast(Uint32, (*it).lineStart+positionInLine));
	}
	else if(n < 0)
	{
		n *= -1;
		end = m_lines.begin();
		if(it == end)
			return;
		for(Sint32 i = 0; i < n; ++i)
		{
			if(--it == end)
				break;
		}
		if((*it).lineLength <= positionInLine)
			positionInLine = (*it).lineLength-1;
		setCursor(SDL_static_cast(Uint32, (*it).lineStart+positionInLine));
	}
}

Uint32 GUI_Console::getCursorPosition(Sint32 x, Sint32 y)
{
	if(m_lines.empty())
		return 0;

	Sint32 count = m_scrollBar->getScrollPos();
	std::vector<ConsoleLine>::iterator it = m_lines.begin();
	std::advance(it, count);

	Sint32 messagesVisible = UTIL_min<Sint32>(m_maxDisplay, SDL_static_cast(Sint32, m_lines.size()));
	Sint32 posY = m_tRect.y1+m_tRect.y2-(messagesVisible*14);
	Sint32 endY = m_tRect.y1+m_tRect.y2-14;
	if(y < posY)
		return SDL_static_cast(Uint32, (*it).lineStart);

	for(std::vector<ConsoleLine>::iterator end = m_lines.end(); it != end; ++it)
	{
		ConsoleLine& currentLine = (*it);
		if(SDL_static_cast(Uint32, y-posY) < 14)
		{
			Sint32 lastPosition = 0;
			for(size_t i = 0; i <= currentLine.lineLength; ++i)
			{
				Uint32 loopTextWidth = g_engine.calculateFontWidth(m_font, m_sText, currentLine.lineStart, i);
				Sint32 position = m_tRect.x1+currentLine.startPosition+loopTextWidth;
				if(x-position < 0)
				{
					if(SDL_abs(x-position) < SDL_abs(x-lastPosition))
						return SDL_static_cast(Uint32, currentLine.lineStart+i);
					else
						return SDL_static_cast(Uint32, currentLine.lineStart+i-1);
				}
				lastPosition = position;
			}
			return SDL_static_cast(Uint32, currentLine.lineStart+currentLine.lineLength-1);
		}
		posY += 14;
		if(posY > endY)
			break;
	}
	return SDL_static_cast(Uint32, (*it).lineStart+(*it).lineLength-1);
}
