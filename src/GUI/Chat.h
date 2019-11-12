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

#ifndef __FILE_CHAT_h_
#define __FILE_CHAT_h_

#include "../defines.h"

#define CHANNEL_ID_DEFAULT 0xFFFFFFF0
#define CHANNEL_ID_SERVERLOG 0xFFFFFFF1
#define CHANNEL_ID_NPC 0xFFFFFFF2
#define CHANNEL_ID_RVR 0xFFFFFFF3

class GUI_Console;
struct Channel
{
	std::string channelName;
	GUI_Console* channelConsole;
	Uint32 channelId;
	Uint32 highlightTime;
	bool workAsServerLog;
	bool channelClosable;
	bool privateChannel;
	bool alreadyClosed;
	bool unreadMessage;
};

enum Volumes
{
	VOLUME_SAY = 0,
	VOLUME_YELL = 1,
	VOLUME_WHISPER = 2
};

enum MessageFlags
{
	MESSAGE_FLAG_REPORTNAME = (1 << 0),
	MESSAGE_FLAG_REPORTSTATEMENT = (1 << 1)
};

class GUI_TextBox;
class Chat
{
	public:
		Chat();
		~Chat();

		void clear();
		void gameStart();
		void navigateHistory(Sint32 direction);

		void setOwnPrivateChannel(Uint32 channelId) {m_ownPrivatechannel = channelId;}
		void openPrivateChannel(const std::string& receiver);
		void openChannel(Uint32 channelId, const std::string channelName, bool privateChannel = false, bool closable = true);
		void closeChannel(Uint32 channelId);
		void leaveChannel(Uint32 channelId);
		void addChannelMessage(Uint32 channelId, MessageMode mode, Uint32 statementId, const std::string& playerName, Uint16 playerLevel, const std::string& text, time_t timestamp);
		void sendMessage();
		Channel* getPrivateChannel(const std::string& receiver);
		Channel* getChannel(Uint32 channelId);
		Channel* getCurrentChannel();

		Uint32 getOwnPrivateChannel() {return m_ownPrivatechannel;}
		Uint32 getHelpChannelId();

		std::pair<size_t, size_t> calculateChannelPages(iRect& rect);
		void onTextInput(const char* textInput);
		void onKeyDown(SDL_Event event);
		void onKeyUp(SDL_Event event);
		void onLMouseDown(iRect& rect, Sint32 x, Sint32 y);
		void onLMouseUp(iRect& rect, Sint32 x, Sint32 y);
		void onRMouseDown(iRect& rect, Sint32 x, Sint32 y);
		void onRMouseUp(iRect& rect, Sint32 x, Sint32 y);
		void onWheel(iRect& rect, Sint32 x, Sint32 y, bool wheelUP);
		void onMouseMove(iRect& rect, Sint32 x, Sint32 y);
		void render(iRect& rect);

	protected:
		std::list<std::string> m_savedMessages;
		std::vector<Channel> m_channels;
		GUI_TextBox* m_textbox;
		size_t m_selectedChannel;
		size_t m_currentPage;
		Sint32 m_historyNavigator;
		Uint32 m_ownPrivatechannel;
		Uint8 m_ignoreListStatus;
		Uint8 m_channelListStatus;
		Uint8 m_serverLogStatus;
		Uint8 m_closeChannelStatus;
		Uint8 m_volumeStatus;
		Uint8 m_volumeAdjustement;
		Uint8 m_buttonNext;
		Uint8 m_buttonPrevious;
};

#endif /* __FILE_CHAT_h_ */
