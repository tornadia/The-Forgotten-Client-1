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

#include "protocolgame.h"
#include "engine.h"
#include "thingManager.h"
#include "rsa.h"
#include "thing.h"
#include "creature.h"
#include "item.h"
#include "map.h"
#include "tile.h"
#include "light.h"
#include "distanceEffect.h"
#include "effect.h"
#include "automap.h"
#include "game.h"

#include "GUI_Elements/GUI_Log.h"
#include "GUI/itemUI.h"

extern RSA g_rsa;
extern Map g_map;
extern LightSystem g_light;
extern Engine g_engine;
extern ThingManager g_thingManager;
extern Automap g_automap;
extern Game g_game;
extern GUI_Log g_logger;
extern Connection* g_connection;
extern Uint32 g_datRevision;
extern Uint16 g_ping;

ProtocolGame::ProtocolGame()
{
	m_expectChannels = 0;
	m_pingReceived = 0;
	m_pingSent = 0;
	m_pingTime = 0;

	m_lastError = CONNECTION_ERROR_INVALID;
	m_skipErrors = false;
	m_gameInitialized = false;
	m_pendingGame = false;
}

void ProtocolGame::parseMessage(InputMessage& msg)
{
	Uint8 header = msg.getU8();
	switch(header)
	{
		case RecvCreatureDataOpcode: parseCreatureData(msg); break;
		case RecvLoginOrPendingOpcode: parseLoginOrPending(msg); break;
		case RecvGMActionsOpcode: (g_clientVersion < 1100 ? parseGMActions(msg) : parseReadyForSecondaryConnection(msg)); break;
		case RecvEnterGameOpcode: parseEnterGame(msg); break;
		case RecvUpdateNeededOpcode: parseUpdateNeeded(msg); break;
		case RecvLoginErrorOpcode: parseLoginError(msg); break;
		case RecvLoginAdviceOpcode: parseLoginAdvice(msg); break;
		case RecvLoginWaitOpcode: parseLoginWait(msg); break;
		case RecvLoginSuccessOpcode: parseLogin(msg); break;
		case RecvLoginTokenOpcode: parseLoginToken(msg); break;
		case RecvStoreButtonIndicatorsOpcode: parseStoreButtonIndicators(msg); break;
		case RecvPingBackOpcode: parsePingBack(msg); break;
		case RecvPingOpcode: parsePing(msg); break;
		case RecvChallengeOpcode: parseChallenge(msg); return;
		case RecvDeathOpcode: parseDeath(msg); break;
		case RecvStashOpcode: parseStash(msg); break;
		case RecvSpecialContainersAvailableOpcode: parseSpecialContainersAvailable(msg); break;
		case RecvClientCheckOpcode: parseClientCheck(msg); break;
		case RecvMapDescriptionOpcode: parseMapDescription(msg); break;
		case RecvMapNorthOpcode: parseMapNorth(msg); break;
		case RecvMapEastOpcode: parseMapEast(msg); break;
		case RecvMapSouthOpcode: parseMapSouth(msg); break;
		case RecvMapWestOpcode: parseMapWest(msg); break;
		case RecvUpdateTileOpcode: parseUpdateTile(msg); break;
		case RecvTileAddThingOpcode: parseTileAddThing(msg); break;
		case RecvTileTransformThingOpcode: parseTileTransformThing(msg); break;
		case RecvTileRemoveThingOpcode: parseTileRemoveThing(msg); break;
		case RecvTileMoveCreatureOpcode: parseTileMoveCreature(msg); break;
		case RecvContainerOpenOpcode: parseContainerOpen(msg); break;
		case RecvContainerCloseOpcode: parseContainerClose(msg); break;
		case RecvContainerAddItemOpcode: parseContainerAddItem(msg); break;
		case RecvContainerTransformItemOpcode: parseContainerTransformItem(msg); break;
		case RecvContainerRemoveItemOpcode: parseContainerRemoveItem(msg); break;
		case RecvFriendSystemDataOpcode: parseFriendSystemData(msg); break;
		case RecvScreenshotEventOpcode: parseScreenshotEvent(msg); break;
		case RecvInspectionListOpcode: parseInspectionList(msg); break;
		case RecvInspectionStateOpcode: parseInspectionState(msg); break;
		case RecvInventoryTransformItemOpcode: parseInventoryTransformItem(msg); break;
		case RecvInventoryRemoveItemOpcode: parseInventoryRemoveItem(msg); break;
		case RecvNPCOpenTradeOpcode: parseNpcOpenTrade(msg); break;
		case RecvNPCPlayerGoodsOpcode: parseNpcPlayerGoods(msg); break;
		case RecvNPCCloseTradeOpcode: parseNpcCloseTrade(msg); break;
		case RecvTradeOwnOpcode: parseTradeOwn(msg); break;
		case RecvTradeCounterOpcode: parseTradeCounter(msg); break;
		case RecvTradeCloseOpcode: parseTradeClose(msg); break;
		case RecvAmbientWorldLightOpcode: parseAmbientWorldLight(msg); break;
		case RecvMagicEffectOpcode: (g_clientVersion >= 1203 ? parseMagicEffects(msg) : parseMagicEffect(msg)); break;
		case RecvAnimatedTextOpcode: (g_clientVersion >= 1200 ? parseRemoveMagicEffect(msg) : parseAnimatedText(msg)); break;
		case RecvDistanceEffectOpcode: parseDistanceEffect(msg); break;
		case RecvCreatureMarkOpcode: parseCreatureMark(msg); break;
		case RecvCreatureTrappersOpcode: parseCreatureTrappers(msg); break;
		case RecvCreatureUpdateOpcode: parseCreatureUpdate(msg); break;
		case RecvCreatureHealthOpcode: parseCreatureHealth(msg); break;
		case RecvCreatureLightOpcode: parseCreatureLight(msg); break;
		case RecvCreatureOutfitOpcode: parseCreatureOutfit(msg); break;
		case RecvCreatureSpeedOpcode: parseCreatureSpeed(msg); break;
		case RecvCreatureSkullOpcode: parseCreatureSkull(msg); break;
		case RecvCreaturePartyOpcode: parseCreatureParty(msg); break;
		case RecvCreatureUnpassableOpcode: parseCreatureUnpassable(msg); break;
		case RecvCreatureMarksOpcode: parseCreatureMarks(msg); break;
		case RecvCreatureHelpersOpcode: (g_clientVersion >= 1220 ? parseDepotSearchResult(msg) : parseCreatureHelpers(msg)); break;
		case RecvCreatureTypeOpcode: parseCreatureType(msg); break;
		case RecvEditTextWindowOpcode: parseEditTextWindow(msg); break;
		case RecvEditHouseWindowOpcode: parseEditHouseWindow(msg); break;
		case RecvGameNewsOpcode: parseGameNews(msg); break;
		case RecvDepotSearchDetailListOpcode: parseDepotSearchDetailList(msg); break;
		case RecvCloseDepotSearchOpcode: parseCloseDepotSearch(msg); break;
		case RecvBlessingsDialogOpcode: parseBlessingsDialog(msg); break;
		case RecvBlessingsOpcode: parseBlessings(msg); break;
		case RecvPresetOpcode: parsePreset(msg); break;
		case RecvPremiumTriggerOpcode: parsePremiumTrigger(msg); break;
		case RecvPlayerDataBasicOpcode: parsePlayerDataBasic(msg); break;
		case RecvPlayerDataOpcode: parsePlayerData(msg); break;
		case RecvPlayerSkillsOpcode: parsePlayerSkills(msg); break;
		case RecvPlayerIconsOpcode: parsePlayerIcons(msg); break;
		case RecvPlayerCancelTargetOpcode: parsePlayerCancelTarget(msg); break;
		case RecvPlayerSpellDelayOpcode: parsePlayerSpellDelay(msg); break;
		case RecvPlayerSpellGroupDelayOpcode: parsePlayerSpellGroupDelay(msg); break;
		case RecvPlayerMultiUseDelayOpcode: parsePlayerMultiUseDelay(msg); break;
		case RecvPlayerModesOpcode: parsePlayerModes(msg); break;
		case RecvPlayerStoreDeepLinkOpcode: parsePlayerStoreDeepLink(msg); break;
		case RecvRestingAreaStateOpcode: parseRestingAreaState(msg); break;
		case RecvPlayerTalkOpcode: parsePlayerTalk(msg); break;
		case RecvChannelsOpcode: parseChannels(msg); break;
		case RecvOpenChannelOpcode: parseOpenChannel(msg); break;
		case RecvOpenPrivateChannelOpcode: parseOpenPrivateChannel(msg); break;
		case RecvRuleViolationChannelOpcode: (g_clientVersion >= 1070 ? parseEditGuildMessage(msg) : parseRuleViolationChannel(msg)); break;
		case RecvRuleViolationRemoveOpcode: parseRuleViolationRemove(msg); break;
		case RecvRuleViolationCancelOpcode: parseRuleViolationCancel(msg); break;
		case RecvRuleViolationLockOpcode: parseRuleViolationLock(msg); break;
		case RecvOpenOwnChannelOpcode: parseOpenOwnChannel(msg); break;
		case RecvCloseChannelOpcode: parseCloseChannel(msg); break;
		case RecvTextMessageOpcode: parseTextMessage(msg); break;
		case RecvCancelWalkOpcode: parseCancelWalk(msg); break;
		case RecvWalkWaitOpcode: parseWalkWait(msg); break;
		case RecvUnjustifiedStatsOpcode: parseUnjustifiedStats(msg); break;
		case RecvPvpSituationsOpcode: parsePvpSituations(msg); break;
		case RecvFloorChangeUpOpcode: parseFloorChangeUp(msg); break;
		case RecvFloorChangeDownOpcode: parseFloorChangeDown(msg); break;
		case RecvUpdateLootContainersOpcode: parseUpdateLootContainers(msg); break;
		case RecvPlayerDataTournamentOpcode: parsePlayerDataTournament(msg); break;
		case RecvTournamentInformationOpcode: parseTournamentInformation(msg); break;
		case RecvTournamentLeaderboardOpcode: parseTournamentLeaderboard(msg); break;
		case RecvChooseOutfitOpcode: parseChooseOutfit(msg); break;
		case RecvExivaSuppressedOpcode: parseExivaSuppressed(msg); break;
		case RecvUpdateExivaOptionsOpcode: parseUpdateExivaOptions(msg); break;
		case RecvTransactionDetailsOpcode: parseTransactionDetails(msg); break;
		case RecvImpactTrackingOpcode: parseImpactTracking(msg); break;
		case RecvItemWastedOpcode: parseItemWasted(msg); break;
		case RecvItemLootedOpcode: parseItemLooted(msg); break;
		case RecvTrackedQuestFlagsOpcode: parseTrackedQuestFlags(msg); break;
		case RecvKillTrackingOpcode: parseKillTracking(msg); break;
		case RecvMarketStatisticsOpcode: parseMarketStatistics(msg);
		case RecvVipAddOpcode: parseVipAdd(msg); break;
		case RecvVipStatusOpcode: parseVipStatus(msg); break;
		case RecvVipStatusLogoutOpcode: parseVipStatusLogout(msg); break;
		case RecvMonsterCyclopediaOpcode: parseMonsterCyclopedia(msg); break;
		case RecvMonsterCyclopediaMonstersOpcode: parseMonsterCyclopediaMonsters(msg); break;
		case RecvMonsterCyclopediaRaceOpcode: parseMonsterCyclopediaRace(msg); break;
		case RecvMonsterCyclopediaBonusEffectsOpcode: parseMonsterCyclopediaBonusEffects(msg); break;
		case RecvMonsterCyclopediaNewDetailsOpcode: parseMonsterCyclopediaNewDetails(msg); break;
		case RecvCyclopediaCharacterInfoOpcode: parseCyclopediaCharacterInfo(msg); break;
		case RecvHirelingNameChangeOpcode: parseHirelingNameChange(msg); break;
		case RecvTutorialHintOpcode: parseTutorialHint(msg); break;
		case RecvAutomapFlagOpcode: (g_clientVersion >= 1180 ? parseCyclopediaMapData(msg) : parseAutomapFlag(msg)); break;
		case RecvDailyRewardCollectionStateOpcode: parseDailyRewardCollectionState(msg); break;
		case RecvStoreCoinBalanceOpcode: parseStoreCoinBalance(msg); break;
		case RecvStoreErrorOpcode: parseStoreError(msg); break;
		case RecvStoreRequestPurchaseOpcode: parseStoreRequestPurchase(msg); break;
		case RecvOpenRewardWallOpcode: parseOpenRewardWall(msg); break;
		case RecvCloseRewardWallOpcode: parseCloseRewardWall(msg); break;
		case RecvDailyRewardBasicOpcode: parseDailyRewardBasic(msg); break;
		case RecvDailyRewardHistoryOpcode: parseDailyRewardHistory(msg); break;
		case RecvPreyFreeListRerollAvailabilityOpcode: parsePreyFreeListRerollAvailability(msg); break;
		case RecvPreyTimeLeftOpcode: parsePreyTimeLeft(msg); break;
		case RecvPreyDataOpcode: parsePreyData(msg); break;
		case RecvPreyPricesOpcode: parsePreyPrices(msg); break;
		case RecvOfferDescriptionOpcode: parseOfferDescription(msg); break;
		case RecvRefreshImbuingDialogOpcode: parseRefreshImbuingDialog(msg); break;
		case RecvCloseImbuingDialogOpcode: parseCloseImbuingDialog(msg); break;
		case RecvShowMessageDialogOpcode: parseShowMessageDialog(msg); break;
		case RecvResourceBalanceOpcode: parseResourceBalance(msg); break;
		case RecvTimeOpcode: parseTime(msg); break;
		case RecvQuestLogOpcode: parseQuestLog(msg); break;
		case RecvQuestLineOpcode: parseQuestLine(msg); break;
		case RecvStoreCoinBalanceUpdatingOpcode: parseStoreCoinBalanceUpdating(msg); break;
		case RecvChannelEventOpcode: parseChannelEvent(msg); break;
		case RecvItemInfoOpcode: parseItemInfo(msg); break;
		case RecvPlayerInventoryOpcode: parsePlayerInventory(msg); break;
		case RecvMarketEnterOpcode: parseMarketEnter(msg); break;
		case RecvMarketLeaveOpcode: parseMarketLeave(msg); break;
		case RecvMarketDetailOpcode: parseMarketDetail(msg); break;
		case RecvMarketBrowseOpcode: parseMarketBrowse(msg); break;
		case RecvModalWindowOpcode: parseModalWindow(msg); break;
		case RecvStoreOpenOpcode: parseStoreOpen(msg); break;
		case RecvStoreOffersOpcode: parseStoreOffers(msg); break;
		case RecvStoreTransactionHistoryOpcode: parseStoreTransactionHistory(msg); break;
		case RecvStoreCompletePurchaseOpcode: parseStoreCompletePurchase(msg); break;
		default:
		{
			msg.setReadPos(msg.getMessageSize());//Make sure we don't read anymore
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "Received unsupported packet header: 0x%02X", SDL_static_cast(Uint32, header));
			UTIL_messageBox("Error", std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		break;
	}
	m_skipErrors = true;//Since we read something skip disconnect errors
}

void ProtocolGame::parseCreatureData(InputMessage& msg)
{
	bool something = msg.getBool();
	(void)something;
}

void ProtocolGame::parseLogin(InputMessage& msg)
{
	Uint32 playerId = msg.getU32();
	Uint16 serverBeat = msg.getU16();
	if(g_game.hasGameFeature(GAME_FEATURE_NEWSPEED_LAW))
	{
		Creature::speedA = msg.getDouble();
		Creature::speedB = msg.getDouble();
		Creature::speedC = msg.getDouble();
	}

	bool canReportBugs = msg.getBool();
	if(g_clientVersion >= 1054)
	{
		bool canChangePvPFrameRate = msg.getBool();
		(void)canChangePvPFrameRate;
	}

	if(g_clientVersion >= 1058)
	{
		bool expertModeEnabled = msg.getBool();
		(void)expertModeEnabled;
		//g_game.setExpertPvpMode(expertModeEnabled);
	}

	if(g_game.hasGameFeature(GAME_FEATURE_STORE))
	{
		std::string storeURL = msg.getString();//URL to store images
		Uint16 storePackages = msg.getU16();//Premium coin package size
		(void)storeURL;
		(void)storePackages;
	}

	if(g_clientVersion >= 1149)
	{
		Uint8 exivaRestrictions = msg.getU8();
		(void)exivaRestrictions;
	}

	if(g_game.hasGameFeature(GAME_FEATURE_TOURNAMENTS))
	{
		Uint8 tournamentStatus = msg.getU8();
		if(tournamentStatus)
		{
			Sint32 tournamentFactor = SDL_static_cast(Sint32, msg.getU32());
			(void)tournamentFactor;
		}
	}

	g_game.setPlayerID(playerId);
	g_game.setServerBeat(serverBeat);
	g_game.setCanReportBugs(canReportBugs);

	Creature* creature = g_map.getCreatureById(playerId);
	if(creature)
	{
		g_map.setLocalCreature(creature);
		creature->setLocalCreature(true);
	}
}

void ProtocolGame::parseLoginOrPending(InputMessage& msg)
{
	if(g_game.hasGameFeature(GAME_FEATURE_LOGIN_PENDING))
	{
		m_pendingGame = true;
		sendEnterGame();
	}
	else
	{
		parseLogin(msg);
		if(!m_gameInitialized)
		{
			g_engine.processGameStart();
			m_gameInitialized = true;
		}
	}
}

void ProtocolGame::parseGMActions(InputMessage& msg)
{
	std::vector<Uint8> actions;
	size_t numViolationReasons;
	if(g_clientVersion >= 726 && g_clientVersion <= 730)
		numViolationReasons = 30;
	else if(g_clientVersion >= 850)
		numViolationReasons = 20;
	else if(g_clientVersion >= 820)
		numViolationReasons = 23;
	else
		numViolationReasons = 32;

	actions.resize(numViolationReasons);
	for(size_t i = 0; i < numViolationReasons; ++i)
		actions[i] = msg.getU8();

	g_game.processGMActions(actions);
}

void ProtocolGame::parseReadyForSecondaryConnection(InputMessage& msg)
{
	msg.getRawString();
}

void ProtocolGame::parseEnterGame(InputMessage&)
{
	m_pendingGame = false;
	if(!m_gameInitialized)
	{
		g_engine.processGameStart();
		m_gameInitialized = true;
	}
}

void ProtocolGame::parseUpdateNeeded(InputMessage& msg)
{
	const std::string signature = msg.getString();
	UTIL_messageBox("Error", "Client needs update.");
}

void ProtocolGame::parseLoginError(InputMessage& msg)
{
	const std::string message = msg.getString();
	UTIL_messageBox("Sorry", message);
}

void ProtocolGame::parseLoginAdvice(InputMessage& msg)
{
	const std::string message = msg.getString();
	UTIL_messageBox("For Your Information", message);
}

void ProtocolGame::parseLoginWait(InputMessage& msg)
{
	const std::string message = msg.getString();
	Uint8 time = msg.getU8();
	g_game.processLoginWait(message, time);
}

void ProtocolGame::parseLoginToken(InputMessage& msg)
{
	msg.getU8(); //Unknown
	UTIL_messageBox("Two-Factor Authentification", "A new authentification token is required.\nPlease login again.");
}

void ProtocolGame::parseStoreButtonIndicators(InputMessage& msg)
{
	msg.getBool(); //??
	msg.getBool(); //??
}

void ProtocolGame::parsePingBack(InputMessage&)
{
	sendPingBack();
}

void ProtocolGame::parsePing(InputMessage&)
{
	if(g_game.hasGameFeature(GAME_FEATURE_PING))
	{
		if(++m_pingReceived == m_pingSent)
			g_ping = SDL_static_cast(Uint16, SDL_GetTicks()-m_pingTime);
	}
	else
		sendPingBack();
}

void ProtocolGame::parseChallenge(InputMessage& msg)
{
	Uint32 timestamp = msg.getU32();
	Uint8 random = msg.getU8();
	sendLogin(timestamp, random);
}

void ProtocolGame::parseDeath(InputMessage& msg)
{
	Uint8 penalty = 100;
	Uint8 deathType = DeathType_Regular;
	if(g_game.hasGameFeature(GAME_FEATURE_DEATH_TYPE))
		deathType = msg.getU8();

	if(g_game.hasGameFeature(GAME_FEATURE_DEATH_PENALTY) && deathType == DeathType_Regular)
		penalty = msg.getU8();

	bool deathRedemption = false;
	if(g_clientVersion >= 1121)
		deathRedemption = msg.getBool();

	(void)deathRedemption;
	g_game.processDeath(deathType, penalty);
}

void ProtocolGame::parseClientCheck(InputMessage& msg)
{
	Uint32 size = msg.getU32();
	for(Uint32 i = 0; i < size; ++i)
		msg.getU8();
}

void ProtocolGame::parseTime(InputMessage& msg)
{
	Uint8 hour = msg.getU8();
	Uint8 minute = msg.getU8();
	(void)hour;
	(void)minute;
}

void ProtocolGame::parseScreenshotEvent(InputMessage& msg)
{
	msg.getU8();//??
}

void ProtocolGame::parseMapDescription(InputMessage& msg)
{
	const Position pos = msg.getPosition();
	g_map.changeMap(DIRECTION_INVALID);
	g_map.setCentralPosition(pos);
	g_map.needUpdateCache();
	g_light.changeFloor(pos.z);
	g_automap.setCentralPosition(pos);
	g_game.stopAutoWalk();
	setMapDescription(msg, pos.x-(MAP_WIDTH_OFFSET-1), pos.y-(MAP_HEIGHT_OFFSET-1), pos.z, GAME_MAP_WIDTH, GAME_MAP_HEIGHT);
}

void ProtocolGame::parseMapNorth(InputMessage& msg)
{
	Position pos = g_map.getCentralPosition();
	--pos.y;
	g_map.changeMap(DIRECTION_NORTH);
	g_map.setCentralPosition(pos);
	g_map.needUpdateCache();
	g_automap.setCentralPosition(pos);
	setMapDescription(msg, pos.x-(MAP_WIDTH_OFFSET-1), pos.y-(MAP_HEIGHT_OFFSET-1), pos.z, GAME_MAP_WIDTH, 1);
}

void ProtocolGame::parseMapEast(InputMessage& msg)
{
	Position pos = g_map.getCentralPosition();
    ++pos.x;
	g_map.changeMap(DIRECTION_EAST);
	g_map.setCentralPosition(pos);
	g_map.needUpdateCache();
	g_automap.setCentralPosition(pos);
    setMapDescription(msg, pos.x+MAP_WIDTH_OFFSET, pos.y-(MAP_HEIGHT_OFFSET-1), pos.z, 1, GAME_MAP_HEIGHT);
}

void ProtocolGame::parseMapSouth(InputMessage& msg)
{
	Position pos = g_map.getCentralPosition();
    ++pos.y;
	g_map.changeMap(DIRECTION_SOUTH);
	g_map.setCentralPosition(pos);
	g_map.needUpdateCache();
	g_automap.setCentralPosition(pos);
    setMapDescription(msg, pos.x-(MAP_WIDTH_OFFSET-1), pos.y+MAP_HEIGHT_OFFSET, pos.z, GAME_MAP_WIDTH, 1);
}

void ProtocolGame::parseMapWest(InputMessage& msg)
{
	Position pos = g_map.getCentralPosition();
    --pos.x;
	g_map.changeMap(DIRECTION_WEST);
	g_map.setCentralPosition(pos);
	g_map.needUpdateCache();
	g_automap.setCentralPosition(pos);
    setMapDescription(msg, pos.x-(MAP_WIDTH_OFFSET-1), pos.y-(MAP_HEIGHT_OFFSET-1), pos.z, 1, GAME_MAP_HEIGHT);
}

void ProtocolGame::parseUpdateTile(InputMessage& msg)
{
	const Position tilePos = msg.getPosition();
	setTileDescription(msg, tilePos, SDL_static_cast(Sint32, g_map.getCentralPosition().z)-SDL_static_cast(Sint32, tilePos.z));
	g_map.needUpdateCache();
}

void ProtocolGame::parseTileAddThing(InputMessage& msg)
{
	const Position pos = msg.getPosition();
	Uint8 stackPos = 0xFF;
	if(g_clientVersion >= 841)
		stackPos = msg.getU8();

	Thing* thing = internalGetThing(msg, pos);
	Tile* tile = g_map.getTile(pos);
	if(!tile || !thing)
	{
		if(!tile)
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(X: %u, Y: %u, Z: %u).", "[ProtocolGame::parseTileAddThing] Tile not found", SDL_static_cast(Uint32, pos.x), SDL_static_cast(Uint32, pos.y), SDL_static_cast(Uint32, pos.z));
			g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		else if(!thing)
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(X: %u, Y: %u, Z: %u).", "[ProtocolGame::parseTileAddThing] Invalid thing", SDL_static_cast(Uint32, pos.x), SDL_static_cast(Uint32, pos.y), SDL_static_cast(Uint32, pos.z));
			g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		return;
	}

	if(stackPos == 0xFF)
	{
		if(thing->getCreature())
			tile->addThing(thing, (g_clientVersion < 853));
		else
			tile->addThing(thing, true);
	}
	else
		tile->insertThing(thing, SDL_static_cast(Uint32, stackPos));

	if(!thing->getCreature())
	{
		updateMinimapTile(pos, tile);
		g_map.needUpdateCache();
	}
}

void ProtocolGame::parseTileTransformThing(InputMessage& msg)
{
	Position pos;
	pos.x = msg.getU16();
	if(pos.x == 0xFFFF)
	{
		Uint32 creatureId = msg.getU32();
		Creature* creature = g_map.getCreatureById(creatureId);
		if(creature)
		{
			pos = creature->getCurrentPosition();
			Thing* thing = internalGetThing(msg, pos);
			if(!thing || !thing->getCreature())
			{
				Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(X: %u, Y: %u, Z: %u).", "[ProtocolGame::parseTileTransformThing] Thing is not a creature", SDL_static_cast(Uint32, pos.x), SDL_static_cast(Uint32, pos.y), SDL_static_cast(Uint32, pos.z));
				g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
				return;
			}
		}
		else
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(X: %u, Y: %u, Z: %u, ID: %u).", "[ProtocolGame::parseTileTransformThing] Creature not found", SDL_static_cast(Uint32, pos.x), SDL_static_cast(Uint32, pos.y), SDL_static_cast(Uint32, pos.z), creatureId);
			g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
			return;
		}
	}
	else
	{
		pos.y = msg.getU16();
		pos.z = msg.getU8();

		Uint32 stackPos = SDL_static_cast(Uint32, msg.getU8());
		Thing* thing = internalGetThing(msg, pos);
		Tile* tile = g_map.getTile(pos);
		if(!tile || !thing)
		{
			if(!tile)
			{
				Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(X: %u, Y: %u, Z: %u).", "[ProtocolGame::parseTileTransformThing] Tile not found", SDL_static_cast(Uint32, pos.x), SDL_static_cast(Uint32, pos.y), SDL_static_cast(Uint32, pos.z));
				g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
			}
			else if(!thing)
			{
				Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(X: %u, Y: %u, Z: %u).", "[ProtocolGame::parseTileTransformThing] Invalid thing", SDL_static_cast(Uint32, pos.x), SDL_static_cast(Uint32, pos.y), SDL_static_cast(Uint32, pos.z));
				g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
			}
			return;
		}

		if(!thing->getCreature())//Creatures get updated automatically so don't waste cpu power
		{
			Thing* excessThing = tile->getThingByStackPos(stackPos);
			if(!excessThing)
			{
				Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(X: %u, Y: %u, Z: %u, Stack: %u).", "[ProtocolGame::parseTileTransformThing] Thing not found", SDL_static_cast(Uint32, pos.x), SDL_static_cast(Uint32, pos.y), SDL_static_cast(Uint32, pos.z), stackPos);
				g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
				return;
			}
			if(!tile->removeThing(excessThing))
			{
				Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(X: %u, Y: %u, Z: %u, Stack: %u).", "[ProtocolGame::parseTileTransformThing] Failed to remove thing", SDL_static_cast(Uint32, pos.x), SDL_static_cast(Uint32, pos.y), SDL_static_cast(Uint32, pos.z), stackPos);
				g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
				return;
			}
			tile->insertThing(thing, stackPos);
			updateMinimapTile(pos, tile);
			g_map.needUpdateCache();
		}
	}
}

void ProtocolGame::parseTileRemoveThing(InputMessage& msg)
{
	Position pos;
	pos.x = msg.getU16();
	if(pos.x == 0xFFFF)
	{
		Uint32 creatureId = msg.getU32();
		Creature* creature = g_map.getCreatureById(creatureId);
		if(creature)
		{
			pos = creature->getCurrentPosition();
			Tile* tile = g_map.getTile(pos);
			if(!tile)
			{
				Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(X: %u, Y: %u, Z: %u).", "[ProtocolGame::parseTileRemoveThing] Tile not found", SDL_static_cast(Uint32, pos.x), SDL_static_cast(Uint32, pos.y), SDL_static_cast(Uint32, pos.z));
				g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
				return;
			}
			if(!tile->removeThing(creature))
			{
				Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(X: %u, Y: %u, Z: %u).", "[ProtocolGame::parseTileRemoveThing] Failed to remove creature", SDL_static_cast(Uint32, pos.x), SDL_static_cast(Uint32, pos.y), SDL_static_cast(Uint32, pos.z));
				g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
				return;
			}
		}
		else
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(X: %u, Y: %u, Z: %u, ID: %u).", "[ProtocolGame::parseTileRemoveThing] Creature not found", SDL_static_cast(Uint32, pos.x), SDL_static_cast(Uint32, pos.y), SDL_static_cast(Uint32, pos.z), creatureId);
			g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
			return;
		}
	}
	else
	{
		pos.y = msg.getU16();
		pos.z = msg.getU8();

		Uint32 stackPos = SDL_static_cast(Uint32, msg.getU8());
		Tile* tile = g_map.getTile(pos);
		if(!tile)
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(X: %u, Y: %u, Z: %u).", "[ProtocolGame::parseTileRemoveThing] Tile not found", SDL_static_cast(Uint32, pos.x), SDL_static_cast(Uint32, pos.y), SDL_static_cast(Uint32, pos.z));
			g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
			return;
		}

		Thing* excessThing = tile->getThingByStackPos(stackPos);
		if(!excessThing)
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(X: %u, Y: %u, Z: %u, Stack: %u).", "[ProtocolGame::parseTileRemoveThing] Thing not found", SDL_static_cast(Uint32, pos.x), SDL_static_cast(Uint32, pos.y), SDL_static_cast(Uint32, pos.z), stackPos);
			g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
			return;
		}
		bool creature = excessThing->getCreature();
		if(!tile->removeThing(excessThing))
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(X: %u, Y: %u, Z: %u, Stack: %u).", "[ProtocolGame::parseTileRemoveThing] Failed to remove thing", SDL_static_cast(Uint32, pos.x), SDL_static_cast(Uint32, pos.y), SDL_static_cast(Uint32, pos.z), stackPos);
			g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
			return;
		}
		if(!creature)
		{
			updateMinimapTile(pos, tile);
			g_map.needUpdateCache();
		}
	}
}

