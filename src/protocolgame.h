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

#ifndef __FILE_PROTOCOL_GAME_h_
#define __FILE_PROTOCOL_GAME_h_

#include "protocol.h"

enum GameOpcodes : Uint8
{
	//Send Opcodes
	GameLoginOpcode = 0x0A,
	GameSecondaryLoginOpcode = 0x0B,
	GameEnterGameOpcode = 0x0F,
	GameLeaveOpcode = 0x14,
	GameConnectionPingBackOpcode = 0x1C,
	GamePingOpcode = 0x1D,
	GamePingBackOpcode = 0x1E,
	GameStashActionOpcode = 0x28,
	GameDepotSearchRetrieveOpcode = 0x29,
	GameClientCheckOpcode = 0x63,
	GameAutowalkOpcode = 0x64,
	GameWalkNorthOpcode = 0x65,
	GameWalkEastOpcode = 0x66,
	GameWalkSouthOpcode = 0x67,
	GameWalkWestOpcode = 0x68,
	GameStopAutowalkOpcode = 0x69,
	GameWalkNorthEastOpcode = 0x6A,
	GameWalkSouthEastOpcode = 0x6B,
	GameWalkSouthWestOpcode = 0x6C,
	GameWalkNorthWestOpcode = 0x6D,
	GameTurnNorthOpcode = 0x6F,
	GameTurnEastOpcode = 0x70,
	GameTurnSouthOpcode = 0x71,
	GameTurnWestOpcode = 0x72,
	GameTeleportOpcode = 0x73,
	GameEqupItemOpcode = 0x77,
	GameThrowOpcode = 0x78,
	GameLookInShopOpcode = 0x79,
	GameNPCPurchaseOpcode = 0x7A,
	GameNPCSellOpcode = 0x7B,
	GameNPCCloseOpcode = 0x7C,
	GameRequestTradeOpcode = 0x7D,
	GameLookInTradeOpcode = 0x7E,
	GameAcceptTradeOpcode = 0x7F,
	GameCloseTradeOpcode = 0x80,
	GameFriendSystemActionOpcode = 0x81,
	GameUseItemOpcode = 0x82,
	GameUseItemExOpcode = 0x83,
	GameUseWithCreatureOpcode = 0x84,
	GameRotateItemOpcode = 0x85,
	GameCloseContainerOpcode = 0x87,
	GameUpContainerOpcode = 0x88,
	GameTextWindowOpcode = 0x89,
	GameHouseWindowOpcode = 0x8A,
	GameWrapStateOpcode = 0x8B,
	GameLookAtOpcode = 0x8C,
	GameLookInBattleOpcode = 0x8D,
	GameJoinAggressionOpcode = 0x8E,
	GameQuickLootOpcode = 0x8F,
	GameLootContainerOpcode = 0x90,
	GameQuickLootBlackWhitelistOpcode = 0x91,
	GameOpenDepotSearchOpcode = 0x92,
	GameCloseDepotSearchOpcode = 0x93,
	GameDepotSearchTypeOpcode = 0x94,
	GameOpenParentContainerOpcode = 0x95,
	GameSayOpcode = 0x96,
	GameRequestChannelsOpcode = 0x97,
	GameOpenChannelOpcode = 0x98,
	GameCloseChannelOpcode = 0x99,
	GameOpenPrivateChannelOpcode = 0x9A,
	GameGuldMessageOpcode = 0x9B,
	GameEditGuldMessageOpcode = 0x9C,
	GameOpenRuleViolationChannelOpcode = 0x9B,
	GameCloseRuleViolationChannelOpcode = 0x9C,
	GameCancelRuleViolationChannelOpcode = 0x9D,
	GameCloseNpcChannelOpcode = 0x9E,
	GameAttackModesOpcode = 0xA0,
	GameAttackOpcode = 0xA1,
	GameFollowOpcode = 0xA2,
	GameInviteToPartyOpcode = 0xA3,
	GameJoinToPartyOpcode = 0xA4,
	GameRevokePartyInviteOpcode = 0xA5,
	GamePassPartyLeadershipOpcode = 0xA6,
	GameLeavePartyOpcode = 0xA7,
	GameEnableSharedPartyExperienceOpcode = 0xA8,
	GameCreatePrivateChannelOpcode = 0xAA,
	GameChannelInviteOpcode = 0xAB,
	GameChannelExcludeOpcode = 0xAC,
	GameCancelAttackAndFollowOpcode = 0xBE,
	GameTournamentInformationOpcode = 0xC4,
	GameSubscribeToUpdatesOpcode = 0xC6,
	GameTournamentLeaderboardOpcode = 0xC7,
	GameTournamentTicketActionOpcode = 0xC8,
	GameGetTransactionDetailsOpcode = 0xC9,
	GameUpdateTileOpcode = 0xC9,
	GameUpdateExivaOptionsOpcode = 0xCA,
	GameUpdateContainerOpcode = 0xCA,
	GameBrowseFieldOpcode = 0xCB,
	GameSeekInContainerOpcode = 0xCC,
	GameInspectObjectOpcode = 0xCD,
	GameInspectPlayerOpcode = 0xCE,
	GameBlessingsDialogOpcode = 0xCF,
	GameTrackQuestFlagsOpcode = 0xD0,
	GameMarketStatisticsOpcode = 0xD1,
	GameRequestOutfitOpcode = 0xD2,
	GameSetOutfitOpcode = 0xD3,
	GameMountOpcode = 0xD4,
	GameApplyImbuementOpcode = 0xD5,
	GameClearCharmOpcode = 0xD6,
	GameImbuementLeaveOpcode = 0xD7,
	GameOpenRewardWallOpcode = 0xD8,
	GameOpenRewardHistoryOpcode = 0xD9,
	GameCollectRewardOpcode = 0xDA,
	GameCyclopediaMapActionOpcode = 0xDB,
	GameAddVipOpcode = 0xDC,
	GameRemoveVipOpcode = 0xDD,
	GameEditVipOpcode = 0xDE,
	GameAddVipGroupOpcode = 0xDF,
	GameMarkGameNewsAsReadOpcode = 0xE0,
	GameOpenMonsterCyclopediaOpcode = 0xE1,
	GameOpenMonsterCyclopediaMonstersOpcode = 0xE2,
	GameOpenMonsterCyclopediaRaceOpcode = 0xE3,
	GameMonsterBonusEffectActionOpcode = 0xE4,
	GameOpenCyclopediaCharacterInfoOpcode = 0xE5,
	GameBugReportOpcode = 0xE6,
	GameRuleViolationOpcode = 0xE7,
	GameThankYouOpcode = 0xE7,
	GameDebugAssertOpcode = 0xE8,
	GameGetOfferDescriptionOpcode = 0xE8,
	GameStoreEventOpcode = 0xE9,
	GameFeatureEventOpcode = 0xEA,
	GamePreyActionOpcode = 0xEB,
	GameSetHirelingNameOpcode = 0xEC,
	GameRequestResourceBalanceOpcode = 0xED,
	GameGreetOpcode = 0xEE,
	GameTransferCoinsOpcode = 0xEF,
	GameShowQuestLogOpcode = 0xF0,
	GameShowQuestLineOpcode = 0xF1,
	GameNewRuleViolationOpcode = 0xF2,
	GameRequestItemInfoOpcode = 0xF3,
	GameMarketLeaveOpcode = 0xF4,
	GameMarketBrowseOpcode = 0xF5,
	GameMarketCreateOpcode = 0xF6,
	GameMarketCancelOpcode = 0xF7,
	GameMarketAcceptOpcode = 0xF8,
	GameAnswerModalDialogOpcode = 0xF9,
	GameOpenStoreOpcode = 0xFA,
	GameRequestStoreOpcode = 0xFB,
	GameBuyStoreOpcode = 0xFC,
	GameOpenTransactionStoreOpcode = 0xFD,
	GameRequestTransactionStoreOpcode = 0xFE,

