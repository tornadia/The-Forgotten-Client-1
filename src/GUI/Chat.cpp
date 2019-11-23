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

//TODO: channel buttons moving, ignore list, resizing

#include "Chat.h"
#include "../GUI_Elements/GUI_TextBox.h"
#include "../GUI_Elements/GUI_Console.h"
#include "../engine.h"
#include "../game.h"

Chat g_chat;
extern Engine g_engine;
extern Game g_game;
extern Uint32 g_frameTime;

#define CHANNEL_ID_PRIVATE_START 0x10000
#define CHANNEL_ID_PRIVATE_END 0x12710

#define CHANNEL_MESSAGE_HELP_LEGACY "Welcome to the help channel! Feel free to ask questions concerning client controls, general game play, use of accounts and the official homepage. In-depth questions about the content of the game will not be answered. Experienced players will be glad to help you to the best of their knowledge. Keep in mind that this is not a chat channel for general conversations. Therefore please limit your statements to relevant questions and answers."
#define CHANNEL_MESSAGE_HELP "Welcome to the help channel.\nIn this channel you can ask questions about Tibia.Experienced players will gladly help you to the best of their knowledge.\nFor detailed information about quests and other game content please take a look at our supported fansites at http://www.tibia.com/community/?subtopic=fansites\nPlease remember to write in English here so everybody can understand you."
#define CHANNEL_MESSAGE_ADVERTISING "Here you can advertise all kinds of things. Among others, you can trade Tibia items, advertise ingame events, seek characters for a quest or a hunting group, find members for your guild or look for somebody to help you with something.\nIt goes without saying that all advertisements must be conform to the Tibia Rules.Keep in mind that it is illegal to advertise trades including premium time, real money or Tibia characters."
#define CHANNEL_MESSAGE_CLOSED "The channel has been closed. You need to re-join the channel if you get invited."

Chat::Chat()
{
	m_textbox = new GUI_TextBox(iRect(0, 0, 0, 0), "", 0, 223, 223, 223);
	m_textbox->setMaxLength(255);
	m_selectedChannel = 0;
	m_currentPage = 0;
	m_historyNavigator = SDL_MIN_SINT32;
	m_ownPrivatechannel = SDL_static_cast(Uint32, -1);
	m_ignoreListStatus = 0;
	m_channelListStatus = 0;
	m_serverLogStatus = 0;
	m_closeChannelStatus = 0;
	m_volumeStatus = 0;
	m_volumeAdjustement = VOLUME_SAY;
	m_buttonNext = 0;
	m_buttonPrevious = 0;
}

Chat::~Chat()
{
	delete m_textbox;
	clear();
}

void Chat::clear()
{
	for(std::vector<Channel>::iterator it = m_channels.begin(), end = m_channels.end(); it != end; ++it)
		delete (*it).channelConsole;

	m_channels.clear();
}

void Chat::gameStart()
{
	clear();
	if(g_clientVersion >= 870)
		openChannel(CHANNEL_ID_DEFAULT, CHANNEL_NAME_DEFAULT, false, false);
	else
		openChannel(CHANNEL_ID_DEFAULT, CHANNEL_NAME_DEFAULT_LEGACY, false, false);

	openChannel(CHANNEL_ID_SERVERLOG, CHANNEL_NAME_SERVERLOG, false, false);

	m_savedMessages.clear();
	m_textbox->setText("");
}

void Chat::navigateHistory(Sint32 direction)
{
	Sint32 messagesSize = SDL_static_cast(Sint32, m_savedMessages.size())-1;
	if(m_historyNavigator == SDL_MIN_SINT32)
	{
		if(m_savedMessages.empty() || direction > 0)
			return;

		m_historyNavigator = messagesSize+1;
	}
	m_historyNavigator = UTIL_max<Sint32>(0, UTIL_min<Sint32>(m_historyNavigator+direction, messagesSize));

	std::list<std::string>::iterator it = m_savedMessages.begin();
	std::advance(it, m_historyNavigator);

	std::string& historyMessage = (*it);
	m_textbox->setText(historyMessage);
	m_textbox->clearSelection();
	m_textbox->moveCursor(SDL_static_cast(Sint32, historyMessage.length()));
}

void Chat::openPrivateChannel(const std::string& receiver)
{
	//Search for available channelId
	Channel* channel = getPrivateChannel(receiver);
	if(channel)
	{
		setCurrentChannel(channel->channelId);
		return;
	}

	for(Uint32 i = CHANNEL_ID_PRIVATE_START; i < CHANNEL_ID_PRIVATE_END; ++i)
	{
		channel = getChannel(i);
		if(!channel)
		{
			openChannel(i, receiver, true);
			setCurrentChannel(i);
			return;
		}
	}
}