void ProtocolGame::parseTileMoveCreature(InputMessage& msg)
{
	Position fromPos;
	Tile* oldTile = NULL;
	Creature* creature = NULL;
	fromPos.x = msg.getU16();
	if(fromPos.x == 0xFFFF)
	{
		Uint32 creatureId = msg.getU32();
		creature = g_map.getCreatureById(creatureId);
		if(creature)
		{
			fromPos = creature->getCurrentPosition();
			oldTile = g_map.getTile(fromPos);
			if(!oldTile)
			{
				Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(X: %u, Y: %u, Z: %u).", "[ProtocolGame::parseTileMoveCreature] Old tile not found", SDL_static_cast(Uint32, fromPos.x), SDL_static_cast(Uint32, fromPos.y), SDL_static_cast(Uint32, fromPos.z));
				g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
			}
		}
		else
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(X: %u, Y: %u, Z: %u, ID: %u).", "[ProtocolGame::parseTileMoveCreature] Creature not found", SDL_static_cast(Uint32, fromPos.x), SDL_static_cast(Uint32, fromPos.y), SDL_static_cast(Uint32, fromPos.z), creatureId);
			g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
	}
	else
	{
		fromPos.y = msg.getU16();
		fromPos.z = msg.getU8();

		Uint32 stackPos = SDL_static_cast(Uint32, msg.getU8());
		oldTile = g_map.getTile(fromPos);
		if(!oldTile)
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(X: %u, Y: %u, Z: %u).", "[ProtocolGame::parseTileMoveCreature] Old tile not found", SDL_static_cast(Uint32, fromPos.x), SDL_static_cast(Uint32, fromPos.y), SDL_static_cast(Uint32, fromPos.z));
			g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		else
		{
			Thing* thing = oldTile->getThingByStackPos(stackPos);
			if(!thing || !thing->getCreature())
			{
				if(thing)
				{
					Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(X: %u, Y: %u, Z: %u, Stack: %u).", "[ProtocolGame::parseTileMoveCreature] Thing is not a valid creature", SDL_static_cast(Uint32, fromPos.x), SDL_static_cast(Uint32, fromPos.y), SDL_static_cast(Uint32, fromPos.z), stackPos);
					g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
				}
				else
				{
					Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(X: %u, Y: %u, Z: %u, Stack: %u).", "[ProtocolGame::parseTileMoveCreature] Thing not found", SDL_static_cast(Uint32, fromPos.x), SDL_static_cast(Uint32, fromPos.y), SDL_static_cast(Uint32, fromPos.z), stackPos);
					g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
				}
			}
			else
				creature = thing->getCreature();
		}
	}

	const Position toPos = msg.getPosition();
	Tile* newTile = g_map.getTile(toPos);
	if(!newTile)
	{
		Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(X: %u, Y: %u, Z: %u).", "[ProtocolGame::parseTileMoveCreature] New tile not found", SDL_static_cast(Uint32, toPos.x), SDL_static_cast(Uint32, toPos.y), SDL_static_cast(Uint32, toPos.z));
		g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
		return;
	}
	else if(!oldTile || !creature)
		return;

	creature->move(fromPos, toPos, oldTile, newTile);
	
	SortMethods sortMethod = g_engine.getBattleSortMethod();
	if(sortMethod == Sort_Ascending_Distance || sortMethod == Sort_Descending_Distance)
		UTIL_sortBattleWindow();
}

void ProtocolGame::parseFloorChangeUp(InputMessage& msg)
{
	Position pos = g_map.getCentralPosition();
	--pos.z;

    Sint32 skip = 0;
	if(pos.z == GAME_PLAYER_FLOOR)
	{
		//Going to surface - floor 7 and 6 are already set
		g_map.removeDistanceEffects(5);
		g_map.removeDistanceEffects(4);
		g_map.removeDistanceEffects(3);
		g_map.removeDistanceEffects(2);
		g_map.removeDistanceEffects(1);
		g_map.removeDistanceEffects(0);
		skip = setFloorDescription(msg, pos.x-(MAP_WIDTH_OFFSET-1), pos.y-(MAP_HEIGHT_OFFSET-1), 5, GAME_MAP_WIDTH, GAME_MAP_HEIGHT, 3, skip);
		skip = setFloorDescription(msg, pos.x-(MAP_WIDTH_OFFSET-1), pos.y-(MAP_HEIGHT_OFFSET-1), 4, GAME_MAP_WIDTH, GAME_MAP_HEIGHT, 4, skip);
		skip = setFloorDescription(msg, pos.x-(MAP_WIDTH_OFFSET-1), pos.y-(MAP_HEIGHT_OFFSET-1), 3, GAME_MAP_WIDTH, GAME_MAP_HEIGHT, 5, skip);
		skip = setFloorDescription(msg, pos.x-(MAP_WIDTH_OFFSET-1), pos.y-(MAP_HEIGHT_OFFSET-1), 2, GAME_MAP_WIDTH, GAME_MAP_HEIGHT, 6, skip);
		skip = setFloorDescription(msg, pos.x-(MAP_WIDTH_OFFSET-1), pos.y-(MAP_HEIGHT_OFFSET-1), 1, GAME_MAP_WIDTH, GAME_MAP_HEIGHT, 7, skip);
		skip = setFloorDescription(msg, pos.x-(MAP_WIDTH_OFFSET-1), pos.y-(MAP_HEIGHT_OFFSET-1), 0, GAME_MAP_WIDTH, GAME_MAP_HEIGHT, 8, skip);
	}
	else if(pos.z > GAME_PLAYER_FLOOR)//Going still underground
	{
		g_map.removeDistanceEffects(pos.z-GAME_MAP_AWARERANGE);
		skip = setFloorDescription(msg, pos.x-(MAP_WIDTH_OFFSET-1), pos.y-(MAP_HEIGHT_OFFSET-1), pos.z-GAME_MAP_AWARERANGE, GAME_MAP_WIDTH, GAME_MAP_HEIGHT, 3, skip);
	}

    ++pos.x;
    ++pos.y;
    g_map.setCentralPosition(pos);
	g_map.needUpdateCache();
	g_automap.setCentralPosition(pos);
	g_light.changeFloor(pos.z);
}

void ProtocolGame::parseFloorChangeDown(InputMessage& msg)
{
	Position pos = g_map.getCentralPosition();
	++pos.z;

	Sint32 skip = 0;
    if(pos.z == (GAME_PLAYER_FLOOR+1))
	{
		//Going from surface to underground
		g_map.removeDistanceEffects(pos.z);
		g_map.removeDistanceEffects(pos.z+1);
		g_map.removeDistanceEffects(pos.z+2);
        skip = setFloorDescription(msg, pos.x-(MAP_WIDTH_OFFSET-1), pos.y-(MAP_HEIGHT_OFFSET-1), pos.z, GAME_MAP_WIDTH, GAME_MAP_HEIGHT, -1, skip);
		skip = setFloorDescription(msg, pos.x-(MAP_WIDTH_OFFSET-1), pos.y-(MAP_HEIGHT_OFFSET-1), pos.z+1, GAME_MAP_WIDTH, GAME_MAP_HEIGHT, -2, skip);
		skip = setFloorDescription(msg, pos.x-(MAP_WIDTH_OFFSET-1), pos.y-(MAP_HEIGHT_OFFSET-1), pos.z+2, GAME_MAP_WIDTH, GAME_MAP_HEIGHT, -3, skip);
    }
	else if(pos.z > (GAME_PLAYER_FLOOR + 1) && pos.z < (GAME_MAP_FLOORS - 1))//Going further down
	{
		g_map.removeDistanceEffects(pos.z+GAME_MAP_AWARERANGE);
		skip = setFloorDescription(msg, pos.x-(MAP_WIDTH_OFFSET-1), pos.y-(MAP_HEIGHT_OFFSET-1), pos.z+GAME_MAP_AWARERANGE, GAME_MAP_WIDTH, GAME_MAP_HEIGHT, -3, skip);
	}

    --pos.x;
    --pos.y;
    g_map.setCentralPosition(pos);
	g_map.needUpdateCache();
	g_automap.setCentralPosition(pos);
	g_light.changeFloor(pos.z);
}

void ProtocolGame::parseContainerOpen(InputMessage& msg)
{
	Uint8 containerId = msg.getU8();
	Uint16 thingId = msg.getU16();
	ItemUI* containerItem = getItemUI(msg, thingId);
	const std::string name = UTIL_formatCreatureName(msg.getString());
	Uint8 capacity = msg.getU8();
	bool hasParent = msg.getBool();

	bool canUseDepotSearch = false;
	bool isUnlocked = true;
	bool hasPages = false;
	Uint16 containerSize = 0;
	Uint16 firstIndex = 0;
	if(g_game.hasGameFeature(GAME_FEATURE_CONTAINER_PAGINATION))
	{
		if(g_clientVersion >= 1220)
			canUseDepotSearch = msg.getBool();

		isUnlocked = msg.getBool();
		hasPages = msg.getBool();
		containerSize = msg.getU16();
		firstIndex = msg.getU16();
	}

	std::vector<ItemUI*> itemVector;
	Uint8 itemCount = msg.getU8();
	for(Uint8 i = 0; i < itemCount; ++i)
	{
		thingId = msg.getU16();
		ItemUI* insideItem = getItemUI(msg, thingId);
		if(!insideItem)
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(ThingID: %u).", "[ProtocolGame::parseContainerOpen] Item not found", SDL_static_cast(Uint32, thingId));
			g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
			continue;
		}
		itemVector.push_back(insideItem);
	}
	if(!containerItem)
	{
		Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(ThingID: %u).", "[ProtocolGame::parseContainerOpen] Item not found", SDL_static_cast(Uint32, thingId));
		g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
	}
	g_game.processContainerOpen(containerId, containerItem, name, capacity, hasParent, canUseDepotSearch, isUnlocked, hasPages, containerSize, firstIndex, itemVector);
}

void ProtocolGame::parseContainerClose(InputMessage& msg)
{
	Uint8 containerId = msg.getU8();
	g_game.processContainerClose(containerId);
}

void ProtocolGame::parseContainerAddItem(InputMessage& msg)
{
	Uint8 containerId = msg.getU8();
	Uint16 slot = 0;
	if(g_game.hasGameFeature(GAME_FEATURE_CONTAINER_PAGINATION))
		slot = msg.getU16();

	ItemUI* item = getItemUI(msg, msg.getU16());
	g_game.processContainerAddItem(containerId, slot, item);
}

void ProtocolGame::parseContainerTransformItem(InputMessage& msg)
{
	Uint8 containerId = msg.getU8();
	Uint16 slot;
	if(g_game.hasGameFeature(GAME_FEATURE_CONTAINER_PAGINATION))
		slot = msg.getU16();
	else
		slot = SDL_static_cast(Uint16, msg.getU8());

	ItemUI* item = getItemUI(msg, msg.getU16());
	g_game.processContainerTransformItem(containerId, slot, item);
}

void ProtocolGame::parseContainerRemoveItem(InputMessage& msg)
{
	Uint8 containerId = msg.getU8();
	Uint16 slot;
	ItemUI* lastItem = NULL;
	if(g_game.hasGameFeature(GAME_FEATURE_CONTAINER_PAGINATION))
	{
		slot = msg.getU16();

		Uint16 thingId = msg.getU16();
		if(thingId != 0)
			lastItem = getItemUI(msg, thingId);
	}
	else
		slot = SDL_static_cast(Uint16, msg.getU8());

	g_game.processContainerRemoveItem(containerId, slot, lastItem);
}

void ProtocolGame::parseStash(InputMessage& msg)
{
	Uint16 availableItems = msg.getU16();
	for(Uint16 i = 0; i < availableItems; ++i)
	{
		Uint16 itemId = msg.getU16();
		Uint32 itemCount = msg.getU32();
		(void)itemId;
		(void)itemCount;
	}

	Uint16 emptySlots = msg.getU16();
	(void)emptySlots;
}

void ProtocolGame::parseSpecialContainersAvailable(InputMessage& msg)
{
	msg.getBool();//??
	msg.getBool();//??
}

void ProtocolGame::parseDepotSearchResult(InputMessage& msg)
{
	Uint16 size = msg.getU16();
	for(Uint16 i = 0; i < size; ++i)
	{
		Uint16 itemId = msg.getU16();
		Uint16 itemCount = msg.getU16();
		(void)itemId;
		(void)itemCount;
	}
}

void ProtocolGame::parseDepotSearchDetailList(InputMessage& msg)
{
	msg.getU16();//??
	msg.getU32();//??
	Uint8 size = msg.getU8();
	for(Uint8 i = 0; i < size; ++i)
	{
		ItemUI* item = getItemUI(msg, msg.getU16());
		(void)item;
	}

	msg.getU32();//??
	size = msg.getU8();
	for(Uint8 i = 0; i < size; ++i)
	{
		ItemUI* item = getItemUI(msg, msg.getU16());
		(void)item;
	}

	size = msg.getU8();
	for(Uint8 i = 0; i < size; ++i)
	{
		msg.getU16();//itemId?
		msg.getU32();//itemCount?
	}
}

void ProtocolGame::parseCloseDepotSearch(InputMessage&)
{
}

void ProtocolGame::parseInspectionList(InputMessage& msg)
{
	Uint8 creatureType = msg.getU8();//1 - player

	Uint8 size = msg.getU8();
	for(Uint8 i = 0; i < size; ++i)
	{
		if(creatureType == 1)
		{
			msg.getU8();//??
		}

		msg.getRawString();//??
		ItemUI* item = getItemUI(msg, msg.getU16());
		(void)item;

		Uint8 count = msg.getU8();
		for(Uint8 j = 0; j < count; ++j)
		{
			msg.getU16();//??
		}

		count = msg.getU8();
		for(Uint8 j = 0; j < count; ++j)
		{
			msg.getRawString();//??
			msg.getRawString();//??
		}
	}

	if(creatureType == 1)
	{
		std::string playerName = msg.getString();
		(void)playerName;

		Uint16 lookType;
		if(g_game.hasGameFeature(GAME_FEATURE_LOOKTYPE_U16))
			lookType = msg.getU16();
		else
			lookType = SDL_static_cast(Uint16, msg.getU8());

		Uint16 lookTypeEx = 0;
		Uint8 lookHead = 0;
		Uint8 lookBody = 0;
		Uint8 lookLegs = 0;
		Uint8 lookFeet = 0;
		Uint8 lookAddons = 0;
		if(lookType != 0)
		{
			lookHead = msg.getU8();
			lookBody = msg.getU8();
			lookLegs = msg.getU8();
			lookFeet = msg.getU8();
			if(g_game.hasGameFeature(GAME_FEATURE_ADDONS))
				lookAddons = msg.getU8();
		}
		else
			lookTypeEx = msg.getU16();

		(void)lookType;
		(void)lookTypeEx;
		(void)lookHead;
		(void)lookBody;
		(void)lookLegs;
		(void)lookFeet;
		(void)lookAddons;

		Uint8 details = msg.getU8();
		for(Uint8 j = 0; j < details; ++j)
		{
			std::string name = msg.getString();
			std::string description = msg.getString();
			(void)name;
			(void)description;
		}
	}
}

void ProtocolGame::parseInspectionState(InputMessage& msg)
{
	Uint32 creatureId = msg.getU32();
	Uint8 state = msg.getU8();
	(void)creatureId;
	(void)state;
}

void ProtocolGame::parseInventoryTransformItem(InputMessage& msg)
{
	Uint8 slot = msg.getU8();
	if(slot > 0)
		slot -= 1;

	Uint16 thingId = msg.getU16();
	ItemUI* item = getItemUI(msg, thingId);
	if(!item)
	{
		Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(ThingID: %u).", "[ProtocolGame::parseInventoryTransformItem] Item not found", SDL_static_cast(Uint32, thingId));
		g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
	}
	g_game.processTransformInventoryItem(slot, item);
}

void ProtocolGame::parseInventoryRemoveItem(InputMessage& msg)
{
	Uint8 slot = msg.getU8();
	if(slot > 0)
		slot -= 1;

	g_game.processTransformInventoryItem(slot, NULL);
}

void ProtocolGame::parseNpcOpenTrade(InputMessage& msg)
{
	std::string npcName;
	if(g_game.hasGameFeature(GAME_FEATURE_NPC_NAME_ON_TRADE))
	{
		npcName = msg.getString();
		if(g_clientVersion >= 1203)
			msg.getU16();
	}
	(void)npcName;

	Uint16 listCount;
	if(g_clientVersion >= 980)
		listCount = msg.getU16();
	else
		listCount = SDL_static_cast(Uint16, msg.getU8());

	for(Uint16 i = 0; i < listCount; ++i)
	{
		Uint16 itemId = msg.getU16();
		Uint8 itemCount = msg.getU8();
		std::string itemName = msg.getString();
		Uint32 itemWeight = msg.getU32();
		Uint32 buyPrice = msg.getU32();
		Uint32 sellPrice = msg.getU32();

		(void)itemId;
		(void)itemCount;
		(void)itemName;
		(void)itemWeight;
		(void)buyPrice;
		(void)sellPrice;
	}
}

void ProtocolGame::parseNpcPlayerGoods(InputMessage& msg)
{
	Uint64 playerMoney;
	if(g_clientVersion >= 973)
		playerMoney = msg.getU64();
	else
		playerMoney = SDL_static_cast(Uint64, msg.getU32());

	Uint8 count = msg.getU8();
	for(Uint8 i = 0; i < count; ++i)
	{
		Uint16 itemId = msg.getU16();
		Uint16 itemCount;
		if(g_game.hasGameFeature(GAME_FEATURE_DOUBLE_SHOPSELLAMOUNT))
			itemCount = msg.getU16();
		else
			itemCount = SDL_static_cast(Uint16, msg.getU8());

		(void)itemId;
		(void)itemCount;
	}
	(void)playerMoney;
}

void ProtocolGame::parseNpcCloseTrade(InputMessage&)
{

}

void ProtocolGame::parseTradeOwn(InputMessage& msg)
{
	const std::string name = UTIL_formatCreatureName(msg.getString());

	std::vector<ItemUI*> itemVector;
	Uint8 itemCount = msg.getU8();
	for(Uint8 i = 0; i < itemCount; ++i)
	{
		Uint16 thingId = msg.getU16();
		ItemUI* insideItem = getItemUI(msg, thingId);
		if(!insideItem)
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(ThingID: %u).", "[ProtocolGame::parseTradeOwn] Item not found", SDL_static_cast(Uint32, thingId));
			g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
			continue;
		}
		itemVector.push_back(insideItem);
	}
	g_game.processTradeOwn(name, itemVector);
}

void ProtocolGame::parseTradeCounter(InputMessage& msg)
{
	const std::string name = UTIL_formatCreatureName(msg.getString());

	std::vector<ItemUI*> itemVector;
	Uint8 itemCount = msg.getU8();
	for(Uint8 i = 0; i < itemCount; ++i)
	{
		Uint16 thingId = msg.getU16();
		ItemUI* insideItem = getItemUI(msg, thingId);
		if(!insideItem)
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(ThingID: %u).", "[ProtocolGame::parseTradeCounter] Item not found", SDL_static_cast(Uint32, thingId));
			g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
			continue;
		}
		itemVector.push_back(insideItem);
	}
	g_game.processTradeCounter(name, itemVector);
}

void ProtocolGame::parseTradeClose(InputMessage&)
{
	g_game.processTradeClose();
}

void ProtocolGame::parseAmbientWorldLight(InputMessage& msg)
{
	Uint8 lightLevel = msg.getU8();
	Uint8 lightColor = msg.getU8();
	g_light.setGlobalLight(SDL_static_cast(Uint16, lightLevel), SDL_static_cast(Uint16, lightColor), g_map.getCentralPosition().z);
}

void ProtocolGame::parseMagicEffects(InputMessage& msg)
{
	Position initialPos = msg.getPosition();
	Position fromPos = initialPos;
	Uint32 delta = 0;
	Uint16 delay = 0;
	Uint8 effectType;
	do
	{
		effectType = msg.getU8();
		switch(effectType)
		{
			case MAGIC_EFFECTS_END_LOOP: return;
			case MAGIC_EFFECTS_DELTA:
			{
				delta += SDL_static_cast(Uint32, msg.getU8());
				fromPos.x = initialPos.x+SDL_static_cast(Uint16, delta%GAME_MAP_WIDTH);
				fromPos.y = initialPos.y+SDL_static_cast(Uint16, delta/GAME_MAP_WIDTH);
			}
			break;
			case MAGIC_EFFECTS_DELAY: delay = msg.getU16(); break;
			case MAGIC_EFFECTS_CREATE_EFFECT:
			{
				Uint16 effectId;
				if(g_game.hasGameFeature(GAME_FEATURE_EFFECTS_U16))
					effectId = msg.getU16();
				else
					effectId = SDL_static_cast(Uint16, msg.getU8());

				g_map.checkMagicEffects();
				Tile* tile = g_map.getTile(fromPos);
				if(tile)
				{
					Effect* effect = Effect::createEffect(fromPos, effectId);
					if(effect)
						tile->addEffect(effect);
				}

				delay &= 0;
			}
			break;
			case MAGIC_EFFECTS_CREATE_DISTANCEEFFECT:
			case MAGIC_EFFECTS_CREATE_DISTANCEEFFECT_REVERSED:
			{
				Uint16 effectId;
				if(g_game.hasGameFeature(GAME_FEATURE_DISTANCEEFFECTS_U16))
					effectId = msg.getU16();
				else
					effectId = SDL_static_cast(Uint16, msg.getU8());

				Sint32 deltaX = SDL_static_cast(Sint32, SDL_static_cast(Sint8, msg.getU8()));
				Sint32 deltaY = SDL_static_cast(Sint32, SDL_static_cast(Sint8, msg.getU8()));
				Position toPos = Position(SDL_static_cast(Uint16, SDL_static_cast(Sint32, fromPos.x)+deltaX), SDL_static_cast(Uint16, SDL_static_cast(Sint32, fromPos.y)+deltaY), fromPos.z);

				g_map.checkDistanceEffects();
				if(effectType == MAGIC_EFFECTS_CREATE_DISTANCEEFFECT_REVERSED)
				{
					DistanceEffect* distanceEffect = DistanceEffect::createDistanceEffect(toPos, fromPos, effectId);
					if(distanceEffect)
						g_map.addDistanceEffect(distanceEffect, toPos.z);
				}
				else
				{
					DistanceEffect* distanceEffect = DistanceEffect::createDistanceEffect(fromPos, toPos, effectId);
					if(distanceEffect)
						g_map.addDistanceEffect(distanceEffect, fromPos.z);
				}

				delay &= 0;
			}
			break;
			default: break;
		}
	} while(true);
}

void ProtocolGame::parseMagicEffect(InputMessage& msg)
{
	const Position pos = msg.getPosition();
	Uint16 type;
	if(g_game.hasGameFeature(GAME_FEATURE_EFFECTS_U16))
		type = msg.getU16();
	else
		type = SDL_static_cast(Uint16, msg.getU8());

	g_map.checkMagicEffects();
	Tile* tile = g_map.getTile(pos);
	if(tile)
	{
		Effect* effect = Effect::createEffect(pos, type);
		if(effect)
			tile->addEffect(effect);
	}
}

void ProtocolGame::parseRemoveMagicEffect(InputMessage& msg)
{
	//TODO: add remove magic effect feature
	const Position pos = msg.getPosition();
	Uint16 effectId;
	if(g_game.hasGameFeature(GAME_FEATURE_EFFECTS_U16))
		effectId = msg.getU16();
	else
		effectId = SDL_static_cast(Uint16, msg.getU8());
	(void)pos;
	(void)effectId;
}

void ProtocolGame::parseAnimatedText(InputMessage& msg)
{
	const Position pos = msg.getPosition();
	Uint8 color = msg.getU8();
	const std::string text = msg.getString();
	g_map.addAnimatedText(pos, color, text);
}

void ProtocolGame::parseDistanceEffect(InputMessage& msg)
{
	const Position fromPos = msg.getPosition();
	const Position toPos = msg.getPosition();
	Uint16 type;
	if(g_game.hasGameFeature(GAME_FEATURE_DISTANCEEFFECTS_U16))
		type = msg.getU16();
	else
		type = SDL_static_cast(Uint16, msg.getU8());

	g_map.checkDistanceEffects();
	DistanceEffect* distanceEffect = DistanceEffect::createDistanceEffect(fromPos, toPos, type);
	if(distanceEffect)
		g_map.addDistanceEffect(distanceEffect, fromPos.z);
}

void ProtocolGame::parseCreatureMark(InputMessage& msg)
{
	Uint32 creatureId = msg.getU32();
	Uint8 color = msg.getU8();
	Creature* creature = g_map.getCreatureById(creatureId);
	if(creature)
		creature->addTimedSquare(color);
	else
	{
		Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(ID: %u).", "[ProtocolGame::parseCreatureMark] Creature not found", creatureId);
		g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
	}
}

void ProtocolGame::parseCreatureTrappers(InputMessage& msg)
{
	Uint8 numTrappers = msg.getU8();
	for(Uint8 i = 0; i < numTrappers; ++i)
	{
		Uint32 creatureId = msg.getU32();
		Creature* creature = g_map.getCreatureById(creatureId);
		if(!creature)
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(ID: %u).", "[ProtocolGame::parseCreatureTrappers] Creature not found", creatureId);
			g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
	}
}

void ProtocolGame::parseCreatureUpdate(InputMessage& msg)
{
	Uint32 creatureId = msg.getU32();
	Uint8 updateType = msg.getU8();
	switch(updateType)
	{
		case 0:
		{
			Uint16 thingId = msg.getU16();
			if(thingId == 0x61 || thingId == 0x62 || thingId == 0x63)
				getCreature(msg, thingId, Position(0xFFFF, 0xFFFF, 0xFF));
		}
		break;
		case 11:
		{
			Uint8 manaPercent = msg.getU8();
			Creature* creature = g_map.getCreatureById(creatureId);
			if(creature)
				creature->setManaPercent(manaPercent);
			else
			{
				Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(ID: %u).", "[ProtocolGame::parseCreatureUpdate] Creature not found", creatureId);
				g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
			}
		}
		break;
		case 12:
		{
			bool showCreatureStatus = msg.getBool();
			Creature* creature = g_map.getCreatureById(creatureId);
			if(creature)
				creature->setShowStatus(showCreatureStatus);
			else
			{
				Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(ID: %u).", "[ProtocolGame::parseCreatureUpdate] Creature not found", creatureId);
				g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
			}
		}
		break;
		case 13:
		{
			Uint8 vocation = msg.getU8() % 10;
			Creature* creature = g_map.getCreatureById(creatureId);
			if(creature)
				creature->setVocation(vocation);
			else
			{
				Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(ID: %u).", "[ProtocolGame::parseCreatureUpdate] Creature not found", creatureId);
				g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
			}
		}
		break;
	}
	(void)creatureId;
}

void ProtocolGame::parseCreatureHealth(InputMessage& msg)
{
	Uint32 creatureId = msg.getU32();
	Uint8 health = msg.getU8();
	Creature* creature = g_map.getCreatureById(creatureId);
	if(creature)
	{
		creature->setHealth(health);

		SortMethods sortMethod = g_engine.getBattleSortMethod();
		if(sortMethod == Sort_Ascending_HP || sortMethod == Sort_Descending_HP)
			UTIL_sortBattleWindow();
	}
	else
	{
		Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(ID: %u).", "[ProtocolGame::parseCreatureHealth] Creature not found", creatureId);
		g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
	}
}

void ProtocolGame::parseCreatureLight(InputMessage& msg)
{
	Uint32 creatureId = msg.getU32();
	Uint16 light[2];
	light[0] = SDL_static_cast(Uint16, msg.getU8());
	light[1] = SDL_static_cast(Uint16, msg.getU8());

	Creature* creature = g_map.getCreatureById(creatureId);
	if(creature)
		creature->setLight(light);
	else
	{
		Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(ID: %u).", "[ProtocolGame::parseCreatureLight] Creature not found", creatureId);
		g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
	}
}