	//Recv Opcodes
	RecvCreatureDataOpcode = 0x03,
	RecvLoginOrPendingOpcode = 0x0A,
	RecvGMActionsOpcode = 0x0B,
	RecvEnterGameOpcode = 0x0F,
	RecvUpdateNeededOpcode = 0x11,
	RecvLoginErrorOpcode = 0x14,
	RecvLoginAdviceOpcode = 0x15,
	RecvLoginWaitOpcode = 0x16,
	RecvLoginSuccessOpcode = 0x17,
	RecvLoginTokenOpcode = 0x18,
	RecvStoreButtonIndicatorsOpcode = 0x19,
	RecvPingBackOpcode = 0x1D,
	RecvPingOpcode = 0x1E,
	RecvChallengeOpcode = 0x1F,
	RecvDeathOpcode = 0x28,
	RecvStashOpcode = 0x29,
	RecvSpecialContainersAvailableOpcode = 0x2A,
	RecvClientCheckOpcode = 0x63,
	RecvMapDescriptionOpcode = 0x64,
	RecvMapNorthOpcode = 0x65,
	RecvMapEastOpcode = 0x66,
	RecvMapSouthOpcode = 0x67,
	RecvMapWestOpcode = 0x68,
	RecvUpdateTileOpcode = 0x69,
	RecvTileAddThingOpcode = 0x6A,
	RecvTileTransformThingOpcode = 0x6B,
	RecvTileRemoveThingOpcode = 0x6C,
	RecvTileMoveCreatureOpcode = 0x6D,
	RecvContainerOpenOpcode = 0x6E,
	RecvContainerCloseOpcode = 0x6F,
	RecvContainerAddItemOpcode = 0x70,
	RecvContainerTransformItemOpcode = 0x71,
	RecvContainerRemoveItemOpcode = 0x72,
	RecvFriendSystemDataOpcode = 0x74,
	RecvScreenshotEventOpcode = 0x75,
	RecvInspectionListOpcode = 0x76,
	RecvInspectionStateOpcode = 0x77,
	RecvInventoryTransformItemOpcode = 0x78,
	RecvInventoryRemoveItemOpcode = 0x79,
	RecvNPCOpenTradeOpcode = 0x7A,
	RecvNPCPlayerGoodsOpcode = 0x7B,
	RecvNPCCloseTradeOpcode = 0x7C,
	RecvTradeOwnOpcode = 0x7D,
	RecvTradeCounterOpcode = 0x7E,
	RecvTradeCloseOpcode = 0x7F,
	RecvAmbientWorldLightOpcode = 0x82,
	RecvMagicEffectOpcode = 0x83,
	RecvAnimatedTextOpcode = 0x84,
	RecvDistanceEffectOpcode = 0x85,
	RecvCreatureMarkOpcode = 0x86,
	RecvCreatureTrappersOpcode = 0x87,
	RecvCreatureUpdateOpcode = 0x8B,
	RecvCreatureHealthOpcode = 0x8C,
	RecvCreatureLightOpcode = 0x8D,
	RecvCreatureOutfitOpcode = 0x8E,
	RecvCreatureSpeedOpcode = 0x8F,
	RecvCreatureSkullOpcode = 0x90,
	RecvCreaturePartyOpcode = 0x91,
	RecvCreatureUnpassableOpcode = 0x92,
	RecvCreatureMarksOpcode = 0x93,
	RecvCreatureHelpersOpcode = 0x94,
	RecvCreatureTypeOpcode = 0x95,
	RecvEditTextWindowOpcode = 0x96,
	RecvEditHouseWindowOpcode = 0x97,
	RecvGameNewsOpcode = 0x98,
	RecvDepotSearchDetailListOpcode = 0x99,
	RecvCloseDepotSearchOpcode = 0x9A,
	RecvBlessingsDialogOpcode = 0x9B,
	RecvBlessingsOpcode = 0x9C,
	RecvPresetOpcode = 0x9D,
	RecvPremiumTriggerOpcode = 0x9E,
	RecvPlayerDataBasicOpcode = 0x9F,
	RecvPlayerDataOpcode = 0xA0,
	RecvPlayerSkillsOpcode = 0xA1,
	RecvPlayerIconsOpcode = 0xA2,
	RecvPlayerCancelTargetOpcode = 0xA3,
	RecvPlayerSpellDelayOpcode = 0xA4,
	RecvPlayerSpellGroupDelayOpcode = 0xA5,
	RecvPlayerMultiUseDelayOpcode = 0xA6,
	RecvPlayerModesOpcode = 0xA7,
	RecvPlayerStoreDeepLinkOpcode = 0xA8,
	RecvRestingAreaStateOpcode = 0xA9,
	RecvPlayerTalkOpcode = 0xAA,
	RecvChannelsOpcode = 0xAB,
	RecvOpenChannelOpcode = 0xAC,
	RecvOpenPrivateChannelOpcode = 0xAD,
	RecvRuleViolationChannelOpcode = 0xAE,
	RecvRuleViolationRemoveOpcode = 0xAF,
	RecvRuleViolationCancelOpcode = 0xB0,
	RecvRuleViolationLockOpcode = 0xB1,
	RecvOpenOwnChannelOpcode = 0xB2,
	RecvCloseChannelOpcode = 0xB3,
	RecvTextMessageOpcode = 0xB4,
	RecvCancelWalkOpcode = 0xB5,
	RecvWalkWaitOpcode = 0xB6,
	RecvUnjustifiedStatsOpcode = 0xB7,
	RecvPvpSituationsOpcode = 0xB8,
	RecvFloorChangeUpOpcode = 0xBE,
	RecvFloorChangeDownOpcode = 0xBF,
	RecvUpdateLootContainersOpcode = 0xC0,
	RecvPlayerDataTournamentOpcode = 0xC1,
	RecvTournamentInformationOpcode = 0xC4,
	RecvTournamentLeaderboardOpcode = 0xC5,
	RecvChooseOutfitOpcode = 0xC8,
	RecvExivaSuppressedOpcode = 0xC9,
	RecvUpdateExivaOptionsOpcode = 0xCA,
	RecvTransactionDetailsOpcode = 0xCB,
	RecvImpactTrackingOpcode = 0xCC,
	RecvMarketStatisticsOpcode = 0xCD,
	RecvItemWastedOpcode = 0xCE,
	RecvItemLootedOpcode = 0xCF,
	RecvTrackedQuestFlagsOpcode = 0xD0,
	RecvKillTrackingOpcode = 0xD1,
	RecvVipAddOpcode = 0xD2,
	RecvVipStatusOpcode = 0xD3,
	RecvVipStatusLogoutOpcode = 0xD4,
	RecvMonsterCyclopediaOpcode = 0xD5,
	RecvMonsterCyclopediaMonstersOpcode = 0xD6,
	RecvMonsterCyclopediaRaceOpcode = 0xD7,
	RecvMonsterCyclopediaBonusEffectsOpcode = 0xD8,
	RecvMonsterCyclopediaNewDetailsOpcode = 0xD9,
	RecvCyclopediaCharacterInfoOpcode = 0xDA,
	RecvHirelingNameChangeOpcode = 0xDB,
	RecvTutorialHintOpcode = 0xDC,
	RecvAutomapFlagOpcode = 0xDD,
	RecvDailyRewardCollectionStateOpcode = 0xDE,
	RecvStoreCoinBalanceOpcode = 0xDF,
	RecvStoreErrorOpcode = 0xE0,
	RecvStoreRequestPurchaseOpcode = 0xE1,
	RecvOpenRewardWallOpcode = 0xE2,
	RecvCloseRewardWallOpcode = 0xE3,
	RecvDailyRewardBasicOpcode = 0xE4,
	RecvDailyRewardHistoryOpcode = 0xE5,
	RecvPreyFreeListRerollAvailabilityOpcode = 0xE6,
	RecvPreyTimeLeftOpcode = 0xE7,
	RecvPreyDataOpcode = 0xE8,
	RecvPreyPricesOpcode = 0xE9,
	RecvOfferDescriptionOpcode = 0xEA,
	RecvRefreshImbuingDialogOpcode = 0xEB,
	RecvCloseImbuingDialogOpcode = 0xEC,
	RecvShowMessageDialogOpcode = 0xED,
	RecvResourceBalanceOpcode = 0xEE,
	RecvTimeOpcode = 0xEF,
	RecvQuestLogOpcode = 0xF0,
	RecvQuestLineOpcode = 0xF1,
	RecvStoreCoinBalanceUpdatingOpcode = 0xF2,
	RecvChannelEventOpcode = 0xF3,
	RecvItemInfoOpcode = 0xF4,
	RecvPlayerInventoryOpcode = 0xF5,
	RecvMarketEnterOpcode = 0xF6,
	RecvMarketLeaveOpcode = 0xF7,
	RecvMarketDetailOpcode = 0xF8,
	RecvMarketBrowseOpcode = 0xF9,
	RecvModalWindowOpcode = 0xFA,
	RecvStoreOpenOpcode = 0xFB,
	RecvStoreOffersOpcode = 0xFC,
	RecvStoreTransactionHistoryOpcode = 0xFD,
	RecvStoreCompletePurchaseOpcode = 0xFE
};

