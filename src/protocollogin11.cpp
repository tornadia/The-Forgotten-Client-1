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

#include "protocollogin11.h"
#include "engine.h"
#include "http.h"

#include "GUI_Elements/GUI_Window.h"
#include "json/json.h"

extern Engine g_engine;
extern Http g_http;

ProtocolLogin11 g_protocolLogin11;

void handleRequest(Uint32, Sint32 internalId)
{
	if(g_protocolLogin11.getRequestId() == SDL_static_cast(Uint32, internalId))
	{
		HttpRequest* request = g_http.getRequest(SDL_static_cast(Uint32, internalId));
		if(request && request->result == HTTP_RESULT_SUCCEEDED)
			g_protocolLogin11.parseMessage();
		else
			UTIL_messageBox("Connection Failed", "Cannot connect to a login server.\n\nError: Connection failed.");
	}
}

ProtocolLogin11::ProtocolLogin11()
{
	m_requestId = 0;
}

void ProtocolLogin11::parseMessage()
{
	SDL_snprintf(g_buffer, sizeof(g_buffer), "%slogin.dat", g_prefPath.c_str());
	SDL_RWops* fp = SDL_RWFromFile(g_buffer, "rb");
	if(!fp)
	{
		UTIL_messageBox("Connection Failed", "Cannot connect to a login server.\n\nError: Failed to call recv().");
		return;
	}

	SDL_RWseek(fp, 0, RW_SEEK_END);
	size_t sizeData = SDL_static_cast(size_t, SDL_RWtell(fp));
	if(sizeData <= 0)
	{
		#ifdef SDL_FILESYSTEM_WINDOWS
		DeleteFileA(g_buffer);
		#elif HAVE_STDIO_H
		remove(g_buffer);
		#endif
		SDL_RWclose(fp);
		UTIL_messageBox("Connection Failed", "Cannot connect to a login server.\n\nError: Protocol mismatched.");
		return;
	}

	char* msgData = SDL_reinterpret_cast(char*, SDL_malloc(sizeData));
	if(!msgData)
	{
		#ifdef SDL_FILESYSTEM_WINDOWS
		DeleteFileA(g_buffer);
		#elif HAVE_STDIO_H
		remove(g_buffer);
		#endif
		SDL_RWclose(fp);
		UTIL_messageBox("Connection Failed", "Cannot connect to a login server.\n\nError: Protocol mismatched.");
		return;
	}

	SDL_RWseek(fp, 0, RW_SEEK_SET);
	SDL_RWread(fp, msgData, 1, sizeData);
	SDL_RWclose(fp);
	#ifdef SDL_FILESYSTEM_WINDOWS
	DeleteFileA(g_buffer);
	#elif HAVE_STDIO_H
	remove(g_buffer);
	#endif

	JSON_VALUE* msgJson = JSON_VALUE::decode(msgData);
	SDL_free(msgData);
	if(msgJson && msgJson->IsObject())
	{
		std::map<Uint8, WorldDetail> worlds;
		std::vector<CharacterDetail> characters;

		bool accountShowOutfits = false;
		Uint8 accountStatus = AccountStatus_Ok;
		Uint8 accountSubStatus = SubscriptionStatus_Free;
		Uint32 accountPremDays = 0;

		JSON_VALUE* sessionJson = msgJson->find("session");
		if(sessionJson && sessionJson->IsObject())
		{
			JSON_VALUE* sessionKey = sessionJson->find("sessionkey");
			if(sessionKey && sessionKey->IsString())
				g_engine.setAccountSessionKey(sessionKey->AsString());

			JSON_VALUE* isPremium = sessionJson->find("ispremium");
			if(isPremium && isPremium->IsBool())
				accountSubStatus = (isPremium->AsBool() ? SubscriptionStatus_Premium : SubscriptionStatus_Free);

			JSON_VALUE* premiumTime = sessionJson->find("premiumuntil");
			if(premiumTime && premiumTime->IsNumber())
			{
				time_t premiumUntil = SDL_static_cast(time_t, premiumTime->AsNumber());
				accountPremDays = UTIL_max<Uint32>(SDL_static_cast(Uint32, SDL_floor((premiumUntil - time(NULL)) / 86400.0)), 1);
			}

			JSON_VALUE* status = sessionJson->find("status");
			if(status && status->IsString())
			{
				std::string statusStr = status->AsString();
				if(statusStr == "suspended")
					accountStatus = AccountStatus_Suspended;
				else if(statusStr == "frozen")
					accountStatus = AccountStatus_Frozen;
				else
					accountStatus = AccountStatus_Ok;
			}
		}

		JSON_VALUE* playDataJson = msgJson->find("playdata");
		if(playDataJson && playDataJson->IsObject())
		{
			JSON_VALUE* worldsJson = playDataJson->find("worlds");
			if(worldsJson && worldsJson->IsArray())
			{
				size_t worldsSize = worldsJson->size();
				for(size_t i = 0; i < worldsSize; ++i)
				{
					JSON_VALUE* worldJson = worldsJson->find(i);
					if(worldJson && worldJson->IsObject())
					{
						WorldDetail world;
						Uint8 worldId = 0xFF;
						JSON_VALUE* idJson = worldJson->find("id");
						if(idJson && idJson->IsNumber())
							worldId = SDL_static_cast(Uint8, idJson->AsNumber());

						JSON_VALUE* nameJson = worldJson->find("name");
						if(nameJson && nameJson->IsString())
							world.worldName = nameJson->AsString();

						JSON_VALUE* previewstateJson = worldJson->find("previewstate");
						if(previewstateJson)
						{
							if(previewstateJson->IsBool())
								world.previewState = previewstateJson->AsBool();
							else if(previewstateJson->IsNumber())
								world.previewState = (SDL_static_cast(Uint8, previewstateJson->AsNumber()) == 1);
						}

						JSON_VALUE* ipJson = worldJson->find("externaladdressprotected");
						if(!ipJson)
						{
							ipJson = worldJson->find("externaladdressunprotected");
							if(!ipJson)
								ipJson = worldJson->find("externaladdress");
						}
						if(ipJson && ipJson->IsString())
							world.worldIp = ipJson->AsString();

						JSON_VALUE* portJson = worldJson->find("externalportprotected");
						if(!portJson)
						{
							portJson = worldJson->find("externalportunprotected");
							if(!portJson)
								portJson = worldJson->find("externalport");
						}
						if(portJson)
						{
							if(portJson->IsNumber())
								world.worldPort = SDL_static_cast(Uint16, SDL_strtoul(portJson->AsString().c_str(), NULL, 10));
							else if(portJson->IsString())
								world.worldPort = SDL_static_cast(Uint16, portJson->AsNumber());
						}
						worlds[worldId] = world;
					}
				}
			}

			JSON_VALUE* charactersJson = playDataJson->find("characters");
			if(charactersJson && charactersJson->IsArray())
			{
				size_t charactersSize = charactersJson->size();
				characters.resize(charactersSize);
				for(size_t i = 0; i < charactersSize; ++i)
				{
					JSON_VALUE* characterJson = charactersJson->find(i);
					if(characterJson && characterJson->IsObject())
					{
						JSON_VALUE* worldidJson = characterJson->find("worldid");
						if(worldidJson && worldidJson->IsNumber())
						{
							std::map<Uint8, WorldDetail>::iterator it = worlds.find(SDL_static_cast(Uint8, worldidJson->AsNumber()));
							if(it != worlds.end())
							{
								CharacterDetail& character = characters[i];
								character.worldName.assign(it->second.worldName);
								character.worldIp.assign(it->second.worldIp);
								character.worldPort = it->second.worldPort;
								character.previewState = it->second.previewState;
								character.lookType = 0;

								JSON_VALUE* nameJson = characterJson->find("name");
								if(nameJson && nameJson->IsString())
									character.name = nameJson->AsString();

								JSON_VALUE* levelJson = characterJson->find("level");
								if(levelJson && levelJson->IsNumber())
									character.level = SDL_static_cast(Uint16, levelJson->AsNumber());

								JSON_VALUE* looktypeJson = characterJson->find("outfitid");
								if(looktypeJson && looktypeJson->IsNumber())
								{
									character.lookType = SDL_static_cast(Uint16, looktypeJson->AsNumber());
									accountShowOutfits = true;
								}

								JSON_VALUE* lookheadJson = characterJson->find("headcolor");
								if(lookheadJson && lookheadJson->IsNumber())
									character.lookHead = SDL_static_cast(Uint8, lookheadJson->AsNumber());

								JSON_VALUE* lookbodyJson = characterJson->find("torsocolor");
								if(lookbodyJson && lookbodyJson->IsNumber())
									character.lookBody = SDL_static_cast(Uint8, lookbodyJson->AsNumber());

								JSON_VALUE* looklegsJson = characterJson->find("legscolor");
								if(looklegsJson && looklegsJson->IsNumber())
									character.lookLegs = SDL_static_cast(Uint8, looklegsJson->AsNumber());

								JSON_VALUE* lookfeetJson = characterJson->find("detailcolor");
								if(lookfeetJson && lookfeetJson->IsNumber())
									character.lookFeet = SDL_static_cast(Uint8, lookfeetJson->AsNumber());

								JSON_VALUE* lookaddonsJson = characterJson->find("addonsflags");
								if(lookaddonsJson && lookaddonsJson->IsNumber())
									character.lookAddons = SDL_static_cast(Uint8, lookaddonsJson->AsNumber());

								JSON_VALUE* healthJson = characterJson->find("health");
								if(healthJson && healthJson->IsNumber())
									character.health = SDL_static_cast(Uint32, healthJson->AsNumber());

								JSON_VALUE* healthmaxJson = characterJson->find("healthmax");
								if(healthmaxJson && healthmaxJson->IsNumber())
									character.healthMax = SDL_static_cast(Uint32, healthmaxJson->AsNumber());

								JSON_VALUE* manaJson = characterJson->find("mana");
								if(manaJson && manaJson->IsNumber())
									character.mana = SDL_static_cast(Uint32, manaJson->AsNumber());

								JSON_VALUE* manamaxJson = characterJson->find("manamax");
								if(manamaxJson && manamaxJson->IsNumber())
									character.manaMax = SDL_static_cast(Uint32, manamaxJson->AsNumber());

								JSON_VALUE* vocnameJson = characterJson->find("vocation");
								if(vocnameJson && vocnameJson->IsString())
									character.vocName = vocnameJson->AsString();
							}
						}
					}
				}
			}
		}

		GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_MESSAGEBOX);
		if(pWindow)
			g_engine.removeWindow(pWindow);

		g_engine.setAccountCharList(characters);
		g_engine.setAccountStatus(accountStatus);
		g_engine.setAccountSubstatus(accountSubStatus);
		g_engine.setAccountPremDays(accountPremDays);
		g_engine.setAccountNewCharList(accountShowOutfits);
		g_engine.setCharacterSelectId(0);
		UTIL_createCharacterList();
	}
	else
		UTIL_messageBox("Connection Failed", "Cannot connect to a login server.\n\nError: Protocol mismatched.");

	if(msgJson)
		delete msgJson;
}

