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

#include "engine.h"
#include "surfaceSoftware.h"
#include "surfaceDirect3D9.h"
#include "surfaceDirect3D11.h"
#include "surfaceDirectDraw.h"
#include "surfaceOpengl.h"
#include "surfaceOpengles.h"
#include "surfaceOpengles2.h"
#include "surfaceVulkan.h"
#include "item.h"
#include "map.h"
#include "tile.h"
#include "thingManager.h"
#include "spriteManager.h"
#include "animator.h"
#include "protocollogin.h"
#include "protocolgame.h"
#include "creature.h"
#include "game.h"
#include "config.h"

#include "GUI_Elements/GUI_Window.h"
#include "GUI_Elements/GUI_Panel.h"
#include "GUI_Elements/GUI_PanelWindow.h"
#include "GUI_Elements/GUI_Description.h"
#include "GUI_Elements/GUI_ContextMenu.h"
#include "GUI_Elements/GUI_Log.h"
#include "GUI/GUI_UTIL.h"
#include "GUI/Chat.h"

extern Map g_map;
extern ThingManager g_thingManager;
extern SpriteManager g_spriteManager;
extern Connection* g_connection;
extern GUI_Log g_logger;
extern Game g_game;
extern Chat g_chat;

extern bool g_running;
extern bool g_inited;
extern Sint32 g_actualCursor;
extern Uint32 g_frameTime;
extern Uint32 g_spriteCounts;
extern Uint16 g_pictureCounts;
extern Uint16 g_ping;

GUI_Window* g_mainWindow = NULL;

Engine::Engine()
{
	m_engines.push_back(CLIENT_ENGINE_SOFTWARE);
	#if defined(SDL_VIDEO_VULKAN)
	m_engines.push_back(CLIENT_ENGINE_VULKAN);
	#endif
	#if defined(SDL_VIDEO_RENDER_D3D11)
	m_engines.push_back(CLIENT_ENGINE_DIRECT3D11);
	#endif
	#if defined(SDL_VIDEO_RENDER_DDRAW)
	m_engines.push_back(CLIENT_ENGINE_DIRECT3D7);
	#endif
	#if defined(SDL_VIDEO_RENDER_OGL)
	m_engines.push_back(CLIENT_ENGINE_OPENGL);
	#else
	#if defined(SDL_VIDEO_RENDER_OGL_ES)
	m_engines.push_back(CLIENT_ENGINE_OPENGLES);
	#endif
	#if defined(SDL_VIDEO_RENDER_OGL_ES2)
	m_engines.push_back(CLIENT_ENGINE_OPENGLES2);
	#endif
	#endif
	#if defined(SDL_VIDEO_RENDER_D3D)
	m_engines.push_back(CLIENT_ENGINE_DIRECT3D);
	#endif

	m_clientHost.assign("127.0.0.1");
	m_clientPort.assign("7171");

	m_window = NULL;
	m_description = NULL;
	m_contextMenu = NULL;

	m_actWindow = NULL;
	m_surface = NULL;
	m_engine = m_engines.back();
	m_windowX = SDL_WINDOWPOS_CENTERED;
	m_windowY = SDL_WINDOWPOS_CENTERED;
	m_windowW = 640;
	m_windowH = 480;
	m_windowCachedW = 640;
	m_windowCachedH = 480;
	m_maximized = true;
	m_fullscreen = false;
	m_vsync = true;
	m_antialiasing = CLIENT_ANTIALIASING_NORMAL;
	m_perfMode = false;
	m_unlimitedFPS = true;
	m_attackMode = ATTACKMODE_BALANCED;
	m_chaseMode = CHASEMODE_STAND;
	m_secureMode = SECUREMODE_SECURE;
	m_pvpMode = PVPMODE_DOVE;
	m_actionData = CLIENT_ACTION_NONE;
	m_lightAmbient = 25;
	m_windowId = 0;
	m_controlFPS = false;
	m_sharpening = false;
	m_newCharacterList = false;
	m_ingame = false;
	m_showPerformance = false;
	m_showLogger = false;

	m_classicControl = true;
	m_autoChaseOff = true;
	m_showNames = true;
	m_showMarks = true;
	m_showPvPFrames = true;
	m_showIcons = true;
	m_showTextualEffects = true;
	m_showCooldown = true;

	m_showInfoMessages = true;
	m_showEventMessages = true;
	m_showStatusMessages = true;
	m_showTimestamps = true;
	m_showLevels = true;
	m_showPrivateMessages = true;

	m_showLevelBar = true;
	m_showStaminaBar = true;
	m_showMagLevelBar = true;
	m_showTrainingBar = true;
	for(Sint32 i = Skills_Fist; i < Skills_LastSkill; ++i)
		m_showSkillsBar[i] = true;

	m_motdText.assign("No current information.");
	m_motdNumber = 0;

	m_fullScreenWidth = 800;
	m_fullScreenHeight = 600;
	m_fullScreenBits = 32;
	m_fullScreenHZ = 60;

	m_characterSelectId = 0;
	m_accountPremDays = 0;
	m_accountStatus = AccountStatus_Ok;
	m_accountSubStatus = SubscriptionStatus_Free;
}

void Engine::loadCFG()
{
	Config cfg;
	SDL_snprintf(g_buffer, sizeof(g_buffer), "%sconfig.cfg", g_prefPath.c_str());
	if(cfg.openToRead(g_buffer))
	{
		std::string data = cfg.fetchKey("Version");
		if(data.empty())
			return;
		#if !(CLIENT_OVVERIDE_VERSION > 0)
		else
			g_clientVersion = SDL_static_cast(Uint32, SDL_strtoul(data.c_str(), NULL, 10));

		data = cfg.fetchKey("Host");
		m_clientHost.assign(data);
		data = cfg.fetchKey("Port");
		m_clientPort.assign(data);
		data = cfg.fetchKey("Proxy");
		m_clientProxy.assign(data);
		data = cfg.fetchKey("ProxyAuth");
		m_clientProxyAuth.assign(data);
		#endif

		data = cfg.fetchKey("Engine");
		m_engine = SDL_static_cast(Uint8, SDL_strtoul(data.c_str(), NULL, 10));
		data = cfg.fetchKey("BasicBrightness");
		m_lightAmbient = SDL_static_cast(Uint8, SDL_strtoul(data.c_str(), NULL, 10));
		if(m_lightAmbient > 100)
			m_lightAmbient = 100;

		data = cfg.fetchKey("WindowedMode");
		if(data.size() > 2)
		{
			data.pop_back();
			data.erase(data.begin());

			StringVector windowData = UTIL_explodeString(data, ",");
			if(windowData.size() == 5)
			{
				m_windowX = SDL_static_cast(Sint32, SDL_strtol(windowData[0].c_str(), NULL, 10));
				m_windowY = SDL_static_cast(Sint32, SDL_strtol(windowData[1].c_str(), NULL, 10));
				m_windowW = SDL_static_cast(Sint32, SDL_strtol(windowData[2].c_str(), NULL, 10));
				m_windowH = SDL_static_cast(Sint32, SDL_strtol(windowData[3].c_str(), NULL, 10));
				m_windowCachedW = m_windowW;
				m_windowCachedH = m_windowH;
				m_maximized = (windowData[4] == "yes" ? true : false);
			}
		}
		data = cfg.fetchKey("Fullscreen");
		m_fullscreen = (data == "yes" ? true : false);
		data = cfg.fetchKey("FullscreenMode");
		if(data.size() > 2)
		{
			data.pop_back();
			data.erase(data.begin());

			StringVector windowData = UTIL_explodeString(data, ",");
			if(windowData.size() == 3)
			{
				m_fullScreenWidth = SDL_static_cast(Sint32, SDL_strtol(windowData[0].c_str(), NULL, 10));
				m_fullScreenHeight = SDL_static_cast(Sint32, SDL_strtol(windowData[1].c_str(), NULL, 10));
				m_fullScreenBits = SDL_static_cast(Sint32, SDL_strtol(windowData[2].c_str(), NULL, 10));
			}
			else if(windowData.size() == 4)
			{
				m_fullScreenWidth = SDL_static_cast(Sint32, SDL_strtol(windowData[0].c_str(), NULL, 10));
				m_fullScreenHeight = SDL_static_cast(Sint32, SDL_strtol(windowData[1].c_str(), NULL, 10));
				m_fullScreenBits = SDL_static_cast(Sint32, SDL_strtol(windowData[2].c_str(), NULL, 10));
				m_fullScreenHZ = SDL_static_cast(Sint32, SDL_strtol(windowData[3].c_str(), NULL, 10));
			}
		}

		data = cfg.fetchKey("ConfineFramerate");
		SDL_setFramerate(&g_fpsmanager, SDL_static_cast(Uint32, SDL_strtoul(data.c_str(), NULL, 10)));
		data = cfg.fetchKey("NolimitFramerate");
		m_unlimitedFPS = (data == "yes" ? true : false);

		data = cfg.fetchKey("VerticalSync");
		m_vsync = (data == "yes" ? true : false);
		data = cfg.fetchKey("Sharpening");
		m_sharpening = (data == "yes" ? true : false);
		data = cfg.fetchKey("Antialiasing");
		m_antialiasing = (data == "yes" ? CLIENT_ANTIALIASING_NORMAL : data == "integer" ? CLIENT_ANTIALIASING_INTEGER : CLIENT_ANTIALIASING_NONE);
		data = cfg.fetchKey("PerfMode");
		m_perfMode = (data == "yes" ? true : false);

		data = cfg.fetchKey("ClassicControl");
		m_classicControl = (data == "yes" ? true : false);
		data = cfg.fetchKey("AutoChaseOff");
		m_autoChaseOff = (data == "yes" ? true : false);
		data = cfg.fetchKey("CreatureInfo");
		m_showNames = (SDL_static_cast(Uint32, SDL_strtoul(data.c_str(), NULL, 10)) > 0 ? true : false);
		data = cfg.fetchKey("CreatureMarks");
		m_showMarks = (data == "yes" ? true : false);
		data = cfg.fetchKey("CreaturePvPFrames");
		m_showPvPFrames = (data == "yes" ? true : false);
		data = cfg.fetchKey("CreatureIcons");
		m_showIcons = (data == "yes" ? true : false);
		data = cfg.fetchKey("TextualEffects");
		m_showTextualEffects = (data == "yes" ? true : false);
		data = cfg.fetchKey("CooldownBar");
		m_showCooldown = (data == "yes" ? true : false);

		data = cfg.fetchKey("InfoMessages");
		m_showInfoMessages = (data == "yes" ? true : false);
		data = cfg.fetchKey("EventMessages");
		m_showEventMessages = (data == "yes" ? true : false);
		data = cfg.fetchKey("StatusMessages");
		m_showStatusMessages = (data == "yes" ? true : false);
		data = cfg.fetchKey("TimeStamps");
		m_showTimestamps = (data == "yes" ? true : false);
		data = cfg.fetchKey("Levels");
		m_showLevels = (data == "yes" ? true : false);
		data = cfg.fetchKey("PrivateMessages");
		m_showPrivateMessages = (data == "yes" ? true : false);

		data = cfg.fetchKey("AttackMode");
		m_attackMode = SDL_static_cast(Uint8, SDL_strtoul(data.c_str(), NULL, 10));
		data = cfg.fetchKey("ChaseMode");
		m_chaseMode = SDL_static_cast(Uint8, SDL_strtoul(data.c_str(), NULL, 10));
		data = cfg.fetchKey("SecureMode");
		m_secureMode = SDL_static_cast(Uint8, SDL_strtoul(data.c_str(), NULL, 10));
		data = cfg.fetchKey("PvpMode");
		m_pvpMode = SDL_static_cast(Uint8, SDL_strtoul(data.c_str(), NULL, 10));

		data = cfg.fetchKey("LastMotD");
		m_motdNumber = SDL_static_cast(Uint32, SDL_strtoul(data.c_str(), NULL, 10));
		//data = cfg.fetchKey("AutomapZoom");
		//g_automap.setZoom(SDL_static_cast(Sint32, SDL_strtol(data.c_str(), NULL, 10)));
		//data = cfg.fetchKey("ConsoleHeight");

		//data = cfg.fetchKey("White");
		//data = cfg.fetchKey("Ignore");
		//data = cfg.fetchKey("KnownTutorialHints");

		data = cfg.fetchKey("LevelBar");
		m_showLevelBar = (data == "yes" ? true : false);
		data = cfg.fetchKey("StaminaBar");
		m_showStaminaBar = (data == "yes" ? true : false);
		data = cfg.fetchKey("MagLevelBar");
		m_showMagLevelBar = (data == "yes" ? true : false);
		data = cfg.fetchKey("OfflineTrainingBar");
		m_showTrainingBar = (data == "yes" ? true : false);
		data = cfg.fetchKey("SkillBars");
		if(data.size() > 2)
		{
			data.pop_back();
			data.erase(data.begin());
			for(Sint32 i = Skills_Fist; i < Skills_LastSkill; ++i)
				m_showSkillsBar[i] = false;

			StringVector skillsData = UTIL_explodeString(data, ",");
			for(StringVector::iterator it = skillsData.begin(), end = skillsData.end(); it != end; ++it)
			{
				Uint32 skillId = SDL_static_cast(Uint32, SDL_strtoul((*it).c_str(), NULL, 10));
				if(skillId < Skills_LastSkill)
					m_showSkillsBar[skillId] = true;
			}
		}

		StringVector vectorData = cfg.fetchKeys("ContentWindow");
		for(StringVector::iterator it = vectorData.begin(), end = vectorData.end(); it != end; ++it)
		{
			data = (*it);
			if(data.size() > 2)
			{
				data.pop_back();
				data.erase(data.begin());

				StringVector windowData = UTIL_explodeString(data, ",");
				if(windowData.size() == 2)
					m_contentWindows[SDL_static_cast(Uint32, SDL_strtoul(windowData[0].c_str(), NULL, 10))] = SDL_static_cast(Sint32, SDL_strtol(windowData[1].c_str(), NULL, 10));
			}
		}

		vectorData = cfg.fetchKeys("OpenDialogs");
		for(StringVector::iterator it = vectorData.begin(), end = vectorData.end(); it != end; ++it)
		{
			data = (*it);
			m_openDialogs.push_back(SDL_static_cast(Uint32, SDL_strtoul(data.c_str(), NULL, 10)));
		}

		//data = cfg.fetchKey("BuddySortmethods");
		//data = cfg.fetchKey("BuddyHideOffline");

		//data = cfg.fetchKey("SortOrderBuy");
		//data = cfg.fetchKey("SortOrderSell");
		//data = cfg.fetchKey("BuyWithBackpacks");
		//data = cfg.fetchKey("IgnoreCapacity");
		//data = cfg.fetchKey("SellEquipped");
	}
	#if CLIENT_OVVERIDE_VERSION > 0
	g_clientVersion = CLIENT_OVVERIDE_PROTOCOL_VERSION;
	g_game.clientChangeVersion(CLIENT_OVVERIDE_PROTOCOL_VERSION, CLIENT_OVVERIDE_FILE_VERSION);
	#else
	g_game.clientChangeVersion(g_clientVersion, g_clientVersion);
	#endif
}

