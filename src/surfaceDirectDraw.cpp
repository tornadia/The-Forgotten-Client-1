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

#include "surfaceDirectDraw.h"

//TODO: use flip model presentation for rendering in fullscreen
//optimize texture updating

#if defined(SDL_VIDEO_RENDER_DDRAW)
#include <ddraw.h>
#include <d3d.h>
#include <SDL2/SDL_syswm.h>
#include "map.h"
#include "creature.h"

#define DDRAW_FVF (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

extern Map g_map;
extern Engine g_engine;
extern Uint32 g_spriteCounts;
extern Uint16 g_pictureCounts;

typedef HRESULT (WINAPI *PFN_DDrawCreate)(GUID* lpGUID, void** lplpDD, REFIID iid, IUnknown* pUnkOuter);

SurfaceDirectDraw::SurfaceDirectDraw() : m_automapTilesBuff(HARDWARE_MAX_AUTOMAPTILES), m_spritesMaskIds(HARDWARE_MAX_SPRITEMASKS)
{
	g_engine.RecreateWindow(false);

	m_ddraw7 = NULL;
	m_renderer = NULL;
	m_device = NULL;
	m_primarybuffer = NULL;
	m_backbuffer = NULL;
	m_clipper = NULL;

	m_directDrawHandle = SDL_LoadObject("DDRAW.DLL");
	if(m_directDrawHandle)
	{
		PFN_DDrawCreate DDrawCreate;
		DDrawCreate = SDL_reinterpret_cast(PFN_DDrawCreate, SDL_LoadFunction(m_directDrawHandle, "DirectDrawCreateEx"));
		if(DDrawCreate)
		{
			HRESULT hr = DDrawCreate(NULL, SDL_reinterpret_cast(void**, &m_ddraw7), IID_IDirectDraw7, NULL);
			if(SUCCEEDED(hr))
				m_ddraw7->QueryInterface(IID_IDirect3D7, (VOID**)&m_renderer);
		}
	}
	
	m_gameWindow = NULL;
	m_scaled_gameWindow = NULL;

	m_pictures = NULL;
	m_hardware = NULL;

	m_maxTextureSize = 1024;
	m_integer_scaling_width = 0;
	m_integer_scaling_height = 0;

	m_clipRectX = 0;
	m_clipRectY = 0;
	m_clipRectW = 0;
	m_clipRectH = 0;
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = 0;
	m_viewPortH = 0;

	m_totalVRAM = 0;
	m_spritesCache = 0;

	m_px = 0;
	m_py = 0;

	m_usingLinearSample = false;
	m_needReset = true;
	m_fullscreen = false;

	#ifdef HAVE_CXX11_SUPPORT
	m_spriteMasks.reserve(HARDWARE_MAX_SPRITEMASKS);
	m_automapTiles.reserve(HARDWARE_MAX_AUTOMAPTILES);
	#endif 
}

SurfaceDirectDraw::~SurfaceDirectDraw()
{
	if(m_hardware)
		SDL_free(m_hardware);

	for(std::vector<DirectDrawTexture*>::iterator it = m_texturesToDelete.begin(), end = m_texturesToDelete.end(); it != end; ++it)
		releaseDirectDrawTexture((*it));

	if(m_pictures)
	{
		for(Uint16 i = 0; i < g_pictureCounts; ++i)
		{
			if(m_pictures[i])
				releaseDirectDrawTexture(m_pictures[i]);
		}

		SDL_free(m_pictures);
	}

	for(U32BDDRAWTextures::iterator it = m_automapTiles.begin(), end = m_automapTiles.end(); it != end; ++it)
		releaseDirectDrawTexture(it->second);

	for(U64BDDRAWTextures::iterator it = m_spriteMasks.begin(), end = m_spriteMasks.end(); it != end; ++it)
		releaseDirectDrawTexture(it->second);

	m_spriteMasks.clear();
	m_spritesMaskIds.clear();
	m_automapTilesBuff.clear();
	m_automapTiles.clear();
	if(m_scaled_gameWindow)
		releaseDirectDrawTexture(m_scaled_gameWindow);

	if(m_gameWindow)
		releaseDirectDrawTexture(m_gameWindow);

	if(m_backbuffer)
		IDirectDrawSurface7_Release(m_backbuffer);

	if(m_primarybuffer)
		IDirectDrawSurface7_Release(m_primarybuffer);

	if(m_clipper)
		IDirectDrawClipper_Release(m_clipper);

	if(m_device)
		IDirect3DDevice7_Release(m_device);

	if(m_renderer)
		IDirect3D7_Release(m_renderer);

	if(m_ddraw7)
		IDirectDraw7_Release(m_ddraw7);

	if(m_directDrawHandle)
		SDL_UnloadObject(m_directDrawHandle);
}

DirectDrawTexture* SurfaceDirectDraw::createDirectDrawTexture(Uint32 width, Uint32 height, bool linearSampler, bool frameBuffer)
{
	IDirectDrawSurface7* dd_surface;
	DDSURFACEDESC2 ddsd;
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = (DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT);
	ddsd.dwWidth = width;
	ddsd.dwHeight = height;
	if(frameBuffer)
	{
		ddsd.ddsCaps.dwCaps = (DDSCAPS_TEXTURE|DDSCAPS_VIDEOMEMORY|DDSCAPS_3DDEVICE);
		ddsd.ddsCaps.dwCaps2 = DDSCAPS2_HINTSTATIC;
	}
	else
	{
		ddsd.ddsCaps.dwCaps = (DDSCAPS_TEXTURE|DDSCAPS_VIDEOMEMORY);
		ddsd.ddsCaps.dwCaps2 = DDSCAPS2_HINTDYNAMIC;
	}
	ddsd.ddpfPixelFormat.dwSize = sizeof(ddsd.ddpfPixelFormat);
	ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB|DDPF_ALPHAPIXELS;
	ddsd.ddpfPixelFormat.dwRGBBitCount = 32;
	ddsd.ddpfPixelFormat.dwRBitMask = 0x00FF0000;
	ddsd.ddpfPixelFormat.dwGBitMask = 0x0000FF00;
	ddsd.ddpfPixelFormat.dwBBitMask = 0x000000FF;
	ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xFF000000;
	HRESULT result = IDirectDraw7_CreateSurface(m_ddraw7, &ddsd, &dd_surface, NULL);
	if(result != DD_OK)
		return NULL;

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	result = IDirectDrawSurface7_Lock(dd_surface, NULL, &ddsd, (DDLOCK_NOSYSLOCK|DDLOCK_WAIT), NULL);
	if(result != DD_OK)
	{
		IDirectDrawSurface7_Release(dd_surface);
		return NULL;
	}
	IDirectDrawSurface7_Unlock(dd_surface, NULL);

	DirectDrawTexture* texture = new DirectDrawTexture();
	if(!texture)
	{
		IDirectDrawSurface7_Release(dd_surface);
		return NULL;
	}
	texture->m_texture = dd_surface;
	texture->m_width = width;
	texture->m_height = height;
	texture->m_scaleW = 1.0f/width;
	texture->m_scaleH = 1.0f/height;
	texture->m_linearSample = linearSampler;
	return texture;
}

bool SurfaceDirectDraw::updateTextureData(DirectDrawTexture* texture, unsigned char* data)
{
	DDSURFACEDESC2 ddsd;
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	HRESULT result = IDirectDrawSurface7_Lock(texture->m_texture, NULL, &ddsd, (DDLOCK_NOSYSLOCK|DDLOCK_WAIT), NULL);
	if(result != DD_OK)
		return false;

	unsigned char* dstData = SDL_reinterpret_cast(unsigned char*, ddsd.lpSurface);
	LONG length = texture->m_width*4;
	if(length == ddsd.lPitch)
		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, dstData), SDL_reinterpret_cast(const Uint8*, data), length*texture->m_height);
	else
	{
		if(length > ddsd.lPitch)
			length = ddsd.lPitch;
		for(Uint32 row = 0; row < texture->m_height; ++row)
		{
			UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, dstData), SDL_reinterpret_cast(const Uint8*, data), length);
			data += length;
			dstData += ddsd.lPitch;
		}
	}
	IDirectDrawSurface7_Unlock(texture->m_texture, NULL);
	return true;
}

void SurfaceDirectDraw::releaseDirectDrawTexture(DirectDrawTexture* texture)
{
	if(texture->m_texture)
		IDirectDrawSurface7_Release(texture->m_texture);
	delete texture;
}

