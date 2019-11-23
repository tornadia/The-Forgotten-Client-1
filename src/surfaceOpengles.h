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

#ifndef __FILE_SURFACE_OPENGLES_h_
#define __FILE_SURFACE_OPENGLES_h_

#include "engine.h"

#if defined(SDL_VIDEO_RENDER_OGL_ES)

typedef void (APIENTRY *PFN_OglBindTexture)(unsigned int target, unsigned int texture);
typedef void (APIENTRY *PFN_OglGenTextures)(int n, unsigned int* textures);
typedef void (APIENTRY *PFN_OglDeleteTextures)(int n, unsigned int* textures);
typedef void (APIENTRY *PFN_OglTexImage2D)(unsigned int target, int level, int iformat, unsigned int width, unsigned int height, int border, unsigned int format, unsigned int type, const void* pixels);
typedef void (APIENTRY *PFN_OglTexSubImage2D)(unsigned int target, int level, int xoffset, int yoffset, unsigned int width, unsigned int height, unsigned int format, unsigned int type, const void* pixels);
typedef void (APIENTRY *PFN_OglTexParameteri)(unsigned int target, unsigned int pname, int param);
typedef void (APIENTRY *PFN_OglTexEnvi)(unsigned int target, unsigned int pname, int param);
typedef void (APIENTRY *PFN_OglPixelStorei)(unsigned int pname, int param);
typedef void (APIENTRY *PFN_OglCopyTexSubImage2D)(unsigned int target, int level, int xoffset, int yoffset, int x, int y, int width, int height);
typedef void (APIENTRY *PFN_OglEnable)(unsigned int cap);
typedef void (APIENTRY *PFN_OglDisable)(unsigned int cap);
typedef void (APIENTRY *PFN_OglEnableClientState)(unsigned int cap);
typedef void (APIENTRY *PFN_OglDisableClientState)(unsigned int cap);
typedef void (APIENTRY *PFN_OglDrawArrays)(unsigned int mode, int first, int count);
typedef void (APIENTRY *PFN_OglBlendFunc)(unsigned int sfactor, unsigned int dfactor);
typedef void (APIENTRY *PFN_OglBlendFuncSeparate)(unsigned int sfactorRGB, unsigned int dfactorRGB, unsigned int sfactorAlpha, unsigned int dfactorAlpha);
typedef void (APIENTRY *PFN_OglBlendEquation)(unsigned int mode);
typedef void (APIENTRY *PFN_OglColorMask)(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);
typedef void (APIENTRY *PFN_OglColor4f)(float red, float green, float blue, float alpha);
typedef void (APIENTRY *PFN_OglTexCoordPointer)(int size, unsigned int type, int stride, void* pointer);
typedef void (APIENTRY *PFN_OglVertexPointer)(int size, unsigned int type, int stride, void* pointer);
typedef void (APIENTRY *PFN_OglColorPointer)(int size, unsigned int type, int stride, void* pointer);
typedef void (APIENTRY *PFN_OglReadPixels)(int x, int y, int width, int height, unsigned int format, unsigned int type, void* pixels);
typedef void (APIENTRY *PFN_OglGetIntegerv)(unsigned int pname, int* params);
typedef void (APIENTRY *PFN_OglLoadIdentity)();
typedef void (APIENTRY *PFN_OglMatrixMode)(unsigned int mode);
typedef void (APIENTRY *PFN_OglOrthof)(float left, float right, float bottom, float top, float zNear, float zFar);
typedef void (APIENTRY *PFN_OglViewport)(int x, int y, int width, int height);
typedef void (APIENTRY *PFN_OglScissor)(int x, int y, int width, int height);
typedef unsigned char* (APIENTRY *PFN_OglGetString)(unsigned int name);
typedef unsigned int (APIENTRY *PFN_OglGetError)();
typedef void (APIENTRY *PFN_OglFinish)();

typedef void (APIENTRY *PFN_OglGenFramebuffers)(int n, unsigned int* framebuffers);
typedef void (APIENTRY *PFN_OglDeleteFramebuffers)(int n, const unsigned int* framebuffers);
typedef void (APIENTRY *PFN_OglFramebufferTexture2D)(unsigned int target, unsigned int attachment, unsigned int textarget, unsigned int texture, int level);
typedef void (APIENTRY *PFN_OglBindFramebuffer)(unsigned int target, unsigned int framebuffer);
typedef unsigned int (APIENTRY *PFN_OglCheckFramebufferStatus)(unsigned int target);

