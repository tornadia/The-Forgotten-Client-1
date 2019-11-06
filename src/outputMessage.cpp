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

#include "outputMessage.h"

void OutputMessage::reset(Uint16 headerPos)
{
	m_messageSize = 0;
	m_writePos = headerPos;
}

void OutputMessage::addU8(Uint8 value)
{
	if(canWrite(1))
	{
		m_networkBuffer[m_writePos++] = value;
		++m_messageSize;
	}
}

void OutputMessage::addU16(Uint16 value)
{
	if(canWrite(2))
	{
		m_networkBuffer[m_writePos++] = SDL_static_cast(Uint8, value);
		m_networkBuffer[m_writePos++] = SDL_static_cast(Uint8, value >> 8);
		m_messageSize += 2;
	}
}

void OutputMessage::addU32(Uint32 value)
{
	if(canWrite(4))
	{
		m_networkBuffer[m_writePos++] = SDL_static_cast(Uint8, value);
		m_networkBuffer[m_writePos++] = SDL_static_cast(Uint8, value >> 8);
		m_networkBuffer[m_writePos++] = SDL_static_cast(Uint8, value >> 16);
		m_networkBuffer[m_writePos++] = SDL_static_cast(Uint8, value >> 24);
		m_messageSize += 4;
	}
}

void OutputMessage::addU64(Uint64 value)
{
	if(canWrite(8))
	{
		m_networkBuffer[m_writePos++] = SDL_static_cast(Uint8, value);
		m_networkBuffer[m_writePos++] = SDL_static_cast(Uint8, value >> 8);
		m_networkBuffer[m_writePos++] = SDL_static_cast(Uint8, value >> 16);
		m_networkBuffer[m_writePos++] = SDL_static_cast(Uint8, value >> 24);
		m_networkBuffer[m_writePos++] = SDL_static_cast(Uint8, value >> 32);
		m_networkBuffer[m_writePos++] = SDL_static_cast(Uint8, value >> 40);
		m_networkBuffer[m_writePos++] = SDL_static_cast(Uint8, value >> 48);
		m_networkBuffer[m_writePos++] = SDL_static_cast(Uint8, value >> 56);
		m_messageSize += 8;
	}
}

void OutputMessage::addString(const char* string)
{
	Uint16 len = SDL_static_cast(Uint16, SDL_strlen(string));
	if(canWrite(len + 2))
	{
		addU16(len);
		SDL_memcpy(m_networkBuffer + m_writePos, string, len);
		m_writePos += len;
		m_messageSize += len;
	}
}

void OutputMessage::addString(const std::string& string)
{
	Uint16 len = SDL_static_cast(Uint16, string.length());
	if(canWrite(len + 2))
	{
		addU16(len);
		SDL_memcpy(m_networkBuffer + m_writePos, string.c_str(), len);
		m_writePos += len;
		m_messageSize += len;
	}
}

void OutputMessage::addDouble(double value, Uint8 precision/* = 2*/)
{
	addU8(precision);
	addU32(SDL_static_cast(Uint32, value * SDL_pow(10.0, SDL_static_cast(double, precision))) + SDL_MAX_SINT32);
}

void OutputMessage::addPosition(const Position& position)
{
	addU16(position.x);
	addU16(position.y);
	addU8(position.z);
}

void OutputMessage::addPaddingBytes(Uint16 bytes, Uint8 byte)
{
	if(bytes == 0)
		return;

	if(canWrite(bytes))
	{
		SDL_memset(m_networkBuffer + m_writePos, byte, bytes);
		m_writePos += bytes;
		m_messageSize += bytes;
	}
}

bool OutputMessage::canWrite(Uint32 bytes)
{
	if(m_writePos + bytes >= OUTPUTMESSAGE_SIZE-8)
		return false;

	return true;
}