void SurfaceDirectDraw::updateTextureScaling(DirectDrawTexture* texture)
{
	if(m_usingLinearSample != texture->m_linearSample)
	{
		m_usingLinearSample = texture->m_linearSample;
		IDirect3DDevice7_SetTextureStageState(m_device, 0, D3DTSS_MINFILTER, (m_usingLinearSample ? D3DFILTER_LINEAR : D3DFILTER_NEAREST));
		IDirect3DDevice7_SetTextureStageState(m_device, 0, D3DTSS_MAGFILTER, (m_usingLinearSample ? D3DFILTER_LINEAR : D3DFILTER_NEAREST));
		IDirect3DDevice7_SetTextureStageState(m_device, 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		IDirect3DDevice7_SetTextureStageState(m_device, 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
	}
}

void SurfaceDirectDraw::drawTriangles(std::vector<VertexDDRAW>& vertices)
{
	DWORD verticesSize = SDL_static_cast(DWORD, vertices.size());
	if(verticesSize > 2046)
	{
		VertexDDRAW* verticeData = &vertices[0];

		DWORD verticeIndex = 0;
		while(verticeIndex < verticesSize)
		{
			DWORD drawSize = UTIL_min<DWORD>(verticesSize-verticeIndex, 2046);
			IDirect3DDevice7_DrawPrimitive(m_device, D3DPT_TRIANGLELIST, DDRAW_FVF, SDL_reinterpret_cast(void*, &verticeData[verticeIndex]), drawSize, 0);
			verticeIndex += drawSize;
		}
	}
	else if(verticesSize > 0)
		IDirect3DDevice7_DrawPrimitive(m_device, D3DPT_TRIANGLELIST, DDRAW_FVF, &vertices[0], verticesSize, 0);
}

void SurfaceDirectDraw::resetRenderer()
{
	if(m_device)
	{
		IDirect3DDevice7_Release(m_device);
		m_device = NULL;
	}

	if(m_backbuffer)
	{
		IDirectDrawSurface7_Release(m_backbuffer);
		m_backbuffer = NULL;
	}

	if(m_primarybuffer)
	{
		IDirectDrawSurface7_Release(m_primarybuffer);
		m_primarybuffer = NULL;
	}

	SDL_SysWMinfo windowinfo;
	SDL_VERSION(&windowinfo.version);
	SDL_GetWindowWMInfo(g_engine.m_window, &windowinfo);

	Uint32 window_flags = SDL_GetWindowFlags(g_engine.m_window);
	if(window_flags & SDL_WINDOW_FULLSCREEN && (window_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != SDL_WINDOW_FULLSCREEN_DESKTOP)
	{
		HRESULT result = IDirectDraw7_SetCooperativeLevel(m_ddraw7, windowinfo.info.win.window, (DDSCL_FULLSCREEN|DDSCL_EXCLUSIVE|DDSCL_ALLOWREBOOT));
		if(result != DD_OK)
		{
			UTIL_MessageBox(true, "DirectDraw: Failed to set cooperative level.");
			exit(-1);
			return;
		}

		SDL_DisplayMode fullscreen_mode;
		SDL_GetWindowDisplayMode(g_engine.m_window, &fullscreen_mode);
		result = IDirectDraw7_SetDisplayMode(m_ddraw7, fullscreen_mode.w, fullscreen_mode.h, 32, fullscreen_mode.refresh_rate, 0);
		if(result != DD_OK)
		{
			result = IDirectDraw7_SetDisplayMode(m_ddraw7, fullscreen_mode.w, fullscreen_mode.h, 24, fullscreen_mode.refresh_rate, 0);
			if(result != DD_OK)
			{
				result = IDirectDraw7_SetDisplayMode(m_ddraw7, fullscreen_mode.w, fullscreen_mode.h, 32, 0, 0);
				if(result != DD_OK)
				{
					result = IDirectDraw7_SetDisplayMode(m_ddraw7, fullscreen_mode.w, fullscreen_mode.h, 24, 0, 0);
					if(result != DD_OK)
					{
						UTIL_MessageBox(true, "DirectDraw: Failed to setup fullscreen display.");
						exit(-1);
						return;
					}
				}
			}
		}

		m_fullscreen = true;
	}
	else
	{
		HRESULT result = IDirectDraw7_SetCooperativeLevel(m_ddraw7, windowinfo.info.win.window, DDSCL_NORMAL);
		if(result != DD_OK)
		{
			UTIL_MessageBox(true, "DirectDraw: Failed to set cooperative level.");
			exit(-1);
			return;
		}

		m_fullscreen = false;
	}

	DDSURFACEDESC2 ddsd;
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = (DDSCAPS_PRIMARYSURFACE|DDSCAPS_VIDEOMEMORY);
	HRESULT result = IDirectDraw7_CreateSurface(m_ddraw7, &ddsd, &m_primarybuffer, NULL);
	if(result != DD_OK)
	{
		UTIL_MessageBox(true, "DirectDraw: Failed to create frontbuffer.");
		exit(-1);
		return;
	}

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = (DDSD_PIXELFORMAT|DDSD_CAPS);
	result = IDirectDrawSurface7_GetSurfaceDesc(m_primarybuffer, &ddsd);
	if(result != DD_OK || !(ddsd.ddpfPixelFormat.dwFlags & DDPF_RGB) || ddsd.ddpfPixelFormat.dwRGBBitCount < 24)
	{
		UTIL_MessageBox(true, "DirectDraw: Frontbuffer doesn't have atleast 24bit color depth.");
		exit(-1);
		return;
	}

	if(!m_fullscreen)
	{
		if(!m_clipper)
		{
			result = IDirectDraw7_CreateClipper(m_ddraw7, 0, &m_clipper, NULL);
			if(result != DD_OK)
			{
				UTIL_MessageBox(true, "DirectDraw: Failed to create clipper.");
				exit(-1);
				return;
			}
		}
		result = IDirectDrawClipper_SetHWnd(m_clipper, 0, windowinfo.info.win.window);
		if(result != DD_OK)
		{
			UTIL_MessageBox(true, "DirectDraw: Failed to setup clipper.");
			exit(-1);
			return;
		}
		result = IDirectDrawSurface7_SetClipper(m_primarybuffer, m_clipper);
		if(result != DD_OK)
		{
			UTIL_MessageBox(true, "DirectDraw: Failed to setup clipper.");
			exit(-1);
			return;
		}

		int m_windowX, m_windowY;
		SDL_GetWindowPosition(g_engine.m_window, &m_windowX, &m_windowY);

		RECT rcRectDest;
		GetClientRect(windowinfo.info.win.window, &rcRectDest);
		ClientToScreen(windowinfo.info.win.window, reinterpret_cast<POINT*>(&rcRectDest.left));
		m_px = rcRectDest.left-m_windowX;
		m_py = rcRectDest.top-m_windowY;
	}
	else
	{
		m_px = 0;
		m_py = 0;
	}

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = (DDSD_WIDTH|DDSD_HEIGHT|DDSD_CAPS);
	ddsd.ddsCaps.dwCaps = (DDSCAPS_OFFSCREENPLAIN|DDSCAPS_3DDEVICE);
	ddsd.dwWidth = g_engine.getWindowWidth();
	ddsd.dwHeight = g_engine.getWindowHeight();
	result = IDirectDraw7_CreateSurface(m_ddraw7, &ddsd, &m_backbuffer, NULL);
	if(result != DD_OK)
	{
		UTIL_MessageBox(true, "DirectDraw: Failed to create backbuffer.");
		exit(-1);
		return;
	}

	result = IDirect3D7_CreateDevice(m_renderer, IID_IDirect3DTnLHalDevice, m_backbuffer, &m_device);
	if(result != DD_OK)
	{
		result = IDirect3D7_CreateDevice(m_renderer, IID_IDirect3DHALDevice, m_backbuffer, &m_device);
		if(result != DD_OK)
		{
			result = IDirect3D7_CreateDevice(m_renderer, IID_IDirect3DRefDevice, m_backbuffer, &m_device);
			if(result != DD_OK)
			{
				UTIL_MessageBox(true, "DirectDraw: Failed to create DirectDraw device.");
				exit(-1);
				return;
			}
		}
	}

	renderTargetsRecreate();
	initRenderer();
}

void SurfaceDirectDraw::initRenderer()
{
	IDirect3DDevice7_SetRenderState(m_device, D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
	IDirect3DDevice7_SetRenderState(m_device, D3DRENDERSTATE_ZENABLE, D3DZB_FALSE);
	IDirect3DDevice7_SetRenderState(m_device, D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
	IDirect3DDevice7_SetRenderState(m_device, D3DRENDERSTATE_LIGHTING, FALSE);
	IDirect3DDevice7_SetRenderState(m_device, D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE);

	IDirect3DDevice7_SetTextureStageState(m_device, 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	IDirect3DDevice7_SetTextureStageState(m_device, 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	IDirect3DDevice7_SetTextureStageState(m_device, 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	IDirect3DDevice7_SetTextureStageState(m_device, 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	IDirect3DDevice7_SetTextureStageState(m_device, 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	IDirect3DDevice7_SetTextureStageState(m_device, 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	IDirect3DDevice7_SetTextureStageState(m_device, 0, D3DTSS_TEXCOORDINDEX, 0);

	D3DMATRIX matrix;
	matrix._11 = matrix._22 = matrix._33 = matrix._44 = 1.0f;
	matrix._12 = matrix._13 = matrix._14 = matrix._41 = 0.0f;
	matrix._21 = matrix._23 = matrix._24 = matrix._42 = 0.0f;
	matrix._31 = matrix._32 = matrix._34 = matrix._43 = 0.0f;
	IDirect3DDevice7_SetTransform(m_device, D3DTRANSFORMSTATE_WORLD, &matrix);
	IDirect3DDevice7_SetTransform(m_device, D3DTRANSFORMSTATE_VIEW, &matrix);
	IDirect3DDevice7_SetRenderTarget(m_device, m_backbuffer, 0);

	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = SDL_static_cast(Uint32, g_engine.getWindowWidth());
	m_viewPortH = SDL_static_cast(Uint32, g_engine.getWindowHeight());
	m_clipRectX = 0;
	m_clipRectY = 0;
	m_clipRectW = m_viewPortW;
	m_clipRectH = m_viewPortH;

	updateRenderer();
	updateViewport();
}

void SurfaceDirectDraw::updateRenderer()
{
	IDirect3DDevice7_SetTextureStageState(m_device, 0, D3DTSS_MINFILTER, D3DFILTER_NEAREST);
	IDirect3DDevice7_SetTextureStageState(m_device, 0, D3DTSS_MAGFILTER, D3DFILTER_NEAREST);
	IDirect3DDevice7_SetTextureStageState(m_device, 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	IDirect3DDevice7_SetTextureStageState(m_device, 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
	m_usingLinearSample = false;
	
	IDirect3DDevice7_SetRenderState(m_device, D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	IDirect3DDevice7_SetRenderState(m_device, D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	IDirect3DDevice7_SetRenderState(m_device, D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

void SurfaceDirectDraw::updateViewport()
{
	D3DVIEWPORT7 vp = {m_viewPortX, m_viewPortY, m_viewPortW, m_viewPortH, 0.0f, 1.0f};
	IDirect3DDevice7_SetViewport(m_device, &vp);

	D3DMATRIX matrix;
	matrix._11 = 2.0f/m_viewPortW; matrix._12 = 0.0f; matrix._13 = 0.0f; matrix._14 = 0.0f;
	matrix._21 = 0.0f; matrix._22 = -2.0f/m_viewPortH; matrix._23 = 0.0f; matrix._24 = 0.0f;
	matrix._31 = 0.0f; matrix._32 = 0.0f; matrix._33 = 1.0f; matrix._34 = 0.0f;
	matrix._41 = -1.0f; matrix._42 = 1.0f; matrix._43 = 0.0f; matrix._44 = 1.0f;
	IDirect3DDevice7_SetTransform(m_device, D3DTRANSFORMSTATE_PROJECTION, &matrix);
}

bool SurfaceDirectDraw::testClipper(Sint32& x, Sint32& y, Sint32& w, Sint32& h, Sint32& sx, Sint32& sy, Sint32& sw, Sint32& sh)
{
	//We don't have native clipper to set in action so let's write something ourselves
	if(w != sw || h != sh)
	{
		double scaling_w = SDL_static_cast(double, w)/sw;
		double scaling_h = SDL_static_cast(double, h)/sh;
		double clip_x0, clip_y0, clip_x1, clip_y1;
		double src_x0, src_y0, src_x1, src_y1;
		double dst_x0, dst_y0, dst_x1, dst_y1;
		dst_x0 = SDL_static_cast(double, x);
		dst_y0 = SDL_static_cast(double, y);
		dst_x1 = dst_x0+SDL_static_cast(double, w)-1.0;
		dst_y1 = dst_y0+SDL_static_cast(double, h)-1.0;
		src_x0 = SDL_static_cast(double, sx);
		src_y0 = SDL_static_cast(double, sy);
		src_x1 = src_x0+SDL_static_cast(double, sw)-1.0;
		src_y1 = src_y0+SDL_static_cast(double, sh)-1.0;
		clip_x0 = SDL_static_cast(double, m_clipRectX);
		clip_y0 = SDL_static_cast(double, m_clipRectY);
		clip_x1 = SDL_static_cast(double, m_clipRectW);
		clip_y1 = SDL_static_cast(double, m_clipRectH);

		dst_x0 -= clip_x0;
		dst_x1 -= clip_x0;
		dst_y0 -= clip_y0;
		dst_y1 -= clip_y0;
		if(dst_x0 < 0)
		{
			src_x0 -= dst_x0/scaling_w;
			dst_x0 = 0;
		}

		if(dst_x1 >= clip_x1)
		{
			src_x1 -= (dst_x1-clip_x1+1.0)/scaling_w;
			dst_x1 = clip_x1-1.0;
		}

		if(dst_y0 < 0)
		{
			src_y0 -= dst_y0/scaling_h;
			dst_y0 = 0;
		}

		if(dst_y1 >= clip_y1)
		{
			src_y1 -= (dst_y1-clip_y1+1.0)/scaling_h;
			dst_y1 = clip_y1-1.0;
		}

		dst_x0 += clip_x0;
		dst_x1 += clip_x0;
		dst_y0 += clip_y0;
		dst_y1 += clip_y0;

		sx = SDL_static_cast(Sint32, SDL_floor(src_x0+0.5));
		sy = SDL_static_cast(Sint32, SDL_floor(src_y0+0.5));
		sw = SDL_static_cast(Sint32, SDL_floor(src_x1+1.5))-SDL_static_cast(Sint32, SDL_floor(src_x0+0.5));
		sh = SDL_static_cast(Sint32, SDL_floor(src_y1+1.5))-SDL_static_cast(Sint32, SDL_floor(src_y0+0.5));

		x = SDL_static_cast(Sint32, SDL_floor(dst_x0+0.5));
		y = SDL_static_cast(Sint32, SDL_floor(dst_y0+0.5));
		w = SDL_static_cast(Sint32, SDL_floor(dst_x1-dst_x0+1.5));
		h = SDL_static_cast(Sint32, SDL_floor(dst_y1-dst_y0+1.5));
		if(w > 0 && h > 0 && sw > 0 && sh > 0)
			return true;
	}
	else
	{
		Sint32 rw, rh, dx, dy;
		rw = w;
		rh = h;

		dx = m_clipRectX-x;
		if(dx > 0)
		{
			rw -= dx;
			x += dx;
			sx += dx;
		}
		dx = x+rw-m_clipRectX-m_clipRectW;
		if(dx > 0)
			rw -= dx;

		dy = m_clipRectY-y;
		if(dy > 0)
		{
			rh -= dy;
			y += dy;
			sy += dy;
		}
		dy = y+rh-m_clipRectY-m_clipRectH;
		if(dy > 0)
			rh -= dy;

		if(rw > 0 && rh > 0)
		{
			w = sw = rw;
			h = sh = rh;
			return true;
		}
	}
	return false;
}

bool SurfaceDirectDraw::isSupported()
{
	if(!m_renderer || !m_ddraw7)
	{
		UTIL_MessageBox(false, "DirectDraw: Failed to create DirectDraw interface.");
		return false;
	}

	SDL_SysWMinfo windowinfo;
	SDL_VERSION(&windowinfo.version);
	SDL_GetWindowWMInfo(g_engine.m_window, &windowinfo);
	HRESULT result = IDirectDraw7_SetCooperativeLevel(m_ddraw7, windowinfo.info.win.window, DDSCL_NORMAL);
	if(result != DD_OK)
	{
		UTIL_MessageBox(false, "DirectDraw: Failed to set cooperative level.");
		return false;
	}

	DDSURFACEDESC2 ddsd;
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = (DDSCAPS_PRIMARYSURFACE|DDSCAPS_VIDEOMEMORY);
	result = IDirectDraw7_CreateSurface(m_ddraw7, &ddsd, &m_primarybuffer, NULL);
	if(result != DD_OK)
	{
		UTIL_MessageBox(false, "DirectDraw: Failed to create frontbuffer.");
		return false;
	}

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = (DDSD_PIXELFORMAT|DDSD_CAPS);
	result = IDirectDrawSurface7_GetSurfaceDesc(m_primarybuffer, &ddsd);
	if(result != DD_OK || !(ddsd.ddpfPixelFormat.dwFlags & DDPF_RGB) || ddsd.ddpfPixelFormat.dwRGBBitCount < 24)
	{
		UTIL_MessageBox(false, "DirectDraw: Frontbuffer doesn't have atleast 24bit color depth.");
		return false;
	}

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = (DDSD_WIDTH|DDSD_HEIGHT|DDSD_CAPS);
	ddsd.ddsCaps.dwCaps = (DDSCAPS_OFFSCREENPLAIN|DDSCAPS_3DDEVICE);
	ddsd.dwWidth = 32;
	ddsd.dwHeight = 32;
	result = IDirectDraw7_CreateSurface(m_ddraw7, &ddsd, &m_backbuffer, NULL);
	if(result != DD_OK)
	{
		UTIL_MessageBox(false, "DirectDraw: Failed to create backbuffer.");
		return false;
	}
	
	result = IDirect3D7_CreateDevice(m_renderer, IID_IDirect3DTnLHalDevice, m_backbuffer, &m_device);
	if(result != DD_OK)
	{
		result = IDirect3D7_CreateDevice(m_renderer, IID_IDirect3DHALDevice, m_backbuffer, &m_device);
		if(result != DD_OK)
		{
			result = IDirect3D7_CreateDevice(m_renderer, IID_IDirect3DRefDevice, m_backbuffer, &m_device);
			if(result != DD_OK)
			{
				UTIL_MessageBox(false, "DirectDraw: Failed to create DirectDraw device.");
				return false;
			}
		}
	}

	D3DDEVICEDESC7 d3dd;
	if(IDirect3DDevice7_GetCaps(m_device, &d3dd) != DD_OK)
	{
		UTIL_MessageBox(false, "DirectDraw: Failed to get device description.");
		return false;
	}

	m_maxTextureSize = UTIL_min<Sint32>(d3dd.dwMaxTextureWidth, d3dd.dwMaxTextureHeight);
	if(m_maxTextureSize < 1024)
	{
		UTIL_MessageBox(false, "DirectDraw: Your device doesn't meet minimum requirement for texture dimension.");
		return false;
	}
	m_needReset = true;
	DDDEVICEIDENTIFIER2 identifier;
	IDirectDraw7_GetDeviceIdentifier(m_ddraw7, &identifier, 0);
	m_hardware = SDL_strdup(identifier.szDescription);

	DDSCAPS2 capsMemory;
	memset(&capsMemory, 0, sizeof(capsMemory));
	capsMemory.dwCaps = DDSCAPS_TEXTURE;

	DWORD totalMemory;
	DWORD freeMemory;
	IDirectDraw7_GetAvailableVidMem(m_ddraw7, &capsMemory, &totalMemory, &freeMemory);
	m_totalVRAM = UTIL_power_of_2(totalMemory/1048576);
	return true;
}

void SurfaceDirectDraw::renderTargetsRecreate()
{
	if(!m_primarybuffer)
		return;

	if(m_scaled_gameWindow)
	{
		releaseDirectDrawTexture(m_scaled_gameWindow);
		m_scaled_gameWindow = NULL;
	}

	if(m_gameWindow)
	{
		releaseDirectDrawTexture(m_gameWindow);
		m_gameWindow = NULL;
	}

	m_gameWindow = createDirectDrawTexture(RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT, (g_engine.getAntialiasing() == CLIENT_ANTIALIASING_NORMAL), true);
	if(!m_gameWindow)
	{
		UTIL_MessageBox(true, "DirectDraw: Out of video memory.");
		exit(-1);
		return;
	}
}

void SurfaceDirectDraw::init()
{
	m_pictures = SDL_reinterpret_cast(DirectDrawTexture**, SDL_calloc(g_pictureCounts, sizeof(DirectDrawTexture*)));
	if(!m_pictures)
	{
		SDL_OutOfMemory();
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Surface::Init() Failed: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
	}
}

void SurfaceDirectDraw::doResize(Sint32, Sint32)
{
	m_needReset = true;
}

void SurfaceDirectDraw::spriteManagerReset()
{
	for(U32BDDRAWTextures::iterator it = m_automapTiles.begin(), end = m_automapTiles.end(); it != end; ++it)
		releaseDirectDrawTexture(it->second);

	for(U64BDDRAWTextures::iterator it = m_spriteMasks.begin(), end = m_spriteMasks.end(); it != end; ++it)
		releaseDirectDrawTexture(it->second);

	m_spriteMasks.clear();
	m_spritesMaskIds.clear();
	m_automapTilesBuff.clear();
	m_automapTiles.clear();
}

unsigned char* SurfaceDirectDraw::getScreenPixels(Sint32& width, Sint32& height, bool& bgra)
{
	if(m_needReset)
		return NULL;

	width = g_engine.getWindowWidth();
	height = g_engine.getWindowHeight();

	unsigned char* pixels = SDL_reinterpret_cast(unsigned char*, SDL_malloc(width*height*4));
	if(!pixels)
		return NULL;
	DDSURFACEDESC2 ddsd;
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	HRESULT result = IDirectDrawSurface7_Lock(m_backbuffer, NULL, &ddsd, (DDLOCK_NOSYSLOCK|DDLOCK_WAIT), NULL);
	if(result != DD_OK)
	{
		SDL_free(pixels);
		return NULL;
	}
	UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, pixels), SDL_reinterpret_cast(const Uint8*, ddsd.lpSurface), SDL_static_cast(size_t, width*height*4));
	IDirectDrawSurface7_Unlock(m_backbuffer, NULL);
	bgra = true;
	return pixels;
}

void SurfaceDirectDraw::beginScene()
{
	if(m_needReset)
	{
		resetRenderer();
		m_needReset = false;
	}
	if(!m_texturesToDelete.empty())
	{
		for(std::vector<DirectDrawTexture*>::iterator it = m_texturesToDelete.begin(), end = m_texturesToDelete.end(); it != end; ++it)
			releaseDirectDrawTexture((*it));
	}

	IDirect3DDevice7_BeginScene(m_device);
	updateRenderer();
}

void SurfaceDirectDraw::endScene()
{
	IDirect3DDevice7_EndScene(m_device);
	if(g_engine.isVsync())
		IDirectDraw7_WaitForVerticalBlank(m_ddraw7, DDWAITVB_BLOCKEND, NULL);
	if(m_fullscreen)
	{
		RECT rcRect;
		rcRect.left = 0;
		rcRect.top = 0;
		rcRect.right = g_engine.getWindowWidth();
		rcRect.bottom = g_engine.getWindowHeight();
		HRESULT result = IDirectDrawSurface7_Blt(m_primarybuffer, &rcRect, m_backbuffer, &rcRect, DDBLT_WAIT, NULL);
		if(result == DDERR_SURFACELOST)
			m_needReset = true;
	}
	else
	{
		int m_windowX, m_windowY;
		SDL_GetWindowPosition(g_engine.m_window, &m_windowX, &m_windowY);

		RECT rcRectSrc;
		RECT rcRectDest;
		rcRectSrc.left = 0;
		rcRectSrc.top = 0;
		rcRectSrc.right = g_engine.getWindowWidth();
		rcRectSrc.bottom = g_engine.getWindowHeight();
		rcRectDest.left = m_px+m_windowX+rcRectSrc.left;
		rcRectDest.top = m_py+m_windowY+rcRectSrc.top;
		rcRectDest.right = m_px+m_windowX+rcRectSrc.right;
		rcRectDest.bottom = m_py+m_windowY+rcRectSrc.bottom;
		HRESULT result = IDirectDrawSurface7_Blt(m_primarybuffer, &rcRectDest, m_backbuffer, &rcRectSrc, DDBLT_WAIT, NULL);
		if(result == DDERR_SURFACELOST)
			m_needReset = true;
	}
}

bool SurfaceDirectDraw::integer_scaling(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	Sint32 limit = UTIL_min<Sint32>(2048, m_maxTextureSize);
	Sint32 width = RENDERTARGET_WIDTH;
	Sint32 height = RENDERTARGET_HEIGHT;
	UTIL_integer_scale(width, height, w, h, limit, limit);
	if(m_integer_scaling_width != width || m_integer_scaling_height != height || !m_scaled_gameWindow)
	{
		if(m_scaled_gameWindow)
		{
			releaseDirectDrawTexture(m_scaled_gameWindow);
			m_scaled_gameWindow = NULL;
		}

		m_scaled_gameWindow = createDirectDrawTexture(width, height, true, true);
		if(!m_scaled_gameWindow)
			return false;

		m_integer_scaling_width = width;
		m_integer_scaling_height = height;
	}

	IDirect3DDevice7_SetRenderTarget(m_device, m_scaled_gameWindow->m_texture, 0);
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = m_scaled_gameWindow->m_width;
	m_viewPortH = m_scaled_gameWindow->m_height;
	updateViewport();
	
	float minx = 0.0f;
	float maxx = SDL_static_cast(float, width);
	float miny = 0.0f;
	float maxy = SDL_static_cast(float, height);

	float minu = (SDL_static_cast(float, sx)+0.5f)*m_gameWindow->m_scaleW;
	float maxu = (SDL_static_cast(float, sx+sw)+0.5f)*m_gameWindow->m_scaleW;
	float minv = (SDL_static_cast(float, sy)+0.5f)*m_gameWindow->m_scaleH;
	float maxv = (SDL_static_cast(float, sy+sh)+0.5f)*m_gameWindow->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexDDRAW vertices[4];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].z = 0.0f;
	vertices[0].color = texColor;
	vertices[0].u = minu;
	vertices[0].v = minv;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].z = 0.0f;
	vertices[1].color = texColor;
	vertices[1].u = minu;
	vertices[1].v = maxv;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].z = 0.0f;
	vertices[2].color = texColor;
	vertices[2].u = maxu;
	vertices[2].v = minv;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].z = 0.0f;
	vertices[3].color = texColor;
	vertices[3].u = maxu;
	vertices[3].v = maxv;

	updateTextureScaling(m_gameWindow);
	IDirect3DDevice7_SetRenderState(m_device, D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	IDirect3DDevice7_SetTexture(m_device, 0, m_gameWindow->m_texture);
	IDirect3DDevice7_DrawPrimitive(m_device, D3DPT_TRIANGLESTRIP, DDRAW_FVF, vertices, 4, 0);
	IDirect3DDevice7_SetRenderTarget(m_device, m_backbuffer, 0);
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = g_engine.getWindowWidth();
	m_viewPortH = g_engine.getWindowHeight();
	updateViewport();

	minx = SDL_static_cast(float, x);
	maxx = SDL_static_cast(float, x+w);
	miny = SDL_static_cast(float, y);
	maxy = SDL_static_cast(float, y+h);

	minu = 0.5f*m_scaled_gameWindow->m_scaleW;
	maxu = (SDL_static_cast(float, m_scaled_gameWindow->m_width)+0.5f)*m_scaled_gameWindow->m_scaleW;
	minv = 0.5f*m_scaled_gameWindow->m_scaleH;
	maxv = (SDL_static_cast(float, m_scaled_gameWindow->m_height)+0.5f)*m_scaled_gameWindow->m_scaleH;

	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].u = minu;
	vertices[0].v = minv;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].u = minu;
	vertices[1].v = maxv;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].u = maxu;
	vertices[2].v = minv;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].u = maxu;
	vertices[3].v = maxv;

	updateTextureScaling(m_scaled_gameWindow);
	IDirect3DDevice7_SetTexture(m_device, 0, m_scaled_gameWindow->m_texture);
	IDirect3DDevice7_DrawPrimitive(m_device, D3DPT_TRIANGLESTRIP, DDRAW_FVF, vertices, 4, 0);
	IDirect3DDevice7_SetRenderState(m_device, D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	return true;
}

void SurfaceDirectDraw::drawGameScene(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	if(!testClipper(x, y, w, h, sx, sy, sw, sh))
		return;

	Uint8 antialiasing = g_engine.getAntialiasing();
	if(antialiasing == CLIENT_ANTIALIASING_INTEGER)
	{
		if(integer_scaling(sx, sy, sw, sh, x, y, w, h))
			return;
		else
		{
			antialiasing = CLIENT_ANTIALIASING_NORMAL;
			g_engine.setAntialiasing(antialiasing);
		}
	}
	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	float minu = (SDL_static_cast(float, sx)+0.5f)*m_gameWindow->m_scaleW;
	float maxu = (SDL_static_cast(float, sx+sw)+0.5f)*m_gameWindow->m_scaleW;
	float minv = (SDL_static_cast(float, sy)+0.5f)*m_gameWindow->m_scaleH;
	float maxv = (SDL_static_cast(float, sy+sh)+0.5f)*m_gameWindow->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexDDRAW vertices[4];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].z = 0.0f;
	vertices[0].color = texColor;
	vertices[0].u = minu;
	vertices[0].v = minv;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].z = 0.0f;
	vertices[1].color = texColor;
	vertices[1].u = minu;
	vertices[1].v = maxv;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].z = 0.0f;
	vertices[2].color = texColor;
	vertices[2].u = maxu;
	vertices[2].v = minv;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].z = 0.0f;
	vertices[3].color = texColor;
	vertices[3].u = maxu;
	vertices[3].v = maxv;

	updateTextureScaling(m_gameWindow);
	IDirect3DDevice7_SetTexture(m_device, 0, m_gameWindow->m_texture);
	IDirect3DDevice7_SetRenderState(m_device, D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	IDirect3DDevice7_DrawPrimitive(m_device, D3DPT_TRIANGLESTRIP, DDRAW_FVF, vertices, 4, 0);
	IDirect3DDevice7_SetRenderState(m_device, D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
}

void SurfaceDirectDraw::beginGameScene()
{
	IDirect3DDevice7_SetRenderTarget(m_device, m_gameWindow->m_texture, 0);
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = m_gameWindow->m_width;
	m_viewPortH = m_gameWindow->m_height;
	updateViewport();
	fillRectangle(0, 0, RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT, 0x00, 0x00, 0x00, 255);
}

void SurfaceDirectDraw::endGameScene()
{
	IDirect3DDevice7_SetRenderTarget(m_device, m_backbuffer, 0);
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = g_engine.getWindowWidth();
	m_viewPortH = g_engine.getWindowHeight();
	updateViewport();
}

void SurfaceDirectDraw::drawLightMap(LightMap* lightmap, Sint32 x, Sint32 y, Sint32, Sint32 width, Sint32 height)
{
	//Since making the clipper for triangles would be pain the ass we'll render to texture and rerender on screen
	//Not the most optimal choice but should have decent performance
	iRect& gameWindowRect = g_engine.getGameWindowRect();
	x = gameWindowRect.x1;
	y = gameWindowRect.y1;
	Sint32 w = gameWindowRect.x2;
	Sint32 h = gameWindowRect.y2;
	Sint32 sx = 0;
	Sint32 sy = 0;
	Sint32 sw = RENDERTARGET_WIDTH;
	Sint32 sh = RENDERTARGET_HEIGHT;
	if(!testClipper(x, y, w, h, sx, sy, sw, sh))
		return;

	VertexDDRAW* vertices = SDL_reinterpret_cast(VertexDDRAW*, SDL_malloc(sizeof(VertexDDRAW)*(width+1)*2));

	IDirect3DDevice7_SetRenderState(m_device, D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
	IDirect3DDevice7_SetRenderState(m_device, D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);

	IDirect3DDevice7_SetRenderTarget(m_device, m_gameWindow->m_texture, 0);
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = m_gameWindow->m_width;
	m_viewPortH = m_gameWindow->m_height;
	updateViewport();

	IDirect3DDevice7_SetTexture(m_device, 0, SDL_reinterpret_cast(IDirectDrawSurface7*, NULL));

	Sint32 offsetX = -g_map.getLocalCreature()->getOffsetX(true);
	Sint32 offsetY = -g_map.getLocalCreature()->getOffsetY(true);
	Sint32 drawY = -48+offsetY;
	height -= 1;
	for(Sint32 j = -1; j < height; ++j)
	{
		Sint32 offset1 = (j + 1) * width;
		Sint32 offset2 = UTIL_max<Sint32>(j, 0) * width;
		Sint32 drawX = -48+offsetX, verticeCount = 0;
		for(Sint32 k = -1; k < width; ++k)
		{
			Sint32 offset = UTIL_max<Sint32>(k, 0);
			vertices[verticeCount].x = SDL_static_cast(float, drawX);
			vertices[verticeCount].y = SDL_static_cast(float, drawY+32);
			vertices[verticeCount].z = 0.0f;
			vertices[verticeCount].color = MAKE_RGBA_COLOR(lightmap[offset1+offset].r, lightmap[offset1+offset].g, lightmap[offset1+offset].b, 255);
			vertices[verticeCount].u = 0.0f;
			vertices[verticeCount].v = 0.0f;
			++verticeCount;

			vertices[verticeCount].x = SDL_static_cast(float, drawX);
			vertices[verticeCount].y = SDL_static_cast(float, drawY);
			vertices[verticeCount].z = 0.0f;
			vertices[verticeCount].color = MAKE_RGBA_COLOR(lightmap[offset2+offset].r, lightmap[offset2+offset].g, lightmap[offset2+offset].b, 255);
			vertices[verticeCount].u = 0.0f;
			vertices[verticeCount].v = 0.0f;
			++verticeCount;
			drawX += 32;
		}

		drawY += 32;
		IDirect3DDevice7_DrawPrimitive(m_device, D3DPT_TRIANGLESTRIP, DDRAW_FVF, vertices, verticeCount, 0);
	}

	IDirect3DDevice7_SetRenderTarget(m_device, m_backbuffer, 0);
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = g_engine.getWindowWidth();
	m_viewPortH = g_engine.getWindowHeight();
	updateViewport();

	IDirect3DDevice7_SetRenderState(m_device, D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO);
	IDirect3DDevice7_SetRenderState(m_device, D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR);

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	float minu = (SDL_static_cast(float, sx)+0.5f)*m_gameWindow->m_scaleW;
	float maxu = (SDL_static_cast(float, sx+sw)+0.5f)*m_gameWindow->m_scaleW;
	float minv = (SDL_static_cast(float, sy)+0.5f)*m_gameWindow->m_scaleH;
	float maxv = (SDL_static_cast(float, sy+sh)+0.5f)*m_gameWindow->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].z = 0.0f;
	vertices[0].color = texColor;
	vertices[0].u = minu;
	vertices[0].v = minv;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].z = 0.0f;
	vertices[1].color = texColor;
	vertices[1].u = minu;
	vertices[1].v = maxv;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].z = 0.0f;
	vertices[2].color = texColor;
	vertices[2].u = maxu;
	vertices[2].v = minv;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].z = 0.0f;
	vertices[3].color = texColor;
	vertices[3].u = maxu;
	vertices[3].v = maxv;

	updateTextureScaling(m_gameWindow);
	IDirect3DDevice7_SetTexture(m_device, 0, m_gameWindow->m_texture);
	IDirect3DDevice7_DrawPrimitive(m_device, D3DPT_TRIANGLESTRIP, DDRAW_FVF, vertices, 4, 0);

	IDirect3DDevice7_SetRenderState(m_device, D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	IDirect3DDevice7_SetRenderState(m_device, D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	SDL_free(vertices);
}

void SurfaceDirectDraw::setClipRect(Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	SDL_Rect rectSrc = {x, y, w, h};
	SDL_Rect rectDst = {SDL_static_cast(Sint32, m_clipRectX), SDL_static_cast(Sint32, m_clipRectY), SDL_static_cast(Sint32, m_viewPortW), SDL_static_cast(Sint32, m_viewPortH)};
	SDL_Rect rectRes;
	SDL_IntersectRect(&rectSrc, &rectDst, &rectRes);

	m_clipRectX = rectRes.x;
	m_clipRectY = rectRes.y;
	m_clipRectW = rectRes.w;
	m_clipRectH = rectRes.h;
}

void SurfaceDirectDraw::disableClipRect()
{
	m_clipRectX = 0;
	m_clipRectY = 0;
	m_clipRectW = m_viewPortW;
	m_clipRectH = m_viewPortH;
}

void SurfaceDirectDraw::drawRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(!testClipper(x, y, w, h, x, y, w, h))
		return;

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w)-1.0f;
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h)-1.0f;

	DWORD texColor = MAKE_RGBA_COLOR(r, g, b, a);

	VertexDDRAW vertices[5];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].z = 0.0f;
	vertices[0].color = texColor;
	vertices[0].u = 0.0f;
	vertices[0].v = 0.0f;

	vertices[1].x = maxx;
	vertices[1].y = miny;
	vertices[1].z = 0.0f;
	vertices[1].color = texColor;
	vertices[1].u = 0.0f;
	vertices[1].v = 0.0f;

	vertices[2].x = maxx;
	vertices[2].y = maxy;
	vertices[2].z = 0.0f;
	vertices[2].color = texColor;
	vertices[2].u = 0.0f;
	vertices[2].v = 0.0f;

	vertices[3].x = minx;
	vertices[3].y = maxy;
	vertices[3].z = 0.0f;
	vertices[3].color = texColor;
	vertices[3].u = 0.0f;
	vertices[3].v = 0.0f;

	vertices[4].x = minx;
	vertices[4].y = miny;
	vertices[4].z = 0.0f;
	vertices[4].color = texColor;
	vertices[4].u = 0.0f;
	vertices[4].v = 0.0f;

	IDirect3DDevice7_SetTexture(m_device, 0, SDL_reinterpret_cast(IDirectDrawSurface7*, NULL));
	IDirect3DDevice7_DrawPrimitive(m_device, D3DPT_LINESTRIP, DDRAW_FVF, vertices, 5, 0);
}

void SurfaceDirectDraw::fillRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(!testClipper(x, y, w, h, x, y, w, h))
		return;

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	DWORD texColor = MAKE_RGBA_COLOR(r, g, b, a);

	VertexDDRAW vertices[4];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].z = 0.0f;
	vertices[0].color = texColor;
	vertices[0].u = 0.0f;
	vertices[0].v = 0.0f;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].z = 0.0f;
	vertices[1].color = texColor;
	vertices[1].u = 0.0f;
	vertices[1].v = 0.0f;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].z = 0.0f;
	vertices[2].color = texColor;
	vertices[2].u = 0.0f;
	vertices[2].v = 0.0f;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].z = 0.0f;
	vertices[3].color = texColor;
	vertices[3].u = 0.0f;
	vertices[3].v = 0.0f;

	IDirect3DDevice7_SetTexture(m_device, 0, SDL_reinterpret_cast(IDirectDrawSurface7*, NULL));
	IDirect3DDevice7_DrawPrimitive(m_device, D3DPT_TRIANGLESTRIP, DDRAW_FVF, vertices, 4, 0);
}

