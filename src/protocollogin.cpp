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

#include "protocollogin.h"
#include "engine.h"
#include "rsa.h"
#include "game.h"

#include "GUI_Elements/GUI_Window.h"

extern RSA g_rsa;
extern Engine g_engine;
extern Game g_game;
extern Uint32 g_datRevision;
extern Uint32 g_picRevision;
extern Uint32 g_sprRevision;

ProtocolLogin::ProtocolLogin()
{
	m_lastError = CONNECTION_ERROR_INVALID;
	m_skipErrors = false;
}

void ProtocolLogin::parseMessage(InputMessage& msg)
{
	m_skipErrors = true;//Since we read something skip disconnect errors

	Uint8 header = msg.getU8();
	switch(header)
	{
		case RecvErrorOpcode:
		case RecvErrorNewOpcode: parseError(msg); break;
		case RecvTokenSuccessOpcode: parseTokenSuccess(msg); break;
		case RecvTokenFailedOpcode: parseTokenFailed(msg); break;
		case RecvMotdOpcode: parseMotd(msg); break;
		case RecvUpdate1Opcode:
		case RecvUpdate2Opcode:
		case RecvUpdate3Opcode: parseUpdate(msg); break;
		case RecvSessionKeyOpcode: parseSessionKey(msg); break;
		case RecvCharacterListOpcode: parseCharacterList(msg); break;
		case RecvCharacterListNewOpcode: parseCharacterListNew(msg); break;
		default:
		{
			msg.setReadPos(msg.getMessageSize());//Make sure we don't read anymore
			SDL_snprintf(g_buffer, sizeof(g_buffer), "Received unsupported packet header: 0x%02X", SDL_static_cast(Uint32, header));
			UTIL_messageBox("Error", std::string(g_buffer));
		}
		break;
	}
}

void ProtocolLogin::parseError(InputMessage& msg)
{
	const std::string& message = msg.getString();
	UTIL_messageBox("Sorry", message);
}

void ProtocolLogin::parseTokenSuccess(InputMessage& msg)
{
	msg.getU8(); //Unknown
}

void ProtocolLogin::parseTokenFailed(InputMessage& msg)
{
	msg.getU8(); //Unknown
	UTIL_messageBox("Two-Factor Authentification", "Invalid authentification token.\nPlease enter a new, valid token:");
}

void ProtocolLogin::parseMotd(InputMessage& msg)
{
	const std::string& motdmsg = msg.getString();
	StringVector motd = UTIL_explodeString(motdmsg, "\n", 2);
	if(motd.size() == 2)
	{
		Uint32 motdNumber = SDL_static_cast(Uint32, SDL_strtoul(motd[0].c_str(), NULL, 10));
		if(g_engine.getMotdNumber() != motdNumber)
		{
			GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_MESSAGEBOX);
			if(pWindow)
				g_engine.removeWindow(pWindow);

			g_engine.setMotdNumber(motdNumber);
			g_engine.setMotdText(motd[1]);
			UTIL_createMotd();
		}
	}
}

void ProtocolLogin::parseUpdate(InputMessage&)
{
	//Updating(OTS don't support this so we don't need to bother)
	UTIL_messageBox("Error", "Client needs update.");
}

void ProtocolLogin::parseSessionKey(InputMessage& msg)
{
	const std::string& sessionKey = msg.getString();
	g_engine.setAccountSessionKey(sessionKey);
}