void ProtocolGame::parseCreatureOutfit(InputMessage& msg)
{
	Uint32 creatureId = msg.getU32();
	Uint16 lookType;
	if(g_game.hasGameFeature(GAME_FEATURE_LOOKTYPE_U16))
		lookType = msg.getU16();
	else
		lookType = SDL_static_cast(Uint16, msg.getU8());

	Uint16 lookTypeEx = 0;
	Uint8 lookHead = 0;
	Uint8 lookBody = 0;
	Uint8 lookLegs = 0;
	Uint8 lookFeet = 0;
	Uint8 lookAddons = 0;
	if(lookType != 0)
	{
		lookHead = msg.getU8();
		lookBody = msg.getU8();
		lookLegs = msg.getU8();
		lookFeet = msg.getU8();
		if(g_game.hasGameFeature(GAME_FEATURE_ADDONS))
			lookAddons = msg.getU8();
	}
	else
		lookTypeEx = msg.getU16();

	Uint16 lookMount = 0;
	if(g_game.hasGameFeature(GAME_FEATURE_MOUNTS))
		lookMount = msg.getU16();

	Creature* creature = g_map.getCreatureById(creatureId);
	if(creature)
		creature->setOutfit(lookType, lookTypeEx, lookHead, lookBody, lookLegs, lookFeet, lookAddons, lookMount);
	else
	{
		Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(ID: %u).", "[ProtocolGame::parseCreatureOutfit] Creature not found", creatureId);
		g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
	}
}

void ProtocolGame::parseCreatureSpeed(InputMessage& msg)
{
	Uint32 creatureId = msg.getU32();
	Sint32 baseSpeed = -1;
	if(g_clientVersion >= 1059)
		baseSpeed = SDL_static_cast(Sint32, msg.getU16());

	Uint16 speed = msg.getU16();
	Creature* creature = g_map.getCreatureById(creatureId);
	if(creature)
	{
		creature->setSpeed(speed);
		(void)baseSpeed;
	}
	else
	{
		Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(ID: %u).", "[ProtocolGame::parseCreatureSpeed] Creature not found", creatureId);
		g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
	}
}

void ProtocolGame::parseCreatureSkull(InputMessage& msg)
{
	Uint32 creatureId = msg.getU32();
	Uint8 skull = msg.getU8();
	Creature* creature = g_map.getCreatureById(creatureId);
	if(creature)
		creature->setSkull(skull);
	else
	{
		Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(ID: %u).", "[ProtocolGame::parseCreatureSkull] Creature not found", creatureId);
		g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
	}
}

void ProtocolGame::parseCreatureParty(InputMessage& msg)
{
	Uint32 creatureId = msg.getU32();
	Uint8 shield = msg.getU8();
	Creature* creature = g_map.getCreatureById(creatureId);
	if(creature)
		creature->setShield(shield);
	else
	{
		Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(ID: %u).", "[ProtocolGame::parseCreatureParty] Creature not found", creatureId);
		g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
	}
}

void ProtocolGame::parseCreatureUnpassable(InputMessage& msg)
{
	Uint32 creatureId = msg.getU32();
	bool unpassable = msg.getBool();
	Creature* creature = g_map.getCreatureById(creatureId);
	if(creature)
		creature->setUnpassable(unpassable);
	else
	{
		Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(ID: %u).", "[ProtocolGame::parseCreatureUnpassable] Creature not found", creatureId);
		g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
	}
}

void ProtocolGame::parseCreatureMarks(InputMessage& msg)
{
	Uint8 len;
	if(g_clientVersion >= 1035)
		len = 1;
	else
		len = msg.getU8();

	for(Uint8 i = 0; i < len; ++i)
	{
		Uint32 creatureId = msg.getU32();
		bool isPermanent = !msg.getBool();
		Uint8 markType = msg.getU8();
		Creature* creature = g_map.getCreatureById(creatureId);
		if(creature)
		{
			if(isPermanent)
			{
                if(markType == 0xFF)
                    creature->hideStaticSquare();
                else
                    creature->showStaticSquare(markType);
            }
			else
                creature->addTimedSquare(markType);
		}
		else
		{
			Sint32 strlen = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(ID: %u).", "[ProtocolGame::parseCreatureMarks] Creature not found", creatureId);
			g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, strlen)));
		}
	}
}

void ProtocolGame::parseCreatureHelpers(InputMessage& msg)
{
	Uint32 creatureId = msg.getU32();
	Uint16 helpers = msg.getU16();
	Creature* creature = g_map.getCreatureById(creatureId);
	if(creature)
		creature->setHelpers(helpers);
	else
	{
		Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(ID: %u).", "[ProtocolGame::parseCreatureHelpers] Creature not found", creatureId);
		g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
	}
}

void ProtocolGame::parseCreatureType(InputMessage& msg)
{
	Uint32 creatureId = msg.getU32();
	Uint8 creatureType = msg.getU8();
	if(g_clientVersion >= 1121 && creatureType == CREATURETYPE_SUMMON_OWN)
	{
		Uint32 masterId = msg.getU32();
		if(masterId != g_game.getPlayerID())
			creatureType = CREATURETYPE_SUMMON_OTHERS;
	}

	Creature* creature = g_map.getCreatureById(creatureId);
	if(creature)
		creature->setType(creatureType);
	else
	{
		Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(ID: %u).", "[ProtocolGame::parseCreatureType] Creature not found", creatureId);
		g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
	}
}

void ProtocolGame::parseEditTextWindow(InputMessage& msg)
{
	Uint32 windowId = msg.getU32();
	Uint16 thingId = msg.getU16();
	ItemUI* item = getItemUI(msg, thingId);
	Uint16 maxLen = msg.getU16();
	const std::string windowText = msg.getString();
	const std::string writer = msg.getString();
	std::string date = "";
	if(g_game.hasGameFeature(GAME_FEATURE_WRITABLE_DATE))
		date = msg.getString();
	if(!item)
	{
		Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(ThingID: %u).", "[ProtocolGame::parseEditTextWindow] Item not found", SDL_static_cast(Uint32, thingId));
		g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
	}
	g_game.processEditTextWindow(windowId, item, maxLen, windowText, writer, date);
}

void ProtocolGame::parseEditHouseWindow(InputMessage& msg)
{
	Uint8 doorId = msg.getU8();
	Uint32 windowId = msg.getU32();
	const std::string windowText = msg.getString();
	g_game.processEditHouseWindow(doorId, windowId, windowText);
}

void ProtocolGame::parseBlessingsDialog(InputMessage& msg)
{
	Uint8 size = msg.getU8();
	for(Uint8 i = 0; i < size; ++i)
	{
		msg.getU16();//??
		msg.getU8();//??
		msg.getU8();//??
	}

	msg.getBool();//??
	msg.getU8();//??
	msg.getU8();//??
	msg.getU8();//??
	msg.getU8();//??
	msg.getU8();//??
	msg.getU8();//??
	msg.getBool();//??
	msg.getBool();//??

	size = msg.getU8();
	for(Uint8 i = 0; i < size; ++i)
	{
		msg.getU32();//??
		msg.getU8();//??
		msg.getRawString();//??
	}
}

void ProtocolGame::parseBlessings(InputMessage& msg)
{
	Uint16 blessings = msg.getU16();
	(void)blessings;
	if(g_clientVersion >= 1121)
	{
		Uint8 status = msg.getU8();
		(void)status;
	}
}

void ProtocolGame::parsePreset(InputMessage& msg)
{
	Uint32 preset = msg.getU32();
	(void)preset;
}

void ProtocolGame::parsePremiumTrigger(InputMessage& msg)
{
	std::vector<Uint8> triggers;
	Uint8 triggerCount = msg.getU8();
	for(Uint8 i = 0; i < triggerCount; ++i)
		triggers.push_back(msg.getU8());

	if(g_clientVersion <= 1096)
	{
		bool something = msg.getBool();
		(void)something;
	}
	(void)triggers;
}