DirectDrawTexture* SurfaceDirectDraw::loadPicture(DirectDrawTexture* s, Uint16 pictureId, bool linear)
{
	Sint32 width, height;
	unsigned char* pixels = g_engine.LoadPicture(pictureId, true, width, height);
	if(!pixels)
		return NULL;

	if(!s)
	{
		s = createDirectDrawTexture(width, height, linear);
		if(!s)
		{
			SDL_free(pixels);
			return NULL;
		}
		m_pictures[pictureId] = s;
	}

	while(!updateTextureData(s, pixels)) {SDL_Delay(10);}
	SDL_free(pixels);
	return s;
}

void SurfaceDirectDraw::drawFont(Uint16 pictureId, Sint32 x, Sint32 y, const std::string& text, size_t pos, size_t len, Uint8 r, Uint8 g, Uint8 b, Sint32 cX[256], Sint32 cY[256], Sint32 cW[256], Sint32 cH[256])
{
	DirectDrawTexture* tex = m_pictures[pictureId];
	if(!tex)
	{
		tex = loadPicture(NULL, pictureId, false);
		if(!tex)
			return;//load failed
	}

	std::vector<VertexDDRAW> vertices;
	vertices.reserve(1536);

	DWORD texColor = MAKE_RGBA_COLOR(r, g, b, 255);

	Sint32 rx = x, ry = y;
	Uint8 character;
	for(size_t i = pos; i < len; ++i)
	{
		character = SDL_static_cast(Uint8, text[i]);
		switch(character)
		{
			case '\r'://carriage return
				rx = x;
				break;
			case '\n'://new line
				rx = x; ry += cH[0];
				break;
			case 0x20://space - don't draw
				rx += cW[character]+cX[0];
				break;
			case 0x0E://Special case - change rendering color
			{
				if(i+4 < len)//First check if we have the color bytes
				{
					Uint8 red = SDL_static_cast(Uint8, text[i+1]);
					Uint8 green = SDL_static_cast(Uint8, text[i+2]);
					Uint8 blue = SDL_static_cast(Uint8, text[i+3]);
					texColor = MAKE_RGBA_COLOR(red, green, blue, 255);
					i += 3;
				}
			}
			break;
			case 0x0F://Special case - change back standard color
				texColor = MAKE_RGBA_COLOR(r, g, b, 255);
				break;
			default:
			{
				Sint32 x1 = rx;
				Sint32 y1 = ry;
				Sint32 w1 = cW[character];
				Sint32 h1 = cH[character];
				Sint32 sx1 = cX[character];
				Sint32 sy1 = cY[character];
				if(testClipper(x1, y1, w1, h1, sx1, sy1, w1, h1))
				{
					float minx = SDL_static_cast(float, x1);
					float maxx = SDL_static_cast(float, x1+w1);
					float miny = SDL_static_cast(float, y1);
					float maxy = SDL_static_cast(float, y1+h1);

					float minu = (SDL_static_cast(float, sx1)+0.5f)*tex->m_scaleW;
					float maxu = (SDL_static_cast(float, sx1+w1)+0.5f)*tex->m_scaleW;
					float minv = (SDL_static_cast(float, sy1)+0.5f)*tex->m_scaleH;
					float maxv = (SDL_static_cast(float, sy1+h1)+0.5f)*tex->m_scaleH;

					VertexDDRAW vertice[4];
					vertice[0].x = minx;
					vertice[0].y = miny;
					vertice[0].z = 0.0f;
					vertice[0].color = texColor;
					vertice[0].u = minu;
					vertice[0].v = minv;

					vertice[1].x = maxx;
					vertice[1].y = miny;
					vertice[1].z = 0.0f;
					vertice[1].color = texColor;
					vertice[1].u = maxu;
					vertice[1].v = minv;

					vertice[2].x = minx;
					vertice[2].y = maxy;
					vertice[2].z = 0.0f;
					vertice[2].color = texColor;
					vertice[2].u = minu;
					vertice[2].v = maxv;

					vertice[3].x = maxx;
					vertice[3].y = maxy;
					vertice[3].z = 0.0f;
					vertice[3].color = texColor;
					vertice[3].u = maxu;
					vertice[3].v = maxv;

					vertices.push_back(vertice[0]);
					vertices.push_back(vertice[1]);
					vertices.push_back(vertice[2]);
					vertices.push_back(vertice[3]);
					vertices.push_back(vertice[2]);
					vertices.push_back(vertice[1]);
				}
				rx += cW[character]+cX[0];
			}
			break;
		}
	}

	updateTextureScaling(tex);
	IDirect3DDevice7_SetTexture(m_device, 0, tex->m_texture);
	drawTriangles(vertices);
}