/*
	QT Client outputmessage functions
	msg+0x10 - addU8()
	msg+0x14 - addU16()
	msg+0x18 - addU32()
	msg+0x1C - addU64()
	msg+0x24 - addString()
	msg+0x28 - addBytes(size)
	msg+0x2C - addBool()

	QT Client(12.20.9183) Gameclient payload functions
	{0x0A, "tibia.protobuf.protocol.GameclientMessageLogin"},								base+244F8C
	{0x0B, "tibia.protobuf.protocol.GameclientMessageSecondaryLogin"},						base+2451b1
	{0x0F, "tibia.protobuf.protocol.GameclientMessageEnterWorld"},							no payload
	{0x14, "tibia.protobuf.protocol.GameclientMessageQuitGame"},							no payload
	{0x1C, "tibia.protobuf.protocol.GameclientMessageConnectionPingBack"},					no payload
	{0x1D, "tibia.protobuf.protocol.GameclientMessagePing"},								no payload
	{0x1E, "tibia.protobuf.protocol.GameclientMessagePingBack"},							no payload
	{0x1F, "tibia.protobuf.protocol.GameclientMessage"},									placeholder
	{0x28, "tibia.protobuf.protocol.GameclientMessageStashAction"},							base+2473ac
	{0x29, "tibia.protobuf.protocol.GameclientMessageDepotSearchRetrieve"},					base+2475b1
	{0x63, "tibia.protobuf.protocol.GameclientMessageClientCheck"},							base+24536a
	{0x64, "tibia.protobuf.protocol.GameclientMessageGoPath"},								base+245327
	{0x65, "tibia.protobuf.protocol.GameclientMessageGoNorth"},								no payload
	{0x66, "tibia.protobuf.protocol.GameclientMessageGoEast"},								no payload
	{0x67, "tibia.protobuf.protocol.GameclientMessageGoSouth"},								no payload
	{0x68, "tibia.protobuf.protocol.GameclientMessageGoWest"},								no payload
	{0x69, "tibia.protobuf.protocol.GameclientMessageStop"},								no payload
	{0x6A, "tibia.protobuf.protocol.GameclientMessageGoNorthEast"},							no payload
	{0x6B, "tibia.protobuf.protocol.GameclientMessageGoSouthEast"},							no payload
	{0x6C, "tibia.protobuf.protocol.GameclientMessageGoSouthWest"},							no payload
	{0x6D, "tibia.protobuf.protocol.GameclientMessageGoNorthWest"},							no payload
	{0x6F, "tibia.protobuf.protocol.GameclientMessageRotateNorth"},							no payload
	{0x70, "tibia.protobuf.protocol.GameclientMessageRotateEast"},							no payload
	{0x71, "tibia.protobuf.protocol.GameclientMessageRotateSouth"},							no payload
	{0x72, "tibia.protobuf.protocol.GameclientMessageRotateWest"},							no payload
	{0x73, "tibia.protobuf.protocol.GameclientMessageTeleport"},							base+246bde
	{0x77, "tibia.protobuf.protocol.GameclientMessageEquipObject"},							base+2453b9
	{0x78, "tibia.protobuf.protocol.GameclientMessageMoveObject"},							base+245401
	{0x79, "tibia.protobuf.protocol.GameclientMessageLookNPCTrade"},						base+246fd2
	{0x7A, "tibia.protobuf.protocol.GameclientMessageBuyObject"},							base+24701a
	{0x7B, "tibia.protobuf.protocol.GameclientMessageSellObject"},							base+247089
	{0x7C, "tibia.protobuf.protocol.GameclientMessageCloseNPCTrade"},						no payload
	{0x7D, "tibia.protobuf.protocol.GameclientMessageTradeObject"},							base+24617b
	{0x7E, "tibia.protobuf.protocol.GameclientMessageLookTrade"},							base+24620c
	{0x7F, "tibia.protobuf.protocol.GameclientMessageAcceptTrade"},							no payload
	{0x80, "tibia.protobuf.protocol.GameclientMessageRejectTrade"},							no payload
	{0x81, "tibia.protobuf.protocol.GameclientMessageFriendSystemAction"},					base+246236
	{0x82, "tibia.protobuf.protocol.GameclientMessageUseObject"},							base+2456bd
	{0x83, "tibia.protobuf.protocol.GameclientMessageUseTwoObjects"},						base+24575e
	{0x84, "tibia.protobuf.protocol.GameclientMessageUseOnCreature"},						base+245855
	{0x85, "tibia.protobuf.protocol.GameclientMessageTurnObject"},							base+24563d
	{0x87, "tibia.protobuf.protocol.GameclientMessageCloseContainer"},						base+245a70
	{0x88, "tibia.protobuf.protocol.GameclientMessageUpContainer"},							base+245a70
	{0x89, "tibia.protobuf.protocol.GameclientMessageEditText"},							base+2464c0
	{0x8A, "tibia.protobuf.protocol.GameclientMessageEditList"},							base+24650b
	{0x8B, "tibia.protobuf.protocol.GameclientMessageToggleWrapState"},						base+2455bd
	{0x8C, "tibia.protobuf.protocol.GameclientMessageLook"},								base+2458e6
	{0x8D, "tibia.protobuf.protocol.GameclientMessageLookAtCreature"},						base+245966
	{0x8E, "tibia.protobuf.protocol.GameclientMessageJoinAggression"},						base+245966
	{0x8F, "tibia.protobuf.protocol.GameclientMessageQuickLoot"},							base+246d3d
	{0x90, "tibia.protobuf.protocol.GameclientMessageLootContainer"},						base+246e20
	{0x91, "tibia.protobuf.protocol.GameclientMessageQuickLootBlackWhitelist"},				base+246dbd
	{0x92, "tibia.protobuf.protocol.GameclientMessageOpenDepotSearch"},						no payload
	{0x93, "tibia.protobuf.protocol.GameclientMessageCloseDepotSearch"},					no payload
	{0x94, "tibia.protobuf.protocol.GameclientMessageDepotSearchType"},						base+245bd8
	{0x95, "tibia.protobuf.protocol.GameclientMessageOpenParentContainer"},					base+247578
	{0x96, "tibia.protobuf.protocol.GameclientMessageTalk"},								base+245b3a
	{0x97, "tibia.protobuf.protocol.GameclientMessageGetChannels"},							no payload
	{0x98, "tibia.protobuf.protocol.GameclientMessageJoinChannel"},							base+245bd8
	{0x99, "tibia.protobuf.protocol.GameclientMessageLeaveChannel"},						base+245bd8
	{0x9A, "tibia.protobuf.protocol.GameclientMessagePrivateChannel"},						base+245c15
	{0x9B, "tibia.protobuf.protocol.GameclientMessageGuildMessage"},						no payload
	{0x9C, "tibia.protobuf.protocol.GameclientMessageEditGuildMessage"},					base+245c15
	{0x9E, "tibia.protobuf.protocol.GameclientMessageCloseNPCChannel"},						no payload
	{0xA0, "tibia.protobuf.protocol.GameclientMessageSetTactics"},							base+245ab1
	{0xA1, "tibia.protobuf.protocol.GameclientMessageAttack"},								base+245bef
	{0xA2, "tibia.protobuf.protocol.GameclientMessageFollow"},								base+245bef
	{0xA3, "tibia.protobuf.protocol.GameclientMessageInviteToParty"},						base+245966
	{0xA4, "tibia.protobuf.protocol.GameclientMessageJoinParty"},							base+245966
	{0xA5, "tibia.protobuf.protocol.GameclientMessageRevokeInvitation"},					base+245966
	{0xA6, "tibia.protobuf.protocol.GameclientMessagePassLeadership"},						base+245966
	{0xA7, "tibia.protobuf.protocol.GameclientMessageLeaveParty"},							no payload
	{0xA8, "tibia.protobuf.protocol.GameclientMessageShareExperience"},						base+245ca4
	{0xA9, "tibia.protobuf.protocol.GameclientMessage"},									placeholder
	{0xAA, "tibia.protobuf.protocol.GameclientMessageOpenChannel"},							no payload
	{0xAB, "tibia.protobuf.protocol.GameclientMessageInviteToChannel"},						base+245c53
	{0xAC, "tibia.protobuf.protocol.GameclientMessageExcludeFromChannel"},					base+245c53
	{0xBE, "tibia.protobuf.protocol.GameclientMessageCancel"},								no payload
	{0xC4, "tibia.protobuf.protocol.GameclientMessageTournamentInformation"},				no payload
	{0xC6, "tibia.protobuf.protocol.GameclientMessageSubscribeToUpdates"},					base+2454c0
	{0xC7, "tibia.protobuf.protocol.GameclientMessageTournamentLeaderboard"},				base+2454ea
	{0xC8, "tibia.protobuf.protocol.GameclientMessageTournamentTicketAction"},				base+2474f4
	{0xC9, "tibia.protobuf.protocol.GameclientMessageGetTransactionDetails"},				base+245966
	{0xCA, "tibia.protobuf.protocol.GameclientMessageUpdateExivaOptions"},					base+2470eb
	{0xCB, "tibia.protobuf.protocol.GameclientMessageBrowseField"},							base+245af5
	{0xCC, "tibia.protobuf.protocol.GameclientMessageSeekInContainer"},						base+245a87
	{0xCD, "tibia.protobuf.protocol.GameclientMessageInspectObject"},						base+2459bf
	{0xCE, "tibia.protobuf.protocol.GameclientMessageInspectPlayer"},						base+24597b
	{0xCF, "tibia.protobuf.protocol.GameclientMessageBlessingsDialog"},						no payload
	{0xD0, "tibia.protobuf.protocol.GameclientMessageTrackQuestflags"},						base+246cab
	{0xD1, "tibia.protobuf.protocol.GameclientMessageMarketStatistics"},					no payload
	{0xD2, "tibia.protobuf.protocol.GameclientMessageGetOutfit"},							base+245cbb
	{0xD3, "tibia.protobuf.protocol.GameclientMessageSetOutfit"},							base+245d04
	{0xD4, "tibia.protobuf.protocol.GameclientMessageMount"},								base+245ca4
	{0xD5, "tibia.protobuf.protocol.GameclientMessageApplyImbuement"},						base+246a84
	{0xD6, "tibia.protobuf.protocol.GameclientMessageApplyClearingCharm"},					base+245a70
	{0xD7, "tibia.protobuf.protocol.GameclientMessageCloseImbuingDialog"},					no payload
	{0xD8, "tibia.protobuf.protocol.GameclientMessageOpenRewardWall"},						no payload
	{0xD9, "tibia.protobuf.protocol.GameclientMessageDailyRewardHistory"},					no payload
	{0xDA, "tibia.protobuf.protocol.GameclientMessageCollectDailyReward"},					base+246c23
	{0xDB, "tibia.protobuf.protocol.GameclientMessageCyclopediaMapAction"},					base+24749a
	{0xDC, "tibia.protobuf.protocol.GameclientMessageAddBuddy"},							base+245c15
	{0xDD, "tibia.protobuf.protocol.GameclientMessageRemoveBuddy"},							base+245966
	{0xDE, "tibia.protobuf.protocol.GameclientMessageEditBuddy"},							base+245e61
	{0xDF, "tibia.protobuf.protocol.GameclientMessageBuddyGroup"},							base+245f5a
	{0xE0, "tibia.protobuf.protocol.GameclientMessageMarkGameNewsAsRead"},					base+24602a
	{0xE1, "tibia.protobuf.protocol.GameclientMessageOpenMonsterCyclopedia"},				no payload
	{0xE2, "tibia.protobuf.protocol.GameclientMessageOpenMonsterCyclopediaMonsters"},		base+246052
	{0xE3, "tibia.protobuf.protocol.GameclientMessageOpenMonsterCyclopediaRace"},			base+245bd8
	{0xE4, "tibia.protobuf.protocol.GameclientMessageMonsterBonusEffectAction"},			base+2460f0
	{0xE5, "tibia.protobuf.protocol.GameclientMessageOpenCyclopediaCharacterInfo"},			base+24612d
	{0xE6, "tibia.protobuf.protocol.GameclientMessageBugReport"},							base+2468e3
	{0xE7, "tibia.protobuf.protocol.GameclientMessageThankYou"},							base+245966
	{0xE8, "tibia.protobuf.protocol.GameclientMessageGetOfferDescription"},					base+245966
	{0xE9, "tibia.protobuf.protocol.GameclientMessageStoreEvent"},							base+2469db
	{0xEA, "tibia.protobuf.protocol.GameclientMessageFeatureEvent"},						base+246d02
	{0xEB, "tibia.protobuf.protocol.GameclientMessagePreyAction"},							base+246ab9
	{0xEC, "tibia.protobuf.protocol.GameclientMessageSetHirelingName"},						base+245e07
	{0xED, "tibia.protobuf.protocol.GameclientMessageRequestResourceBalance"},				base+245a70
	{0xEE, "tibia.protobuf.protocol.GameclientMessageGreet"},								base+245966
	{0xEF, "tibia.protobuf.protocol.GameclientMessageTransferCurrency"},					base+246894
	{0xF0, "tibia.protobuf.protocol.GameclientMessageGetQuestLog"},							no payload
	{0xF1, "tibia.protobuf.protocol.GameclientMessageGetQuestLine"},						base+245bd8
	{0xF2, "tibia.protobuf.protocol.GameclientMessageRuleViolationReport"},					base+2463b5
	{0xF3, "tibia.protobuf.protocol.GameclientMessageGetObjectInfo"},						base+246b19
	{0xF4, "tibia.protobuf.protocol.GameclientMessageMarketLeave"},							no payload
	{0xF5, "tibia.protobuf.protocol.GameclientMessageMarketBrowse"},						base+245bd8
	{0xF6, "tibia.protobuf.protocol.GameclientMessageMarketCreate"},						base+246f83
	{0xF7, "tibia.protobuf.protocol.GameclientMessageMarketCancel"},						base+246eea
	{0xF8, "tibia.protobuf.protocol.GameclientMessageMarketAccept"},						base+246f30
	{0xF9, "tibia.protobuf.protocol.GameclientMessageAnswerModalDialog"},					base+246380
	{0xFA, "tibia.protobuf.protocol.GameclientMessageRequestStoreCategories"},				no payload
	{0xFB, "tibia.protobuf.protocol.GameclientMessageRequestStoreOffers"},					base+246561
	{0xFC, "tibia.protobuf.protocol.GameclientMessageBuyStoreOffer"},						base+24671f
	{0xFD, "tibia.protobuf.protocol.GameclientMessageOpenTransactionHistory"},				base+245a70
	{0xFE, "tibia.protobuf.protocol.GameclientMessageGetTransactionHistory"},				base+24686c
*/

