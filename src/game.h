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

#ifndef __FILE_GAME_h_
#define __FILE_GAME_h_

#include "position.h"

enum Stats_Cached_State : Uint16
{
	CACHED_STAT_LEVEL = (1 << 0),
	CACHED_STAT_EXPERIENCE = (1 << 1),
	CACHED_STAT_CAPACITY = (1 << 2),
	CACHED_STAT_HEALTH = (1 << 3),
	CACHED_STAT_MANA = (1 << 4),
	CACHED_STAT_MAGICLEVEL = (1 << 5),
	CACHED_STAT_STAMINA = (1 << 6),
	CACHED_STAT_SOUL = (1 << 7),
	CACHED_STAT_SPEED = (1 << 8),
	CACHED_STAT_REGENERATION = (1 << 9),
	CACHED_STAT_TRAINING = (1 << 10),
	CACHED_STAT_XPGAIN = (1 << 11)
};

enum Skills_Cached_State : Uint16
{
	CACHED_SKILL_FIST = (1 << 0),
	CACHED_SKILL_CLUB = (1 << 1),
	CACHED_SKILL_SWORD = (1 << 2),
	CACHED_SKILL_AXE = (1 << 3),
	CACHED_SKILL_DISTANCE = (1 << 4),
	CACHED_SKILL_SHIELDING = (1 << 5),
	CACHED_SKILL_FISHING = (1 << 6),
	CACHED_SKILL_CRITICAL = (1 << 7),
	CACHED_SKILL_LIFELEECH = (1 << 8),
	CACHED_SKILL_MANALEECH = (1 << 9)
};

class Creature;
class ItemUI;
class Container;
class Game
{
	public:
		Game();
		~Game();

		void reset();

		void clientChangeVersion(Uint32 clientVersion, Uint32 fileVersion);
		bool hasGameFeature(GameFeatures feature) {return m_gameFeatures[feature];}
		void enableGameFeature(GameFeatures feature) {m_gameFeatures.set(feature, true);}
		void disableGameFeature(GameFeatures feature) {m_gameFeatures.set(feature, false);}

		void processGMActions(std::vector<Uint8> privileges);
		void processLoginWait(const std::string& message, Uint8 time);
		void processDeath(Uint8 deathType, Uint8 penalty);
		void processContainerOpen(Uint8 containerId, ItemUI* item, const std::string& name, Uint8 capacity, bool hasParent, bool canUseDepotSearch, bool isUnlocked, bool hasPages, Uint16 containerSize, Uint16 firstIndex, std::vector<ItemUI*>& itemVector);
		void processContainerClose(Uint8 containerId);
		void processContainerAddItem(Uint8 containerId, Uint16 slot, ItemUI* item);
		void processContainerTransformItem(Uint8 containerId, Uint16 slot, ItemUI* item);
		void processContainerRemoveItem(Uint8 containerId, Uint16 slot, ItemUI* lastItem);
		void processTransformInventoryItem(Uint8 slot, ItemUI* item);
		void processTradeOwn(const std::string& name, std::vector<ItemUI*>& itemVector);
		void processTradeCounter(const std::string& name, std::vector<ItemUI*>& itemVector);
		void processTradeClose();
		void processEditTextWindow(Uint32 windowId, ItemUI* item, Uint16 maxLen, const std::string& text, const std::string& writer, const std::string& date);
		void processEditHouseWindow(Uint8 doorId, Uint32 windowId, const std::string& text);
		void processTextMessage(MessageMode mode, const std::string& text, Uint32 channelId = 0);
		void processWalkCancel(Direction direction);
		void processOutfits(Uint16 lookType, Uint8 lookHead, Uint8 lookBody, Uint8 lookLegs, Uint8 lookFeet, Uint8 lookAddons, Uint16 lookMount, std::vector<OutfitDetail>& outfits, std::vector<MountDetail>& mounts);
		void processTutorialHint(Uint8 tutorialId);
		void processCancelTarget(Uint32 sequence);
		void processSpellDelay(Uint8 spellId, Uint32 delay);
		void processSpellGroupDelay(Uint8 groupId, Uint32 delay);
		void processMultiUseDelay(Uint32 delay);
		void processPlayerModes(Uint8 fightMode, Uint8 chaseMode, Uint8 safeMode, Uint8 pvpMode);
		void processTalk(const std::string& name, Uint32 statementId, Uint16 playerLevel, MessageMode mode, std::string& text, Uint32 channelId, const Position& position);
		void processChannelList(std::vector<ChannelDetail>& channels);
		void processOpenChannel(Uint16 channelId, const std::string& channelName);
		void processOpenPrivateChannel(const std::string& receiver);
		void processOpenOwnPrivateChannel(Uint16 channelId, const std::string& channelName);
		void processCloseChannel(Uint16 channelId);