void Chat::openChannel(Uint32 channelId, const std::string channelName, bool privateChannel, bool closable)
{
	for(std::vector<Channel>::iterator it = m_channels.begin(), end = m_channels.end(); it != end; ++it)
	{
		Channel& currentChannel = (*it);
		if(currentChannel.channelId == channelId)
		{
			delete currentChannel.channelConsole;
			m_channels.erase(it);
			break;
		}
	}

	Channel newChannel;
	newChannel.channelName = std::move(channelName);
	newChannel.channelConsole = new GUI_Console(iRect(0, 0, 0, 0));
	newChannel.channelId = channelId;
	newChannel.highlightTime = 0;
	newChannel.workAsServerLog = false;
	newChannel.channelClosable = closable;
	newChannel.privateChannel = privateChannel;
	newChannel.alreadyClosed = false;
	newChannel.unreadMessage = false;
	m_channels.push_back(newChannel);
	if(g_clientVersion >= 871)
	{
		if(channelId == 7)//Help Channel
			addChannelMessage(channelId, MessageChannelManagement, 0, "", 0, CHANNEL_MESSAGE_HELP, time(NULL));
		else if(channelId == 5 || channelId == 6)//Advertising Channels
			addChannelMessage(channelId, MessageChannelManagement, 0, "", 0, CHANNEL_MESSAGE_ADVERTISING, time(NULL));
	}
	else if(g_clientVersion == 870)//Some weird special case with 8.70 version
	{
		if(channelId == 6)//Help Channel
			addChannelMessage(channelId, MessageChannelManagement, 0, "", 0, CHANNEL_MESSAGE_HELP, time(NULL));
		else if(channelId == 4 || channelId == 5)//Advertising Channels
			addChannelMessage(channelId, MessageChannelManagement, 0, "", 0, CHANNEL_MESSAGE_ADVERTISING, time(NULL));
	}
	else if(g_clientVersion >= 840)
	{
		if(channelId == 9)//Help Channel
		{
			if(g_clientVersion >= 854)
				addChannelMessage(channelId, MessageChannelManagement, 0, "", 0, CHANNEL_MESSAGE_HELP, time(NULL));
			else
				addChannelMessage(channelId, MessageGamemasterChannel, 0, "", 0, CHANNEL_MESSAGE_HELP_LEGACY, time(NULL));
		}
	}
	else if(g_clientVersion >= 790)
	{
		if(channelId == 8)//Help Channel
			addChannelMessage(channelId, MessageGamemasterChannel, 0, "", 0, CHANNEL_MESSAGE_HELP_LEGACY, time(NULL));
	}
	else
	{
		if(channelId == 7)//Help Channel
			addChannelMessage(channelId, MessageGamemasterChannel, 0, "", 0, CHANNEL_MESSAGE_HELP_LEGACY, time(NULL));
	}
}

void Chat::closeChannel(Uint32 channelId)
{
	addChannelMessage(channelId, MessageChannelManagement, 0, "", 0, CHANNEL_MESSAGE_CLOSED, time(NULL));
	for(std::vector<Channel>::iterator it = m_channels.begin(), end = m_channels.end(); it != end; ++it)
	{
		Channel& currentChannel = (*it);
		if(currentChannel.channelId == channelId)
		{
			currentChannel.alreadyClosed = true;
			break;
		}
	}
}

void Chat::leaveChannel(Uint32 channelId)
{
	bool alreadyClosed = false;
	for(std::vector<Channel>::iterator it = m_channels.begin(), end = m_channels.end(); it != end; ++it)
	{
		Channel& currentChannel = (*it);
		if(currentChannel.channelId == channelId)
		{
			alreadyClosed = currentChannel.alreadyClosed;
			delete currentChannel.channelConsole;
			m_channels.erase(it);
			break;
		}
	}

	if(channelId == CHANNEL_ID_NPC)
		g_game.sendCloseNPCChannel();
	else if(!alreadyClosed)
		g_game.sendCloseChannel(SDL_static_cast(Uint16, channelId));

	if(channelId == m_ownPrivatechannel)
		m_ownPrivatechannel = SDL_static_cast(Uint32, -1);

	if(m_selectedChannel > 0 && m_selectedChannel == m_channels.size())
		--m_selectedChannel;
}

void Chat::addChannelMessage(Uint32 channelId, MessageMode mode, Uint32 statementId, const std::string& playerName, Uint16 playerLevel, const std::string& text, time_t timestamp)
{
	Uint32 messageFlags = 0;
	Uint8 red, green, blue;
	switch(mode)
	{
		case MessageSay:
		case MessageWhisper:
		case MessageYell:
		case MessageChannel:
		{
			messageFlags = (MESSAGE_FLAG_REPORTNAME|MESSAGE_FLAG_REPORTSTATEMENT);
			red = 240;
			green = 240;
			blue = 0;
		}
		break;
		case MessageSpell:
		{
			messageFlags = MESSAGE_FLAG_REPORTNAME;
			red = 240;
			green = 240;
			blue = 0;
		}
		break;
		case MessageRVRContinue:
		{
			red = 240;
			green = 240;
			blue = 0;
		}
		break;
		case MessageMonsterSay:
		case MessageMonsterYell:
		case MessageBarkLow:
		case MessageBarkLoud:
		{
			red = 255;
			green = 100;
			blue = 0;
		}
		break;
		case MessageNpcFrom:
		case MessageNpcFromStartBlock:
		{
			red = 96;
			green = 250;
			blue = 250;
		}
		break;
		case MessageNpcTo:
		case MessageGamemasterPrivateTo:
		case MessageBlue:
		{
			red = 160;
			green = 160;
			blue = 255;
		}
		break;
		case MessagePrivateFrom:
		{
			messageFlags = (MESSAGE_FLAG_REPORTNAME|MESSAGE_FLAG_REPORTSTATEMENT);
			red = 96;
			green = 250;
			blue = 250;
		}
		break;
		case MessagePrivateTo:
		{
			messageFlags = (MESSAGE_FLAG_REPORTNAME|MESSAGE_FLAG_REPORTSTATEMENT);
			red = 160;
			green = 160;
			blue = 255;
		}
		break;
		case MessageChannelManagement:
		case MessageLogin:
		case MessageGame:
		case MessageDamageDealed:
		case MessageDamageReceived:
		case MessageHeal:
		case MessageExp:
		case MessageDamageOthers:
		case MessageHealOthers:
		case MessageExpOthers:
		case MessageStatus:
		case MessageReport:
		case MessageTutorialHint:
		case MessageThankyou:
		case MessageMana:
		case MessageBeyondLast:
		case MessageRVRChannel:
		{
			red = 255;
			green = 255;
			blue = 255;
		}
		break;
		case MessageChannelHighlight:
		case MessageGuild:
		case MessagePartyManagement:
		case MessageParty:
		{
			messageFlags = (MESSAGE_FLAG_REPORTNAME|MESSAGE_FLAG_REPORTSTATEMENT);
			red = 255;
			green = 255;
			blue = 255;
		}
		break;
		case MessageGamemasterBroadcast:
		case MessageGamemasterChannel:
		case MessageGamemasterPrivateFrom:
		case MessageWarning:
		case MessageRed:
		case MessageRVRAnswer:
		case MessageGameHighlight:
		{
			red = 250;
			green = 96;
			blue = 96;
		}
		break;
		case MessageLook:
		case MessageHotkeyUse:
		case MessageLoot:
		case MessageTradeNpc:
		{
			red = 0;
			green = 240;
			blue = 0;
		}
		break;
		default: return;
	}

	if(channelId == CHANNEL_ID_NPC)
	{
		Channel* NPCchannel = getChannel(CHANNEL_ID_NPC);
		if(!NPCchannel)
		{
			openChannel(CHANNEL_ID_NPC, CHANNEL_NAME_NPC);
			setCurrentChannel(CHANNEL_ID_NPC);
		}
	}

	for(std::vector<Channel>::iterator it = m_channels.begin(), end = m_channels.end(); it != end; ++it)
	{
		Channel& currentChannel = (*it);
		if(currentChannel.channelId == channelId || (channelId == CHANNEL_ID_SERVERLOG && currentChannel.workAsServerLog))
		{
			currentChannel.channelConsole->addMessage(statementId, timestamp, playerName, playerLevel, text, red, green, blue, messageFlags);
			currentChannel.highlightTime = g_frameTime+1000;
			currentChannel.unreadMessage = true;
			return;
		}
	}
}

