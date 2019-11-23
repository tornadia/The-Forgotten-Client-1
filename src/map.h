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

#ifndef __FILE_MAP_h_
#define __FILE_MAP_h_

#include "position.h"

#define MAP_WIDTH_OFFSET (GAME_MAP_WIDTH/2)
#define MAP_HEIGHT_OFFSET (GAME_MAP_HEIGHT/2)

class Tile;
class DistanceEffect;
class AnimatedText;
class StaticText;
class ScreenText;
class Creature;

typedef std::unordered_map<Uint32, Creature*> knownCreatures;

struct AStarNode
{
	AStarNode* parent;
	Sint32 f, g;
	Uint16 x, y;
};

static const Sint32 MAX_NODES_COMPLEXITY = 16384;//Should be divisible by 32, most welcomed multiply factor of 2
static const Sint32 MAP_NORMALWALKFACTOR = 1;
static const Sint32 MAP_DIAGONALWALKFACTOR = 2;

class AStarNodes
{
	public:
		AStarNodes(const Position& startPos);
		~AStarNodes();

		bool createOpenNode(AStarNode* parent, Sint32 f, Uint32 xy, Sint32 Sx, Sint32 Sy, const Position& targetPos, const Position& pos);
		AStarNode* getBestNode();
		void closeNode(AStarNode* node);
		void openNode(AStarNode* node);
		SDL_FORCE_INLINE Sint32 getNodeSize() const {return curNode;}
		SDL_FORCE_INLINE Sint32 getClosedNodes() const {return closedNodes;}
		AStarNode* getNodeByPosition(Uint32 xy);

		static Sint32 getMapWalkFactor(AStarNode* node, const Position& neighborPos);

	private:
		std::unordered_map<Uint32, Uint32> nodesTable;
		Sint32* calculatedNodes;
		AStarNode* nodes;
		bool* openNodes;
		Sint32 closedNodes;
		Sint32 curNode;
};

class Map
{
	public:
		Map();
		~Map();

		Tile* getTile(const Position& position);
		Tile* resetTile(const Position& position, Sint32 offset);
		void cleanTile(const Position& position, Sint32 offset);

		void render();
		void renderInformations(Sint32 px, Sint32 py, Sint32 pw, Sint32 ph, float scale, Sint32 scaledSize);

		void addOnscreenText(OnscreenMessages position, MessageMode mode, const std::string& text);
		void addAnimatedText(const Position& position, Uint8 color, const std::string& text);
		void addStaticText(const Position& position, const std::string& name, MessageMode mode, const std::string& text);
		void removeDistanceEffects(Uint8 posZ);
		void changeMap(Direction direction);

		void checkMagicEffects();
		void checkDistanceEffects();
		void addDistanceEffect(DistanceEffect* distanceEffect, Uint8 posZ);

		PathFind findPath(std::vector<Direction>& directions, const Position& startPos, const Position& endPos, Uint32 flags = 0);
		Tile* findTile(Sint32 x, Sint32 y, iRect& gameWindow, Sint32 scaledSize, float scale, Creature* &topCreature, bool multifloor);

		Creature* getLocalCreature() {return m_localCreature;}
		void setLocalCreature(Creature* creature) {m_localCreature = creature;}

		void resetCreatures();
		void addCreatureById(Uint32 creatureId, Creature* creature);
		void removeCreatureById(Uint32 creatureId);
		Creature* getCreatureById(Uint32 creatureId);

		void setCentralPosition(Position pos);
		SDL_FORCE_INLINE Position& getCentralPosition() {return m_centerPosition;}

		SDL_INLINE void needUpdateCache() {m_needUpdateCache = true;}
		void updateCacheMap();

	protected:
		knownCreatures m_knownCreatures;
		std::vector<DistanceEffect*> m_distanceEffects[GAME_MAP_FLOORS+1];
		std::vector<AnimatedText*> m_animatedTexts;
		std::vector<StaticText*> m_staticTexts;
		Tile* m_tiles[GAME_MAP_FLOORS+1][GAME_MAP_HEIGHT][GAME_MAP_WIDTH];
		ScreenText* m_onscreenMessages[ONSCREEN_MESSAGE_LAST];

		Creature* m_localCreature;
		Position m_centerPosition;
		
		Uint32 m_magicEffectsTime;
		Uint32 m_distanceEffectsTime;

		Sint32 m_cachedFirstVisibleGround[GAME_MAP_HEIGHT][GAME_MAP_WIDTH];
		Sint32 m_cachedFirstFullGrounds[GAME_MAP_HEIGHT][GAME_MAP_WIDTH];
		Sint32 m_cachedLastVisibleFloor;
		Sint32 m_cachedFirstVisibleFloor;

		bool m_needUpdateCache;
};

#endif /* __FILE_MAP_h_ */
