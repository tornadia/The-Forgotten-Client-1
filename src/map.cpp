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

#include "map.h"
#include "automap.h"
#include "engine.h"
#include "tile.h"
#include "creature.h"
#include "effect.h"
#include "distanceEffect.h"
#include "animatedText.h"
#include "staticText.h"
#include "screenText.h"
#include "light.h"

#include <algorithm>

Map g_map;
extern Automap g_automap;
extern Engine g_engine;
extern LightSystem g_light;
extern Uint32 g_frameTime;

AStarNodes::AStarNodes(const Position& startPos)
{
	nodesTable.reserve(MAX_NODES_COMPLEXITY);
	#ifdef __USE_SSE__
	if(SDL_HasSSE())
	{
		calculatedNodes = SDL_reinterpret_cast(Sint32*, _mm_malloc(sizeof(Sint32)*MAX_NODES_COMPLEXITY, 32));
		nodes = SDL_reinterpret_cast(AStarNode*, SDL_malloc(sizeof(AStarNode)*MAX_NODES_COMPLEXITY));
		openNodes = SDL_reinterpret_cast(bool*, SDL_malloc(sizeof(bool)*MAX_NODES_COMPLEXITY));
		SDL_memset4(calculatedNodes, SDL_MAX_SINT32, MAX_NODES_COMPLEXITY);
	}
	else
	#endif
	{
		calculatedNodes = SDL_reinterpret_cast(Sint32*, SDL_malloc(sizeof(Sint32)*MAX_NODES_COMPLEXITY));
		nodes = SDL_reinterpret_cast(AStarNode*, SDL_malloc(sizeof(AStarNode)*MAX_NODES_COMPLEXITY));
		openNodes = SDL_reinterpret_cast(bool*, SDL_malloc(sizeof(bool)*MAX_NODES_COMPLEXITY));
	}

	curNode = 1;
	closedNodes = 0;
	openNodes[0] = true;

	AStarNode& startNode = nodes[0];
	startNode.parent = nullptr;
	startNode.x = startPos.x;
	startNode.y = startPos.y;
	startNode.f = 0;
	startNode.g = 0;
	nodesTable[(startPos.x << 16) | startPos.y] = 0;
	calculatedNodes[0] = 0;
}

AStarNodes::~AStarNodes()
{
	#ifdef __USE_SSE__
	if(SDL_HasSSE())
	{
		_mm_free(calculatedNodes);
		SDL_free(nodes);
		SDL_free(openNodes);
	}
	else
	#endif
	{
		SDL_free(calculatedNodes);
		SDL_free(nodes);
		SDL_free(openNodes);
	}
}

bool AStarNodes::createOpenNode(AStarNode* parent, Sint32 f, Uint32 xy, Sint32 Sx, Sint32 Sy, const Position& targetPos, const Position& pos)
{
	if(curNode >= MAX_NODES_COMPLEXITY)
		return false;

	int32_t retNode = curNode++;
	openNodes[retNode] = true;

	const Sint32 Dx = Position::getDistanceX(targetPos, pos);
	const Sint32 Dy = Position::getDistanceY(targetPos, pos);

	AStarNode& node = nodes[retNode];
	node.parent = parent;
	node.x = pos.x;
	node.y = pos.y;
	node.f = f;
	node.g = ((Dx - Sx) << 3) + ((Dy - Sy) << 3) + (((Dx > Dy) ? Dx : Dy) << 3);
	nodesTable[xy] = retNode;
	calculatedNodes[retNode] = f + node.g;
	return true;
}

AStarNode* AStarNodes::getBestNode()
{
	#ifdef __USE_AVX2__
	if(SDL_HasAVX2())
	{
		const __m256i increment = _mm256_set1_epi32(8);
		__m256i indices = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
		__m256i minindices = indices;
		__m256i minvalues = _mm256_load_si256(reinterpret_cast<const __m256i*>(calculatedNodes));
		for(Sint32 pos = 8; pos < curNode; pos += 8)
		{
			const __m256i values = _mm256_load_si256(reinterpret_cast<const __m256i*>(&calculatedNodes[pos]));
			indices = _mm256_add_epi32(indices, increment);
			minindices = _mm256_blendv_epi8(minindices, indices, _mm256_cmpgt_epi32(minvalues, values));
			minvalues = _mm256_min_epi32(values, minvalues);
		}

		alignas(32) Sint32 values_array[8];
		alignas(32) Sint32 indices_array[8];
		_mm256_store_si256(reinterpret_cast<__m256i*>(values_array), minvalues);
		_mm256_store_si256(reinterpret_cast<__m256i*>(indices_array), minindices);

		Sint32 best_node = indices_array[0];
		Sint32 best_node_f = values_array[0];
		for(Sint32 i = 1; i < 8; ++i)
		{
			if(values_array[i] < best_node_f)
			{
				best_node_f = values_array[i];
				best_node = indices_array[i];
			}
		}
		return ((best_node != -1 && openNodes[best_node]) ? &nodes[best_node] : NULL);
	}
	else
	#endif
	#ifdef __USE_SSE2__
	if(SDL_HasSSE2())
	{
		const __m128i increment = _mm_set1_epi32(4);
		__m128i indices = _mm_setr_epi32(0, 1, 2, 3);
		__m128i minindices = indices;
		__m128i minvalues = _mm_load_si128(reinterpret_cast<const __m128i*>(calculatedNodes));
		for(Sint32 pos = 4; pos < curNode; pos += 4)
		{
			const __m128i values = _mm_load_si128(reinterpret_cast<const __m128i*>(&calculatedNodes[pos]));
			indices = _mm_add_epi32(indices, increment);
			minindices = _mm_blendv_epi8(minindices, indices, _mm_cmplt_epi32(values, minvalues));
			minvalues = _mm_min_epi32(values, minvalues);
		}

		alignas(16) Sint32 values_array[4];
		alignas(16) Sint32 indices_array[4];
		_mm_store_si128(reinterpret_cast<__m128i*>(values_array), minvalues);
		_mm_store_si128(reinterpret_cast<__m128i*>(indices_array), minindices);

		Sint32 best_node = indices_array[0];
		Sint32 best_node_f = values_array[0];
		for(Sint32 i = 1; i < 4; ++i)
		{
			if(values_array[i] < best_node_f)
			{
				best_node_f = values_array[i];
				best_node = indices_array[i];
			}
		}
		return ((best_node != -1 && openNodes[best_node]) ? &nodes[best_node] : NULL);
	}
	else
	#endif
	{
		Sint32 best_node_f = SDL_MAX_SINT32;
		Sint32 best_node = -1;
		for(Sint32 i = 0; i < curNode; ++i)
		{
			if(!openNodes[i])
				continue;

			if(calculatedNodes[i] < best_node_f)
			{
				best_node_f = calculatedNodes[i];
				best_node = i;
			}
		}
		return (best_node != -1 ? &nodes[best_node] : NULL);
	}
}