void Chat::sendMessage()
{
	Channel* selectedchannel = getCurrentChannel();
	if(!selectedchannel)
		return;

	std::string message = m_textbox->getActualText();
	if(message.empty())
		return;

	Uint32 channelId = selectedchannel->channelId;
	std::string receiver;
	MessageMode mode = (m_volumeAdjustement == VOLUME_SAY ? MessageSay : m_volumeAdjustement == VOLUME_YELL ? MessageYell : m_volumeAdjustement == VOLUME_WHISPER ? MessageWhisper : MessageNone);
	if(selectedchannel->alreadyClosed)
		mode = MessageNone;
	else if(selectedchannel->privateChannel)
	{
		mode = MessagePrivateTo;
		receiver = selectedchannel->channelName;
	}
	else if(channelId == CHANNEL_ID_NPC)
		mode = MessageNpcTo;
	else if(channelId != CHANNEL_ID_DEFAULT && channelId != CHANNEL_ID_SERVERLOG)
		mode = MessageChannel;

	if(m_savedMessages.size() >= CHAT_MAXIMUM_MESSAGE_HISTORY)
		m_savedMessages.pop_front();

	m_savedMessages.push_back(message);
	m_historyNavigator = SDL_MIN_SINT32;

	m_textbox->setText("");
	m_textbox->clearSelection();
	if(message.size() > 3)
	{
		//#a, #d and $name$text are deprecated and not supported by any ots(?) so I don't see any point to implement them
		if(message[0] == '#')
		{
			char command = message[1];
			size_t found = message.find(' ', 2);
			if(found != std::string::npos)
				message = message.substr(found+1);
			else
				return;

			if(command == 'b')
				mode = MessageGamemasterBroadcast;
			else if(command == 'c')
			{
				if(mode != MessageChannel)
				{
					mode = MessageNone;
					g_game.processTextMessage(MessageFailure, "This is not a chat channel.");
				}
				else
					mode = MessageGamemasterChannel;
			}
			else if(command == 'i')
			{
				mode = MessageNone;
				if(m_ownPrivatechannel != SDL_static_cast(Uint32, -1))
					g_game.sendChannelInvite(message, SDL_static_cast(Uint16, m_ownPrivatechannel));
			}
			else if(command == 'x')
			{
				mode = MessageNone;
				if(m_ownPrivatechannel != SDL_static_cast(Uint32, -1))
					g_game.sendChannelExclude(message, SDL_static_cast(Uint16, m_ownPrivatechannel));
			}
			else if(command == 's')
				mode = MessageSay;
			else if(command == 'w')
				mode = MessageWhisper;
			else if(command == 'y')
				mode = MessageYell;
		}
		else if(message[0] == '@')
		{
			size_t found = message.find('@', 2);
			if(found != std::string::npos)
			{
				mode = MessageGamemasterPrivateTo;
				receiver = message.substr(1, found-1);
				if(selectedchannel->privateChannel && receiver != selectedchannel->channelName)
				{
					Channel* havePrivateChanel = getPrivateChannel(receiver);
					if(!havePrivateChanel)
						channelId = CHANNEL_ID_DEFAULT;
					else
						channelId = havePrivateChanel->channelId;
				}
				message = message.substr(found+1);
			}
		}
		else if(message[0] == '*')
		{
			size_t found = message.find('*', 2);
			if(found != std::string::npos)
			{
				mode = MessagePrivateTo;
				receiver = message.substr(1, found-1);
				if(selectedchannel->privateChannel && receiver != selectedchannel->channelName)
				{
					m_textbox->setText(message.substr(0, found+1) + ' ');
					Channel* havePrivateChanel = getPrivateChannel(receiver);
					if(!havePrivateChanel)
						channelId = CHANNEL_ID_DEFAULT;
					else
						channelId = havePrivateChanel->channelId;
				}
				message = message.substr(found+1);
			}
		}
	}

	switch(mode)
	{
		case MessageSay:
		case MessageWhisper:
		case MessageYell:
		case MessageGamemasterBroadcast:
		case MessageChannel:
		case MessageGamemasterChannel:
			g_game.sendSay(mode, SDL_static_cast(Uint16, channelId), receiver, message);
			break;
		case MessagePrivateTo:
		case MessageGamemasterPrivateTo:
		{
			addChannelMessage(channelId, mode, 0, g_game.getPlayerName(), g_game.getPlayerLevel(), message, time(NULL));
			g_game.sendSay(mode, 0, receiver, message);
		}
		break;
		case MessageNpcTo:
		{
			addChannelMessage(channelId, mode, 0, g_game.getPlayerName(), g_game.getPlayerLevel(), message, time(NULL));
			g_game.sendSay(mode, 0, receiver, message);
		}
		break;
	}
}