		void sendLogout();
		void sendAutoWalk(const std::vector<Direction>& path);
		void sendWalk(Direction dir);
		void sendTurn(Direction dir);
		void sendSay(MessageMode mode, Uint16 channelId, const std::string& receiver, const std::string& message);
		void sendRequestChannels();
		void sendCreatePrivateChannel();
		void sendOpenChannel(Uint16 channelId);
		void sendCloseChannel(Uint16 channelId);
		void sendOpenPrivateChannel(const std::string& receiver);
		void sendCloseNPCChannel();
		void sendChannelInvite(const std::string& target, Uint16 channelId);
		void sendChannelExclude(const std::string& target, Uint16 channelId);
		void sendOpenQuestLog();
		void sendOpenQuestLine(Uint16 questId);
		void sendPing();
		void sendPingBack();
		void sendAttackModes();
		void sendAttack(Creature* creature);
		void sendFollow(Creature* creature);
		void sendCancelAttackAndFollow();
		void sendJoinAggression(Uint32 creatureId);
		void sendInviteToParty(Uint32 creatureId);
		void sendJoinToParty(Uint32 creatureId);
		void sendRevokePartyInvitation(Uint32 creatureId);
		void sendPassPartyLeadership(Uint32 creatureId);
		void sendLeaveParty();
		void sendEnableSharedPartyExperience(bool active);
		void sendRequestOutfit();
		void sendSetOutfit(Uint16 lookType, Uint8 lookHead, Uint8 lookBody, Uint8 lookLegs, Uint8 lookFeet, Uint8 lookAddons, Uint16 lookMount);
		void sendMount(bool active);
		void sendEquipItem(Uint16 itemid, Uint16 count);
		void sendMove(const Position& fromPos, Uint16 itemid, Uint8 stackpos, const Position& toPos, Uint16 count);
		void sendLookAt(const Position& position, Uint16 itemId, Uint8 stackpos);
		void sendLookInBattle(Uint32 creatureId);
		void sendUseItem(const Position& position, Uint16 itemId, Uint8 stackpos, Uint8 index);
		void sendUseItemEx(const Position& fromPos, Uint16 itemId, Uint8 fromStackPos, const Position& toPos, Uint16 toItemId, Uint8 toStackPos);
		void sendUseOnCreature(const Position& position, Uint16 itemId, Uint8 stackpos, Uint32 creatureId);
		void sendRotateItem(const Position& position, Uint16 itemId, Uint8 stackpos);
		void sendWrapState(const Position& position, Uint16 itemId, Uint8 stackpos);
		void sendCloseContainer(Uint8 containerId);
		void sendUpContainer(Uint8 containerId);
		void sendUpdateContainer(Uint8 containerId);
		void sendSeekInContainer(Uint8 containerId, Uint16 index);
		void sendBrowseField(const Position& position);
		void sendOpenParentContainer(const Position& position);
		void sendTextWindow(Uint32 windowId, const std::string& text);
		void sendHouseWindow(Uint32 windowId, Uint8 doorId, const std::string& text);

		void stopActions();
		void startAutoWalk(const Position& toPosition);
		void stopAutoWalk();
		void checkServerMovement(Direction dir);
		void checkLocalCreatureMovement();
		void checkMovement(Direction dir);
		void releaseMovement();