void ProtocolGame::parseTextMessage(InputMessage& msg)
{
	Uint32 channelId = 0;
	MessageMode mode = translateMessageModeFromServer(msg.getU8());
	std::string text;
	switch(mode)
	{
		case MessageChannelManagement:
		case MessageGuild:
		case MessagePartyManagement:
		case MessageParty:
		{
			channelId = SDL_static_cast(Uint32, msg.getU16());
			text = msg.getString();
		}
		break;
		case MessageDamageDealed:
		case MessageDamageReceived:
		case MessageDamageOthers:
		{
			Position pos = msg.getPosition();
			Sint32 value[2];
			Uint8 color[2];

			//Physical Damage
			value[0] = SDL_static_cast(Sint32, msg.getU32());
			color[0] = msg.getU8();

			//Magic Damage
			value[1] = SDL_static_cast(Sint32, msg.getU32());
			color[1] = msg.getU8();

			text = msg.getString();
			for(Sint32 i = 0; i < 2; ++i)
			{
				if(value[i] == 0)
					continue;

				Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%d", value[i]);
				g_map.addAnimatedText(pos, color[i], std::string(g_buffer, SDL_static_cast(size_t, len)));
			}
		}
		break;
		case MessageHeal:
		case MessageMana:
		case MessageExp:
		case MessageHealOthers:
		case MessageExpOthers:
		{
			Position pos = msg.getPosition();
			Sint32 value = SDL_static_cast(Sint32, msg.getU32());
			Uint8 color = msg.getU8();
			text = msg.getString();

			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%d", value);
			g_map.addAnimatedText(pos, color, std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		break;
		case MessageInvalid:
			break;
		default:
			text = msg.getString();
			break;
	}
	if(mode != MessageInvalid)
		g_game.processTextMessage(mode, text, channelId);
}

void ProtocolGame::parseCancelWalk(InputMessage& msg)
{
	Direction direction = SDL_static_cast(Direction, msg.getU8());
	g_game.processWalkCancel(direction);
}

void ProtocolGame::parseWalkWait(InputMessage& msg)
{
	Uint16 miliSeconds = msg.getU16();
	(void)miliSeconds;
}

void ProtocolGame::parseUnjustifiedStats(InputMessage& msg)
{
	Uint8 killsDay = msg.getU8();
	Uint8 killsDayRemaining = msg.getU8();
	Uint8 killsWeek = msg.getU8();
	Uint8 killsWeekRemaining = msg.getU8();
	Uint8 killsMonth = msg.getU8();
	Uint8 killsMonthRemaining = msg.getU8();
	Uint8 skullTime = msg.getU8();
	(void)killsDay;
	(void)killsDayRemaining;
	(void)killsWeek;
	(void)killsWeekRemaining;
	(void)killsMonth;
	(void)killsMonthRemaining;
	(void)skullTime;
}

void ProtocolGame::parsePvpSituations(InputMessage& msg)
{
	Uint8 openPvpSituations = msg.getU8();
	(void)openPvpSituations;
}

void ProtocolGame::parseUpdateLootContainers(InputMessage& msg)
{
	bool unknown = msg.getBool();
	Uint8 count = msg.getU8();
	for(Uint8 i = 0; i < count; ++i)
	{
		Uint8 type = msg.getU8();
		Uint16 objectId = msg.getU16();
		(void)type;
		(void)objectId;
	}
	(void)unknown;
}

void ProtocolGame::parseChooseOutfit(InputMessage& msg)
{
	std::vector<OutfitDetail> outfits;
	std::vector<MountDetail> mounts;
	
	Uint16 lookType;
	if(g_game.hasGameFeature(GAME_FEATURE_LOOKTYPE_U16))
		lookType = msg.getU16();
	else
		lookType = SDL_static_cast(Uint16, msg.getU8());

	Uint16 lookTypeEx = 0;
	Uint8 lookHead = 0;
	Uint8 lookBody = 0;
	Uint8 lookLegs = 0;
	Uint8 lookFeet = 0;
	Uint8 lookAddons = 0;
	if(lookType != 0)
	{
		lookHead = msg.getU8();
		lookBody = msg.getU8();
		lookLegs = msg.getU8();
		lookFeet = msg.getU8();
		if(g_game.hasGameFeature(GAME_FEATURE_ADDONS))
			lookAddons = msg.getU8();
	}
	else
		lookTypeEx = msg.getU16();

	Uint16 lookMount = 0;
	if(g_game.hasGameFeature(GAME_FEATURE_MOUNTS))
		lookMount = msg.getU16();

	if(g_game.hasGameFeature(GAME_FEATURE_NEWOUTFITS))
	{
		Uint16 outfitsCount;
		if(g_clientVersion >= 1185)
			outfitsCount = msg.getU16();
		else
			outfitsCount = SDL_static_cast(Uint16, msg.getU8());

		outfits.resize(SDL_static_cast(size_t, outfitsCount));
		for(Uint16 i = 0; i < outfitsCount; ++i)
		{
			OutfitDetail& newOutfit = outfits[i];
			newOutfit.outfitID = msg.getU16();
			newOutfit.outfitName = msg.getString();
			newOutfit.outfitAddons = msg.getU8();

			Uint8 outfitLocked = 0;
			Uint32 offerId = 0;
			if(g_clientVersion >= 1185)
			{
				outfitLocked = msg.getU8();
				if(outfitLocked == 1)
					offerId = msg.getU32();
			}
		}
	}
	else
	{
		Uint16 outfitsStart, outfitsEnd;
		if(g_game.hasGameFeature(GAME_FEATURE_LOOKTYPE_U16))
		{
			outfitsStart = msg.getU16();
			outfitsEnd = msg.getU16();
		}
		else
		{
			outfitsStart = SDL_static_cast(Uint16, msg.getU8());
			outfitsEnd = SDL_static_cast(Uint16, msg.getU8());
		}
		for(Uint16 i = outfitsStart; i <= outfitsEnd; ++i)
		{
			OutfitDetail newOutfit;
			newOutfit.outfitID = i;
			newOutfit.outfitName = "Your Character";
			newOutfit.outfitAddons = 0;
			outfits.push_back(newOutfit);
		}
	}

	if(g_game.hasGameFeature(GAME_FEATURE_MOUNTS))
	{
		Uint16 mountsCount;
		if(g_clientVersion >= 1185)
			mountsCount = msg.getU16();
		else
			mountsCount = SDL_static_cast(Uint16, msg.getU8());

		mounts.resize(SDL_static_cast(size_t, mountsCount));
		for(Uint16 i = 0; i < mountsCount; ++i)
		{
			MountDetail& newMount = mounts[i];
			newMount.mountID = msg.getU16();
			newMount.mountName = msg.getString();

			Uint8 outfitLocked = 0;
			Uint32 offerId = 0;
			if(g_clientVersion >= 1185)
			{
				outfitLocked = msg.getU8();
				if(outfitLocked == 1)
					offerId = msg.getU32();
			}
		}
	}
	if(g_clientVersion >= 1185)
	{
		Uint8 tryOutfit = msg.getU8();
		bool mounted = msg.getBool();
		(void)mounted;
		if(tryOutfit == 3)
		{
			msg.getU32();//??
		}
		else if(tryOutfit == 4)
		{
			Uint16 size = msg.getU16();
			for(Uint16 i = 0; i < size; ++i)
			{
				msg.getU16();//??
				msg.getU16();//??
			}
		}
	}
	g_game.processOutfits(lookType, lookHead, lookBody, lookLegs, lookFeet, lookAddons, lookMount, outfits, mounts);
}

void ProtocolGame::parseExivaSuppressed(InputMessage&)
{
	//Don't have any data
}

void ProtocolGame::parseUpdateExivaOptions(InputMessage& msg)
{
	msg.getBool();//??
	msg.getBool();//??
	msg.getBool();//??
	msg.getBool();//??
	msg.getBool();//??
	msg.getBool();//??

	Uint16 count = msg.getU16();
	for(Uint16 i = 0; i < count; ++i)
		msg.getRawString();//??

	count = msg.getU16();
	for(Uint16 i = 0; i < count; ++i)
		msg.getRawString();//??

	count = msg.getU16();
	for(Uint16 i = 0; i < count; ++i)
		msg.getRawString();//??

	count = msg.getU16();
	for(Uint16 i = 0; i < count; ++i)
		msg.getRawString();//??
}

void ProtocolGame::parseTutorialHint(InputMessage& msg)
{
	Uint8 tutorialId = msg.getU8();
	g_game.processTutorialHint(tutorialId);
}

void ProtocolGame::parseAutomapFlag(InputMessage& msg)
{
	Position pos = msg.getPosition();
	Uint8 icon = msg.getU8();
	const std::string description = msg.getString();

	bool remove = false;
	if(g_game.hasGameFeature(GAME_FEATURE_MINIMAP_REMOVEMARK))
		remove = msg.getBool();

	(void)pos;
	(void)icon;
	(void)description;
	(void)remove;
}

void ProtocolGame::parseModalWindow(InputMessage& msg)
{
	Uint32 windowId = msg.getU32();
	const std::string title = msg.getString();
	const std::string message = msg.getString();
	(void)windowId;
	(void)title;
	(void)message;

	Uint8 buttons = msg.getU8();
	for(Uint8 i = 0; i < buttons; ++i)
	{
		const std::string buttonText = msg.getString();
		Uint8 buttonId = msg.getU8();
		(void)buttonText;
		(void)buttonId;
	}

	Uint8 choices = msg.getU8();
	for(Uint8 i = 0; i < choices; ++i)
	{
		const std::string choiceText = msg.getString();
		Uint8 choiceId = msg.getU8();
		(void)choiceText;
		(void)choiceId;
	}

	Uint8 enterButton, escapeButton;
	if(g_clientVersion > 970)
	{
		escapeButton = msg.getU8();
		enterButton = msg.getU8();
	}
	else
	{
		enterButton = msg.getU8();
		escapeButton = msg.getU8();
	}
	(void)enterButton;
	(void)escapeButton;

	bool priority = msg.getBool();
	(void)priority;
}

void ProtocolGame::parseShowMessageDialog(InputMessage& msg)
{
	Uint8 messageType = msg.getU8();
	const std::string message = msg.getString();
	(void)messageType;
	(void)message;
}

void ProtocolGame::parseOfferDescription(InputMessage& msg)
{
	msg.getU32();//??
	msg.getRawString();//??
}

void ProtocolGame::parseEditGuildMessage(InputMessage& msg)
{
	msg.getRawString();//??
}

void ProtocolGame::parsePlayerDataBasic(InputMessage& msg)
{
	bool premium = msg.getBool();
	if(g_game.hasGameFeature(GAME_FEATURE_PREMIUM_EXPIRATION))
		msg.getU32();//Premium expiration timestamp used for premium advertisement

	Uint8 vocation = msg.getU8();
	bool promoted = (vocation >= 10);
	vocation %= 10;
	if(g_clientVersion >= 1100)
	{
		bool hasReachedMain = msg.getBool();
		(void)hasReachedMain;
	}

	Uint16 spellCount = msg.getU16();
	for(Uint16 i = 0; i < spellCount; ++i)
		msg.getU8();//Spellid

	(void)premium;
	(void)promoted;
	(void)vocation;
}

void ProtocolGame::parsePlayerData(InputMessage& msg)
{
	Uint32 health, maxHealth;
	if(g_game.hasGameFeature(GAME_FEATURE_DOUBLE_HEALTH))
	{
		health = msg.getU32();
		maxHealth = msg.getU32();
	}
	else
	{
		health = SDL_static_cast(Uint32, msg.getU16());
		maxHealth = SDL_static_cast(Uint32, msg.getU16());
	}

	double capacity;
	if(g_game.hasGameFeature(GAME_FEATURE_DOUBLE_CAPACITY))
		capacity = msg.getU32() / 100.0;
	else
		capacity = msg.getU16() / 100.0;

	double totalCapacity = 0.0;
	if(g_game.hasGameFeature(GAME_FEATURE_TOTAL_CAPACITY))
		totalCapacity = msg.getU32() / 100.0;

	Uint64 experience;
	if(g_game.hasGameFeature(GAME_FEATURE_DOUBLE_EXPERIENCE))
		experience = msg.getU64();
	else
		experience = SDL_static_cast(Uint64, msg.getU32());

	Uint16 level = msg.getU16();
	Uint8 levelPercent = msg.getU8();
	if(g_game.hasGameFeature(GAME_FEATURE_EXPERIENCE_BONUS))
	{
		if(g_game.hasGameFeature(GAME_FEATURE_DETAILED_EXPERIENCE_BONUS))
		{
			Uint16 baseXpGain = msg.getU16();
			Uint16 voucherXpGain = 0;
			if(g_clientVersion < 1150)
				voucherXpGain = msg.getU16();
			Uint16 grindingXpGain = msg.getU16();
			Uint16 storeBoostXpGain = msg.getU16();
			Uint16 huntingBoostXpGain = msg.getU16();
			g_game.setPlayerExpBonus(baseXpGain, voucherXpGain, grindingXpGain, storeBoostXpGain, huntingBoostXpGain);
		}
		else
		{
			double expBonus = msg.getDouble();
			g_game.setPlayerExpBonus(expBonus);
		}
	}

	Uint32 mana, maxMana;
	if(g_game.hasGameFeature(GAME_FEATURE_DOUBLE_HEALTH))
	{
		mana = msg.getU32();
		maxMana = msg.getU32();
	}
	else
	{
		mana = SDL_static_cast(Uint32, msg.getU16());
		maxMana = SDL_static_cast(Uint32, msg.getU16());
	}

	if(g_clientVersion < 1200)
	{
		Uint16 magicLevel = SDL_static_cast(Uint16, msg.getU8());
		Uint16 baseMagicLevel = magicLevel;
		if(g_game.hasGameFeature(GAME_FEATURE_BASE_SKILLS))
			baseMagicLevel = SDL_static_cast(Uint16, msg.getU8());

		Uint8 magicLevelPercent = msg.getU8();
		g_game.setPlayerMagicLevel(magicLevel, baseMagicLevel, magicLevelPercent);
	}

	Uint8 soul = msg.getU8();
	Uint16 stamina = 0;
	if(g_game.hasGameFeature(GAME_FEATURE_STAMINA))
		stamina = msg.getU16();

	Uint16 baseSpeed = 0;
	if(g_game.hasGameFeature(GAME_FEATURE_BASE_SKILLS))
		baseSpeed = msg.getU16();

	Uint16 regeneration = 0;
	if(g_game.hasGameFeature(GAME_FEATURE_REGENERATION_TIME))
		regeneration = msg.getU16();

	Uint16 training = 0;
	if(g_game.hasGameFeature(GAME_FEATURE_OFFLINE_TRAINING))
		training = msg.getU16();

	if(g_game.hasGameFeature(GAME_FEATURE_DETAILED_EXPERIENCE_BONUS))
	{
		//Since we don't display xp boost store button we don't need these variables
		Uint16 remainingStoreXpBoostSeconds = msg.getU16();
		bool canBuyMoreStoreXpBoosts = msg.getBool();
		(void)remainingStoreXpBoostSeconds;
		(void)canBuyMoreStoreXpBoosts;
	}
	g_game.setPlayerExperience(experience);
	g_game.setPlayerCapacity(capacity, totalCapacity);
	g_game.setPlayerHealth(health, maxHealth);
	g_game.setPlayerMana(mana, maxMana);
	g_game.setPlayerLevel(level, levelPercent);
	g_game.setPlayerStamina(stamina);
	g_game.setPlayerSoul(soul);
	g_game.setPlayerBaseSpeed(baseSpeed);
	g_game.setPlayerRegeneration(regeneration);
	g_game.setPlayerOfflineTraining(training);

	UTIL_updateHealthPanel();
	UTIL_updateSkillsWindowStats();
	g_game.resetCachedStats();
}

void ProtocolGame::parsePlayerSkills(InputMessage& msg)
{
	Sint32 skills = Skills_LastSkill;
	if(g_game.hasGameFeature(GAME_FEATURE_ADDITIONAL_SKILLS))
		skills = Skills_LastAdditionalSkill;

	for(Sint32 skill = (g_clientVersion >= 1200 ? -1 : 0); skill < skills; ++skill)
	{
		Uint16 level;
		if(g_game.hasGameFeature(GAME_FEATURE_DOUBLE_SKILLS))
			level = msg.getU16();
		else
			level = SDL_static_cast(Uint16, msg.getU8());
		
		Uint16 baseLevel = level;
		if(g_game.hasGameFeature(GAME_FEATURE_BASE_SKILLS))
		{
			if(g_game.hasGameFeature(GAME_FEATURE_DOUBLE_SKILLS))
				baseLevel = msg.getU16();
			else
				baseLevel = SDL_static_cast(Uint16, msg.getU8());
		}

		Uint8 levelPercent = 0;
		if(skill <= Skills_Fishing)
		{
			if(g_game.hasGameFeature(GAME_FEATURE_DOUBLE_PERCENT_SKILLS))
			{
				msg.getU16();//loyalty bonus
				levelPercent = SDL_static_cast(Uint8, msg.getU16() / 100);
			}
			else
				levelPercent = msg.getU8();
		}

		if(skill == -1)
			g_game.setPlayerMagicLevel(level, baseLevel, levelPercent);
		else
			g_game.setPlayerSkill(SDL_static_cast(Skills, skill), level, baseLevel, levelPercent);
	}

	if(g_clientVersion >= 1150)
	{
		double totalCapacity = msg.getU32() / 100.0;
		msg.getU32();
		g_game.setPlayerCapacity(g_game.getPlayerCapacity(), totalCapacity);
		UTIL_updateSkillsWindowStats();
		g_game.resetCachedStats();
	}

	UTIL_updateSkillsWindowSkills();
	g_game.resetCachedSkills();
}

void ProtocolGame::parsePlayerIcons(InputMessage& msg)
{
	Uint32 icons;
	if(g_game.hasGameFeature(GAME_FEATURE_PLAYERICONS_U32))
		icons = msg.getU32();
	else if(g_game.hasGameFeature(GAME_FEATURE_PLAYERICONS_U16))
		icons = SDL_static_cast(Uint32, msg.getU16());
	else
		icons = SDL_static_cast(Uint32, msg.getU8());

	g_game.setIcons(icons);
}

void ProtocolGame::parsePlayerCancelTarget(InputMessage& msg)
{
	Uint32 sequence = 0;
	if(g_game.hasGameFeature(GAME_FEATURE_ATTACK_SEQUENCE))
		sequence = msg.getU32();

	g_game.processCancelTarget(sequence);
}

void ProtocolGame::parsePlayerSpellDelay(InputMessage& msg)
{
	Uint8 spellId = msg.getU8();
	Uint32 delay = msg.getU32();
	g_game.processSpellDelay(spellId, delay);
}

void ProtocolGame::parsePlayerSpellGroupDelay(InputMessage& msg)
{
	Uint8 groupId = msg.getU8();
	Uint32 delay = msg.getU32();
	g_game.processSpellGroupDelay(groupId, delay);
}

void ProtocolGame::parsePlayerMultiUseDelay(InputMessage& msg)
{
	Uint32 delay = msg.getU32();
	g_game.processMultiUseDelay(delay);
}

void ProtocolGame::parsePlayerModes(InputMessage& msg)
{
	Uint8 fightMode = msg.getU8();
	Uint8 chaseMode = msg.getU8();
	bool safeMode = msg.getBool();

	Uint8 pvpMode = 0;
	if(g_game.hasGameFeature(GAME_FEATURE_PVP_MODE))
		pvpMode = msg.getU8();

	g_game.processPlayerModes(fightMode, chaseMode, safeMode, pvpMode);
}

void ProtocolGame::parsePlayerStoreDeepLink(InputMessage& msg)
{
	Uint8 currentlyFeaturedServiceType = msg.getU8();
	if(g_clientVersion >= 1200)
	{
		switch(currentlyFeaturedServiceType)
		{
			case 1:
			case 3:
			{
				msg.getU8();//??
				msg.getU8();//??
				msg.getU8();//??
			}
			break;
			case 2:
			{
				msg.getRawString();//??
				msg.getRawString();//??
				msg.getU8();//??
				msg.getU8();//??
			}
			break;
			case 4:
			{
				msg.getU32();//??
				msg.getU8();//??
				msg.getU8();//??
			}
			break;
			case 5:
			{
				msg.getRawString();//??
				msg.getU8();//??
				msg.getU8();//??
			}
			break;
			default:
			{
				msg.getU8();//??
				msg.getU8();//??
			}
			break;
		}
	}
}

void ProtocolGame::parsePlayerTalk(InputMessage& msg)
{
	Uint32 statementId = 0;
	if(g_game.hasGameFeature(GAME_FEATURE_MESSAGE_STATEMENT))
		statementId = msg.getU32();

	const std::string playerName = UTIL_formatCreatureName(msg.getString());

	Uint16 level = 0;
	if(g_game.hasGameFeature(GAME_FEATURE_MESSAGE_LEVEL))
		level = msg.getU16();

	MessageMode talkMode = translateMessageModeFromServer(msg.getU8());
	Uint32 channelId = 0;
	Position pos;
	switch(talkMode)
	{
		case MessageSay:
		case MessageWhisper:
		case MessageYell:
		case MessageMonsterSay:
		case MessageMonsterYell:
		case MessageBarkLow:
		case MessageBarkLoud:
		case MessageSpell:
		case MessageNpcFromStartBlock:
			pos = msg.getPosition();
			break;
		case MessageChannel:
		case MessageChannelManagement:
		case MessageChannelHighlight:
		case MessageGamemasterChannel:
			channelId = SDL_static_cast(Uint32, msg.getU16());
			break;
		case MessageNpcFrom:
		case MessagePrivateFrom:
		case MessageGamemasterBroadcast:
		case MessageGamemasterPrivateFrom:
		case MessageRVRAnswer:
		case MessageRVRContinue:
			break;
		case MessageRVRChannel:
			channelId = (g_clientVersion >= 713 ? msg.getU32() : 0);
			break;
		default:
			talkMode = MessageInvalid;
			break;
	}

	std::string text = msg.getString();
	if(talkMode != MessageInvalid)
		g_game.processTalk(playerName, statementId, level, talkMode, text, channelId, pos);
}

void ProtocolGame::parseItemInfo(InputMessage& msg)
{
	Uint8 size = msg.getU8();
	for(Uint8 i = 0; i < size; ++i)
	{
		Uint16 itemId = msg.getU16();
		Uint8 countOrSubtype = msg.getU8();
		const std::string description = msg.getString();
		(void)itemId;
		(void)countOrSubtype;
		(void)description;
	}
}

void ProtocolGame::parsePlayerInventory(InputMessage& msg)
{
	Uint16 size = msg.getU16();
	for(Uint16 i = 0; i < size; ++i)
	{
		Uint16 itemId = msg.getU16();
		Uint8 subtype = msg.getU8();
		Uint16 count = msg.getU16();
		(void)itemId;
		(void)subtype;
		(void)count;
	}
}

void ProtocolGame::parseRestingAreaState(InputMessage& msg)
{
	bool inRestingArea = msg.getBool();
	bool restingBonusActive = msg.getBool();
	const std::string description = msg.getString();
	(void)inRestingArea;
	(void)restingBonusActive;
	(void)description;
}

void ProtocolGame::parseChannels(InputMessage& msg)
{
	if(m_expectChannels)
	{
		std::vector<ChannelDetail> channels;
		Uint8 count = msg.getU8();
		channels.resize(SDL_static_cast(size_t, count));
		for(Uint8 i = 0; i < count; ++i)
		{
			ChannelDetail& newChannel = channels[i];
			newChannel.channelId = msg.getU16();
			newChannel.channelName = msg.getString();
		}
		g_game.processChannelList(channels);
		--m_expectChannels;
	}
	else
	{
		//Our emulated ping for older versions
		if(++m_pingReceived == m_pingSent)
			g_ping = SDL_static_cast(Uint16, SDL_GetTicks()-m_pingTime);

		Uint8 count = msg.getU8();
		for(Uint8 i = 0; i < count; ++i)
		{
			msg.getU16();
			msg.getRawString();
		}
	}
}

void ProtocolGame::parseOpenChannel(InputMessage& msg)
{
	Uint16 channelId = msg.getU16();
	const std::string name = msg.getString();
	if(g_game.hasGameFeature(GAME_FEATURE_CHAT_PLAYERLIST))
	{
		Uint16 joinedPlayers = msg.getU16();
		for(Uint16 i = 0; i < joinedPlayers; ++i)
			msg.getRawString(); //Player Name

		Uint16 invitedPlayers = msg.getU16();
		for(Uint16 j = 0; j < invitedPlayers; ++j)
			msg.getRawString(); //Player Name
	}
	g_game.processOpenChannel(channelId, name);
}

void ProtocolGame::parseOpenPrivateChannel(InputMessage& msg)
{
	const std::string name = UTIL_formatCreatureName(msg.getString());
	g_game.processOpenPrivateChannel(name);
}

void ProtocolGame::parseRuleViolationChannel(InputMessage& msg)
{
	Uint16 channelId = msg.getU16();
	(void)channelId;
}

void ProtocolGame::parseRuleViolationRemove(InputMessage& msg)
{
	const std::string name = msg.getString();
	(void)name;
}

void ProtocolGame::parseRuleViolationCancel(InputMessage& msg)
{
	const std::string name = msg.getString();
	(void)name;
}

void ProtocolGame::parseRuleViolationLock(InputMessage&)
{

}

void ProtocolGame::parseOpenOwnChannel(InputMessage& msg)
{
	Uint16 channelId = msg.getU16();
	const std::string name = msg.getString();
	if(g_game.hasGameFeature(GAME_FEATURE_CHAT_PLAYERLIST))
	{
		Uint16 joinedPlayers = msg.getU16();
		for(Uint16 i = 0; i < joinedPlayers; ++i)
			msg.getRawString(); //Player Name

		Uint16 invitedPlayers = msg.getU16();
		for(Uint16 j = 0; j < invitedPlayers; ++j)
			msg.getRawString(); //Player Name
	}
	g_game.processOpenOwnPrivateChannel(channelId, name);
}

void ProtocolGame::parseCloseChannel(InputMessage& msg)
{
	Uint16 channelId = msg.getU16();
	g_game.processCloseChannel(channelId);
}

void ProtocolGame::parseChannelEvent(InputMessage& msg)
{
	Uint16 channelId = msg.getU16();
	const std::string playerName = msg.getString();
	Uint8 eventType = msg.getU8();
	(void)channelId;
	(void)playerName;
	(void)eventType;
}

void ProtocolGame::parseVipAdd(InputMessage& msg)
{
	Uint32 iconId = 0;
	bool notifyLogin = false;
	Uint32 playerGUID = msg.getU32();
	const std::string playerName = UTIL_formatCreatureName(msg.getString());
	std::string description = "";
	if(g_game.hasGameFeature(GAME_FEATURE_ADDITIONAL_VIPINFO))
	{
		description = msg.getString();
		iconId = msg.getU32();
		notifyLogin = msg.getBool();
	}
	Uint8 status = msg.getU8();
	if(g_game.hasGameFeature(GAME_FEATURE_VIP_GROUPS))
	{
		Uint8 groupCount = msg.getU8();
		for(Uint8 i = 0; i < groupCount; ++i)
		{
			Uint8 groupId = msg.getU8();
			(void)groupId;
		}
	}
	(void)iconId;
	(void)notifyLogin;
	(void)playerGUID;
	(void)playerName;
	(void)description;
	(void)status;
}

void ProtocolGame::parseVipStatus(InputMessage& msg)
{
	Uint32 playerGUID = msg.getU32();
	if(g_game.hasGameFeature(GAME_FEATURE_VIP_STATUS))
	{
		Uint8 status = msg.getU8();
		(void)status;
	}
	else
		(void)playerGUID;
}

void ProtocolGame::parseVipStatusLogout(InputMessage& msg)
{
	if(g_game.hasGameFeature(GAME_FEATURE_VIP_GROUPS))
	{
		Uint8 groupCount = msg.getU8();
		for(Uint8 i = 0; i < groupCount; ++i)
		{
			Uint8 groupId = msg.getU8();
			std::string groupName = msg.getString();
			bool groupSomething = msg.getBool();
			(void)groupId;
			(void)groupName;
			(void)groupSomething;
		}

		Uint8 groupAllowedCount = msg.getU8();
		(void)groupAllowedCount;
	}
	else
	{
		Uint32 playerGUID = msg.getU32();
		(void)playerGUID;
	}
}

void ProtocolGame::parseFriendSystemData(InputMessage& msg)
{
	Uint8 type = msg.getU8();
	switch(type)
	{
		case 0:
		{
			Uint8 something = msg.getU8();//??
			if(something != 1)
				msg.getRawString();//??
		}
		break;
		case 1:
		{
			//no payload
		}
		break;
		case 2:
		{
			Uint16 size = msg.getU16();
			for(Uint16 i = 0; i < size; ++i)
			{
				msg.getU32();//??
				msg.getU32();//??
				msg.getRawString();//??
				msg.getRawString();//??
				msg.getRawString();//??
				msg.getRawString();//??
				msg.getU16();//??
				msg.getU8();//??
				msg.getU32();//??
			}
		}
		break;
		case 3:
		{
			Uint16 size = msg.getU16();
			for(Uint16 i = 0; i < size; ++i)
			{
				msg.getBool();//??
				msg.getU32();//??
				msg.getRawString();//??
				msg.getRawString();//??
				msg.getRawString();//??
				msg.getU32();//??
			}
		}
		break;
		case 4:
		{
			Uint16 size = msg.getU16();
			for(Uint16 i = 0; i < size; ++i)
			{
				msg.getU32();//??
				msg.getRawString();//??
				msg.getU32();//??
			}
		}
		break;
		case 5:
		{
			msg.getU32();//??
			msg.getBool();//??
			msg.getBool();//??
			bool something = msg.getBool();
			if(something)
			{
				msg.getBool();//??
				msg.getU32();//??
				msg.getBool();//??
				msg.getRawString();//??

				Uint8 size = msg.getU8();
				for(Uint8 i = 0; i < size; ++i)
				{
					msg.getU32();//??
					msg.getRawString();//??
				}
			}

			Uint8 size = msg.getU8();
			for(Uint8 i = 0; i < size; ++i)
			{
				msg.getU32();//??
				msg.getRawString();//??
				msg.getBool();//??
				msg.getRawString();//??
				msg.getRawString();//??
				msg.getRawString();//??
				msg.getU16();//??
				msg.getBool();//??

				Uint16 lookType;
				if(g_game.hasGameFeature(GAME_FEATURE_LOOKTYPE_U16))
					lookType = msg.getU16();
				else
					lookType = SDL_static_cast(Uint16, msg.getU8());

				Uint16 lookTypeEx = 0;
				Uint8 lookHead = 0;
				Uint8 lookBody = 0;
				Uint8 lookLegs = 0;
				Uint8 lookFeet = 0;
				Uint8 lookAddons = 0;
				if(lookType != 0)
				{
					lookHead = msg.getU8();
					lookBody = msg.getU8();
					lookLegs = msg.getU8();
					lookFeet = msg.getU8();
					if(g_game.hasGameFeature(GAME_FEATURE_ADDONS))
						lookAddons = msg.getU8();
				}
				else
					lookTypeEx = msg.getU16();

				(void)lookType;
				(void)lookTypeEx;
				(void)lookHead;
				(void)lookBody;
				(void)lookLegs;
				(void)lookFeet;
				(void)lookAddons;
			}
		}
		break;
		case 6:
		{
			Uint8 size = msg.getU8();
			for(Uint8 i = 0; i < size; ++i)
			{
				msg.getU32();//??
				msg.getRawString();//??
				msg.getRawString();//??
				msg.getBool();//??
				msg.getBool();//??
			}
		}
		break;
		case 7:
		{
			//no payload
		}
		break;
		case 8:
		{
			Uint8 size = msg.getU8();
			for(Uint8 i = 0; i < size; ++i)
			{
				msg.getU8();//??
				msg.getBool();//??
				msg.getBool();//??
				msg.getBool();//??
			}
		}
		break;
	}
}

void ProtocolGame::parseQuestLog(InputMessage& msg)
{
	std::vector<QuestLogDetail> questLogElements;
	Uint16 questsCount = msg.getU16();
	questLogElements.resize(SDL_static_cast(size_t, questsCount));
	for(Uint16 i = 0; i < questsCount; ++i)
	{
		QuestLogDetail& newQuestLogElement = questLogElements[i];
		newQuestLogElement.questId = msg.getU16();
		newQuestLogElement.questName = msg.getString();
		newQuestLogElement.questCompleted = msg.getBool();
	}
	(void)questLogElements;
}

void ProtocolGame::parseQuestLine(InputMessage& msg)
{
	std::vector<QuestLineDetail> questLineElements;
	Uint16 questId = msg.getU16();
	Uint8 missionCount = msg.getU8();
	questLineElements.resize(SDL_static_cast(size_t, missionCount));
	for(Uint8 i = 0; i < missionCount; ++i)
	{
		QuestLineDetail& newQuestLineElement = questLineElements[i];
		if(g_clientVersion >= 1121)
			msg.getU16();//MissionId
		newQuestLineElement.missionName = msg.getString();
		newQuestLineElement.missionDescription = msg.getString();
	}
	(void)questId;
	(void)questLineElements;
}

void ProtocolGame::parseTrackedQuestFlags(InputMessage& msg)
{
	bool full = msg.getBool();
	if(full)
	{
		msg.getU8();
		Uint8 size = msg.getU8();
		for(Uint8 i = 0; i < size; ++i)
		{
			msg.getU16();
			msg.getRawString();
			msg.getRawString();
		}
	}
	else
	{
		msg.getU16();
		msg.getRawString();
		msg.getRawString();
	}
}

void ProtocolGame::parseMarketStatistics(InputMessage& msg)
{
	Uint16 count = msg.getU16();
	for(Uint16 i = 0; i < count; ++i)
	{
		Uint16 itemId = msg.getU16();
		Uint32 itemPrice = msg.getU32();
		(void)itemId;
		(void)itemPrice;
	}
}

void ProtocolGame::parseMarketEnter(InputMessage& msg)
{
	Uint64 balance;
	if(g_clientVersion >= 981)
		balance = msg.getU64();
	else
		balance = SDL_static_cast(Uint64, msg.getU32());

	Sint16 vocation;
	if(g_clientVersion < 950)
		vocation = SDL_static_cast(Sint16, SDL_static_cast(Sint8, msg.getU8()));
	else
		vocation = -1;

	(void)balance;
	(void)vocation;

	Uint8 offers = msg.getU8();
	Uint16 depotItemsCount = msg.getU16();
	for(Uint16 i = 0; i < depotItemsCount; ++i)
	{
		Uint16 itemId = msg.getU16();
		Uint16 itemCount = msg.getU16();
		(void)itemId;
		(void)itemCount;
	}
	(void)offers;
}

void ProtocolGame::parseMarketLeave(InputMessage&)
{

}

void ProtocolGame::parseMarketDetail(InputMessage& msg)
{
	Uint16 itemId = msg.getU16();

	const std::string armor = msg.getString();
	const std::string attack = msg.getString();
	const std::string container = msg.getString();
	const std::string defense = msg.getString();
	const std::string description = msg.getString();
	const std::string decayTime = msg.getString();
	const std::string absorb = msg.getString();
	const std::string reqlvl = msg.getString();
	const std::string reqmaglvl = msg.getString();
	const std::string vocation = msg.getString();
	const std::string runespellname = msg.getString();
	const std::string stats = msg.getString();
	const std::string charges = msg.getString();
	const std::string weaponName = msg.getString();
	const std::string weight = msg.getString();
	if(g_game.hasGameFeature(GAME_FEATURE_IMBUING))
	{
		const std::string imbuements = msg.getString();
		(void)imbuements;
	}

	(void)itemId;
	(void)armor;
	(void)attack;
	(void)container;
	(void)defense;
	(void)description;
	(void)decayTime;
	(void)absorb;
	(void)reqlvl;
	(void)reqmaglvl;
	(void)vocation;
	(void)runespellname;
	(void)stats;
	(void)charges;
	(void)weaponName;
	(void)weight;

	Uint8 count = msg.getU8();
	for(Uint8 i = 0; i < count; ++i)
	{
		Uint32 transactions = msg.getU32();
		Uint32 totalPrice = msg.getU32();
		Uint32 highestPrice = msg.getU32();
		Uint32 lowestPrice = msg.getU32();
		(void)transactions;
		(void)totalPrice;
		(void)highestPrice;
		(void)lowestPrice;
	}

	count = msg.getU8();
	for(Uint8 i = 0; i < count; ++i)
	{
		Uint32 transactions = msg.getU32();
		Uint32 totalPrice = msg.getU32();
		Uint32 highestPrice = msg.getU32();
		Uint32 lowestPrice = msg.getU32();
		(void)transactions;
		(void)totalPrice;
		(void)highestPrice;
		(void)lowestPrice;
	}
}

void ProtocolGame::parseMarketBrowse(InputMessage& msg)
{
	Uint16 request = msg.getU16();
	Uint32 buyOfferCount = msg.getU32();
	for(Uint32 i = 0; i < buyOfferCount; ++i)
	{
		Uint32 timestamp = msg.getU32();
		Uint16 counter = msg.getU16();

		Uint16 itemId;
		if(request == MARKETREQUEST_OWN_OFFERS || request == MARKETREQUEST_OWN_HISTORY)
			itemId = msg.getU16();
		else
			itemId = request;

		Uint16 amount = msg.getU16();
		Uint32 price = msg.getU32();

		std::string playerName = "";
		Uint8 state = OFFERSTATE_ACTIVE;
		if(request == MARKETREQUEST_OWN_HISTORY)
			state = msg.getU8();
		else if(request != MARKETREQUEST_OWN_OFFERS)
			playerName = msg.getString();

		(void)timestamp;
		(void)counter;
		(void)itemId;
		(void)amount;
		(void)price;
		(void)playerName;
		(void)state;
	}

	Uint32 sellOfferCount = msg.getU32();
	for(Uint32 i = 0; i < sellOfferCount; ++i)
	{
		Uint32 timestamp = msg.getU32();
		Uint16 counter = msg.getU16();

		Uint16 itemId;
		if(request == MARKETREQUEST_OWN_OFFERS || request == MARKETREQUEST_OWN_HISTORY)
			itemId = msg.getU16();
		else
			itemId = request;

		Uint16 amount = msg.getU16();
		Uint32 price = msg.getU32();

		std::string playerName = "";
		Uint8 state = OFFERSTATE_ACTIVE;
		if(request == MARKETREQUEST_OWN_HISTORY)
			state = msg.getU8();
		else if(request != MARKETREQUEST_OWN_OFFERS)
			playerName = msg.getString();

		(void)timestamp;
		(void)counter;
		(void)itemId;
		(void)amount;
		(void)price;
		(void)playerName;
		(void)state;
	}
}

void ProtocolGame::parseStoreCoinBalance(InputMessage& msg)
{
	bool update = msg.getBool();
	Uint32 coins = SDL_static_cast(Uint32, -1);
	Uint32 transferableCoins = SDL_static_cast(Uint32, -1);
	if(update)
	{
		coins = msg.getU32();
		transferableCoins = msg.getU32();
		if(g_game.hasGameFeature(GAME_FEATURE_TOURNAMENTS))
		{
			Uint32 tournamentCoins = msg.getU32();
			(void)tournamentCoins;
		}
	}
	(void)update;
	(void)coins;
	(void)transferableCoins;
}

void ProtocolGame::parseStoreError(InputMessage& msg)
{
	Uint8 errorType = msg.getU8();
	const std::string message = msg.getString();
	(void)errorType;
	(void)message;
}

void ProtocolGame::parseStoreRequestPurchase(InputMessage& msg)
{
	Uint32 transactionId = msg.getU32();
	Uint8 productType = msg.getU8();
	switch(productType)
	{
		case 9:
		{
			msg.getBool();//??
			Uint8 size = msg.getU8();
			for(Uint8 i = 0; i < size; ++i)
			{
				msg.getRawString();//??
				msg.getBool();//??
				msg.getBool();//??
				msg.getU8();//??
			}

			msg.getBool();//??
			msg.getBool();//??
			msg.getBool();//??
			msg.getBool();//??
			msg.getBool();//??
		}
		break;
		case 11:
		{
			msg.getRawString();//??
			Uint8 size = msg.getU8();
			for(Uint8 i = 0; i < size; ++i)
			{
				msg.getRawString();//??
			}

			size = msg.getU8();
			for(Uint8 i = 0; i < size; ++i)
			{
				msg.getU8();//??
			}

			size = msg.getU8();
			for(Uint8 i = 0; i < size; ++i)
			{
				msg.getRawString();//??
			}
		}
		break;
		case 12:
		{
			Uint8 size = msg.getU8();
			for(Uint8 i = 0; i < size; ++i)
			{
				msg.getRawString();//??
				msg.getRawString();//??
				msg.getRawString();//??
				msg.getU16();//??
			}
		}
		break;
	}
	(void)transactionId;
}

void ProtocolGame::parseStoreCoinBalanceUpdating(InputMessage& msg)
{
	bool isUpdating = msg.getBool();
	(void)isUpdating;
}

void ProtocolGame::parseStoreOpen(InputMessage& msg)
{
	if(g_clientVersion < 1180)
		parseStoreCoinBalance(msg);

	Uint16 categories = msg.getU16();
	for(Uint16 i = 0; i < categories; ++i)
	{
		const std::string category = msg.getString();
		std::string description;
		if(g_clientVersion < 1180)
			description = msg.getString();

		(void)category;
		(void)description;

		Uint8 highlightState = 0;
		if(g_game.hasGameFeature(GAME_FEATURE_STORE_HIGHLIGHTS))
			highlightState = msg.getU8();

		(void)highlightState;

		Uint8 iconCount = msg.getU8();
		for(Uint8 j = 0; j < iconCount; ++j)
		{
			const std::string icon = msg.getString();
			(void)icon;
		}

		const std::string parentCategory = msg.getString();
		(void)parentCategory;
	}
}

void ProtocolGame::parseStoreOffers(InputMessage& msg)
{
	const std::string categoryName = msg.getString();
	if(g_clientVersion >= 1180)
	{
		Uint32 selectedOfferId = msg.getU32();
		Uint8 sortType = msg.getU8();
		Uint8 filters = msg.getU8();
		for(Uint8 i = 0; i < filters; ++i)
		{
			const std::string filter = msg.getString();
			(void)filter;
		}
		(void)selectedOfferId;
		(void)sortType;
		if(g_clientVersion >= 1185)
		{
			Uint16 shownFilters = msg.getU16();
			for(Uint16 i = 0; i < shownFilters; ++i)
			{
				Uint8 filterId = msg.getU8();
				(void)filterId;
			}
		}

		Uint16 offers = msg.getU16();
		for(Uint16 i = 0; i < offers; ++i)
		{
			const std::string offerName = msg.getString();
			(void)offerName;

			Uint8 quantitys = msg.getU8();
			for(Uint8 j = 0; j < quantitys; ++j)
			{
				Uint32 offerId = msg.getU32();
				Uint16 offerCount = msg.getU16();
				Uint32 offerPrice = msg.getU32();
				Uint8 offerCoinType = msg.getU8();
				(void)offerId;
				(void)offerCount;
				(void)offerPrice;
				(void)offerCoinType;

				std::vector<std::string> disabledReasons;
				bool disabled = msg.getBool();
				if(disabled)
				{
					Uint8 errors = msg.getU8();
					disabledReasons.resize(SDL_static_cast(size_t, errors));
					for(Uint8 k = 0; k < errors; ++k)
						disabledReasons[k].assign(msg.getString());
				}
				(void)disabledReasons;

				Uint8 highlightState = msg.getU8();
				if(highlightState == 2)
				{
					Uint32 saleTimestamp = msg.getU32();
					Uint32 basePrice = msg.getU32();
					(void)saleTimestamp;
					(void)basePrice;
				}
			}

			Uint8 offerType = msg.getU8();
			switch(offerType)
			{
				case 0:
				{
					const std::string icon = msg.getString();
					(void)icon;
				}
				break;
				case 1:
				{
					Uint16 lookMount = msg.getU16();
					(void)lookMount;
				}
				break;
				case 2:
				{
					Uint16 lookType;
					if(g_game.hasGameFeature(GAME_FEATURE_LOOKTYPE_U16))
						lookType = msg.getU16();
					else
						lookType = SDL_static_cast(Uint16, msg.getU8());

					Uint8 lookHead = msg.getU8();
					Uint8 lookBody = msg.getU8();
					Uint8 lookLegs = msg.getU8();
					Uint8 lookFeet = msg.getU8();
					(void)lookType;
					(void)lookHead;
					(void)lookBody;
					(void)lookLegs;
					(void)lookFeet;
				}
				break;
				case 3:
				{
					Uint16 itemId = msg.getU16();
					(void)itemId;
				}
				break;
				case 4:
				{
					msg.getU8();//??
					msg.getU16();//??
					msg.getU16();//??
					msg.getU8();//??
					msg.getU8();//??
					msg.getU8();//??
					msg.getU8();//??
				}
				break;
			}
			if(g_clientVersion >= 1212)
			{
				msg.getU8();//??
			}

			const std::string filter = msg.getString();
			Uint16 timesBought = msg.getU16();
			Uint32 timeAddedToStore = msg.getU32();
			bool requiresConfiguration = msg.getBool();
			(void)filter;
			(void)timeAddedToStore;
			(void)timesBought;
			(void)requiresConfiguration;

			Uint16 products = msg.getU16();
			for(Uint16 j = 0; j < products; ++j)
			{
				const std::string productName = msg.getString();
				Uint8 productType = msg.getU8();
				switch(productType)
				{
					case 0:
					{
						const std::string icon = msg.getString();
						(void)icon;
					}
					break;
					case 1:
					{
						Uint16 lookMount = msg.getU16();
						(void)lookMount;
					}
					break;
					case 2:
					{
						Uint16 lookType;
						if(g_game.hasGameFeature(GAME_FEATURE_LOOKTYPE_U16))
							lookType = msg.getU16();
						else
							lookType = SDL_static_cast(Uint16, msg.getU8());

						Uint8 lookHead = msg.getU8();
						Uint8 lookBody = msg.getU8();
						Uint8 lookLegs = msg.getU8();
						Uint8 lookFeet = msg.getU8();
						(void)lookType;
						(void)lookHead;
						(void)lookBody;
						(void)lookLegs;
						(void)lookFeet;
					}
					break;
					case 3:
					{
						Uint16 itemId = msg.getU16();
						(void)itemId;
					}
					break;
					case 4:
					{
						msg.getU8();//??
						msg.getU16();//??
						msg.getU16();//??
						msg.getU8();//??
						msg.getU8();//??
						msg.getU8();//??
						msg.getU8();//??
					}
					break;
				}
			}
		}

		if(categoryName == "Home")
		{
			Uint8 featuredOffers = msg.getU8();
			for(Uint8 i = 0; i < featuredOffers; ++i)
			{
				const std::string icon = msg.getString();
				Uint8 openAction = msg.getU8();
				switch(openAction)
				{
					case 1:
					{
						Uint8 featuredCategoryType = msg.getU8();
						(void)featuredCategoryType;
					}
					break;
					case 2:
					{
						const std::string featuredCategoryName = msg.getString();
						const std::string featuredCategoryFilter = msg.getString();
						(void)featuredCategoryName;
						(void)featuredCategoryFilter;
					}
					break;
					case 3:
					{
						Uint8 featuredOfferType = msg.getU8();
						(void)featuredOfferType;
					}
					break;
					case 4:
					{
						Uint32 featuredOfferId = msg.getU32();
						(void)featuredOfferId;
					}
					break;
					case 5:
					{
						const std::string featuredCategoryName = msg.getString();
						(void)featuredCategoryName;
					}
					break;
				}

				msg.getU8();//??
				msg.getU8();//??
			}

			msg.getU8();//??
		}
		else if(categoryName == "Search")
		{
			msg.getBool();//??
		}
		return;
	}

	Uint16 offers = msg.getU16();
	for(Uint16 i = 0; i < offers; ++i)
	{
		Uint32 offerId = msg.getU32();
		const std::string offerName = msg.getString();
		const std::string offerDescription = msg.getString();
		(void)offerId;
		(void)offerName;
		(void)offerDescription;

		Uint32 price = msg.getU32();
		Uint8 highlightState = msg.getU8();
		if(highlightState == 2 && g_game.hasGameFeature(GAME_FEATURE_STORE_HIGHLIGHTS2))
		{
			Uint32 saleTimestamp = msg.getU32();
			Uint32 basePrice = msg.getU32();
			(void)saleTimestamp;
			(void)basePrice;
		}
		(void)price;

		Uint8 disabledState = msg.getU8();
		std::string disabledReason = "";
		if(g_game.hasGameFeature(GAME_FEATURE_STORE_HIGHLIGHTS) && disabledState == 1)
			disabledReason = msg.getString();

		(void)disabledState;
		(void)disabledReason;

		Uint8 icons = msg.getU8();
		for(Uint8 j = 0; j < icons; ++j)
		{
			const std::string icon = msg.getString();
			(void)icon;
		}

		Uint16 subOffers = msg.getU16();
		for(Uint16 j = 0; j < subOffers; ++j)
		{
			const std::string name = msg.getString();
			const std::string description = msg.getString();
			(void)name;
			(void)description;

			Uint8 subIcons = msg.getU8();
			for(Uint8 k = 0; k < subIcons; ++k)
			{
				const std::string icon = msg.getString();
				(void)icon;
			}

			const std::string serviceType = msg.getString();
			(void)serviceType;
		}
	}
}

void ProtocolGame::parseStoreTransactionHistory(InputMessage& msg)
{
	bool hasNextPage;
	Uint32 pageCount;
	Uint32 currentPage;
	if(g_clientVersion <= 1096)
	{
		currentPage = SDL_static_cast(Uint32, msg.getU16());
		pageCount = 0;//We don't know how many pages we have :)
		hasNextPage = msg.getBool();
	}
	else
	{
		currentPage = msg.getU32();
		pageCount = msg.getU32();
		hasNextPage = false;
	}
	(void)hasNextPage;
	(void)pageCount;
	(void)currentPage;

	Uint8 entries = msg.getU8();
	for(Uint8 i = 0; i < entries; ++i)
	{
		if(g_clientVersion >= 1220)//Detect client version
		{
			msg.getU32();//Offer id?
			msg.getU32();//Timestamp?
			msg.getU8();//Product type?
			msg.getU32();//Coin change?
			msg.getU8();//Coin type?
			msg.getRawString();//Product name?
			msg.getBool();//Details button?
		}
		else
		{
			Uint32 time = msg.getU32();
			Uint8 productType = msg.getU8();
			Uint32 coinChange = msg.getU32();
			const std::string productName = msg.getString();
			(void)time;
			(void)productType;
			(void)coinChange;
			(void)productName;
		}
	}
}

void ProtocolGame::parseStoreCompletePurchase(InputMessage& msg)
{
	Uint8 unknown = msg.getU8();
	(void)unknown;

	const std::string message = msg.getString();
	if(g_clientVersion < 1220)//Detect clientversion
	{
		Uint32 coins = msg.getU32();
		Uint32 transferableCoins = msg.getU32();
		(void)coins;
		(void)transferableCoins;
	}
	(void)message;
}

void ProtocolGame::parseTransactionDetails(InputMessage& msg)
{
	msg.getU32();//??
	Uint8 type = msg.getU8();
	switch(type)
	{
		case 0:
		{
			msg.getU32();//??
			msg.getRawString();//??
			msg.getRawString();//??
			SDL_static_cast(Sint32, msg.getU32());//??
			msg.getU32();//??
			SDL_static_cast(Sint32, msg.getU32());//??
		}
		break;
		case 1:
		{
			msg.getRawString();//??
			msg.getU32();//??
			msg.getU32();//??
			msg.getU32();//??
			msg.getU32();//??
			msg.getU32();//??

			Uint16 size = msg.getU16();
			for(Uint16 i = 0; i < size; ++i)
			{
				msg.getU32();//??
				msg.getRawString();//??
				msg.getU8();//??
				SDL_static_cast(Sint32, msg.getU32());//??
			}
		}
		break;
	}
}

void ProtocolGame::parseImpactTracking(InputMessage& msg)
{
	Uint8 healing = msg.getU8();
	Uint32 damage = msg.getU32();
	(void)healing;
	(void)damage;
}

void ProtocolGame::parseItemWasted(InputMessage& msg)
{
	Uint16 itemId = msg.getU16();
	(void)itemId;
}

void ProtocolGame::parseItemLooted(InputMessage& msg)
{
	ItemUI* item = getItemUI(msg, msg.getU16());
	const std::string itemName = msg.getString();
	(void)item;
	(void)itemName;
}

void ProtocolGame::parseKillTracking(InputMessage& msg)
{;
	const std::string name = msg.getString();
	Uint16 lookType;
	if(g_game.hasGameFeature(GAME_FEATURE_LOOKTYPE_U16))
		lookType = msg.getU16();
	else
		lookType = SDL_static_cast(Uint16, msg.getU8());

	Uint16 lookTypeEx = 0;
	Uint8 lookHead = 0;
	Uint8 lookBody = 0;
	Uint8 lookLegs = 0;
	Uint8 lookFeet = 0;
	Uint8 lookAddons = 0;
	if(lookType != 0)
	{
		lookHead = msg.getU8();
		lookBody = msg.getU8();
		lookLegs = msg.getU8();
		lookFeet = msg.getU8();
		if(g_game.hasGameFeature(GAME_FEATURE_ADDONS))
			lookAddons = msg.getU8();
	}
	else
		lookTypeEx = msg.getU16();

	(void)lookType;
	(void)lookTypeEx;
	(void)lookHead;
	(void)lookBody;
	(void)lookLegs;
	(void)lookFeet;
	(void)lookAddons;

	Uint8 lootCount = msg.getU8();
	for(Uint8 i = 0; i < lootCount; ++i)
	{
		ItemUI* item = getItemUI(msg, msg.getU16());
		(void)item;
	}
	(void)name;
}

void ProtocolGame::parseRefreshImbuingDialog(InputMessage& msg)
{
	msg.getU16();//??
	Uint8 size8 = msg.getU8();
	for(Uint8 i = 0; i < size8; ++i)
	{
		bool something = msg.getBool();
		if(something)
		{
			msg.getU32();//??
			msg.getRawString();//??
			msg.getRawString();//??
			msg.getRawString();//??
			msg.getU16();//??
			msg.getU32();//??
			msg.getBool();//??

			Uint8 size = msg.getU8();
			for(Uint8 j = 0; j < size; ++j)
			{
				msg.getU16();//??
				msg.getRawString();//??
				msg.getU16();//??
			}

			msg.getU32();//??
			msg.getU8();//??
			msg.getU32();//??
			msg.getU32();//??
			msg.getU32();//??
		}
	}

	Uint16 size16 = msg.getU16();
	for(Uint16 i = 0; i < size16; ++i)
	{
		msg.getU32();//??
		msg.getRawString();//??
		msg.getRawString();//??
		msg.getRawString();//??
		msg.getU16();//??
		msg.getU32();//??
		msg.getBool();//??

		Uint8 size = msg.getU8();
		for(Uint8 j = 0; j < size; ++j)
		{
			msg.getU16();//??
			msg.getRawString();//??
			msg.getU16();//??
		}

		msg.getU32();//??
		msg.getU8();//??
		msg.getU32();//??
	}

	Uint32 size32 = msg.getU32();
	for(Uint32 i = 0; i < size32; ++i)
	{
		msg.getU16();//??
		msg.getU16();//??
	}
}

void ProtocolGame::parseCloseImbuingDialog(InputMessage&)
{
}

void ProtocolGame::parseResourceBalance(InputMessage& msg)
{
	Uint8 type = msg.getU8();
	if(type == 30)
	{
		Sint32 difference = SDL_static_cast(Sint32, msg.getU32());
		(void)difference;
	}
	else
	{
		Uint64 balance = msg.getU64();
		(void)balance;
	}
}

void ProtocolGame::parsePreyFreeListRerollAvailability(InputMessage& msg)
{
	msg.getU8();//??
	msg.getU16();//??
}

void ProtocolGame::parsePreyTimeLeft(InputMessage& msg)
{
	msg.getU8();//??
	msg.getU16();//??
}

void ProtocolGame::parsePreyData(InputMessage& msg)
{
	msg.getU8();//??
	Uint8 type = msg.getU8();
	switch(type)
	{
		case 0:
		{
			msg.getU8();//??
		}
		break;
		case 1: break;//no payload
		case 2:
		{
			msg.getRawString();//??
			Uint16 lookType;
			if(g_game.hasGameFeature(GAME_FEATURE_LOOKTYPE_U16))
				lookType = msg.getU16();
			else
				lookType = SDL_static_cast(Uint16, msg.getU8());

			Uint16 lookTypeEx = 0;
			Uint8 lookHead = 0;
			Uint8 lookBody = 0;
			Uint8 lookLegs = 0;
			Uint8 lookFeet = 0;
			Uint8 lookAddons = 0;
			if(lookType != 0)
			{
				lookHead = msg.getU8();
				lookBody = msg.getU8();
				lookLegs = msg.getU8();
				lookFeet = msg.getU8();
				if(g_game.hasGameFeature(GAME_FEATURE_ADDONS))
					lookAddons = msg.getU8();
			}
			else
				lookTypeEx = msg.getU16();

			(void)lookType;
			(void)lookTypeEx;
			(void)lookHead;
			(void)lookBody;
			(void)lookLegs;
			(void)lookFeet;
			(void)lookAddons;

			msg.getU8();//??
			msg.getU16();//??
			msg.getU8();//??
			msg.getU16();//??
		}
		break;
		case 3:
		{
			Uint8 size = msg.getU8();
			for(Uint8 i = 0; i < size; ++i)
			{
				msg.getRawString();//??
				Uint16 lookType;
				if(g_game.hasGameFeature(GAME_FEATURE_LOOKTYPE_U16))
					lookType = msg.getU16();
				else
					lookType = SDL_static_cast(Uint16, msg.getU8());

				Uint16 lookTypeEx = 0;
				Uint8 lookHead = 0;
				Uint8 lookBody = 0;
				Uint8 lookLegs = 0;
				Uint8 lookFeet = 0;
				Uint8 lookAddons = 0;
				if(lookType != 0)
				{
					lookHead = msg.getU8();
					lookBody = msg.getU8();
					lookLegs = msg.getU8();
					lookFeet = msg.getU8();
					if(g_game.hasGameFeature(GAME_FEATURE_ADDONS))
						lookAddons = msg.getU8();
				}
				else
					lookTypeEx = msg.getU16();

				(void)lookType;
				(void)lookTypeEx;
				(void)lookHead;
				(void)lookBody;
				(void)lookLegs;
				(void)lookFeet;
				(void)lookAddons;
			}
		}
		break;
		case 4:
		{
			msg.getU8();//??
			msg.getU16();//??
			msg.getU8();//??

			Uint8 size = msg.getU8();
			for(Uint8 i = 0; i < size; ++i)
			{
				msg.getRawString();//??
				Uint16 lookType;
				if(g_game.hasGameFeature(GAME_FEATURE_LOOKTYPE_U16))
					lookType = msg.getU16();
				else
					lookType = SDL_static_cast(Uint16, msg.getU8());

				Uint16 lookTypeEx = 0;
				Uint8 lookHead = 0;
				Uint8 lookBody = 0;
				Uint8 lookLegs = 0;
				Uint8 lookFeet = 0;
				Uint8 lookAddons = 0;
				if(lookType != 0)
				{
					lookHead = msg.getU8();
					lookBody = msg.getU8();
					lookLegs = msg.getU8();
					lookFeet = msg.getU8();
					if(g_game.hasGameFeature(GAME_FEATURE_ADDONS))
						lookAddons = msg.getU8();
				}
				else
					lookTypeEx = msg.getU16();

				(void)lookType;
				(void)lookTypeEx;
				(void)lookHead;
				(void)lookBody;
				(void)lookLegs;
				(void)lookFeet;
				(void)lookAddons;
			}
		}
		break;
		case 5:
		{
			Uint16 size = msg.getU16();
			for(Uint16 i = 0; i < size; ++i)
			{
				msg.getU16();//??
			}
		}
		break;
		case 6:
		{
			msg.getU8();//??
			msg.getU16();//??
			msg.getU8();//??
			
			Uint16 size = msg.getU16();
			for(Uint16 i = 0; i < size; ++i)
			{
				msg.getU16();//??
			}
		}
		break;
	}

	msg.getU16();//??
	if(g_clientVersion >= 1190)
	{
		msg.getU8();//??
	}
}

void ProtocolGame::parsePreyPrices(InputMessage& msg)
{
	msg.getU32();//??
	if(g_clientVersion >= 1190)
	{
		msg.getU8();//??
		msg.getU8();//??
	}
}

void ProtocolGame::parseMonsterCyclopedia(InputMessage& msg)
{
	Uint16 races = msg.getU16();
	for(Uint16 i = 0; i < races; ++i)
	{
		const std::string raceName = msg.getRawString();
		Uint16 monsters = msg.getU16();
		Uint16 knownMonsters = msg.getU16();
		(void)raceName;
		(void)monsters;
		(void)knownMonsters;
	}
}

void ProtocolGame::parseMonsterCyclopediaMonsters(InputMessage& msg)
{
	const std::string race = msg.getString();
	(void)race;

	Uint16 monsters = msg.getU16();
	for(Uint16 i = 0; i < monsters; ++i)
	{
		Uint16 monsterId = msg.getU16();
		Uint8 progress = msg.getBool();
		if(progress > 0)
		{
			Uint8 occurence = msg.getU8();
			(void)occurence;
		}
		(void)monsterId;
	}
}

void ProtocolGame::parseMonsterCyclopediaRace(InputMessage& msg)
{
	Uint16 monsterId = msg.getU16();
	const std::string raceName = msg.getRawString();
	Uint8 progress = msg.getU8();
	Uint32 totalKills = msg.getU32();
	Uint16 killsToProgress1 = msg.getU16();
	Uint16 killsToProgress2 = msg.getU16();
	Uint16 killsToProgress3 = msg.getU16();
	(void)monsterId;
	(void)raceName;
	(void)totalKills;
	(void)killsToProgress1;
	(void)killsToProgress2;
	(void)killsToProgress3;
	if(progress == 0)
		return;

	Uint8 difficult = msg.getU8();
	Uint8 occurence = msg.getU8();
	(void)difficult;
	(void)occurence;

	Uint8 items = msg.getU8();
	for(Uint8 i = 0; i < items; ++i)
	{
		Uint16 itemId = msg.getU16();
		Uint8 rarity = msg.getU8();
		bool specialEventItem = msg.getBool();
		if(itemId != 0)
		{
			const std::string itemName = msg.getString();
			bool onePlus = msg.getBool();
			(void)itemName;
			(void)onePlus;
		}
		(void)itemId;
		(void)rarity;
		(void)specialEventItem;
	}
	if(progress == 1)
		return;

	Uint16 charmPoints = msg.getU16();
	Uint8 attackType = msg.getU8();
	bool castSpells = msg.getBool();
	Uint32 hitPoints = msg.getU32();
	Uint32 experience = msg.getU32();
	Uint16 speed = msg.getU16();
	Uint16 armor = msg.getU16();
	(void)charmPoints;
	(void)attackType;
	(void)castSpells;
	(void)hitPoints;
	(void)experience;
	(void)speed;
	(void)armor;
	if(progress == 2)
		return;

	Uint8 combats = msg.getU8();
	for(Uint8 i = 0; i < combats; ++i)
	{
		Uint8 combatType = msg.getU8();
		Sint16 combatValue = SDL_static_cast(Sint16, msg.getU16());
		(void)combatType;
		(void)combatValue;
	}

	Uint16 locations = msg.getU16();
	for(Uint16 i = 0; i < locations; ++i)
	{
		const std::string location = msg.getString();
		(void)location;
	}
	if(progress == 3)
		return;

	bool haveCharm = msg.getBool();
	if(haveCharm)
	{
		msg.getU8();//??
		msg.getU32();//??
	}
	else
		msg.getBool();//??
}

void ProtocolGame::parseMonsterCyclopediaBonusEffects(InputMessage& msg)
{
	Sint32 charmPoints = SDL_static_cast(Sint32, msg.getU32());
	(void)charmPoints;
	Uint8 charms = msg.getU8();
	for(Uint8 i = 0; i < charms; ++i)
	{
		Uint8 charmId = msg.getU8();
		const std::string charmName = msg.getString();
		const std::string charmDescription = msg.getString();
		msg.getU8();//??
		Uint16 charmPrice = msg.getU16();
		bool unlocked = msg.getBool();
		bool activated = msg.getBool();
		if(activated)
		{
			Uint16 monsterId = msg.getU16();
			Uint32 clearPrice = msg.getU32();
			(void)monsterId;
			(void)clearPrice;
		}
		(void)charmId;
		(void)charmName;
		(void)charmDescription;
		(void)charmPrice;
		(void)unlocked;
	}

	Uint8 remainingAssignableCharms = msg.getU8();
	bool hasCharmExpansion = (remainingAssignableCharms == 0xFF);
	(void)remainingAssignableCharms;
	(void)hasCharmExpansion;

	Uint16 assignableMonsters = msg.getU16();
	for(Uint16 i = 0; i < assignableMonsters; ++i)
	{
		Uint16 monsterId = msg.getU16();
		(void)monsterId;
	}
}

void ProtocolGame::parseMonsterCyclopediaNewDetails(InputMessage& msg)
{
	msg.getU16();//??
}

void ProtocolGame::parseCyclopediaCharacterInfo(InputMessage& msg)
{
	Uint8 type = msg.getU8();
	if(g_clientVersion >= 1215)
	{
		bool noData = msg.getBool();
		if(noData)
		{
			UTIL_messageBox("Error", "No data available at the moment. Please try again later. Note that it takes approximately 15 minutes if this character has recently logged out from a game world.");
			return;
		}
	}
	switch(type)
	{
		case CHARACTERINFO_BASEINFORMATION:
		{
			const std::string characterName = msg.getString();
			const std::string characterVocation = msg.getString();
			Uint16 characterLevel = msg.getU16();
			(void)characterName;
			(void)characterVocation;
			(void)characterLevel;

			Uint16 lookType;
			if(g_game.hasGameFeature(GAME_FEATURE_LOOKTYPE_U16))
				lookType = msg.getU16();
			else
				lookType = SDL_static_cast(Uint16, msg.getU8());

			Uint16 lookTypeEx = 0;
			Uint8 lookHead = 0;
			Uint8 lookBody = 0;
			Uint8 lookLegs = 0;
			Uint8 lookFeet = 0;
			Uint8 lookAddons = 0;
			if(lookType != 0)
			{
				lookHead = msg.getU8();
				lookBody = msg.getU8();
				lookLegs = msg.getU8();
				lookFeet = msg.getU8();
				if(g_game.hasGameFeature(GAME_FEATURE_ADDONS))
					lookAddons = msg.getU8();
			}
			else
				lookTypeEx = msg.getU16();

			(void)lookType;
			(void)lookTypeEx;
			(void)lookHead;
			(void)lookBody;
			(void)lookLegs;
			(void)lookFeet;
			(void)lookAddons;

			msg.getBool();
			bool enableStoreNTitles = msg.getBool();
			const std::string characterTitle = msg.getString();
			(void)enableStoreNTitles;
			(void)characterTitle;
		}
		break;
		case CHARACTERINFO_GENERALSTATS:
		{
			Uint64 experience = msg.getU64();
			Uint16 level = msg.getU16();
			Uint8 levelPercent = msg.getU8();
			(void)experience;
			(void)level;
			(void)levelPercent;

			Uint16 baseXp = msg.getU16();
			if(g_clientVersion >= 1215)
			{
				Sint32 tournamentFactor = SDL_static_cast(Sint32, msg.getU32());
				(void)tournamentFactor;
			}
			Uint16 grindingXp = msg.getU16();
			Uint16 storeBoostXp = msg.getU16();
			Uint16 huntingBoostXp = msg.getU16();
			(void)baseXp;
			(void)grindingXp;
			(void)storeBoostXp;
			(void)huntingBoostXp;

			Uint16 xpBoostRemainingTime = msg.getU16();
			bool canBuyXpBoost = msg.getBool();
			(void)xpBoostRemainingTime;
			(void)canBuyXpBoost;

			Uint16 health = msg.getU16();
			Uint16 healthMax = msg.getU16();
			Uint16 mana = msg.getU16();
			Uint16 manaMax = msg.getU16();
			(void)health;
			(void)healthMax;
			(void)mana;
			(void)manaMax;

			Uint8 soul = msg.getU8();
			Uint16 stamina = msg.getU16();
			Uint16 regeneration = msg.getU16();
			Uint16 training = msg.getU16();
			(void)soul;
			(void)stamina;
			(void)regeneration;
			(void)training;

			Uint16 speed = msg.getU16();
			Uint16 baseSpeed = msg.getU16();
			(void)speed;
			(void)baseSpeed;

			Uint32 totalCapacity = msg.getU32();
			Uint32 baseCapacity = msg.getU32();
			Uint32 freeCapacity = msg.getU32();
			(void)totalCapacity;
			(void)baseCapacity;
			(void)freeCapacity;

			Uint8 skills = msg.getU8();
			for(Uint8 i = 0; i < skills; ++i)
			{
				Uint8 skillId = msg.getU8();

				Uint16 skillLevel;
				if(g_game.hasGameFeature(GAME_FEATURE_DOUBLE_SKILLS))
					skillLevel = msg.getU16();
				else
					skillLevel = SDL_static_cast(Uint16, msg.getU8());

				Uint16 baseSkillLevel = skillLevel;
				if(g_game.hasGameFeature(GAME_FEATURE_BASE_SKILLS))
				{
					if(g_game.hasGameFeature(GAME_FEATURE_DOUBLE_SKILLS))
						baseSkillLevel = msg.getU16();
					else
						baseSkillLevel = SDL_static_cast(Uint16, msg.getU8());
				}

				Uint8 skillLevelPercent = 0;
				if(g_game.hasGameFeature(GAME_FEATURE_DOUBLE_PERCENT_SKILLS))
				{
					msg.getU16();//loyalty bonus
					skillLevelPercent = SDL_static_cast(Uint8, msg.getU16() / 100);
				}
				else
					skillLevelPercent = msg.getU8();

				(void)skillId;
				(void)skillLevel;
				(void)baseSkillLevel;
				(void)skillLevelPercent;
			}
		}
		break;
		case CHARACTERINFO_COMBATSTATS:
		{
			for(Sint32 i = Skills_CriticalChance; i < Skills_LastAdditionalSkill; ++i)
			{
				Uint16 level;
				if(g_game.hasGameFeature(GAME_FEATURE_DOUBLE_SKILLS))
					level = msg.getU16();
				else
					level = SDL_static_cast(Uint16, msg.getU8());

				Uint16 baseLevel = level;
				if(g_game.hasGameFeature(GAME_FEATURE_BASE_SKILLS))
				{
					if(g_game.hasGameFeature(GAME_FEATURE_DOUBLE_SKILLS))
						baseLevel = msg.getU16();
					else
						baseLevel = SDL_static_cast(Uint16, msg.getU8());
				}

				(void)level;
				(void)baseLevel;
			}

			Uint8 blessings = msg.getU8();
			Uint8 totalBlessings = msg.getU8();
			(void)blessings;
			(void)totalBlessings;

			Uint16 attackValue = msg.getU16();
			Uint8 damageType = msg.getU8();
			(void)attackValue;
			(void)damageType;

			Uint8 convertedDamage = msg.getU8();
			Uint8 convertedType = msg.getU8();
			(void)convertedDamage;
			(void)convertedType;

			Uint16 armorValue = msg.getU16();
			Uint16 defenseValue = msg.getU16();
			(void)armorValue;
			(void)defenseValue;

			Uint8 combats = msg.getU8();
			for(Uint8 i = 0; i < combats; ++i)
			{
				Uint8 combatType = msg.getU8();
				Sint8 combatValue = SDL_static_cast(Sint8, msg.getU8());
				(void)combatType;
				(void)combatValue;
			}
		}
		break;
		case CHARACTERINFO_RECENTDEATHS:
		{
			Uint16 currentPage = msg.getU16();
			Uint16 availablePages = msg.getU16();
			Uint16 deaths = msg.getU16();
			for(Uint16 i = 0; i < deaths; ++i)
			{
				Uint32 timestamp = msg.getU32();
				const char* cause = msg.getRawString();
				(void)timestamp;
				(void)cause;
			}
			(void)currentPage;
			(void)availablePages;
		}
		break;
		case CHARACTERINFO_RECENTPVPKILLS:
		{
			Uint16 currentPage = msg.getU16();
			Uint16 availablePages = msg.getU16();
			Uint16 kills = msg.getU16();
			for(Uint16 i = 0; i < kills; ++i)
			{
				Uint32 timestamp = msg.getU32();
				const char* description = msg.getRawString();
				Uint8 status = msg.getU8();
				(void)timestamp;
				(void)description;
				(void)status;
			}
			(void)currentPage;
			(void)availablePages;
		}
		break;
		case CHARACTERINFO_ACHIEVEMENTS:
		{
			Uint16 totalPoints = msg.getU16();
			Uint16 totalSecretAchievements = msg.getU16();
			(void)totalPoints;
			(void)totalSecretAchievements;

			Uint16 achievements = msg.getU16();
			for(Uint16 i = 0; i < achievements; ++i)
			{
				Uint16 achievementId = msg.getU16();
				Uint32 timestamp = msg.getU32();
				bool secret = msg.getBool();
				if(secret)
				{
					const char* name = msg.getRawString();
					const char* description = msg.getRawString();
					Uint8 grade = msg.getU8();
					(void)name;
					(void)description;
					(void)grade;
				}
				(void)achievementId;
				(void)timestamp;
			}
		}
		break;
		case CHARACTERINFO_ITEMSUMMARY:
		{
			Uint16 size = msg.getU16();
			for(Uint16 i = 0; i < size; ++i)
			{
				msg.getU16();//??
				msg.getU32();//??
			}

			size = msg.getU16();
			for(Uint16 i = 0; i < size; ++i)
			{
				msg.getU16();//??
				msg.getU32();//??
			}

			size = msg.getU16();
			for(Uint16 i = 0; i < size; ++i)
			{
				msg.getU16();//??
				msg.getU32();//??
			}

			size = msg.getU16();
			for(Uint16 i = 0; i < size; ++i)
			{
				msg.getU16();//??
				msg.getU32();//??
			}

			size = msg.getU16();
			for(Uint16 i = 0; i < size; ++i)
			{
				msg.getU16();//??
				msg.getU32();//??
			}
		}
		break;
		case CHARACTERINFO_OUTFITSMOUNTS:
		{
			Uint16 outfits = msg.getU16();
			for(Uint16 i = 0; i < outfits; ++i)
			{
				Uint16 lookType = msg.getU16();
				const std::string outfitName = msg.getString();
				Uint8 lookAddons = msg.getU8();
				Uint8 outfitType = msg.getU8();
				Uint32 outfitId = msg.getU32();
				(void)lookType;
				(void)outfitName;
				(void)lookAddons;
				(void)outfitType;
				(void)outfitId;
			}
			if(outfits > 0)
			{
				Uint8 lookHead = msg.getU8();
				Uint8 lookBody = msg.getU8();
				Uint8 lookLegs = msg.getU8();
				Uint8 lookFeet = msg.getU8();
				(void)lookHead;
				(void)lookBody;
				(void)lookLegs;
				(void)lookFeet;
			}

			Uint16 mounts = msg.getU16();
			for(Uint16 i = 0; i < mounts; ++i)
			{
				Uint16 lookMount = msg.getU16();
				const std::string mountName = msg.getString();
				Uint8 mountType = msg.getU8();
				Uint32 mountId = msg.getU32();
				(void)lookMount;
				(void)mountName;
				(void)mountType;
				(void)mountId;
			}
		}
		break;
		case CHARACTERINFO_STORESUMMARY:
		{
			msg.getU32();//??
			msg.getU32();//??
			Uint8 size = msg.getU8();
			for(Uint8 i = 0; i < size; ++i)
			{
				msg.getRawString();//??
				msg.getU8();//??
			}

			msg.getU8();//??
			msg.getU8();//??
			msg.getU8();//??
			msg.getBool();//??
			msg.getU8();//??
			size = msg.getU8();
			for(Uint8 i = 0; i < size; ++i)
			{
				msg.getU8();//??
			}

			size = msg.getU8();
			for(Uint8 i = 0; i < size; ++i)
			{
				msg.getRawString();//??
				msg.getU8();//??
			}

			Uint16 count = msg.getU16();
			for(Uint16 i = 0; i < count; ++i)
			{
				msg.getU16();//??
				msg.getRawString();//??
				msg.getU8();//??
			}
		}
		break;
		case CHARACTERINFO_INSPECTION:
		{
			Uint8 items = msg.getU8();
			for(Uint8 i = 0; i < items; ++i)
			{
				Uint8 slot = msg.getU8();
				const std::string itemName = msg.getString();
				(void)slot;
				(void)itemName;

				ItemUI* item = getItemUI(msg, msg.getU16());
				(void)item;

				Uint8 imbuements = msg.getU8();
				for(Uint8 j = 0; j < imbuements; ++j)
				{
					msg.getU16();//??
				}

				Uint8 messages = msg.getU8();
				for(Uint8 j = 0; j < messages; ++j)
				{
					const std::string title = msg.getString();
					const std::string value = msg.getString();
					(void)title;
					(void)value;
				}
			}

			const std::string characterName = msg.getString();
			(void)characterName;

			Uint16 lookType;
			if(g_game.hasGameFeature(GAME_FEATURE_LOOKTYPE_U16))
				lookType = msg.getU16();
			else
				lookType = SDL_static_cast(Uint16, msg.getU8());

			Uint16 lookTypeEx = 0;
			Uint8 lookHead = 0;
			Uint8 lookBody = 0;
			Uint8 lookLegs = 0;
			Uint8 lookFeet = 0;
			Uint8 lookAddons = 0;
			if(lookType != 0)
			{
				lookHead = msg.getU8();
				lookBody = msg.getU8();
				lookLegs = msg.getU8();
				lookFeet = msg.getU8();
				if(g_game.hasGameFeature(GAME_FEATURE_ADDONS))
					lookAddons = msg.getU8();
			}
			else
				lookTypeEx = msg.getU16();

			(void)lookType;
			(void)lookTypeEx;
			(void)lookHead;
			(void)lookBody;
			(void)lookLegs;
			(void)lookFeet;
			(void)lookAddons;

			Uint8 messages = msg.getU8();
			for(Uint8 i = 0; i < messages; ++i)
			{
				const std::string title = msg.getString();
				const std::string value = msg.getString();
				(void)title;
				(void)value;
			}
		}
		break;
		case CHARACTERINFO_BADGES:
		{
			bool enabledBadges = msg.getBool();
			if(!enabledBadges)
				return;

			msg.getBool();//??
			msg.getBool();//??
			msg.getRawString();//??
			Uint8 size = msg.getU8();
			for(Uint8 i = 0; i < size; ++i)
			{
				msg.getU32();//??
				msg.getRawString();//??
			}
		}
		break;
		case CHARACTERINFO_TITLES:
		{
			msg.getU8();//??
			Uint8 size = msg.getU8();
			for(Uint8 i = 0; i < size; ++i)
			{
				msg.getU8();//??
				msg.getRawString();//??
				msg.getRawString();//??
				msg.getBool();//??
				msg.getBool();//??
			}
		}
		break;
	}
}

void ProtocolGame::parseHirelingNameChange(InputMessage& msg)
{
	msg.getU32();//??
	msg.getU32();//??
}

void ProtocolGame::parseCyclopediaMapData(InputMessage& msg)
{
	Uint8 type = msg.getU8();
	switch(type)
	{
		case 0:
		{
			Position markPos = msg.getPosition();
			Uint8 marktype = msg.getU8();
			const std::string markDescription = msg.getString();
			(void)markPos;
			(void)marktype;
			(void)markDescription;
		}
		break;
		case 1:
		{
			Uint16 size = msg.getU16();
			for(Uint16 i = 0; i < size; ++i)
			{
				msg.getU16();//??
				msg.getU8();//??
				msg.getU8();//??
			}

			size = msg.getU16();
			for(Uint16 i = 0; i < size; ++i)
			{
				msg.getU16();//??
			}

			size = msg.getU16();
			for(Uint16 i = 0; i < size; ++i)
			{
				msg.getU16();//??
			}
		}
		break;
		case 2:
		{
			msg.getPosition();//??
			msg.getU8();//??
		}
		break;
		case 3:
		{
			msg.getU16();//??
			msg.getU8();//??
		}
		break;
		case 4:
		{
			msg.getU16();//??
			msg.getU8();//??
		}
		break;
		case 5:
		{
			msg.getU16();//??
			msg.getU8();//??
			Uint8 size = msg.getU8();
			for(Uint8 i = 0; i < size; ++i)
			{
				msg.getPosition();//??
				msg.getU8();//??
			}
		}
		break;
		case 6:
		{
			msg.getPosition();//??
			Uint8 size = msg.getU8();
			for(Uint8 i = 0; i < size; ++i)
			{
				msg.getU16();//??
			}
		}
		break;
		case 7:
		{
			Uint8 size = msg.getU8();
			for(Uint8 i = 0; i < size; ++i)
			{
				msg.getU16();//??
				Uint8 count = msg.getU8();
				for(Uint8 j = 0; j < count; ++j)
				{
					msg.getU16();//??
					msg.getBool();//??
					msg.getU8();//??
				}
			}
		}
		break;
		case 8:
		{
			msg.getU16();//??
			msg.getBool();//??
		}
		break;
		case 9:
		{
			msg.getU64();//??
			Uint8 size = msg.getU8();
			for(Uint8 i = 0; i < size; ++i)
			{
				msg.getU16();//??
				msg.getBool();//??
				msg.getU64();//??
			}
		}
		break;
		case 10:
		{
			msg.getU16();//??
		}
		break;
	}
}

void ProtocolGame::parseOpenRewardWall(InputMessage& msg)
{
	msg.getBool();//??
	msg.getU32();//??
	msg.getU8();//??
	Uint8 something = msg.getU8();
	if(something != 0)
	{
		msg.getRawString();//??
	}

	msg.getU32();//??
	msg.getU16();//??
	msg.getU16();//??
}

void ProtocolGame::parseCloseRewardWall(InputMessage&)
{
}

void ProtocolGame::parseDailyRewardBasic(InputMessage& msg)
{
	Uint8 size = msg.getU8();
	for(Uint8 i = 0; i < size; ++i)
	{
		Uint8 type = msg.getU8();
		switch(type)
		{
			case 1:
			{
				msg.getU8();//??
				Uint8 count = msg.getU8();
				for(Uint8 j = 0; j < count; ++j)
				{
					msg.getU16();//??
					msg.getRawString();//??
					msg.getU32();//??
				}
			}
			break;
			case 2:
			{
				Uint8 count = msg.getU8();
				for(Uint8 j = 0; j < count; ++j)
				{
					Uint8 subType = msg.getU8();
					switch(subType)
					{
						case 1:
						{
							msg.getU16();//??
							msg.getRawString();//??
							msg.getU8();//??
						}
						break;
						case 2:
						{
							msg.getU8();//??
						}
						break;
						case 3:
						{
							msg.getU16();//??
						}
						break;
					}
				}
			}
			break;
		}
		type = msg.getU8();
		switch(type)
		{
			case 1:
			{
				msg.getU8();//??
				Uint8 count = msg.getU8();
				for(Uint8 j = 0; j < count; ++j)
				{
					msg.getU16();//??
					msg.getRawString();//??
					msg.getU32();//??
				}
			}
			break;
			case 2:
			{
				Uint8 count = msg.getU8();
				for(Uint8 j = 0; j < count; ++j)
				{
					Uint8 subType = msg.getU8();
					switch(subType)
					{
						case 1:
						{
							msg.getU16();//??
							msg.getRawString();//??
							msg.getU8();//??
						}
						break;
						case 2:
						{
							msg.getU8();//??
						}
						break;
						case 3:
						{
							msg.getU16();//??
						}
						break;
					}
				}
			}
			break;
		}
	}

	size = msg.getU8();
	for(Uint8 i = 0; i < size; ++i)
	{
		msg.getRawString();//??
		msg.getU8();//??
	}

	msg.getU8();//??
}

void ProtocolGame::parseDailyRewardHistory(InputMessage& msg)
{
	Uint8 size = msg.getU8();
	for(Uint8 i = 0; i < size; ++i)
	{
		msg.getU32();//??
		msg.getU8();//??
		msg.getRawString();//??
		msg.getU16();//??
	}
}

void ProtocolGame::parseDailyRewardCollectionState(InputMessage& msg)
{
	msg.getBool();//??
}

void ProtocolGame::parseGameNews(InputMessage& msg)
{
	Uint32 timestamp = msg.getU32();
	bool readed = msg.getBool();
	(void)timestamp;
	(void)readed;
}

void ProtocolGame::parsePlayerDataTournament(InputMessage& msg)
{
	Uint32 dailyPlaytime = msg.getU32();
	(void)dailyPlaytime;
}

void ProtocolGame::parseTournamentInformation(InputMessage& msg)
{
	Uint8 tournamentType = msg.getU8();
	switch(tournamentType)
	{
		case TOURNAMENT_INFORMATION_WORLDS:
		{
			msg.getU8();//need more tests to determine what it is, ??
			Uint32 regularWorldCost = msg.getU32();
			bool enableRestrictedWorld = msg.getBool();
			Uint32 restrictedWorldCost = msg.getU32();
			bool something = msg.getBool();
			if(something)
			{
				msg.getBool();//need more tests to determine what it is, ??
				msg.getBool();//need more tests to determine what it is, ??
				msg.getU16();//need more tests to determine what it is, ??
			}
			(void)regularWorldCost;
			(void)enableRestrictedWorld;
			(void)restrictedWorldCost;
		}
		break;
		case TOURNAMENT_INFORMATION_STAGE:
		{
			Uint32 signupTimestamp = msg.getU32();
			Uint32 runningTimestamp = msg.getU32();
			Uint32 finishTimestamp = msg.getU32();
			StageInformation status = SDL_static_cast(StageInformation, msg.getU8());
			msg.getU32();//need more tests to determine what it is, ??
			Uint32 timeleft = msg.getU32();
			(void)signupTimestamp;
			(void)runningTimestamp;
			(void)finishTimestamp;
			(void)status;
			(void)timeleft;
		}
		break;
		case TOURNAMENT_INFORMATION_RULES:
		{
			RulePvPType pvpType = SDL_static_cast(RulePvPType, msg.getU8());
			Uint32 dailyPlaytime = msg.getU32();
			Uint16 deathPenaltyModifier = msg.getU16();
			Uint16 expMultiplier = msg.getU16();
			Uint16 skillMultiplier = msg.getU16();
			Uint16 spawnMultiplier = msg.getU16();
			Uint16 lootProbability = msg.getU16();
			Uint8 rentPercentage = msg.getU8();
			Uint8 houseAuctionDurations = msg.getU8();
			msg.getRawString();//need more tests to determine what it is, ??
			msg.getRawString();//need more tests to determine what it is, ??
			(void)pvpType;
			(void)dailyPlaytime;
			(void)deathPenaltyModifier;
			(void)expMultiplier;
			(void)skillMultiplier;
			(void)spawnMultiplier;
			(void)lootProbability;
			(void)rentPercentage;
			(void)houseAuctionDurations;
		}
		break;
		case TOURNAMENT_INFORMATION_UNKNOWN:
		{
			Uint8 size = msg.getU8();
			for(Uint8 i = 0; i < size; ++i)
			{
				msg.getRawString();//??
			}

			size = msg.getU8();
			for(Uint8 i = 0; i < size; ++i)
			{
				msg.getRawString();//??
			}

			size = msg.getU8();
			for(Uint8 i = 0; i < size; ++i)
			{
				msg.getU8();//??
			}

			size = msg.getU8();
			for(Uint8 i = 0; i < size; ++i)
			{
				msg.getRawString();//??
			}

			bool something = msg.getBool();
			if(something)
			{
				msg.getRawString();//??
				msg.getU8();//??
				msg.getRawString();//??
			}
		}
		break;
		case TOURNAMENT_INFORMATION_CHARACTERS:
		{
			bool hasRegularWorldCharacter = msg.getBool();
			if(hasRegularWorldCharacter)
			{
				//characterGUID is for cyclopedia character info
				Uint32 characterGUID = msg.getU32();
				std::string characterName = msg.getString();
				Uint16 completedTournaments = msg.getU16();
				Uint32 highestRank = msg.getU32();
				Uint32 highestRankTimestamp = msg.getU32();
				std::string worldName = msg.getString();
				Uint32 tournamentCoinsRewarded = msg.getU32();
				Uint32 tibiaCoinsRewarded = msg.getU32();
				Uint16 tournamentVouchersRewarded = msg.getU16();
				(void)characterGUID;
				(void)characterName;
				(void)completedTournaments;
				(void)highestRank;
				(void)highestRankTimestamp;
				(void)worldName;
				(void)tournamentCoinsRewarded;
				(void)tibiaCoinsRewarded;
				(void)tournamentVouchersRewarded;

				Uint16 lookType;
				if(g_game.hasGameFeature(GAME_FEATURE_LOOKTYPE_U16))
					lookType = msg.getU16();
				else
					lookType = SDL_static_cast(Uint16, msg.getU8());

				Uint16 lookTypeEx = 0;
				Uint8 lookHead = 0;
				Uint8 lookBody = 0;
				Uint8 lookLegs = 0;
				Uint8 lookFeet = 0;
				Uint8 lookAddons = 0;
				if(lookType != 0)
				{
					lookHead = msg.getU8();
					lookBody = msg.getU8();
					lookLegs = msg.getU8();
					lookFeet = msg.getU8();
					if(g_game.hasGameFeature(GAME_FEATURE_ADDONS))
						lookAddons = msg.getU8();
				}
				else
					lookTypeEx = msg.getU16();

				(void)lookType;
				(void)lookTypeEx;
				(void)lookHead;
				(void)lookBody;
				(void)lookLegs;
				(void)lookFeet;
				(void)lookAddons;

				Uint32 totalPlaytime = msg.getU32();
				Uint32 totalDeaths = msg.getU32();
				(void)totalPlaytime;
				(void)totalDeaths;
			}

			bool hasRestrictedWorldCharacter = msg.getBool();
			if(hasRestrictedWorldCharacter)
			{
				Uint32 characterGUID = msg.getU32();
				std::string characterName = msg.getString();
				Uint16 completedTournaments = msg.getU16();
				Uint32 highestRank = msg.getU32();
				Uint32 highestRankTimestamp = msg.getU32();
				std::string worldName = msg.getString();
				Uint32 tournamentCoinsRewarded = msg.getU32();
				Uint32 tibiaCoinsRewarded = msg.getU32();
				Uint16 tournamentVouchersRewarded = msg.getU16();
				(void)characterGUID;
				(void)characterName;
				(void)completedTournaments;
				(void)highestRank;
				(void)highestRankTimestamp;
				(void)worldName;
				(void)tournamentCoinsRewarded;
				(void)tibiaCoinsRewarded;
				(void)tournamentVouchersRewarded;

				Uint16 lookType;
				if(g_game.hasGameFeature(GAME_FEATURE_LOOKTYPE_U16))
					lookType = msg.getU16();
				else
					lookType = SDL_static_cast(Uint16, msg.getU8());

				Uint16 lookTypeEx = 0;
				Uint8 lookHead = 0;
				Uint8 lookBody = 0;
				Uint8 lookLegs = 0;
				Uint8 lookFeet = 0;
				Uint8 lookAddons = 0;
				if(lookType != 0)
				{
					lookHead = msg.getU8();
					lookBody = msg.getU8();
					lookLegs = msg.getU8();
					lookFeet = msg.getU8();
					if(g_game.hasGameFeature(GAME_FEATURE_ADDONS))
						lookAddons = msg.getU8();
				}
				else
					lookTypeEx = msg.getU16();

				(void)lookType;
				(void)lookTypeEx;
				(void)lookHead;
				(void)lookBody;
				(void)lookLegs;
				(void)lookFeet;
				(void)lookAddons;

				Uint32 totalPlaytime = msg.getU32();
				Uint32 totalDeaths = msg.getU32();
				(void)totalPlaytime;
				(void)totalDeaths;
			}
		}
		break;
	}
}

void ProtocolGame::parseTournamentLeaderboard(InputMessage& msg)
{
	Uint8 type = msg.getU8();
	bool noData = msg.getBool();
	if(noData)
	{
		//pass no data available to leaderboard window
		return;
	}

	switch(type)
	{
		case TOURNAMENT_LEADERBOARD_WORLDS:
		{
			Uint8 worlds = msg.getU8();
			for(Uint8 i = 0; i < worlds; ++i)
			{
				std::string worldName = msg.getRawString();
				(void)worldName;
			}

			std::string currentWorld = msg.getString();
			Uint16 refreshInSeconds = msg.getU16();
			Uint16 currentPage = msg.getU16();
			Uint16 totalPages = msg.getU16();
			(void)currentWorld;
			(void)refreshInSeconds;
			(void)currentPage;
			(void)totalPages;
		}
		case TOURNAMENT_LEADERBOARD_CHARACTERS:
		{
			Uint8 characters = msg.getU8();
			for(Uint8 i = 0; i < characters; ++i)
			{
				Uint32 currentRank = msg.getU32();
				Uint32 lastRank = msg.getU32();
				std::string name = msg.getString();
				Uint8 vocation = msg.getU8();
				bool promoted = (vocation >= 10);
				vocation %= 10;

				Uint64 score = msg.getU64();
				RankIndicator rankIndicator = SDL_static_cast(RankIndicator, msg.getU8());
				bool unk1 = msg.getBool();//need more tests to determine what it is, ??
				bool unk2 = msg.getBool();//need more tests to determine what it is, change color name to green(our character indicator?)

				(void)currentRank;
				(void)lastRank;
				(void)name;
				(void)vocation;
				(void)promoted;
				(void)score;
				(void)rankIndicator;
				(void)unk1;
				(void)unk2;
			}

			Uint8 unk3 = msg.getU8();//need more tests to determine what it is, ??
			std::string unk4 = msg.getString();//need more tests to determine what it is, ??
			(void)unk3;
			(void)unk4;
		}
		break;
	}
}

void ProtocolGame::onConnect()
{
	if(g_game.hasGameFeature(GAME_FEATURE_SERVER_SENDFIRST))
		setChecksumMethod(CHECKSUM_METHOD_CHALLENGE);
	else
		sendLogin(0, 0);

	if(g_game.hasGameFeature(GAME_FEATURE_CLIENT_SENDSERVERNAME))
		sendServerName(g_engine.getCharacterWorldName());
}

void ProtocolGame::onConnectionError(ConnectionError error)
{
	if(!m_skipErrors)
		m_lastError = error;
}

void ProtocolGame::onDisconnect()
{
	if(m_gameInitialized)
		g_engine.processGameEnd();

	if(m_lastError != CONNECTION_ERROR_INVALID)
	{
		switch(m_lastError)
		{
			case CONNECTION_ERROR_RESOLVE_HOST:
				UTIL_messageBox("Connection Failed", "Cannot connect to a game server.\n\nError: Cannot resolve host.");
				break;
			case CONNECTION_ERROR_CREATE_SOCKET:
				UTIL_messageBox("Connection Failed", "Cannot connect to a game server.\n\nError: Could not create socket.");
				break;
			case CONNECTION_ERROR_SET_NONBLOCKING_SOCKET:
				UTIL_messageBox("Connection Failed", "Cannot connect to a game server.\n\nError: Could not create a non-blocking socket.");
				break;
			case CONNECTION_ERROR_CANNOT_CONNECT:
				UTIL_messageBox("Connection Failed", "Cannot connect to a game server.\n\nError: Cannot connect.");
				break;
			case CONNECTION_ERROR_FAIL_CONNECT:
				UTIL_messageBox("Connection Failed", "Cannot connect to a game server.\n\nError: Connection failed.");
				break;
			case CONNECTION_ERROR_REFUSED_CONNECT:
				UTIL_messageBox("Connection Failed", "Cannot connect to a game server.\n\nError: Connection refused.");
				break;
			case CONNECTION_ERROR_TIMEOUT:
				UTIL_messageBox("Connection Failed", "Cannot connect to a game server.\n\nError: Connection timed out.");
				break;
			case CONNECTION_ERROR_RECV_FAIL:
				UTIL_messageBox("Connection Failed", "Cannot connect to a game server.\n\nError: Failed to call recv().");
				break;
			case CONNECTION_ERROR_SEND_FAIL:
				UTIL_messageBox("Connection Failed", "Cannot connect to a game server.\n\nError: Failed to call send().");
				break;
			case CONNECTION_ERROR_PROTOCOL_FAIL:
				UTIL_messageBox("Connection Failed", "Cannot connect to a game server.\n\nError: Protocol mismatched.");
				break;
			default:
				break;
		}
	}
	delete this;
}

void ProtocolGame::sendServerName(const std::string& serverName)
{
	if(!g_connection)
		return;

	OutputMessage msg(0);
	for(size_t i = 0, len = serverName.length(); i < len; ++i)
		msg.addU8(serverName[i]);
	msg.addU8('\n');
	g_connection->sendMessage(msg);
}

void ProtocolGame::sendLogin(Uint32 challengeTimestamp, Uint8 challengeRandom)
{
	bool checksumFeature = (g_game.hasGameFeature(GAME_FEATURE_CHECKSUM) || g_game.hasGameFeature(GAME_FEATURE_PROTOCOLSEQUENCE));
	bool xteaFeature = g_game.hasGameFeature(GAME_FEATURE_XTEA);
	bool rsa1024Feature = g_game.hasGameFeature(GAME_FEATURE_RSA1024);

	OutputMessage msg((checksumFeature ? 6 : 2));
	msg.addU8(GameLoginOpcode);
	msg.addU16(Protocol::getOS());
	msg.addU16(Protocol::getProtocolVersion());
	if(g_game.hasGameFeature(GAME_FEATURE_CLIENT_VERSION))
		msg.addU32(Protocol::getClientVersion());

	if(g_game.hasGameFeature(GAME_FEATURE_CONTENT_REVISION))
		msg.addU16(SDL_static_cast(Uint16, g_datRevision));

	if(g_game.hasGameFeature(GAME_FEATURE_PREVIEW_STATE))
		msg.addU8(0);

	if(g_clientVersion >= 1149 && g_clientVersion < 1200)
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

	msg.addU8(0);//Is gamemaster mode
	if(g_game.hasGameFeature(GAME_FEATURE_SESSIONKEY))
	{
		msg.addString(g_engine.getAccountSessionKey());
		msg.addString(g_engine.getCharacterName());
	}
	else
	{
		if(g_game.hasGameFeature(GAME_FEATURE_ACCOUNT_NAME))
			msg.addString(g_engine.getAccountName());
		else
			msg.addU32(SDL_static_cast(Uint32, SDL_strtoul(g_engine.getAccountName().c_str(), NULL, 10)));

		msg.addString(g_engine.getCharacterName());
		msg.addString(g_engine.getAccountPassword());
		if(g_game.hasGameFeature(GAME_FEATURE_AUTHENTICATOR))
			msg.addString("");//Token
	}

	if(g_game.hasGameFeature(GAME_FEATURE_SERVER_SENDFIRST))
	{
		msg.addU32(challengeTimestamp);
		msg.addU8(challengeRandom);
	}

	if(rsa1024Feature)
	{
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

void ProtocolGame::sendSecondaryLogin()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameSecondaryLoginOpcode);

	Uint16 firstByte = msg.getWritePos();
	msg.addU8(0);
	msg.addString(g_engine.getAccountSessionKey());
	msg.addString(g_engine.getCharacterName());
	msg.addPaddingBytes(128-(msg.getWritePos()-firstByte));
	g_rsa.encrypt(msg.getBuffer()+firstByte);
	onSend(msg);
}

void ProtocolGame::sendEnterGame()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameEnterGameOpcode);
	onSend(msg);
}