Channel* Chat::getPrivateChannel(const std::string& receiver)
{
	for(std::vector<Channel>::iterator it = m_channels.begin(), end = m_channels.end(); it != end; ++it)
	{
		Channel& currentChannel = (*it);
		if(currentChannel.privateChannel && currentChannel.channelName == receiver)
			return &currentChannel;
	}
	return NULL;
}

Channel* Chat::getChannel(Uint32 channelId)
{
	for(std::vector<Channel>::iterator it = m_channels.begin(), end = m_channels.end(); it != end; ++it)
	{
		Channel& currentChannel = (*it);
		if(currentChannel.channelId == channelId)
			return &currentChannel;
	}
	return NULL;
}

void Chat::switchToNextChannel()
{
	if(m_channels.empty())
		return;

	++m_selectedChannel;
	if(m_selectedChannel == m_channels.size())
		m_selectedChannel = 0;
}

void Chat::switchToPreviousChannel()
{
	if(m_channels.empty())
		return;

	if(m_selectedChannel == 0)
		m_selectedChannel = m_channels.size();
	--m_selectedChannel;
}

void Chat::setCurrentChannel(Uint32 channelId)
{
	for(size_t it = 0, end = m_channels.size(); it != end; ++it)
	{
		Channel& currentChannel = m_channels[it];
		if(currentChannel.channelId == channelId)
		{
			m_selectedChannel = it;
			return;
		}
	}
}

Channel* Chat::getCurrentChannel()
{
	if(m_channels.empty())
		return NULL;

	if(m_selectedChannel < m_channels.size())
		return &m_channels[m_selectedChannel];

	m_selectedChannel = 0;
	return &m_channels[m_selectedChannel];
}

Uint32 Chat::getHelpChannelId()
{
	if(g_clientVersion >= 871)
		return 7;
	else if(g_clientVersion == 870)//Some weird special case with 8.70 version
		return 6;
	else if(g_clientVersion >= 840)
		return 9;
	else if(g_clientVersion >= 790)
		return 8;

	return 7;
}

std::pair<size_t, size_t> Chat::calculateChannelPages(iRect& rect)
{
	size_t maxChannels = SDL_static_cast(size_t, (rect.x2-100)/96);
	size_t channels = m_channels.size();
	if(channels > maxChannels)
		return std::make_pair(channels-maxChannels, maxChannels);

	return std::make_pair(0, maxChannels);
}

void Chat::onTextInput(const char* textInput)
{
	Channel* selectedchannel = getCurrentChannel();
	if(!selectedchannel)
		return;

	m_textbox->onTextInput(textInput);
}

void Chat::onKeyDown(SDL_Event event)
{
	Channel* selectedchannel = getCurrentChannel();
	if(!selectedchannel)
		return;

	m_textbox->onKeyDown(event);
}

void Chat::onKeyUp(SDL_Event event)
{
	Channel* selectedchannel = getCurrentChannel();
	if(!selectedchannel)
		return;

	m_textbox->onKeyUp(event);
}

void Chat::onLMouseDown(iRect& rect, Sint32 x, Sint32 y)
{
	Channel* selectedchannel = getCurrentChannel();
	if(!selectedchannel)
		return;

	iRect irect = iRect(rect.x1+rect.x2-16, rect.y1+5, 16, 16);
	if(irect.isPointInside(x, y))
		m_ignoreListStatus = 1;

	irect = iRect(rect.x1+rect.x2-32, rect.y1+5, 16, 16);
	if(irect.isPointInside(x, y))
		m_channelListStatus = 1;

	if(selectedchannel->channelClosable)
	{
		irect = iRect(rect.x1+rect.x2-48, rect.y1+5, 16, 16);
		if(irect.isPointInside(x, y))
			m_serverLogStatus = 1;
		
		irect = iRect(rect.x1+rect.x2-64, rect.y1+5, 16, 16);
		if(irect.isPointInside(x, y))
			m_closeChannelStatus = 1;
	}
	else
	{
		irect = iRect(rect.x1+5, rect.y1+rect.y2-20, 16, 16);
		if(irect.isPointInside(x, y))
			m_volumeStatus = 1;
	}

	std::pair<size_t, size_t> calculatedPages = calculateChannelPages(rect);
	if(calculatedPages.first > 0)
	{
		if(m_currentPage > calculatedPages.first)
			m_currentPage = calculatedPages.first;

		if(m_currentPage != calculatedPages.first)
		{
			irect = iRect(rect.x1+rect.x2-82, rect.y1+5, 18, 18);
			if(irect.isPointInside(x, y))
				m_buttonNext = 1;
		}
		if(m_currentPage != 0)
		{
			irect = iRect(rect.x1, rect.y1+5, 18, 18);
			if(irect.isPointInside(x, y))
				m_buttonPrevious = 1;
		}
	}
	else
		m_currentPage = 0;

	Sint32 posX = rect.x1+18;
	irect = iRect(posX, rect.y1+5, rect.x2-100, 18);
	if(irect.isPointInside(x, y))
	{
		for(size_t it = m_currentPage, end = UTIL_min<size_t>(m_channels.size(), it+calculatedPages.second+1); it != end; ++it)
		{
			irect = iRect(posX, rect.y1+5, 96, 18);
			if(irect.isPointInside(x, y))
			{
				m_selectedChannel = it;
				break;
			}
			posX += 96;
		}
	}

	if(m_textbox->getRect().isPointInside(x, y))
	{
		m_textbox->onLMouseDown(x, y);
		selectedchannel->channelConsole->clearSelection();
	}
	else if(selectedchannel->channelConsole->getRect().isPointInside(x, y))
	{
		m_textbox->clearSelection();
		selectedchannel->channelConsole->onLMouseDown(x, y);
	}
}