void AStarNodes::closeNode(AStarNode* node)
{
	Sint32 index = SDL_static_cast(Sint32, node - nodes);
	calculatedNodes[index] = SDL_MAX_SINT32;
	openNodes[index] = false;
	++closedNodes;
}

void AStarNodes::openNode(AStarNode* node)
{
	Sint32 index = SDL_static_cast(Sint32, node - nodes);
	calculatedNodes[index] = nodes[index].f + nodes[index].g;
	if(!openNodes[index])
	{
		openNodes[index] = true;
		--closedNodes;
	}
}

AStarNode* AStarNodes::getNodeByPosition(Uint32 xy)
{
	//We could make this a little faster using better hash map like for example google dense hash map
	std::unordered_map<Uint32, Uint32>::iterator it = nodesTable.find(xy);
	return (it == nodesTable.end() ? NULL : &nodes[it->second]);
}

Sint32 AStarNodes::getMapWalkFactor(AStarNode* node, const Position& neighborPos)
{
	return (((std::abs(node->x-neighborPos.x)+std::abs(node->y-neighborPos.y))-1)*MAP_DIAGONALWALKFACTOR)+MAP_NORMALWALKFACTOR;
}

Map::Map()
{
	for(Sint32 z = 0; z <= GAME_MAP_FLOORS; ++z)
	{
		for(Sint32 y = 0; y < GAME_MAP_HEIGHT; ++y)
		{
			for(Sint32 x = 0; x < GAME_MAP_WIDTH; ++x)
				m_tiles[z][y][x] = NULL;
		}
	}
	for(Sint32 i = ONSCREEN_MESSAGE_BOTTOM; i < ONSCREEN_MESSAGE_LAST; ++i)
		m_onscreenMessages[i] = new ScreenText(SDL_static_cast(OnscreenMessages, i));

	m_localCreature = NULL;

	m_magicEffectsTime = 0;
	m_distanceEffectsTime = 0;

	m_cachedLastVisibleFloor = 7;
	m_cachedFirstVisibleFloor = 0;

	m_needUpdateCache = true;
}

Map::~Map()
{
	for(Sint32 z = 0; z <= GAME_MAP_FLOORS; ++z)
	{
		std::vector<DistanceEffect*>& distanceEffects = m_distanceEffects[z];
		for(std::vector<DistanceEffect*>::iterator it = distanceEffects.begin(), end = distanceEffects.end(); it != end; ++it)
			delete (*it);

		distanceEffects.clear();
		for(Sint32 y = 0; y < GAME_MAP_HEIGHT; ++y)
		{
			for(Sint32 x = 0; x < GAME_MAP_WIDTH; ++x)
			{
				Tile* tile = m_tiles[z][y][x];
				if(tile)
					delete tile;
			}
		}
	}
	for(Sint32 i = ONSCREEN_MESSAGE_BOTTOM; i < ONSCREEN_MESSAGE_LAST; ++i)
		delete m_onscreenMessages[i];

	for(knownCreatures::iterator it = m_knownCreatures.begin(), end = m_knownCreatures.end(); it != end; ++it)
		delete it->second;

	m_knownCreatures.clear();
	for(std::vector<AnimatedText*>::iterator it = m_animatedTexts.begin(), end = m_animatedTexts.end(); it != end; ++it)
		delete (*it);

	m_animatedTexts.clear();
	for(std::vector<StaticText*>::iterator it = m_staticTexts.begin(), end = m_staticTexts.end(); it != end; ++it)
		delete (*it);

	m_staticTexts.clear();
}

Tile* Map::getTile(const Position& position)
{
	Sint16 offset = Position::getOffsetZ(m_centerPosition, position);
	Sint32 posX = Position::getOffsetX(position, m_centerPosition)+(MAP_WIDTH_OFFSET-1)-offset;
	Sint32 posY = Position::getOffsetY(position, m_centerPosition)+(MAP_HEIGHT_OFFSET-1)-offset;
	if(posX < 0 || posX >= GAME_MAP_WIDTH || posY < 0 || posY >= GAME_MAP_HEIGHT)
		return NULL;

	return m_tiles[position.z][posY][posX];
}

Tile* Map::resetTile(const Position& position, Sint32 offset)
{
	Sint32 posX = Position::getOffsetX(position, m_centerPosition)+(MAP_WIDTH_OFFSET-1)-offset;
	Sint32 posY = Position::getOffsetY(position, m_centerPosition)+(MAP_HEIGHT_OFFSET-1)-offset;
	if(posX < 0 || posX >= GAME_MAP_WIDTH || posY < 0 || posY >= GAME_MAP_HEIGHT)
	{
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Tile Pos: %d, %d, %d\nPlayer Pos: %d, %d, %d\nRows: %d, %d\nOffset: %d", position.x, position.y, position.z, m_centerPosition.x, m_centerPosition.y, m_centerPosition.z, posX, posY, offset);
		UTIL_MessageBox(true, g_buffer);
		return NULL;
	}

	Tile* tile = m_tiles[position.z][posY][posX];
	if(tile)
		delete tile;
	
	m_tiles[position.z][posY][posX] = new Tile(position);
	return m_tiles[position.z][posY][posX];
}