void Engine::saveCFG()
{
	Config cfg;
	SDL_snprintf(g_buffer, sizeof(g_buffer), "%sconfig.cfg", g_prefPath.c_str());
	if(cfg.openToSave(g_buffer))
	{
		#if CLIENT_OVVERIDE_VERSION > 0
		Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", CLIENT_OVVERIDE_VERSION);
		#else
		Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", g_clientVersion);
		#endif
		cfg.insertKey("Version", std::string(g_buffer, SDL_static_cast(size_t, len)));

		#if !(CLIENT_OVVERIDE_VERSION > 0)
		cfg.insertKey("Host", m_clientHost);
		cfg.insertKey("Port", m_clientPort);
		cfg.insertKey("Proxy", m_clientProxy);
		cfg.insertKey("ProxyAuth", m_clientProxyAuth);
		#endif

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, m_engine));
		cfg.insertKey("Engine", std::string(g_buffer, SDL_static_cast(size_t, len)));

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", "yes");
		cfg.insertKey("LightEffects", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, m_lightAmbient));
		cfg.insertKey("BasicBrightness", std::string(g_buffer, SDL_static_cast(size_t, len)));

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "(%d,%d,%d,%d,%s)", m_windowX, m_windowY, m_windowCachedW, m_windowCachedH, (m_maximized ? "yes" : "no"));
		cfg.insertKey("WindowedMode", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_fullscreen ? "yes" : "no"));
		cfg.insertKey("Fullscreen", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "(%d,%d,%d,%d)", m_fullScreenWidth, m_fullScreenHeight, m_fullScreenBits, m_fullScreenHZ);
		cfg.insertKey("FullscreenMode", std::string(g_buffer, SDL_static_cast(size_t, len)));

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", g_fpsmanager.rate);
		cfg.insertKey("ConfineFramerate", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_unlimitedFPS ? "yes" : "no"));
		cfg.insertKey("NolimitFramerate", std::string(g_buffer, SDL_static_cast(size_t, len)));

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_vsync ? "yes" : "no"));
		cfg.insertKey("VerticalSync", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_sharpening ? "yes" : "no"));
		cfg.insertKey("Sharpening", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_antialiasing == CLIENT_ANTIALIASING_NORMAL ? "yes" : m_antialiasing == CLIENT_ANTIALIASING_INTEGER ? "integer" : "no"));
		cfg.insertKey("Antialiasing", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_perfMode ? "yes" : "no"));
		cfg.insertKey("PerfMode", std::string(g_buffer, SDL_static_cast(size_t, len)));

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_classicControl ? "yes" : "no"));
		cfg.insertKey("ClassicControl", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_autoChaseOff ? "yes" : "no"));
		cfg.insertKey("AutoChaseOff", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", (m_showNames ? 2 : 0));
		cfg.insertKey("CreatureInfo", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showMarks ? "yes" : "no"));
		cfg.insertKey("CreatureMarks", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showPvPFrames ? "yes" : "no"));
		cfg.insertKey("CreaturePvPFrames", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showIcons ? "yes" : "no"));
		cfg.insertKey("CreatureIcons", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showTextualEffects ? "yes" : "no"));
		cfg.insertKey("TextualEffects", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showCooldown ? "yes" : "no"));
		cfg.insertKey("CooldownBar", std::string(g_buffer, SDL_static_cast(size_t, len)));

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showInfoMessages ? "yes" : "no"));
		cfg.insertKey("InfoMessages", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showEventMessages ? "yes" : "no"));
		cfg.insertKey("EventMessages", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showStatusMessages ? "yes" : "no"));
		cfg.insertKey("StatusMessages", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showTimestamps ? "yes" : "no"));
		cfg.insertKey("TimeStamps", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showLevels ? "yes" : "no"));
		cfg.insertKey("Levels", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showPrivateMessages ? "yes" : "no"));
		cfg.insertKey("PrivateMessages", std::string(g_buffer, SDL_static_cast(size_t, len)));

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, m_attackMode));
		cfg.insertKey("AttackMode", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, m_chaseMode));
		cfg.insertKey("ChaseMode", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, m_secureMode));
		cfg.insertKey("SecureMode", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, m_pvpMode));
		cfg.insertKey("PvpMode", std::string(g_buffer, SDL_static_cast(size_t, len)));

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", m_motdNumber);
		cfg.insertKey("LastMotD", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", 1);
		cfg.insertKey("AutomapZoom", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", 0);
		cfg.insertKey("ConsoleHeight", std::string(g_buffer, SDL_static_cast(size_t, len)));

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "{}");
		cfg.insertKey("White", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "{}");
		cfg.insertKey("Ignore", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "{}");
		cfg.insertKey("KnownTutorialHints", std::string(g_buffer, SDL_static_cast(size_t, len)));

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showLevelBar ? "yes" : "no"));
		cfg.insertKey("LevelBar", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showStaminaBar ? "yes" : "no"));
		cfg.insertKey("StaminaBar", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showMagLevelBar ? "yes" : "no"));
		cfg.insertKey("MagLevelBar", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showTrainingBar ? "yes" : "no"));
		cfg.insertKey("OfflineTrainingBar", std::string(g_buffer, SDL_static_cast(size_t, len)));

		bool itFirst = true;
		Uint32 itPosition = 0;
		g_buffer[itPosition++] = '{';
		for(Sint32 i = Skills_Fist; i < Skills_LastSkill; ++i)
		{
			if(m_showSkillsBar[i])
			{
				if(itFirst)
					itFirst = false;
				else
					g_buffer[itPosition++] = ',';

				g_buffer[itPosition++] = SDL_static_cast(char, i) + '0';
			}
		}
		g_buffer[itPosition++] = '}';
		cfg.insertKey("SkillBars", std::string(g_buffer, SDL_static_cast(size_t, itPosition)));
		for(std::map<Uint32, Sint32>::iterator it = m_contentWindows.begin(), end = m_contentWindows.end(); it != end; ++it)
		{
			len = SDL_snprintf(g_buffer, sizeof(g_buffer), "(%u,%d)", it->first, it->second);
			cfg.insertKey("ContentWindow", std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		for(std::vector<Uint32>::iterator it = m_openDialogs.begin(), end = m_openDialogs.end(); it != end; ++it)
		{
			len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", (*it));
			cfg.insertKey("OpenDialogs", std::string(g_buffer, SDL_static_cast(size_t, len)));
		}

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, m_battleSortmethod));
		cfg.insertKey("BattleSortmethod", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_buddySortmethod == 0 ? "(0,1,2)" : m_buddySortmethod == 1 ? "(1,2,0)" : "(2,0,1)"));
		cfg.insertKey("BuddySortmethods", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_buddyHideOffline ? "yes" : "no"));
		cfg.insertKey("BuddyHideOffline", std::string(g_buffer, SDL_static_cast(size_t, len)));

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "0");
		cfg.insertKey("SortOrderBuy", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "2");
		cfg.insertKey("SortOrderSell", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "no");
		cfg.insertKey("BuyWithBackpacks", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "no");
		cfg.insertKey("IgnoreCapacity", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "no");
		cfg.insertKey("SellEquipped", std::string(g_buffer, SDL_static_cast(size_t, len)));
	}
}

void Engine::attachFullScreenInfo()
{
	SDL_DisplayMode displayMode;
	Sint32 displayIndex = SDL_GetWindowDisplayIndex(m_window);
	Sint32 displayModes = SDL_GetNumDisplayModes(displayIndex);
	for(Sint32 i = 0; i < displayModes; ++i)
	{
		Sint32 reportBits;
		SDL_GetDisplayMode(displayIndex, i, &displayMode);
		if(m_fullScreenWidth == displayMode.w && m_fullScreenHeight == displayMode.h && m_fullScreenHZ == displayMode.refresh_rate)
		{
			if(displayMode.format == SDL_PIXELFORMAT_UNKNOWN && m_fullScreenBits == 32)
			{
				SDL_SetWindowDisplayMode(m_window, &displayMode);
				return;
			}
			else
			{
				reportBits = SDL_BITSPERPIXEL(displayMode.format);
				if(reportBits == 24)
					reportBits = SDL_BYTESPERPIXEL(displayMode.format)*8;
				if(m_fullScreenBits == reportBits)
				{
					SDL_SetWindowDisplayMode(m_window, &displayMode);
					return;
				}
			}
		}
	}
}

void Engine::run()
{
	SDL_DisplayMode displayMode;
	if(SDL_GetDesktopDisplayMode(0, &displayMode) == 0)
	{
		m_fullScreenWidth = displayMode.w;
		m_fullScreenHeight = displayMode.h;
		m_fullScreenHZ = displayMode.refresh_rate;
		if(displayMode.format == SDL_PIXELFORMAT_UNKNOWN)
			m_fullScreenBits = 32;
		else
		{
			m_fullScreenBits = SDL_BITSPERPIXEL(displayMode.format);
			if(m_fullScreenBits == 24)
				m_fullScreenBits = SDL_BYTESPERPIXEL(displayMode.format)*8;
		}
	}
	loadCFG();

	Uint32 windowflags = (SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);
	if(m_fullscreen)
	{
		windowflags |= SDL_WINDOW_FULLSCREEN;
		m_windowW = m_fullScreenWidth;
		m_windowH = m_fullScreenHeight;
	}
	else if(m_maximized)
	{
		windowflags |= SDL_WINDOW_MAXIMIZED;
		m_windowW = m_windowCachedW;
		m_windowH = m_windowCachedH;
	}
	
	m_window = SDL_CreateWindow(PRODUCT_NAME, m_windowX, m_windowY, m_windowW, m_windowH, windowflags);
	if(!m_window)
	{
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Couldn't create SDL Window: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		g_running = false;
		return;
	}

	if(m_fullscreen)
	{
		attachFullScreenInfo();
		SDL_GetWindowDisplayMode(m_window, &displayMode);
		m_windowW = displayMode.w;
		m_windowH = displayMode.h;
	}
	else
	{
		SDL_GetWindowSize(m_window, &m_windowW, &m_windowH);
		SDL_SetWindowMinimumSize(m_window, GAME_MINIMUM_WIDTH, GAME_MINIMUM_HEIGHT);
	}
	m_windowId = SDL_GetWindowID(m_window);
	m_controlFPS = (!m_unlimitedFPS && !m_vsync);

	//createwindow don't generate resize event so send one ourselves
	UTIL_ResizeEvent(m_windowId, m_windowW, m_windowH);

	initFont(CLIENT_FONT_NONOUTLINED, 256, 128, 32, 7, 8, 16);
	initFont(CLIENT_FONT_OUTLINED, 512, 128, 32, 7, 16, 16);
	initFont(CLIENT_FONT_SMALL, 256, 64, 32, 7, 8, 8);

	UTIL_createMainWindow();
	resetToDefaultHotkeys(false);
}

void Engine::terminate()
{
	if(m_window && !m_fullscreen)
	{
		//Some disgusting "hack" to save original window parameters
		//TOOD: do it the proper way
		if(m_maximized)
			SDL_RestoreWindow(m_window);

		SDL_GetWindowPosition(m_window, &m_windowX, &m_windowY);
		SDL_GetWindowSize(m_window, &m_windowW, &m_windowH);
		m_windowCachedW = m_windowW;
		m_windowCachedH = m_windowH;
	}
	clearWindows();
	clearPanels();
	checkReleaseQueue();
	if(m_surface)
		delete m_surface;

	if(m_window)
	{
		SDL_DestroyWindow(m_window);
		m_window = NULL;
	}
	saveCFG();
}