/*
	QT Client inputmessage functions
	msg+0x10 - getU8()
	msg+0x18 - getU16()
	msg+0x1C - getU32()
	msg+0x20 - getU64()
	msg+0x24 - getS8()
	msg+0x28 - getS16()
	msg+0x2C - getS32()
	msg+0x30 - getS64()
	msg+0x34 - getString()
	msg+0x38 - getBytes(size)
	msg+0x40 - getBool()
	msg+0x44 - getDouble()

	QT Client(12.20.9183) Gameserver payload functions
	{0x03, "tibia.protobuf.protocol.GameserverMessageCreatureData"},						base+23B469
	{0x0A, "tibia.protobuf.protocol.GameserverMessagePendingStateEntered"},					no payload
	{0x0B, "tibia.protobuf.protocol.GameserverMessageReadyForSecondaryConnection"},			base+240457
	{0x0F, "tibia.protobuf.protocol.GameserverMessageWorldEntered"},						no payload
	{0x14, "tibia.protobuf.protocol.GameserverMessageLoginError"},							base+23b4b9
	{0x15, "tibia.protobuf.protocol.GameserverMessageLoginAdvice"},							base+23b4b9
	{0x16, "tibia.protobuf.protocol.GameserverMessageLoginWait"},							base+23c123
	{0x17, "tibia.protobuf.protocol.GameserverMessageLoginSuccess"},						base+23aecd
	{0x19, "tibia.protobuf.protocol.GameserverMessageStoreButtonIndicators"},				base+23c18b
	{0x1D, "tibia.protobuf.protocol.GameserverMessagePing"},								no payload
	{0x1E, "tibia.protobuf.protocol.GameserverMessagePingBack"},							no payload
	{0x1F, "tibia.protobuf.protocol.GameserverMessageLoginChallenge"},						base+23b488
	{0x28, "tibia.protobuf.protocol.GameserverMessageDead"},								base+23c1b9
	{0x29, "tibia.protobuf.protocol.GameserverMessageStash"},								base+2421a7
	{0x2A, "tibia.protobuf.protocol.GameserverMessageSpecialContainersAvailable"},			base+23c18b
	{0x63, "tibia.protobuf.protocol.GameserverMessageClientCheck"},							base+237d3a
	{0x64, "tibia.protobuf.protocol.GameserverMessageFullMap"},								base+23873e
	{0x65, "tibia.protobuf.protocol.GameserverMessageTopRow"},								base+23882f
	{0x66, "tibia.protobuf.protocol.GameserverMessageRightColumn"},							base+2387ef
	{0x67, "tibia.protobuf.protocol.GameserverMessageBottomRow"},							base+238869
	{0x68, "tibia.protobuf.protocol.GameserverMessageLeftColumn"},							base+2387af
	{0x69, "tibia.protobuf.protocol.GameserverMessageFieldData"},							base+2388a3
	{0x6A, "tibia.protobuf.protocol.GameserverMessageCreateOnMap"},							base+23a8d9
	{0x6B, "tibia.protobuf.protocol.GameserverMessageChangeOnMap"},							base+23a9dd
	{0x6C, "tibia.protobuf.protocol.GameserverMessageDeleteOnMap"},							base+23aa94
	{0x6D, "tibia.protobuf.protocol.GameserverMessageMoveCreature"},						base+23ab35
	{0x6E, "tibia.protobuf.protocol.GameserverMessageContainer"},							base+23c200
	{0x6F, "tibia.protobuf.protocol.GameserverMessageCloseContainer"},						base+23b84d
	{0x70, "tibia.protobuf.protocol.GameserverMessageCreateInContainer"},					base+23c31a
	{0x71, "tibia.protobuf.protocol.GameserverMessageChangeInContainer"},					base+23b8fb
	{0x72, "tibia.protobuf.protocol.GameserverMessageDeleteInContainer"},					base+23c31a
	{0x74, "tibia.protobuf.protocol.GameserverMessageFriendSystemData"},					base+24188a
	{0x75, "tibia.protobuf.protocol.GameserverMessageScreenshotEvent"},						base+23b84d
	{0x76, "tibia.protobuf.protocol.GameserverMessageInspectionList"},						base+23b53a
	{0x77, "tibia.protobuf.protocol.GameserverMessageInspectionState"},						base+23b80f
	{0x78, "tibia.protobuf.protocol.GameserverMessageSetInventory"},						base+23b509
	{0x79, "tibia.protobuf.protocol.GameserverMessageDeleteInventory"},						base+23b84d
	{0x7A, "tibia.protobuf.protocol.GameserverMessageNPCOffer"},							base+23c370
	{0x7B, "tibia.protobuf.protocol.GameserverMessagePlayerGoods"},							base+23c51c
	{0x7C, "tibia.protobuf.protocol.GameserverMessageCloseNPCTrade"},						no payload
	{0x7D, "tibia.protobuf.protocol.GameserverMessageOwnOffer"},							base+23c580
	{0x7E, "tibia.protobuf.protocol.GameserverMessageCounterOffer"},						base+23c580
	{0x7F, "tibia.protobuf.protocol.GameserverMessageCloseTrade"},							no payload
	{0x82, "tibia.protobuf.protocol.GameserverMessageAmbientLight"},						base+23a5b0
	{0x83, "tibia.protobuf.protocol.GameserverMessageGraphicalEffects"},					base+23a5e4
	{0x84, "tibia.protobuf.protocol.GameserverMessageRemoveGraphicalEffect"},				base+23a800
	{0x8B, "tibia.protobuf.protocol.GameserverMessageCreatureUpdate"},						base+23b028
	{0x8C, "tibia.protobuf.protocol.GameserverMessageCreatureHealth"},						base+23b10a
	{0x8D, "tibia.protobuf.protocol.GameserverMessageCreatureLight"},						base+23b942
	{0x8E, "tibia.protobuf.protocol.GameserverMessageCreatureOutfit"},						base+23b145
	{0x8F, "tibia.protobuf.protocol.GameserverMessageCreatureSpeed"},						base+23b194
	{0x90, "tibia.protobuf.protocol.GameserverMessageCreatureSkull"},						base+23b1eb
	{0x91, "tibia.protobuf.protocol.GameserverMessageCreatureParty"},						base+23b229
	{0x92, "tibia.protobuf.protocol.GameserverMessageCreatureUnpass"},						base+23b267
	{0x93, "tibia.protobuf.protocol.GameserverMessageCreatureMarks"},						base+23b2a2
	{0x94, "tibia.protobuf.protocol.GameserverMessageDepotSearchResult"},					base+242ae2
	{0x95, "tibia.protobuf.protocol.GameserverMessageCreatureType"},						base+23c5fc
	{0x96, "tibia.protobuf.protocol.GameserverMessageEditText"},							base+23c641
	{0x97, "tibia.protobuf.protocol.GameserverMessageEditList"},							base+23c743
	{0x98, "tibia.protobuf.protocol.GameserverMessageShowGameNews"},						base+23c7b6
	{0x99, "tibia.protobuf.protocol.GameserverMessageDepotSearchDetailList"},				base+242b43
	{0x9A, "tibia.protobuf.protocol.GameserverMessageCloseDepotSearch"},					no payload
	{0x9B, "tibia.protobuf.protocol.GameserverMessageBlessingsDialog"},						base+23b995
	{0x9C, "tibia.protobuf.protocol.GameserverMessageBlessings"},							base+23bb95
	{0x9D, "tibia.protobuf.protocol.GameserverMessageSwitchPreset"},						base+23c7e4
	{0x9E, "tibia.protobuf.protocol.GameserverMessagePremiumTrigger"},						base+23bc59
	{0x9F, "tibia.protobuf.protocol.GameserverMessagePlayerDataBasic"},						base+23b869
	{0xA0, "tibia.protobuf.protocol.GameserverMessagePlayerDataCurrent"},					base+23b2e5
	{0xA1, "tibia.protobuf.protocol.GameserverMessagePlayerSkills"},						base+23bf41
	{0xA2, "tibia.protobuf.protocol.GameserverMessagePlayerState"},							base+23bcb6
	{0xA3, "tibia.protobuf.protocol.GameserverMessageClearTarget"},							base+23c7e4
	{0xA4, "tibia.protobuf.protocol.GameserverMessageSpellDelay"},							base+23c7fd
	{0xA5, "tibia.protobuf.protocol.GameserverMessageSpellGroupDelay"},						base+23c7fd
	{0xA6, "tibia.protobuf.protocol.GameserverMessageMultiUseDelay"},						base+23c7e4
	{0xA7, "tibia.protobuf.protocol.GameserverMessageSetTactics"},							base+23c82e
	{0xA8, "tibia.protobuf.protocol.GameserverMessageSetStoreButtonDeeplink"},				base+23bc99
	{0xA9, "tibia.protobuf.protocol.GameserverMessageRestingAreaState"},					base+23ff6e
	{0xAA, "tibia.protobuf.protocol.GameserverMessageTalk"},								base+23abd8
	{0xAB, "tibia.protobuf.protocol.GameserverMessageChannels"},							base+23c880
	{0xAC, "tibia.protobuf.protocol.GameserverMessageOpenChannel"},							base+23bd7d
	{0xAD, "tibia.protobuf.protocol.GameserverMessagePrivateChannel"},						base+23b4b9
	{0xAE, "tibia.protobuf.protocol.GameserverMessageEditGuildMessage"},					base+23b4b9
	{0xB2, "tibia.protobuf.protocol.GameserverMessageOpenOwnChannel"},						base+23bd7d
	{0xB3, "tibia.protobuf.protocol.GameserverMessageCloseChannel"},						base+23c90e
	{0xB4, "tibia.protobuf.protocol.GameserverMessageMessage"},								base+23ad3f
	{0xB5, "tibia.protobuf.protocol.GameserverMessageSnapBack"},							base+23b84d
	{0xB6, "tibia.protobuf.protocol.GameserverMessageWait"},								base+23c90e
	{0xB7, "tibia.protobuf.protocol.GameserverMessageUnjustifiedPoints"},					base+23bee7
	{0xB8, "tibia.protobuf.protocol.GameserverMessagePvpSituations"},						base+23b84d
	{0xBE, "tibia.protobuf.protocol.GameserverMessageTopFloor"},							base+23a86b
	{0xBF, "tibia.protobuf.protocol.GameserverMessageBottomFloor"},							base+23a8a2
	{0xC0, "tibia.protobuf.protocol.GameserverMessageUpdateLootContainers"},				base+240605
	{0xC1, "tibia.protobuf.protocol.GameserverMessagePlayerDataTournament"},				base+23c7e4
	{0xC4, "tibia.protobuf.protocol.GameserverMessageTournamentInformation"},				base+2424b5
	{0xC5, "tibia.protobuf.protocol.GameserverMessageTournamentLeaderboard"},				base+24221d
	{0xC8, "tibia.protobuf.protocol.GameserverMessageOutfit"},								base+23c92a
	{0xC9, "tibia.protobuf.protocol.GameserverMessageExivaSuppressed"},						no payload
	{0xCA, "tibia.protobuf.protocol.GameserverMessageUpdateExivaOptions"},					base+24163c
	{0xCB, "tibia.protobuf.protocol.GameserverMessageTransactionDetails"},					base+23cc28
	{0xCC, "tibia.protobuf.protocol.GameserverMessageImpactTracking"},						base+23c7fd
	{0xCD, "tibia.protobuf.protocol.GameserverMessageMarketStatistics"},					base+23cec6
	{0xCE, "tibia.protobuf.protocol.GameserverMessageItemWasted"},							base+23c90e
	{0xCF, "tibia.protobuf.protocol.GameserverMessageItemLooted"},							base+23cb7b
	{0xD0, "tibia.protobuf.protocol.GameserverMessageTrackQuestflags"},						base+2404c1
	{0xD1, "tibia.protobuf.protocol.GameserverMessageKillTracking"},						base+23cbde
	{0xD2, "tibia.protobuf.protocol.GameserverMessageBuddyData"},							base+23cf24
	{0xD3, "tibia.protobuf.protocol.GameserverMessageBuddyStatusChange"},					base+23d065
	{0xD4, "tibia.protobuf.protocol.GameserverMessageBuddyGroupData"},						base+23d0a5
	{0xD5, "tibia.protobuf.protocol.GameserverMessageMonsterCyclopedia"},					base+23d160
	{0xD6, "tibia.protobuf.protocol.GameserverMessageMonsterCyclopediaMonsters"},			base+23d200
	{0xD7, "tibia.protobuf.protocol.GameserverMessageMonsterCyclopediaRace"},				base+23d2b6
	{0xD8, "tibia.protobuf.protocol.GameserverMessageMonsterCyclopediaBonusEffects"},		base+23d61b
	{0xD9, "tibia.protobuf.protocol.GameserverMessageMonsterCyclopediaNewDetails"},			base+23c90e
	{0xDA, "tibia.protobuf.protocol.GameserverMessageCyclopediaCharacterInfo"},				base+23d7cd
	{0xDB, "tibia.protobuf.protocol.GameserverMessageHirelingNameChange"},					base+23eb81
	{0xDC, "tibia.protobuf.protocol.GameserverMessageTutorialHint"},						base+23b84d
	{0xDD, "tibia.protobuf.protocol.GameserverMessageCyclopediaMapData"},					base+23ebaf
	{0xDE, "tibia.protobuf.protocol.GameserverMessageDailyRewardCollectionState"},			base+2404a8
	{0xDF, "tibia.protobuf.protocol.GameserverMessageCreditBalance"},						base+2414b0
	{0xE0, "tibia.protobuf.protocol.GameserverMessageStoreError"},							base+23fec5
	{0xE1, "tibia.protobuf.protocol.GameserverMessageRequestPurchaseData"},					base+2410d0
	{0xE2, "tibia.protobuf.protocol.GameserverMessageOpenRewardWall"},						base+23ffde
	{0xE3, "tibia.protobuf.protocol.GameserverMessageCloseRewardWall"},						no payload
	{0xE4, "tibia.protobuf.protocol.GameserverMessageDailyRewardBasic"},					base+2400b0
	{0xE5, "tibia.protobuf.protocol.GameserverMessageDailyRewardHistory"},					base+24041f
	{0xE6, "tibia.protobuf.protocol.GameserverMessagePreyFreeListRerollAvailability"},		base+23f902
	{0xE7, "tibia.protobuf.protocol.GameserverMessagePreyTimeLeft"},						base+23f902
	{0xE8, "tibia.protobuf.protocol.GameserverMessagePreyData"},							base+23f936
	{0xE9, "tibia.protobuf.protocol.GameserverMessagePreyPrices"},							base+23b2a2
	{0xEA, "tibia.protobuf.protocol.GameserverMessageOfferDescription"},					base+23fb53
	{0xEB, "tibia.protobuf.protocol.GameserverMessageImbuingDialogRefresh"},				base+23fbb4
	{0xEC, "tibia.protobuf.protocol.GameserverMessageCloseImbuingDialog"},					no payload
	{0xED, "tibia.protobuf.protocol.GameserverMessageShowMessageDialog"},					base+23fec5
	{0xEE, "tibia.protobuf.protocol.GameserverMessageResourceBalance"},						base+23ff29
	{0xEF, "tibia.protobuf.protocol.GameserverMessageTibiaTime"},							base+23a5b0
	{0xF0, "tibia.protobuf.protocol.GameserverMessageQuestLog"},							base+23f073
	{0xF1, "tibia.protobuf.protocol.GameserverMessageQuestLine"},							base+23f110
	{0xF2, "tibia.protobuf.protocol.GameserverMessageUpdatingShopBalance"},					base+24151d
	{0xF3, "tibia.protobuf.protocol.GameserverMessageChannelEvent"},						base+23f1f8
	{0xF4, "tibia.protobuf.protocol.GameserverMessageObjectInfo"},							base+23f272
	{0xF5, "tibia.protobuf.protocol.GameserverMessagePlayerInventory"},						base+23f301
	{0xF6, "tibia.protobuf.protocol.GameserverMessageMarketEnter"},							base+23f35f
	{0xF7, "tibia.protobuf.protocol.GameserverMessageMarketLeave"},							no payload
	{0xF8, "tibia.protobuf.protocol.GameserverMessageMarketDetail"},						base+23f3d3
	{0xF9, "tibia.protobuf.protocol.GameserverMessageMarketBrowse"},						base+24066f
	{0xFA, "tibia.protobuf.protocol.GameserverMessageShowModalDialog"},						base+2408bc
	{0xFB, "tibia.protobuf.protocol.GameserverMessageStoreCategories"},						base+240a96
	{0xFC, "tibia.protobuf.protocol.GameserverMessageStoreOffers"},							base+240be4
	{0xFD, "tibia.protobuf.protocol.GameserverMessageTransactionHistory"},					base+24153c
	{0xFE, "tibia.protobuf.protocol.GameserverMessageStoreSuccess"},						base+23fec5
*/