void Map::cleanTile(const Position& position, Sint32 offset)
{
	Sint32 posX = Position::getOffsetX(position, m_centerPosition)+(MAP_WIDTH_OFFSET-1)-offset;
	Sint32 posY = Position::getOffsetY(position, m_centerPosition)+(MAP_HEIGHT_OFFSET-1)-offset;
	if(posX < 0 || posX >= GAME_MAP_WIDTH || posY < 0 || posY >= GAME_MAP_HEIGHT)
		return;

	Tile* tile = m_tiles[position.z][posY][posX];
	if(tile)
	{
		delete tile;
		m_tiles[position.z][posY][posX] = NULL;
	}
}

void Map::render()
{
	if(!m_localCreature) //If somehow we don't have localcreature avoid crashing
		return;

	if(m_needUpdateCache)
		updateCacheMap();

	m_localCreature->update();
	Sint32 offsetX = -m_localCreature->getOffsetX(true);
	Sint32 offsetY = -m_localCreature->getOffsetY(true);
	g_light.initLightMap(offsetX, offsetY);

	Surface* renderer = g_engine.getRender();
	renderer->beginGameScene();
	for(Sint32 z = m_cachedLastVisibleFloor; z >= m_cachedFirstVisibleFloor; --z)
	{
		if(z != m_cachedLastVisibleFloor)
		{
			for(Sint32 y = 0; y < GAME_MAP_HEIGHT; ++y)
			{
				for(Sint32 x = 0; x < GAME_MAP_WIDTH; ++x)
				{
					//Reset only the first visible full ground tiles to minimalize cpu usage
					if(m_cachedFirstFullGrounds[y][x] == z)
						g_light.resetLightSource(x, y);
				}
			}
		}
		Sint32 posY = -32+offsetY;
		for(Sint32 y = 0; y < GAME_MAP_HEIGHT; ++y)
		{
			Sint32 posX = -32+offsetX;
			for(Sint32 x = 0; x < GAME_MAP_WIDTH; ++x)
			{
				Tile* tile = m_tiles[z][y][x];
				if(tile)
				{
					tile->render(posX, posY, (m_cachedFirstFullGrounds[y][x] >= z));
					if(tile->isCreatureLying())//Redraw tiles in <^ directions
					{
						for(Sint32 y2 = 1; y2 >= 0; --y2)
						{
							for(Sint32 x2 = 1; x2 >= 0; --x2)
							{
								if(x2 == 0 && y2 == 0)
									continue;

								Sint32 indexX = x-x2;
								Sint32 indexY = y-y2;
								if(indexX >= 0 && indexY >= 0)
								{
									tile = m_tiles[z][indexY][indexX];
									if(tile)
										tile->rerenderTile(posX-x2*32, posY-y2*32, (m_cachedFirstFullGrounds[indexY][indexX] >= z));
								}
							}
						}
					}
					if(z == GAME_PLAYER_FLOOR+1)
					{
						//Check for translucent on sea floor so if needed we draw small light source
						Tile* seaTile = m_tiles[GAME_PLAYER_FLOOR][y][x];
						if(seaTile && seaTile->hasTranslucentLight())
						{
							Uint16 translucentLight[2];
							translucentLight[0] = 1;
							translucentLight[1] = 215;
							g_light.addLightSource(posX, posY, translucentLight);
						}
					}
				}
				posX += 32;
			}
			posY += 32;
		}

		Sint32 offsetZ = (z-SDL_static_cast(Sint32, m_centerPosition.z));
		std::vector<DistanceEffect*>& distanceEffects = m_distanceEffects[z];
		for(std::vector<DistanceEffect*>::iterator it = distanceEffects.begin(); it != distanceEffects.end();)
		{
			float flightProgress = (*it)->getFlightProgress();
			if(flightProgress > 1.f)
			{
				delete (*it);
				it = distanceEffects.erase(it);
			}
			else
			{
				const Position& fromPos = (*it)->getFromPos();
				const Position& toPos = (*it)->getToPos();

				Sint32 screenxFrom = ((Position::getOffsetX(fromPos, m_centerPosition)+(MAP_WIDTH_OFFSET-2)+offsetZ)*32)+offsetX;
				Sint32 screenyFrom = ((Position::getOffsetY(fromPos, m_centerPosition)+(MAP_HEIGHT_OFFSET-2)+offsetZ)*32)+offsetY;

				Sint32 screenxTo = ((Position::getOffsetX(toPos, m_centerPosition)+(MAP_WIDTH_OFFSET-2)+offsetZ)*32)+offsetX;
				Sint32 screenyTo = ((Position::getOffsetY(toPos, m_centerPosition)+(MAP_HEIGHT_OFFSET-2)+offsetZ)*32)+offsetY;

				Sint32 screenx = screenxFrom+SDL_static_cast(Sint32, (screenxTo-screenxFrom)*flightProgress);
				Sint32 screeny = screenyFrom+SDL_static_cast(Sint32, (screenyTo-screenyFrom)*flightProgress);
				(*it)->render(screenx, screeny);
				++it;
			}
		}
	}
	renderer->endGameScene();
}

