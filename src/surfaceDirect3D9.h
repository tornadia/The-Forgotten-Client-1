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

#ifndef __FILE_SURFACE_DIRECT3D9_h_
#define __FILE_SURFACE_DIRECT3D9_h_

#include "engine.h"

#if defined(SDL_VIDEO_RENDER_D3D)

struct VertexD3D9
{
	VertexD3D9(float x, float y, float u, float v, DWORD color) : x(x), y(y), z(0.0), color(color), u(u), v(v) {}
	VertexD3D9(float x, float y, DWORD color) : x(x), y(y), z(0.0), color(color), u(0.0f), v(0.0f) {}

	float x, y, z;
	DWORD color;
	float u, v;
};

typedef struct IDirect3DTexture9 IDirect3DTexture9;
struct Direct3D9Texture
{
	Direct3D9Texture() : m_texture(NULL) {}

	IDirect3DTexture9* m_texture;
	Uint32 m_width;
	Uint32 m_height;
	float m_scaleW;
	float m_scaleH;
	bool m_linearSample;
};

struct Direct3D9SpriteData
{
	Direct3D9SpriteData() : m_xOffset(0), m_yOffset(0), m_surface(0), m_lastUsage(0) {}

	Uint32 m_xOffset;
	Uint32 m_yOffset;
	Uint32 m_surface;
	Uint32 m_lastUsage;
};

typedef std::unordered_map<Uint32, Direct3D9Texture*> U32BD3D9Textures;
typedef std::unordered_map<Uint64, Direct3D9SpriteData> U64BD3D9Textures;

typedef struct IDirect3D9 IDirect3D9;
typedef struct IDirect3D9Ex IDirect3D9Ex;
typedef struct IDirect3DDevice9Ex IDirect3DDevice9Ex;
typedef struct IDirect3DSurface9 IDirect3DSurface9;
typedef struct IDirect3DPixelShader9 IDirect3DPixelShader9;
class SurfaceDirect3D9 : public Surface
{
	public:
		SurfaceDirect3D9();
		virtual ~SurfaceDirect3D9();

		Direct3D9Texture* createDirect3DTexture(Uint32 width, Uint32 height, bool linearSampler, bool frameBuffer = false);
		bool updateTextureData(Direct3D9Texture* texture, unsigned char* data);
		void releaseDirect3DTexture(Direct3D9Texture* texture);
		void updateTextureScaling(Direct3D9Texture* texture);
		void drawTriangles(std::vector<VertexD3D9>& vertices);

		void initRenderer();
		void resetRenderer();
		void updateRenderer();
		void updateViewport();

		virtual bool isSupported();
		virtual const char* getName() {return "Direct3D9";}
		virtual const char* getSoftware() {return m_software;}
		virtual const char* getHardware() {return m_hardware;}
		virtual Uint32 getVRAM() {return m_totalVRAM;}

		void generateSpriteAtlases();
		void checkScheduledSprites();

		void renderTargetsRecreate();
		virtual void init();
		virtual void doResize(Sint32 w, Sint32 h);
		virtual void spriteManagerReset();
		void releaseTextures();
		virtual unsigned char* getScreenPixels(Sint32& width, Sint32& height, bool& bgra);

		virtual void beginScene();
		virtual void endScene();

