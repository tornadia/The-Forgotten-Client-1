/*
  The Forgotten Client
  Copyright (C) 2020 Saiyans King

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
	openNodes.reserve(MAX_NODES_COMPLEXITY);
	openNodeTotalCost.reserve(MAX_NODES_COMPLEXITY);
	nodesTable.reserve(MAX_NODES_COMPLEXITY);
	nodes = SDL_reinterpret_cast(AStarNode*, SDL_malloc(sizeof(AStarNode) * MAX_NODES_COMPLEXITY));

	curNode = 1;

	AStarNode& startNode = nodes[0];
	startNode.parent = nullptr;
	startNode.x = startPos.x;
	startNode.y = startPos.y;
	startNode.f = 0;
	startNode.g = 0;
	nodesTable[(startPos.x << 16) | startPos.y] = 0;
	openNodes.emplace_back(0);
	openNodeTotalCost.emplace_back(0);
}

AStarNodes::~AStarNodes()
{
	SDL_free(nodes);
}

bool AStarNodes::createOpenNode(AStarNode* parent, Sint32 f, Uint32 xy, Sint32 Sx, Sint32 Sy, const Position& targetPos, const Position& pos)
{
	if(curNode >= MAX_NODES_COMPLEXITY)
		return false;

	Sint32 retNode = curNode++;
	const Sint32 Dx = Position::getDistanceX(targetPos, pos);
	const Sint32 Dy = Position::getDistanceY(targetPos, pos);

	AStarNode& node = nodes[retNode];
	node.parent = parent;
	node.x = pos.x;
	node.y = pos.y;
	node.f = f;
	node.g = ((Dx - Sx) << 3) + ((Dy - Sy) << 3) + (((Dx > Dy) ? Dx : Dy) << 3);
	nodesTable[xy] = retNode;
	openNodes.emplace_back(retNode);
	openNodeTotalCost.emplace_back(f + node.g);
	return true;
}

#ifdef __USE_SSE2__
SDL_FORCE_INLINE __m128i _mm_sse2_min_epi32(const __m128i a, const __m128i b)
{
	__m128i mask = _mm_cmpgt_epi32(a, b);
	return _mm_or_si128(_mm_and_si128(mask, b), _mm_andnot_si128(mask, a));
};

SDL_FORCE_INLINE __m128i _mm_sse2_blendv_epi8(const __m128i a, const __m128i b, __m128i mask)
{
	mask = _mm_cmplt_epi8(mask, _mm_setzero_si128());
	return _mm_or_si128(_mm_andnot_si128(mask, a), _mm_and_si128(mask, b));
};
#endif

AStarNode* AStarNodes::getBestNode()
{
	//Branchless best node search
	Sint32 best_node_f = SDL_MAX_SINT32;
	Sint32 best_node = -1;

	size_t it = 0;
	size_t end = openNodes.size();
	if(end >= 8)
	{
		#ifdef __USE_AVX2__
		if(SDL_HasAVX2())
		{
			const __m256i increment = _mm256_set1_epi32(8);
			__m256i indices = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
			__m256i minindices = indices;
			__m256i minvalues = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&openNodeTotalCost[it]));

			it += 8; end -= 8;
			for(; it <= end; it += 8)
			{
				__m256i values = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&openNodeTotalCost[it]));
				indices = _mm256_add_epi32(indices, increment);
				minindices = _mm256_blendv_epi8(minindices, indices, _mm256_cmpgt_epi32(minvalues, values));
				minvalues = _mm256_min_epi32(values, minvalues);
			}
			end += 8;

			alignas(32) Sint32 values_array[8];
			alignas(32) Sint32 indices_array[8];
			_mm256_store_si256(reinterpret_cast<__m256i*>(values_array), minvalues);
			_mm256_store_si256(reinterpret_cast<__m256i*>(indices_array), minindices);

			best_node = openNodes[indices_array[0]];
			best_node_f = values_array[0];
			for(Sint32 i = 1; i < 8; ++i)
			{
				Sint32 total_cost = values_array[i];
				best_node = (total_cost < best_node_f ? openNodes[indices_array[i]] : best_node);
				best_node_f = (total_cost < best_node_f ? total_cost : best_node_f);
			}
		}
		else
		#endif
		#ifdef __USE_SSE4_1__
		if(SDL_HasSSE41())
		{
			const __m128i increment = _mm_set1_epi32(4);
			__m128i indices = _mm_setr_epi32(0, 1, 2, 3);
			__m128i minindices = indices;
			__m128i minvalues = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&openNodeTotalCost[it]));

			it += 4; end -= 4;
			for(; it <= end; it += 4)
			{
				__m128i values = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&openNodeTotalCost[it]));
				indices = _mm_add_epi32(indices, increment);
				minindices = _mm_blendv_epi8(minindices, indices, _mm_cmplt_epi32(values, minvalues));
				minvalues = _mm_min_epi32(values, minvalues);
			}
			end += 4;
			
			alignas(16) Sint32 values_array[4];
			alignas(16) Sint32 indices_array[4];
			_mm_store_si128(reinterpret_cast<__m128i*>(values_array), minvalues);
			_mm_store_si128(reinterpret_cast<__m128i*>(indices_array), minindices);

			best_node = openNodes[indices_array[0]];
			best_node_f = values_array[0];
			for(Sint32 i = 1; i < 4; ++i)
			{
				Sint32 total_cost = values_array[i];
				best_node = (total_cost < best_node_f ? openNodes[indices_array[i]] : best_node);
				best_node_f = (total_cost < best_node_f ? total_cost : best_node_f);
			}
		}
		else
		#endif
		#ifdef __USE_SSE2__
		if(SDL_HasSSE2())
		{
			const __m128i increment = _mm_set1_epi32(4);
			__m128i indices = _mm_setr_epi32(0, 1, 2, 3);
			__m128i minindices = indices;
			__m128i minvalues = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&openNodeTotalCost[it]));

			it += 4; end -= 4;
			for(; it <= end; it += 4)
			{
				__m128i values = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&openNodeTotalCost[it]));
				indices = _mm_add_epi32(indices, increment);
				minindices = _mm_sse2_blendv_epi8(minindices, indices, _mm_cmplt_epi32(values, minvalues));
				minvalues = _mm_sse2_min_epi32(values, minvalues);
			}
			end += 4;

			alignas(16) Sint32 values_array[4];
			alignas(16) Sint32 indices_array[4];
			_mm_store_si128(reinterpret_cast<__m128i*>(values_array), minvalues);
			_mm_store_si128(reinterpret_cast<__m128i*>(indices_array), minindices);

			best_node = openNodes[indices_array[0]];
			best_node_f = values_array[0];
			for(Sint32 i = 1; i < 4; ++i)
			{
				Sint32 total_cost = values_array[i];
				best_node = (total_cost < best_node_f ? openNodes[indices_array[i]] : best_node);
				best_node_f = (total_cost < best_node_f ? total_cost : best_node_f);
			}
		}
		else
		#endif
		{
			//Nothing here because we use standard code part for non-simd code
		}
	}

	for(; it < end; ++it)
	{
		Sint32 total_cost = openNodeTotalCost[it];
		best_node = (total_cost < best_node_f ? openNodes[it] : best_node);
		best_node_f = (total_cost < best_node_f ? total_cost : best_node_f);
	}
	return (best_node != -1 ? &nodes[best_node] : NULL);
}

void AStarNodes::closeNode(AStarNode* node)
{
	Sint32 index = SDL_static_cast(Sint32, node - nodes);
	for(size_t it = 0, end = openNodes.size(); it < end; ++it)
	{
		if(openNodes[it] == index)
		{
			openNodes.erase(openNodes.begin() + it);
			openNodeTotalCost.erase(openNodeTotalCost.begin() + it);
			break;
		}
	}
}

void AStarNodes::openNode(AStarNode* node)
{
	Sint32 index = SDL_static_cast(Sint32, node - nodes);
	for(size_t it = 0, end = openNodes.size(); it < end; ++it)
	{
		if(openNodes[it] == index)
		{
			openNodeTotalCost[it] = nodes[index].f + nodes[index].g;
			return;
		}
	}

	openNodes.emplace_back(index);
	openNodeTotalCost.emplace_back(nodes[index].f + nodes[index].g);
}

AStarNode* AStarNodes::getNodeByPosition(Uint32 xy)
{
	//We could make this a little faster using better hash map like for example google dense hash map
	std::unordered_map<Uint32, Uint32>::iterator it = nodesTable.find(xy);
	return (it == nodesTable.end() ? NULL : &nodes[it->second]);
}

Sint32 AStarNodes::getMapWalkFactor(AStarNode* node, const Position& neighborPos)
{
	return (((std::abs(node->x - neighborPos.x) + std::abs(node->y - neighborPos.y)) - 1) * MAP_DIAGONALWALKFACTOR) + MAP_NORMALWALKFACTOR;
}

Map::Map()
{
	for(Sint32 i = ONSCREEN_MESSAGE_BOTTOM; i < ONSCREEN_MESSAGE_LAST; ++i)
		m_onscreenMessages[i] = new ScreenText(SDL_static_cast(OnscreenMessages, i));
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
	Sint32 posX = Position::getOffsetX(position, m_centerPosition) + (MAP_WIDTH_OFFSET - 1) - offset;
	Sint32 posY = Position::getOffsetY(position, m_centerPosition) + (MAP_HEIGHT_OFFSET - 1) - offset;
	if(posX < 0 || posX >= GAME_MAP_WIDTH || posY < 0 || posY >= GAME_MAP_HEIGHT)
		return NULL;

	return m_tiles[position.z][posY][posX];
}

Tile* Map::getTileOrCreate(const Position& position)
{
	Sint16 offset = Position::getOffsetZ(m_centerPosition, position);
	Sint32 posX = Position::getOffsetX(position, m_centerPosition) + (MAP_WIDTH_OFFSET - 1) - offset;
	Sint32 posY = Position::getOffsetY(position, m_centerPosition) + (MAP_HEIGHT_OFFSET - 1) - offset;
	if(posX < 0 || posX >= GAME_MAP_WIDTH || posY < 0 || posY >= GAME_MAP_HEIGHT)
		return NULL;

	Tile*& tile = m_tiles[position.z][posY][posX];
	if(!tile)
		tile = new Tile(position);

	return tile;
}

Tile* Map::resetTile(const Position& position, Sint32 offset)
{
	Sint32 posX = Position::getOffsetX(position, m_centerPosition) + (MAP_WIDTH_OFFSET - 1) - offset;
	Sint32 posY = Position::getOffsetY(position, m_centerPosition) + (MAP_HEIGHT_OFFSET - 1) - offset;
	if(posX < 0 || posX >= GAME_MAP_WIDTH || posY < 0 || posY >= GAME_MAP_HEIGHT)
	{
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Tile Pos: %d, %d, %d\nPlayer Pos: %d, %d, %d\nRows: %d, %d\nOffset: %d", position.x, position.y, position.z, m_centerPosition.x, m_centerPosition.y, m_centerPosition.z, posX, posY, offset);
		UTIL_MessageBox(true, g_buffer);
		return NULL;
	}

	Tile* tile = m_tiles[position.z][posY][posX];
	if(tile)
		tile->reset();
	else
	{
		m_tiles[position.z][posY][posX] = new Tile(position);
		return m_tiles[position.z][posY][posX];
	}
	
	return tile;
}

void Map::cleanTile(const Position& position, Sint32 offset)
{
	Sint32 posX = Position::getOffsetX(position, m_centerPosition) + (MAP_WIDTH_OFFSET - 1) - offset;
	Sint32 posY = Position::getOffsetY(position, m_centerPosition) + (MAP_HEIGHT_OFFSET - 1) - offset;
	if(posX < 0 || posX >= GAME_MAP_WIDTH || posY < 0 || posY >= GAME_MAP_HEIGHT)
		return;

	Tile* tile = m_tiles[position.z][posY][posX];
	if(tile)
	{
		delete tile;
		m_tiles[position.z][posY][posX] = NULL;
	}
}

void Map::update()
{
	Effect::update();
	if(!m_localCreature) //If somehow we don't have localcreature avoid crashing
		return;

	m_localCreature->update();
}

void Map::render()
{
	if(!m_localCreature) //If somehow we don't have localcreature avoid crashing
		return;

	if(m_needUpdateCache)
		updateCacheMap();

	Sint32 offsetX = -m_localCreature->getOffsetX(true);
	Sint32 offsetY = -m_localCreature->getOffsetY(true);
	if(g_engine.getLightMode() != CLIENT_LIGHT_MODE_NONE)
		g_light.initLightMap(offsetX, offsetY, getCentralPosition().z);

	auto& renderer = g_engine.getRender();
	renderer->beginGameScene();
	for(Sint32 z = m_cachedLastVisibleFloor; z >= m_cachedFirstVisibleFloor; --z)
	{
		if(g_engine.getLightMode() != CLIENT_LIGHT_MODE_NONE && z != m_cachedLastVisibleFloor)
		{
			for(Sint32 y = 0; y < GAME_MAP_HEIGHT; ++y)
			{
				for(Sint32 x = 0; x < GAME_MAP_WIDTH; ++x)
				{
					//Reset only the first visible ground tiles to minimalize cpu usage
					if(m_cachedFirstGrounds[y][x] == z)
						g_light.resetLightSource(x, y);
				}
			}
		}
		Sint32 posY = -32 + offsetY;
		for(Sint32 y = 0; y < GAME_MAP_HEIGHT; ++y)
		{
			Sint32 posX = -32 + offsetX;
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

								Sint32 indexX = x - x2;
								Sint32 indexY = y - y2;
								if(indexX >= 0 && indexY >= 0)
								{
									tile = m_tiles[z][indexY][indexX];
									if(tile)
										tile->render(posX - x2 * 32, posY - y2 * 32, (m_cachedFirstFullGrounds[indexY][indexX] >= z));
								}
							}
						}
					}
					if(g_engine.getLightMode() != CLIENT_LIGHT_MODE_NONE && z == GAME_PLAYER_FLOOR + 1)
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

		Sint32 offsetZ = (z - SDL_static_cast(Sint32, m_centerPosition.z));
		std::vector<DistanceEffect*>& distanceEffects = m_distanceEffects[z];
		for(std::vector<DistanceEffect*>::iterator it = distanceEffects.begin(); it != distanceEffects.end();)
		{
			DistanceEffect* distanceEffect = (*it);
			if(distanceEffect->isDelayed())
			{
				++it;
				continue;
			}

			float flightProgress = distanceEffect->getFlightProgress();
			if(flightProgress > 1.f)
			{
				delete (*it);
				it = distanceEffects.erase(it);
			}
			else
			{
				const Position& fromPos = distanceEffect->getFromPos();
				const Position& toPos = distanceEffect->getToPos();

				Sint32 screenxFrom = ((Position::getOffsetX(fromPos, m_centerPosition) + (MAP_WIDTH_OFFSET - 2) + offsetZ) * 32) + offsetX;
				Sint32 screenyFrom = ((Position::getOffsetY(fromPos, m_centerPosition) + (MAP_HEIGHT_OFFSET - 2) + offsetZ) * 32) + offsetY;

				Sint32 screenxTo = ((Position::getOffsetX(toPos, m_centerPosition) + (MAP_WIDTH_OFFSET - 2) + offsetZ) * 32) + offsetX;
				Sint32 screenyTo = ((Position::getOffsetY(toPos, m_centerPosition) + (MAP_HEIGHT_OFFSET - 2) + offsetZ) * 32) + offsetY;

				Sint32 screenx = screenxFrom + SDL_static_cast(Sint32, (screenxTo - screenxFrom) * flightProgress);
				Sint32 screeny = screenyFrom + SDL_static_cast(Sint32, (screenyTo - screenyFrom) * flightProgress);
				distanceEffect->render(screenx, screeny);
				++it;
			}
		}
	}
	if(g_engine.getLightMode() != CLIENT_LIGHT_MODE_NONE)
	{
		if(g_engine.getLevelSeparator() != 100)
		{
			float brightness = g_engine.getLevelSeparator() / 100.f;
			for(Sint32 y = 0; y < GAME_MAP_HEIGHT; ++y)
			{
				for(Sint32 x = 0; x < GAME_MAP_WIDTH; ++x)
				{
					if(m_cachedFirstGrounds[y][x] > SDL_static_cast(Sint32, getCentralPosition().z))
						g_light.setLightSource(x, y, brightness);
				}
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
		endX = (GAME_MAP_WIDTH - 2);
		endY = (GAME_MAP_HEIGHT - 2);
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
				endX = (GAME_MAP_WIDTH - 1);
				endY = (GAME_MAP_HEIGHT - 2);
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
				endX = (GAME_MAP_WIDTH - 2);
				endY = (GAME_MAP_HEIGHT - 1);
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
				endX = (GAME_MAP_WIDTH - 3);
				endY = (GAME_MAP_HEIGHT - 2);
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
				endX = (GAME_MAP_WIDTH - 3);
				endY = (GAME_MAP_HEIGHT - 1);
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
				endX = (GAME_MAP_WIDTH - 1);
				endY = (GAME_MAP_HEIGHT - 1);
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
				endX = (GAME_MAP_WIDTH - 3);
				endY = (GAME_MAP_HEIGHT - 3);
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
				endX = (GAME_MAP_WIDTH - 1);
				endY = (GAME_MAP_HEIGHT - 3);
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
				endX = (GAME_MAP_WIDTH - 2);
				endY = (GAME_MAP_HEIGHT - 3);
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
		endX = (GAME_MAP_WIDTH - 2);
		endY = (GAME_MAP_HEIGHT - 2);
	}

	Sint32 fixedSize = scaledSize / 2;
	Sint32 z = SDL_static_cast(Sint32, m_centerPosition.z);
	Sint32 posY = offsetY + tileOffsetY;
	for(Sint32 y = startY; y < endY; ++y)
	{
		Sint32 posX = offsetX + tileOffsetX + 16;
		for(Sint32 x = startX; x < endX; ++x)
		{
			Tile* tile = m_tiles[z][y][x];
			if(tile)
				tile->renderInformations(px, py, posX, posY, scale, (m_cachedFirstVisibleGround[y][x] >= z));

			posX += 32;
		}
		posY += 32;
	}

	if(g_engine.getLightMode() != CLIENT_LIGHT_MODE_NONE)
	{
		if(g_engine.getLightMode() == CLIENT_LIGHT_MODE_OLD)
			g_engine.getRender()->drawLightMap_old(g_light.getLightMap(), px + SDL_static_cast(Sint32, offsetX * scale), py + SDL_static_cast(Sint32, offsetY * scale), scaledSize, GAME_MAP_WIDTH, GAME_MAP_HEIGHT);
		else
			g_engine.getRender()->drawLightMap_new(g_light.getLightMap(), px + SDL_static_cast(Sint32, offsetX * scale), py + SDL_static_cast(Sint32, offsetY * scale), scaledSize, GAME_MAP_WIDTH, GAME_MAP_HEIGHT);
	}
	
	pw += px;
	ph += py;
	for(std::vector<StaticText*>::iterator it = m_staticTexts.begin(); it != m_staticTexts.end();)
	{
		StaticText* staticText = (*it);
		const Position& position = staticText->getPosition();
		Sint32 offsetZ = (SDL_static_cast(Sint32, position.z) - z);
		Sint32 screenx = ((Position::getOffsetX(position, m_centerPosition) + (MAP_WIDTH_OFFSET - 2) + offsetZ) * 32) + offsetX;
		Sint32 screeny = ((Position::getOffsetY(position, m_centerPosition) + (MAP_HEIGHT_OFFSET - 2) + offsetZ) * 32) + offsetY;
		screenx = SDL_static_cast(Sint32, screenx * scale) + px + fixedSize;
		screeny = SDL_static_cast(Sint32, screeny * scale) + py;
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
		Sint32 screenx = ((Position::getOffsetX(position, m_centerPosition) + (MAP_WIDTH_OFFSET - 2) + offsetZ) * 32) + offsetX;
		Sint32 screeny = ((Position::getOffsetY(position, m_centerPosition) + (MAP_HEIGHT_OFFSET - 2) + offsetZ) * 32) + offsetY;
		screenx = SDL_static_cast(Sint32, screenx * scale) + px + fixedSize;
		screeny = SDL_static_cast(Sint32, screeny * scale) + py;
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
	if(g_engine.hasShowTextualEffects())
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
					Tile* tile = m_tiles[z][GAME_MAP_HEIGHT - 1][x];
					if(tile)
						delete tile;
				}

				for(Sint32 y = GAME_MAP_HEIGHT - 1; --y >= 0;)
				{
					for(Sint32 x = 0; x < GAME_MAP_WIDTH; ++x)
						m_tiles[z][y + 1][x] = m_tiles[z][y][x];
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
					for(Sint32 x = 0; x < GAME_MAP_WIDTH - 1; ++x)
						m_tiles[z][y][x] = m_tiles[z][y][x + 1];
				}

				for(Sint32 y = 0; y < GAME_MAP_HEIGHT; ++y)
				{
					//Reset the tiles that we get later
					m_tiles[z][y][GAME_MAP_WIDTH - 1] = NULL;
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

				for(Sint32 y = 0; y < GAME_MAP_HEIGHT - 1; ++y)
				{
					for(Sint32 x = 0; x < GAME_MAP_WIDTH; ++x)
						m_tiles[z][y][x] = m_tiles[z][y + 1][x];
				}

				for(Sint32 x = 0; x < GAME_MAP_WIDTH; ++x)
				{
					//Reset the tiles that we get later
					m_tiles[z][GAME_MAP_HEIGHT - 1][x] = NULL;
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
					Tile* tile = m_tiles[z][y][GAME_MAP_WIDTH - 1];
					if(tile)
						delete tile;
				}

				for(Sint32 y = 0; y < GAME_MAP_HEIGHT; ++y)
				{
					for(Sint32 x = GAME_MAP_WIDTH - 1; --x >= 0;)
						m_tiles[z][y][x + 1] = m_tiles[z][y][x];
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
			if(!(*it)->isDelayed() && (*it)->getFlightProgress() > 1.f)
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

void Map::removeMagicEffects(Uint8 posZ)
{
	std::vector<DistanceEffect*>& distanceEffects = m_distanceEffects[posZ];
	for(std::vector<DistanceEffect*>::iterator it = distanceEffects.begin(), end = distanceEffects.end(); it != end; ++it)
		delete (*it);

	distanceEffects.clear();
}

void Map::removeMagicEffects(const Position& position, Uint16 effectId)
{
	//QT Client have a bug that it doesn't register properly first created magic effect so it is registered with id 0
	//that's why in some cases this function might produce different results but I don't want to reproduce this bug
	Tile* tile = getTile(position);
	if(tile)
		tile->removeMagicEffects(effectId);
	
	std::vector<DistanceEffect*>& distanceEffects = m_distanceEffects[position.z];
	for(std::vector<DistanceEffect*>::iterator it = distanceEffects.begin(); it != distanceEffects.end();)
	{
		if((*it)->getFromPos() == position && (*it)->getID() == effectId)
		{
			delete (*it);
			it = distanceEffects.erase(it);
		}
		else
			++it;
	}
}

bool Map::checkSightLine(const Position& fromPos, const Position& toPos)
{
	if(fromPos == toPos)
		return true;

	Position start(fromPos.z > toPos.z ? toPos : fromPos);
	Position destination(fromPos.z > toPos.z ? fromPos : toPos);

	const Sint8 mx = (start.x < destination.x ? 1 : start.x == destination.x ? 0 : -1);
	const Sint8 my = (start.y < destination.y ? 1 : start.y == destination.y ? 0 : -1);

	Sint32 A = Position::getOffsetY(destination, start);
	Sint32 B = Position::getOffsetX(start, destination);
	Sint32 C = -(A * destination.x + B * destination.y);
	while(start.x != destination.x || start.y != destination.y)
	{
		Sint32 move_hor = std::abs(A * (start.x + mx) + B * (start.y) + C);
		Sint32 move_ver = std::abs(A * (start.x) + B * (start.y + my) + C);
		Sint32 move_cross = std::abs(A * (start.x + mx) + B * (start.y + my) + C);

		if(start.y != destination.y && (start.x == destination.x || move_hor > move_ver || move_hor > move_cross))
			start.y += my;

		if(start.x != destination.x && (start.y == destination.y || move_ver > move_hor || move_ver > move_cross))
			start.x += mx;

		Tile* tile = getTile(start);
		if(tile && !tile->isLookPossible())
			return false;
	}

	// now we need to perform a jump between floors to see if everything is clear (literally)
	while(start.z != destination.z)
	{
		Tile* tile = getTile(start);
		if(tile && tile->getThingCount() > 0)
			return false;

		start.z++;
	}

	return true;
}

bool Map::isSightClear(const Position& fromPos, const Position& toPos)
{
	// Cast two converging rays and see if either yields a result.
	return checkSightLine(fromPos, toPos) || checkSightLine(toPos, fromPos);
}

PathFind Map::findPath(std::vector<Direction>& directions, const Position& startPos, const Position& endPos)
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
			return PathFind_ReturnImpossible;
	}
	else
	{
		if(g_automap.getSpeed(endPos) == 0)
			return PathFind_ReturnImpossible;
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
	if(Sx > 127 || Sy > 127)
		return PathFind_ReturnTooFar;

	AutomapArea* areas[4];
	areas[0] = g_automap.getArea(startPos.x - 127, startPos.y - 127, startPos.z);
	areas[1] = g_automap.getArea(startPos.x - 127, startPos.y + 127, startPos.z);
	areas[2] = g_automap.getArea(startPos.x + 127, startPos.y + 127, startPos.z);
	areas[3] = g_automap.getArea(startPos.x + 127, startPos.y - 127, startPos.z);

	Sint32 startX = startPos.x - 127;
	Sint32 startY = startPos.y - 127;
	Uint16 areaX = areas[0]->getBasePosition().x + 256;
	Uint16 areaY = areas[0]->getBasePosition().y + 256;

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
			Sint32 posX = x + *neighbors++;
			Sint32 posY = y + *neighbors++;
			if(SDL_static_cast(Uint32, posX - startX) > 254 || SDL_static_cast(Uint32, posY - startY) > 254)
				continue;

			pos.x = SDL_static_cast(Uint16, posX);
			pos.y = SDL_static_cast(Uint16, posY);

			Sint32 speed = 100;
			endTile = getTile(pos);
			if(endTile)
			{
				if(((pos.x != endPos.x || pos.y != endPos.y) && (endTile->getTopCreature() || !endTile->isPathable())) || !endTile->isWalkable())
					continue;

				speed = SDL_static_cast(Sint32, endTile->getGroundSpeed());
			}
			else
			{
				Sint32 index;
				if(pos.x < areaX)
					index = (pos.y < areaY ? 0 : 1);
				else
					index = (pos.y < areaY ? 3 : 2);

				//Not seen tiles should have 250 speed - that's how it working on Tibia
				speed = SDL_static_cast(Sint32, areas[index]->getSpeed(pos.x, pos.y));
				if(speed == 0)
					continue;

				/*if(areas[index]->getColor(pos.x, pos.y) == 0)
				{
					//Make unseen tile cost so high that they should be consider last
					//or should we not because these tiles already have high cost?
					speed *= 100000;
				}*/
			}

			const Sint32 walkFactor = AStarNodes::getMapWalkFactor(n, pos);
			const Sint32 cost = f + (speed * walkFactor);

			const Uint32 posXY = (SDL_static_cast(Uint32, pos.x) << 16) | SDL_static_cast(Uint32, pos.y);
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

		endTile = getTile(pos);
		if(endTile)
		{
			//We can only send the tiles we know
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

Tile* Map::findTile(Sint32 x, Sint32 y, iRect& gameWindow, Sint32 scaledSize, float scale, Creature*& topCreature, bool multifloor)
{
	Tile* bestPossibleTile = NULL;
	if(!m_localCreature) //If somehow we don't have localcreature avoid crashing
		return bestPossibleTile;

	Sint32 xOffset = SDL_static_cast(Sint32, m_localCreature->getOffsetX(true) * scale);
	Sint32 yOffset = SDL_static_cast(Sint32, m_localCreature->getOffsetY(true) * scale);

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

	Sint32 posX = ((x - gameWindow.x1) / scaledSize) + 1;
	Sint32 posY = ((y - gameWindow.y1) / scaledSize) + 1;
	for(Sint32 z = lastVisibleFloor; z <= firstVisibleFloor; ++z)
	{
		if(!topCreature)
		{
			for(Sint32 y2 = 1; y2 >= -1; --y2)
			{
				for(Sint32 x2 = 1; x2 >= -1; --x2)
				{
					Sint32 indexX = posX + x2;
					Sint32 indexY = posY + y2;
					if(indexX >= 0 && indexY >= 0 && indexX < GAME_MAP_WIDTH && indexY < GAME_MAP_HEIGHT)
					{
						Tile* tile = m_tiles[z][indexY][indexX];
						if(tile)
						{
							Sint32 elevationSize = SDL_static_cast(Sint32, tile->getTileElevation() * scale);
							iRect irect = iRect(gameWindow.x1 + ((indexX - 1) * scaledSize) - elevationSize - xOffset, gameWindow.y1 + ((indexY - 1) * scaledSize) - elevationSize - yOffset, scaledSize + elevationSize, scaledSize + elevationSize);
							topCreature = tile->getTopCreature(x, y, irect, scale);
							if(topCreature)
								goto Search_for_Tile;
						}
					}
				}
			}
		}

		Search_for_Tile:
		Sint32 indexX = posX;
		Sint32 indexY = posY;
		if(indexX >= 0 && indexY >= 0 && indexX < GAME_MAP_WIDTH && indexY < GAME_MAP_HEIGHT)
		{
			Tile* tile = m_tiles[z][indexY][indexX];
			if(tile)
			{
				iRect irect = iRect(gameWindow.x1 + ((indexX - 1) * scaledSize) - xOffset, gameWindow.y1 + ((indexY - 1) * scaledSize) - yOffset, scaledSize, scaledSize);
				if(irect.isPointInside(x, y))
				{
					if(tile->isLookingPossible())
					{
						//When we don't have ground let's see if we can get another tile
						bestPossibleTile = tile;
						if(tile->hasground())
							return bestPossibleTile;
					}
				}
			}
		}
	}
	return bestPossibleTile;
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
		m_cachedLastVisibleFloor = posZ + GAME_MAP_AWARERANGE;
	else
		m_cachedLastVisibleFloor = GAME_PLAYER_FLOOR;

	Sint32 firstFloor = 0;
	if(posZ > GAME_PLAYER_FLOOR)
		firstFloor = UTIL_max<Sint32>(posZ - GAME_MAP_AWARERANGE, GAME_PLAYER_FLOOR + 1);

	for(Sint32 x = -1; x <= 1 && firstFloor < posZ; ++x)
	{
		for(Sint32 y = -1; y <= 1 && firstFloor < posZ; ++y)
		{
			Position pos(SDL_static_cast(Uint16, SDL_static_cast(Sint32, m_centerPosition.x) + x), SDL_static_cast(Uint16, SDL_static_cast(Sint32, m_centerPosition.y) + y), m_centerPosition.z);

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
						firstFloor = upperPos.z + 1;
						break;
					}

					//Check tile geometrically above
					tile = getTile(coveredPos);
					if(tile && tile->limitsFloorsView(isLookPossible))
					{
						firstFloor = coveredPos.z + 1;
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
			bool haveFirstVisibleGround = false;
			bool haveFirstGrounds = false;
			bool haveFirstFullGrounds = false;
			for(Sint32 z = m_cachedFirstVisibleFloor; z < m_cachedLastVisibleFloor; ++z)
			{
				Tile* tile = m_tiles[z][y][x];
				if(tile)
				{
					if(!haveFirstVisibleGround && tile->limitsFloorsView(false))
					{
						m_cachedFirstVisibleGround[y][x] = z;
						haveFirstVisibleGround = true;
					}
					if(!haveFirstGrounds && tile->hasground())
					{
						m_cachedFirstGrounds[y][x] = z;
						haveFirstGrounds = true;
					}
					if(!haveFirstFullGrounds && tile->isFullground())
					{
						m_cachedFirstFullGrounds[y][x] = z;
						haveFirstFullGrounds = true;
					}
				}
			}

			if(!haveFirstVisibleGround)
				m_cachedFirstVisibleGround[y][x] = m_cachedLastVisibleFloor;

			if(!haveFirstGrounds)
				m_cachedFirstGrounds[y][x] = m_cachedLastVisibleFloor;

			if(!haveFirstFullGrounds)
				m_cachedFirstFullGrounds[y][x] = m_cachedLastVisibleFloor;
		}
	}
}