class Thing;
class Creature;
class Item;
class ItemUI;
class Tile;
class ProtocolGame : public Protocol
{
	public:
		ProtocolGame();

		virtual ProtocolGame* getProtocolGame() {return this;}
		virtual const ProtocolGame* getProtocolGame() const {return this;}

		bool canPerformAction() {return true;}

		virtual void parseMessage(InputMessage& msg);
		virtual void onConnect();
		virtual void onConnectionError(ConnectionError error);
		virtual void onDisconnect();

		//Main Functions
		void sendServerName(const std::string& serverName);
		void sendLogin(Uint32 challengeTimestamp, Uint8 challengeRandom);
		void sendSecondaryLogin();
		void sendEnterGame();
		void sendLogout();
		void sendPing();
		void sendPingBack();
		void sendConnectionPingBack();

		//Walking Events
		void sendAutoWalk(const std::vector<Direction>& path);
		void sendWalk(Direction dir);
		void sendTurnNorth();
		void sendTurnEast();
		void sendTurnSouth();
		void sendTurnWest();

		//Item Move Events
		void sendEquipItem(Uint16 itemid, Uint16 count);
		void sendMove(const Position& fromPos, Uint16 itemid, Uint8 stackpos, const Position& toPos, Uint16 count);

		//NPC Trade System
		void sendLookInShop(Uint16 itemid, Uint16 count);
		void sendNPCPurchase(Uint16 itemid, Uint8 subtype, Uint16 amount, bool ignoreCapacity, bool buyWithBackpack);
		void sendNPCSell(Uint16 itemid, Uint8 subtype, Uint16 amount, bool ignoreEquipped);
		void sendNPCClose();