void ProtocolGame::sendLogout()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameLeaveOpcode);
	onSend(msg);
}

void ProtocolGame::sendPing()
{
	if(m_pingReceived != m_pingSent)
		return;

	if(g_game.hasGameFeature(GAME_FEATURE_PING))
	{
		OutputMessage msg(getHeaderPos());
		msg.addU8(GamePingOpcode);
		onSend(msg);
	}
	else
	{
		//Emulate ping through request channels packet on older versions
		OutputMessage msg(getHeaderPos());
		msg.addU8(GameRequestChannelsOpcode);
		onSend(msg);
	}

	++m_pingSent;
	m_pingTime = SDL_GetTicks();
}

void ProtocolGame::sendPingBack()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GamePingBackOpcode);
	onSend(msg);
}

void ProtocolGame::sendConnectionPingBack()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameConnectionPingBackOpcode);
	onSend(msg);
}

void ProtocolGame::sendAutoWalk(const std::vector<Direction>& path)
{
	if(path.empty())
		return;

	//In cipbia the max path tiles is 127(probably because of int8_t limit)
	size_t tileSize = UTIL_min<size_t>(0x7F, path.size());

	OutputMessage msg(getHeaderPos());
	msg.addU8(GameAutowalkOpcode);
	msg.addU8(SDL_static_cast(Uint8, tileSize));
	for(size_t i = 1; i <= tileSize; ++i)
	{
		Uint8 walkByte;
		Direction dir = path[path.size()-i];
		switch(dir)
		{
			case DIRECTION_NORTH: walkByte = 3; break;
			case DIRECTION_EAST: walkByte = 1; break;
			case DIRECTION_SOUTH: walkByte = 7; break;
			case DIRECTION_WEST: walkByte = 5; break;
			case DIRECTION_SOUTHWEST: walkByte = 6; break;
			case DIRECTION_SOUTHEAST: walkByte = 8; break;
			case DIRECTION_NORTHWEST: walkByte = 4; break;
			case DIRECTION_NORTHEAST: walkByte = 2; break;
			default: walkByte = 0; break;
		}
		msg.addU8(walkByte);
	}
	onSend(msg);
}