void SurfaceDirectDraw::drawBackground(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	if(!testClipper(x, y, w, h, sx, sy, sw, sh))
		return;

	DirectDrawTexture* tex = m_pictures[pictureId];
	if(!tex)
	{
		tex = loadPicture(NULL, pictureId, g_engine.hasAntialiasing());
		if(!tex)
			return;//load failed
	}

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	float minu = (SDL_static_cast(float, sx)+0.5f)*tex->m_scaleW;
	float maxu = (SDL_static_cast(float, sx+sw)+0.5f)*tex->m_scaleW;
	float minv = (SDL_static_cast(float, sy)+0.5f)*tex->m_scaleH;
	float maxv = (SDL_static_cast(float, sy+sh)+0.5f)*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexDDRAW vertices[4];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].z = 0.0f;
	vertices[0].color = texColor;
	vertices[0].u = minu;
	vertices[0].v = minv;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].z = 0.0f;
	vertices[1].color = texColor;
	vertices[1].u = minu;
	vertices[1].v = maxv;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].z = 0.0f;
	vertices[2].color = texColor;
	vertices[2].u = maxu;
	vertices[2].v = minv;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].z = 0.0f;
	vertices[3].color = texColor;
	vertices[3].u = maxu;
	vertices[3].v = maxv;

	updateTextureScaling(tex);
	IDirect3DDevice7_SetTexture(m_device, 0, tex->m_texture);
	IDirect3DDevice7_SetRenderState(m_device, D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	IDirect3DDevice7_DrawPrimitive(m_device, D3DPT_TRIANGLESTRIP, DDRAW_FVF, vertices, 4, 0);
	IDirect3DDevice7_SetRenderState(m_device, D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
}

void SurfaceDirectDraw::drawPictureRepeat(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	DirectDrawTexture* tex = m_pictures[pictureId];
	if(!tex)
	{
		tex = loadPicture(NULL, pictureId, false);
		if(!tex)
			return;//load failed
	}

	std::vector<VertexDDRAW> vertices;
	vertices.reserve(1536);

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	Sint32 curW, curH, cx;
	for(Sint32 j = h; j > 0; j -= sh)
	{
		curH = (j > sh ? sh : j);
		cx = x;
		for(Sint32 k = w; k > 0; k -= sw)
		{
			curW = (k > sw ? sw : k);
			Sint32 x1 = cx;
			Sint32 y1 = y;
			Sint32 w1 = curW;
			Sint32 h1 = curH;
			Sint32 sx1 = sx;
			Sint32 sy1 = sy;
			if(testClipper(x1, y1, w1, h1, sx1, sy1, w1, h1))
			{
				float minx = SDL_static_cast(float, x1);
				float maxx = SDL_static_cast(float, x1+w1);
				float miny = SDL_static_cast(float, y1);
				float maxy = SDL_static_cast(float, y1+h1);

				float minu = (SDL_static_cast(float, sx1)+0.5f)*tex->m_scaleW;
				float maxu = (SDL_static_cast(float, sx1+w1)+0.5f)*tex->m_scaleW;
				float minv = (SDL_static_cast(float, sy1)+0.5f)*tex->m_scaleH;
				float maxv = (SDL_static_cast(float, sy1+h1)+0.5f)*tex->m_scaleH;

				VertexDDRAW vertice[4];
				vertice[0].x = minx;
				vertice[0].y = miny;
				vertice[0].z = 0.0f;
				vertice[0].color = texColor;
				vertice[0].u = minu;
				vertice[0].v = minv;

				vertice[1].x = maxx;
				vertice[1].y = miny;
				vertice[1].z = 0.0f;
				vertice[1].color = texColor;
				vertice[1].u = maxu;
				vertice[1].v = minv;

				vertice[2].x = minx;
				vertice[2].y = maxy;
				vertice[2].z = 0.0f;
				vertice[2].color = texColor;
				vertice[2].u = minu;
				vertice[2].v = maxv;

				vertice[3].x = maxx;
				vertice[3].y = maxy;
				vertice[3].z = 0.0f;
				vertice[3].color = texColor;
				vertice[3].u = maxu;
				vertice[3].v = maxv;

				vertices.push_back(vertice[0]);
				vertices.push_back(vertice[1]);
				vertices.push_back(vertice[2]);
				vertices.push_back(vertice[3]);
				vertices.push_back(vertice[2]);
				vertices.push_back(vertice[1]);
			}
			cx += sw;
		}
		y += sh;
	}

	updateTextureScaling(tex);
	IDirect3DDevice7_SetTexture(m_device, 0, tex->m_texture);
	drawTriangles(vertices);
}

void SurfaceDirectDraw::drawPicture(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	if(!testClipper(x, y, w, h, sx, sy, w, h))
		return;

	DirectDrawTexture* tex = m_pictures[pictureId];
	if(!tex)
	{
		tex = loadPicture(NULL, pictureId, false);
		if(!tex)
			return;//load failed
	}

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	float minu = (SDL_static_cast(float, sx)+0.5f)*tex->m_scaleW;
	float maxu = (SDL_static_cast(float, sx+w)+0.5f)*tex->m_scaleW;
	float minv = (SDL_static_cast(float, sy)+0.5f)*tex->m_scaleH;
	float maxv = (SDL_static_cast(float, sy+h)+0.5f)*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexDDRAW vertices[4];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].z = 0.0f;
	vertices[0].color = texColor;
	vertices[0].u = minu;
	vertices[0].v = minv;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].z = 0.0f;
	vertices[1].color = texColor;
	vertices[1].u = minu;
	vertices[1].v = maxv;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].z = 0.0f;
	vertices[2].color = texColor;
	vertices[2].u = maxu;
	vertices[2].v = minv;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].z = 0.0f;
	vertices[3].color = texColor;
	vertices[3].u = maxu;
	vertices[3].v = maxv;

	updateTextureScaling(tex);
	IDirect3DDevice7_SetTexture(m_device, 0, tex->m_texture);
	IDirect3DDevice7_DrawPrimitive(m_device, D3DPT_TRIANGLESTRIP, DDRAW_FVF, vertices, 4, 0);
}