		//Trade System
		void sendRequestTrade(const Position& position, Uint16 thingId, Uint8 stackpos, Uint32 creatureId);
		void sendLookInTrade(bool counterOffer, Uint8 index);
		void sendAcceptTrade();
		void sendCloseTrade();

		//Use Item Events
		void sendUseItem(const Position& position, Uint16 itemId, Uint8 stackpos, Uint8 index);
		void sendUseItemEx(const Position& fromPos, Uint16 itemId, Uint8 fromStackPos, const Position& toPos, Uint16 toItemId, Uint8 toStackPos);
		void sendUseOnCreature(const Position& position, Uint16 itemId, Uint8 stackpos, Uint32 creatureId);
		void sendRotateItem(const Position& position, Uint16 itemId, Uint8 stackpos);
		void sendRequestItemInfo(std::vector<RequestItems>& requestItems);
		void sendWrapState(const Position& position, Uint16 itemId, Uint8 stackpos);

		//Container System
		void sendCloseContainer(Uint8 containerId);
		void sendUpContainer(Uint8 containerId);
		void sendUpdateContainer(Uint8 containerId);
		void sendSeekInContainer(Uint8 containerId, Uint16 index);
		void sendBrowseField(const Position& position);
		void sendOpenParentContainer(const Position& position);
		void sendOpenDepotSearch();
		void sendCloseDepotSearch();
		void sendDepotSearchType(Uint16 itemId);
		void sendDepotSearchRetrieve(Uint16 itemId, Uint8 itemCount);
		void sendStashAction(Sint8 unk1);

		//Text Windows Events
		void sendTextWindow(Uint32 windowId, const std::string& text);
		void sendHouseWindow(Uint32 windowId, Uint8 doorId, const std::string& text);

		//Looks Events
		void sendLookAt(const Position& position, Uint16 thingId, Uint8 stackpos);
		void sendLookInBattle(Uint32 creatureId);