struct OpenglESTexture
{
	OpenglESTexture() : m_texture(0), m_framebuffer(0) {}

	unsigned int m_texture;
	unsigned int m_framebuffer;
	Uint32 m_width;
	Uint32 m_height;
	float m_scaleW;
	float m_scaleH;
};

typedef std::unordered_map<Uint32, OpenglESTexture*> U32BGLESTextures;
typedef std::unordered_map<Uint64, OpenglESTexture*> U64BGLESTextures;

class SurfaceOpenglES : public Surface
{
	public:
		SurfaceOpenglES();
		virtual ~SurfaceOpenglES();

		OpenglESTexture* createOpenGLESTexture(Uint32 width, Uint32 height, bool linearSampler, bool frameBuffer = false);
		bool updateTextureData(OpenglESTexture* texture, unsigned char* data);
		void releaseOpenGLESTexture(OpenglESTexture* texture);

		void drawTriangles(std::vector<float>& vertices, std::vector<float>& texCoords);
		void changeBlending(unsigned int sfactorRGB, unsigned int dfactorRGB, unsigned int sfactorAlpha, unsigned int dfactorAlpha);
		void updateViewport();
		void updateRenderer();

		virtual bool isSupported();
		virtual const char* getName() {return "OpenGL ES";}
		virtual const char* getSoftware() {return m_software;}
		virtual const char* getHardware() {return m_hardware;}
		virtual Uint32 getVRAM() {return m_totalVRAM;}

		virtual void init();
		virtual void doResize(Sint32 w, Sint32 h);
		virtual void spriteManagerReset();
		virtual unsigned char* getScreenPixels(Sint32& width, Sint32& height, bool& bgra);

		virtual void beginScene();
		virtual void endScene();

		void copyFromScreen(Sint32 x, Sint32 y, Uint32 w, Uint32 h);
		void drawBackup(Sint32 x, Sint32 y, Sint32 w, Sint32 h);

