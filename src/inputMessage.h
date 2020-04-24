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

#ifndef __FILE_INPUT_MESSAGE_h_
#define __FILE_INPUT_MESSAGE_h_

#include "position.h"
#define INPUTMESSAGE_SIZE 65536

class Protocol;
class ProtocolLogin;
class ProtocolGame;
class Connection;
class InputMessage
{
	public:
		InputMessage() {reset();}

		void reset() {m_readPos = 0; m_messageSize = 0;}
		void setReadPos(Uint16 readPos) {m_readPos = readPos;}
		void setMessageSize(Uint16 size) {m_messageSize = size;}

		Uint8 getU8();
		Uint16 getU16();
		Uint32 getU32();
		Uint64 getU64();
		const char* getRawString();
		std::string getString();
		double getDouble();
		Position getPosition();
		bool getBool() {return (getU8() != 0);}

		Uint8 peekU8() {Uint8 v = getU8();m_readPos--;return v;}
		Uint16 peekU16() {Uint16 v = getU16();m_readPos-=2;return v;}
		Uint32 peekU32() {Uint32 v = getU32();m_readPos-=4;return v;}
		Uint64 peekU64() {Uint64 v = getU64();m_readPos-=8;return v;}

		Uint16 getReadPos() {return m_readPos;}
		Uint16 getUnreadSize() {return m_messageSize - m_readPos;}
		Uint16 getMessageSize() {return m_messageSize;}
		bool eof() {return m_readPos >= m_messageSize;}

	protected:
		Uint8* getBuffer() {return m_networkBuffer;}
		Uint8* getReadBuffer() {return m_networkBuffer + m_readPos;}

		bool readAdler32();

		friend class Protocol;
		friend class ProtocolLogin;
		friend class ProtocolGame;
		friend class Connection;

	private:
		bool canRead(Uint32 bytes);

		Uint16 m_readPos;
		Uint16 m_messageSize;
		Uint8 m_networkBuffer[INPUTMESSAGE_SIZE];
};

#endif /* __FILE_INPUT_MESSAGE_h_ */