		//Talk Event
		void sendSay(MessageMode mode, Uint16 channelId, const std::string& receiver, const std::string& message);
		void sendUpdateExivaOptions();

		//Chat System
		void sendRequestChannels();
		void sendOpenChannel(Uint16 channelId);
		void sendCloseChannel(Uint16 channelId);
		void sendOpenPrivateChannel(const std::string& receiver);
		void sendOpenRuleViolationChannel(const std::string& reporter);
		void sendCloseRuleViolationChannel(const std::string& reporter);
		void sendCancelRuleViolationChannel();
		void sendCloseNPCChannel();
		void sendCreatePrivateChannel();
		void sendChannelInvite(const std::string& target, Uint16 channelId);
		void sendChannelExclude(const std::string& target, Uint16 channelId);

		//Fight System
		void sendAttackModes(Uint8 attackMode, Uint8 chaseMode, Uint8 secureMode, Uint8 pvpMode);
		void sendAttack(Uint32 creatureId, Uint32 sequence);
		void sendFollow(Uint32 creatureId, Uint32 sequence);
		void sendCancelAttackAndFollow();

		//PvP System
		void sendJoinAggression(Uint32 creatureId);

		//Party System
		void sendInviteToParty(Uint32 creatureId);
		void sendJoinToParty(Uint32 creatureId);
		void sendRevokePartyInvitation(Uint32 creatureId);
		void sendPassPartyLeadership(Uint32 creatureId);
		void sendLeaveParty();
		void sendEnableSharedPartyExperience(bool active);
		
		//Outfit Events
		void sendRequestOutfit();
		void sendSetOutfit(Uint16 lookType, Uint8 lookHead, Uint8 lookBody, Uint8 lookLegs, Uint8 lookFeet, Uint8 lookAddons, Uint16 lookMount);
		void sendMount(bool active);

		//Vip System
		void sendAddVip(const std::string& name);
		void sendRemoveVip(Uint32 playerGUID);
		void sendEditVip(Uint32 playerGUID, const std::string& description, Uint32 iconId, bool notifyLogin, std::vector<Uint8>& groupIds);
		void sendAddVipGroup(Uint8 groupType, const std::string& name);
		void sendFriendSystemAction();

		//Reports System
		void sendNewBugReport(BugReportCategory category, const std::string& comment, const std::string& typoSpeaker, const std::string& typoText);
		void sendBugReport(const std::string& comment);
		void sendRuleViolation(const std::string& target, Uint8 reason, Uint8 action, const std::string& comment, const std::string& statement, Uint16 statementId, bool ipBanishment);
		void sendNewRuleViolation(Uint8 reason, Uint8 action, const std::string& characterName, const std::string& comment, const std::string& translation, Uint32 statementId);
		void sendDebugReport(const std::string& line, const std::string& date, const std::string& decription, const std::string& comment);

		//Questlog System
		void sendShowQuestLog();
		void sendShowQuestLine(Uint16 questId);

		//Modal Dialog Window
		void sendAnswerModalDialog(Uint32 dialogId, Uint8 button, Uint8 choice);

		//Store System
		void sendOpenStore(Uint8 serviceType, const std::string& category);
		void sendRequestStore(Uint8 serviceType, const std::string& category);
		void sendBuyInStore(Uint32 offerId, StoreProductTypes productType, const std::string& name);
		void sendOpenTransactionHistory(Uint8 entriesPerPage);
		void sendRequestTransactionHistory(Uint32 page, Uint8 entriesPerPage);
		void sendTransferCoins(const std::string& recipient, Uint32 amount);
		void sendGetTransactionDetails(Uint32 unknown);
		void sendStoreEvent();

		//Market System
		void sendMarketLeave();
		void sendMarketBrowse(Uint16 browseId);
		void sendMarketCreateOffer(Uint8 type, Uint16 thingId, Uint16 amount, Uint32 price, bool anonymous);
		void sendMarketCancelOffer(Uint32 timestamp, Uint16 counter);
		void sendMarketAcceptOffer(Uint32 timestamp, Uint16 counter, Uint16 amount);

		//Analytics System
		void sendTrackQuestFlags(std::vector<Uint16>& missionIds);
		void sendOpenCyclopediaCharacterInfo(CyclopediaCharacterInfoType characterInfoType);
		void sendFeatureEvent(FeatureEventType type, bool active);

		//Inspection System
		void sendInspectPlayer(InspectPlayerState playerState, Uint32 playerGUID = 0);
		void sendInspectObject(InspectObjectData& inspectData);

		//Imbuing System
		void sendApplyImbuement(Uint8 slot, Uint32 imbuementId, bool protectiveCharm);
		void sendClearCharm(Uint8 slot);
		void sendImbuementLeave();

		//Prey system
		void sendPreyAction(Uint8 preyId, PreyAction action, Uint8 listIndex);

		//Cyclopedia System
		void sendMarketStatistics();
		void sendOpenMonsterCyclopedia();
		void sendOpenMonsterCyclopediaMonsters(const std::string& race);
		void sendOpenMonsterCyclopediaRace(Uint16 raceId);
		void sendCyclopediaMapAction();
		void sendMonsterBonusEffectAction(CyclopediaBonusEffectAction action, Uint8 charmId, Uint16 raceId);
		void sendSetHirelingName();

		//Reward Wall System
		void sendOpenRewardWall();
		void sendOpenRewardHistory();
		void sendColectReward(bool shrine, std::vector<RewardCollection>& rewards);

		//Quick Loot System
		void sendQuickLoot(const Position& position, Uint16 itemId, Uint8 stackpos);
		void sendLootContainer(LootContainerData& lootData);
		void sendQuickLootBlackWhitelist(QuickLootFilter filter, std::vector<Uint16>& itemIds);

		//Compedium System
		void sendSubscribeToUpdates(bool unknownB, Uint8 unknownU);
		void sendMarkGameNewsAsRead(Uint32 newsId, bool readed);

		//Tournament System
		void sendTournamentInformation();
		void sendTournamentLeaderboard(const std::string& worldName, Uint16 currentPage, const std::string& searchCharacter, Uint8 elementsPerPage = 20);
		void sendTournamentTicketAction();

		//Some Ingame Stuff Sends
		void sendThankYou(Uint32 statementId);
		void sendGetOfferDescription(Uint32 offerId);
		void sendRequestResourceBalance(Uint8 resource);
		void sendGreet(Uint32 statementId);
		void sendGuildMessage();
		void sendEditGuildMessage(const std::string& message);
		void sendBlessingsDialog();
		void sendClientCheck();
		void sendTeleport(const Position& position);

	private:
		bool canSee(const Position& position);

		MessageMode translateMessageModeFromServer(Uint8 mode);
		Uint8 translateMessageModeToServer(MessageMode mode);

		//Main Functions
		void parseCreatureData(InputMessage& msg);
		void parseLogin(InputMessage& msg);
		void parseLoginOrPending(InputMessage& msg);
		void parseGMActions(InputMessage& msg);
		void parseReadyForSecondaryConnection(InputMessage& msg);
		void parseEnterGame(InputMessage& msg);
		void parseUpdateNeeded(InputMessage& msg);
		void parseLoginError(InputMessage& msg);
		void parseLoginAdvice(InputMessage& msg);
		void parseLoginWait(InputMessage& msg);
		void parseLoginToken(InputMessage& msg);
		void parseStoreButtonIndicators(InputMessage& msg);
		void parsePingBack(InputMessage& msg);
		void parsePing(InputMessage& msg);
		void parseChallenge(InputMessage& msg);
		void parseDeath(InputMessage& msg);
		void parseClientCheck(InputMessage& msg);
		void parseTime(InputMessage& msg);
		void parseScreenshotEvent(InputMessage& msg);

		//Map Parser
		void parseMapDescription(InputMessage& msg);
		void parseMapNorth(InputMessage& msg);
		void parseMapEast(InputMessage& msg);
		void parseMapSouth(InputMessage& msg);
		void parseMapWest(InputMessage& msg);
		void parseUpdateTile(InputMessage& msg);
		void parseTileAddThing(InputMessage& msg);
		void parseTileTransformThing(InputMessage& msg);
		void parseTileRemoveThing(InputMessage& msg);
		void parseTileMoveCreature(InputMessage& msg);
		void parseFloorChangeUp(InputMessage& msg);
		void parseFloorChangeDown(InputMessage& msg);

		//Container Parser
		void parseContainerOpen(InputMessage& msg);
		void parseContainerClose(InputMessage& msg);
		void parseContainerAddItem(InputMessage& msg);
		void parseContainerTransformItem(InputMessage& msg);
		void parseContainerRemoveItem(InputMessage& msg);
		void parseStash(InputMessage& msg);
		void parseSpecialContainersAvailable(InputMessage& msg);
		void parseDepotSearchResult(InputMessage& msg);
		void parseDepotSearchDetailList(InputMessage& msg);
		void parseCloseDepotSearch(InputMessage& msg);