void ProtocolGame::sendWalk(Direction dir)
{
	OutputMessage msg(getHeaderPos());
	Uint8 headerByte;
	switch(dir)
	{
		case DIRECTION_NORTH: headerByte = GameWalkNorthOpcode; break;
		case DIRECTION_EAST: headerByte = GameWalkEastOpcode; break;
		case DIRECTION_SOUTH: headerByte = GameWalkSouthOpcode; break;
		case DIRECTION_WEST: headerByte = GameWalkWestOpcode; break;
		case DIRECTION_SOUTHWEST: headerByte = GameWalkSouthWestOpcode; break;
		case DIRECTION_SOUTHEAST: headerByte = GameWalkSouthEastOpcode; break;
		case DIRECTION_NORTHWEST: headerByte = GameWalkNorthWestOpcode; break;
		case DIRECTION_NORTHEAST: headerByte = GameWalkNorthEastOpcode; break;
		default: headerByte = GameStopAutowalkOpcode; break;
	}
	msg.addU8(headerByte);
	onSend(msg);
}

void ProtocolGame::sendTurnNorth()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameTurnNorthOpcode);
	onSend(msg);
}

void ProtocolGame::sendTurnEast()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameTurnEastOpcode);
	onSend(msg);
}

void ProtocolGame::sendTurnSouth()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameTurnSouthOpcode);
	onSend(msg);
}

void ProtocolGame::sendTurnWest()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameTurnWestOpcode);
	onSend(msg);
}

void ProtocolGame::sendEquipItem(Uint16 itemid, Uint16 count)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameEqupItemOpcode);
	msg.addU16(itemid);
	if(g_game.hasGameFeature(GAME_FEATURE_ITEMS_U16))
		msg.addU16(count);
	else
		msg.addU8(SDL_static_cast(Uint8, count));
	onSend(msg);
}

void ProtocolGame::sendMove(const Position& fromPos, Uint16 itemid, Uint8 stackpos, const Position& toPos, Uint16 count)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameThrowOpcode);
	msg.addPosition(fromPos);
	msg.addU16(itemid);
	msg.addU8(stackpos);
	msg.addPosition(toPos);
	if(g_game.hasGameFeature(GAME_FEATURE_ITEMS_U16))
		msg.addU16(count);
	else
		msg.addU8(SDL_static_cast(Uint8, count));
	onSend(msg);
}

void ProtocolGame::sendLookInShop(Uint16 itemid, Uint16 count)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameLookInShopOpcode);
	msg.addU16(itemid);
	if(g_game.hasGameFeature(GAME_FEATURE_ITEMS_U16))
		msg.addU16(count);
	else
		msg.addU8(SDL_static_cast(Uint8, count));
	onSend(msg);
}

void ProtocolGame::sendNPCPurchase(Uint16 itemid, Uint8 subtype, Uint16 amount, bool ignoreCapacity, bool buyWithBackpack)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameNPCPurchaseOpcode);
	msg.addU16(itemid);
	msg.addU8(subtype);
	if(g_game.hasGameFeature(GAME_FEATURE_ITEMS_U16))
		msg.addU16(amount);
	else
		msg.addU8(SDL_static_cast(Uint8, amount));
	msg.addBool(ignoreCapacity);
	msg.addBool(buyWithBackpack);
	onSend(msg);
}

void ProtocolGame::sendNPCSell(Uint16 itemid, Uint8 subtype, Uint16 amount, bool ignoreEquipped)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameNPCSellOpcode);
	msg.addU16(itemid);
	msg.addU8(subtype);
	if(g_game.hasGameFeature(GAME_FEATURE_ITEMS_U16))
		msg.addU16(amount);
	else
		msg.addU8(SDL_static_cast(Uint8, amount));
	msg.addBool(ignoreEquipped);
	onSend(msg);
}

void ProtocolGame::sendNPCClose()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameNPCCloseOpcode);
	onSend(msg);
}

void ProtocolGame::sendRequestTrade(const Position& position, Uint16 thingId, Uint8 stackpos, Uint32 creatureId)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameRequestTradeOpcode);
	msg.addPosition(position);
	msg.addU16(thingId);
	msg.addU8(stackpos);
	msg.addU32(creatureId);
	onSend(msg);
}

void ProtocolGame::sendLookInTrade(bool counterOffer, Uint8 index)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameLookInTradeOpcode);
	msg.addBool(counterOffer);
	msg.addU8(index);
	onSend(msg);
}

void ProtocolGame::sendAcceptTrade()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameAcceptTradeOpcode);
	onSend(msg);
}

void ProtocolGame::sendCloseTrade()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameCloseTradeOpcode);
	onSend(msg);
}

void ProtocolGame::sendUseItem(const Position& position, Uint16 itemId, Uint8 stackpos, Uint8 index)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameUseItemOpcode);
	msg.addPosition(position);
	msg.addU16(itemId);
	msg.addU8(stackpos);
	msg.addU8(index);
	onSend(msg);
}

void ProtocolGame::sendUseItemEx(const Position& fromPos, Uint16 itemId, Uint8 fromStackPos, const Position& toPos, Uint16 toItemId, Uint8 toStackPos)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameUseItemExOpcode);
	msg.addPosition(fromPos);
	msg.addU16(itemId);
	msg.addU8(fromStackPos);
	msg.addPosition(toPos);
	msg.addU16(toItemId);
	msg.addU8(toStackPos);
	onSend(msg);
}

void ProtocolGame::sendUseOnCreature(const Position& position, Uint16 itemId, Uint8 stackpos, Uint32 creatureId)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameUseWithCreatureOpcode);
	msg.addPosition(position);
	msg.addU16(itemId);
	msg.addU8(stackpos);
	msg.addU32(creatureId);
	onSend(msg);
}

void ProtocolGame::sendRotateItem(const Position& position, Uint16 itemId, Uint8 stackpos)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameRotateItemOpcode);
	msg.addPosition(position);
	msg.addU16(itemId);
	msg.addU8(stackpos);
	onSend(msg);
}

void ProtocolGame::sendRequestItemInfo(std::vector<RequestItems>& requestItems)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameRequestItemInfoOpcode);

	size_t items = UTIL_min<size_t>(0xFF, requestItems.size());
	msg.addU8(SDL_static_cast(Uint8, items));
	for(size_t i = 0; i < items; ++i)
	{
		RequestItems& requestItem = requestItems[i];
		msg.addU16(requestItem.itemId);
		msg.addU8(requestItem.itemCount);
	}

	onSend(msg);
}

void ProtocolGame::sendWrapState(const Position& position, Uint16 itemId, Uint8 stackpos)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameWrapStateOpcode);
	msg.addPosition(position);
	msg.addU16(itemId);
	msg.addU8(stackpos);
	onSend(msg);
}

void ProtocolGame::sendCloseContainer(Uint8 containerId)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameCloseContainerOpcode);
	msg.addU8(containerId);
	onSend(msg);
}

void ProtocolGame::sendUpContainer(Uint8 containerId)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameUpContainerOpcode);
	msg.addU8(containerId);
	onSend(msg);
}

void ProtocolGame::sendUpdateContainer(Uint8 containerId)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameUpdateContainerOpcode);
	msg.addU8(containerId);
	onSend(msg);
}

void ProtocolGame::sendSeekInContainer(Uint8 containerId, Uint16 index)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameSeekInContainerOpcode);
	msg.addU8(containerId);
	msg.addU16(index);
	onSend(msg);
}

void ProtocolGame::sendBrowseField(const Position& position)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameBrowseFieldOpcode);
	msg.addPosition(position);
	onSend(msg);
}

void ProtocolGame::sendOpenParentContainer(const Position& position)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameOpenParentContainerOpcode);
	msg.addPosition(position);
	onSend(msg);
}

void ProtocolGame::sendOpenDepotSearch()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameOpenDepotSearchOpcode);
	onSend(msg);
}

void ProtocolGame::sendCloseDepotSearch()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameCloseDepotSearchOpcode);
	onSend(msg);
}

void ProtocolGame::sendDepotSearchType(Uint16 itemId)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameDepotSearchTypeOpcode);
	msg.addU16(itemId);
	onSend(msg);
}

void ProtocolGame::sendDepotSearchRetrieve(Uint16 itemId, Uint8 itemCount)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameDepotSearchRetrieveOpcode);
	msg.addU16(itemId);
	msg.addU8(itemCount);
	onSend(msg);
}

void ProtocolGame::sendStashAction(Sint8 unk1)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameStashActionOpcode);
	msg.addU8(SDL_static_cast(Uint8, unk1));
	if(!(unk1 < 0))
	{
		if(unk1 <= 2)
		{
			msg.addPosition(Position(0xFFFF, 0xFFFF, 0xFF));//??
			msg.addU16(0);//??
			msg.addU8(0);//??
			if(unk1 == 0)
			{
				msg.addU8(0);//??
			}
		}
		else if(unk1 == 3)
		{
			msg.addU16(0);//??
			msg.addU32(0);//??
			msg.addU8(0);//??
		}
	}
	onSend(msg);
}

void ProtocolGame::sendTextWindow(Uint32 windowId, const std::string& text)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameTextWindowOpcode);
	msg.addU32(windowId);
	msg.addString(text);
	onSend(msg);
}

void ProtocolGame::sendHouseWindow(Uint32 windowId, Uint8 doorId, const std::string& text)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameHouseWindowOpcode);
	msg.addU8(doorId);
	msg.addU32(windowId);
	msg.addString(text);
	onSend(msg);
}