DirectDrawTexture* SurfaceDirectDraw::loadSpriteMask(DirectDrawTexture* s, Uint64 tempPos, Uint32 spriteId, Uint32 maskSpriteId, Uint32 outfitColor)
{
	unsigned char* pixels = g_engine.LoadSpriteMask(spriteId, maskSpriteId, outfitColor, true);
	if(!pixels)
		return NULL;

	if(!s)
	{
		if(m_spriteMasks.size() >= HARDWARE_MAX_SPRITEMASKS)
		{
			U64BDDRAWTextures::iterator it = m_spriteMasks.find(m_spritesMaskIds.front());
			if(it == m_spriteMasks.end())//Something weird happen - let's erase the first one entry
				it = m_spriteMasks.begin();

			m_texturesToDelete.push_back(it->second);
			m_spriteMasks.erase(it);
			m_spritesMaskIds.pop_front();
		}
		s = createDirectDrawTexture(32, 32, false);
		if(!s)
		{
			SDL_free(pixels);
			return NULL;
		}

		m_spriteMasks[tempPos] = s;
		m_spritesMaskIds.push_back(tempPos);
	}

	while(!updateTextureData(s, pixels)) {SDL_Delay(10);}
	SDL_free(pixels);
	return s;
}

void SurfaceDirectDraw::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor)
{
	if(spriteId > g_spriteCounts || maskSpriteId > g_spriteCounts)
		return;

	#if SIZEOF_VOIDP == 4
	Uint64 tempPos;
	Uint32* u32p = SDL_reinterpret_cast(Uint32*, &tempPos);
	u32p[0] = spriteId;
	u32p[1] = outfitColor;
	#else
	Uint64 tempPos = SDL_static_cast(Uint64, (spriteId | SDL_static_cast(Uint64, outfitColor) << 32));
	#endif
	DirectDrawTexture* tex;
	U64BDDRAWTextures::iterator it = m_spriteMasks.find(tempPos);
	if(it == m_spriteMasks.end())
	{
		tex = loadSpriteMask(NULL, tempPos, spriteId, maskSpriteId, outfitColor);
		if(!tex)
			return;//load failed
	}
	else
	{
		tex = it->second;
		if(m_spritesMaskIds.front() == tempPos)
		{
			m_spritesMaskIds.pop_front();
			m_spritesMaskIds.push_back(tempPos);
		}
	}

	Sint32 w = SDL_static_cast(Sint32, tex->m_width);
	Sint32 h = SDL_static_cast(Sint32, tex->m_height);
	Sint32 sx = 0;
	Sint32 sy = 0;
	if(!testClipper(x, y, w, h, sx, sy, w, h))
		return;

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	float minu = (SDL_static_cast(float, sx)+0.5f)*tex->m_scaleW;
	float maxu = (SDL_static_cast(float, sx+w)+0.5f)*tex->m_scaleW;
	float minv = (SDL_static_cast(float, sy)+0.5f)*tex->m_scaleH;
	float maxv = (SDL_static_cast(float, sy+h)+0.5f)*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexDDRAW vertices[4];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].z = 0.0f;
	vertices[0].color = texColor;
	vertices[0].u = minu;
	vertices[0].v = minv;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].z = 0.0f;
	vertices[1].color = texColor;
	vertices[1].u = minu;
	vertices[1].v = maxv;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].z = 0.0f;
	vertices[2].color = texColor;
	vertices[2].u = maxu;
	vertices[2].v = minv;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].z = 0.0f;
	vertices[3].color = texColor;
	vertices[3].u = maxu;
	vertices[3].v = maxv;

	updateTextureScaling(tex);
	IDirect3DDevice7_SetTexture(m_device, 0, tex->m_texture);
	IDirect3DDevice7_DrawPrimitive(m_device, D3DPT_TRIANGLESTRIP, DDRAW_FVF, vertices, 4, 0);
}