void Map::renderInformations(Sint32 px, Sint32 py, Sint32 pw, Sint32 ph, float scale, Sint32 scaledSize)
{
	if(!m_localCreature) //If somehow we don't have localcreature avoid crashing
		return;

	Sint32 offsetX;
	Sint32 offsetY;
	Sint32 tileOffsetX;
	Sint32 tileOffsetY;
	Sint32 startX;
	Sint32 startY;
	Sint32 endX;
	Sint32 endY;
	if(!m_localCreature->isWalking())
	{
		offsetX = 0;
		offsetY = 0;
		tileOffsetX = 0;
		tileOffsetY = 0;
		startX = 1;
		startY = 1;
		endX = (GAME_MAP_WIDTH-2);
		endY = (GAME_MAP_HEIGHT-2);
	}
	else if(m_localCreature->isPreWalking())
	{
		switch(m_localCreature->getWalkDirection())
		{
			case DIRECTION_EAST:
			{
				offsetX = -m_localCreature->getOffsetX(true);
				offsetY = 0;
				tileOffsetX = 32;
				tileOffsetY = 0;
				startX = 2;
				startY = 1;
				endX = (GAME_MAP_WIDTH-1);
				endY = (GAME_MAP_HEIGHT-2);
			}
			break;
			case DIRECTION_SOUTH:
			{
				offsetX = 0;
				offsetY = -m_localCreature->getOffsetY(true);
				tileOffsetX = 0;
				tileOffsetY = 32;
				startX = 1;
				startY = 2;
				endX = (GAME_MAP_WIDTH-2);
				endY = (GAME_MAP_HEIGHT-1);
			}
			break;
			case DIRECTION_WEST:
			{
				offsetX = -m_localCreature->getOffsetX(true);
				offsetY = 0;
				tileOffsetX = -32;
				tileOffsetY = 0;
				startX = 0;
				startY = 1;
				endX = (GAME_MAP_WIDTH-3);
				endY = (GAME_MAP_HEIGHT-2);
			}
			break;
			case DIRECTION_SOUTHWEST:
			{
				offsetX = -m_localCreature->getOffsetX(true);
				offsetY = -m_localCreature->getOffsetY(true);
				tileOffsetX = -32;
				tileOffsetY = 32;
				startX = 0;
				startY = 2;
				endX = (GAME_MAP_WIDTH-3);
				endY = (GAME_MAP_HEIGHT-1);
			}
			break;
			case DIRECTION_SOUTHEAST:
			{
				offsetX = -m_localCreature->getOffsetX(true);
				offsetY = -m_localCreature->getOffsetY(true);
				tileOffsetX = 32;
				tileOffsetY = 32;
				startX = 2;
				startY = 2;
				endX = (GAME_MAP_WIDTH-1);
				endY = (GAME_MAP_HEIGHT-1);
			}
			break;
			case DIRECTION_NORTHWEST:
			{
				offsetX = -m_localCreature->getOffsetX(true);
				offsetY = -m_localCreature->getOffsetY(true);
				tileOffsetX = -32;
				tileOffsetY = -32;
				startX = 0;
				startY = 0;
				endX = (GAME_MAP_WIDTH-3);
				endY = (GAME_MAP_HEIGHT-3);
			}
			break;
			case DIRECTION_NORTHEAST:
			{
				offsetX = -m_localCreature->getOffsetX(true);
				offsetY = -m_localCreature->getOffsetY(true);
				tileOffsetX = 32;
				tileOffsetY = -32;
				startX = 2;
				startY = 0;
				endX = (GAME_MAP_WIDTH-1);
				endY = (GAME_MAP_HEIGHT-3);
			}
			break;
			default://DIRECTION_NORTH
			{
				offsetX = 0;
				offsetY = -m_localCreature->getOffsetY(true);
				tileOffsetX = 0;
				tileOffsetY = -32;
				startX = 1;
				startY = 0;
				endX = (GAME_MAP_WIDTH-2);
				endY = (GAME_MAP_HEIGHT-3);
			}
			break;
		}
	}
	else
	{
		offsetX = -m_localCreature->getOffsetX();
		offsetY = -m_localCreature->getOffsetY();
		tileOffsetX = 0;
		tileOffsetY = 0;
		startX = 1;
		startY = 1;
		endX = (GAME_MAP_WIDTH-2);
		endY = (GAME_MAP_HEIGHT-2);
	}

	Sint32 fixedSize = scaledSize/2;
	Sint32 z = SDL_static_cast(Sint32, m_centerPosition.z);
	Sint32 posY = offsetY+tileOffsetY;
	for(Sint32 y = startY; y < endY; ++y)
	{
		Sint32 posX = offsetX+tileOffsetX+16;
		for(Sint32 x = startX; x < endX; ++x)
		{
			Tile* tile = m_tiles[z][y][x];
			if(tile)
				tile->renderInformations(px, py, posX, posY, scale, (m_cachedFirstVisibleGround[y][x] >= z));
			posX += 32;
		}
		posY += 32;
	}
	g_engine.getRender()->drawLightMap(g_light.getLightMap(), px+SDL_static_cast(Sint32, offsetX*scale), py+SDL_static_cast(Sint32, offsetY*scale), scaledSize, GAME_MAP_WIDTH, GAME_MAP_HEIGHT);
	
	pw += px;
	ph += py;
	for(std::vector<StaticText*>::iterator it = m_staticTexts.begin(); it != m_staticTexts.end();)
	{
		StaticText* staticText = (*it);
		const Position& position = staticText->getPosition();

		Sint32 offsetZ = (SDL_static_cast(Sint32, position.z) - z);
		Sint32 screenx = ((Position::getOffsetX(position, m_centerPosition)+(MAP_WIDTH_OFFSET-2)+offsetZ)*32)+offsetX;
		Sint32 screeny = ((Position::getOffsetY(position, m_centerPosition)+(MAP_HEIGHT_OFFSET-2)+offsetZ)*32)+offsetY;
		screenx = SDL_static_cast(Sint32, screenx*scale)+px+fixedSize;
		screeny = SDL_static_cast(Sint32, screeny*scale)+py;
		staticText->render(screenx, screeny, px, py, pw, ph);
		if(staticText->canBeDeleted())
		{
			delete staticText;
			it = m_staticTexts.erase(it);
		}
		else
			++it;
	}
	for(std::vector<AnimatedText*>::iterator it = m_animatedTexts.begin(); it != m_animatedTexts.end();)
	{
		AnimatedText* animatedText = (*it);
		if(animatedText->canBeDeleted())
		{
			delete animatedText;
			it = m_animatedTexts.erase(it);
			continue;
		}
		const Position& position = animatedText->getPosition();

		Sint32 offsetZ = (SDL_static_cast(Sint32, position.z) - z);
		Sint32 screenx = ((Position::getOffsetX(position, m_centerPosition)+(MAP_WIDTH_OFFSET-2)+offsetZ)*32)+offsetX;
		Sint32 screeny = ((Position::getOffsetY(position, m_centerPosition)+(MAP_HEIGHT_OFFSET-2)+offsetZ)*32)+offsetY;
		screenx = SDL_static_cast(Sint32, screenx*scale)+px+fixedSize;
		screeny = SDL_static_cast(Sint32, screeny*scale)+py;
		animatedText->render(screenx, screeny, px, py, pw, ph);
		++it;
	}
	for(Sint32 i = ONSCREEN_MESSAGE_BOTTOM; i < ONSCREEN_MESSAGE_LAST; ++i)
		m_onscreenMessages[i]->render(px, py, pw, ph);

	UTIL_refreshBattleWindow();
}