void ProtocolGame::sendLookAt(const Position& position, Uint16 thingId, Uint8 stackpos)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameLookAtOpcode);
	msg.addPosition(position);
	msg.addU16(thingId);
	msg.addU8(stackpos);
	onSend(msg);
}

void ProtocolGame::sendLookInBattle(Uint32 creatureId)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameLookInBattleOpcode);
	msg.addU32(creatureId);
	onSend(msg);
}

void ProtocolGame::sendSay(MessageMode mode, Uint16 channelId, const std::string& receiver, const std::string& message)
{
	if(message.empty() || message.length() > 255)
		return;

	Uint8 talkMode = translateMessageModeToServer(mode);
	if(talkMode == MessageInvalid)
		return;

	OutputMessage msg(getHeaderPos());
	msg.addU8(GameSayOpcode);
	msg.addU8(talkMode);
	switch(mode)
	{
		case MessagePrivateTo:
		case MessageGamemasterPrivateTo:
		case MessageRVRAnswer:
			msg.addString(receiver);
			break;
		case MessageChannel:
		case MessageChannelHighlight:
		case MessageChannelManagement:
		case MessageGamemasterChannel:
			msg.addU16(channelId);
			break;
		default:
			break;
	}
	msg.addString(message);
	onSend(msg);
}

void ProtocolGame::sendUpdateExivaOptions()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameUpdateExivaOptionsOpcode);
	msg.addBool(false);//??
	msg.addBool(false);//??
	msg.addBool(false);//??
	msg.addBool(false);//??
	msg.addBool(false);//??
	msg.addBool(false);//??
	msg.addU16(0);//?? for(Uint16 i = 0, end = size; i < end; ++i) msg.addString("");//??
	msg.addU16(0);//?? for(Uint16 i = 0, end = size; i < end; ++i) msg.addString("");//??
	msg.addU16(0);//?? for(Uint16 i = 0, end = size; i < end; ++i) msg.addString("");//??
	msg.addU16(0);//?? for(Uint16 i = 0, end = size; i < end; ++i) msg.addString("");//??
	onSend(msg);
}

void ProtocolGame::sendRequestChannels()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameRequestChannelsOpcode);
	onSend(msg);
	++m_expectChannels;
}

void ProtocolGame::sendOpenChannel(Uint16 channelId)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameOpenChannelOpcode);
	msg.addU16(channelId);
	onSend(msg);
}

void ProtocolGame::sendCloseChannel(Uint16 channelId)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameCloseChannelOpcode);
	msg.addU16(channelId);
	onSend(msg);
}

void ProtocolGame::sendOpenPrivateChannel(const std::string& receiver)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameOpenPrivateChannelOpcode);
	msg.addString(receiver);
	onSend(msg);
}

void ProtocolGame::sendOpenRuleViolationChannel(const std::string& reporter)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameOpenRuleViolationChannelOpcode);
	msg.addString(reporter);
	onSend(msg);
}

void ProtocolGame::sendCloseRuleViolationChannel(const std::string& reporter)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameCloseRuleViolationChannelOpcode);
	msg.addString(reporter);
	onSend(msg);
}

void ProtocolGame::sendCancelRuleViolationChannel()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameCancelRuleViolationChannelOpcode);
	onSend(msg);
}

void ProtocolGame::sendCloseNPCChannel()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameCloseNpcChannelOpcode);
	onSend(msg);
}

void ProtocolGame::sendCreatePrivateChannel()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameCreatePrivateChannelOpcode);
	onSend(msg);
}

void ProtocolGame::sendChannelInvite(const std::string& target, Uint16 channelId)
{
	if(target.empty())
		return;

	OutputMessage msg(getHeaderPos());
	msg.addU8(GameChannelInviteOpcode);
	msg.addString(target);
	if(g_clientVersion >= 1070)
		msg.addU16(channelId);
	onSend(msg);
}

void ProtocolGame::sendChannelExclude(const std::string& target, Uint16 channelId)
{
	if(target.empty())
		return;

	OutputMessage msg(getHeaderPos());
	msg.addU8(GameChannelExcludeOpcode);
	msg.addString(target);
	if(g_clientVersion >= 1070)
		msg.addU16(channelId);
	onSend(msg);
}

void ProtocolGame::sendAttackModes(Uint8 attackMode, Uint8 chaseMode, Uint8 secureMode, Uint8 pvpMode)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameAttackModesOpcode);
	msg.addU8(attackMode);
	msg.addU8(chaseMode);
	msg.addU8(secureMode);
	if(g_game.hasGameFeature(GAME_FEATURE_PVP_MODE))
		msg.addU8(pvpMode);
	onSend(msg);
}

void ProtocolGame::sendAttack(Uint32 creatureId, Uint32 sequence)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameAttackOpcode);
	msg.addU32(creatureId);
	if(g_game.hasGameFeature(GAME_FEATURE_ATTACK_SEQUENCE))
		msg.addU32(sequence);
	onSend(msg);
}

void ProtocolGame::sendFollow(Uint32 creatureId, Uint32 sequence)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameFollowOpcode);
	msg.addU32(creatureId);
	if(g_game.hasGameFeature(GAME_FEATURE_ATTACK_SEQUENCE))
		msg.addU32(sequence);
	onSend(msg);
}

void ProtocolGame::sendCancelAttackAndFollow()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameCancelAttackAndFollowOpcode);
	onSend(msg);
}

void ProtocolGame::sendJoinAggression(Uint32 creatureId)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameJoinAggressionOpcode);
	msg.addU32(creatureId);
	onSend(msg);
}

void ProtocolGame::sendInviteToParty(Uint32 creatureId)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameInviteToPartyOpcode);
	msg.addU32(creatureId);
	onSend(msg);
}

void ProtocolGame::sendJoinToParty(Uint32 creatureId)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameJoinToPartyOpcode);
	msg.addU32(creatureId);
	onSend(msg);
}

void ProtocolGame::sendRevokePartyInvitation(Uint32 creatureId)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameRevokePartyInviteOpcode);
	msg.addU32(creatureId);
	onSend(msg);
}

void ProtocolGame::sendPassPartyLeadership(Uint32 creatureId)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GamePassPartyLeadershipOpcode);
	msg.addU32(creatureId);
	onSend(msg);
}

void ProtocolGame::sendLeaveParty()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameLeavePartyOpcode);
	onSend(msg);
}

void ProtocolGame::sendEnableSharedPartyExperience(bool active)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameEnableSharedPartyExperienceOpcode);
	msg.addBool(active);
	onSend(msg);
}

void ProtocolGame::sendRequestOutfit()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameRequestOutfitOpcode);
	if(g_clientVersion >= 1220)
	{
		volatile Uint8 something = 0;
		msg.addU8(something);
		if(something == 1 || something == 3)
			msg.addU16(0);
		else if(something == 2)
			msg.addU32(0);
	}
	onSend(msg);
}

void ProtocolGame::sendSetOutfit(Uint16 lookType, Uint8 lookHead, Uint8 lookBody, Uint8 lookLegs, Uint8 lookFeet, Uint8 lookAddons, Uint16 lookMount)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameSetOutfitOpcode);
	Uint8 outfitType = 0;
	if(g_clientVersion >= 1220)//Detect client version
		msg.addU8(outfitType);

	if(g_game.hasGameFeature(GAME_FEATURE_LOOKTYPE_U16))
		msg.addU16(lookType);
	else
		msg.addU8(SDL_static_cast(Uint8, lookType));

	msg.addU8(lookHead);
	msg.addU8(lookBody);
	msg.addU8(lookLegs);
	msg.addU8(lookFeet);
	if(g_game.hasGameFeature(GAME_FEATURE_ADDONS))
		msg.addU8(lookAddons);
	if(outfitType == 0)
	{
		if(g_game.hasGameFeature(GAME_FEATURE_MOUNTS))
			msg.addU16(lookMount);
	}
	else if(outfitType == 1)
	{
		msg.addU32(0);//??
	}
	onSend(msg);
}

void ProtocolGame::sendMount(bool active)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameMountOpcode);
	msg.addBool(active);
	onSend(msg);
}

void ProtocolGame::sendAddVip(const std::string& name)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameAddVipOpcode);
	msg.addString(name);
	onSend(msg);
}

void ProtocolGame::sendRemoveVip(Uint32 playerGUID)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameRemoveVipOpcode);
	msg.addU32(playerGUID);
	onSend(msg);
}

void ProtocolGame::sendEditVip(Uint32 playerGUID, const std::string& description, Uint32 iconId, bool notifyLogin, std::vector<Uint8>& groupIds)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameEditVipOpcode);
	msg.addU32(playerGUID);
	msg.addString(description);
	msg.addU32(iconId);
	msg.addBool(notifyLogin);
	if(g_game.hasGameFeature(GAME_FEATURE_VIP_GROUPS))
	{
		size_t groups = UTIL_min<size_t>(0xFF, groupIds.size());
		msg.addU8(SDL_static_cast(Uint8, groups));
		for(size_t i = 0; i < groups; ++i)
			msg.addU8(groupIds[i]);
	}
	onSend(msg);
}

void ProtocolGame::sendAddVipGroup(Uint8 groupType, const std::string& name)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameAddVipGroupOpcode);
	if(groupType = 1)
	{
		msg.addU8(1);
		msg.addString(name);
	}
	else if(groupType == 2)
	{
		msg.addU8(3);
		msg.addU8(0);//??
		msg.addString(name);
	}
	else if(groupType == 3)
	{
		msg.addU8(3);
		msg.addU8(0);//??
	}
	onSend(msg);
}

void ProtocolGame::sendFriendSystemAction()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameFriendSystemActionOpcode);

	Uint8 something = 0;
	msg.addU8(something);
	switch(something)
	{
		case 3:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		{
			msg.addU32(0);//??
		}
		break;
		case 4:
		case 10:
		{
			msg.addU32(0);//??
			msg.addU8(0);//??
		}
		break;
		case 11:
		{
			msg.addString("");//??
		}
		break;
		case 13:
		{
			msg.addU32(0);//??
			msg.addBool(0);//??
		}
		break;
		case 14:
		{
			msg.addU8(0);//??
		}
		break;
		case 16:
		{
			msg.addU8(0);//??
			msg.addBool(0);//??
			msg.addBool(0);//??
			msg.addBool(0);//??
		}
		break;
	}
	onSend(msg);
}

void ProtocolGame::sendNewBugReport(BugReportCategory category, const std::string& comment, const std::string& typoSpeaker, const std::string& typoText)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameBugReportOpcode);
	msg.addU8(SDL_static_cast(Uint8, category));
	msg.addString(comment);
	switch(category)
	{
		case BUG_CATEGORY_MAP:
		{
			Creature* player = g_map.getLocalCreature();
			if(player)
				msg.addPosition(player->getCurrentPosition());
			else
			{
				msg.addU16(0xFFFF);
				msg.addU16(0xFFFF);
				msg.addU8(0xFF);
			}
		}
		break;
		case BUG_CATEGORY_TYPO:
		{
			msg.addString(typoSpeaker);
			msg.addString(typoText);
		}
		break;
		default:
			break;
	}
	onSend(msg);
}

void ProtocolGame::sendBugReport(const std::string& comment)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameBugReportOpcode);
	msg.addString(comment);
	onSend(msg);
}

void ProtocolGame::sendRuleViolation(const std::string& target, Uint8 reason, Uint8 action, const std::string& comment, const std::string& statement, Uint16 statementId, bool ipBanishment)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameRuleViolationOpcode);
	msg.addString(target);
	msg.addU8(reason);
	msg.addU8(action);
	msg.addString(comment);
	msg.addString(statement);
	msg.addU16(statementId);
	msg.addBool(ipBanishment);
	onSend(msg);
}

void ProtocolGame::sendNewRuleViolation(Uint8 reason, Uint8 action, const std::string& characterName, const std::string& comment, const std::string& translation, Uint32 statementId)
{
	if(characterName.empty())
		return;

	OutputMessage msg(getHeaderPos());
	msg.addU8(GameNewRuleViolationOpcode);
	msg.addU8(reason);
	msg.addU8(action);
	msg.addString(characterName);
	msg.addString(comment);
	switch(reason)
	{
		case 0:
			msg.addString(translation);
			break;
		case 1:
		case 3:
		{
			msg.addString(translation);
			msg.addU32(statementId);
		}
		break;
		default:
			break;
	}
	onSend(msg);
}

void ProtocolGame::sendDebugReport(const std::string& line, const std::string& date, const std::string& decription, const std::string& comment)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameDebugAssertOpcode);
	msg.addString(line);
	msg.addString(date);
	msg.addString(decription);
	msg.addString(comment);
	onSend(msg);
}

void ProtocolGame::sendShowQuestLog()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameShowQuestLogOpcode);
	onSend(msg);
}

void ProtocolGame::sendShowQuestLine(Uint16 questId)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameShowQuestLineOpcode);
	msg.addU16(questId);
	onSend(msg);
}

void ProtocolGame::sendAnswerModalDialog(Uint32 dialogId, Uint8 button, Uint8 choice)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameAnswerModalDialogOpcode);
	msg.addU32(dialogId);
	msg.addU8(button);
	msg.addU8(choice);
	onSend(msg);
}

void ProtocolGame::sendOpenStore(Uint8 serviceType, const std::string& category)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameOpenStoreOpcode);
	if(g_game.hasGameFeature(GAME_FEATURE_STORE_SERVICETYPE))
	{
		msg.addU8(serviceType);
		msg.addString(category);
	}
	onSend(msg);
}

void ProtocolGame::sendRequestStore(Uint8 serviceType, const std::string& category)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameRequestStoreOpcode);
	if(g_clientVersion >= 1220)//Detect version
	{
		msg.addU8(serviceType);
		switch(serviceType)
		{
			case 1:
			{
				msg.addU8(0);//??
			}
			break;
			case 2:
			{
				msg.addString("");//??
				msg.addString("");//??
			}
			break;
			case 3:
			{
				msg.addU8(0);//??
			}
			break;
			case 4:
			{
				msg.addU32(0);//??
			}
			break;
			case 5:
			{
				msg.addString("");//??
			}
			break;
		}

		msg.addU8(0);//??
		msg.addU8(0);//??
	}
	else
	{
		if(g_game.hasGameFeature(GAME_FEATURE_STORE_SERVICETYPE))
			msg.addU8(serviceType);
		msg.addString(category);
	}
	onSend(msg);
}

void ProtocolGame::sendBuyInStore(Uint32 offerId, StoreProductTypes productType, const std::string& name)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameBuyStoreOpcode);
	msg.addU32(offerId);
	msg.addU8(SDL_static_cast(Uint8, productType));
	if(productType == ProductTypeNameChange)
		msg.addString(name);
	else if(productType == 9)
	{
		msg.addString("");//??
	}
	else if(productType == 10)
	{
		msg.addString("");//??
		msg.addU8(0);//??
	}
	else if(productType == 11)
	{
		msg.addString("");//??
		msg.addU8(0);//??
		msg.addString("");//??
	}
	else if(productType == 12)
	{
		msg.addString("");//??
	}
	onSend(msg);
}

void ProtocolGame::sendOpenTransactionHistory(Uint8 entriesPerPage)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameOpenTransactionStoreOpcode);
	msg.addU8(entriesPerPage);//16
	onSend(msg);
}

void ProtocolGame::sendRequestTransactionHistory(Uint32 page, Uint8 entriesPerPage)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameRequestTransactionStoreOpcode);
	if(g_clientVersion <= 1096)
	{
		msg.addU16(SDL_static_cast(Uint16, page));
		msg.addU8(SDL_static_cast(Uint32, entriesPerPage));
	}
	else
	{
		msg.addU32(page);
		msg.addU8(entriesPerPage);
	}
	onSend(msg);
}

void ProtocolGame::sendTransferCoins(const std::string& recipient, Uint32 amount)
{
	if(recipient.empty())
		return;

	OutputMessage msg(getHeaderPos());
	msg.addU8(GameTransferCoinsOpcode);
	msg.addString(recipient);
	msg.addU32(amount);
	onSend(msg);
}

void ProtocolGame::sendGetTransactionDetails(Uint32 unknown)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameGetTransactionDetailsOpcode);
	msg.addU32(unknown);
	onSend(msg);
}

void ProtocolGame::sendStoreEvent()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameStoreEventOpcode);

	Uint8 storeEventType = 0;
	msg.addU8(storeEventType);
	switch(storeEventType)
	{
		case 0:
		{
			msg.addU32(0);//??
		}
		break;
		case 1:
		{
			msg.addString("");//??
		}
		break;
		case 2:
		{
			msg.addU8(0);//??
		}
		break;
		case 3:
		{
			msg.addU32(0);//??
		}
		break;
	}
	onSend(msg);
}

void ProtocolGame::sendMarketLeave()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameMarketLeaveOpcode);
	onSend(msg);
}

void ProtocolGame::sendMarketBrowse(Uint16 browseId)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameMarketBrowseOpcode);
	msg.addU16(browseId);
	onSend(msg);
}

void ProtocolGame::sendMarketCreateOffer(Uint8 type, Uint16 thingId, Uint16 amount, Uint32 price, bool anonymous)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameMarketCreateOpcode);
	msg.addU8(type);
	msg.addU16(thingId);
	msg.addU16(amount);
	msg.addU32(price);
	msg.addBool(anonymous);
	onSend(msg);
}

void ProtocolGame::sendMarketCancelOffer(Uint32 timestamp, Uint16 counter)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameMarketCancelOpcode);
	msg.addU32(timestamp);
	msg.addU16(counter);
	onSend(msg);
}

void ProtocolGame::sendMarketAcceptOffer(Uint32 timestamp, Uint16 counter, Uint16 amount)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameMarketAcceptOpcode);
	msg.addU32(timestamp);
	msg.addU16(counter);
	msg.addU16(amount);
	onSend(msg);
}

void ProtocolGame::sendTrackQuestFlags(std::vector<Uint16>& missionIds)
{
	size_t missions = UTIL_min<size_t>(0xFF, missionIds.size());
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameTrackQuestFlagsOpcode);
	msg.addU8(SDL_static_cast(Uint8, missions));
	for(size_t i = 0; i < missions; ++i)
		msg.addU16(missionIds[i]);
	onSend(msg);
}

void ProtocolGame::sendOpenCyclopediaCharacterInfo(CyclopediaCharacterInfoType characterInfoType)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameOpenCyclopediaCharacterInfoOpcode);
	if(g_clientVersion >= 1220)
	{
		msg.addU32(0);//??
		msg.addU8(SDL_static_cast(Uint8, characterInfoType));
		if(characterInfoType == CHARACTERINFO_RECENTDEATHS || characterInfoType == CHARACTERINFO_RECENTPVPKILLS)
		{
			msg.addU16(0);//??
			msg.addU16(0);//??
		}
	}
	else
		msg.addU8(SDL_static_cast(Uint8, characterInfoType));
	onSend(msg);
}

void ProtocolGame::sendFeatureEvent(FeatureEventType type, bool active)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameFeatureEventOpcode);
	msg.addU8(SDL_static_cast(Uint8, type));
	msg.addBool(active);
	if(!active)
		msg.addU32(0);//timestamp??
	onSend(msg);
}

void ProtocolGame::sendInspectPlayer(InspectPlayerState playerState, Uint32 playerGUID)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameInspectPlayerOpcode);
	msg.addU8(SDL_static_cast(Uint8, playerState));
	if(playerState <= PLAYERSTATE_REVOKEPERMISSION)
		msg.addU32(playerGUID);
	onSend(msg);
}

void ProtocolGame::sendInspectObject(InspectObjectData& inspectData)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameInspectObjectOpcode);
	msg.addU8(inspectData.type);
	switch(inspectData.type)
	{
		case INSPECT_NORMALOBJECT:
		{
			Position position(inspectData.normalObject.posX, inspectData.normalObject.posY, inspectData.normalObject.posZ);
			msg.addPosition(position);
		}
		break;
		case INSPECT_NPCTRADE:
		case INSPECT_CYCLOPEDIA:
		{
			msg.addU16(inspectData.npcTrade.itemId);
			msg.addU8(inspectData.npcTrade.itemCount);
		}
		break;
		case INSPECT_UNKNOWN:
		{
			msg.addU8(0);//??
			msg.addU8(0);//??
		}
		break;
	}
	onSend(msg);
}

void ProtocolGame::sendApplyImbuement(Uint8 slot, Uint32 imbuementId, bool protectiveCharm)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameApplyImbuementOpcode);
	msg.addU8(slot);
	msg.addU32(imbuementId);
	msg.addBool(protectiveCharm);
	onSend(msg);
}

void ProtocolGame::sendClearCharm(Uint8 slot)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameClearCharmOpcode);
	msg.addU8(slot);
	onSend(msg);
}

void ProtocolGame::sendImbuementLeave()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameImbuementLeaveOpcode);
	onSend(msg);
}

void ProtocolGame::sendPreyAction(Uint8 preyId, PreyAction action, Uint8 listIndex)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GamePreyActionOpcode);
	msg.addU8(preyId);
	msg.addU8(SDL_static_cast(Uint8, action));
	if(action == PREYACTION_SELECTPREY)
		msg.addU8(listIndex);
	else if(action == PREYACTION_UNKNOWN4)
		msg.addU16(0);
	else if(action == PREYACTION_UNKNOWN5)
		msg.addU8(0);
	onSend(msg);
}

void ProtocolGame::sendMarketStatistics()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameMarketStatisticsOpcode);
	onSend(msg);
}

void ProtocolGame::sendOpenMonsterCyclopedia()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameOpenMonsterCyclopediaOpcode);
	onSend(msg);
}

void ProtocolGame::sendOpenMonsterCyclopediaMonsters(const std::string& race)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameOpenMonsterCyclopediaMonstersOpcode);
	if(g_clientVersion >= 1215)
	{
		Uint8 type = 0;
		msg.addU8(type);
		switch(type)
		{
			case 0:
			{
				msg.addString(race);
			}
			break;
			case 1:
			{
				msg.addU16(0);//size of uint16_t elements
				/*
				for(Uint16 i = 0; i < elements; ++i)
				{
					msg.addU16(0);//?
				}
				*/
			}
			break;
		}
	}
	else
		msg.addString(race);
	onSend(msg);
}

void ProtocolGame::sendOpenMonsterCyclopediaRace(Uint16 raceId)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameOpenMonsterCyclopediaRaceOpcode);
	msg.addU16(raceId);
	onSend(msg);
}

void ProtocolGame::sendCyclopediaMapAction()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameCyclopediaMapActionOpcode);

	Uint8 something = 0;
	if(!(something & 1))
	{
		if((something >> 1) & 1)
		{
			msg.addU8(1);//??
			msg.addU16(0);//??
			msg.addU32(0);//??
		}
	}
	else
	{
		msg.addU8(0);//??
		msg.addU16(0);//??
	}
	onSend(msg);
}

void ProtocolGame::sendMonsterBonusEffectAction(CyclopediaBonusEffectAction action, Uint8 charmId, Uint16 raceId)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameMonsterBonusEffectActionOpcode);
	msg.addU8(charmId);
	msg.addU8(SDL_static_cast(Uint8, action));
	if(action == CYCLOPEDIA_BONUSEFFECT_SELECT)
		msg.addU16(raceId);
	onSend(msg);
}

void ProtocolGame::sendSetHirelingName()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameSetHirelingNameOpcode);
	msg.addString("");//??
	msg.addU32(0);//??
	msg.addU32(0);//??
	onSend(msg);
}

void ProtocolGame::sendOpenRewardWall()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameOpenRewardWallOpcode);
	onSend(msg);
}

void ProtocolGame::sendOpenRewardHistory()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameOpenRewardHistoryOpcode);
	onSend(msg);
}

void ProtocolGame::sendColectReward(bool shrine, std::vector<RewardCollection>& rewards)
{
	size_t rewardSize = UTIL_min<size_t>(0xFF, rewards.size());
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameCollectRewardOpcode);
	msg.addBool(shrine);
	msg.addU8(SDL_static_cast(Uint8, rewardSize));
	for(size_t i = 0; i < rewardSize; ++i)
	{
		RewardCollection& reward = rewards[i];
		msg.addU16(reward.itemId);
		msg.addU8(reward.itemCount);
	}
	onSend(msg);
}

void ProtocolGame::sendQuickLoot(const Position& position, Uint16 itemId, Uint8 stackpos)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameQuickLootOpcode);
	msg.addPosition(position);
	msg.addU16(itemId);
	msg.addU8(stackpos);
	onSend(msg);
}

void ProtocolGame::sendLootContainer(LootContainerData& lootData)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameLootContainerOpcode);
	msg.addU8(lootData.action);
	switch(lootData.action)
	{
		case LOOTACTION_ASSIGNCONTAINER:
		{
			Position position(lootData.assignContainer.posX, lootData.assignContainer.posY, lootData.assignContainer.posZ);
			msg.addU8(lootData.assignContainer.objectCategory);
			msg.addPosition(position);
			msg.addU16(lootData.assignContainer.itemId);
			msg.addU8(lootData.assignContainer.itemCount);
		}
		break;
		case LOOTACTION_CLEARCONTAINER:
			msg.addU8(lootData.clearContainer.objectCategory);
			break;
		case LOOTACTION_SETUSEFALLBACK:
			msg.addBool(lootData.setUseFallback.useFallback);
			break;
	}
	onSend(msg);
}

void ProtocolGame::sendQuickLootBlackWhitelist(QuickLootFilter filter, std::vector<Uint16>& itemIds)
{
	size_t items = UTIL_min<size_t>(0xFFFF, itemIds.size());
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameQuickLootBlackWhitelistOpcode);
	msg.addU8(SDL_static_cast(Uint8, filter));
	msg.addU16(SDL_static_cast(Uint16, items));
	for(size_t i = 0; i < items; ++i)
		msg.addU16(itemIds[i]);
	onSend(msg);
}

void ProtocolGame::sendSubscribeToUpdates(bool unknownB, Uint8 unknownU)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameSubscribeToUpdatesOpcode);
	msg.addBool(unknownB);
	msg.addU8(unknownU);
	onSend(msg);
}

void ProtocolGame::sendMarkGameNewsAsRead(Uint32 newsId, bool readed)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameMarkGameNewsAsReadOpcode);
	msg.addU32(newsId);
	msg.addBool(readed);
	onSend(msg);
}

void ProtocolGame::sendTournamentInformation()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameTournamentInformationOpcode);
	onSend(msg);
}

void ProtocolGame::sendTournamentLeaderboard(const std::string& worldName, Uint16 currentPage, const std::string& searchCharacter, Uint8 elementsPerPage/* = 20*/)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameTournamentLeaderboardOpcode);

	Uint8 ledaerboardType = (searchCharacter.empty() ? 0 : 1);
	msg.addU8(ledaerboardType);
	switch(ledaerboardType)
	{
		case 0:
		{
			msg.addString(worldName);
			msg.addU16(currentPage);
		}
		break;
		case 1:
		{
			msg.addString(worldName);
			msg.addString(searchCharacter);
		}
		break;
	}
	msg.addU8(elementsPerPage);
	onSend(msg);
}

void ProtocolGame::sendTournamentTicketAction()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameTournamentTicketActionOpcode);
	msg.addU8(0);
	msg.addString("");
	msg.addU8(0);
	msg.addString("");
	onSend(msg);
}

void ProtocolGame::sendThankYou(Uint32 statementId)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameThankYouOpcode);
	msg.addU32(statementId);
	onSend(msg);
}

void ProtocolGame::sendGetOfferDescription(Uint32 offerId)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameGetOfferDescriptionOpcode);
	msg.addU32(offerId);
	onSend(msg);
}

void ProtocolGame::sendRequestResourceBalance(Uint8 resource)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameRequestResourceBalanceOpcode);
	msg.addU8(resource);
	onSend(msg);
}

void ProtocolGame::sendGreet(Uint32 statementId)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameGreetOpcode);
	msg.addU32(statementId);
	onSend(msg);
}

void ProtocolGame::sendGuildMessage()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameGuldMessageOpcode);
	onSend(msg);
}

void ProtocolGame::sendEditGuildMessage(const std::string& message)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameEditGuldMessageOpcode);
	msg.addString(message);
	onSend(msg);
}

void ProtocolGame::sendBlessingsDialog()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameBlessingsDialogOpcode);
	onSend(msg);
}

void ProtocolGame::sendClientCheck()
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameClientCheckOpcode);
	msg.addU32(0);//Bytes size + actual bytes = whatever
	onSend(msg);
}

void ProtocolGame::sendTeleport(const Position& position)
{
	OutputMessage msg(getHeaderPos());
	msg.addU8(GameTeleportOpcode);
	msg.addPosition(position);
	onSend(msg);
}