void Chat::onLMouseUp(iRect& rect, Sint32 x, Sint32 y)
{
	Channel* selectedchannel = getCurrentChannel();
	if(!selectedchannel)
	{
		m_ignoreListStatus = 0;
		m_channelListStatus = 0;
		m_serverLogStatus = 0;
		m_closeChannelStatus = 0;
		m_volumeStatus = 0;
		m_buttonNext = 0;
		m_buttonPrevious = 0;
		return;
	}
	
	if(m_ignoreListStatus > 0)
	{
		m_ignoreListStatus = 0;

		/*iRect irect = iRect(rect.x1+rect.x2-16, rect.y1+5, 16, 16);
		if(irect.isPointInside(x, y))
			;*/
	}
	else if(m_channelListStatus > 0)
	{
		m_channelListStatus = 0;

		iRect irect = iRect(rect.x1+rect.x2-32, rect.y1+5, 16, 16);
		if(irect.isPointInside(x, y))
			g_game.sendRequestChannels();
	}
	else if(m_serverLogStatus > 0)
	{
		m_serverLogStatus = 0;

		iRect irect = iRect(rect.x1+rect.x2-48, rect.y1+5, 16, 16);
		if(irect.isPointInside(x, y) && selectedchannel->channelClosable)
			selectedchannel->workAsServerLog = !selectedchannel->workAsServerLog;
	}
	else if(m_closeChannelStatus > 0)
	{
		m_closeChannelStatus = 0;

		iRect irect = iRect(rect.x1+rect.x2-64, rect.y1+5, 16, 16);
		if(irect.isPointInside(x, y) && selectedchannel->channelClosable)
			leaveChannel(selectedchannel->channelId);
	}
	else if(m_volumeStatus > 0)
	{
		m_volumeStatus = 0;

		iRect irect = iRect(rect.x1+5, rect.y1+rect.y2-20, 16, 16);
		if(irect.isPointInside(x, y))
		{
			if(m_volumeAdjustement == VOLUME_SAY)
				m_volumeAdjustement = VOLUME_YELL;
			else if(m_volumeAdjustement == VOLUME_YELL)
				m_volumeAdjustement = VOLUME_WHISPER;
			else if(m_volumeAdjustement == VOLUME_WHISPER)
				m_volumeAdjustement = VOLUME_SAY;
		}
	}
	else if(m_buttonNext > 0)
	{
		m_buttonNext = 0;

		iRect irect = iRect(rect.x1+rect.x2-82, rect.y1+5, 18, 18);
		if(irect.isPointInside(x, y))
			++m_currentPage;
	}
	else if(m_buttonPrevious > 0)
	{
		m_buttonPrevious = 0;

		iRect irect = iRect(rect.x1, rect.y1+5, 18, 18);
		if(irect.isPointInside(x, y))
			--m_currentPage;
	}

	m_textbox->onLMouseUp(x, y);
	selectedchannel->channelConsole->onLMouseUp(x, y);
}

void Chat::onRMouseDown(iRect&, Sint32 x, Sint32 y)
{
	Channel* selectedchannel = getCurrentChannel();
	if(!selectedchannel)
		return;

	if(m_textbox->getRect().isPointInside(x, y))
		m_textbox->onRMouseDown(x, y);
	else if(selectedchannel->channelConsole->getRect().isPointInside(x, y))
		selectedchannel->channelConsole->onRMouseDown(x, y);
}

void Chat::onRMouseUp(iRect&, Sint32 x, Sint32 y)
{
	Channel* selectedchannel = getCurrentChannel();
	if(!selectedchannel)
		return;

	m_textbox->onRMouseUp(x, y);
	selectedchannel->channelConsole->onRMouseUp(x, y);
}

void Chat::onWheel(iRect&, Sint32 x, Sint32 y, bool wheelUP)
{
	Channel* selectedchannel = getCurrentChannel();
	if(!selectedchannel)
		return;

	if(m_textbox->getRect().isPointInside(x, y))
		m_textbox->onWheel(x, y, wheelUP);
	else if(selectedchannel->channelConsole->getRect().isPointInside(x, y))
		selectedchannel->channelConsole->onWheel(x, y, wheelUP);
}

void Chat::onMouseMove(iRect& rect, Sint32 x, Sint32 y)
{
	Channel* selectedchannel = getCurrentChannel();
	if(!selectedchannel)
		return;

	if(m_ignoreListStatus > 0)
	{
		iRect irect = iRect(rect.x1+rect.x2-16, rect.y1+5, 16, 16);
		bool inside = irect.isPointInside(x, y);
		if(m_ignoreListStatus == 1 && !inside)
			m_ignoreListStatus = 2;
		else if(m_ignoreListStatus == 2 && inside)
			m_ignoreListStatus = 1;
	}
	else if(m_channelListStatus > 0)
	{
		iRect irect = iRect(rect.x1+rect.x2-32, rect.y1+5, 16, 16);
		bool inside = irect.isPointInside(x, y);
		if(m_channelListStatus == 1 && !inside)
			m_channelListStatus = 2;
		else if(m_channelListStatus == 2 && inside)
			m_channelListStatus = 1;
	}
	else if(m_serverLogStatus > 0)
	{
		iRect irect = iRect(rect.x1+rect.x2-48, rect.y1+5, 16, 16);
		bool inside = irect.isPointInside(x, y);
		if(m_serverLogStatus == 1 && !inside)
			m_serverLogStatus = 2;
		else if(m_serverLogStatus == 2 && inside)
			m_serverLogStatus = 1;
	}
	else if(m_closeChannelStatus > 0)
	{
		iRect irect = iRect(rect.x1+rect.x2-64, rect.y1+5, 16, 16);
		bool inside = irect.isPointInside(x, y);
		if(m_closeChannelStatus == 1 && !inside)
			m_closeChannelStatus = 2;
		else if(m_closeChannelStatus == 2 && inside)
			m_closeChannelStatus = 1;
	}
	else if(m_volumeStatus > 0)
	{
		iRect irect = iRect(rect.x1+5, rect.y1+rect.y2-20, 16, 16);
		bool inside = irect.isPointInside(x, y);
		if(m_volumeStatus == 1 && !inside)
			m_volumeStatus = 2;
		else if(m_volumeStatus == 2 && inside)
			m_volumeStatus = 1;
	}

	bool isInsideParent = rect.isPointInside(x, y);
	m_textbox->onMouseMove(x, y, isInsideParent);
	selectedchannel->channelConsole->onMouseMove(x, y, isInsideParent);
}