void Engine::parseCommands(int argc, char* argv[])
{
	//Does argv[0] on all platforms is the path to our application?
	for(int i = 1; i < argc; ++i)
	{
		if(SDL_strcasecmp(argv[i], "-compatibility") == 0)
			m_perfMode = false;
		else if(SDL_strcasecmp(argv[i], "-performance") == 0)
			m_perfMode = true;
		else if(SDL_strcasecmp(argv[i], "-force-d3d11") == 0)
			m_engine = CLIENT_ENGINE_DIRECT3D11;
		else if(SDL_strcasecmp(argv[i], "-force-d3d9") == 0)
			m_engine = CLIENT_ENGINE_DIRECT3D;
		else if(SDL_strcasecmp(argv[i], "-force-d3d7") == 0)
			m_engine = CLIENT_ENGINE_DIRECT3D7;
		else if(SDL_strcasecmp(argv[i], "-force-vulkan") == 0)
			m_engine = CLIENT_ENGINE_VULKAN;
		else if(SDL_strcasecmp(argv[i], "-force-opengl") == 0)
			m_engine = CLIENT_ENGINE_OPENGL;
		else if(SDL_strcasecmp(argv[i], "-force-opengles") == 0)
			m_engine = CLIENT_ENGINE_OPENGLES;
		else if(SDL_strcasecmp(argv[i], "-force-opengles2") == 0)
			m_engine = CLIENT_ENGINE_OPENGLES2;
		else if(SDL_strcasecmp(argv[i], "-force-software") == 0)
			m_engine = CLIENT_ENGINE_SOFTWARE;
	}
}

bool Engine::RecreateWindow(bool vulkan, bool opengl)
{
	if(m_window)
	{
		SDL_DestroyWindow(m_window);
		m_window = NULL;
	}

	Uint32 windowflags = (SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);
	if(m_fullscreen)
	{
		windowflags |= SDL_WINDOW_FULLSCREEN;
		m_windowW = m_fullScreenWidth;
		m_windowH = m_fullScreenHeight;
	}
	else if(m_maximized)
	{
		windowflags |= SDL_WINDOW_MAXIMIZED;
		m_windowW = m_windowCachedW;
		m_windowH = m_windowCachedH;
	}

	if(vulkan)
		windowflags |= SDL_WINDOW_VULKAN;
	else if(opengl)
		windowflags |= SDL_WINDOW_OPENGL;

	m_window = SDL_CreateWindow(PRODUCT_NAME, m_windowX, m_windowY, m_windowW, m_windowH, windowflags);
	if(!m_window)
	{
		if(!vulkan && !opengl)
		{
			SDL_snprintf(g_buffer, sizeof(g_buffer), "Couldn't recreate SDL Window: %s", SDL_GetError());
			UTIL_MessageBox(true, g_buffer);
			exit(-1);
		}
		return false;
	}

	if(m_fullscreen)
	{
		attachFullScreenInfo();

		SDL_DisplayMode displayMode;
		SDL_GetWindowDisplayMode(m_window, &displayMode);
		m_windowW = displayMode.w;
		m_windowH = displayMode.h;
	}
	else
	{
		SDL_GetWindowSize(m_window, &m_windowW, &m_windowH);
		SDL_SetWindowMinimumSize(m_window, GAME_MINIMUM_WIDTH, GAME_MINIMUM_HEIGHT);
	}
	m_windowId = SDL_GetWindowID(m_window);

	//createwindow don't generate resize event so send one ourselves
	UTIL_ResizeEvent(m_windowId, m_windowW, m_windowH);
	return true;
}

bool Engine::init()
{
	if(m_surface)
		delete m_surface;

	if(m_engine == CLIENT_ENGINE_SOFTWARE)
		m_surface = new SurfaceSoftware();
	#if defined(SDL_VIDEO_VULKAN)
	else if(m_engine == CLIENT_ENGINE_VULKAN)
	{
		if(m_perfMode)
			m_surface = new SurfaceVulkanPerf();
		else
			m_surface = new SurfaceVulkanComp();
	}
	#endif
	#if defined(SDL_VIDEO_RENDER_OGL)
	else if(m_engine == CLIENT_ENGINE_OPENGL)
	{
		if(m_perfMode)
			m_surface = new SurfaceOpenglPerf();
		else
			m_surface = new SurfaceOpenglComp();
	}
	#endif
	#if defined(SDL_VIDEO_RENDER_OGL_ES)
	else if(m_engine == CLIENT_ENGINE_OPENGLES)
	{
		if(m_perfMode)
			m_surface = new SurfaceOpenglESPerf();
		else
			m_surface = new SurfaceOpenglESComp();
	}
	#endif
	#if defined(SDL_VIDEO_RENDER_OGL_ES2)
	else if(m_engine == CLIENT_ENGINE_OPENGLES2)
	{
		if(m_perfMode)
			m_surface = new SurfaceOpenglES2Perf();
		else
			m_surface = new SurfaceOpenglES2Comp();
	}
	#endif
	#if defined(SDL_VIDEO_RENDER_D3D)
	else if(m_engine == CLIENT_ENGINE_DIRECT3D)
	{
		if(m_perfMode)
			m_surface = new SurfaceDirect3D9Perf();
		else
			m_surface = new SurfaceDirect3D9Comp();
	}
	#endif
	#if defined(SDL_VIDEO_RENDER_D3D11)
	else if(m_engine == CLIENT_ENGINE_DIRECT3D11)
	{
		if(m_perfMode)
			m_surface = new SurfaceDirect3D11Perf();
		else
			m_surface = new SurfaceDirect3D11Comp();
	}
	#endif
	#if defined(SDL_VIDEO_RENDER_DDRAW)
	else if(m_engine == CLIENT_ENGINE_DIRECT3D7)
	{
		if(m_perfMode)
			m_surface = new SurfaceDirectDrawPerf();
		else
			m_surface = new SurfaceDirectDrawComp();
	}
	#endif
	else
	{
		m_engine = m_engines.back();
		return false;
	}
	
	if(!m_surface || !m_surface->isSupported())
	{
		for(std::vector<Uint8>::iterator it = m_engines.begin(), end = m_engines.end(); it != end; ++it)
		{
			if((*it) == m_engine)
			{
				m_engines.erase(it);
				break;
			}
		}
		if(m_engines.empty())
		{
			UTIL_MessageBox(true, "The engine couldn't find any sufficient graphic engine.");
			g_running = false;
			return false;
		}
		m_engine = m_engines.back();
		return false;
	}
	m_surface->doResize(m_windowW, m_windowH);
	m_surface->init();
	return true;
}

void Engine::initFont(Uint8 font, Sint32 width, Sint32 height, Sint32 hchars, Sint32 vchars, Sint32 maxchw, Sint32 maxchh)
{
	Uint16 picture = 0;
	switch(font)
	{
		case CLIENT_FONT_NONOUTLINED:
			picture = 2;
			break;
		case CLIENT_FONT_OUTLINED:
			picture = 4;
			break;
		case CLIENT_FONT_SMALL:
			picture = 5;
			break;
	}

	if(!picture)
		return;//TODO; assertion

	Sint32 w, h;
	unsigned char* pixels = LoadPicture(picture, true, w, h);
	if(!pixels || width != w || height != h)
		return;//TODO; assertion

	Uint32 protectionSize = (w*h*4)-4;
	m_charPicture[font] = picture;
	for(Sint32 i = 1; i < 32; ++i)
	{
		m_charx[font][i] = 0;m_chary[font][i] = 0;
		m_charw[font][i] = 0;m_charh[font][i] = 0;
	}

	for(Sint32 j = 0; j < vchars; ++j)
	{
		for(Sint32 k = 0; k < hchars; ++k)
		{
			Sint32 ch = 32 + (j * hchars) + k;
			if(ch > 255)
				continue;

			m_charx[font][ch] = k*maxchw;
			m_chary[font][ch] = j*maxchh;

			Sint32 chWidth = 0, chHeight = 0;
			for(Sint32 xPos = m_charx[font][ch]; xPos < m_charx[font][ch]+maxchw; ++xPos)
			{
				for(Sint32 yPos = m_chary[font][ch]; yPos < m_chary[font][ch]+maxchh; ++yPos)
				{
					Uint32 offset = (yPos*width+xPos)*4;
					if(offset <= protectionSize && pixels[offset+3] == 0xFF)
					{
						if(chWidth < xPos-m_charx[font][ch])
							chWidth = xPos-m_charx[font][ch];
						if(chHeight < yPos-m_chary[font][ch])
							chHeight = yPos-m_chary[font][ch];
					}
				}
			}

			m_charw[font][ch] = chWidth+1;
			m_charh[font][ch] = chHeight+1;
		}
	}

	SDL_free(pixels);
	m_charw[font][0] = maxchw;
	m_charh[font][0] = maxchh;
	switch(font)
	{
		case CLIENT_FONT_NONOUTLINED:
			m_charx[font][0] = 1;m_chary[font][0] = 0;
			m_charw[font][32] = m_charw[font][160] = 2;
			break;

		case CLIENT_FONT_OUTLINED:
			m_charx[font][0] = -1;m_chary[font][0] = 1;
			m_charw[font][32] = m_charw[font][160] = 4;
			break;
			
		case CLIENT_FONT_SMALL:
			m_charx[font][0] = m_chary[font][0] = 0;
			m_charw[font][32] = m_charw[font][160] = 2;
			break;
	}
}

Uint32 Engine::calculateFontWidth(Uint8 fontId, const std::string& text, size_t pos, size_t len)
{
	if(fontId >= CLIENT_FONT_LAST)
		return 0;

	len += pos;
	if(len > text.length())
		len = text.length();

	Sint32 xSpace = m_charx[fontId][0];
	Sint32 ySpace = m_chary[fontId][0];
	Uint32 calculatedWidth = 0;
	Uint8 character;
	for(size_t i = pos; i < len; ++i)
	{
		character = SDL_static_cast(Uint8, text[i]);
		switch(character)
		{
			case '\n':
			case '\r'://return here?
				return calculatedWidth+ySpace;
			case 0x0E://Special case - change rendering color
			{
				if(i+4 < len)//First check if we have the color bytes
					i += 3;
			}
			break;
			case 0x0F://Special case - change back standard color
				break;
			default:
				calculatedWidth += m_charw[fontId][character]+xSpace;
				break;
		}
	}
	return calculatedWidth+ySpace;
}

Uint32 Engine::calculateFontWidth(Uint8 fontId, const std::string& text)
{
	if(fontId >= CLIENT_FONT_LAST)
		return 0;

	Sint32 xSpace = m_charx[fontId][0];
	Sint32 ySpace = m_chary[fontId][0];
	Uint32 calculatedWidth = 0;
	Uint8 character;
	for(size_t i = 0, len = text.length(); i < len; ++i)
	{
		character = SDL_static_cast(Uint8, text[i]);
		switch(character)
		{
			case '\n':
			case '\r':
				return calculatedWidth+ySpace;
			case 0x0E://Special case - change rendering color
			{
				if(i+4 < len)//First check if we have the color bytes
					i += 3;
			}
			break;
			case 0x0F://Special case - change back standard color
				break;
			default:
				calculatedWidth += m_charw[fontId][character]+xSpace;
				break;
		}
	}
	return calculatedWidth+ySpace;
}

void Engine::exitGame()
{
	if(m_ingame)
		UTIL_exitWarning();
	else
	{
		g_inited = false;
		g_running = false;
	}
}

void Engine::checkReleaseQueue()
{
	for(std::vector<GUI_Window*>::iterator it = m_toReleaseWindows.begin(), end = m_toReleaseWindows.end(); it != end; ++it)
		delete (*it);

	m_toReleaseWindows.clear();
	for(std::vector<GUI_Panel*>::iterator it = m_toReleasePanels.begin(), end = m_toReleasePanels.end(); it != end; ++it)
		delete (*it);

	m_toReleasePanels.clear();
}

void Engine::updateThink()
{
	//Event that get called by every 5 seconds
	checkReleaseQueue();
	g_game.updatePlayerExperienceTable();
	g_game.sendPing();
}

void Engine::clearWindows()
{
	if(m_actWindow)
	{
		m_toReleaseWindows.push_back(m_actWindow);
		m_actWindow = NULL;
	}
	for(std::vector<GUI_Window*>::iterator it = m_windows.begin(), end = m_windows.end(); it != end; ++it)
		m_toReleaseWindows.push_back((*it));

	m_windows.clear();
}

void Engine::addWindow(GUI_Window* pWindow, bool topMost)
{
	pWindow->onReshape(m_windowW, m_windowH);
	if(topMost)
	{
		if(m_actWindow)
			m_windows.insert(m_windows.begin(), m_actWindow);
		m_actWindow = pWindow;
	}
	else if(!m_actWindow)
		m_actWindow = pWindow;
	else
		m_windows.push_back(pWindow);
}

