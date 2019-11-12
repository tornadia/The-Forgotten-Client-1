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

#include "inputMessage.h"

Uint8 InputMessage::getU8()
{
	if(!canRead(1))
		return 0;

	return m_networkBuffer[m_readPos++];
}

Uint16 InputMessage::getU16()
{
	if(!canRead(2))
		return 0;

	Uint16 v = SDL_static_cast(Uint16, m_networkBuffer[m_readPos]) | SDL_static_cast(Uint16, m_networkBuffer[m_readPos + 1]) << 8;
	m_readPos += 2;
	return v;
}

Uint32 InputMessage::getU32()
{
	if(!canRead(4))
		return 0;

	Uint32 v = SDL_static_cast(Uint32, m_networkBuffer[m_readPos]) | (SDL_static_cast(Uint32, m_networkBuffer[m_readPos + 1]) << 8)
		| (SDL_static_cast(Uint32, m_networkBuffer[m_readPos + 2]) << 16) | (SDL_static_cast(Uint32, m_networkBuffer[m_readPos + 3]) << 24);
	m_readPos += 4;
	return v;
}

Uint64 InputMessage::getU64()
{
	if(!canRead(8))
		return 0;

	Uint64 v = SDL_static_cast(Uint64, m_networkBuffer[m_readPos]) | (SDL_static_cast(Uint64, m_networkBuffer[m_readPos + 1]) << 8)
		| (SDL_static_cast(Uint64, m_networkBuffer[m_readPos + 2]) << 16) | (SDL_static_cast(Uint64, m_networkBuffer[m_readPos + 3]) << 24)
		| (SDL_static_cast(Uint64, m_networkBuffer[m_readPos + 4]) << 32) | (SDL_static_cast(Uint64, m_networkBuffer[m_readPos + 5]) << 40)
		| (SDL_static_cast(Uint64, m_networkBuffer[m_readPos + 6]) << 48) | (SDL_static_cast(Uint64, m_networkBuffer[m_readPos + 7]) << 56);
	m_readPos += 8;
	return v;
}

const char* InputMessage::getRawString()
{
	static char EmptyString[] = "\0";
	Uint16 stringLength = getU16();
	if(!canRead(stringLength))
		return EmptyString;

	Uint16 readPos = m_readPos;
	m_readPos += stringLength;
	return SDL_reinterpret_cast(char*, m_networkBuffer + readPos);
}

std::string InputMessage::getString()
{
	Uint16 stringLength = getU16();
	if(!canRead(stringLength))
		return std::string();

	Uint16 readPos = m_readPos;
	m_readPos += stringLength;
	return std::string(SDL_reinterpret_cast(char*, m_networkBuffer + readPos), stringLength);
}

double InputMessage::getDouble()
{
	Uint8 precision = getU8();
	Sint32 v = getU32() - SDL_MAX_SINT32;
	return (v / SDL_pow(10.0, SDL_static_cast(double, precision)));
}

Position InputMessage::getPosition()
{
	Position pos;
	pos.x = getU16();
	pos.y = getU16();
	pos.z = getU8();
	return pos;
}

bool InputMessage::readChecksum()
{
	Uint32 receivedChecksum = getU32();
	Uint32 checksum = adler32Checksum(m_networkBuffer + m_readPos, getUnreadSize());
	return (receivedChecksum == checksum);
}

bool InputMessage::canRead(Uint32 bytes)
{
	if(m_readPos + bytes > m_messageSize)
		return false;

	return true;
}