void ProtocolLogin11::initializeConnection()
{
	std::string& accountToken = g_engine.getAccountToken();
	JSON_OBJECT jsonObject;
	jsonObject["email"] = new JSON_VALUE(g_engine.getAccountName());
	jsonObject["accountname"] = new JSON_VALUE(g_engine.getAccountName());
	jsonObject["password"] = new JSON_VALUE(g_engine.getAccountPassword());
	jsonObject["stayloggedin"] = new JSON_VALUE(true);
	if(!accountToken.empty())
		jsonObject["token"] = new JSON_VALUE(accountToken);
	jsonObject["type"] = new JSON_VALUE("login");

	SDL_snprintf(g_buffer, sizeof(g_buffer), "%slogin.dat", g_prefPath.c_str());
	JSON_VALUE* json = new JSON_VALUE(jsonObject);
	#if CLIENT_OVVERIDE_VERSION == 0
	m_requestId = g_http.addRequest(g_engine.getClientHost(), g_buffer, JSON_VALUE::encode(json), &handleRequest, 0);
	#else
	m_requestId = g_http.addRequest(CLIENT_OVVERIDE_LOGIN_HOST, g_buffer, JSON_VALUE::encode(json), &handleRequest, 0);
	#endif
	delete json;
}

void ProtocolLogin11::closeConnection()
{
	if(m_requestId != 0)
	{
		g_http.removeRequest(m_requestId);
		m_requestId = 0;

		SDL_snprintf(g_buffer, sizeof(g_buffer), "%slogin.dat", g_prefPath.c_str());
		#ifdef SDL_FILESYSTEM_WINDOWS
		DeleteFileA(g_buffer);
		#elif HAVE_STDIO_H
		remove(g_buffer);
		#endif
	}
}