void ProtocolLogin::parseCharacterList(InputMessage& msg)
{
	std::vector<CharacterDetail> characters;
	if(g_clientVersion > 1010)
	{
		std::map<Uint8, WorldDetail> worlds;

		Uint8 worldCount = msg.getU8();
		for(Uint8 i = 0; i < worldCount; ++i)
		{
			WorldDetail world;
			Uint8 worldId = msg.getU8();
			world.worldName = msg.getString();
			world.worldIp = msg.getString();
			world.worldPort = msg.getU16();
			world.previewState = (msg.getU8() == 1);
			worlds[worldId] = world;
		}

		Uint8 characterCount = msg.getU8();
		characters.resize(SDL_static_cast(size_t, characterCount));
		for(Uint8 i = 0; i < characterCount; ++i)
		{
			Uint8 worldId = msg.getU8();
			std::map<Uint8, WorldDetail>::iterator it = worlds.find(worldId);
			if(it != worlds.end())
			{
				CharacterDetail& character = characters[i];
				character.name = msg.getString();
				character.worldName.assign(it->second.worldName);
				character.worldIp.assign(it->second.worldIp);
				character.worldPort = it->second.worldPort;
				character.previewState = it->second.previewState;
				character.lookType = 0;
			}
			else
				msg.getRawString();
		}
	}
	else
	{
		Uint8 characterCount = msg.getU8();
		characters.resize(SDL_static_cast(size_t, characterCount));
		for(Uint8 i = 0; i < characterCount; ++i)
		{
			CharacterDetail& character = characters[i];
			character.name = msg.getString();
			character.worldName = msg.getString();
			character.worldIp = UTIL_ipv4_tostring(msg.getU32());
			character.worldPort = msg.getU16();
			if(g_game.hasGameFeature(GAME_FEATURE_PREVIEW_STATE))
				character.previewState = (msg.getU8() == 1);
			else
				character.previewState = false;
			
			character.lookType = 0;
		}
	}

	Uint8 accountStatus;
	Uint8 accountSubStatus;
	Uint32 accountPremDays;
	if(g_clientVersion > 1077)
	{
		accountStatus = msg.getU8();
		accountSubStatus = msg.getU8();
		accountPremDays = msg.getU32();
		if(accountPremDays != 0)
			accountPremDays = UTIL_max<Uint32>(SDL_static_cast(Uint32, SDL_floor((accountPremDays - time(NULL)) / 86400.0)), 1);
	}
	else
	{
		accountStatus = AccountStatus_Ok;
		accountPremDays = SDL_static_cast(Uint32, msg.getU16());
		accountSubStatus = (accountPremDays > 0 ? SubscriptionStatus_Premium : SubscriptionStatus_Free);
		if(accountPremDays == 0xFFFF)
			accountPremDays = 0;
	}

	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_MESSAGEBOX);
	if(pWindow)
		g_engine.removeWindow(pWindow);

	g_engine.setAccountCharList(characters);
	g_engine.setAccountStatus(accountStatus);
	g_engine.setAccountSubstatus(accountSubStatus);
	g_engine.setAccountPremDays(accountPremDays);
	g_engine.setAccountNewCharList(false);
	g_engine.setCharacterSelectId(0);
	UTIL_createCharacterList();
}

void ProtocolLogin::parseCharacterListNew(InputMessage& msg)
{
	std::vector<CharacterDetail> characters;
	Uint8 characterCount = msg.getU8();
	characters.resize(SDL_static_cast(size_t, characterCount));
	for(Uint8 i = 0; i < characterCount; ++i)
	{
		CharacterDetail& character = characters[i];
		character.level = msg.getU16();
		character.lookType = msg.getU16();
		character.lookHead = msg.getU8();
		character.lookBody = msg.getU8();
		character.lookLegs = msg.getU8();
		character.lookFeet = msg.getU8();
		character.lookAddons = msg.getU8();
		character.health = msg.getU32();
		character.healthMax = msg.getU32();
		character.mana = msg.getU32();
		character.manaMax = msg.getU32();
		character.vocName = msg.getString();
		character.name = msg.getString();
		character.worldName = msg.getString();
		character.worldIp = UTIL_ipv4_tostring(msg.getU32());
		character.worldPort = msg.getU16();
		character.previewState = false;
	}

	Uint8 accountStatus = AccountStatus_Ok;
	Uint32 accountPremDays = SDL_static_cast(Uint32, msg.getU16());
	Uint8 accountSubStatus = (accountPremDays > 0 ? SubscriptionStatus_Premium : SubscriptionStatus_Free);
	if(accountPremDays == 0xFFFF)
		accountPremDays = 0;

	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_MESSAGEBOX);
	if(pWindow)
		g_engine.removeWindow(pWindow);

	g_engine.setAccountCharList(characters);
	g_engine.setAccountStatus(accountStatus);
	g_engine.setAccountSubstatus(accountSubStatus);
	g_engine.setAccountPremDays(accountPremDays);
	g_engine.setAccountNewCharList(true);
	g_engine.setCharacterSelectId(0);
	UTIL_createCharacterList();
}