void Engine::removeWindow(GUI_Window* pWindow)
{
	m_toReleaseWindows.push_back(pWindow);
	if(pWindow == m_actWindow)
	{
		m_actWindow = NULL;
		if(!m_windows.empty())
		{
			m_actWindow = m_windows.front();
			m_windows.erase(m_windows.begin());
		}
		return;
	}

	for(std::vector<GUI_Window*>::iterator it = m_windows.begin(), end = m_windows.end(); it != end; ++it)
	{
		if((*it) == pWindow)
		{
			m_windows.erase(it);
			return;
		}
	}
}

GUI_Window* Engine::getWindow(Uint32 internalID)
{
	if(m_actWindow && m_actWindow->getInternalID() == internalID)
		return m_actWindow;
	for(std::vector<GUI_Window*>::iterator it = m_windows.begin(), end = m_windows.end(); it != end; ++it)
	{
		if((*it)->getInternalID() == internalID)
			return (*it);
	}
	return NULL;
}

void Engine::onKeyDown(SDL_Event event)
{
	if(m_contextMenu)
		return;

	if(m_description)
	{
		delete m_description;
		m_description = NULL;
	}

	HotkeyUsage* hotkey = getHotkey(event.key.keysym.sym, event.key.keysym.mod);
	if(hotkey)
	{
		switch(hotkey->hotkey)
		{
			case CLIENT_HOTKEY_DIALOGS_OPENTERMINAL: return;
			case CLIENT_HOTKEY_UI_TOGGLEFPSINDICATOR: return;
			case CLIENT_HOTKEY_UI_TOGGLEFULLSCREEN: return;
		}
	}
	
	if(m_showLogger)
	{
		g_logger.onKeyDown(event);
		return;
	}

	if(m_actWindow)
	{
		m_actWindow->onKeyDown(event);
		return;
	}

	if(m_ingame)
	{
		if(hotkey)
		{
			switch(hotkey->hotkey)
			{
				case CLIENT_HOTKEY_MOVEMENT_GOEAST: g_game.checkMovement(DIRECTION_EAST); break;
				case CLIENT_HOTKEY_MOVEMENT_GONORTH: g_game.checkMovement(DIRECTION_NORTH); break;
				case CLIENT_HOTKEY_MOVEMENT_GOWEST: g_game.checkMovement(DIRECTION_WEST); break;
				case CLIENT_HOTKEY_MOVEMENT_GOSOUTH: g_game.checkMovement(DIRECTION_SOUTH); break;
				case CLIENT_HOTKEY_MOVEMENT_GONORTHWEST: g_game.checkMovement(DIRECTION_NORTHWEST); break;
				case CLIENT_HOTKEY_MOVEMENT_GONORTHEAST: g_game.checkMovement(DIRECTION_NORTHEAST); break;
				case CLIENT_HOTKEY_MOVEMENT_GOSOUTHWEST: g_game.checkMovement(DIRECTION_SOUTHWEST); break;
				case CLIENT_HOTKEY_MOVEMENT_GOSOUTHEAST: g_game.checkMovement(DIRECTION_SOUTHEAST); break;
				case CLIENT_HOTKEY_MOVEMENT_TURNEAST: g_game.sendTurn(DIRECTION_EAST); break;
				case CLIENT_HOTKEY_MOVEMENT_TURNNORTH: g_game.sendTurn(DIRECTION_NORTH); break;
				case CLIENT_HOTKEY_MOVEMENT_TURNWEST: g_game.sendTurn(DIRECTION_WEST); break;
				case CLIENT_HOTKEY_MOVEMENT_TURNSOUTH: g_game.sendTurn(DIRECTION_SOUTH); break;
				case CLIENT_HOTKEY_CHAT_NEXTCHANNEL: g_game.switchToNextChannel(); break;
				case CLIENT_HOTKEY_CHAT_PREVIOUSCHANNEL: g_game.switchToPreviousChannel(); break;
				case CLIENT_HOTKEY_MINIMAP_CENTER: g_game.minimapCenter(); break;
				case CLIENT_HOTKEY_MINIMAP_FLOORDOWN: g_game.minimapFloorDown(); break;
				case CLIENT_HOTKEY_MINIMAP_FLOORUP: g_game.minimapFloorUp(); break;
				case CLIENT_HOTKEY_MINIMAP_SCROLLEAST: g_game.minimapScrollEast(); break;
				case CLIENT_HOTKEY_MINIMAP_SCROLLNORTH: g_game.minimapScrollNorth(); break;
				case CLIENT_HOTKEY_MINIMAP_SCROLLSOUTH: g_game.minimapScrollSouth(); break;
				case CLIENT_HOTKEY_MINIMAP_SCROLLWEST: g_game.minimapScrollWest(); break;
				case CLIENT_HOTKEY_MINIMAP_ZOOMIN: g_game.minimapZoomIn(); break;
				case CLIENT_HOTKEY_MINIMAP_ZOOMOUT: g_game.minimapZoomOut(); break;
				case CLIENT_HOTKEY_MISC_NEXTPRESET:
				{
					//Switch to next preset
				}
				break;
				case CLIENT_HOTKEY_MISC_PREVIOUSPRESET:
				{
					//Switch to previous preset
				}
				break;
				case CLIENT_HOTKEY_ACTION:
				{
					//Implement actions
				}
				break;
			}
			return;
		}

		g_chat.onKeyDown(event);
	}
}

void Engine::onKeyUp(SDL_Event event)
{
	if(m_contextMenu)
		return;

	if(m_description)
	{
		delete m_description;
		m_description = NULL;
	}

	HotkeyUsage* hotkey = getHotkey(event.key.keysym.sym, event.key.keysym.mod);
	if(hotkey)
	{
		switch(hotkey->hotkey)
		{
			case CLIENT_HOTKEY_DIALOGS_OPENTERMINAL:
				m_showLogger = !m_showLogger;
				return;
			case CLIENT_HOTKEY_UI_TOGGLEFPSINDICATOR:
				m_showPerformance = !m_showPerformance;
				return;
			case CLIENT_HOTKEY_UI_TOGGLEFULLSCREEN:
			{
				m_windowW = m_windowCachedW;
				m_windowH = m_windowCachedH;
				m_fullscreen = !m_fullscreen;
				g_inited = false;
			}
			return;
		}
	}

	if(m_showLogger)
	{
		g_logger.onKeyUp(event);
		return;
	}

	if(m_actWindow)
	{
		m_actWindow->onKeyUp(event);
		return;
	}

	if(m_ingame)
	{
		if(hotkey)
		{
			switch(hotkey->hotkey)
			{
				case CLIENT_HOTKEY_MOVEMENT_GOEAST:
				case CLIENT_HOTKEY_MOVEMENT_GONORTH:
				case CLIENT_HOTKEY_MOVEMENT_GOWEST:
				case CLIENT_HOTKEY_MOVEMENT_GOSOUTH:
				case CLIENT_HOTKEY_MOVEMENT_GONORTHWEST:
				case CLIENT_HOTKEY_MOVEMENT_GONORTHEAST:
				case CLIENT_HOTKEY_MOVEMENT_GOSOUTHWEST:
				case CLIENT_HOTKEY_MOVEMENT_GOSOUTHEAST:
					g_game.releaseMovement();
					break;
				case CLIENT_HOTKEY_MOVEMENT_MOUNT:
				{
					//Send mount
				}
				break;
				case CLIENT_HOTKEY_MOVEMENT_STOPACTIONS:
				{
					//Stop actions
				}
				break;
				case CLIENT_HOTKEY_DIALOGS_OPENBUGREPORTS:
				{
					//Open bug reports
				}
				break;
				case CLIENT_HOTKEY_DIALOGS_OPENIGNORELIST:
				{
					//Open ignore list
				}
				break;
				case CLIENT_HOTKEY_DIALOGS_OPENOPTIONS: UTIL_options(); break;
				case CLIENT_HOTKEY_DIALOGS_OPENHOTKEYS: UTIL_hotkeyOptions(); break;
				case CLIENT_HOTKEY_DIALOGS_OPENQUESTLOG: g_game.sendOpenQuestLog(); break;
				case CLIENT_HOTKEY_WINDOWS_OPENVIPWINDOW:
				{
					//Toggle vip window
				}
				break;
				case CLIENT_HOTKEY_WINDOWS_OPENBATTLEWINDOW:
				{
					//Toggle battle window
				}
				break;
				case CLIENT_HOTKEY_WINDOWS_OPENSKILLSWINDOW:
				{
					//Toggle skills window
				}
				break;
				case CLIENT_HOTKEY_CHAT_CLOSECHANNEL: g_game.closeCurrentChannel(); break;
				case CLIENT_HOTKEY_CHAT_OPENCHANNELLIST: g_game.sendRequestChannels(); break;
				case CLIENT_HOTKEY_CHAT_OPENHELPCHANNEL: g_game.openHelpChannel(); break;
				case CLIENT_HOTKEY_CHAT_OPENNPCCHANNEL: g_game.openNPCChannel(); break;
				case CLIENT_HOTKEY_CHAT_DEFAULTCHANNEL: g_game.switchToDefault(); break;
				case CLIENT_HOTKEY_CHAT_TOGGLECHAT:
				{
					//Toggle chat
				}
				break;
				case CLIENT_HOTKEY_UI_TOGGLECREATUREINFO:
				{
					//Toggle creature info
				}
				break;
				case CLIENT_HOTKEY_COMBAT_SETOFFENSIVE:
				{
					//Set to offensive
				}
				break;
				case CLIENT_HOTKEY_COMBAT_SETBALANCED:
				{
					//Set to balanced
				}
				break;
				case CLIENT_HOTKEY_COMBAT_SETDEFENSIVE:
				{
					//Set to defensive
				}
				break;
				case CLIENT_HOTKEY_COMBAT_TOGGLECHASEMODE:
				{
					//Toggle chase mode
				}
				break;
				case CLIENT_HOTKEY_COMBAT_TOGGLESECUREMODE:
				{
					//Toggle secure mode
				}
				break;
				case CLIENT_HOTKEY_PVPMODE_SETDOVE:
				{
					//Set to dove
				}
				break;
				case CLIENT_HOTKEY_PVPMODE_SETREDFIST:
				{
					//Set to red fist
				}
				break;
				case CLIENT_HOTKEY_PVPMODE_SETWHITEHAND:
				{
					//Set to white hand
				}
				break;
				case CLIENT_HOTKEY_PVPMODE_SETYELLOWHAND:
				{
					//Set to yellow hand
				}
				break;
				case CLIENT_HOTKEY_MISC_LENSHELP:
				{
					//Activate lens help
				}
				break;
				case CLIENT_HOTKEY_MISC_CHANGECHARACTER: UTIL_createCharacterList(); break;
				case CLIENT_HOTKEY_MISC_CHANGEOUTFIT:
				{
					//Open outfit list
				}
				break;
				case CLIENT_HOTKEY_MISC_LOGOUT: g_game.sendLogout(); break;
				case CLIENT_HOTKEY_MISC_TAKESCREENSHOT:
				{
					//Screenshot
				}
				break;
			}
			return;
		}

		g_chat.onKeyUp(event);
	}
}

void Engine::onMouseMove(Sint32 x, Sint32 y)
{
	if(m_contextMenu)
	{
		m_contextMenu->onMouseMove(x, y, m_contextMenu->isInsideRect(x, y));
		return;
	}

	if(m_description)
	{
		delete m_description;
		m_description = NULL;
	}

	if(m_showLogger)
	{
		g_logger.onMouseMove(x, y, true);
		return;
	}

	if(m_actWindow)
	{
		m_actWindow->onMouseMove(x, y, m_actWindow->isInsideRect(x, y));
		return;
	}

	if(m_ingame)
	{
		for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
			(*it)->onMouseMove(x, y, (*it)->isInsideRect(x, y));

		g_chat.onMouseMove(m_chatWindowRect, x, y);
		if(m_actionData == CLIENT_ACTION_MOVEITEM || m_actionData == CLIENT_ACTION_USEWITH || m_actionData == CLIENT_ACTION_SEARCHHOTKEY)
			g_actualCursor = CLIENT_CURSOR_CROSSHAIR;
		//else if(m_actionData == CLIENT_ACTION_LENSHELP)
			//g_actualCursor = CLIENT_CURSOR_LENSHELP;
	}
}