void SurfaceDirectDraw::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Uint32 outfitColor)
{
	if(spriteId > g_spriteCounts || maskSpriteId > g_spriteCounts)
		return;

	#if SIZEOF_VOIDP == 4
	Uint64 tempPos;
	Uint32* u32p = SDL_reinterpret_cast(Uint32*, &tempPos);
	u32p[0] = spriteId;
	u32p[1] = outfitColor;
	#else
	Uint64 tempPos = SDL_static_cast(Uint64, (spriteId | SDL_static_cast(Uint64, outfitColor) << 32));
	#endif
	DirectDrawTexture* tex;
	U64BDDRAWTextures::iterator it = m_spriteMasks.find(tempPos);
	if(it == m_spriteMasks.end())
	{
		tex = loadSpriteMask(NULL, tempPos, spriteId, maskSpriteId, outfitColor);
		if(!tex)
			return;//load failed
	}
	else
	{
		tex = it->second;
		if(m_spritesMaskIds.front() == tempPos)
		{
			m_spritesMaskIds.pop_front();
			m_spritesMaskIds.push_back(tempPos);
		}
	}
	if(!testClipper(x, y, w, h, sx, sy, sw, sh))
		return;

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	float minu = (SDL_static_cast(float, sx)+0.5f)*tex->m_scaleW;
	float maxu = (SDL_static_cast(float, sx+sw)+0.5f)*tex->m_scaleW;
	float minv = (SDL_static_cast(float, sy)+0.5f)*tex->m_scaleH;
	float maxv = (SDL_static_cast(float, sy+sh)+0.5f)*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexDDRAW vertices[4];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].z = 0.0f;
	vertices[0].color = texColor;
	vertices[0].u = minu;
	vertices[0].v = minv;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].z = 0.0f;
	vertices[1].color = texColor;
	vertices[1].u = minu;
	vertices[1].v = maxv;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].z = 0.0f;
	vertices[2].color = texColor;
	vertices[2].u = maxu;
	vertices[2].v = minv;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].z = 0.0f;
	vertices[3].color = texColor;
	vertices[3].u = maxu;
	vertices[3].v = maxv;

	updateTextureScaling(tex);
	IDirect3DDevice7_SetTexture(m_device, 0, tex->m_texture);
	IDirect3DDevice7_DrawPrimitive(m_device, D3DPT_TRIANGLESTRIP, DDRAW_FVF, vertices, 4, 0);
}