		void switchToNextChannel();
		void switchToPreviousChannel();
		void closeCurrentChannel();
		void openHelpChannel();
		void openNPCChannel();
		void switchToDefault();

		void minimapCenter();
		void minimapFloorDown();
		void minimapFloorUp();
		void minimapScrollEast();
		void minimapScrollNorth();
		void minimapScrollSouth();
		void minimapScrollWest();
		void minimapScrollNorthWest();
		void minimapScrollNorthEast();
		void minimapScrollSouthWest();
		void minimapScrollSouthEast();
		void minimapZoomIn();
		void minimapZoomOut();

		Container* findContainer(Uint8 cid) {return m_containers[cid];}
		Uint8 findEmptyContainerId();
		bool containerHasParent(Uint8 cid);

		SDL_FORCE_INLINE ItemUI* getInventoryItem(Uint8 slot) {return m_inventoryItem[slot];}

		SDL_INLINE void setPlayerID(Uint32 localPlayerId) {m_localPlayerId = localPlayerId;}
		SDL_FORCE_INLINE Uint32 getPlayerID() {return m_localPlayerId;}

		SDL_INLINE void setAttackID(Uint32 attackId) {m_attackId = attackId;}
		SDL_FORCE_INLINE Uint32 getAttackID() {return m_attackId;}

		SDL_INLINE void setFollowID(Uint32 followId) {m_followId = followId;}
		SDL_FORCE_INLINE Uint32 getFollowID() {return m_followId;}

		SDL_INLINE void setSelectID(Uint32 selectId) {m_selectId = selectId;}
		SDL_FORCE_INLINE Uint32 getSelectID() {return m_selectId;}

		void resetPlayerExperienceTable();
		void updatePlayerExperienceTable();
		SDL_FORCE_INLINE double getPlayerExperienceSpeed() {return m_playerExpSpeed;}
		std::string getPlayerName();

		void setPlayerExperience(Uint64 experience);
		SDL_FORCE_INLINE Uint64 getPlayerExperience() {return m_playerExperience;}
		SDL_FORCE_INLINE Uint64 getExpForLevel(Sint32 lvl) {Uint64 lv = SDL_static_cast(Uint64, lvl-1); return ((50*lv*lv*lv)-(150*lv*lv)+(400*lv))/3;}

		void setPlayerCapacity(double capacity, double totalCapacity);
		SDL_FORCE_INLINE double getPlayerCapacity() {return m_playerCapacity;}
		SDL_FORCE_INLINE double getPlayerTotalCapacity() {return m_playerTotalCapacity;}

		void setPlayerHealth(Uint32 health, Uint32 maxHealth);
		SDL_FORCE_INLINE Uint32 getPlayerHealth() {return m_playerHealth;}
		SDL_FORCE_INLINE Uint32 getPlayerMaxHealth() {return m_playerMaxHealth;}
		SDL_FORCE_INLINE Uint8 getPlayerHealthPercent() {return m_playerHealthPercent;}

		void setPlayerMana(Uint32 mana, Uint32 maxMana);
		SDL_FORCE_INLINE Uint32 getPlayerMana() {return m_playerMana;}
		SDL_FORCE_INLINE Uint32 getPlayerMaxMana() {return m_playerMaxMana;}
		SDL_FORCE_INLINE Uint8 getPlayerManaPercent() {return m_playerManaPercent;}

		void setPlayerLevel(Uint16 level, Uint8 levelPercent);
		SDL_FORCE_INLINE Uint16 getPlayerLevel() {return m_playerLevel;}
		SDL_FORCE_INLINE Uint8 getPlayerLevelPercent() {return m_playerLevelPercent;}

		void setPlayerMagicLevel(Uint16 level, Uint16 baseLevel, Uint8 levelPercent);
		SDL_FORCE_INLINE Uint16 getPlayerMagicLevel() {return m_playerMagicLevel;}
		SDL_FORCE_INLINE Uint16 getPlayerBaseMagicLevel() {return m_playerBaseMagicLevel;}
		SDL_FORCE_INLINE Uint8 getPlayerMagicLevelPercent() {return m_playerMagicLevelPercent;}