void Engine::onLMouseDown(Sint32 x, Sint32 y)
{
	if(m_contextMenu)
		return;

	if(m_description)
	{
		delete m_description;
		m_description = NULL;
	}

	if(m_showLogger)
	{
		g_logger.onLMouseDown(x, y);
		return;
	}

	if(m_actWindow)
	{
		if(m_actWindow->isInsideRect(x, y))
			m_actWindow->onLMouseDown(x, y);
		return;
	}

	if(m_ingame)
	{
		GUI_Panel* gPanel = NULL;
		std::vector<GUI_Panel*>::iterator it = m_panels.begin();
		for(std::vector<GUI_Panel*>::iterator end = m_panels.end(); it != end; ++it)
		{
			if((*it)->isInsideRect(x, y))
			{
				gPanel = (*it);
				break;
			}
		}
		if(gPanel)
		{
			m_panels.erase(it);
			m_panels.push_back(gPanel);
			gPanel->onLMouseDown(x, y);
			return;
		}
		if(m_gameWindowRect.isPointInside(x, y))
		{
			if(m_actionData != CLIENT_ACTION_NONE)
			{
				if(m_classicControl && m_actionData == CLIENT_ACTION_RIGHTMOUSE)
					setAction(CLIENT_ACTION_EXTRAMOUSE);

				return;
			}

			Tile* tile = g_map.findTile(x, y, m_gameWindowRect, m_scaledSize, m_scale, true);
			if(tile)
			{
				Thing* thing = tile->getTopLookThing();
				if(thing)
				{
					Position& position = tile->getPosition();
					setActionData(CLIENT_ACTION_FROMPOSITION, (thing->getItem() ? thing->getItem()->getID() : 0x62), position.x, position.y, position.z, SDL_static_cast(Uint8, tile->getThingStackPos(thing)));
				}
				setAction(CLIENT_ACTION_LEFTMOUSE);
			}
		}
		else
			g_chat.onLMouseDown(m_chatWindowRect, x, y);
	}
	else
	{
		if(g_mainWindow)
		{
			if(g_mainWindow->isInsideRect(x, y))
				g_mainWindow->onLMouseDown(x, y);
		}
	}
}

void Engine::onLMouseUp(Sint32 x, Sint32 y)
{
	if(m_contextMenu)
	{
		m_contextMenu->onLMouseUp(x, y);
		delete m_contextMenu;
		m_contextMenu = NULL;
		return;
	}

	if(m_description)
	{
		delete m_description;
		m_description = NULL;
	}

	if(m_showLogger)
	{
		g_logger.onLMouseUp(x, y);
		return;
	}

	if(m_actWindow)
	{
		m_actWindow->onLMouseUp(x, y);
		return;
	}

	if(m_ingame)
	{
		for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
			(*it)->onLMouseUp(x, y);

		g_chat.onLMouseUp(m_chatWindowRect, x, y);
		if(m_actionData != CLIENT_ACTION_NONE)
		{
			Uint32 mouseState = SDL_GetMouseState(NULL, NULL);
			if(!(mouseState & SDL_BUTTON_RMASK))
			{
				Uint16 keyMods = UTIL_parseModifiers(SDL_static_cast(Uint16, SDL_GetModState()));
				if(m_actionData == CLIENT_ACTION_EXTRAMOUSE || keyMods == KMOD_SHIFT)
				{
					ClientActionData& clientData = m_actionDataStructure[CLIENT_ACTION_FROMPOSITION];
					g_game.sendLookAt(Position(clientData.posX, clientData.posY, clientData.posZ), clientData.itemId, clientData.posStack);
				}

				setAction(CLIENT_ACTION_NONE);
			}
		}
	}
	else
	{
		if(g_mainWindow)
			g_mainWindow->onLMouseUp(x, y);
	}
}

void Engine::onRMouseDown(Sint32 x, Sint32 y)
{
	if(m_contextMenu)
		return;

	if(m_description)
	{
		delete m_description;
		m_description = NULL;
	}

	if(m_showLogger)
	{
		g_logger.onRMouseDown(x, y);
		return;
	}

	if(m_actWindow)
	{
		if(m_actWindow->isInsideRect(x, y))
			m_actWindow->onRMouseDown(x, y);
		return;
	}

	if(m_ingame)
	{
		GUI_Panel* gPanel = NULL;
		std::vector<GUI_Panel*>::iterator it = m_panels.begin();
		for(std::vector<GUI_Panel*>::iterator end = m_panels.end(); it != end; ++it)
		{
			if((*it)->isInsideRect(x, y))
			{
				gPanel = (*it);
				break;
			}
		}
		if(gPanel)
		{
			m_panels.erase(it);
			m_panels.push_back(gPanel);
			gPanel->onRMouseDown(x, y);
			return;
		}
		if(m_gameWindowRect.isPointInside(x, y))
		{
			if(m_actionData != CLIENT_ACTION_NONE)
			{
				if(m_classicControl && m_actionData == CLIENT_ACTION_LEFTMOUSE)
					setAction(CLIENT_ACTION_EXTRAMOUSE);

				return;
			}

			Tile* tile = g_map.findTile(x, y, m_gameWindowRect, m_scaledSize, m_scale, true);
			if(tile)
			{
				Thing* thing = tile->getTopLookThing();
				if(thing)
				{
					Position& position = tile->getPosition();
					setActionData(CLIENT_ACTION_FROMPOSITION, (thing->getItem() ? thing->getItem()->getID() : 0x62), position.x, position.y, position.z, SDL_static_cast(Uint8, tile->getThingStackPos(thing)));
				}
				setAction(CLIENT_ACTION_RIGHTMOUSE);
			}
		}
		else
			g_chat.onRMouseDown(m_chatWindowRect, x, y);
	}
	else
	{
		if(g_mainWindow)
		{
			if(g_mainWindow->isInsideRect(x, y))
				g_mainWindow->onRMouseDown(x, y);
		}
	}
}

void Engine::onRMouseUp(Sint32 x, Sint32 y)
{
	if(m_contextMenu)
	{
		delete m_contextMenu;
		m_contextMenu = NULL;
		return;
	}

	if(m_description)
	{
		delete m_description;
		m_description = NULL;
	}

	if(m_showLogger)
	{
		g_logger.onRMouseUp(x, y);
		return;
	}

	if(m_actWindow)
	{
		m_actWindow->onRMouseUp(x, y);
		return;
	}

	if(m_ingame)
	{
		for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
			(*it)->onRMouseUp(x, y);

		g_chat.onRMouseUp(m_chatWindowRect, x, y);
		if(m_actionData != CLIENT_ACTION_NONE)
		{
			Uint32 mouseState = SDL_GetMouseState(NULL, NULL);
			if(!(mouseState & SDL_BUTTON_LMASK))
			{
				Uint16 keyMods = UTIL_parseModifiers(SDL_static_cast(Uint16, SDL_GetModState()));
				if(m_actionData == CLIENT_ACTION_EXTRAMOUSE || keyMods == KMOD_SHIFT)
				{
					ClientActionData& clientData = m_actionDataStructure[CLIENT_ACTION_FROMPOSITION];
					g_game.sendLookAt(Position(clientData.posX, clientData.posY, clientData.posZ), clientData.itemId, clientData.posStack);
				}

				setAction(CLIENT_ACTION_NONE);
			}
		}
	}
	else
	{
		if(g_mainWindow)
			g_mainWindow->onRMouseUp(x, y);
	}
}

void Engine::onWheel(Sint32 x, Sint32 y, bool wheelUP)
{
	if(m_contextMenu)
		return;

	if(m_description)
	{
		delete m_description;
		m_description = NULL;
	}

	if(m_showLogger)
	{
		g_logger.onWheel(x, y, wheelUP);
		return;
	}

	if(m_actWindow)
	{
		if(m_actWindow->isInsideRect(x, y))
			m_actWindow->onWheel(x, y, wheelUP);
		return;
	}

	if(m_ingame)
	{
		for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
		{
			if((*it)->isInsideRect(x, y))
			{
				(*it)->onWheel(x, y, wheelUP);
				return;
			}
		}
		g_chat.onWheel(m_chatWindowRect, x, y, wheelUP);
	}
	else
	{
		if(g_mainWindow && g_mainWindow->isInsideRect(x, y))
			g_mainWindow->onWheel(x, y, wheelUP);
	}
}

void Engine::onTextInput(const char* textInput)
{
	if(m_showLogger)
	{
		g_logger.onTextInput(textInput);
		return;
	}

	if(m_actWindow)
	{
		m_actWindow->onTextInput(textInput);
		return;
	}

	if(m_ingame)
		g_chat.onTextInput(textInput);
	else
	{
		if(g_mainWindow)
			g_mainWindow->onTextInput(textInput);
	}
}

void Engine::windowResize(Sint32 width, Sint32 height)
{
	m_windowW = width;
	m_windowH = height;
	if(!m_maximized && !m_fullscreen)
	{
		m_windowCachedW = m_windowW;
		m_windowCachedH = m_windowH;
	}
	m_surface->doResize(m_windowW, m_windowH);
	for(std::vector<GUI_Window*>::iterator it = m_windows.begin(), end = m_windows.end(); it != end; ++it)
		(*it)->onReshape(m_windowW, m_windowH);

	if(m_actWindow)
		m_actWindow->onReshape(m_windowW, m_windowH);

	if(g_mainWindow)
		g_mainWindow->onReshape(m_windowW, m_windowH);

	recalculateGameWindow();
}

void Engine::windowMoved(Sint32 x, Sint32 y)
{
	if(!m_maximized && !m_fullscreen)
	{
		m_windowX = x;
		m_windowY = y;
	}
}

void Engine::takeScreenshot(void* data1, void* data2)
{
	//TODO: add other popular extensions like png
	char* fileName = SDL_reinterpret_cast(char*, data1);
	Uint32 flags = SDL_static_cast(Uint32, SDL_reinterpret_cast(size_t, data2));
	if(!fileName || !m_surface)
		return;

	Sint32 screenWidth;
	Sint32 screenHeight;
	bool pixelsBGRA;
	unsigned char* pixels = m_surface->getScreenPixels(screenWidth, screenHeight, pixelsBGRA);
	if(!pixels)
	{
		//We don't have pixels this frame so we can't do much
		SDL_free(fileName);
		return;
	}
	if(flags == SCREENSHOT_FLAG_SAVEASBMP)
	{
		Uint32 Rmask, Gmask, Bmask;
		if(pixelsBGRA)
		{
			#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			Rmask = 0x0000FF00;
			Gmask = 0x00FF0000;
			Bmask = 0xFF000000;
			#else
			Rmask = 0x00FF0000;
			Gmask = 0x0000FF00;
			Bmask = 0x000000FF;
			#endif
		}
		else
		{
			#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			Rmask = 0xFF000000;
			Gmask = 0x00FF0000;
			Bmask = 0x0000FF00;
			#else
			Rmask = 0x000000FF;
			Gmask = 0x0000FF00;
			Bmask = 0x00FF0000;
			#endif
		}
		SDL_Surface* s = SDL_CreateRGBSurfaceFrom(pixels, screenWidth, screenHeight, 32, 4 * screenWidth, Rmask, Gmask, Bmask, 0x00000000);
		SDL_SaveBMP(s, fileName);
		SDL_FreeSurface(s);
	}
	SDL_free(fileName);
	SDL_free(pixels);
}

HotkeyUsage* Engine::getHotkey(SDL_Keycode key, Uint16 mods)
{
	std::map<Uint16, std::map<SDL_Keycode, size_t>>::iterator mit = m_hotkeyFastAccess.find(mods);
	if(mit != m_hotkeyFastAccess.end())
	{
		std::map<SDL_Keycode, size_t>::iterator it = mit->second.find(key);
		if(it != mit->second.end())
			return &m_hotkeys[it->second];
		else
			return NULL;
	}
	return NULL;
}

void Engine::bindHotkey(ClientHotkeyKeys hotKey, SDL_Keycode key, Uint16 mods, ClientHotkeys hotkeyType)
{
	std::map<Uint16, std::map<SDL_Keycode, size_t>>::iterator mit = m_hotkeyFastAccess.find(mods);
	if(mit != m_hotkeyFastAccess.end())
	{
		std::map<SDL_Keycode, size_t>::iterator it = mit->second.find(key);
		if(it != mit->second.end())
			mit->second.erase(it);
	}

	HotkeyUsage newHotkey;
	newHotkey.action.type = CLIENT_HOTKEY_ACTION_NONE;
	newHotkey.keycode = key;
	newHotkey.hotkey = hotkeyType;
	newHotkey.modifers = mods;
	newHotkey.keyid = SDL_static_cast(Uint8, hotKey);
	m_hotkeyFastAccess[mods][key] = m_hotkeys.size();
	m_hotkeys.push_back(newHotkey);
}

