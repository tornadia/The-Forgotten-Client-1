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

#include "protocol.h"
#include "rsa.h"

RSA g_rsa;

extern Connection* g_connection;

Protocol::Protocol()
{
	m_encryption = false;
	m_checksum = false;
}

Protocol::~Protocol()
{
}

bool Protocol::onRecv(InputMessage& msg)
{
	if(!g_connection) //WTF?
		return false;

	if(m_checksum && !msg.readChecksum())
		return false;

	if(m_encryption)
	{
		XTEA_decrypt(msg.getReadBuffer(), msg.getUnreadSize(), m_encryptionKeys);
		Uint16 messageSize = msg.getU16();
		if(messageSize > msg.getUnreadSize())
			return false;

		msg.setMessageSize(messageSize + msg.getReadPos());
	}

	while(!msg.eof())
		parseMessage(msg);

	return true;
}

bool Protocol::onSend(OutputMessage& msg)
{
	if(!g_connection) //WTF?
		return false;

	if(m_encryption)
	{
		Uint16 msgSize = msg.getMessageSize();
		Uint16 paddingBytes = ((msgSize+2) & 7);
		if(paddingBytes != 0)
			msg.addPaddingBytes(8-paddingBytes);

		msg.setWritePos(m_checksum ? 6 : 2);
		msg.addU16(msgSize);
		if(!XTEA_encrypt(msg.getWriteBuffer()-2, msgSize+10-paddingBytes, m_encryptionKeys))
			return false;
	}

	if(m_checksum)
	{
		Uint32 checksum = adler32Checksum(msg.getBuffer()+6, msg.getMessageSize());
		msg.setWritePos(2);
		msg.addU32(checksum);
	}

	msg.setWritePos(0);
	msg.addU16(msg.getMessageSize());
	g_connection->sendMessage(msg);
	return true;
}

Uint16 Protocol::getHeaderPos()
{
	return 2 + (m_encryption ? 2 : 0) + (m_checksum ? 4 : 0);
}

Uint16 Protocol::getOS()
{
	if(g_clientVersion <= 760)
		return PROTOCOL_OS_WINDOWS;

	#if defined(SDL_VIDEO_DRIVER_WINDOWS) || defined(SDL_VIDEO_DRIVER_WINRT)
	return PROTOCOL_OS_WINDOWS;
	#elif defined(SDL_VIDEO_DRIVER_EMSCRIPTEN)
	//Since it's website based report as flash because it's similar
	return PROTOCOL_OS_FLASH;
	#else
	//Everything other report as linux
	return PROTOCOL_OS_LINUX;
	#endif
}

Uint16 Protocol::getProtocolVersion()
{
	#if CLIENT_OVVERIDE_VERSION > 0
	return CLIENT_OVVERIDE_VERSION;
	#else
	switch(g_clientVersion)
	{
		case 980: return 971;
		case 981: return 973;
		case 982: return 974;
		case 983: return 975;
		case 984: return 976;
		case 985: return 977;
		case 986: return 978;
		case 1001: return 979;
		case 1002: return 980;
		case 1101: return 1100;
		case 1102: return 1101;
		case 1103: return 1101;
		case 1104: return 1101;
		case 1121: return 1120;
		case 1141: return 1140;
		case 1142: return 1140;
		case 1143: return 1141;
		case 1144: return 1141;
		case 1146: return 1145;
		case 1147: return 1145;
		case 1149: return 1148;
		case 1156: return 1150;
	}
	return SDL_static_cast(Uint16, g_clientVersion);
	#endif
}

Uint32 Protocol::getClientVersion()
{
	#if CLIENT_OVVERIDE_VERSION > 0
	return CLIENT_OVVERIDE_VERSION;
	#else
	return g_clientVersion;
	#endif
}