void ProtocolLogin::onConnect()
{
	bool checksumFeature = (g_game.hasGameFeature(GAME_FEATURE_CHECKSUM) || g_game.hasGameFeature(GAME_FEATURE_PROTOCOLSEQUENCE));
	bool xteaFeature = g_game.hasGameFeature(GAME_FEATURE_XTEA);
	bool rsa1024Feature = g_game.hasGameFeature(GAME_FEATURE_RSA1024);

	OutputMessage msg((checksumFeature ? 6 : 2));
	msg.addU8(SendLoginOpcode);
	msg.addU16(Protocol::getOS());
	msg.addU16(Protocol::getProtocolVersion());
	if(g_game.hasGameFeature(GAME_FEATURE_CLIENT_VERSION))
		msg.addU32(Protocol::getClientVersion());

	msg.addU32(g_datRevision);
	msg.addU32(g_sprRevision);
	msg.addU32(g_picRevision);
	if(g_game.hasGameFeature(GAME_FEATURE_PREVIEW_STATE))
		msg.addU8(0);

	Uint16 firstByte = msg.getWritePos();
	if(rsa1024Feature)
		msg.addU8(0);//First byte with RSA have to be 0

	Uint32 keys[4];
	if(xteaFeature)
	{
		keys[0] = (UTIL_random(0, 0xFF) << 24) | (UTIL_random(0, 0xFF) << 16) | (UTIL_random(0, 0xFF) << 8) | UTIL_random(0, 0xFF);
		keys[1] = (UTIL_random(0, 0xFF) << 24) | (UTIL_random(0, 0xFF) << 16) | (UTIL_random(0, 0xFF) << 8) | UTIL_random(0, 0xFF);
		keys[2] = (UTIL_random(0, 0xFF) << 24) | (UTIL_random(0, 0xFF) << 16) | (UTIL_random(0, 0xFF) << 8) | UTIL_random(0, 0xFF);
		keys[3] = (UTIL_random(0, 0xFF) << 24) | (UTIL_random(0, 0xFF) << 16) | (UTIL_random(0, 0xFF) << 8) | UTIL_random(0, 0xFF);
		msg.addU32(keys[0]);
		msg.addU32(keys[1]);
		msg.addU32(keys[2]);
		msg.addU32(keys[3]);
	}

	if(g_game.hasGameFeature(GAME_FEATURE_ACCOUNT_NAME))
		msg.addString(g_engine.getAccountName());
	else
		msg.addU32(SDL_static_cast(Uint32, SDL_strtoul(g_engine.getAccountName().c_str(), NULL, 10)));

	msg.addString(g_engine.getAccountPassword());
	if(rsa1024Feature)
	{
		msg.addPaddingBytes(128-(msg.getWritePos()-firstByte));
		g_rsa.encrypt(msg.getBuffer()+firstByte);
	}

	if(g_game.hasGameFeature(GAME_FEATURE_RENDER_INFORMATION))
	{
		//Does any OTS even fetch these values?
		msg.addU8(1);
		msg.addU8(1);
		msg.addString(g_engine.getRender()->getHardware());
		msg.addString(g_engine.getRender()->getSoftware());
	}

	if(g_game.hasGameFeature(GAME_FEATURE_AUTHENTICATOR))
	{
		firstByte = msg.getWritePos();
		msg.addU8(0);//First byte with RSA have to be 0
		msg.addString(g_engine.getAccountToken());//Token
		if(g_game.hasGameFeature(GAME_FEATURE_SESSIONKEY))
			msg.addU8(1);//Stay logged

		msg.addPaddingBytes(128-(msg.getWritePos()-firstByte));
		g_rsa.encrypt(msg.getBuffer()+firstByte);
	}

	if(checksumFeature)
		setChecksumMethod(g_game.hasGameFeature(GAME_FEATURE_PROTOCOLSEQUENCE) ? CHECKSUM_METHOD_SEQUENCE : CHECKSUM_METHOD_ADLER32);

	onSend(msg);
	if(xteaFeature)
	{
		setEncryption(true);
		setEncryptionKeys(keys);
	}
}

void ProtocolLogin::onConnectionError(ConnectionError error)
{
	if(!m_skipErrors)
		m_lastError = error;
}

void ProtocolLogin::onDisconnect()
{
	if(m_lastError != CONNECTION_ERROR_INVALID)
	{
		switch(m_lastError)
		{
			case CONNECTION_ERROR_RESOLVE_HOST:
				UTIL_messageBox("Connection Failed", "Cannot connect to a login server.\n\nError: Cannot resolve host.");
				break;
			case CONNECTION_ERROR_CREATE_SOCKET:
				UTIL_messageBox("Connection Failed", "Cannot connect to a login server.\n\nError: Could not create socket.");
				break;
			case CONNECTION_ERROR_SET_NONBLOCKING_SOCKET:
				UTIL_messageBox("Connection Failed", "Cannot connect to a login server.\n\nError: Could not create a non-blocking socket.");
				break;
			case CONNECTION_ERROR_CANNOT_CONNECT:
				UTIL_messageBox("Connection Failed", "Cannot connect to a login server.\n\nError: Cannot connect.");
				break;
			case CONNECTION_ERROR_FAIL_CONNECT:
				UTIL_messageBox("Connection Failed", "Cannot connect to a login server.\n\nError: Connection failed.");
				break;
			case CONNECTION_ERROR_REFUSED_CONNECT:
				UTIL_messageBox("Connection Failed", "Cannot connect to a login server.\n\nError: Connection refused.");
				break;
			case CONNECTION_ERROR_TIMEOUT:
				UTIL_messageBox("Connection Failed", "Cannot connect to a login server.\n\nError: Connection timed out.");
				break;
			case CONNECTION_ERROR_RECV_FAIL:
				UTIL_messageBox("Connection Failed", "Cannot connect to a login server.\n\nError: Failed to call recv().");
				break;
			case CONNECTION_ERROR_SEND_FAIL:
				UTIL_messageBox("Connection Failed", "Cannot connect to a login server.\n\nError: Failed to call send().");
				break;
			case CONNECTION_ERROR_PROTOCOL_FAIL:
				UTIL_messageBox("Connection Failed", "Cannot connect to a login server.\n\nError: Protocol mismatched.");
				break;
			default:
				break;
		}
	}
	delete this;
}