void Engine::resetToDefaultHotkeys(bool wasd)
{
	if(wasd)
	{
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_w, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_TURNNORTH);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_a, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_TURNWEST);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_s, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_TURNSOUTH);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_d, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_TURNEAST);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_w, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GONORTH);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_a, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GOWEST);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_s, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GOSOUTH);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_d, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GOEAST);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_RETURN, KMOD_NONE, CLIENT_HOTKEY_CHAT_TOGGLECHAT);
		bindHotkey(CLIENT_HOTKEY_SECOND_KEY, SDLK_KP_ENTER, KMOD_NONE, CLIENT_HOTKEY_CHAT_TOGGLECHAT);
	}
	else
	{
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_UP, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_TURNNORTH);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_LEFT, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_TURNWEST);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_DOWN, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_TURNSOUTH);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_RIGHT, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_TURNEAST);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_UP, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GONORTH);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_LEFT, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GOWEST);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_DOWN, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GOSOUTH);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_RIGHT, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GOEAST);
	}
	bindHotkey(CLIENT_HOTKEY_SECOND_KEY, SDLK_KP_8, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_TURNNORTH);
	bindHotkey(CLIENT_HOTKEY_SECOND_KEY, SDLK_KP_4, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_TURNWEST);
	bindHotkey(CLIENT_HOTKEY_SECOND_KEY, SDLK_KP_2, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_TURNSOUTH);
	bindHotkey(CLIENT_HOTKEY_SECOND_KEY, SDLK_KP_6, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_TURNEAST);
	bindHotkey(CLIENT_HOTKEY_SECOND_KEY, SDLK_KP_8, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GONORTH);
	bindHotkey(CLIENT_HOTKEY_SECOND_KEY, SDLK_KP_4, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GOWEST);
	bindHotkey(CLIENT_HOTKEY_SECOND_KEY, SDLK_KP_2, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GOSOUTH);
	bindHotkey(CLIENT_HOTKEY_SECOND_KEY, SDLK_KP_6, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GOEAST);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_KP_7, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GONORTHWEST);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_KP_9, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GONORTHEAST);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_KP_1, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GOSOUTHWEST);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_KP_3, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GOSOUTHEAST);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_t, (KMOD_CTRL+KMOD_ALT), CLIENT_HOTKEY_DIALOGS_OPENTERMINAL);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_r, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_MOUNT);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_z, KMOD_CTRL, CLIENT_HOTKEY_DIALOGS_OPENBUGREPORTS);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_i, KMOD_CTRL, CLIENT_HOTKEY_DIALOGS_OPENIGNORELIST);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_k, KMOD_CTRL, CLIENT_HOTKEY_DIALOGS_OPENHOTKEYS);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_u, KMOD_CTRL, CLIENT_HOTKEY_DIALOGS_OPENQUESTLOG);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_p, KMOD_CTRL, CLIENT_HOTKEY_WINDOWS_OPENVIPWINDOW);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_b, KMOD_CTRL, CLIENT_HOTKEY_WINDOWS_OPENBATTLEWINDOW);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_s, KMOD_CTRL, CLIENT_HOTKEY_WINDOWS_OPENSKILLSWINDOW);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_e, KMOD_CTRL, CLIENT_HOTKEY_CHAT_CLOSECHANNEL);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_o, KMOD_CTRL, CLIENT_HOTKEY_CHAT_OPENCHANNELLIST);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_t, KMOD_CTRL, CLIENT_HOTKEY_CHAT_OPENHELPCHANNEL);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_n, KMOD_CTRL, CLIENT_HOTKEY_UI_TOGGLECREATUREINFO);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_f, KMOD_CTRL, CLIENT_HOTKEY_UI_TOGGLEFULLSCREEN);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_h, KMOD_CTRL, CLIENT_HOTKEY_MISC_LENSHELP);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_g, KMOD_CTRL, CLIENT_HOTKEY_MISC_CHANGECHARACTER);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_l, KMOD_CTRL, CLIENT_HOTKEY_MISC_LOGOUT);
	bindHotkey(CLIENT_HOTKEY_SECOND_KEY, SDLK_q, KMOD_CTRL, CLIENT_HOTKEY_MISC_LOGOUT);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_TAB, KMOD_SHIFT, CLIENT_HOTKEY_CHAT_PREVIOUSCHANNEL);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_d, KMOD_ALT, CLIENT_HOTKEY_CHAT_DEFAULTCHANNEL);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_PAGEDOWN, KMOD_ALT, CLIENT_HOTKEY_MINIMAP_FLOORDOWN);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_PAGEUP, KMOD_ALT, CLIENT_HOTKEY_MINIMAP_FLOORUP);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_RIGHT, KMOD_ALT, CLIENT_HOTKEY_MINIMAP_SCROLLEAST);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_UP, KMOD_ALT, CLIENT_HOTKEY_MINIMAP_SCROLLNORTH);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_DOWN, KMOD_ALT, CLIENT_HOTKEY_MINIMAP_SCROLLSOUTH);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_LEFT, KMOD_ALT, CLIENT_HOTKEY_MINIMAP_SCROLLWEST);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_END, KMOD_ALT, CLIENT_HOTKEY_MINIMAP_ZOOMIN);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_HOME, KMOD_ALT, CLIENT_HOTKEY_MINIMAP_ZOOMOUT);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_F8, KMOD_ALT, CLIENT_HOTKEY_UI_TOGGLEFPSINDICATOR);
	bindHotkey(CLIENT_HOTKEY_SECOND_KEY, SDLK_RETURN, KMOD_ALT, CLIENT_HOTKEY_UI_TOGGLEFULLSCREEN);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_ESCAPE, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_STOPACTIONS);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_TAB, KMOD_NONE, CLIENT_HOTKEY_CHAT_NEXTCHANNEL);
}

void Engine::recalculateGameWindow()
{
	Sint32 width = m_windowW-176;
	Sint32 height = m_windowH-140;

	Sint32 scaledSize = width/(GAME_MAP_WIDTH-3);
	Sint32 scaledSize2 = height/(GAME_MAP_HEIGHT-3);
	if(scaledSize < scaledSize2)
		m_scaledSize = scaledSize;
	else
		m_scaledSize = scaledSize2;

	m_scale = m_scaledSize*0.03125f;

	Sint32 windowWidth = m_scaledSize*(GAME_MAP_WIDTH-3);
	Sint32 windowHeight = m_scaledSize*(GAME_MAP_HEIGHT-3);
	Sint32 windowX = UTIL_max<Sint32>(width/2 - windowWidth/2, 1);
	Sint32 windowY = UTIL_max<Sint32>(height/2 - windowHeight/2, 1);
	m_gameWindowRect = iRect(windowX, windowY, windowWidth, windowHeight);
	m_chatWindowRect = iRect(0, height, width, 140);
}

void Engine::redraw()
{
	m_surface->beginScene();
	if(m_ingame)
	{
		g_map.render();
		m_surface->drawPictureRepeat(3, 0, 0, 96, 96, 0, 0, m_windowW-176, m_windowH-140);
		m_surface->setClipRect(m_gameWindowRect.x1, m_gameWindowRect.y1, m_gameWindowRect.x2, m_gameWindowRect.y2);
		m_surface->drawGameScene(0, 0, RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT, m_gameWindowRect.x1, m_gameWindowRect.y1, m_gameWindowRect.x2, m_gameWindowRect.y2);
		g_map.renderInformations(m_gameWindowRect.x1, m_gameWindowRect.y1, m_gameWindowRect.x2, m_gameWindowRect.y2, m_scale, m_scaledSize);
		m_surface->disableClipRect();
		for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
			(*it)->render();
		g_chat.render(m_chatWindowRect);
	}
	else
	{
		m_surface->drawBackground(0, 0, 0, 640, 480, 0, 0, m_windowW, m_windowH);
		if(g_mainWindow)
			g_mainWindow->render();
	}

	if(m_showPerformance)
	{
		static Uint64 lastFrameTime = 0;
		Uint64 previousFrameTime = lastFrameTime;
		lastFrameTime = SDL_GetPerformanceCounter();
		double frameTime = SDL_static_cast(double, (lastFrameTime-previousFrameTime)*1000)/SDL_GetPerformanceFrequency();
		double framePerSecond = 1000.0/frameTime;

		Sint32 posX = m_windowW-5;
		if(m_ingame)
			posX -= 176;

		Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%.0f FPS", framePerSecond);
		drawFont(CLIENT_FONT_OUTLINED, posX, 5, std::string(g_buffer, SDL_static_cast(size_t, len)), 255, 255, 255, CLIENT_FONT_ALIGN_RIGHT);
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%.2fms", frameTime);
		drawFont(CLIENT_FONT_OUTLINED, posX, 19, std::string(g_buffer, SDL_static_cast(size_t, len)), 255, 255, 255, CLIENT_FONT_ALIGN_RIGHT);
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "Ping: %u ms", g_ping);
		drawFont(CLIENT_FONT_OUTLINED, posX, 33, std::string(g_buffer, SDL_static_cast(size_t, len)), 255, 255, 255, CLIENT_FONT_ALIGN_RIGHT);
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "Software: %s", getRender()->getSoftware());
		drawFont(CLIENT_FONT_OUTLINED, posX, 47, std::string(g_buffer, SDL_static_cast(size_t, len)), 255, 255, 255, CLIENT_FONT_ALIGN_RIGHT);
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "Hardware: %s", getRender()->getHardware());
		drawFont(CLIENT_FONT_OUTLINED, posX, 61, std::string(g_buffer, SDL_static_cast(size_t, len)), 255, 255, 255, CLIENT_FONT_ALIGN_RIGHT);
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "VRAM: %u MB", getRender()->getVRAM());
		drawFont(CLIENT_FONT_OUTLINED, posX, 75, std::string(g_buffer, SDL_static_cast(size_t, len)), 255, 255, 255, CLIENT_FONT_ALIGN_RIGHT);
	}

	if(m_actWindow)
		m_actWindow->render();

	if(m_showLogger)
		g_logger.render(0, 0, m_windowW, m_windowH);

	if(m_description)
		m_description->render();

	if(m_contextMenu)
		m_contextMenu->render();

	m_surface->endScene();
}

void Engine::drawFont(Uint8 fontId, Sint32 x, Sint32 y, const std::string& text, Uint8 r, Uint8 g, Uint8 b, Sint32 align, size_t pos, size_t len)
{
	if(align != CLIENT_FONT_ALIGN_LEFT)
	{
		drawFont(fontId, x, y, text.substr(pos, len), r, g, b, align);
		return;
	}

	len += pos;
	if(len > text.length())
		len = text.length();

	m_surface->drawFont(m_charPicture[fontId], x, y, text, pos, len, r, g, b, m_charx[fontId], m_chary[fontId], m_charw[fontId], m_charh[fontId]);
}

void Engine::drawFont(Uint8 fontId, Sint32 x, Sint32 y, const std::string& text, Uint8 r, Uint8 g, Uint8 b, Sint32 align)
{
	if(align != CLIENT_FONT_ALIGN_LEFT)
	{
		Sint32 rx = x, ry = y;
		size_t pos = 0, start = 0;
		while((pos = text.find('\n', pos)) != std::string::npos)
		{
			Uint32 calculatedWidth = calculateFontWidth(fontId, text, start, pos);
			switch(align)
			{
				case CLIENT_FONT_ALIGN_RIGHT:
					rx -= calculatedWidth;
					break;
				case CLIENT_FONT_ALIGN_CENTER:
					rx -= calculatedWidth / 2;
					break;
			}
			m_surface->drawFont(m_charPicture[fontId], rx, ry, text, start, pos, r, g, b, m_charx[fontId], m_chary[fontId], m_charw[fontId], m_charh[fontId]);
			++pos;
			start = pos;
			rx = x; ry += m_charh[fontId][0];
		}

		pos = text.length();
		if(start >= pos)
			return;

		Uint32 calculatedWidth = calculateFontWidth(fontId, text, start, pos);
		switch(align)
		{
			case CLIENT_FONT_ALIGN_RIGHT:
				rx -= calculatedWidth;
				break;
			case CLIENT_FONT_ALIGN_CENTER:
				rx -= calculatedWidth/2;
				break;
		}
		m_surface->drawFont(m_charPicture[fontId], rx, ry, text, start, pos, r, g, b, m_charx[fontId], m_chary[fontId], m_charw[fontId], m_charh[fontId]);
		return;
	}
	m_surface->drawFont(m_charPicture[fontId], x, y, text, 0, text.length(), r, g, b, m_charx[fontId], m_chary[fontId], m_charw[fontId], m_charh[fontId]);
}

void Engine::drawItem(ThingType* thing, Sint32 x, Sint32 y, Sint32 scaled, Uint8 xPattern, Uint8 yPattern, Uint8 zPattern)
{
	Uint8 animationFrame;
	if(thing->m_frameGroup[ThingFrameGroup_Default].m_animCount > 1)
	{
		/*static Animation m_animation;
		if(thing->m_frameGroup[ThingFrameGroup_Default].m_animator)
			animationFrame = SDL_static_cast(Uint8, thing->m_frameGroup[ThingFrameGroup_Default].m_animator->getPhase(m_animation));
		else*/
			animationFrame = UTIL_safeMod<Uint8>(SDL_static_cast(Uint8, (g_frameTime / ITEM_TICKS_PER_FRAME)), thing->m_frameGroup[ThingFrameGroup_Default].m_animCount);
	}
	else
		animationFrame = 0;

	Uint8 fx = thing->m_frameGroup[ThingFrameGroup_Default].m_width;
	Uint8 fy = thing->m_frameGroup[ThingFrameGroup_Default].m_height;
	Uint8 fl = thing->m_frameGroup[ThingFrameGroup_Default].m_layers;
	if(fx == 1 && fy == 1)
	{
		for(Uint8 l = 0; l < fl; ++l)
		{
			Uint32 sprite = thing->getSprite(ThingFrameGroup_Default, 0, 0, l, xPattern, yPattern, zPattern, animationFrame);
			if(sprite != 0)
				m_surface->drawSprite(sprite, x, y, scaled, scaled, 0, 0, 32, 32);
		}
	}
	else
	{
		Uint8 frs = thing->m_frameGroup[ThingFrameGroup_Default].m_realSize;
		Uint8 fmax = SDL_static_cast(Uint8, (SDL_static_cast(Uint32, frs)+31)/32);
		fx = (fx > fmax ? fmax : fx);
		fy = (fy > fmax ? fmax : fy);
		Sint32 scale = SDL_static_cast(Sint32, scaled/(frs*0.03125f));
		for(Uint8 l = 0; l < fl; ++l)
		{
			Sint32 posYc = y+scaled-scale;
			for(Uint8 cy = 0; cy < fy; ++cy)
			{
				Sint32 posXc = x+scaled-scale;
				for(Uint8 cx = 0; cx < fx; ++cx)
				{
					Uint32 sprite = thing->getSprite(ThingFrameGroup_Default, cx, cy, l, xPattern, yPattern, zPattern, animationFrame);
					if(sprite != 0)
					{
						Sint32 dx = posXc, dy = posYc, dw = scale, dh = scale;
						Sint32 sx = 0, sy = 0, sw = 32, sh = 32;
						if(dx < x)
						{
							Sint32 diff = x-dx;
							dx += diff;
							dw -= diff;
							sx = SDL_static_cast(Sint32, SDL_static_cast(float, diff)/SDL_static_cast(float, scale)*32.f);
							sw -= sx;
						}
						if(dy < y)
						{
							Sint32 diff = y-dy;
							dy += diff;
							dh -= diff;
							sy = SDL_static_cast(Sint32, SDL_static_cast(float, diff)/SDL_static_cast(float, scale)*32.f);
							sh -= sy;
						}
						m_surface->drawSprite(sprite, dx, dy, dw, dh, sx, sy, sw, sh);
					}
					posXc -= scale;
				}
				posYc -= scale;
			}
		}
	}
}