void Map::addOnscreenText(OnscreenMessages position, MessageMode mode, const std::string& text)
{
	m_onscreenMessages[position]->addMessage(mode, text);
}

void Map::addAnimatedText(const Position& position, Uint8 color, const std::string& text)
{
	Sint32 additionalOffset = 0;
	for(std::vector<AnimatedText*>::iterator it = m_animatedTexts.begin(), end = m_animatedTexts.end(); it != end; ++it)
	{
		AnimatedText* animatedText = (*it);
		if(animatedText->getPosition() == position)
		{
			if(animatedText->merge(color, text))
				return;
			else
			{
				if(animatedText->needAdditionalOffset())
					additionalOffset += 10;
			}
		}
	}

	AnimatedText* newAnimatedText = new AnimatedText(position, color, additionalOffset, text);
	m_animatedTexts.push_back(newAnimatedText);
}

void Map::addStaticText(const Position& position, const std::string& name, MessageMode mode, const std::string& text)
{
	for(std::vector<StaticText*>::iterator it = m_staticTexts.begin(), end = m_staticTexts.end(); it != end; ++it)
	{
		StaticText* staticText = (*it);
		if(staticText->getMode() == mode && staticText->getPosition() == position && staticText->getName() == name)
		{
			staticText->addMessage(name, mode, text);
			return;
		}
	}

	StaticText* newStaticText = new StaticText(position);
	newStaticText->addMessage(name, mode, text);
	m_staticTexts.push_back(newStaticText);
}

void Map::removeDistanceEffects(Uint8 posZ)
{
	std::vector<DistanceEffect*>& distanceEffects = m_distanceEffects[posZ];
	for(std::vector<DistanceEffect*>::iterator it = distanceEffects.begin(), end = distanceEffects.end(); it != end; ++it)
		delete (*it);

	distanceEffects.clear();
}

void Map::changeMap(Direction direction)
{
	switch(direction)
	{
		case DIRECTION_NORTH:
		{
			for(Sint32 z = 0; z <= GAME_MAP_FLOORS; ++z)
			{
				for(Sint32 x = 0; x < GAME_MAP_WIDTH; ++x)
				{
					Tile* tile = m_tiles[z][GAME_MAP_HEIGHT-1][x];
					if(tile)
						delete tile;
				}

				for(Sint32 y = GAME_MAP_HEIGHT-1; --y >= 0;)
				{
					for(Sint32 x = 0; x < GAME_MAP_WIDTH; ++x)
						m_tiles[z][y+1][x] = m_tiles[z][y][x];
				}

				for(Sint32 x = 0; x < GAME_MAP_WIDTH; ++x)
				{
					//Reset the tiles that we get later
					m_tiles[z][0][x] = NULL;
				}
			}
		}
		break;
		case DIRECTION_EAST:
		{
			for(Sint32 z = 0; z <= GAME_MAP_FLOORS; ++z)
			{
				for(Sint32 y = 0; y < GAME_MAP_HEIGHT; ++y)
				{
					Tile* tile = m_tiles[z][y][0];
					if(tile)
						delete tile;
				}

				for(Sint32 y = 0; y < GAME_MAP_HEIGHT; ++y)
				{
					for(Sint32 x = 0; x < GAME_MAP_WIDTH-1; ++x)
						m_tiles[z][y][x] = m_tiles[z][y][x+1];
				}

				for(Sint32 y = 0; y < GAME_MAP_HEIGHT; ++y)
				{
					//Reset the tiles that we get later
					m_tiles[z][y][GAME_MAP_WIDTH-1] = NULL;
				}
			}
		}
		break;
		case DIRECTION_SOUTH:
		{
			for(Sint32 z = 0; z <= GAME_MAP_FLOORS; ++z)
			{
				for(Sint32 x = 0; x < GAME_MAP_WIDTH; ++x)
				{
					Tile* tile = m_tiles[z][0][x];
					if(tile)
						delete tile;
				}

				for(Sint32 y = 0; y < GAME_MAP_HEIGHT-1; ++y)
				{
					for(Sint32 x = 0; x < GAME_MAP_WIDTH; ++x)
						m_tiles[z][y][x] = m_tiles[z][y+1][x];
				}

				for(Sint32 x = 0; x < GAME_MAP_WIDTH; ++x)
				{
					//Reset the tiles that we get later
					m_tiles[z][GAME_MAP_HEIGHT-1][x] = NULL;
				}
			}
		}
		break;
		case DIRECTION_WEST:
		{
			for(Sint32 z = 0; z <= GAME_MAP_FLOORS; ++z)
			{
				for(Sint32 y = 0; y < GAME_MAP_HEIGHT; ++y)
				{
					Tile* tile = m_tiles[z][y][GAME_MAP_WIDTH-1];
					if(tile)
						delete tile;
				}

				for(Sint32 y = 0; y < GAME_MAP_HEIGHT; ++y)
				{
					for(Sint32 x = GAME_MAP_WIDTH-1; --x >= 0;)
						m_tiles[z][y][x+1] = m_tiles[z][y][x];
				}

				for(Sint32 y = 0; y < GAME_MAP_HEIGHT; ++y)
				{
					//Reset the tiles that we get later
					m_tiles[z][y][0] = NULL;
				}
			}
		}
		break;
		default:
		{
			for(Sint32 z = 0; z <= GAME_MAP_FLOORS; ++z)
			{
				std::vector<DistanceEffect*>& distanceEffects = m_distanceEffects[z];
				for(std::vector<DistanceEffect*>::iterator it = distanceEffects.begin(), end = distanceEffects.end(); it != end; ++it)
					delete (*it);

				distanceEffects.clear();
				for(Sint32 y = 0; y < GAME_MAP_HEIGHT; ++y)
				{
					for(Sint32 x = 0; x < GAME_MAP_WIDTH; ++x)
					{
						Tile* tile = m_tiles[z][y][x];
						if(tile)
						{
							delete tile;
							m_tiles[z][y][x] = NULL;
						}
					}
				}
			}
		}
		break;
	}
}