void Chat::render(iRect& rect)
{
	Surface* renderer = g_engine.getRender();
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_X, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_Y, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_W, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_H, rect.x1, rect.y1, rect.x2, 1);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_BACKGROUND_GREY_X, GUI_UI_BACKGROUND_GREY_Y, GUI_UI_BACKGROUND_GREY_W, GUI_UI_BACKGROUND_GREY_H, rect.x1, rect.y1+1, rect.x2, 3);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_LINE_DARK_X, GUI_UI_ICON_HORIZONTAL_LINE_DARK_Y, GUI_UI_ICON_HORIZONTAL_LINE_DARK_W, GUI_UI_ICON_HORIZONTAL_LINE_DARK_H, rect.x1, rect.y1+4, rect.x2, 1);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_CONSOLE_BACKGROUND_X, GUI_UI_ICON_CONSOLE_BACKGROUND_Y, GUI_UI_ICON_CONSOLE_BACKGROUND_W, GUI_UI_ICON_CONSOLE_BACKGROUND_H, rect.x1, rect.y1+5, rect.x2, 16);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_BACKGROUND_GREY_X, GUI_UI_BACKGROUND_GREY_Y, GUI_UI_BACKGROUND_GREY_W, GUI_UI_BACKGROUND_GREY_H, rect.x1+2, rect.y1+23, rect.x2-4, rect.y2-25);
	
	renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_TOPLEFT_BORDER_X, GUI_UI_ICON_TOPLEFT_BORDER_Y, rect.x1+4, rect.y1+26, GUI_UI_ICON_TOPLEFT_BORDER_W, GUI_UI_ICON_TOPLEFT_BORDER_H);
	renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BOTLEFT_BORDER_X, GUI_UI_ICON_BOTLEFT_BORDER_Y, rect.x1+4, rect.y1+rect.y2-25, GUI_UI_ICON_BOTLEFT_BORDER_W, GUI_UI_ICON_BOTLEFT_BORDER_H);
	renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_TOPRIGHT_BORDER_X, GUI_UI_ICON_TOPRIGHT_BORDER_Y, rect.x1+rect.x2-7, rect.y1+26, GUI_UI_ICON_TOPRIGHT_BORDER_W, GUI_UI_ICON_TOPRIGHT_BORDER_H);
	renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BOTRIGHT_BORDER_X, GUI_UI_ICON_BOTRIGHT_BORDER_Y, rect.x1+rect.x2-7, rect.y1+rect.y2-25, GUI_UI_ICON_BOTRIGHT_BORDER_W, GUI_UI_ICON_BOTRIGHT_BORDER_H);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_DIVIDER_X, GUI_UI_ICON_HORIZONTAL_DIVIDER_Y, GUI_UI_ICON_HORIZONTAL_DIVIDER_W, GUI_UI_ICON_HORIZONTAL_DIVIDER_H, rect.x1+7, rect.y1+26, rect.x2-14, 3);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_DIVIDER_X, GUI_UI_ICON_HORIZONTAL_DIVIDER_Y, GUI_UI_ICON_HORIZONTAL_DIVIDER_W, GUI_UI_ICON_HORIZONTAL_DIVIDER_H, rect.x1+7, rect.y1+rect.y2-25, rect.x2-14, 3);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_VERTICAL_DIVIDER_X, GUI_UI_ICON_VERTICAL_DIVIDER_Y, GUI_UI_ICON_VERTICAL_DIVIDER_W, GUI_UI_ICON_VERTICAL_DIVIDER_H, rect.x1+4, rect.y1+29, 3, rect.y2-54);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_VERTICAL_DIVIDER_X, GUI_UI_ICON_VERTICAL_DIVIDER_Y, GUI_UI_ICON_VERTICAL_DIVIDER_W, GUI_UI_ICON_VERTICAL_DIVIDER_H, rect.x1+rect.x2-7, rect.y1+29, 3, rect.y2-54);

	renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_EXTRA_BORDER_X, GUI_UI_ICON_EXTRA_BORDER_Y, rect.x1+rect.x2-2, rect.y1+21, GUI_UI_ICON_EXTRA_BORDER_W, GUI_UI_ICON_EXTRA_BORDER_H);
	renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_EXTRA_BORDER_X, GUI_UI_ICON_EXTRA_BORDER_Y, rect.x1, rect.y1+rect.y2-2, GUI_UI_ICON_EXTRA_BORDER_W, GUI_UI_ICON_EXTRA_BORDER_H);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_X, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_Y, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_W, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_H, rect.x1, rect.y1+21, rect.x2-2, 2);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_X, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_Y, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_W, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_H, rect.x1, rect.y1+23, 2, rect.y2-25);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_LINE_DARK_X, GUI_UI_ICON_HORIZONTAL_LINE_DARK_Y, GUI_UI_ICON_HORIZONTAL_LINE_DARK_W, GUI_UI_ICON_HORIZONTAL_LINE_DARK_H, rect.x1+2, rect.y1+rect.y2-2, rect.x2-2, 2);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_VERTICAL_LINE_DARK_X, GUI_UI_ICON_VERTICAL_LINE_DARK_Y, GUI_UI_ICON_VERTICAL_LINE_DARK_W, GUI_UI_ICON_VERTICAL_LINE_DARK_H, rect.x1+rect.x2-2, rect.y1+23, 2, rect.y2-25);

	Channel* selectedchannel = getCurrentChannel();
	if(!selectedchannel)
		return;

	selectedchannel->highlightTime = 0;
	selectedchannel->unreadMessage = false;

	std::pair<size_t, size_t> calculatedPages = calculateChannelPages(rect);
	if(calculatedPages.first > 0)
	{
		if(m_currentPage > calculatedPages.first)
			m_currentPage = calculatedPages.first;

		if(m_currentPage != calculatedPages.first)
		{
			Uint8 buttonType = 0;
			for(size_t it = m_currentPage+calculatedPages.second, end = m_channels.size(); it != end; ++it)
			{
				Channel& currentChannel = m_channels[it];
				if(currentChannel.highlightTime >= g_frameTime)
				{
					buttonType = 2;
					break;
				}
				else if(currentChannel.unreadMessage)
					buttonType = 1;
			}
			if(buttonType == 2)
			{
				if(m_buttonNext == 1)
					renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BROWSE_RIGHT_UPDATE_DOWN_X, GUI_UI_ICON_BROWSE_RIGHT_UPDATE_DOWN_Y, rect.x1+rect.x2-82, rect.y1+5, GUI_UI_ICON_BROWSE_RIGHT_UPDATE_DOWN_W, GUI_UI_ICON_BROWSE_RIGHT_UPDATE_DOWN_H);
				else
					renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BROWSE_RIGHT_UPDATE_UP_X, GUI_UI_ICON_BROWSE_RIGHT_UPDATE_UP_Y, rect.x1+rect.x2-82, rect.y1+5, GUI_UI_ICON_BROWSE_RIGHT_UPDATE_UP_W, GUI_UI_ICON_BROWSE_RIGHT_UPDATE_UP_H);
			}
			else if(buttonType == 1)
			{
				if(m_buttonNext == 1)
					renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BROWSE_RIGHT_FLASH_DOWN_X, GUI_UI_ICON_BROWSE_RIGHT_FLASH_DOWN_Y, rect.x1+rect.x2-82, rect.y1+5, GUI_UI_ICON_BROWSE_RIGHT_FLASH_DOWN_W, GUI_UI_ICON_BROWSE_RIGHT_FLASH_DOWN_H);
				else
					renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BROWSE_RIGHT_FLASH_UP_X, GUI_UI_ICON_BROWSE_RIGHT_FLASH_UP_Y, rect.x1+rect.x2-82, rect.y1+5, GUI_UI_ICON_BROWSE_RIGHT_FLASH_UP_W, GUI_UI_ICON_BROWSE_RIGHT_FLASH_UP_H);
			}
			else
			{
				if(m_buttonNext == 1)
					renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BROWSE_RIGHT_DOWN_X, GUI_UI_ICON_BROWSE_RIGHT_DOWN_Y, rect.x1+rect.x2-82, rect.y1+5, GUI_UI_ICON_BROWSE_RIGHT_DOWN_W, GUI_UI_ICON_BROWSE_RIGHT_DOWN_H);
				else
					renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BROWSE_RIGHT_UP_X, GUI_UI_ICON_BROWSE_RIGHT_UP_Y, rect.x1+rect.x2-82, rect.y1+5, GUI_UI_ICON_BROWSE_RIGHT_UP_W, GUI_UI_ICON_BROWSE_RIGHT_UP_H);
			}
		}
		if(m_currentPage != 0)
		{
			Uint8 buttonType = 0;
			for(size_t it = 0, end = m_currentPage; it != end; ++it)
			{
				Channel& currentChannel = m_channels[it];
				if(currentChannel.highlightTime >= g_frameTime)
				{
					buttonType = 2;
					break;
				}
				else if(currentChannel.unreadMessage)
					buttonType = 1;
			}
			if(buttonType == 2)
			{
				if(m_buttonPrevious == 1)
					renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BROWSE_LEFT_UPDATE_DOWN_X, GUI_UI_ICON_BROWSE_LEFT_UPDATE_DOWN_Y, rect.x1, rect.y1+5, GUI_UI_ICON_BROWSE_LEFT_UPDATE_DOWN_W, GUI_UI_ICON_BROWSE_LEFT_UPDATE_DOWN_H);
				else
					renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BROWSE_LEFT_UPDATE_UP_X, GUI_UI_ICON_BROWSE_LEFT_UPDATE_UP_Y, rect.x1, rect.y1+5, GUI_UI_ICON_BROWSE_LEFT_UPDATE_UP_W, GUI_UI_ICON_BROWSE_LEFT_UPDATE_UP_H);
			}
			else if(buttonType == 1)
			{
				if(m_buttonPrevious == 1)
					renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BROWSE_LEFT_FLASH_DOWN_X, GUI_UI_ICON_BROWSE_LEFT_FLASH_DOWN_Y, rect.x1, rect.y1+5, GUI_UI_ICON_BROWSE_LEFT_FLASH_DOWN_W, GUI_UI_ICON_BROWSE_LEFT_FLASH_DOWN_H);
				else
					renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BROWSE_LEFT_FLASH_UP_X, GUI_UI_ICON_BROWSE_LEFT_FLASH_UP_Y, rect.x1, rect.y1+5, GUI_UI_ICON_BROWSE_LEFT_FLASH_UP_W, GUI_UI_ICON_BROWSE_LEFT_FLASH_UP_H);
			}
			else
			{
				if(m_buttonPrevious == 1)
					renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BROWSE_LEFT_DOWN_X, GUI_UI_ICON_BROWSE_LEFT_DOWN_Y, rect.x1, rect.y1+5, GUI_UI_ICON_BROWSE_LEFT_DOWN_W, GUI_UI_ICON_BROWSE_LEFT_DOWN_H);
				else
					renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BROWSE_LEFT_UP_X, GUI_UI_ICON_BROWSE_LEFT_UP_Y, rect.x1, rect.y1+5, GUI_UI_ICON_BROWSE_LEFT_UP_W, GUI_UI_ICON_BROWSE_LEFT_UP_H);
			}
		}
	}
	else
		m_currentPage = 0;

	Sint32 posX = rect.x1+18;
	renderer->setClipRect(posX, rect.y1+5, rect.x2-100, 18);
	for(size_t it = m_currentPage, end = UTIL_min<size_t>(m_channels.size(), it+calculatedPages.second+1); it != end; ++it)
	{
		Channel& currentChannel = m_channels[it];
		if(m_selectedChannel == it)
		{
			renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_ACTIVE_CHANNEL_X, GUI_UI_ICON_ACTIVE_CHANNEL_Y, posX, rect.y1+5, GUI_UI_ICON_ACTIVE_CHANNEL_W, GUI_UI_ICON_ACTIVE_CHANNEL_H);
			g_engine.drawFont(CLIENT_FONT_OUTLINED, posX+48, rect.y1+9, currentChannel.channelName, 223, 223, 223, CLIENT_FONT_ALIGN_CENTER);
		}
		else
		{
			renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_UNACTIVE_CHANNEL_X, GUI_UI_ICON_UNACTIVE_CHANNEL_Y, posX, rect.y1+5, GUI_UI_ICON_UNACTIVE_CHANNEL_W, GUI_UI_ICON_UNACTIVE_CHANNEL_H);
			if(currentChannel.highlightTime >= g_frameTime)
				g_engine.drawFont(CLIENT_FONT_OUTLINED, posX+48, rect.y1+9, currentChannel.channelName, 250, 250, 250, CLIENT_FONT_ALIGN_CENTER);
			else if(currentChannel.unreadMessage)
				g_engine.drawFont(CLIENT_FONT_OUTLINED, posX+48, rect.y1+9, currentChannel.channelName, 250, 96, 96, CLIENT_FONT_ALIGN_CENTER);
			else
				g_engine.drawFont(CLIENT_FONT_OUTLINED, posX+48, rect.y1+9, currentChannel.channelName, 127, 127, 127, CLIENT_FONT_ALIGN_CENTER);
		}
		posX += 96;
	}
	renderer->disableClipRect();

	renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_IGNORE_WINDOW_UP_X, (m_ignoreListStatus == 1 ? GUI_UI_ICON_IGNORE_WINDOW_DOWN_Y : GUI_UI_ICON_IGNORE_WINDOW_UP_Y), rect.x1+rect.x2-16, rect.y1+5, GUI_UI_ICON_IGNORE_WINDOW_UP_W, GUI_UI_ICON_IGNORE_WINDOW_UP_H);
	renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_REQUEST_CHANNELS_UP_X, (m_channelListStatus == 1 ? GUI_UI_ICON_REQUEST_CHANNELS_DOWN_Y : GUI_UI_ICON_REQUEST_CHANNELS_UP_Y), rect.x1+rect.x2-32, rect.y1+5, GUI_UI_ICON_REQUEST_CHANNELS_UP_W, GUI_UI_ICON_REQUEST_CHANNELS_UP_H);
	if(selectedchannel->channelClosable)
	{
		renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_CONSOLE_MESSAGES_UP_X, ((selectedchannel->workAsServerLog || m_serverLogStatus == 1) ? GUI_UI_ICON_CONSOLE_MESSAGES_DOWN_Y : GUI_UI_ICON_CONSOLE_MESSAGES_UP_Y), rect.x1+rect.x2-48, rect.y1+5, GUI_UI_ICON_CONSOLE_MESSAGES_UP_W, GUI_UI_ICON_CONSOLE_MESSAGES_UP_H);
		renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_CLOSE_CHANNEL_UP_X, (m_closeChannelStatus == 1 ? GUI_UI_ICON_CLOSE_CHANNEL_DOWN_Y : GUI_UI_ICON_CLOSE_CHANNEL_UP_Y), rect.x1+rect.x2-64, rect.y1+5, GUI_UI_ICON_CLOSE_CHANNEL_UP_W, GUI_UI_ICON_CLOSE_CHANNEL_UP_H);
	}
	else
	{
		if(m_volumeAdjustement == VOLUME_SAY)
			renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_VOLUME1_WINDOW_UP_X, (m_volumeStatus == 1 ? GUI_UI_ICON_VOLUME1_WINDOW_DOWN_Y : GUI_UI_ICON_VOLUME1_WINDOW_UP_Y), rect.x1+5, rect.y1+rect.y2-20, GUI_UI_ICON_VOLUME1_WINDOW_UP_W, GUI_UI_ICON_VOLUME1_WINDOW_UP_H);
		else if(m_volumeAdjustement == VOLUME_YELL)
			renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_VOLUME2_WINDOW_UP_X, (m_volumeStatus == 1 ? GUI_UI_ICON_VOLUME2_WINDOW_DOWN_Y : GUI_UI_ICON_VOLUME2_WINDOW_UP_Y), rect.x1+5, rect.y1+rect.y2-20, GUI_UI_ICON_VOLUME2_WINDOW_UP_W, GUI_UI_ICON_VOLUME2_WINDOW_UP_H);
		else if(m_volumeAdjustement == VOLUME_WHISPER)
			renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_VOLUME0_WINDOW_UP_X, (m_volumeStatus == 1 ? GUI_UI_ICON_VOLUME0_WINDOW_DOWN_Y : GUI_UI_ICON_VOLUME0_WINDOW_UP_Y), rect.x1+5, rect.y1+rect.y2-20, GUI_UI_ICON_VOLUME0_WINDOW_UP_W, GUI_UI_ICON_VOLUME0_WINDOW_UP_H);
	}

	if(!m_textbox->isActive())
		m_textbox->activate();

	iRect tRect = iRect(rect.x1+23, rect.y1+rect.y2-20, rect.x2-27, 16);
	m_textbox->setRect(tRect);
	m_textbox->render();

	iRect cRect = iRect(rect.x1+6, rect.y1+28, rect.x2-12, rect.y2-52);
	selectedchannel->channelConsole->setRect(cRect);
	selectedchannel->channelConsole->render();
}