void Engine::drawOutfit(ThingType* thing, Sint32 x, Sint32 y, Sint32 scaled, Uint8 xPattern, Uint8 yPattern, Uint8 zPattern, Uint32 outfitColor)
{
	Uint8 animationFrame;
	if(thing->m_category != ThingCategory_Creature)
	{
		if(thing->m_category == ThingCategory_Effect)
		{
			Uint8 animCount = UTIL_max<Uint8>(1, (thing->m_frameGroup[ThingFrameGroup_Idle].m_animCount-2));
			animationFrame = UTIL_safeMod<Uint8>(SDL_static_cast(Uint8, (g_frameTime / ITEM_TICKS_PER_FRAME)), animCount)+1;
		}
		else
			animationFrame = UTIL_safeMod<Uint8>(SDL_static_cast(Uint8, (g_frameTime / ITEM_TICKS_PER_FRAME)), thing->m_frameGroup[ThingFrameGroup_Idle].m_animCount);
	}
	else
	{
		if(false || thing->hasFlag(ThingAttribute_AnimateAlways))
		{
			Sint32 ticks = (1000 / thing->m_frameGroup[ThingFrameGroup_Idle].m_animCount);
			animationFrame = UTIL_safeMod<Uint8>(SDL_static_cast(Uint8, (g_frameTime / ticks)), thing->m_frameGroup[ThingFrameGroup_Idle].m_animCount);
		}
		else
			animationFrame = 0;
	}

	Uint8 fx = thing->m_frameGroup[ThingFrameGroup_Idle].m_width;
	Uint8 fy = thing->m_frameGroup[ThingFrameGroup_Idle].m_height;
	Uint8 fl = thing->m_frameGroup[ThingFrameGroup_Idle].m_layers;
	if(fx == 1 && fy == 1)
	{
		if(fl > 1)
		{
			Uint32 sprite = thing->getSprite(ThingFrameGroup_Idle, 0, 0, 0, xPattern, yPattern, zPattern, animationFrame);
			Uint32 spriteMask = thing->getSprite(ThingFrameGroup_Idle, 0, 0, 1, xPattern, yPattern, zPattern, animationFrame);
			if(sprite != 0)
			{
				if(spriteMask != 0)
					m_surface->drawSpriteMask(sprite, spriteMask, x, y, scaled, scaled, 0, 0, 32, 32, outfitColor);
				else
					m_surface->drawSprite(sprite, x, y, scaled, scaled, 0, 0, 32, 32);
			}
		}
		else
		{
			Uint32 sprite = thing->getSprite(ThingFrameGroup_Idle, 0, 0, 0, xPattern, yPattern, zPattern, animationFrame);
			if(sprite != 0)
				m_surface->drawSprite(sprite, x, y, scaled, scaled, 0, 0, 32, 32);
		}
	}
	else
	{
		Uint8 frs = thing->m_frameGroup[ThingFrameGroup_Idle].m_realSize;
		Uint8 fmax = SDL_static_cast(Uint8, (SDL_static_cast(Uint32, frs)+31)/32);
		fx = (fx > fmax ? fmax : fx);
		fy = (fy > fmax ? fmax : fy);
		Sint32 scale = SDL_static_cast(Sint32, scaled/(frs*0.03125f));
		if(fl > 1)
		{
			Sint32 posYc = y+scaled-scale;
			for(Uint8 cy = 0; cy < fy; ++cy)
			{
				Sint32 posXc = x+scaled-scale;
				for(Uint8 cx = 0; cx < fx; ++cx)
				{
					Uint32 sprite = thing->getSprite(ThingFrameGroup_Idle, cx, cy, 0, xPattern, yPattern, zPattern, animationFrame);
					Uint32 spriteMask = thing->getSprite(ThingFrameGroup_Idle, cx, cy, 1, xPattern, yPattern, zPattern, animationFrame);
					if(sprite != 0)
					{
						Sint32 dx = posXc, dy = posYc, dw = scale, dh = scale;
						Sint32 sx = 0, sy = 0, sw = 32, sh = 32;
						if(dx < x)
						{
							Sint32 diff = x-dx;
							dx += diff;
							dw -= diff;
							sx = SDL_static_cast(Sint32, SDL_static_cast(float, diff)/SDL_static_cast(float, scale)*32.f);
							sw -= sx;
						}
						if(dy < y)
						{
							Sint32 diff = y-dy;
							dy += diff;
							dh -= diff;
							sy = SDL_static_cast(Sint32, SDL_static_cast(float, diff)/SDL_static_cast(float, scale)*32.f);
							sh -= sy;
						}
						if(spriteMask != 0)
							m_surface->drawSpriteMask(sprite, spriteMask, dx, dy, dw, dh, sx, sy, sw, sh, outfitColor);
						else
							m_surface->drawSprite(sprite, dx, dy, dw, dh, sx, sy, sw, sh);
					}
					posXc -= scale;
				}
				posYc -= scale;
			}
		}
		else
		{
			Sint32 posYc = y+scaled-scale;
			for(Uint8 cy = 0; cy < fy; ++cy)
			{
				Sint32 posXc = x+scaled-scale;
				for(Uint8 cx = 0; cx < fx; ++cx)
				{
					Uint32 sprite = thing->getSprite(ThingFrameGroup_Idle, cx, cy, 0, xPattern, yPattern, zPattern, animationFrame);
					if(sprite != 0)
					{
						Sint32 dx = posXc, dy = posYc, dw = scale, dh = scale;
						Sint32 sx = 0, sy = 0, sw = 32, sh = 32;
						if(dx < x)
						{
							Sint32 diff = x-dx;
							dx += diff;
							dw -= diff;
							sx = SDL_static_cast(Sint32, SDL_static_cast(float, diff)/SDL_static_cast(float, scale)*32.f);
							sw -= sx;
						}
						if(dy < y)
						{
							Sint32 diff = y-dy;
							dy += diff;
							dh -= diff;
							sy = SDL_static_cast(Sint32, SDL_static_cast(float, diff)/SDL_static_cast(float, scale)*32.f);
							sh -= sy;
						}
						m_surface->drawSprite(sprite, dx, dy, dw, dh, sx, sy, sw, sh);
					}
					posXc -= scale;
				}
				posYc -= scale;
			}
		}
	}
}

void Engine::drawEffect(ThingType* thing, Sint32 x, Sint32 y, Sint32 scaled, Uint8 xPattern, Uint8 yPattern, Uint8 zPattern)
{
	Uint8 animationFrame;
	if(thing->m_frameGroup[ThingFrameGroup_Default].m_animCount > 1)
		animationFrame = UTIL_safeMod<Uint8>(SDL_static_cast(Uint8, (g_frameTime / EFFECT_TICKS_PER_FRAME)), thing->m_frameGroup[ThingFrameGroup_Default].m_animCount);
	else
		animationFrame = 0;

	Uint8 fx = thing->m_frameGroup[ThingFrameGroup_Default].m_width;
	Uint8 fy = thing->m_frameGroup[ThingFrameGroup_Default].m_height;
	if(fx == 1 && fy == 1)
	{
		Uint32 sprite = thing->getSprite(ThingFrameGroup_Default, 0, 0, 0, xPattern, yPattern, zPattern, animationFrame);
		if(sprite != 0)
			m_surface->drawSprite(sprite, x, y, scaled, scaled, 0, 0, 32, 32);
	}
	else
	{
		Uint8 frs = thing->m_frameGroup[ThingFrameGroup_Default].m_realSize;
		Uint8 fmax = SDL_static_cast(Uint8, (SDL_static_cast(Uint32, frs)+31)/32);
		fx = (fx > fmax ? fmax : fx);
		fy = (fy > fmax ? fmax : fy);
		Sint32 scale = SDL_static_cast(Sint32, scaled/(frs*0.03125f));
		Sint32 posYc = y+scaled-scale;
		for(Uint8 cy = 0; cy < fy; ++cy)
		{
			Sint32 posXc = x+scaled-scale;
			for(Uint8 cx = 0; cx < fx; ++cx)
			{
				Uint32 sprite = thing->getSprite(ThingFrameGroup_Default, cx, cy, 0, xPattern, yPattern, zPattern, animationFrame);
				if(sprite != 0)
				{
					Sint32 dx = posXc, dy = posYc, dw = scale, dh = scale;
					Sint32 sx = 0, sy = 0, sw = 32, sh = 32;
					if(dx < x)
					{
						Sint32 diff = x-dx;
						dx += diff;
						dw -= diff;
						sx = SDL_static_cast(Sint32, SDL_static_cast(float, diff)/SDL_static_cast(float, scale)*32.f);
						sw -= sx;
					}
					if(dy < y)
					{
						Sint32 diff = y-dy;
						dy += diff;
						dh -= diff;
						sy = SDL_static_cast(Sint32, SDL_static_cast(float, diff)/SDL_static_cast(float, scale)*32.f);
						sh -= sy;
					}
					m_surface->drawSprite(sprite, dx, dy, dw, dh, sx, sy, sw, sh);
				}
				posXc -= scale;
			}
			posYc -= scale;
		}
	}
}

void Engine::drawDistanceEffect(ThingType* thing, Sint32 x, Sint32 y, Sint32 scaled, Uint8 xPattern, Uint8 yPattern, Uint8 zPattern)
{
	Uint8 fx = thing->m_frameGroup[ThingFrameGroup_Default].m_width;
	Uint8 fy = thing->m_frameGroup[ThingFrameGroup_Default].m_height;
	if(fx == 1 && fy == 1)
	{
		Uint32 sprite = thing->getSprite(ThingFrameGroup_Default, 0, 0, 0, xPattern, yPattern, zPattern, 0);
		if(sprite != 0)
			m_surface->drawSprite(sprite, x, y, scaled, scaled, 0, 0, 32, 32);
	}
	else
	{
		Uint8 frs = thing->m_frameGroup[ThingFrameGroup_Default].m_realSize;
		Uint8 fmax = SDL_static_cast(Uint8, (SDL_static_cast(Uint32, frs)+31)/32);
		fx = (fx > fmax ? fmax : fx);
		fy = (fy > fmax ? fmax : fy);
		Sint32 scale = SDL_static_cast(Sint32, scaled/(frs*0.03125f));
		Sint32 posYc = y+scaled-scale;
		for(Uint8 cy = 0; cy < fy; ++cy)
		{
			Sint32 posXc = x+scaled-scale;
			for(Uint8 cx = 0; cx < fx; ++cx)
			{
				Uint32 sprite = thing->getSprite(ThingFrameGroup_Default, cx, cy, 0, xPattern, yPattern, zPattern, 0);
				if(sprite != 0)
				{
					Sint32 dx = posXc, dy = posYc, dw = scale, dh = scale;
					Sint32 sx = 0, sy = 0, sw = 32, sh = 32;
					if(dx < x)
					{
						Sint32 diff = x-dx;
						dx += diff;
						dw -= diff;
						sx = SDL_static_cast(Sint32, SDL_static_cast(float, diff)/SDL_static_cast(float, scale)*32.f);
						sw -= sx;
					}
					if(dy < y)
					{
						Sint32 diff = y-dy;
						dy += diff;
						dh -= diff;
						sy = SDL_static_cast(Sint32, SDL_static_cast(float, diff)/SDL_static_cast(float, scale)*32.f);
						sh -= sy;
					}
					m_surface->drawSprite(sprite, dx, dy, dw, dh, sx, sy, sw, sh);
				}
				posXc -= scale;
			}
			posYc -= scale;
		}
	}
}

unsigned char* Engine::LoadSprite(Uint32 spriteId, bool bgra)
{
	if(spriteId == 0 || spriteId > g_spriteCounts)
		return NULL;

	if(bgra)
		return g_spriteManager.LoadSprite_BGRA(spriteId);
	return g_spriteManager.LoadSprite_RGBA(spriteId);
}