void Map::checkMagicEffects()
{
	if(g_frameTime == m_magicEffectsTime)
		return;

	for(Sint32 z = 0; z <= GAME_MAP_FLOORS; ++z)
	{
		for(Sint32 y = 0; y < GAME_MAP_HEIGHT; ++y)
		{
			for(Sint32 x = 0; x < GAME_MAP_WIDTH; ++x)
			{
				Tile* tile = m_tiles[z][y][x];
				if(tile)
					tile->checkMagicEffects();
			}
		}
	}
}

void Map::checkDistanceEffects()
{
	if(g_frameTime == m_distanceEffectsTime)
		return;

	for(Sint32 z = 0; z <= GAME_MAP_FLOORS; ++z)
	{
		std::vector<DistanceEffect*>& distanceEffects = m_distanceEffects[z];
		for(std::vector<DistanceEffect*>::iterator it = distanceEffects.begin(); it != distanceEffects.end();)
		{
			if((*it)->getFlightProgress() > 1.f)
			{
				delete (*it);
				it = distanceEffects.erase(it);
			}
			else
				++it;
		}
	}
}

void Map::addDistanceEffect(DistanceEffect* distanceEffect, Uint8 posZ)
{
	if(posZ > GAME_MAP_FLOORS)
	{
		delete distanceEffect;
		return;
	}
	m_distanceEffects[posZ].push_back(distanceEffect);
}

