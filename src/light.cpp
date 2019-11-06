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

#include "light.h"
#include "engine.h"

LightSystem g_light;

extern Engine g_engine;

LightSystem::LightSystem()
{
	m_light[0] = 0;
	m_light[1] = 0;

	m_lightRed = 255;
	m_lightGreen = 255;
	m_lightBlue = 255;
}

void LightSystem::setGlobalLight(Uint16 intensity, Uint16 color, Uint8 z)
{
	m_light[0] = intensity;
	m_light[1] = color;
	changeFloor(z);
}

void LightSystem::changeFloor(Uint8 z)
{
	if(z <= GAME_PLAYER_FLOOR)
	{
		colorFrom8bit(SDL_static_cast(Uint8, m_light[1]), m_lightRed, m_lightGreen, m_lightBlue);
		float ambientBrightness = (g_engine.getAmbientLight()/200.0f)+1.0f;
		float brightness = (m_light[0]/255.0f);
		Sint32 r = SDL_static_cast(Sint32, (m_lightRed*brightness*ambientBrightness));
		Sint32 g = SDL_static_cast(Sint32, (m_lightGreen*brightness*ambientBrightness));
		Sint32 b = SDL_static_cast(Sint32, (m_lightBlue*brightness*ambientBrightness));
		m_lightRed = (r > 255 ? 255 : SDL_static_cast(Uint8, r));
		m_lightGreen = (g > 255 ? 255 : SDL_static_cast(Uint8, g));
		m_lightBlue = (b > 255 ? 255 : SDL_static_cast(Uint8, b));
	}
	else
	{
		float ambientBrightness = (g_engine.getAmbientLight()/100.0f);
		m_lightRed = SDL_static_cast(Uint8, (40*ambientBrightness));
		m_lightGreen = SDL_static_cast(Uint8, (40*ambientBrightness));
		m_lightBlue = SDL_static_cast(Uint8, (40*ambientBrightness));
	}
}

void LightSystem::addLightSource(Sint32 x, Sint32 y, Uint16 light[2])
{
	Sint32 rad = SDL_static_cast(Sint32, light[0]);
	float radius = SDL_static_cast(float, rad);
	Uint8 r, g, b;
	colorFrom8bit(SDL_static_cast(Uint8, light[1]), r, g, b);

	float brightness = 0.5f+(radius/8.f)*0.5f;
	r = SDL_static_cast(Uint8, UTIL_min<Sint32>(SDL_static_cast(Sint32, r*brightness), 255));
	g = SDL_static_cast(Uint8, UTIL_min<Sint32>(SDL_static_cast(Sint32, g*brightness), 255));
	b = SDL_static_cast(Uint8, UTIL_min<Sint32>(SDL_static_cast(Sint32, b*brightness), 255));

	//Adjust with local creature offset
	x -= m_offsetX;
	y -= m_offsetY;

	Sint32 offsetX = (x%32);
	Sint32 offsetY = (y%32);
	x = (x/32)+1;
	y = (y/32)+1;

	float Xinfluence = offsetX*0.03125f;
	float Yinfluence = offsetY*0.03125f;

	Sint32 x_start = UTIL_max<Sint32>(x-rad, 0);
	Sint32 x_end = UTIL_min<Sint32>(x+rad+(offsetX > 0 ? 1 : 0), GAME_MAP_WIDTH);
	Sint32 y_start = UTIL_max<Sint32>(y-rad, 0);
	Sint32 y_end = UTIL_min<Sint32>(y+rad+(offsetY > 0 ? 1 : 0), GAME_MAP_HEIGHT);
	for(Sint32 j = y_start; j < y_end; ++j)
	{
		float y_res = (j-y)-Yinfluence;
		y_res *= y_res;

		Sint32 index = ((j * GAME_MAP_WIDTH) + x_start);
		for(Sint32 i = x_start; i < x_end; ++i)
		{
			float x_res = (i-x)-Xinfluence;
			float distance = UTIL_sqrtf(SDL_static_cast(float, x_res*x_res + y_res));
			if(distance <= radius)
			{
				float influence = 1.0f-(distance/radius);
				m_lightMap[index].r = UTIL_max<Sint32>(m_lightMap[index].r, SDL_static_cast(Sint32, r*influence));
				m_lightMap[index].g = UTIL_max<Sint32>(m_lightMap[index].g, SDL_static_cast(Sint32, g*influence));
				m_lightMap[index].b = UTIL_max<Sint32>(m_lightMap[index].b, SDL_static_cast(Sint32, b*influence));
			}
			++index;
		}
	}
}

void LightSystem::resetLightSource(Sint32 x, Sint32 y)
{
	Sint32 index = ((y * GAME_MAP_WIDTH) + x);
	m_lightMap[index].r = SDL_static_cast(Sint32, m_lightRed);
	m_lightMap[index].g = SDL_static_cast(Sint32, m_lightGreen);
	m_lightMap[index].b = SDL_static_cast(Sint32, m_lightBlue);
}

void LightSystem::initLightMap(Sint32 offsetX, Sint32 offsetY)
{
	Sint32 R = SDL_static_cast(Sint32, m_lightRed);
	Sint32 G = SDL_static_cast(Sint32, m_lightGreen);
	Sint32 B = SDL_static_cast(Sint32, m_lightBlue);
	for(Sint32 i = 0; i < GAME_MAP_WIDTH*GAME_MAP_HEIGHT; ++i)
	{
		m_lightMap[i].r = R;
		m_lightMap[i].g = G;
		m_lightMap[i].b = B;
	}
	m_offsetX = offsetX;
	m_offsetY = offsetY;
}