		void setPlayerStamina(Uint16 stamina);
		SDL_FORCE_INLINE Uint16 getPlayerStamina() {return m_playerStamina;}

		void setPlayerSoul(Uint8 soul);
		SDL_FORCE_INLINE Uint8 getPlayerSoul() {return m_playerSoul;}

		void setPlayerBaseSpeed(Uint16 baseSpeed);
		SDL_FORCE_INLINE Uint16 getPlayerBaseSpeed() {return m_playerBaseSpeed;}

		void setPlayerRegeneration(Uint16 regeneration);
		SDL_FORCE_INLINE Uint16 getPlayerRegeneration() {return m_playerRegeneration;}

		void setPlayerOfflineTraining(Uint16 offlineTime);
		SDL_FORCE_INLINE Uint16 getPlayerOfflineTraining() {return m_playerOfflineTraining;}

		void setPlayerExpBonus(double expBonus);
		void setPlayerTournamentFactor(Sint32 factor) {m_playerTournamentFactor = factor;}
		void setPlayerExpBonus(Uint16 baseXp, Uint16 voucherXp, Uint16 grindingXp, Uint16 storeXp, Uint16 huntingXp);
		SDL_FORCE_INLINE double getPlayerExpBonus() {return m_playerExpBonus;}
		SDL_FORCE_INLINE Uint16 getPlayerBaseXpGain() {return m_playerBaseXpGain;}
		SDL_FORCE_INLINE Sint32 getPlayerTournamentFactor() {return m_playerTournamentFactor;}
		SDL_FORCE_INLINE Uint16 getPlayerVoucherXpGain() {return m_playerVoucherXpGain;}
		SDL_FORCE_INLINE Uint16 getPlayerGrindingXpGain() {return m_playerGrindingXpGain;}
		SDL_FORCE_INLINE Uint16 getPlayerStoreXpGain() {return m_playerStoreXpGain;}
		SDL_FORCE_INLINE Uint16 getPlayerHuntingXpGain() {return m_playerHuntingXpGain;}
		
		void setPlayerSkill(Skills skill, Uint16 level, Uint16 baseLevel, Uint8 levelPercent);
		SDL_FORCE_INLINE Uint16 getPlayerSkillLevel(Skills skill) {return m_playerSkillsLevel[skill];}
		SDL_FORCE_INLINE Uint16 getPlayerSkillBaseLevel(Skills skill) {return m_playerSkillsBaseLevel[skill];}
		SDL_FORCE_INLINE Uint8 getPlayerSkillLevelPercent(Skills skill) {return m_playerSkillsLevelPercent[skill];}

		SDL_INLINE void setServerBeat(Uint16 serverBeat) {m_serverBeat = serverBeat;}
		SDL_FORCE_INLINE Uint16 getServerBeat() {return m_serverBeat;}

		SDL_INLINE void setIcons(Uint32 icons) {m_icons = icons;}
		SDL_FORCE_INLINE Uint32 getIcons() {return m_icons;}

		SDL_INLINE void resetCachedStats() {m_cached_stats = 0;}
		SDL_INLINE void resetCachedSkills() {m_cached_skills = 0;}
		SDL_FORCE_INLINE bool hasCachedStat(Stats_Cached_State flag) {return (m_cached_stats & flag) != 0;}
		SDL_FORCE_INLINE bool hasCachedSkill(Skills_Cached_State flag) {return (m_cached_skills & flag) != 0;}

		SDL_INLINE void setCanReportBugs(bool canReportBugs) {m_canReportBugs = canReportBugs;}
		SDL_FORCE_INLINE bool getCanReportBugs() {return m_canReportBugs;}