DirectDrawTexture* SurfaceDirectDraw::createAutomapTile(Uint32 currentArea)
{
	if(m_automapTiles.size() >= HARDWARE_MAX_AUTOMAPTILES)
	{
		U32BDDRAWTextures::iterator it = m_automapTiles.find(m_automapTilesBuff.front());
		if(it == m_automapTiles.end())//Something weird happen - let's erase the first one entry
			it = m_automapTiles.begin();

		m_texturesToDelete.push_back(it->second);
		m_automapTiles.erase(it);
		m_automapTilesBuff.pop_front();
	}
	DirectDrawTexture* s = createDirectDrawTexture(256, 256, false);
	if(!s)
		return NULL;

	m_automapTiles[currentArea] = s;
	m_automapTilesBuff.push_back(currentArea);
	return s;
}

void SurfaceDirectDraw::uploadAutomapTile(DirectDrawTexture* texture, Uint8 color[256][256])
{
	unsigned char* pixels = SDL_reinterpret_cast(unsigned char*, SDL_malloc(262144));//256*256*4
	if(!pixels)
		return;

	unsigned char* ptrpixels = pixels;
	for(Sint32 y = 0; y < 256; ++y)
	{
		for(Sint32 x = 0; x < 256; ++x)
		{
			Uint8 r, g, b;
			Uint8 c = color[x][y];
			colorFrom8bit(c, r, g, b);
			*ptrpixels++ = b;
			*ptrpixels++ = g;
			*ptrpixels++ = r;
			*ptrpixels++ = 255;
		}
	}
	while(!updateTextureData(texture, pixels)) {SDL_Delay(10);}
	SDL_free(pixels);
}

void SurfaceDirectDraw::drawAutomapTile(Uint32 currentArea, bool& recreate, Uint8 color[256][256], Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
{
	if(!testClipper(x, y, w, h, sx, sy, w, h))
		return;

	DirectDrawTexture* tex;
	U32BDDRAWTextures::iterator it = m_automapTiles.find(currentArea);
	if(it == m_automapTiles.end())
	{
		tex = createAutomapTile(currentArea);
		if(!tex)
			return;//create failed

		uploadAutomapTile(tex, color);
		recreate = false;
	}
	else
	{
		tex = it->second;
		if(m_automapTilesBuff.front() == currentArea)
		{
			m_automapTilesBuff.pop_front();
			m_automapTilesBuff.push_back(currentArea);
		}
		if(recreate)
		{
			uploadAutomapTile(tex, color);
			recreate = false;
		}
	}

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	float minu = (SDL_static_cast(float, sx)+0.5f)*tex->m_scaleW;
	float maxu = (SDL_static_cast(float, sx+sw)+0.5f)*tex->m_scaleW;
	float minv = (SDL_static_cast(float, sy)+0.5f)*tex->m_scaleH;
	float maxv = (SDL_static_cast(float, sy+sh)+0.5f)*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexDDRAW vertices[4];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].z = 0.0f;
	vertices[0].color = texColor;
	vertices[0].u = minu;
	vertices[0].v = minv;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].z = 0.0f;
	vertices[1].color = texColor;
	vertices[1].u = minu;
	vertices[1].v = maxv;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].z = 0.0f;
	vertices[2].color = texColor;
	vertices[2].u = maxu;
	vertices[2].v = minv;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].z = 0.0f;
	vertices[3].color = texColor;
	vertices[3].u = maxu;
	vertices[3].v = maxv;

	updateTextureScaling(tex);
	IDirect3DDevice7_SetTexture(m_device, 0, tex->m_texture);
	IDirect3DDevice7_DrawPrimitive(m_device, D3DPT_TRIANGLESTRIP, DDRAW_FVF, vertices, 4, 0);
}

SurfaceDirectDrawComp::SurfaceDirectDrawComp()
{
	m_sprites = NULL;
}

SurfaceDirectDrawComp::~SurfaceDirectDrawComp()
{
	if(m_sprites)
	{
		for(Uint32 i = 1; i <= m_spritesCache; ++i)
		{
			if(m_sprites[i])
				releaseDirectDrawTexture(m_sprites[i]);
		}

		SDL_free(m_sprites);
	}
}

void SurfaceDirectDrawComp::init()
{
	SurfaceDirectDraw::init();
	m_sprites = SDL_reinterpret_cast(DirectDrawTexture**, SDL_calloc(g_spriteCounts+1, sizeof(DirectDrawTexture*)));
	if(!m_sprites)
	{
		SDL_OutOfMemory();
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Surface::Init() Failed: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
	}
	m_spritesCache = g_spriteCounts;
}

void SurfaceDirectDrawComp::spriteManagerReset()
{
	SurfaceDirectDraw::spriteManagerReset();
	if(m_sprites)
	{
		for(Uint32 i = 1; i <= m_spritesCache; ++i)
		{
			if(m_sprites[i])
				releaseDirectDrawTexture(m_sprites[i]);
		}

		SDL_free(m_sprites);
	}
	m_sprites = SDL_reinterpret_cast(DirectDrawTexture**, SDL_calloc(g_spriteCounts+1, sizeof(DirectDrawTexture*)));
	if(!m_sprites)
	{
		SDL_OutOfMemory();
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Surface::spriteManagerReset() Failed: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
	}
	m_spritesCache = g_spriteCounts;
}

DirectDrawTexture* SurfaceDirectDrawComp::loadSprite(DirectDrawTexture* s, Uint32 spriteId)
{
	unsigned char* pixels = g_engine.LoadSprite(spriteId, true);
	if(!pixels)
		return NULL;

	if(!s)
	{
		s = createDirectDrawTexture(32, 32, false);
		if(!s)
		{
			SDL_free(pixels);
			return NULL;
		}
		m_sprites[spriteId] = s;
	}

	while(!updateTextureData(s, pixels)) {SDL_Delay(10);}
	SDL_free(pixels);
	return s;
}

void SurfaceDirectDrawComp::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y)
{
	if(spriteId > g_spriteCounts)
		return;

	DirectDrawTexture* tex = m_sprites[spriteId];
	if(!tex)
	{
		tex = loadSprite(NULL, spriteId);
		if(!tex)
			return;//load failed
	}

	Sint32 w = SDL_static_cast(Sint32, tex->m_width);
	Sint32 h = SDL_static_cast(Sint32, tex->m_height);
	Sint32 sx = 0;
	Sint32 sy = 0;
	if(!testClipper(x, y, w, h, sx, sy, w, h))
		return;

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	float minu = (SDL_static_cast(float, sx)+0.5f)*tex->m_scaleW;
	float maxu = (SDL_static_cast(float, sx+w)+0.5f)*tex->m_scaleW;
	float minv = (SDL_static_cast(float, sy)+0.5f)*tex->m_scaleH;
	float maxv = (SDL_static_cast(float, sy+h)+0.5f)*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexDDRAW vertices[4];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].z = 0.0f;
	vertices[0].color = texColor;
	vertices[0].u = minu;
	vertices[0].v = minv;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].z = 0.0f;
	vertices[1].color = texColor;
	vertices[1].u = minu;
	vertices[1].v = maxv;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].z = 0.0f;
	vertices[2].color = texColor;
	vertices[2].u = maxu;
	vertices[2].v = minv;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].z = 0.0f;
	vertices[3].color = texColor;
	vertices[3].u = maxu;
	vertices[3].v = maxv;

	updateTextureScaling(tex);
	IDirect3DDevice7_SetTexture(m_device, 0, tex->m_texture);
	IDirect3DDevice7_DrawPrimitive(m_device, D3DPT_TRIANGLESTRIP, DDRAW_FVF, vertices, 4, 0);
}

void SurfaceDirectDrawComp::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
{
	if(spriteId > g_spriteCounts)
		return;

	DirectDrawTexture* tex = m_sprites[spriteId];
	if(!tex)
	{
		tex = loadSprite(NULL, spriteId);
		if(!tex)
			return;//load failed
	}
	if(!testClipper(x, y, w, h, sx, sy, sw, sh))
		return;

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	float minu = (SDL_static_cast(float, sx)+0.5f)*tex->m_scaleW;
	float maxu = (SDL_static_cast(float, sx+sw)+0.5f)*tex->m_scaleW;
	float minv = (SDL_static_cast(float, sy)+0.5f)*tex->m_scaleH;
	float maxv = (SDL_static_cast(float, sy+sh)+0.5f)*tex->m_scaleH;
	
	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexDDRAW vertices[4];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].z = 0.0f;
	vertices[0].color = texColor;
	vertices[0].u = minu;
	vertices[0].v = minv;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].z = 0.0f;
	vertices[1].color = texColor;
	vertices[1].u = minu;
	vertices[1].v = maxv;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].z = 0.0f;
	vertices[2].color = texColor;
	vertices[2].u = maxu;
	vertices[2].v = minv;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].z = 0.0f;
	vertices[3].color = texColor;
	vertices[3].u = maxu;
	vertices[3].v = maxv;

	updateTextureScaling(tex);
	IDirect3DDevice7_SetTexture(m_device, 0, tex->m_texture);
	IDirect3DDevice7_DrawPrimitive(m_device, D3DPT_TRIANGLESTRIP, DDRAW_FVF, vertices, 4, 0);
}

SurfaceDirectDrawPerf::SurfaceDirectDrawPerf()
{
	m_stagging = NULL;
	m_sprites = NULL;
	m_spriteAtlas = NULL;

	m_spriteAtlases = 0;
	m_spritesPerAtlas = 0;
	m_spritesPerModulo = 0;

	m_scheduleSpriteDraw = false;
}

SurfaceDirectDrawPerf::~SurfaceDirectDrawPerf()
{
	if(m_stagging)
		releaseDirectDrawTexture(m_stagging);

	if(m_sprites)
		SDL_free(m_sprites);

	for(std::vector<DirectDrawTexture*>::iterator it = m_spritesAtlas.begin(), end = m_spritesAtlas.end(); it != end; ++it)
		releaseDirectDrawTexture((*it));

	m_spritesAtlas.clear();
}