PathFind Map::findPath(std::vector<Direction>& directions, const Position& startPos, const Position& endPos, Uint32 flags)
{
	directions.clear();
	if(startPos == endPos)
		return PathFind_ReturnSamePosition;
	
	if(startPos.z < endPos.z)
		return PathFind_ReturnFirstGoDownStairs;
	else if(startPos.z > endPos.z)
		return PathFind_ReturnFirstGoUpStairs;

	Tile* endTile = getTile(endPos);
	if(endTile)
	{
		if(!endTile->isWalkable())
			return PathFind_ReturnNoWay;
	}

	static Sint32 dirNeighbors[8][5][2] = {
		{{-1, 0},{0,  1},{ 1,  0},{ 1,  1},{-1,  1}},
		{{-1, 0},{0,  1},{ 0, -1},{-1, -1},{-1,  1}},
		{{-1, 0},{1,  0},{ 0, -1},{-1, -1},{ 1, -1}},
		{{ 0, 1},{1,  0},{ 0, -1},{ 1, -1},{ 1,  1}},
		{{ 1, 0},{0, -1},{-1, -1},{ 1, -1},{ 1,  1}},
		{{-1, 0},{0, -1},{-1, -1},{ 1, -1},{-1,  1}},
		{{ 0, 1},{1,  0},{ 1, -1},{ 1,  1},{-1,  1}},
		{{-1, 0},{0,  1},{-1, -1},{ 1,  1},{-1,  1}}
	};
	static Sint32 allNeighbors[8][2] = {
		{-1, 0},{0, 1},{1, 0},{0, -1},{-1, -1},{1, -1},{1, 1},{-1, 1}
	};

	Position pos = startPos;

	const Sint32 Sx = Position::getDistanceX(endPos, startPos);
	const Sint32 Sy = Position::getDistanceY(endPos, startPos);

	AStarNodes nodes(startPos);
	AStarNode* found = nullptr;
	while(nodes.getNodeSize() < MAX_NODES_COMPLEXITY)
	{
		AStarNode* n = nodes.getBestNode();
		if(!n)
		{
			if(found)
				break;
			return PathFind_ReturnNoWay;
		}

		const Sint32 x = n->x;
		const Sint32 y = n->y;
		pos.x = SDL_static_cast(Uint16, x);
		pos.y = SDL_static_cast(Uint16, y);
		if(n->x == endPos.x && n->y == endPos.y && (!found || n->f < found->f))
			found = n;

		if(found && n->f >= found->f)
			break;

		Uint32 dirCount;
		Sint32* neighbors;
		if(n->parent)
		{
			const Sint32 offset_x = n->parent->x - x;
			const Sint32 offset_y = n->parent->y - y;
			if(offset_y == 0)
			{
				if(offset_x == -1)
					neighbors = *dirNeighbors[DIRECTION_WEST];
				else
					neighbors = *dirNeighbors[DIRECTION_EAST];
			}
			else if(offset_x == 0)
			{
				if(offset_y == -1)
					neighbors = *dirNeighbors[DIRECTION_NORTH];
				else
					neighbors = *dirNeighbors[DIRECTION_SOUTH];
			}
			else if(offset_y == -1)
			{
				if(offset_x == -1)
					neighbors = *dirNeighbors[DIRECTION_NORTHWEST];
				else
					neighbors = *dirNeighbors[DIRECTION_NORTHEAST];
			}
			else if(offset_x == -1)
				neighbors = *dirNeighbors[DIRECTION_SOUTHWEST];
			else
				neighbors = *dirNeighbors[DIRECTION_SOUTHEAST];
			dirCount = 5;
		}
		else
		{
			dirCount = 8;
			neighbors = *allNeighbors;
		}

		const Sint32 f = n->f;
		for(Uint32 i = 0; i < dirCount; ++i)
		{
			pos.x = SDL_static_cast(Uint16, x + *neighbors++);
			pos.y = SDL_static_cast(Uint16, y + *neighbors++);

			bool wasSeen = false;
			bool hasCreature = false;
			bool isNotWalkable = false;
			bool isNotPathable = false;
			Uint16 speed = 100;

			endTile = getTile(pos);
			if(endTile)
			{
				wasSeen = true;
				hasCreature = endTile->getTopCreature();
				isNotWalkable = !endTile->isWalkable();
				isNotPathable = !endTile->isPathable();
				speed = endTile->getGroundSpeed();
			}
			else
			{
				bool wasSeenTile = g_automap.wasSeen(pos);
				if(wasSeenTile)
				{
					wasSeen = true;
					speed = g_automap.getSpeed(pos);
					isNotWalkable = (speed == 0);
				}
			}

			const Sint32 walkFactor = AStarNodes::getMapWalkFactor(n, pos);
			if(pos.x != endPos.x || pos.y != endPos.y)
			{
				if(!(flags & PathFindFlags_AllowUnseenTiles) && !wasSeen)
					continue;
				if(wasSeen)
				{
					if(!(flags & PathFindFlags_AllowCreatures) && hasCreature)
						continue;
					if(!(flags & PathFindFlags_AllowNonPathable) && isNotPathable)
						continue;
					if(!(flags & PathFindFlags_AllowNonWalkable) && isNotWalkable)
						continue;
				}
			}
			else
			{
				if(!(flags & PathFindFlags_AllowUnseenTiles) && !wasSeen)
					continue;
				if(wasSeen)
				{
					if(!(flags & PathFindFlags_AllowNonWalkable) && isNotWalkable)
						continue;
				}
			}

			const Sint32 cost = f + (SDL_static_cast(Sint32, speed) * walkFactor);

			const Uint32 posXY = (pos.x << 16) | pos.y;
			AStarNode* neighborNode = nodes.getNodeByPosition(posXY);
			if(neighborNode)
			{
				if(neighborNode->f <= cost)
					continue;

				neighborNode->f = cost;
				neighborNode->parent = n;
				nodes.openNode(neighborNode);
			}
			else
			{
				if(!nodes.createOpenNode(n, cost, posXY, Sx, Sy, endPos, pos))
				{
					if(found)
						break;
					return PathFind_ReturnNoWay;
				}
			}
		}
		nodes.closeNode(n);
	}
	if(!found)
		return PathFind_ReturnNoWay;

	Sint32 prevx = endPos.x;
	Sint32 prevy = endPos.y;
	while(found)
	{
		pos.x = found->x;
		pos.y = found->y;

		Sint32 dx = pos.x - prevx;
		Sint32 dy = pos.y - prevy;

		prevx = pos.x;
		prevy = pos.y;
		if((flags & PathFindFlags_AllowNonPathable) || getTile(pos))//In case of not allowing non pathable tiles we can only send the tiles we know
		{
			if(dx == 1)
			{
				if(dy == 1)
					directions.emplace_back(DIRECTION_NORTHWEST);
				else if(dy == -1)
					directions.emplace_back(DIRECTION_SOUTHWEST);
				else
					directions.emplace_back(DIRECTION_WEST);
			}
			else if(dx == -1)
			{
				if(dy == 1)
					directions.emplace_back(DIRECTION_NORTHEAST);
				else if(dy == -1)
					directions.emplace_back(DIRECTION_SOUTHEAST);
				else
					directions.emplace_back(DIRECTION_EAST);
			}
			else if(dy == 1)
				directions.emplace_back(DIRECTION_NORTH);
			else if(dy == -1)
				directions.emplace_back(DIRECTION_SOUTH);
		}
		found = found->parent;
	}
	return PathFind_ReturnSuccessfull;
}

Tile* Map::findTile(Sint32 x, Sint32 y, iRect& gameWindow, Sint32 scaledSize, float scale, Creature* &topCreature, bool multifloor)
{
	if(!m_localCreature) //If somehow we don't have localcreature avoid crashing
		return NULL;

	Sint32 xOffset = SDL_static_cast(Sint32, m_localCreature->getOffsetX(true)*scale);
	Sint32 yOffset = SDL_static_cast(Sint32, m_localCreature->getOffsetY(true)*scale);

	Sint32 firstVisibleFloor;
	Sint32 lastVisibleFloor;
	if(multifloor)
	{
		firstVisibleFloor = m_cachedLastVisibleFloor;
		lastVisibleFloor = m_cachedFirstVisibleFloor;
	}
	else
	{
		firstVisibleFloor = SDL_static_cast(Sint32, m_centerPosition.z);
		lastVisibleFloor = SDL_static_cast(Sint32, m_centerPosition.z);
	}
	topCreature = NULL;

	Sint32 posX = ((x-gameWindow.x1)/scaledSize)+1;
	Sint32 posY = ((y-gameWindow.y1)/scaledSize)+1;
	for(Sint32 z = lastVisibleFloor; z <= firstVisibleFloor; ++z)
	{
		if(!topCreature)
		{
			for(Sint32 y2 = 1; y2 >= -1; --y2)
			{
				for(Sint32 x2 = 1; x2 >= -1; --x2)
				{
					Sint32 indexX = posX+x2;
					Sint32 indexY = posY+y2;
					if(indexX >= 0 && indexY >= 0 && indexX < GAME_MAP_WIDTH && indexY < GAME_MAP_HEIGHT)
					{
						Tile* tile = m_tiles[z][indexY][indexX];
						if(tile)
						{
							Sint32 elevationSize = SDL_static_cast(Sint32, tile->getTileElevation()*scale);
							iRect irect = iRect(gameWindow.x1+((indexX-1)*scaledSize)-elevationSize-xOffset, gameWindow.y1+((indexY-1)*scaledSize)-elevationSize-yOffset, scaledSize+elevationSize, scaledSize+elevationSize);
							topCreature = tile->getTopCreature(x, y, irect, scale);
							if(topCreature)
								goto Search_for_Tile;
						}
					}
				}
			}
		}

		Search_for_Tile:
		for(Sint32 y2 = 1; y2 >= -1; --y2)
		{
			for(Sint32 x2 = 1; x2 >= -1; --x2)
			{
				Sint32 indexX = posX+x2;
				Sint32 indexY = posY+y2;
				if(indexX >= 0 && indexY >= 0 && indexX < GAME_MAP_WIDTH && indexY < GAME_MAP_HEIGHT)
				{
					Tile* tile = m_tiles[z][indexY][indexX];
					if(tile)
					{
						Sint32 elevationSize = SDL_static_cast(Sint32, tile->getTileElevation()*scale);
						iRect irect = iRect(gameWindow.x1+((indexX-1)*scaledSize)-elevationSize-xOffset, gameWindow.y1+((indexY-1)*scaledSize)-elevationSize-yOffset, scaledSize+elevationSize, scaledSize+elevationSize);
						if(irect.isPointInside(x, y))
						{
							if(tile->isLookingPossible())
								return tile;
						}
					}
				}
			}
		}
	}
	return NULL;
}