		SDL_INLINE void setExpertPvpMode(bool expertPvpMode) {m_expertPvpMode = expertPvpMode;}
		SDL_FORCE_INLINE bool getExpertPvpMode() {return m_expertPvpMode;}
		SDL_INLINE void setCanChangePvpFrames(bool canChangePvpFrames) {m_canChangePvpFrames = canChangePvpFrames;}
		SDL_FORCE_INLINE bool getCanChangePvpFrames() {return m_canChangePvpFrames;}
		SDL_INLINE void setHaveExivaRestrictions(bool haveExivaRestrictions) {m_haveExivaRestrictions = haveExivaRestrictions;}
		SDL_FORCE_INLINE bool getHaveExivaRestrictions() {return m_haveExivaRestrictions;}
		SDL_INLINE void setTournamentEnabled(bool tournamentEnabled) {m_tournamentEnabled = tournamentEnabled;}
		SDL_FORCE_INLINE bool getTournamentEnabled() {return m_tournamentEnabled;}
		
		SDL_INLINE void setStoreUrl(std::string storeUrl) {m_storeUrl = std::move(storeUrl);}
		SDL_FORCE_INLINE std::string& getStoreUrl() {return m_storeUrl;}
		SDL_INLINE void setStorePackages(Uint16 storePackages) {m_storePackages = storePackages;}
		SDL_FORCE_INLINE Uint16 getStorePackages() {return m_storePackages;}
		
	protected:
		std::bitset<GAME_FEATURE_LAST> m_gameFeatures;
		std::vector<Direction> m_autoWalkDirections;
		std::vector<std::pair<Uint32, Uint64>> m_expTable;
		std::string m_storeUrl;

		ItemUI* m_inventoryItem[SLOT_LAST];
		Container* m_containers[GAME_MAX_CONTAINERS];

		Uint64 m_playerExperience;
		double m_playerExpBonus;
		double m_playerExpSpeed;
		double m_playerCapacity;
		double m_playerTotalCapacity;

		Uint32 m_localPlayerId;
		Uint32 m_attackId;
		Uint32 m_followId;
		Uint32 m_selectId;
		Uint32 m_sequence;
		Uint32 m_playerHealth;
		Uint32 m_playerMaxHealth;
		Uint32 m_playerMana;
		Uint32 m_playerMaxMana;
		Uint32 m_icons;
		Uint32 m_cancelWalkCounter;

		Sint32 m_playerTournamentFactor;

		Position m_autoWalkDestination;
		Position m_limitWalkDestination;
		Position m_lastCancelWalkPos;

		Uint16 m_playerLevel;
		Uint16 m_playerMagicLevel;
		Uint16 m_playerBaseMagicLevel;
		Uint16 m_playerStamina;
		Uint16 m_playerBaseSpeed;
		Uint16 m_playerRegeneration;
		Uint16 m_playerOfflineTraining;
		Uint16 m_playerBaseXpGain;
		Uint16 m_playerVoucherXpGain;
		Uint16 m_playerGrindingXpGain;
		Uint16 m_playerStoreXpGain;
		Uint16 m_playerHuntingXpGain;
		Uint16 m_playerSkillsLevel[Skills_LastAdditionalSkill];
		Uint16 m_playerSkillsBaseLevel[Skills_LastAdditionalSkill];
		Uint16 m_serverBeat;
		Uint16 m_storePackages;
		Uint16 m_cached_stats;
		Uint16 m_cached_skills;

		Uint8 m_playerHealthPercent;
		Uint8 m_playerManaPercent;
		Uint8 m_playerLevelPercent;
		Uint8 m_playerMagicLevelPercent;
		Uint8 m_playerSkillsLevelPercent[Skills_LastAdditionalSkill];
		Uint8 m_playerSoul;

		Uint8 m_playerMovement;
		Uint8 m_playerCurrentDir;
		Uint8 m_playerLastDir;

		bool m_canReportBugs;
		bool m_expertPvpMode;
		bool m_canChangePvpFrames;
		bool m_haveExivaRestrictions;
		bool m_tournamentEnabled;
};

#endif /* __FILE_GAME_h_ */