void SurfaceDirectDrawPerf::generateSpriteAtlases()
{
	//TODO: don't make atlases for whole sprites because on big .spr it eat up whole vram
	//use only necessary amount of textures
	if(m_maxTextureSize >= 16384 && g_spriteCounts > 65536)
	{
		m_spriteAtlases = (g_spriteCounts+262143)/262144;
		m_spritesPerAtlas = 262144;
		m_spritesPerModulo = 16384;
	}
	else if(m_maxTextureSize >= 8192 && g_spriteCounts > 16384)
	{
		m_spriteAtlases = (g_spriteCounts+65535)/65536;
		m_spritesPerAtlas = 65536;
		m_spritesPerModulo = 8192;
	}
	else if(m_maxTextureSize >= 4096 && g_spriteCounts > 4096)
	{
		m_spriteAtlases = (g_spriteCounts+16383)/16384;
		m_spritesPerAtlas = 16384;
		m_spritesPerModulo = 4096;
	}
	else if(m_maxTextureSize >= 2048 && g_spriteCounts > 1024)
	{
		m_spriteAtlases = (g_spriteCounts+4095)/4096;
		m_spritesPerAtlas = 4096;
		m_spritesPerModulo = 2048;
	}
	else
	{
		m_spriteAtlases = (g_spriteCounts+1023)/1024;
		m_spritesPerAtlas = 1024;
		m_spritesPerModulo = 1024;
	}
	for(std::vector<DirectDrawTexture*>::iterator it = m_spritesAtlas.begin(), end = m_spritesAtlas.end(); it != end; ++it)
		releaseDirectDrawTexture((*it));

	m_spritesAtlas.clear();
	for(Uint32 i = 0; i < m_spriteAtlases; ++i)
	{
		DirectDrawTexture* texture = createDirectDrawTexture(m_spritesPerModulo, m_spritesPerModulo, false);
		if(!texture)
		{
			UTIL_MessageBox(true, "DirectDraw: Out of video memory.");
			exit(-1);
			return;
		}
		m_spritesAtlas.push_back(texture);
	}
}

void SurfaceDirectDrawPerf::checkScheduledSprites()
{
	size_t vertices = m_gameWindowVertices.size();
	if(vertices > 0)
	{
		updateTextureScaling(m_spriteAtlas);
		IDirect3DDevice7_SetTexture(m_device, 0, m_spriteAtlas->m_texture);
		drawTriangles(m_gameWindowVertices);

		m_gameWindowVertices.clear();
		m_gameWindowVertices.reserve(30720);
	}
}

void SurfaceDirectDrawPerf::init()
{
	SurfaceDirectDraw::init();
	m_sprites = SDL_reinterpret_cast(bool*, SDL_calloc(g_spriteCounts+1, sizeof(bool)));
	if(!m_sprites)
	{
		SDL_OutOfMemory();
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Surface::Init() Failed: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
	}
	m_spritesCache = g_spriteCounts;
}

void SurfaceDirectDrawPerf::spriteManagerReset()
{
	SurfaceDirectDraw::spriteManagerReset();
	if(m_sprites)
		SDL_free(m_sprites);

	m_sprites = SDL_reinterpret_cast(bool*, SDL_calloc(g_spriteCounts+1, sizeof(bool)));
	if(!m_sprites)
	{
		SDL_OutOfMemory();
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Surface::spriteManagerReset() Failed: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
	}
	m_spritesCache = g_spriteCounts;
	generateSpriteAtlases();
}

void SurfaceDirectDrawPerf::beginScene()
{
	SurfaceDirectDraw::beginScene();
	m_scheduleSpriteDraw = false;
}

void SurfaceDirectDrawPerf::beginGameScene()
{
	SurfaceDirectDraw::beginGameScene();
	m_gameWindowVertices.clear();
	m_gameWindowVertices.reserve(30720);
	m_scheduleSpriteDraw = true;
}

void SurfaceDirectDrawPerf::endGameScene()
{
	checkScheduledSprites();
	m_scheduleSpriteDraw = false;
	SurfaceDirectDraw::endGameScene();
}

void SurfaceDirectDrawPerf::drawRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(m_scheduleSpriteDraw)
		checkScheduledSprites();
	SurfaceDirectDraw::drawRectangle(x, y, w, h, r, g, b, a);
}

void SurfaceDirectDrawPerf::fillRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(m_scheduleSpriteDraw)
		checkScheduledSprites();
	SurfaceDirectDraw::fillRectangle(x, y, w, h, r, g, b, a);
}

bool SurfaceDirectDrawPerf::loadSprite(Uint32 spriteId, DirectDrawTexture* texture, Uint32 xoff, Uint32 yoff)
{
	if(!m_stagging)
	{
		//Without stagging texture lock/unlock is slow af
		m_stagging = createDirectDrawTexture(32, 32, false);
		if(!m_stagging)
			return false;
	}

	unsigned char* pixels = g_engine.LoadSprite(spriteId, true);
	if(!pixels)
		return false;

	while(!updateTextureData(m_stagging, pixels)) {SDL_Delay(10);}
	SDL_free(pixels);

	RECT d3drect;
	d3drect.left = 0;
	d3drect.right = 32;
	d3drect.top = 0;
	d3drect.bottom = 32;
	IDirectDrawSurface7_BltFast(texture->m_texture, xoff, yoff, m_stagging->m_texture, &d3drect, (DDBLTFAST_NOCOLORKEY|DDBLTFAST_WAIT));
	return true;
}

void SurfaceDirectDrawPerf::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y)
{
	if(spriteId > g_spriteCounts)
		return;

	Uint32 spriteIndex = spriteId-1;
	Uint32 spriteAtlas = spriteIndex/m_spritesPerAtlas;
	spriteIndex = (spriteIndex%m_spritesPerAtlas)*32;
	if(spriteAtlas >= m_spritesAtlas.size())
		return;

	Uint32 xOffset = spriteIndex%m_spritesPerModulo;
	Uint32 yOffset = (spriteIndex/m_spritesPerModulo)*32;

	DirectDrawTexture* tex = m_spritesAtlas[spriteAtlas];
	if(!m_sprites[spriteId])
	{
		if(!loadSprite(spriteId, tex, xOffset, yOffset))
			return;//load failed
		m_sprites[spriteId] = true;
	}

	Sint32 w = 32;
	Sint32 h = 32;
	Sint32 sx = SDL_static_cast(Sint32, xOffset);
	Sint32 sy = SDL_static_cast(Sint32, yOffset);
	if(!testClipper(x, y, w, h, sx, sy, w, h))
		return;

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	float minu = (SDL_static_cast(float, sx)+0.5f)*tex->m_scaleW;
	float maxu = (SDL_static_cast(float, sx+w)+0.5f)*tex->m_scaleW;
	float minv = (SDL_static_cast(float, sy)+0.5f)*tex->m_scaleH;
	float maxv = (SDL_static_cast(float, sy+h)+0.5f)*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexDDRAW vertices[4];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].z = 0.0f;
	vertices[0].color = texColor;
	vertices[0].u = minu;
	vertices[0].v = minv;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].z = 0.0f;
	vertices[1].color = texColor;
	vertices[1].u = minu;
	vertices[1].v = maxv;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].z = 0.0f;
	vertices[2].color = texColor;
	vertices[2].u = maxu;
	vertices[2].v = minv;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].z = 0.0f;
	vertices[3].color = texColor;
	vertices[3].u = maxu;
	vertices[3].v = maxv;
	if(m_scheduleSpriteDraw)
	{
		if(tex != m_spriteAtlas)
		{
			checkScheduledSprites();
			m_spriteAtlas = tex;
		}

		m_gameWindowVertices.push_back(vertices[0]);
		m_gameWindowVertices.push_back(vertices[1]);
		m_gameWindowVertices.push_back(vertices[2]);
		m_gameWindowVertices.push_back(vertices[3]);
		m_gameWindowVertices.push_back(vertices[2]);
		m_gameWindowVertices.push_back(vertices[1]);
	}
	else
	{
		updateTextureScaling(tex);
		IDirect3DDevice7_SetTexture(m_device, 0, tex->m_texture);
		IDirect3DDevice7_DrawPrimitive(m_device, D3DPT_TRIANGLESTRIP, DDRAW_FVF, vertices, 4, 0);
	}
}

void SurfaceDirectDrawPerf::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
{
	if(spriteId > g_spriteCounts)
		return;

	Uint32 spriteIndex = spriteId-1;
	Uint32 spriteAtlas = spriteIndex/m_spritesPerAtlas;
	spriteIndex = (spriteIndex%m_spritesPerAtlas)*32;
	if(spriteAtlas >= m_spritesAtlas.size())
		return;

	Uint32 xOffset = spriteIndex%m_spritesPerModulo;
	Uint32 yOffset = (spriteIndex/m_spritesPerModulo)*32;

	DirectDrawTexture* tex = m_spritesAtlas[spriteAtlas];
	if(!m_sprites[spriteId])
	{
		if(!loadSprite(spriteId, tex, xOffset, yOffset))
			return;//load failed
		m_sprites[spriteId] = true;
	}
	if(!testClipper(x, y, w, h, sx, sy, sw, sh))
		return;

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	sx += SDL_static_cast(Sint32, xOffset);
	sy += SDL_static_cast(Sint32, yOffset);

	float minu = (SDL_static_cast(float, sx)+0.5f)*tex->m_scaleW;
	float maxu = (SDL_static_cast(float, sx+sw)+0.5f)*tex->m_scaleW;
	float minv = (SDL_static_cast(float, sy)+0.5f)*tex->m_scaleH;
	float maxv = (SDL_static_cast(float, sy+sh)+0.5f)*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexDDRAW vertices[4];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].z = 0.0f;
	vertices[0].color = texColor;
	vertices[0].u = minu;
	vertices[0].v = minv;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].z = 0.0f;
	vertices[1].color = texColor;
	vertices[1].u = minu;
	vertices[1].v = maxv;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].z = 0.0f;
	vertices[2].color = texColor;
	vertices[2].u = maxu;
	vertices[2].v = minv;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].z = 0.0f;
	vertices[3].color = texColor;
	vertices[3].u = maxu;
	vertices[3].v = maxv;

	updateTextureScaling(tex);
	IDirect3DDevice7_SetTexture(m_device, 0, tex->m_texture);
	IDirect3DDevice7_DrawPrimitive(m_device, D3DPT_TRIANGLESTRIP, DDRAW_FVF, vertices, 4, 0);
}

void SurfaceDirectDrawPerf::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor)
{
	if(m_scheduleSpriteDraw)
		checkScheduledSprites();
	SurfaceDirectDraw::drawSpriteMask(spriteId, maskSpriteId, x, y, outfitColor);
}
#endif