		//Inspection Parser
		void parseInspectionList(InputMessage& msg);
		void parseInspectionState(InputMessage& msg);

		//Inventory Parser
		void parseInventoryTransformItem(InputMessage& msg);
		void parseInventoryRemoveItem(InputMessage& msg);

		//Npc Trade Parser
		void parseNpcOpenTrade(InputMessage& msg);
		void parseNpcPlayerGoods(InputMessage& msg);
		void parseNpcCloseTrade(InputMessage& msg);

		//Trade Parser
		void parseTradeOwn(InputMessage& msg);
		void parseTradeCounter(InputMessage& msg);
		void parseTradeClose(InputMessage& msg);

		//World Light Parser
		void parseAmbientWorldLight(InputMessage& msg);

		//Effects Pareser
		void parseMagicEffects(InputMessage& msg);
		void parseMagicEffect(InputMessage& msg);
		void parseRemoveMagicEffect(InputMessage& msg);
		void parseAnimatedText(InputMessage& msg);
		void parseDistanceEffect(InputMessage& msg);

		//Creature Parser
		void parseCreatureMark(InputMessage& msg);
		void parseCreatureTrappers(InputMessage& msg);
		void parseCreatureUpdate(InputMessage& msg);
		void parseCreatureHealth(InputMessage& msg);
		void parseCreatureLight(InputMessage& msg);
		void parseCreatureOutfit(InputMessage& msg);
		void parseCreatureSpeed(InputMessage& msg);
		void parseCreatureSkull(InputMessage& msg);
		void parseCreatureParty(InputMessage& msg);
		void parseCreatureUnpassable(InputMessage& msg);
		void parseCreatureMarks(InputMessage& msg);
		void parseCreatureHelpers(InputMessage& msg);
		void parseCreatureType(InputMessage& msg);

		//Text Window Parser
		void parseEditTextWindow(InputMessage& msg);
		void parseEditHouseWindow(InputMessage& msg);

		//Some Ingame Stuff Parser
		void parseBlessingsDialog(InputMessage& msg);
		void parseBlessings(InputMessage& msg);
		void parsePreset(InputMessage& msg);
		void parsePremiumTrigger(InputMessage& msg);
		void parseTextMessage(InputMessage& msg);
		void parseCancelWalk(InputMessage& msg);
		void parseWalkWait(InputMessage& msg);
		void parseUnjustifiedStats(InputMessage& msg);
		void parsePvpSituations(InputMessage& msg);
		void parseUpdateLootContainers(InputMessage& msg);
		void parseChooseOutfit(InputMessage& msg);
		void parseExivaSuppressed(InputMessage& msg);
		void parseUpdateExivaOptions(InputMessage& msg);
		void parseTutorialHint(InputMessage& msg);
		void parseAutomapFlag(InputMessage& msg);
		void parseModalWindow(InputMessage& msg);
		void parseShowMessageDialog(InputMessage& msg);
		void parseOfferDescription(InputMessage& msg);
		void parseEditGuildMessage(InputMessage& msg);

		//Player Parser
		void parsePlayerDataBasic(InputMessage& msg);
		void parsePlayerData(InputMessage& msg);
		void parsePlayerSkills(InputMessage& msg);
		void parsePlayerIcons(InputMessage& msg);
		void parsePlayerCancelTarget(InputMessage& msg);
		void parsePlayerSpellDelay(InputMessage& msg);
		void parsePlayerSpellGroupDelay(InputMessage& msg);
		void parsePlayerMultiUseDelay(InputMessage& msg);
		void parsePlayerModes(InputMessage& msg);
		void parsePlayerStoreDeepLink(InputMessage& msg);
		void parsePlayerTalk(InputMessage& msg);
		void parseItemInfo(InputMessage& msg);
		void parsePlayerInventory(InputMessage& msg);
		void parseRestingAreaState(InputMessage& msg);

		//Chat Parser
		void parseChannels(InputMessage& msg);
		void parseOpenChannel(InputMessage& msg);
		void parseOpenPrivateChannel(InputMessage& msg);
		void parseRuleViolationChannel(InputMessage& msg);
		void parseRuleViolationRemove(InputMessage& msg);
		void parseRuleViolationCancel(InputMessage& msg);
		void parseRuleViolationLock(InputMessage& msg);
		void parseOpenOwnChannel(InputMessage& msg);
		void parseCloseChannel(InputMessage& msg);
		void parseChannelEvent(InputMessage& msg);

		//Vip Parser
		void parseVipAdd(InputMessage& msg);
		void parseVipStatus(InputMessage& msg);
		void parseVipStatusLogout(InputMessage& msg);
		void parseFriendSystemData(InputMessage& msg);

		//Quest Parser
		void parseQuestLog(InputMessage& msg);
		void parseQuestLine(InputMessage& msg);
		void parseTrackedQuestFlags(InputMessage& msg);

		//Market Parser
		void parseMarketStatistics(InputMessage& msg);
		void parseMarketEnter(InputMessage& msg);
		void parseMarketLeave(InputMessage& msg);
		void parseMarketDetail(InputMessage& msg);
		void parseMarketBrowse(InputMessage& msg);

		//Store Parser
		void parseStoreCoinBalance(InputMessage& msg);
		void parseStoreError(InputMessage& msg);
		void parseStoreRequestPurchase(InputMessage& msg);
		void parseStoreCoinBalanceUpdating(InputMessage& msg);
		void parseStoreOpen(InputMessage& msg);
		void parseStoreOffers(InputMessage& msg);
		void parseStoreTransactionHistory(InputMessage& msg);
		void parseStoreCompletePurchase(InputMessage& msg);
		void parseTransactionDetails(InputMessage& msg);
		
		//Analytics Parser
		void parseImpactTracking(InputMessage& msg);
		void parseItemWasted(InputMessage& msg);
		void parseItemLooted(InputMessage& msg);
		void parseKillTracking(InputMessage& msg);

		//Imbuing Parser
		void parseRefreshImbuingDialog(InputMessage& msg);
		void parseCloseImbuingDialog(InputMessage& msg);
		void parseResourceBalance(InputMessage& msg);

		//Prey Parser
		void parsePreyFreeListRerollAvailability(InputMessage& msg);
		void parsePreyTimeLeft(InputMessage& msg);
		void parsePreyData(InputMessage& msg);
		void parsePreyPrices(InputMessage& msg);

		//Cyclopedia Parser
		void parseMonsterCyclopedia(InputMessage& msg);
		void parseMonsterCyclopediaMonsters(InputMessage& msg);
		void parseMonsterCyclopediaRace(InputMessage& msg);
		void parseMonsterCyclopediaBonusEffects(InputMessage& msg);
		void parseMonsterCyclopediaNewDetails(InputMessage& msg);
		void parseCyclopediaCharacterInfo(InputMessage& msg);
		void parseHirelingNameChange(InputMessage& msg);
		void parseCyclopediaMapData(InputMessage& msg);

		//Reward Wall Parser
		void parseOpenRewardWall(InputMessage& msg);
		void parseCloseRewardWall(InputMessage& msg);
		void parseDailyRewardBasic(InputMessage& msg);
		void parseDailyRewardHistory(InputMessage& msg);
		void parseDailyRewardCollectionState(InputMessage& msg);
		
		//Compedium Parser
		void parseGameNews(InputMessage& msg);

		//Tournament Parser
		void parsePlayerDataTournament(InputMessage& msg);
		void parseTournamentInformation(InputMessage& msg);
		void parseTournamentLeaderboard(InputMessage& msg);

		Thing* internalGetThing(InputMessage& msg, const Position& pos);
		Creature* getCreature(InputMessage& msg, Uint16 thingId, const Position& pos);
		Item* getItem(InputMessage& msg, Uint16 thingId, const Position& pos);
		ItemUI* getItemUI(InputMessage& msg, Uint16 thingId);

		void setMapDescription(InputMessage& msg, Sint32 x, Sint32 y, Uint8 z, Sint32 width, Sint32 height);
		Sint32 setFloorDescription(InputMessage& msg, Sint32 x, Sint32 y, Uint8 z, Sint32 width, Sint32 height, Sint32 offset, Sint32 skip);
		Sint32 setTileDescription(InputMessage& msg, const Position& position, Sint32 offset);
		void updateMinimapTile(const Position& position, Tile* tile);

		Uint32 m_expectChannels;
		Uint32 m_pingReceived;
		Uint32 m_pingSent;
		Uint32 m_pingTime;

		ConnectionError m_lastError;
		bool m_skipErrors;
		bool m_gameInitialized;
		bool m_pendingGame;
};

#endif /* __FILE_PROTOCOL_GAME_h_ */
