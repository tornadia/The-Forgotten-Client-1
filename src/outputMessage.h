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

#ifndef __FILE_OUTPUT_MESSAGE_h_
#define __FILE_OUTPUT_MESSAGE_h_

#include "position.h"
#define OUTPUTMESSAGE_SIZE 65536

class Protocol;
class ProtocolLogin;
class ProtocolGame;
class Connection;
class OutputMessage
{
	public:
		OutputMessage(Uint16 headerPos) {reset(headerPos);}

		void reset(Uint16 headerPos);

		void addU8(Uint8 value);
		void addU16(Uint16 value);
		void addU32(Uint32 value);
		void addU64(Uint64 value);
		void addString(const char* string);
		void addString(const std::string& string);
		void addDouble(double value, Uint8 precision = 2);
		void addPosition(const Position& position);
		void addPaddingBytes(Uint16 bytes, Uint8 byte = 0);
		void addBool(bool value) {addU8(value ? 1 : 0);}

		Uint16 getWritePos() {return m_writePos;}
		Uint16 getMessageSize() {return m_messageSize;}

		void setWritePos(Uint16 writePos) {m_writePos = writePos;}
		void setMessageSize(Uint16 messageSize) {m_messageSize = messageSize;}

	protected:
		Uint8* getBuffer() {return m_networkBuffer;}
		Uint8* getWriteBuffer() {return m_networkBuffer + m_writePos;}

		friend class Protocol;
		friend class ProtocolLogin;
		friend class ProtocolGame;
		friend class Connection;

	private:
		bool canWrite(Uint32 bytes);

		Uint16 m_writePos;
		Uint16 m_messageSize;
		Uint8 m_networkBuffer[OUTPUTMESSAGE_SIZE];
};

#endif /* __FILE_OUTPUT_MESSAGE_h_ */