unsigned char* Engine::LoadSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Uint32 outfitColor, bool bgra)
{
	unsigned char* pixels = LoadSprite(spriteId, bgra);
	if(!pixels)
		return NULL;

	unsigned char* tempPixels = LoadSprite(maskSpriteId, bgra);
	if(!tempPixels)
	{
		SDL_free(pixels);
		return NULL;
	}

	float hR, hG, hB, bR, bG, bB, lR, lG, lB, fR, fG, fB;
	getOutfitColorFloat(SDL_static_cast(Uint8, outfitColor), hR, hG, hB);
	getOutfitColorFloat(SDL_static_cast(Uint8, outfitColor >> 8), bR, bG, bB);
	getOutfitColorFloat(SDL_static_cast(Uint8, outfitColor >> 16), lR, lG, lB);
	getOutfitColorFloat(SDL_static_cast(Uint8, outfitColor >> 24), fR, fG, fB);
	for(Uint16 i = 0; i <= 4092; i += 4)
	{
		if(tempPixels[i+3] == SDL_ALPHA_OPAQUE)
		{
			Uint32 U32pixel = *SDL_reinterpret_cast(Uint32*, &tempPixels[i]);
			#if SDL_BYTEORDER == SDL_LIL_ENDIAN //Head
			if(U32pixel == 0xFFFFFF00)
			#else
			if(U32pixel == 0x00FFFFFF)
			#endif
			{
				pixels[i+2] = SDL_static_cast(Uint8, pixels[i+2]*hR);
				pixels[i+1] = SDL_static_cast(Uint8, pixels[i+1]*hG);
				pixels[i] = SDL_static_cast(Uint8, pixels[i]*hB);
			}
			#if SDL_BYTEORDER == SDL_LIL_ENDIAN //Body
			else if(U32pixel == 0xFFFF0000)
			#else
			else if(U32pixel == 0x0000FFFF)
			#endif
			{
				pixels[i+2] = SDL_static_cast(Uint8, pixels[i+2]*bR);
				pixels[i+1] = SDL_static_cast(Uint8, pixels[i+1]*bG);
				pixels[i] = SDL_static_cast(Uint8, pixels[i]*bB);
			}
			#if SDL_BYTEORDER == SDL_LIL_ENDIAN //Legs
			else if(U32pixel == 0xFF00FF00)
			#else
			else if(U32pixel == 0x00FF00FF)
			#endif
			{
				pixels[i+2] = SDL_static_cast(Uint8, pixels[i+2]*lR);
				pixels[i+1] = SDL_static_cast(Uint8, pixels[i+1]*lG);
				pixels[i] = SDL_static_cast(Uint8, pixels[i]*lB);
			}
			#if SDL_BYTEORDER == SDL_LIL_ENDIAN //Feet
			else if(U32pixel == 0xFF0000FF)
			#else
			else if(U32pixel == 0xFF0000FF)
			#endif
			{
				pixels[i+2] = SDL_static_cast(Uint8, pixels[i+2]*fR);
				pixels[i+1] = SDL_static_cast(Uint8, pixels[i+1]*fG);
				pixels[i] = SDL_static_cast(Uint8, pixels[i]*fB);
			}
		}
	}
	SDL_free(tempPixels);
	return pixels;
}

unsigned char* Engine::LoadPicture(Uint16 pictureId, bool bgra, Sint32& width, Sint32& height)
{
	if(pictureId >= g_pictureCounts)
		return NULL;

	SDL_RWops* pictures = SDL_RWFromFile(g_picPath.c_str(), "rb");
	if(!pictures)
		return NULL;

	SDL_RWseek(pictures, 6, RW_SEEK_SET);
	for(Uint16 i = 0; i <= pictureId; ++i)
	{
		Uint8 w = SDL_ReadU8(pictures);
		Uint8 h = SDL_ReadU8(pictures);
		SDL_RWseek(pictures, 3, RW_SEEK_CUR);//ignore colorkey
		if(i == pictureId)
		{
			width = SDL_static_cast(Sint32, w*32);
			height = SDL_static_cast(Sint32, h*32);

			Uint32 protectionSize = width*height*4;
			unsigned char* pixels = SDL_reinterpret_cast(unsigned char*, SDL_malloc(protectionSize));
			if(!pixels)
			{
				SDL_RWclose(pictures);
				return NULL;
			}

			Uint32 chunkLoc, oldLoc;
			protectionSize -= 4;
			for(Uint8 y = 0; y < h; ++y)
			{
				for(Uint8 x = 0; x < w; ++x)
				{
					Uint16 pixelSize, chunkSize;
					Uint16 readData = 0, writeData = 0;
					bool state = false;

					chunkLoc = SDL_ReadLE32(pictures);
					oldLoc = SDL_static_cast(Uint32, SDL_RWtell(pictures));
					SDL_RWseek(pictures, chunkLoc, RW_SEEK_SET);

					pixelSize = SDL_ReadLE16(pictures);
					while(readData < pixelSize)
					{
						chunkSize = SDL_ReadLE16(pictures);
						readData += 2;
						for(Uint16 j = 0; j < chunkSize; ++j)
						{
							Uint16 xPos = x*32+(writeData+j) % 32;
							Uint16 yPos = y*32+(writeData+j) / 32;
							Uint32 offset = (yPos*width+xPos)*4;
							if(offset <= protectionSize)
							{
								if(state)
								{
									if(bgra)
									{
										pixels[offset+2] = SDL_ReadU8(pictures);
										pixels[offset+1] = SDL_ReadU8(pictures);
										pixels[offset] = SDL_ReadU8(pictures);
									}
									else
									{
										pixels[offset] = SDL_ReadU8(pictures);
										pixels[offset+1] = SDL_ReadU8(pictures);
										pixels[offset+2] = SDL_ReadU8(pictures);
									}
									pixels[offset+3] = SDL_ALPHA_OPAQUE;
									readData += 3;
								}
								else
									pixels[offset+3] = SDL_ALPHA_TRANSPARENT;
							}
						}
						writeData += chunkSize;
						state = !state;
					}
					SDL_RWseek(pictures, oldLoc, RW_SEEK_SET);
				}
			}
			SDL_RWclose(pictures);
			return pixels;
		}
		else
			SDL_RWseek(pictures, 4*h*w, RW_SEEK_CUR);
	}
	SDL_RWclose(pictures);
	return NULL;
}

void Engine::issueNewConnection(bool protocolGame)
{
	releaseConnection();

	Protocol* protocol;
	if(protocolGame)
	{
		protocol = new ProtocolGame();

		CharacterDetail& character = m_characters[SDL_static_cast(size_t, m_characterSelectId)];
		#if CLIENT_OVVERIDE_VERSION == 0
		g_connection = new Connection(character.worldIp.c_str(), character.worldPort, m_clientProxy.c_str(), m_clientProxyAuth.c_str(), protocol);
		#else
		g_connection = new Connection(character.worldIp.c_str(), character.worldPort, "", "", protocol);
		#endif
	}
	else
	{
		protocol = new ProtocolLogin();
		#if CLIENT_OVVERIDE_VERSION == 0
		g_connection = new Connection(m_clientHost.c_str(), SDL_static_cast(Uint16, SDL_strtoul(m_clientPort.c_str(), NULL, 10)), m_clientProxy.c_str(), m_clientProxyAuth.c_str(), protocol);
		#else
		g_connection = new Connection(CLIENT_OVVERIDE_LOGIN_HOST, CLIENT_OVVERIDE_LOGIN_PORT, "", "", protocol);
		#endif
	}
}

void Engine::releaseConnection()
{
	if(g_connection)
	{
		delete g_connection;
		g_connection = NULL;
	}
}

void Engine::setActionData(ClientActions data, Uint16 itemId, Uint16 posX, Uint16 posY, Uint8 posZ, Uint8 posStack)
{
	ClientActionData& actionData = m_actionDataStructure[data];
	actionData.itemId = itemId;
	actionData.posX = posX;
	actionData.posY = posY;
	actionData.posZ = posZ;
	actionData.posStack = posStack;
}

void Engine::showContextMenu(GUI_ContextMenu* menu, Sint32 mouseX, Sint32 mouseY)
{
	if(m_contextMenu)
		delete m_contextMenu;
	m_contextMenu = menu;
	m_contextMenu->setDisplay(mouseX, mouseY);
}

void Engine::showDescription(Sint32 mouseX, Sint32 mouseY, const std::string& description, Uint32 delay)
{
	if(!m_description)
		m_description = new GUI_Description();
	m_description->setDisplay(mouseX, mouseY, description, delay);
}

bool Engine::addToPanel(GUI_PanelWindow* pPanel, Sint32 preferredPanel)
{
	for(std::vector<GUI_Panel*>::reverse_iterator it = m_panels.rbegin(), end = m_panels.rend(); it != end; ++it)
	{
		if(preferredPanel != GUI_PANEL_RANDOM)
		{
			if((*it)->getInternalID() == preferredPanel)
			{
				if((*it)->getFreeHeight() >= pPanel->getRect().y2 || (*it)->tryFreeHeight(pPanel))
				{
					if(pPanel->getParent())
						pPanel->getParent()->removePanel(pPanel, false);

					(*it)->addPanel(pPanel);
					pPanel->setParent((*it));
					return true;
				}
				return false;
			}
		}
		else
		{
			if((*it)->getFreeHeight() >= pPanel->getRect().y2)
			{
				if(pPanel->getParent())
					pPanel->getParent()->removePanel(pPanel, false);

				(*it)->addPanel(pPanel);
				pPanel->setParent((*it));
				return true;
			}
		}
	}
	if(preferredPanel == GUI_PANEL_RANDOM)
	{
		for(std::vector<GUI_Panel*>::reverse_iterator it = m_panels.rbegin(), end = m_panels.rend(); it != end; ++it)
		{
			if((*it)->tryFreeHeight(pPanel))
			{
				if(pPanel->getParent())
					pPanel->getParent()->removePanel(pPanel, false);

				(*it)->addPanel(pPanel);
				pPanel->setParent((*it));
				return true;
			}
		}
	}
	return false;
}

void Engine::removePanelWindow(GUI_PanelWindow* pPanel)
{
	for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
		(*it)->removePanel(pPanel, false);
	
	delete pPanel;
}

GUI_PanelWindow* Engine::getPanel(Uint32 internalID)
{
	GUI_PanelWindow* panel = NULL;
	for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
	{
		panel = (*it)->getPanel(internalID);
		if(panel)
			return panel;
	}
	return panel;
}

void Engine::clearPanels()
{
	for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
		m_toReleasePanels.push_back((*it));
	m_panels.clear();
}

void Engine::checkPanelWindows(GUI_PanelWindow* pPanel, Sint32 x, Sint32 y)
{
	GUI_Panel* gPanel = NULL;
	iRect currentRect = pPanel->getRect();
	GUI_Panel* parent = pPanel->getParent();
	std::vector<GUI_Panel*>::iterator it = m_panels.begin();
	for(std::vector<GUI_Panel*>::iterator end = m_panels.end(); it != end; ++it)
	{
		if((*it)->isInsideRect(x, y))
		{
			if((*it) == parent)
			{
				parent->checkPanel(pPanel, x, y);
				pPanel->setRect(currentRect);
			}
			else if(pPanel->isParentChangeable())
			{
				if((*it)->getFreeHeight() >= currentRect.y2)
				{
					gPanel = (*it);
					break;
				}
			}
			return;
		}
	}
	if(gPanel)
	{
		if(parent)
		{
			parent->removePanel(pPanel, false);
			parent->setActPanel(NULL);
		}
		gPanel->addPanel(pPanel);
		gPanel->checkPanel(pPanel, x, y);
		gPanel->setActPanel(pPanel);
		pPanel->setRect(currentRect);
		pPanel->setParent(gPanel);
		m_panels.erase(it);
		m_panels.push_back(gPanel);
	}
}

void Engine::resizePanel(GUI_PanelWindow* pPanel, Sint32 x, Sint32 y)
{
	GUI_Panel* parent = pPanel->getParent();
	if(parent)
		parent->resizePanel(pPanel, x, y);
}

void Engine::processGameStart()
{
	m_ingame = true;
	g_ping = 0;

	g_chat.gameStart();
	g_game.reset();
	g_game.resetPlayerExperienceTable();
	recalculateGameWindow();
	clearWindows();
	clearPanels();
	checkReleaseQueue();

	g_game.sendPing();
	g_game.sendPingBack();
	g_game.sendAttackModes();

	m_panels.push_back(new GUI_Panel(iRect(m_windowW-176, 0, 176, 349), GUI_PANEL_MAIN));
	m_panels.push_back(new GUI_Panel(iRect(m_windowW-176, 349, 176, m_windowH-349), GUI_PANEL_RIGHT1));
	UTIL_createMinimapPanel();
	UTIL_createHealthPanel();
	UTIL_createInventoryPanel();
	UTIL_createButtonsPanel();

	UTIL_createSkillsWindow();
	//UTIL_createBattleWindow();
	//UTIL_createVipWindow();
}

void Engine::processGameEnd()
{
	m_ingame = false;
	g_ping = 0;

	clearPanels();
	checkReleaseQueue();
}