		bool integer_scaling(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void drawLightMap_old(LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height);
		virtual void drawLightMap_new(LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height);
		virtual void drawGameScene(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void beginGameScene();
		virtual void endGameScene();

		virtual void setClipRect(Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void disableClipRect();
		virtual void drawRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
		virtual void fillRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

		Direct3D9Texture* loadPicture(Uint16 pictureId, bool linear);
		virtual void drawFont(Uint16 pictureId, Sint32 x, Sint32 y, const std::string& text, size_t pos, size_t len, Uint8 r, Uint8 g, Uint8 b, Sint32 cX[256], Sint32 cY[256], Sint32 cW[256], Sint32 cH[256]);
		virtual void drawBackground(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void drawPictureRepeat(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void drawPicture(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 x, Sint32 y, Sint32 w, Sint32 h);

		bool loadSprite(Uint32 spriteId, Direct3D9Texture* texture, Uint32 xoff, Uint32 yoff);
		bool loadSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Uint32 outfitColor, Direct3D9Texture* texture, Uint32 xoff, Uint32 yoff);
		virtual void drawSprite(Uint32 spriteId, Sint32 x, Sint32 y);
		virtual void drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh);
		virtual void drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor);
		virtual void drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Uint32 outfitColor);

		Direct3D9Texture* createAutomapTile(Uint32 currentArea);
		void uploadAutomapTile(Direct3D9Texture* texture, Uint8 color[256][256]);
		virtual void drawAutomapTile(Uint32 m_currentArea, bool& m_recreate, Uint8 m_color[256][256], Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh);

	protected:
		std::vector<VertexD3D9> m_vertices;
		std::vector<Direct3D9Texture*> m_spritesAtlas;
		U32BD3D9Textures m_automapTiles;
		U64BD3D9Textures m_sprites;
		std::circular_buffer<Uint32> m_automapTilesBuff;
		std::circular_buffer<Uint64> m_spritesIds;

		Direct3D9Texture* m_spriteAtlas;
		Direct3D9Texture** m_pictures;
		char* m_software;
		char* m_hardware;

		Direct3D9Texture* m_gameWindow;
		Direct3D9Texture* m_scaled_gameWindow;

		void* m_d3d9Handle;
		IDirect3D9* m_d3d9;
		IDirect3D9Ex* m_d3d9Ex;
		IDirect3DDevice9* m_device;
		IDirect3DDevice9Ex* m_deviceEx;

		IDirect3DSurface9* m_defaultRenderTarget;
		IDirect3DSurface9* m_currentRenderTarget;

		IDirect3DPixelShader9* m_pixelShader_sharpen;

		Sint32 m_maxTextureSize;
		Sint32 m_integer_scaling_width;
		Sint32 m_integer_scaling_height;

		Sint32 m_adapter;
		Uint32 m_viewPortX;
		Uint32 m_viewPortY;
		Uint32 m_viewPortW;
		Uint32 m_viewPortH;

		Uint32 m_totalVRAM;
		Uint32 m_spriteChecker;
		Uint32 m_currentFrame;

		Uint32 m_spriteAtlases;
		Uint32 m_spritesPerAtlas;
		Uint32 m_spritesPerModulo;

		bool m_usingLinearSample;
		bool m_useAlphaBlending;
		bool m_needReset;
		bool m_haveSharpening;
		bool m_scheduleSpriteDraw;
};

static const DWORD D3D9_sharpen[] =
{
	0xffff0200, 0x002ffffe, 0x42415443, 0x0000001c, 0x0000008f, 0xffff0200,
	0x00000002, 0x0000001c, 0x00008100, 0x00000088, 0x00000044, 0x00000002,
	0x00020001, 0x00000050, 0x00000000, 0x00000060, 0x00000003, 0x00000001,
	0x00000078, 0x00000000, 0x74786574, 0x53657275, 0x00657a69, 0x00030001,
	0x00020001, 0x00000001, 0x00000000, 0x53656874, 0x6c706d61, 0x742b7265,
	0x65546568, 0x72757478, 0xabab0065, 0x00070004, 0x00040001, 0x00000001,
	0x00000000, 0x325f7370, 0x4d00305f, 0x6f726369, 0x74666f73, 0x29522820,
	0x534c4820, 0x6853204c, 0x72656461, 0x6d6f4320, 0x656c6970, 0x30312072,
	0xab00312e, 0x05000051, 0xa00f0001, 0x3e800000, 0x3f800000, 0x3dcccccd,
	0xbd4ccccd, 0x05000051, 0xa00f0002, 0x40881062, 0x4164dd2f, 0x3fb8d4fe,
	0x3f000000, 0x0200001f, 0x80000000, 0xb0030000, 0x0200001f, 0x90000000,
	0xa00f0800, 0x03000002, 0x80010000, 0xb0000000, 0xa0000000, 0x03000002,
	0x80020000, 0xb0550000, 0xa1550000, 0x03000002, 0x80030001, 0xb0e40000,
	0xa1e40000, 0x03000002, 0x80030002, 0xb0e40000, 0xa0e40000, 0x02000001,
	0x80010003, 0xa1000000, 0x02000001, 0x80020003, 0xa0550000, 0x03000002,
	0x80030003, 0x80e40003, 0xb0e40000, 0x03000042, 0x800f0000, 0x80e40000,
	0xa0e40800, 0x03000042, 0x800f0001, 0x80e40001, 0xa0e40800, 0x03000042,
	0x800f0002, 0x80e40002, 0xa0e40800, 0x03000042, 0x800f0003, 0x80e40003,
	0xa0e40800, 0x03000042, 0x800f0004, 0xb0e40000, 0xa0e40800, 0x03000002,
	0x80070000, 0x80e40000, 0x80e40001, 0x03000002, 0x80070000, 0x80e40002,
	0x80e40000, 0x03000002, 0x80070000, 0x80e40003, 0x80e40000, 0x04000004,
	0x80070000, 0x80e40000, 0xa1000001, 0x80e40004, 0x02000001, 0x80080000,
	0xa0550001, 0x03000009, 0x80180004, 0x80e40000, 0xa0e40002, 0x04000004,
	0x80080004, 0x80ff0004, 0xa0aa0001, 0xa0ff0001, 0x03000002, 0x80170000,
	0x80ff0004, 0x80e40004, 0x02000001, 0x80080000, 0xa0550001, 0x02000001,
	0x800f0800, 0x80e40000, 0x0000ffff
};

#endif
#endif /* __FILE_SURFACE_DIRECT3D9_h_ */