void Map::resetCreatures()
{
	for(knownCreatures::iterator it = m_knownCreatures.begin(), end = m_knownCreatures.end(); it != end; ++it)
		delete it->second;

	m_knownCreatures.clear();
	UTIL_resetBattleCreatures();
}

void Map::addCreatureById(Uint32 creatureId, Creature* creature)
{
	removeCreatureById(creatureId);//Make sure we don't get any memory leak
	m_knownCreatures[creatureId] = creature;
	UTIL_addBattleCreature(SDL_reinterpret_cast(void*, creature));
}

void Map::removeCreatureById(Uint32 creatureId)
{
	knownCreatures::iterator it = m_knownCreatures.find(creatureId);
	if(it != m_knownCreatures.end())
	{
		UTIL_removeBattleCreature(SDL_reinterpret_cast(void*, it->second));
		delete it->second;
		m_knownCreatures.erase(it);
	}
}

Creature* Map::getCreatureById(Uint32 creatureId)
{
	knownCreatures::iterator it = m_knownCreatures.find(creatureId);
	if(it != m_knownCreatures.end())
		return it->second;
	else
		return NULL;
}

void Map::setCentralPosition(Position pos)
{
	if(m_centerPosition == pos)
		return;

	m_centerPosition = pos;
}

void Map::updateCacheMap()
{
	Sint32 posZ = SDL_static_cast(Sint32, m_centerPosition.z);
	if(posZ > GAME_PLAYER_FLOOR)
		m_cachedLastVisibleFloor = posZ+GAME_MAP_AWARERANGE;
	else
		m_cachedLastVisibleFloor = GAME_PLAYER_FLOOR;

	Sint32 firstFloor = 0;
	if(posZ > GAME_PLAYER_FLOOR)
		firstFloor = UTIL_max<Sint32>(posZ-GAME_MAP_AWARERANGE, GAME_PLAYER_FLOOR+1);

	for(Sint32 x = -1; x <= 1 && firstFloor < posZ; ++x)
	{
		for(Sint32 y = -1; y <= 1 && firstFloor < posZ; ++y)
		{
			Position pos(SDL_static_cast(Uint16, SDL_static_cast(Sint32, m_centerPosition.x)+x), SDL_static_cast(Uint16, SDL_static_cast(Sint32, m_centerPosition.y)+y), m_centerPosition.z);

			bool isLookPossible = false;
			Tile* tile = getTile(pos);
			if(tile && tile->isLookPossible())
				isLookPossible = true;

			if((x == 0 && y == 0) || ((std::abs(x) != std::abs(y)) && isLookPossible))
			{
				Position upperPos = pos;
				Position coveredPos = pos;
				while(coveredPos.coveredUp() && upperPos.up() && upperPos.z >= firstFloor)
				{
					//Check tile physically above
					tile = getTile(upperPos);
					if(tile && tile->limitsFloorsView(!isLookPossible))
					{
						firstFloor = upperPos.z+1;
						break;
					}

					//Check tile geometrically above
					tile = getTile(coveredPos);
					if(tile && tile->limitsFloorsView(isLookPossible))
					{
						firstFloor = coveredPos.z+1;
						break;
					}
				}
			}
		}
	}

	m_cachedLastVisibleFloor = UTIL_max<Sint32>(0, UTIL_min<Sint32>(GAME_MAP_FLOORS, m_cachedLastVisibleFloor));
	m_cachedFirstVisibleFloor = UTIL_max<Sint32>(0, UTIL_min<Sint32>(GAME_MAP_FLOORS, firstFloor));
	m_needUpdateCache = false;
	for(Sint32 y = 0; y < GAME_MAP_HEIGHT; ++y)
	{
		for(Sint32 x = 0; x < GAME_MAP_WIDTH; ++x)
		{
			bool haveFirstVisible = false;
			bool haveFirstFloor = false;
			for(Sint32 z = m_cachedFirstVisibleFloor; z < m_cachedLastVisibleFloor; ++z)
			{
				Tile* tile = m_tiles[z][y][x];
				if(tile)
				{
					if(!haveFirstVisible && tile->limitsFloorsView(false))
					{
						m_cachedFirstVisibleGround[y][x] = z;
						haveFirstVisible = true;
					}

					if(!haveFirstFloor && tile->isFullground())
					{
						m_cachedFirstFullGrounds[y][x] = z;
						haveFirstFloor = true;
					}
				}
			}

			if(!haveFirstVisible)
				m_cachedFirstVisibleGround[y][x] = m_cachedLastVisibleFloor;

			if(!haveFirstFloor)
				m_cachedFirstFullGrounds[y][x] = m_cachedLastVisibleFloor;
		}
	}
}
