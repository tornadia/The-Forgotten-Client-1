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

#ifndef __FILE_STATICTEXT_h_
#define __FILE_STATICTEXT_h_

#include "position.h"

struct StaticTextMessage
{
	std::string m_message;
	Uint32 m_endTime;
};

struct StaticTextMessageLine
{
	size_t lineStart;
	size_t lineLength;
	Uint32 lineWidth;
};

class StaticText
{
	public:
		StaticText(const Position& pos);
		
		SDL_FORCE_INLINE std::string& getName() {return m_name;}
		SDL_FORCE_INLINE MessageMode getMode() {return m_mode;}
		
		SDL_FORCE_INLINE const Position& getPosition() {return m_position;}
		SDL_FORCE_INLINE bool canBeDeleted() {return m_isCompleted;}
		void addMessage(const std::string& name, MessageMode mode, std::string text);

		void composeText();
		void update();
		void render(Sint32 posX, Sint32 posY, Sint32 boundLeft, Sint32 boundTop, Sint32 boundRight, Sint32 boundBottom);

	protected:
		std::vector<StaticTextMessage> m_messages;
		std::vector<StaticTextMessageLine> m_messagesLine;

		std::string m_text;
		std::string m_name;

		Uint32 m_textWidth;
		Position m_position;

		MessageMode m_mode;
		Uint8 m_red;
		Uint8 m_green;
		Uint8 m_blue;
		bool m_isCompleted;
};

#endif /* __FILE_STATICTEXT_h_ */