		bool integer_scaling(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void drawLightMap(LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height);
		virtual void drawGameScene(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void beginGameScene();
		virtual void endGameScene();

		virtual void setClipRect(Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void disableClipRect();
		virtual void drawRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
		virtual void fillRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

		OpenglESTexture* loadPicture(Uint16 pictureId, bool linear);
		virtual void drawFont(Uint16 pictureId, Sint32 x, Sint32 y, const std::string& text, size_t pos, size_t len, Uint8 r, Uint8 g, Uint8 b, Sint32 cX[256], Sint32 cY[256], Sint32 cW[256], Sint32 cH[256]);
		virtual void drawBackground(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void drawPictureRepeat(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void drawPicture(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 x, Sint32 y, Sint32 w, Sint32 h);

		OpenglESTexture* loadSpriteMask(Uint64 tempPos, Uint32 spriteId, Uint32 maskSpriteId, Uint32 outfitColor);
		virtual void drawSprite(Uint32, Sint32, Sint32) {;}
		virtual void drawSprite(Uint32, Sint32, Sint32, Sint32, Sint32, Sint32, Sint32, Sint32, Sint32) {;}
		virtual void drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor);
		virtual void drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Uint32 outfitColor);

		OpenglESTexture* createAutomapTile(Uint32 currentArea);
		void uploadAutomapTile(OpenglESTexture* texture, Uint8 color[256][256]);
		virtual void drawAutomapTile(Uint32 m_currentArea, bool& m_recreate, Uint8 m_color[256][256], Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh);

	protected:
		U32BGLESTextures m_automapTiles;
		U64BGLESTextures m_spriteMasks;
		std::circular_buffer<Uint32> m_automapTilesBuff;
		std::circular_buffer<Uint64> m_spritesMaskIds;

		OpenglESTexture** m_pictures;
		char* m_software;
		char* m_hardware;

		SDL_GLContext m_oglContext;
		OpenglESTexture* m_renderTarget;

		OpenglESTexture* m_gameWindow;
		OpenglESTexture* m_scaled_gameWindow;
		OpenglESTexture* m_backup;

		PFN_OglBindTexture OglBindTexture;
		PFN_OglGenTextures OglGenTextures;
		PFN_OglDeleteTextures OglDeleteTextures;
		PFN_OglTexImage2D OglTexImage2D;
		PFN_OglTexSubImage2D OglTexSubImage2D;
		PFN_OglTexParameteri OglTexParameteri;
		PFN_OglTexEnvi OglTexEnvi;
		PFN_OglPixelStorei OglPixelStorei;
		PFN_OglCopyTexSubImage2D OglCopyTexSubImage2D;
		PFN_OglEnable OglEnable;
		PFN_OglDisable OglDisable;
		PFN_OglEnableClientState OglEnableClientState;
		PFN_OglDisableClientState OglDisableClientState;
		PFN_OglDrawArrays OglDrawArrays;
		PFN_OglBlendFunc OglBlendFunc;
		PFN_OglBlendFuncSeparate OglBlendFuncSeparate;
		PFN_OglBlendEquation OglBlendEquation;
		PFN_OglColorMask OglColorMask;
		PFN_OglColor4f OglColor4f;
		PFN_OglTexCoordPointer OglTexCoordPointer;
		PFN_OglVertexPointer OglVertexPointer;
		PFN_OglColorPointer OglColorPointer;
		PFN_OglReadPixels OglReadPixels;
		PFN_OglGetIntegerv OglGetIntegerv;
		PFN_OglLoadIdentity OglLoadIdentity;
		PFN_OglMatrixMode OglMatrixMode;
		PFN_OglOrthof OglOrthof;
		PFN_OglViewport OglViewport;
		PFN_OglScissor OglScissor;
		PFN_OglGetString OglGetString;
		PFN_OglGetError OglGetError;
		PFN_OglFinish OglFinish;

		PFN_OglGenFramebuffers OglGenFramebuffers;
		PFN_OglDeleteFramebuffers OglDeleteFramebuffers;
		PFN_OglFramebufferTexture2D OglFramebufferTexture2D;
		PFN_OglBindFramebuffer OglBindFramebuffer;
		PFN_OglCheckFramebufferStatus OglCheckFramebufferStatus;

		unsigned int window_framebuffer;

		Sint32 m_maxTextureSize;
		Sint32 m_integer_scaling_width;
		Sint32 m_integer_scaling_height;

		Uint32 m_totalVRAM;
		Uint32 m_spritesCache;

		Sint32 m_viewPortX;
		Sint32 m_viewPortY;
		Sint32 m_viewPortW;
		Sint32 m_viewPortH;

		bool m_useFBO;
		bool m_useNonPower2;
		bool m_useAlphaBlending;
		bool m_useBlendEquation;
};

class SurfaceOpenglESComp : public SurfaceOpenglES
{
	public:
		SurfaceOpenglESComp();
		virtual ~SurfaceOpenglESComp();

		virtual void init();
		virtual void spriteManagerReset();

		OpenglESTexture* loadSprite(Uint32 spriteId);
		virtual void drawSprite(Uint32 spriteId, Sint32 x, Sint32 y);
		virtual void drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh);

	protected:
		OpenglESTexture** m_sprites;
};

class SurfaceOpenglESPerf : public SurfaceOpenglES
{
	public:
		SurfaceOpenglESPerf();
		virtual ~SurfaceOpenglESPerf();

		void generateSpriteAtlases();
		void checkScheduledSprites();

		virtual void init();
		virtual void spriteManagerReset();

		virtual void beginScene();
		virtual void beginGameScene();
		virtual void endGameScene();

		virtual void drawRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
		virtual void fillRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

		bool loadSprite(Uint32 spriteId, OpenglESTexture* texture, Uint32 xoff, Uint32 yoff);
		virtual void drawSprite(Uint32 spriteId, Sint32 x, Sint32 y);
		virtual void drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh);
		virtual void drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor);

	protected:
		std::vector<float> m_gameWindowVertices;
		std::vector<float> m_gameWindowTexCoords;
		std::vector<OpenglESTexture*> m_spritesAtlas;

		OpenglESTexture* m_spriteAtlas;

		Uint32 m_spriteAtlases;
		Uint32 m_spritesPerAtlas;
		Uint32 m_spritesPerModulo;

		bool* m_sprites;
		bool m_scheduleSpriteDraw;
};

#endif
#endif /* __FILE_SURFACE_OPENGLES_h_ */