MessageMode ProtocolGame::translateMessageModeFromServer(Uint8 mode)
{
	if(g_clientVersion >= 1200 && mode == 0x31)
		return MessageBeyondLast;

	if(g_clientVersion >= 1094 && mode == 0x2B)
		return MessageMana;

	if(g_clientVersion >= 1055)
	{
		switch(mode)
		{
			case 0x00: return MessageNone;
			case 0x01: return MessageSay;
			case 0x02: return MessageWhisper;
			case 0x03: return MessageYell;
			case 0x04: return MessagePrivateFrom;
			case 0x05: return MessagePrivateTo;
			case 0x06: return MessageChannelManagement;
			case 0x07: return MessageChannel;
			case 0x08: return MessageChannelHighlight;
			case 0x09: return MessageSpell;
			case 0x0A: return MessageNpcFromStartBlock;
			case 0x0B: return MessageNpcFrom;
			case 0x0C: return MessageNpcTo;
			case 0x0D: return MessageGamemasterBroadcast;
			case 0x0E: return MessageGamemasterChannel;
			case 0x0F: return MessageGamemasterPrivateFrom;
			case 0x10: return MessageGamemasterPrivateTo;
			case 0x11: return MessageLogin;
			case 0x12: return MessageWarning;
			case 0x13: return MessageGame;
			case 0x14: return MessageGameHighlight;
			case 0x15: return MessageFailure;
			case 0x16: return MessageLook;
			case 0x17: return MessageDamageDealed;
			case 0x18: return MessageDamageReceived;
			case 0x19: return MessageHeal;
			case 0x1A: return MessageExp;
			case 0x1B: return MessageDamageOthers;
			case 0x1C: return MessageHealOthers;
			case 0x1D: return MessageExpOthers;
			case 0x1E: return MessageStatus;
			case 0x1F: return MessageLoot;
			case 0x20: return MessageTradeNpc;
			case 0x21: return MessageGuild;
			case 0x22: return MessagePartyManagement;
			case 0x23: return MessageParty;
			case 0x24: return MessageBarkLow;
			case 0x25: return MessageBarkLoud;
			case 0x26: return MessageReport;
			case 0x27: return MessageHotkeyUse;
			case 0x28: return MessageTutorialHint;
			case 0x29: return MessageThankyou;
			case 0x2A: return MessageMarket;
			default: break;
		}
	}
	else if(g_clientVersion >= 1036)
	{
		if(mode <= SDL_static_cast(Uint8, MessageMarket))
		{
			if(mode >= SDL_static_cast(Uint8, MessageNpcTo))
				return SDL_static_cast(MessageMode, mode - 1);
			else
				return SDL_static_cast(MessageMode, mode);
		}
	}
	else if(g_clientVersion >= 900)
	{
		if(mode <= SDL_static_cast(Uint8, MessageThankyou))
			return SDL_static_cast(MessageMode, mode);
	}
	else if(g_clientVersion >= 861)
	{
		switch(mode)
		{
			case 0x00: return MessageNone;
			case 0x01: return MessageSay;
			case 0x02: return MessageWhisper;
			case 0x03: return MessageYell;
			case 0x04: return MessageNpcTo;
			case 0x05: return MessageNpcFromStartBlock;
			case 0x06: return MessagePrivateFrom;
			case 0x07: return MessageChannel;
			case 0x08: return MessageChannelManagement;
			case 0x09: return MessageGamemasterBroadcast;
			case 0x0A: return MessageGamemasterChannel;
			case 0x0B: return MessageGamemasterPrivateFrom;
			case 0x0C: return MessageChannelHighlight;
			case 0x0D: return MessageMonsterSay;
			case 0x0E: return MessageMonsterYell;
			case 0x0F: return MessageWarning;
			case 0x10: return MessageGame;
			case 0x11: return MessageLogin;
			case 0x12: return MessageStatus;
			case 0x13: return MessageLook;
			case 0x14: return MessageFailure;
			case 0x15: return MessageBlue;
			case 0x16: return MessageRed;
			default: break;
		}
	}
	else if(g_clientVersion >= 840)
	{
		switch(mode)
		{
			case 0x00: return MessageNone;
			case 0x01: return MessageSay;
			case 0x02: return MessageWhisper;
			case 0x03: return MessageYell;
			case 0x04: return MessageNpcTo;
			case 0x05: return MessageNpcFromStartBlock;
			case 0x06: return MessagePrivateFrom;
			case 0x07: return MessageChannel;
			case 0x08: return MessageChannelManagement;
			case 0x09: return MessageRVRChannel;
			case 0x0A: return MessageRVRAnswer;
			case 0x0B: return MessageRVRContinue;
			case 0x0C: return MessageGamemasterBroadcast;
			case 0x0D: return MessageGamemasterChannel;
			case 0x0E: return MessageGamemasterPrivateFrom;
			case 0x0F: return MessageChannelHighlight;
			case 0x12: return MessageRed;
			case 0x13: return MessageMonsterSay;
			case 0x14: return MessageMonsterYell;
			case 0x15: return MessageWarning;
			case 0x16: return MessageGame;
			case 0x17: return MessageLogin;
			case 0x18: return MessageStatus;
			case 0x19: return MessageLook;
			case 0x1A: return MessageFailure;
			case 0x1B: return MessageBlue;
			default: break;
		}
	}
	else if(g_clientVersion >= 820)
	{
		switch(mode)
		{
			case 0x00: return MessageNone;
			case 0x01: return MessageSay;
			case 0x02: return MessageWhisper;
			case 0x03: return MessageYell;
			case 0x04: return MessageNpcTo;
			case 0x05: return MessageNpcFromStartBlock;
			case 0x06: return MessagePrivateFrom;
			case 0x07: return MessageChannel;
			case 0x08: return MessageRVRChannel;
			case 0x09: return MessageRVRAnswer;
			case 0x0A: return MessageRVRContinue;
			case 0x0B: return MessageGamemasterBroadcast;
			case 0x0C: return MessageGamemasterChannel;
			case 0x0D: return MessageGamemasterPrivateFrom;
			case 0x0E: return MessageChannelHighlight;
			case 0x11: return MessageRed;
			case 0x12: return MessageMonsterSay;
			case 0x13: return MessageMonsterYell;
			case 0x14: return MessageWarning;
			case 0x15: return MessageGame;
			case 0x16: return MessageLogin;
			case 0x17: return MessageStatus;
			case 0x18: return MessageLook;
			case 0x19: return MessageFailure;
			case 0x1A: return MessageBlue;
			default: break;
		}
	}
	else if(g_clientVersion >= 723)
	{
		switch(mode)
		{
			case 0x00: return MessageNone;
			case 0x01: return MessageSay;
			case 0x02: return MessageWhisper;
			case 0x03: return MessageYell;
			case 0x04: return MessagePrivateFrom;
			case 0x05: return MessageChannel;
			case 0x06: return MessageRVRChannel;
			case 0x07: return MessageRVRAnswer;
			case 0x08: return MessageRVRContinue;
			case 0x09: return MessageGamemasterBroadcast;
			case 0x0A: return MessageGamemasterChannel;
			case 0x0B: return MessageGamemasterPrivateFrom;
			case 0x0C: return MessageChannelHighlight;
			case 0x10: return MessageMonsterSay;
			case 0x11: return MessageMonsterYell;
			case 0x12: return MessageWarning;
			case 0x13: return MessageGame;
			case 0x14: return MessageLogin;
			case 0x15: return MessageStatus;
			case 0x16: return MessageLook;
			case 0x17: return MessageFailure;
			case 0x18: return MessageBlue;
			case 0x19: return MessageRed;
			default: break;
		}
	}
	else if(g_clientVersion >= 710)
	{
		switch(mode)
		{
			case 0x00: return MessageNone;
			case 0x01: return MessageSay;
			case 0x02: return MessageWhisper;
			case 0x03: return MessageYell;
			case 0x04: return MessagePrivateFrom;
			case 0x05: return MessageChannel;
			case 0x06: return MessageRVRChannel;
			case 0x07: return MessageRVRAnswer;
			case 0x08: return MessageRVRContinue;
			case 0x09: return MessageGamemasterBroadcast;
			case 0x0A: return MessageGamemasterPrivateFrom;
			case 0x0D: return MessageMonsterSay;
			case 0x0E: return MessageMonsterYell;
			case 0x0F: return MessageWarning;
			case 0x10: return MessageGame;
			case 0x11: return MessageLogin;
			case 0x12: return MessageStatus;
			case 0x13: return MessageLook;
			case 0x14: return MessageFailure;
			default: break;
		}
	}
	return MessageInvalid;
}

Uint8 ProtocolGame::translateMessageModeToServer(MessageMode mode)
{
	if(g_clientVersion >= 1200 && mode == MessageBeyondLast)
		return 0x31;

	if(g_clientVersion >= 1094 && mode == MessageMana)
		return 0x2B;

	if(g_clientVersion >= 1055)
	{
		switch(mode)
		{
			case MessageNone: return 0x00;
			case MessageSay: return 0x01;
			case MessageWhisper: return 0x02;
			case MessageYell: return 0x03;
			case MessagePrivateFrom: return 0x04;
			case MessagePrivateTo: return 0x05;
			case MessageChannelManagement: return 0x06;
			case MessageChannel: return 0x07;
			case MessageChannelHighlight: return 0x08;
			case MessageSpell: return 0x09;
			case MessageNpcFromStartBlock: return 0x0A;
			case MessageNpcFrom: return 0x0B;
			case MessageNpcTo: return 0x0C;
			case MessageGamemasterBroadcast: return 0x0D;
			case MessageGamemasterChannel: return 0x0E;
			case MessageGamemasterPrivateFrom: return 0x0F;
			case MessageGamemasterPrivateTo: return 0x10;
			case MessageLogin: return 0x11;
			case MessageWarning: return 0x12;
			case MessageGame: return 0x13;
			case MessageGameHighlight: return 0x14;
			case MessageFailure: return 0x15;
			case MessageLook: return 0x16;
			case MessageDamageDealed: return 0x17;
			case MessageDamageReceived: return 0x18;
			case MessageHeal: return 0x19;
			case MessageExp: return 0x1A;
			case MessageDamageOthers: return 0x1B;
			case MessageHealOthers: return 0x1C;
			case MessageExpOthers: return 0x1D;
			case MessageStatus: return 0x1E;
			case MessageLoot: return 0x1F;
			case MessageTradeNpc: return 0x20;
			case MessageGuild: return 0x21;
			case MessagePartyManagement: return 0x22;
			case MessageParty: return 0x23;
			case MessageBarkLow: return 0x24;
			case MessageBarkLoud: return 0x25;
			case MessageReport: return 0x26;
			case MessageHotkeyUse: return 0x27;
			case MessageTutorialHint: return 0x28;
			case MessageThankyou: return 0x29;
			case MessageMarket: return 0x2A;
			default: break;
		}
	}
	else if(g_clientVersion >= 1036)
	{
		if(mode <= MessageThankyou)
		{
			if(mode >= MessageNpcTo)
				return SDL_static_cast(Uint8, mode + 1);
			else
				return SDL_static_cast(Uint8, mode);
		}
	}
	else if(g_clientVersion >= 900)
	{
		if(mode <= MessageThankyou)
			return SDL_static_cast(Uint8, mode);
	}
	else if(g_clientVersion >= 861)
	{
		switch(mode)
		{
			case MessageNone: return 0x00;
			case MessageSay: return 0x01;
			case MessageWhisper: return 0x02;
			case MessageYell: return 0x03;
			case MessageNpcTo: return 0x04;
			case MessageNpcFromStartBlock: return 0x05;
			case MessagePrivateFrom: return 0x06;
			case MessagePrivateTo: return 0x06;
			case MessageChannel: return 0x07;
			case MessageChannelManagement: return 0x08;
			case MessageGamemasterBroadcast: return 0x09;
			case MessageGamemasterChannel: return 0x0A;
			case MessageGamemasterPrivateFrom: return 0x0B;
			case MessageGamemasterPrivateTo: return 0x0B;
			case MessageChannelHighlight: return 0x0C;
			case MessageMonsterSay: return 0x0D;
			case MessageMonsterYell: return 0x0E;
			case MessageWarning: return 0x0F;
			case MessageGame: return 0x10;
			case MessageLogin: return 0x11;
			case MessageStatus: return 0x12;
			case MessageLook: return 0x13;
			case MessageFailure: return 0x14;
			case MessageBlue: return 0x15;
			case MessageRed: return 0x16;
			default: break;
		}
	}
	else if(g_clientVersion >= 840)
	{
		switch(mode)
		{
			case MessageNone: return 0x00;
			case MessageSay: return 0x01;
			case MessageWhisper: return 0x02;
			case MessageYell: return 0x03;
			case MessageNpcTo: return 0x04;
			case MessageNpcFromStartBlock: return 0x05;
			case MessagePrivateFrom: return 0x06;
			case MessagePrivateTo: return 0x06;
			case MessageChannel: return 0x07;
			case MessageChannelManagement: return 0x08;
			case MessageRVRChannel: return 0x09;
			case MessageRVRAnswer: return 0x0A;
			case MessageRVRContinue: return 0x0B;
			case MessageGamemasterBroadcast: return 0x0C;
			case MessageGamemasterChannel: return 0x0D;
			case MessageGamemasterPrivateFrom: return 0x0E;
			case MessageGamemasterPrivateTo: return 0x0E;
			case MessageChannelHighlight: return 0x0F;
			case MessageRed: return 0x12;
			case MessageMonsterSay: return 0x13;
			case MessageMonsterYell: return 0x14;
			case MessageWarning: return 0x15;
			case MessageGame: return 0x16;
			case MessageLogin: return 0x17;
			case MessageStatus: return 0x18;
			case MessageLook: return 0x19;
			case MessageFailure: return 0x1A;
			case MessageBlue: return 0x1B;
			default: break;
		}
	}
	else if(g_clientVersion >= 820)
	{
		switch(mode)
		{
			case MessageNone: return 0x00;
			case MessageSay: return 0x01;
			case MessageWhisper: return 0x02;
			case MessageYell: return 0x03;
			case MessageNpcTo: return 0x04;
			case MessageNpcFromStartBlock: return 0x05;
			case MessagePrivateFrom: return 0x06;
			case MessagePrivateTo: return 0x06;
			case MessageChannel: return 0x07;
			case MessageRVRChannel: return 0x08;
			case MessageRVRAnswer: return 0x09;
			case MessageRVRContinue: return 0x0A;
			case MessageGamemasterBroadcast: return 0x0B;
			case MessageGamemasterChannel: return 0x0C;
			case MessageGamemasterPrivateFrom: return 0x0D;
			case MessageGamemasterPrivateTo: return 0x0D;
			case MessageChannelHighlight: return 0x0E;
			case MessageRed: return 0x11;
			case MessageMonsterSay: return 0x12;
			case MessageMonsterYell: return 0x13;
			case MessageWarning: return 0x14;
			case MessageGame: return 0x15;
			case MessageLogin: return 0x16;
			case MessageStatus: return 0x17;
			case MessageLook: return 0x18;
			case MessageFailure: return 0x19;
			case MessageBlue: return 0x1A;
			default: break;
		}
	}
	else if(g_clientVersion >= 723)
	{
		switch(mode)
		{
			case MessageNone: return 0x00;
			case MessageSay: return 0x01;
			case MessageWhisper: return 0x02;
			case MessageYell: return 0x03;
			case MessagePrivateFrom: return 0x04;
			case MessagePrivateTo: return 0x04;
			case MessageChannel: return 0x05;
			case MessageRVRChannel: return 0x06;
			case MessageRVRAnswer: return 0x07;
			case MessageRVRContinue: return 0x08;
			case MessageGamemasterBroadcast: return 0x09;
			case MessageGamemasterChannel: return 0x0A;
			case MessageGamemasterPrivateFrom: return 0x0B;
			case MessageGamemasterPrivateTo: return 0x0B;
			case MessageChannelHighlight: return 0x0C;
			case MessageMonsterSay: return 0x10;
			case MessageMonsterYell: return 0x11;
			case MessageWarning: return 0x12;
			case MessageGame: return 0x13;
			case MessageLogin: return 0x14;
			case MessageStatus: return 0x15;
			case MessageLook: return 0x16;
			case MessageFailure: return 0x17;
			case MessageBlue: return 0x18;
			case MessageRed: return 0x19;
			default: break;
		}
	}
	else if(g_clientVersion >= 710)
	{
		switch(mode)
		{
			case MessageNone: return 0x00;
			case MessageSay: return 0x01;
			case MessageWhisper: return 0x02;
			case MessageYell: return 0x03;
			case MessagePrivateFrom: return 0x04;
			case MessagePrivateTo: return 0x04;
			case MessageChannel: return 0x05;
			case MessageRVRChannel: return 0x06;
			case MessageRVRAnswer: return 0x07;
			case MessageRVRContinue: return 0x08;
			case MessageGamemasterBroadcast: return 0x09;
			case MessageGamemasterPrivateFrom: return 0x0A;
			case MessageGamemasterPrivateTo: return 0x0A;
			case MessageMonsterSay: return 0x0D;
			case MessageMonsterYell: return 0x0E;
			case MessageWarning: return 0x0F;
			case MessageGame: return 0x10;
			case MessageLogin: return 0x11;
			case MessageStatus: return 0x12;
			case MessageLook: return 0x13;
			case MessageFailure: return 0x14;
			default: break;
		}
	}
	return SDL_static_cast(Uint8, MessageInvalid);
}

Thing* ProtocolGame::internalGetThing(InputMessage& msg, const Position& pos)
{
	Thing* thing = NULL;
	Uint16 thingId = msg.getU16();
	if(thingId == 0x61 || thingId == 0x62 || thingId == 0x63)
		thing = getCreature(msg, thingId, pos);
	else
		thing = getItem(msg, thingId, pos);

	return thing;
}

Creature* ProtocolGame::getCreature(InputMessage& msg, Uint16 thingId, const Position&)
{
	Creature* creature;
	switch(thingId)
	{
		case 0x61:
		{
			Uint32 removeId = msg.getU32();
			g_map.removeCreatureById(removeId);

			Uint32 creatureId = msg.getU32();
			Uint8 creatureType;
			if(g_clientVersion >= 910)
				creatureType = msg.getU8();
			else
			{
				if(creatureId >= 0x10000000 && creatureId < 0x40000000)
					creatureType = CREATURETYPE_PLAYER;
				else if(creatureId >= 0x40000000 && creatureId < 0x80000000)
					creatureType = CREATURETYPE_MONSTER;
				else
					creatureType = CREATURETYPE_NPC;
			}

			if(g_clientVersion >= 1121 && creatureType == CREATURETYPE_SUMMON_OWN)
			{
				Uint32 masterId = msg.getU32();
				if(masterId != g_game.getPlayerID())
					creatureType = CREATURETYPE_SUMMON_OTHERS;
			}

			const std::string& creatureName = UTIL_formatCreatureName(msg.getString());
			creature = new Creature();
			if(creature)
			{
				creature->setId(creatureId);
				creature->setType(creatureType);
				creature->setName(creatureName);
				g_map.addCreatureById(creatureId, creature);

				//If somehow server doesn't send playerid the server is not worth to use with our client
				if(g_game.getPlayerID() == creatureId)
				{
					g_map.setLocalCreature(creature);
					creature->setLocalCreature(true);
				}
			}
		}
		break;
		case 0x62:
		{
			Uint32 creatureId = msg.getU32();
			creature = g_map.getCreatureById(creatureId);
		}
		break;
		case 0x63:
		{
			Uint32 creatureId = msg.getU32();
			Uint8 direction = msg.getU8();
			creature = g_map.getCreatureById(creatureId);
			if(creature)
				creature->turnDirection(SDL_static_cast(Direction, direction));
			if(g_clientVersion >= 953)
			{
				bool unpassable = msg.getBool();
				if(creature)
					creature->setUnpassable(unpassable);
			}
		}
		return creature;
		default: return NULL;
	}

	Uint8 health = msg.getU8();
	Uint8 direction = msg.getU8();

	Uint16 lookType;
	if(g_game.hasGameFeature(GAME_FEATURE_LOOKTYPE_U16))
		lookType = msg.getU16();
	else
		lookType = SDL_static_cast(Uint16, msg.getU8());

	Uint16 lookTypeEx = 0;
	Uint8 lookHead = 0;
	Uint8 lookBody = 0;
	Uint8 lookLegs = 0;
	Uint8 lookFeet = 0;
	Uint8 lookAddons = 0;
	if(lookType != 0)
	{
		lookHead = msg.getU8();
		lookBody = msg.getU8();
		lookLegs = msg.getU8();
		lookFeet = msg.getU8();
		if(g_game.hasGameFeature(GAME_FEATURE_ADDONS))
			lookAddons = msg.getU8();
	}
	else
		lookTypeEx = msg.getU16();

	Uint16 lookMount = 0;
	if(g_game.hasGameFeature(GAME_FEATURE_MOUNTS))
		lookMount = msg.getU16();

	Uint16 light[2];
	light[0] = SDL_static_cast(Uint16, msg.getU8());
	light[1] = SDL_static_cast(Uint16, msg.getU8());

	Uint16 speed = msg.getU16();
	Uint8 skull = msg.getU8();
	Uint8 shield = msg.getU8();

	Uint8 emblem = 0;
	Uint8 icon = 0;
	bool unpassable = true;
	if(g_game.hasGameFeature(GAME_FEATURE_CREATURE_EMBLEM) && thingId == 0x61)
		emblem = msg.getU8();

	if(g_game.hasGameFeature(GAME_FEATURE_CREATURE_TYPE))
	{
		Uint8 creatureType = msg.getU8();
		if(creature)
			creature->setType(creatureType);

		if(g_clientVersion >= 1121 && creatureType == CREATURETYPE_SUMMON_OWN)
		{
			Uint32 masterId = msg.getU32();
			if(masterId != g_game.getPlayerID())
				creatureType = CREATURETYPE_SUMMON_OTHERS;
		}

		if(g_clientVersion >= 1220 && creatureType == CREATURETYPE_PLAYER)
		{
			//We need %10 because cipsoft declare promoted vocation as vocationid + 10
			//so in case you need to check promotion just check if >= 10
			Uint8 vocation = msg.getU8() % 10;
			if(creature)
				creature->setVocation(vocation);
		}
	}

	if(g_game.hasGameFeature(GAME_FEATURE_CREATURE_ICONS))
		icon = msg.getU8();

	if(g_game.hasGameFeature(GAME_FEATURE_CREATURE_MARK))
	{
		Uint8 markType = msg.getU8();
		if(g_game.hasGameFeature(GAME_FEATURE_INSPECTION))
			msg.getU8();

		if(g_clientVersion < 1185)
		{
			Uint16 helpers = msg.getU16();
			if(creature)
				creature->setHelpers(helpers);
		}

		if(creature)
		{
			if(markType == 0xFF)
				creature->hideStaticSquare();
			else
				creature->showStaticSquare(markType);
		}
	}

	if(g_clientVersion >= 854)
		unpassable = msg.getBool();

	if(creature)
	{
		creature->setHealth(health);
		creature->turnDirection(SDL_static_cast(Direction, direction));
		creature->setOutfit(lookType, lookTypeEx, lookHead, lookBody, lookLegs, lookFeet, lookAddons, lookMount);
		creature->setSpeed(speed);
		creature->setSkull(skull);
		creature->setShield(shield);
		creature->setUnpassable(unpassable);
		creature->setLight(light);
		creature->setIcon(icon);
		if(thingId == 0x61)
			creature->setEmblem(emblem);
	}
	return creature;
}

Item* ProtocolGame::getItem(InputMessage& msg, Uint16 thingId, const Position& pos)
{
	if(g_game.hasGameFeature(GAME_FEATURE_ITEM_MARK))
		msg.getU8();

	ThingType* ttype = g_thingManager.getThingType(ThingCategory_Item, thingId);
	if(!ttype)
		return Item::createItem(pos, thingId, 0);//Create null item :)

	Uint16 count = 1;
	if(ttype->hasFlag(ThingAttribute_Stackable) || ttype->hasFlag(ThingAttribute_Chargeable))
	{
		if(g_game.hasGameFeature(GAME_FEATURE_ITEMS_U16))
			count = msg.getU16();
		else
			count = SDL_static_cast(Uint16, msg.getU8());
	}
	else if(ttype->hasFlag(ThingAttribute_Splash) || ttype->hasFlag(ThingAttribute_FluidContainer))
		count = SDL_static_cast(Uint16, msg.getU8());
	else if(ttype->hasFlag(ThingAttribute_Hangable))
	{
		Tile* tile = g_map.getTile(pos);
		if(tile)
		{
			if(tile->mustHookSouth())
				count = 2;
			else if(tile->mustHookEast())
				count = 3;
		}
	}

	Sint32 phase = AnimationPhase_Automatic;
	if(g_game.hasGameFeature(GAME_FEATURE_ITEM_ANIMATION_PHASES))
	{
		if(ttype->m_frameGroup[ThingFrameGroup_Default].m_animCount > 1)
		{
			Uint8 startPhase = msg.getU8();
			if(startPhase == 0xFF)
				phase = AnimationPhase_Async;
			else if(startPhase == 0xFE)
				phase = AnimationPhase_Random;
			else
				phase = SDL_static_cast(Sint32, startPhase);
		}
	}

	if(g_game.hasGameFeature(GAME_FEATURE_QUICK_LOOT) && ttype->hasFlag(ThingAttribute_Container))
	{
		bool assignedToQuickLoot = msg.getBool();
		if(assignedToQuickLoot)
			msg.getU32();
	}
	return Item::createItem(pos, thingId, count, phase);
}

ItemUI* ProtocolGame::getItemUI(InputMessage& msg, Uint16 thingId)
{
	if(g_game.hasGameFeature(GAME_FEATURE_ITEM_MARK))
		msg.getU8();

	ThingType* type = g_thingManager.getThingType(ThingCategory_Item, thingId);
	if(!type)
		return NULL;

	Uint16 count = 1;
	if(type->hasFlag(ThingAttribute_Stackable) || type->hasFlag(ThingAttribute_Chargeable))
	{
		if(g_game.hasGameFeature(GAME_FEATURE_ITEMS_U16))
			count = msg.getU16();
		else
			count = SDL_static_cast(Uint16, msg.getU8());
	}
	else if(type->hasFlag(ThingAttribute_Splash) || type->hasFlag(ThingAttribute_FluidContainer))
		count = SDL_static_cast(Uint16, msg.getU8());

	Sint32 phase = AnimationPhase_Automatic;
	if(g_game.hasGameFeature(GAME_FEATURE_ITEM_ANIMATION_PHASES))
	{
		if(type->m_frameGroup[ThingFrameGroup_Default].m_animCount > 1)
		{
			Uint8 startPhase = msg.getU8();
			if(startPhase == 0xFF)
				phase = AnimationPhase_Async;
			else if(startPhase == 0xFE)
				phase = AnimationPhase_Random;
			else
				phase = SDL_static_cast(Sint32, startPhase);
		}
	}

	if(g_game.hasGameFeature(GAME_FEATURE_QUICK_LOOT) && type->hasFlag(ThingAttribute_Container))
	{
		bool assignedToQuickLoot = msg.getBool();
		if(assignedToQuickLoot)
			msg.getU32();
	}
	return ItemUI::createItemUI(type, count, phase);
}

void ProtocolGame::setMapDescription(InputMessage& msg, Sint32 x, Sint32 y, Uint8 z, Sint32 width, Sint32 height)
{
	Sint32 startz, endz, zstep;
	if(z > GAME_PLAYER_FLOOR)
	{
		startz = z - GAME_MAP_AWARERANGE;
		endz = UTIL_min<Sint32>(SDL_static_cast(Sint32, z) + GAME_MAP_AWARERANGE, GAME_MAP_FLOORS);
		zstep = 1;
	}
	else
	{
		startz = GAME_PLAYER_FLOOR;
		endz = 0;
		zstep = -1;
	}

	Sint32 skip = 0;
	for(Sint32 nz = startz; nz != endz + zstep; nz += zstep)
		skip = setFloorDescription(msg, x, y, SDL_static_cast(Uint8, nz), width, height, SDL_static_cast(Sint32, z) - nz, skip);
}

Sint32 ProtocolGame::setFloorDescription(InputMessage& msg, Sint32 x, Sint32 y, Uint8 z, Sint32 width, Sint32 height, Sint32 offset, Sint32 skip)
{
	for(Sint32 nx = 0; nx < width; ++nx)
	{
		for(Sint32 ny = 0; ny < height; ++ny)
		{
			Position tilePos(SDL_static_cast(Uint16, x + nx + offset), SDL_static_cast(Uint16, y + ny + offset), z);
			if(skip == 0)
				skip = setTileDescription(msg, tilePos, offset);
			else
			{
				g_map.cleanTile(tilePos, offset);
				updateMinimapTile(tilePos, NULL);
				--skip;
			}
		}
	}
	return skip;
}

Sint32 ProtocolGame::setTileDescription(InputMessage& msg, const Position& position, Sint32 offset)
{
	Tile* tile = g_map.resetTile(position, offset);
	if(!tile)
	{
		Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(X: %u, Y: %u, Z: %u, Offset: %d).", "[ProtocolGame::setTileDescription] Tile not found", SDL_static_cast(Uint32, position.x), SDL_static_cast(Uint32, position.y), SDL_static_cast(Uint32, position.z), offset);
		g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
	}

	bool gotEffect = false;
	for(Sint32 stackPos = 0; stackPos < 256; ++stackPos)
	{
		if(msg.peekU16() >= 0xFF00)
		{
			updateMinimapTile(position, tile);
			return (msg.getU16() & 0xFF);
		}

		if(g_game.hasGameFeature(GAME_FEATURE_ENVIRONMENT_EFFECTS) && !gotEffect)
		{
			msg.getU16(); //Environment effect
			gotEffect = true;
			continue;
		}

		if(stackPos > 10)
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s(X: %u, Y: %u, Z: %u, Stack: %u).", "[ProtocolGame::setTileDescription] Too many things", SDL_static_cast(Uint32, position.x), SDL_static_cast(Uint32, position.y), SDL_static_cast(Uint32, position.z), SDL_static_cast(Uint32, stackPos));
			g_logger.addLog(LOG_CATEGORY_ERROR, std::string(g_buffer, SDL_static_cast(size_t, len)));
		}

		Thing* thing = internalGetThing(msg, position);
		if(tile)
			tile->addThing(thing);
	}
	updateMinimapTile(position, tile);
	return 0;
}

void ProtocolGame::updateMinimapTile(const Position& position, Tile* tile)
{
	if(position.z == g_map.getCentralPosition().z)
	{
		Uint8 minimapColor = 0, groundSpeed = 0;
		if(tile)
		{
			minimapColor = SDL_static_cast(Uint8, tile->getMinimapColor());
			groundSpeed = SDL_static_cast(Uint8, tile->getMinimapSpeed());
		}

		g_automap.setTileDetail(position.x, position.y, position.z, minimapColor, groundSpeed);
	}
}
