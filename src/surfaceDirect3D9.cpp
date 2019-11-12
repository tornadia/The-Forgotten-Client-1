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

#include "surfaceDirect3D9.h"

#if defined(SDL_VIDEO_RENDER_D3D)
#include <d3d9.h>
#include <SDL2/SDL_syswm.h>

extern Engine g_engine;
extern Uint32 g_spriteCounts;
extern Uint16 g_pictureCounts;

static D3DPRESENT_PARAMETERS d3d9_pparams;

typedef IDirect3D9*(WINAPI *PFN_CREATE_D3D9)(UINT SDKVersion);
typedef HRESULT(WINAPI *PFN_CREATE_D3D9Ex)(UINT SDKVersion, IDirect3D9Ex** d3d9Ex);

static const GUID D3D9_IID_IDirect3D9 = {0x81BDCBCA, 0x64D4, 0x426D,{0xAE, 0x8D, 0xAD, 0x01, 0x47, 0xF4, 0x27, 0x5C}};
static const GUID D3D9_IID_IDirect3DDevice9Ex = {0xB18B10CE, 0x2649, 0x405A,{0x87, 0x0F, 0x95, 0xF7, 0x77, 0xD4, 0x31, 0x3A}};

SurfaceDirect3D9::SurfaceDirect3D9() : m_automapTilesBuff(HARDWARE_MAX_AUTOMAPTILES), m_spritesMaskIds(HARDWARE_MAX_SPRITEMASKS)
{
	g_engine.RecreateWindow(false);

	m_d3d9Handle = SDL_LoadObject("D3D9.DLL");
	m_d3d9 = NULL;
	m_d3d9Ex = NULL;
	m_device = NULL;
	m_deviceEx = NULL;

	m_defaultRenderTarget = NULL;
	m_currentRenderTarget = NULL;

	m_pictures = NULL;
	m_software = "Direct3D 9.0";
	m_hardware = NULL;

	m_gameWindow = NULL;
	m_scaled_gameWindow = NULL;
	m_pixelShader_sharpen = NULL;

	m_maxTextureSize = 1024;
	m_integer_scaling_width = 0;
	m_integer_scaling_height = 0;

	m_adapter = 0;
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = 0;
	m_viewPortH = 0;

	m_totalVRAM = 0;
	m_spritesCache = 0;

	m_usingLinearSample = false;
	m_useAlphaBlending = false;
	m_needReset = true;
	m_haveSharpening = false;

	#ifdef HAVE_CXX11_SUPPORT
	m_spriteMasks.reserve(HARDWARE_MAX_SPRITEMASKS);
	m_automapTiles.reserve(HARDWARE_MAX_AUTOMAPTILES);
	#endif
}

SurfaceDirect3D9::~SurfaceDirect3D9()
{
	if(m_hardware)
		SDL_free(m_hardware);

	if(m_pixelShader_sharpen)
		IDirect3DPixelShader9_Release(m_pixelShader_sharpen);

	if(m_defaultRenderTarget)
		IDirect3DSurface9_Release(m_defaultRenderTarget);

	if(m_currentRenderTarget)
		IDirect3DSurface9_Release(m_currentRenderTarget);

	if(m_pictures)
	{
		for(Uint16 i = 0; i < g_pictureCounts; ++i)
		{
			if(m_pictures[i])
				releaseDirect3DTexture(m_pictures[i]);
		}

		SDL_free(m_pictures);
	}

	for(U32BD3D9Textures::iterator it = m_automapTiles.begin(), end = m_automapTiles.end(); it != end; ++it)
		releaseDirect3DTexture(it->second);

	for(U64BD3D9Textures::iterator it = m_spriteMasks.begin(), end = m_spriteMasks.end(); it != end; ++it)
		releaseDirect3DTexture(it->second);

	m_spriteMasks.clear();
	m_spritesMaskIds.clear();
	m_automapTilesBuff.clear();
	m_automapTiles.clear();
	if(m_scaled_gameWindow)
		releaseDirect3DTexture(m_scaled_gameWindow);

	if(m_gameWindow)
		releaseDirect3DTexture(m_gameWindow);

	if(m_device)
		IDirect3DDevice9_Release(m_device);

	if(m_deviceEx)
		IDirect3DDevice9Ex_Release(m_deviceEx);

	if(m_d3d9)
		IDirect3D9_Release(m_d3d9);

	if(m_d3d9Ex)
		IDirect3D9Ex_Release(m_d3d9Ex);

	if(m_d3d9Handle)
		SDL_UnloadObject(m_d3d9Handle);
}

Direct3D9Texture* SurfaceDirect3D9::createDirect3DTexture(Uint32 width, Uint32 height, bool linearSampler, bool frameBuffer)
{
	Direct3D9Texture* texture = new Direct3D9Texture();
	if(!texture)
		return NULL;

	HRESULT result = IDirect3DDevice9_CreateTexture(m_device, width, height, 1, (frameBuffer ? D3DUSAGE_RENDERTARGET : 0), D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture->m_texture, NULL);
	if(FAILED(result))
	{
		releaseDirect3DTexture(texture);
		return NULL;
	}
	texture->m_width = width;
	texture->m_height = height;
	texture->m_scaleW = 1.0f/width;
	texture->m_scaleH = 1.0f/height;
	texture->m_linearSample = linearSampler;
	return texture;
}

bool SurfaceDirect3D9::updateTextureData(Direct3D9Texture* texture, unsigned char* data)
{
	IDirect3DTexture9* stagingTexture;
	HRESULT result = IDirect3DDevice9_CreateTexture(m_device, texture->m_width, texture->m_height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &stagingTexture, NULL);
	if(FAILED(result))
		return false;

	D3DLOCKED_RECT locked;
	result = IDirect3DTexture9_LockRect(stagingTexture, 0, &locked, NULL, D3DLOCK_NOSYSLOCK);
	if(FAILED(result))
	{
		IDirect3DTexture9_Release(stagingTexture);
		return false;
	}

	unsigned char* dstData = SDL_reinterpret_cast(unsigned char*, locked.pBits);
	int length = texture->m_width*4;
	if(length == locked.Pitch)
		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, dstData), SDL_reinterpret_cast(const Uint8*, data), length*texture->m_height);
	else
	{
		if(length > locked.Pitch)
			length = locked.Pitch;
		for(Uint32 row = 0; row < texture->m_height; ++row)
		{
			UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, dstData), SDL_reinterpret_cast(const Uint8*, data), length);
			data += length;
			dstData += locked.Pitch;
		}
	}
	result = IDirect3DTexture9_UnlockRect(stagingTexture, 0);
	if(FAILED(result))
	{
		IDirect3DTexture9_Release(stagingTexture);
		return false;
	}
	IDirect3DDevice9_UpdateTexture(m_device, SDL_reinterpret_cast(IDirect3DBaseTexture9*, stagingTexture), SDL_reinterpret_cast(IDirect3DBaseTexture9*, texture->m_texture));
	IDirect3DTexture9_Release(stagingTexture);
	return true;
}

void SurfaceDirect3D9::releaseDirect3DTexture(Direct3D9Texture* texture)
{
	if(texture->m_texture)
		IDirect3DTexture9_Release(texture->m_texture);
	delete texture;
}

void SurfaceDirect3D9::updateTextureScaling(Direct3D9Texture* texture)
{
	if(m_usingLinearSample != texture->m_linearSample)
	{
		m_usingLinearSample = texture->m_linearSample;
		IDirect3DDevice9_SetSamplerState(m_device, 0, D3DSAMP_MINFILTER, (m_usingLinearSample ? D3DTEXF_LINEAR : D3DTEXF_POINT));
		IDirect3DDevice9_SetSamplerState(m_device, 0, D3DSAMP_MAGFILTER, (m_usingLinearSample ? D3DTEXF_LINEAR : D3DTEXF_POINT));
		IDirect3DDevice9_SetSamplerState(m_device, 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		IDirect3DDevice9_SetSamplerState(m_device, 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	}
}

void SurfaceDirect3D9::drawTriangles(std::vector<VertexD3D9>& vertices)
{
	UINT verticesSize = SDL_static_cast(UINT, vertices.size());
	if(verticesSize > 2046)
	{
		VertexD3D9* verticeData = &vertices[0];

		UINT verticeIndex = 0;
		while(verticeIndex < verticesSize)
		{
			UINT drawSize = UTIL_min<UINT>(verticesSize-verticeIndex, 2046);
			IDirect3DDevice9_DrawPrimitiveUP(m_device, D3DPT_TRIANGLELIST, (drawSize/3), SDL_reinterpret_cast(void*, &verticeData[verticeIndex]), sizeof(VertexD3D9));
			verticeIndex += drawSize;
		}
	}
	else if(verticesSize > 0)
		IDirect3DDevice9_DrawPrimitiveUP(m_device, D3DPT_TRIANGLELIST, (verticesSize/3), &vertices[0], sizeof(VertexD3D9));
}

void SurfaceDirect3D9::initRenderer()
{
	IDirect3DDevice9_SetVertexShader(m_device, NULL);
	IDirect3DDevice9_SetPixelShader(m_device, NULL);
	IDirect3DDevice9_SetFVF(m_device, (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1));
	IDirect3DDevice9_SetRenderState(m_device, D3DRS_ZENABLE, D3DZB_FALSE);
	IDirect3DDevice9_SetRenderState(m_device, D3DRS_CULLMODE, D3DCULL_NONE);
	IDirect3DDevice9_SetRenderState(m_device, D3DRS_LIGHTING, FALSE);

	IDirect3DDevice9_SetTextureStageState(m_device, 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	IDirect3DDevice9_SetTextureStageState(m_device, 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	IDirect3DDevice9_SetTextureStageState(m_device, 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	IDirect3DDevice9_SetTextureStageState(m_device, 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	IDirect3DDevice9_SetTextureStageState(m_device, 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	IDirect3DDevice9_SetTextureStageState(m_device, 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	if(m_useAlphaBlending)
		IDirect3DDevice9_SetRenderState(m_device, D3DRS_SEPARATEALPHABLENDENABLE, TRUE);

	IDirect3DDevice9_SetTextureStageState(m_device, 1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	IDirect3DDevice9_SetTextureStageState(m_device, 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	D3DMATRIX matrix;
	matrix._11 = matrix._22 = matrix._33 = matrix._44 = 1.0f;
	matrix._12 = matrix._13 = matrix._14 = matrix._41 = 0.0f;
	matrix._21 = matrix._23 = matrix._24 = matrix._42 = 0.0f;
	matrix._31 = matrix._32 = matrix._34 = matrix._43 = 0.0f;
	IDirect3DDevice9_SetTransform(m_device, D3DTS_WORLD, &matrix);
	IDirect3DDevice9_SetTransform(m_device, D3DTS_VIEW, &matrix);
	IDirect3DDevice9_SetRenderTarget(m_device, 0, m_defaultRenderTarget);

	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = SDL_static_cast(Uint32, g_engine.getWindowWidth());
	m_viewPortH = SDL_static_cast(Uint32, g_engine.getWindowHeight());

	updateRenderer();
	updateViewport();
}

void SurfaceDirect3D9::resetRenderer()
{
	releaseTextures();
	if(m_defaultRenderTarget)
	{
		IDirect3DSurface9_Release(m_defaultRenderTarget);
		m_defaultRenderTarget = NULL;
	}

	if(m_currentRenderTarget)
	{
		IDirect3DSurface9_Release(m_currentRenderTarget);
		m_currentRenderTarget = NULL;
	}

	if(m_gameWindow)
	{
		releaseDirect3DTexture(m_gameWindow);
		m_gameWindow = NULL;
	}

	if(m_scaled_gameWindow)
	{
		releaseDirect3DTexture(m_scaled_gameWindow);
		m_scaled_gameWindow = NULL;
	}

	HRESULT result = IDirect3DDevice9_Reset(m_device, &d3d9_pparams);
	if(FAILED(result))
	{
		//Restore later
		return;
	}

	IDirect3DDevice9_GetRenderTarget(m_device, 0, &m_defaultRenderTarget);
	initRenderer();
	renderTargetsRecreate();
}

void SurfaceDirect3D9::updateRenderer()
{
	IDirect3DDevice9_SetSamplerState(m_device, 0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	IDirect3DDevice9_SetSamplerState(m_device, 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	IDirect3DDevice9_SetSamplerState(m_device, 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	IDirect3DDevice9_SetSamplerState(m_device, 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	m_usingLinearSample = false;

	IDirect3DDevice9_SetRenderState(m_device, D3DRS_ALPHABLENDENABLE, TRUE);
	IDirect3DDevice9_SetRenderState(m_device, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	IDirect3DDevice9_SetRenderState(m_device, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	if(m_useAlphaBlending)
	{
		IDirect3DDevice9_SetRenderState(m_device, D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
		IDirect3DDevice9_SetRenderState(m_device, D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
	}
}

void SurfaceDirect3D9::updateViewport()
{
	D3DVIEWPORT9 viewport = {m_viewPortX, m_viewPortY, m_viewPortW, m_viewPortH, 0.0f, 1.0f};
	IDirect3DDevice9_SetViewport(m_device, &viewport);

	D3DMATRIX matrix;
	matrix._11 = 2.0f/m_viewPortW; matrix._12 = 0.0f; matrix._13 = 0.0f; matrix._14 = 0.0f;
	matrix._21 = 0.0f; matrix._22 = -2.0f/m_viewPortH; matrix._23 = 0.0f; matrix._24 = 0.0f;
	matrix._31 = 0.0f; matrix._32 = 0.0f; matrix._33 = 1.0f; matrix._34 = 0.0f;
	matrix._41 = -1.0f; matrix._42 = 1.0f; matrix._43 = 0.0f; matrix._44 = 1.0f;
	IDirect3DDevice9_SetTransform(m_device, D3DTS_PROJECTION, &matrix);
}

bool SurfaceDirect3D9::isSupported()
{
	PFN_CREATE_D3D9 CreateD3D9Func;
	PFN_CREATE_D3D9Ex CreateD3D9ExFunc;
	if(!m_d3d9Handle)
	{
		UTIL_MessageBox(false, "Direct3D9: Failed to create Direct3D9 interface.");
		return false;
	}

	CreateD3D9Func = SDL_reinterpret_cast(PFN_CREATE_D3D9, SDL_LoadFunction(m_d3d9Handle, "Direct3DCreate9"));
	if(!CreateD3D9Func)
	{
		UTIL_MessageBox(false, "Direct3D9: Failed to fetch 'Direct3DCreate9' function.");
		return false;
	}

	CreateD3D9ExFunc = SDL_reinterpret_cast(PFN_CREATE_D3D9Ex, SDL_LoadFunction(m_d3d9Handle, "Direct3DCreate9Ex"));
	if(CreateD3D9ExFunc)
	{
		HRESULT result = CreateD3D9ExFunc(D3D_SDK_VERSION, &m_d3d9Ex);
		if(SUCCEEDED(result))
			IDirect3D9Ex_QueryInterface(m_d3d9Ex, D3D9_IID_IDirect3D9, SDL_reinterpret_cast(void**, &m_d3d9));
	}
	if(!m_d3d9)
	{
		m_d3d9 = CreateD3D9Func(D3D_SDK_VERSION);
		if(!m_d3d9Handle)
		{
			UTIL_MessageBox(false, "Direct3D9: Failed to create Direct3D9 interface.");
			return false;
		}
	}

	SDL_SysWMinfo windowinfo;
	SDL_VERSION(&windowinfo.version);
	SDL_GetWindowWMInfo(g_engine.m_window, &windowinfo);

	int w_width, w_height;
	SDL_DisplayMode fullscreen_mode;
	Uint32 window_flags = SDL_GetWindowFlags(g_engine.m_window);
	SDL_GetWindowSize(g_engine.m_window, &w_width, &w_height);
	SDL_GetWindowDisplayMode(g_engine.m_window, &fullscreen_mode);

	SDL_zero(d3d9_pparams);
	d3d9_pparams.hDeviceWindow = windowinfo.info.win.window;
	d3d9_pparams.BackBufferWidth = w_width;
	d3d9_pparams.BackBufferHeight = w_height;
	if(m_d3d9Ex)
		d3d9_pparams.BackBufferCount = 2;
	else
		d3d9_pparams.BackBufferCount = 1;
	if(m_d3d9Ex)
		d3d9_pparams.SwapEffect = D3DSWAPEFFECT_FLIPEX;
	else
		d3d9_pparams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	if(window_flags & SDL_WINDOW_FULLSCREEN && (window_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != SDL_WINDOW_FULLSCREEN_DESKTOP)
	{
		d3d9_pparams.Windowed = FALSE;
		if(fullscreen_mode.format == SDL_PIXELFORMAT_ARGB8888)
			d3d9_pparams.BackBufferFormat = D3DFMT_A8R8G8B8;
		else
			d3d9_pparams.BackBufferFormat = D3DFMT_X8R8G8B8;
		d3d9_pparams.FullScreen_RefreshRateInHz = fullscreen_mode.refresh_rate;
	}
	else
	{
		d3d9_pparams.Windowed = TRUE;
		d3d9_pparams.BackBufferFormat = D3DFMT_UNKNOWN;
		d3d9_pparams.FullScreen_RefreshRateInHz = 0;
	}
	if(g_engine.isVsync())
		d3d9_pparams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	else
		d3d9_pparams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	int displayIndex = SDL_GetWindowDisplayIndex(g_engine.m_window);
	m_adapter = SDL_Direct3D9GetAdapterIndex(displayIndex);

	D3DCAPS9 caps;
	IDirect3D9_GetDeviceCaps(m_d3d9, m_adapter, D3DDEVTYPE_HAL, &caps);

	DWORD device_flags = (D3DCREATE_FPU_PRESERVE|D3DCREATE_NOWINDOWCHANGES);
	if(caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		device_flags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		device_flags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	HRESULT result = IDirect3D9_CreateDevice(m_d3d9, m_adapter, D3DDEVTYPE_HAL, d3d9_pparams.hDeviceWindow, device_flags, &d3d9_pparams, &m_device);
	if(FAILED(result))
	{
		UTIL_MessageBox(false, "Direct3D9: Failed to create Direct3D9 Device Context.");
		return false;
	}
	if(m_d3d9Ex)
		IDirect3DDevice9_QueryInterface(m_device, D3D9_IID_IDirect3DDevice9Ex, SDL_reinterpret_cast(void**, &m_deviceEx));
	IDirect3DDevice9_GetDeviceCaps(m_device, &caps);
	
	m_maxTextureSize = UTIL_min<Sint32>(caps.MaxTextureWidth, caps.MaxTextureHeight);
	if(m_maxTextureSize < 1024)
	{
		UTIL_MessageBox(false, "Direct3D9: Your device doesn't meet minimum requirement for texture dimension.");
		return false;
	}
	if(caps.PrimitiveMiscCaps & D3DPMISCCAPS_SEPARATEALPHABLEND)
		m_useAlphaBlending = true;

	if(caps.PixelShaderVersion <= D3DPS_VERSION(1, 0))
		m_software = "Direct3D 8.0";
	else if(caps.PixelShaderVersion <= D3DPS_VERSION(1, 3))
		m_software = "Direct3D 8.0a";
	else if(caps.PixelShaderVersion <= D3DPS_VERSION(1, 4))
		m_software = "Direct3D 8.1";
	else if(caps.PixelShaderVersion <= D3DPS_VERSION(2, 0))
		m_software = "Direct3D 9.0";
	else if(caps.PixelShaderVersion < D3DPS_VERSION(3, 0))
		m_software = "Direct3D 9.0b";
	else if(caps.PixelShaderVersion <= D3DPS_VERSION(3, 0))
		m_software = "Direct3D 9.0c";

	if(SUCCEEDED(IDirect3DDevice9_CreatePixelShader(m_device, D3D9_sharpen, &m_pixelShader_sharpen)))
		m_haveSharpening = true;

	IDirect3DDevice9_GetRenderTarget(m_device, 0, &m_defaultRenderTarget);
	initRenderer();

	D3DADAPTER_IDENTIFIER9 identifier;
	if(SUCCEEDED(IDirect3D9_GetAdapterIdentifier(m_d3d9, m_adapter, 0, &identifier)))
		m_hardware = SDL_strdup(identifier.Description);
	m_totalVRAM = UTIL_power_of_2(IDirect3DDevice9_GetAvailableTextureMem(m_device)/1048576)*2;
	return true;
}

void SurfaceDirect3D9::renderTargetsRecreate()
{
	if(m_gameWindow)
	{
		releaseDirect3DTexture(m_gameWindow);
		m_gameWindow = NULL;
	}

	m_gameWindow = createDirect3DTexture(RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT, (g_engine.getAntialiasing() == CLIENT_ANTIALIASING_NORMAL), true);
	if(!m_gameWindow)
	{
		UTIL_MessageBox(false, "Direct3D9: Out of video memory.");
		exit(-1);
		return;
	}
}

void SurfaceDirect3D9::init()
{
	m_pictures = SDL_reinterpret_cast(Direct3D9Texture**, SDL_calloc(g_pictureCounts, sizeof(Direct3D9Texture*)));
	if(!m_pictures)
	{
		SDL_OutOfMemory();
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Surface::Init() Failed: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
	}
	renderTargetsRecreate();
}

void SurfaceDirect3D9::doResize(Sint32, Sint32)
{
	m_needReset = true;
}

void SurfaceDirect3D9::spriteManagerReset()
{
	for(U32BD3D9Textures::iterator it = m_automapTiles.begin(), end = m_automapTiles.end(); it != end; ++it)
		releaseDirect3DTexture(it->second);

	for(U64BD3D9Textures::iterator it = m_spriteMasks.begin(), end = m_spriteMasks.end(); it != end; ++it)
		releaseDirect3DTexture(it->second);

	m_spriteMasks.clear();
	m_spritesMaskIds.clear();
	m_automapTilesBuff.clear();
	m_automapTiles.clear();
}

void SurfaceDirect3D9::releaseTextures()
{
	if(m_pictures)
	{
		for(Uint16 i = 0; i < g_pictureCounts; ++i)
		{
			if(m_pictures[i])
			{
				releaseDirect3DTexture(m_pictures[i]);
				m_pictures[i] = NULL;
			}
		}
	}

	for(U32BD3D9Textures::iterator it = m_automapTiles.begin(), end = m_automapTiles.end(); it != end; ++it)
		releaseDirect3DTexture(it->second);

	for(U64BD3D9Textures::iterator it = m_spriteMasks.begin(), end = m_spriteMasks.end(); it != end; ++it)
		releaseDirect3DTexture(it->second);

	m_spriteMasks.clear();
	m_spritesMaskIds.clear();
	m_automapTilesBuff.clear();
	m_automapTiles.clear();
}

unsigned char* SurfaceDirect3D9::getScreenPixels(Sint32& width, Sint32& height, bool& bgra)
{
	if(m_needReset)
		return NULL;

	D3DSURFACE_DESC desc;
	HRESULT result = IDirect3DSurface9_GetDesc(m_defaultRenderTarget, &desc);
	if(FAILED(result))
		return NULL;
	IDirect3DSurface9* surface;
	result = IDirect3DDevice9_CreateOffscreenPlainSurface(m_device, desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &surface, NULL);
	if(FAILED(result))
		return NULL;
	result = IDirect3DDevice9_GetRenderTargetData(m_device, m_defaultRenderTarget, surface);
	if(FAILED(result))
	{
		IDirect3DSurface9_Release(surface);
		return NULL;
	}

	width = SDL_static_cast(Sint32, desc.Width);
	height = SDL_static_cast(Sint32, desc.Height);

	unsigned char* pixels = SDL_reinterpret_cast(unsigned char*, SDL_malloc(width*height*4));
	if(!pixels)
	{
		IDirect3DSurface9_Release(surface);
		return NULL;
	}
	D3DLOCKED_RECT locked;
	result = IDirect3DSurface9_LockRect(surface, &locked, NULL, D3DLOCK_READONLY);
	if(FAILED(result))
	{
		IDirect3DSurface9_Release(surface);
		SDL_free(pixels);
		return NULL;
	}
	if(desc.Format == D3DFMT_X8R8G8B8 || desc.Format == D3DFMT_A8R8G8B8)
	{
		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, pixels), SDL_reinterpret_cast(const Uint8*, locked.pBits), SDL_static_cast(size_t, width*height*4));
		bgra = true;
	}
	else
	{
		IDirect3DSurface9_UnlockRect(surface);
		IDirect3DSurface9_Release(surface);
		SDL_free(pixels);
		return NULL;
	}
	IDirect3DSurface9_UnlockRect(surface);
	IDirect3DSurface9_Release(surface);
	return pixels;
}

void SurfaceDirect3D9::beginScene()
{
	if(m_needReset)
	{
		int w_width, w_height;
		SDL_GetWindowSize(g_engine.m_window, &w_width, &w_height);
		d3d9_pparams.BackBufferWidth = w_width;
		d3d9_pparams.BackBufferHeight = w_height;

		Uint32 window_flags = SDL_GetWindowFlags(g_engine.m_window);
		if(window_flags & SDL_WINDOW_FULLSCREEN && (window_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != SDL_WINDOW_FULLSCREEN_DESKTOP)
		{
			SDL_DisplayMode fullscreen_mode;
			SDL_GetWindowDisplayMode(g_engine.m_window, &fullscreen_mode);

			d3d9_pparams.Windowed = FALSE;
			if(fullscreen_mode.format == SDL_PIXELFORMAT_ARGB8888)
				d3d9_pparams.BackBufferFormat = D3DFMT_A8R8G8B8;
			else
				d3d9_pparams.BackBufferFormat = D3DFMT_X8R8G8B8;
			d3d9_pparams.FullScreen_RefreshRateInHz = fullscreen_mode.refresh_rate;
		}
		else
		{
			d3d9_pparams.Windowed = TRUE;
			d3d9_pparams.BackBufferFormat = D3DFMT_UNKNOWN;
			d3d9_pparams.FullScreen_RefreshRateInHz = 0;
		}

		resetRenderer();
		m_needReset = false;
	}

	HRESULT result = IDirect3DDevice9_BeginScene(m_device);
	if(result == D3DERR_DEVICELOST)
	{
		resetRenderer();
		IDirect3DDevice9_BeginScene(m_device);
	}
	updateRenderer();
}

void SurfaceDirect3D9::endScene()
{
	IDirect3DDevice9_EndScene(m_device);

	HRESULT result = IDirect3DDevice9_TestCooperativeLevel(m_device);
	if(result == D3DERR_DEVICELOST)
	{
		//Reset later
		return;
	}
	else if(result == D3DERR_DEVICENOTRESET)
		resetRenderer();
	if(m_deviceEx)
		IDirect3DDevice9Ex_PresentEx(m_deviceEx, NULL, NULL, NULL, NULL, (g_engine.isVsync() ? 0 : D3DPRESENT_FORCEIMMEDIATE));
	else
		IDirect3DDevice9_Present(m_device, NULL, NULL, NULL, NULL);
}

bool SurfaceDirect3D9::integer_scaling(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	Sint32 width = RENDERTARGET_WIDTH;
	Sint32 height = RENDERTARGET_HEIGHT;
	UTIL_integer_scale(width, height, w, h, m_maxTextureSize, m_maxTextureSize);
	if(m_integer_scaling_width != width || m_integer_scaling_height != height || !m_scaled_gameWindow)
	{
		if(m_scaled_gameWindow)
		{
			releaseDirect3DTexture(m_scaled_gameWindow);
			m_scaled_gameWindow = NULL;
		}

		m_scaled_gameWindow = createDirect3DTexture(width, height, true, true);
		if(!m_scaled_gameWindow)
			return false;

		m_integer_scaling_width = width;
		m_integer_scaling_height = height;
	}

	if(m_currentRenderTarget)
	{
		IDirect3DSurface9_Release(m_currentRenderTarget);
		m_currentRenderTarget = NULL;
	}
	IDirect3DTexture9_GetSurfaceLevel(m_scaled_gameWindow->m_texture, 0, &m_currentRenderTarget);
	IDirect3DDevice9_SetRenderTarget(m_device, 0, m_currentRenderTarget);

	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = m_scaled_gameWindow->m_width;
	m_viewPortH = m_scaled_gameWindow->m_height;
	updateViewport();

	float minx = -0.5f;
	float maxx = minx+SDL_static_cast(float, width);
	float miny = -0.5f;
	float maxy = miny+SDL_static_cast(float, height);

	float minu = sx*m_gameWindow->m_scaleW;
	float maxu = (sx+sw)*m_gameWindow->m_scaleW;
	float minv = sy*m_gameWindow->m_scaleH;
	float maxv = (sy+sh)*m_gameWindow->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexD3D9 vertices[4];
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
	IDirect3DDevice9_SetTexture(m_device, 0, SDL_reinterpret_cast(IDirect3DBaseTexture9*, m_gameWindow->m_texture));
	IDirect3DDevice9_SetRenderState(m_device, D3DRS_ALPHABLENDENABLE, FALSE);
	IDirect3DDevice9_DrawPrimitiveUP(m_device, D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexD3D9));
	if(m_currentRenderTarget)
	{
		IDirect3DSurface9_Release(m_currentRenderTarget);
		m_currentRenderTarget = NULL;
	}
	IDirect3DDevice9_SetRenderTarget(m_device, 0, m_defaultRenderTarget);

	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = g_engine.getWindowWidth();
	m_viewPortH = g_engine.getWindowHeight();
	updateViewport();

	minx = SDL_static_cast(float, x)-0.5f;
	maxx = minx+SDL_static_cast(float, w);
	miny = SDL_static_cast(float, y)-0.5f;
	maxy = miny+SDL_static_cast(float, h);

	minu = 0.0f;
	maxu = m_scaled_gameWindow->m_width*m_scaled_gameWindow->m_scaleW;
	minv = 0.0f;
	maxv = m_scaled_gameWindow->m_height*m_scaled_gameWindow->m_scaleH;

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
	IDirect3DDevice9_SetTexture(m_device, 0, SDL_reinterpret_cast(IDirect3DBaseTexture9*, m_scaled_gameWindow->m_texture));
	if(g_engine.isSharpening() && m_haveSharpening)
	{
		float textureDimensions[2] = {1.0f / w, 1.0f / h};
		IDirect3DDevice9_SetPixelShader(m_device, m_pixelShader_sharpen);
		IDirect3DDevice9_SetPixelShaderConstantF(m_device, 0, textureDimensions, 2);
	}
	IDirect3DDevice9_DrawPrimitiveUP(m_device, D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexD3D9));
	if(g_engine.isSharpening() && m_haveSharpening)
		IDirect3DDevice9_SetPixelShader(m_device, NULL);
	IDirect3DDevice9_SetRenderState(m_device, D3DRS_ALPHABLENDENABLE, TRUE);
	return true;
}

void SurfaceDirect3D9::drawGameScene(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
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
	float minx = SDL_static_cast(float, x)-0.5f;
	float maxx = minx+SDL_static_cast(float, w);
	float miny = SDL_static_cast(float, y)-0.5f;
	float maxy = miny+SDL_static_cast(float, h);

	float minu = sx*m_gameWindow->m_scaleW;
	float maxu = (sx+sw)*m_gameWindow->m_scaleW;
	float minv = sy*m_gameWindow->m_scaleH;
	float maxv = (sy+sh)*m_gameWindow->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexD3D9 vertices[4];
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
	IDirect3DDevice9_SetTexture(m_device, 0, SDL_reinterpret_cast(IDirect3DBaseTexture9*, m_gameWindow->m_texture));
	IDirect3DDevice9_SetRenderState(m_device, D3DRS_ALPHABLENDENABLE, FALSE);
	if(g_engine.isSharpening() && m_haveSharpening)
	{
		float textureDimensions[2] = {1.0f / w, 1.0f / h};
		IDirect3DDevice9_SetPixelShader(m_device, m_pixelShader_sharpen);
		IDirect3DDevice9_SetPixelShaderConstantF(m_device, 0, textureDimensions, 2);
	}
	IDirect3DDevice9_DrawPrimitiveUP(m_device, D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexD3D9));
	if(g_engine.isSharpening() && m_haveSharpening)
		IDirect3DDevice9_SetPixelShader(m_device, NULL);
	IDirect3DDevice9_SetRenderState(m_device, D3DRS_ALPHABLENDENABLE, TRUE);
}

void SurfaceDirect3D9::beginGameScene()
{
	if(m_currentRenderTarget)
	{
		IDirect3DSurface9_Release(m_currentRenderTarget);
		m_currentRenderTarget = NULL;
	}
	IDirect3DTexture9_GetSurfaceLevel(m_gameWindow->m_texture, 0, &m_currentRenderTarget);
	IDirect3DDevice9_SetRenderTarget(m_device, 0, m_currentRenderTarget);

	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = m_gameWindow->m_width;
	m_viewPortH = m_gameWindow->m_height;
	updateViewport();
	fillRectangle(0, 0, RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT, 0x00, 0x00, 0x00, 255);
}

void SurfaceDirect3D9::endGameScene()
{
	if(m_currentRenderTarget)
	{
		IDirect3DSurface9_Release(m_currentRenderTarget);
		m_currentRenderTarget = NULL;
	}
	IDirect3DDevice9_SetRenderTarget(m_device, 0, m_defaultRenderTarget);

	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = g_engine.getWindowWidth();
	m_viewPortH = g_engine.getWindowHeight();
	updateViewport();
}

void SurfaceDirect3D9::drawLightMap(LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height)
{
	VertexD3D9* vertices = SDL_reinterpret_cast(VertexD3D9*, SDL_malloc(sizeof(VertexD3D9)*(width+1)*2));

	IDirect3DDevice9_SetRenderState(m_device, D3DRS_SRCBLEND, D3DBLEND_ZERO);
	IDirect3DDevice9_SetRenderState(m_device, D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
	if(m_useAlphaBlending)
	{
		IDirect3DDevice9_SetRenderState(m_device, D3DRS_SRCBLENDALPHA, D3DBLEND_ZERO);
		IDirect3DDevice9_SetRenderState(m_device, D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);
	}
	IDirect3DDevice9_SetTexture(m_device, 0, SDL_reinterpret_cast(IDirect3DBaseTexture9*, NULL));

	Sint32 drawY = y-scale-(scale/2);
	height -= 1;
	for(Sint32 j = -1; j < height; ++j)
	{
		Sint32 offset1 = (j + 1) * width;
		Sint32 offset2 = UTIL_max<Sint32>(j, 0) * width;
		Sint32 drawX = x-scale-(scale/2), verticeCount = 0;
		for(Sint32 k = -1; k < width; ++k)
		{
			Sint32 offset = UTIL_max<Sint32>(k, 0);
			vertices[verticeCount].x = SDL_static_cast(float, drawX)-0.5f;
			vertices[verticeCount].y = SDL_static_cast(float, drawY+scale)-0.5f;
			vertices[verticeCount].z = 0.0f;
			vertices[verticeCount].color = MAKE_RGBA_COLOR(lightmap[offset1+offset].r, lightmap[offset1+offset].g, lightmap[offset1+offset].b, 255);
			vertices[verticeCount].u = 0.0f;
			vertices[verticeCount].v = 0.0f;
			++verticeCount;

			vertices[verticeCount].x = SDL_static_cast(float, drawX)-0.5f;
			vertices[verticeCount].y = SDL_static_cast(float, drawY)-0.5f;
			vertices[verticeCount].z = 0.0f;
			vertices[verticeCount].color = MAKE_RGBA_COLOR(lightmap[offset2+offset].r, lightmap[offset2+offset].g, lightmap[offset2+offset].b, 255);
			vertices[verticeCount].u = 0.0f;
			vertices[verticeCount].v = 0.0f;
			++verticeCount;
			drawX += scale;
		}

		drawY += scale;
		IDirect3DDevice9_DrawPrimitiveUP(m_device, D3DPT_TRIANGLESTRIP, verticeCount-2, vertices, sizeof(VertexD3D9));
	}

	IDirect3DDevice9_SetRenderState(m_device, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	IDirect3DDevice9_SetRenderState(m_device, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	if(m_useAlphaBlending)
	{
		IDirect3DDevice9_SetRenderState(m_device, D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
		IDirect3DDevice9_SetRenderState(m_device, D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
	}
	SDL_free(vertices);
}

void SurfaceDirect3D9::setClipRect(Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	IDirect3DDevice9_SetRenderState(m_device, D3DRS_SCISSORTESTENABLE, TRUE);

	RECT r;
	r.left = m_viewPortX+x;
	r.top = m_viewPortY+y;
	r.right = m_viewPortX+x+w;
	r.bottom = m_viewPortY+y+h;
	IDirect3DDevice9_SetScissorRect(m_device, &r);
}

void SurfaceDirect3D9::disableClipRect()
{
	IDirect3DDevice9_SetRenderState(m_device, D3DRS_SCISSORTESTENABLE, FALSE);
}

void SurfaceDirect3D9::drawRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w-1);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h-1);

	DWORD texColor = MAKE_RGBA_COLOR(r, g, b, a);

	VertexD3D9 vertices[5];
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

	IDirect3DDevice9_SetTexture(m_device, 0, SDL_reinterpret_cast(IDirect3DBaseTexture9*, NULL));
	IDirect3DDevice9_DrawPrimitiveUP(m_device, D3DPT_LINESTRIP, 4, vertices, sizeof(VertexD3D9));
}

void SurfaceDirect3D9::fillRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	float minx = SDL_static_cast(float, x)-0.5f;
	float maxx = minx+SDL_static_cast(float, w);
	float miny = SDL_static_cast(float, y)-0.5f;
	float maxy = miny+SDL_static_cast(float, h);
	
	DWORD texColor = MAKE_RGBA_COLOR(r, g, b, a);

	VertexD3D9 vertices[4];
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

	IDirect3DDevice9_SetTexture(m_device, 0, SDL_reinterpret_cast(IDirect3DBaseTexture9*, NULL));
	IDirect3DDevice9_DrawPrimitiveUP(m_device, D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexD3D9));
}

Direct3D9Texture* SurfaceDirect3D9::loadPicture(Uint16 pictureId, bool linear)
{
	Sint32 width, height;
	unsigned char* pixels = g_engine.LoadPicture(pictureId, true, width, height);
	if(!pixels)
		return NULL;

	Direct3D9Texture* s = createDirect3DTexture(width, height, linear);
	if(!s)
	{
		SDL_free(pixels);
		return NULL;
	}

	updateTextureData(s, pixels);
	SDL_free(pixels);
	m_pictures[pictureId] = s;
	return s;
}

void SurfaceDirect3D9::drawFont(Uint16 pictureId, Sint32 x, Sint32 y, const std::string& text, size_t pos, size_t len, Uint8 r, Uint8 g, Uint8 b, Sint32 cX[256], Sint32 cY[256], Sint32 cW[256], Sint32 cH[256])
{
	Direct3D9Texture* tex = m_pictures[pictureId];
	if(!tex)
	{
		tex = loadPicture(pictureId, false);
		if(!tex)
			return;//load failed
	}

	std::vector<VertexD3D9> vertices;
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
				float minx = SDL_static_cast(float, rx)-0.5f;
				float maxx = minx+SDL_static_cast(float, cW[character]);
				float miny = SDL_static_cast(float, ry)-0.5f;
				float maxy = miny+SDL_static_cast(float, cH[character]);

				float minu = cX[character]*tex->m_scaleW;
				float maxu = (cX[character]+cW[character])*tex->m_scaleW;
				float minv = cY[character]*tex->m_scaleH;
				float maxv = (cY[character]+cH[character])*tex->m_scaleH;

				VertexD3D9 vertice[4];
				vertice[0].x = minx;
				vertice[0].y = miny;
				vertice[0].z = 0.0f;
				vertice[0].color = texColor;
				vertice[0].u = minu;
				vertice[0].v = minv;

				vertice[1].x = minx;
				vertice[1].y = maxy;
				vertice[1].z = 0.0f;
				vertice[1].color = texColor;
				vertice[1].u = minu;
				vertice[1].v = maxv;

				vertice[2].x = maxx;
				vertice[2].y = miny;
				vertice[2].z = 0.0f;
				vertice[2].color = texColor;
				vertice[2].u = maxu;
				vertice[2].v = minv;

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
				rx += cW[character]+cX[0];
			}
			break;
		}
	}

	updateTextureScaling(tex);
	IDirect3DDevice9_SetTexture(m_device, 0, SDL_reinterpret_cast(IDirect3DBaseTexture9*, tex->m_texture));
	drawTriangles(vertices);
}

void SurfaceDirect3D9::drawBackground(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	Direct3D9Texture* tex = m_pictures[pictureId];
	if(!tex)
	{
		tex = loadPicture(pictureId, g_engine.hasAntialiasing());
		if(!tex)
			return;//load failed
	}

	float minx = SDL_static_cast(float, x)-0.5f;
	float maxx = minx+SDL_static_cast(float, w);
	float miny = SDL_static_cast(float, y)-0.5f;
	float maxy = miny+SDL_static_cast(float, h);

	float minu = sx*tex->m_scaleW;
	float maxu = (sx+sw)*tex->m_scaleW;
	float minv = sy*tex->m_scaleH;
	float maxv = (sy+sh)*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexD3D9 vertices[4];
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
	IDirect3DDevice9_SetTexture(m_device, 0, SDL_reinterpret_cast(IDirect3DBaseTexture9*, tex->m_texture));
	IDirect3DDevice9_SetRenderState(m_device, D3DRS_ALPHABLENDENABLE, FALSE);
	IDirect3DDevice9_DrawPrimitiveUP(m_device, D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexD3D9));
	IDirect3DDevice9_SetRenderState(m_device, D3DRS_ALPHABLENDENABLE, TRUE);
}

void SurfaceDirect3D9::drawPictureRepeat(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	Direct3D9Texture* tex = m_pictures[pictureId];
	if(!tex)
	{
		tex = loadPicture(pictureId, false);
		if(!tex)
			return;//load failed
	}

	std::vector<VertexD3D9> vertices;
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
			float minx = SDL_static_cast(float, cx)-0.5f;
			float maxx = minx+SDL_static_cast(float, curW);
			float miny = SDL_static_cast(float, y)-0.5f;
			float maxy = miny+SDL_static_cast(float, curH);

			float minu = sx*tex->m_scaleW;
			float maxu = (sx+curW)*tex->m_scaleW;
			float minv = sy*tex->m_scaleH;
			float maxv = (sy+curH)*tex->m_scaleH;

			VertexD3D9 vertice[4];
			vertice[0].x = minx;
			vertice[0].y = miny;
			vertice[0].z = 0.0f;
			vertice[0].color = texColor;
			vertice[0].u = minu;
			vertice[0].v = minv;

			vertice[1].x = minx;
			vertice[1].y = maxy;
			vertice[1].z = 0.0f;
			vertice[1].color = texColor;
			vertice[1].u = minu;
			vertice[1].v = maxv;

			vertice[2].x = maxx;
			vertice[2].y = miny;
			vertice[2].z = 0.0f;
			vertice[2].color = texColor;
			vertice[2].u = maxu;
			vertice[2].v = minv;

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
			cx += sw;
		}
		y += sh;
	}

	updateTextureScaling(tex);
	IDirect3DDevice9_SetTexture(m_device, 0, SDL_reinterpret_cast(IDirect3DBaseTexture9*, tex->m_texture));
	drawTriangles(vertices);
}

void SurfaceDirect3D9::drawPicture(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	Direct3D9Texture* tex = m_pictures[pictureId];
	if(!tex)
	{
		tex = loadPicture(pictureId, false);
		if(!tex)
			return;//load failed
	}

	float minx = SDL_static_cast(float, x)-0.5f;
	float maxx = minx+SDL_static_cast(float, w);
	float miny = SDL_static_cast(float, y)-0.5f;
	float maxy = miny+SDL_static_cast(float, h);

	float minu = sx*tex->m_scaleW;
	float maxu = (sx+w)*tex->m_scaleW;
	float minv = sy*tex->m_scaleH;
	float maxv = (sy+h)*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexD3D9 vertices[4];
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
	IDirect3DDevice9_SetTexture(m_device, 0, SDL_reinterpret_cast(IDirect3DBaseTexture9*, tex->m_texture));
	IDirect3DDevice9_DrawPrimitiveUP(m_device, D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexD3D9));
}

Direct3D9Texture* SurfaceDirect3D9::loadSpriteMask(Uint64 tempPos, Uint32 spriteId, Uint32 maskSpriteId, Uint32 outfitColor)
{
	unsigned char* pixels = g_engine.LoadSpriteMask(spriteId, maskSpriteId, outfitColor, true);
	if(!pixels)
		return NULL;

	if(m_spriteMasks.size() >= HARDWARE_MAX_SPRITEMASKS)
	{
		U64BD3D9Textures::iterator it = m_spriteMasks.find(m_spritesMaskIds.front());
		if(it == m_spriteMasks.end())//Something weird happen - let's erase the first one entry
			it = m_spriteMasks.begin();

		releaseDirect3DTexture(it->second);
		m_spriteMasks.erase(it);
		m_spritesMaskIds.pop_front();
	}
	Direct3D9Texture* s = createDirect3DTexture(32, 32, false);
	if(!s)
	{
		SDL_free(pixels);
		return NULL;
	}

	updateTextureData(s, pixels);
	SDL_free(pixels);

	m_spriteMasks[tempPos] = s;
	m_spritesMaskIds.push_back(tempPos);
	return s;
}

void SurfaceDirect3D9::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor)
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
	Direct3D9Texture* tex;
	U64BD3D9Textures::iterator it = m_spriteMasks.find(tempPos);
	if(it == m_spriteMasks.end())
	{
		tex = loadSpriteMask(tempPos, spriteId, maskSpriteId, outfitColor);
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

	float minx = SDL_static_cast(float, x)-0.5f;
	float maxx = minx+SDL_static_cast(float, tex->m_width);
	float miny = SDL_static_cast(float, y)-0.5f;
	float maxy = miny+SDL_static_cast(float, tex->m_height);

	float minu = 0.0f;
	float maxu = tex->m_width*tex->m_scaleW;
	float minv = 0.0f;
	float maxv = tex->m_height*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexD3D9 vertices[4];
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
	IDirect3DDevice9_SetTexture(m_device, 0, SDL_reinterpret_cast(IDirect3DBaseTexture9*, tex->m_texture));
	IDirect3DDevice9_DrawPrimitiveUP(m_device, D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexD3D9));
}

void SurfaceDirect3D9::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Uint32 outfitColor)
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
	Direct3D9Texture* tex;
	U64BD3D9Textures::iterator it = m_spriteMasks.find(tempPos);
	if(it == m_spriteMasks.end())
	{
		tex = loadSpriteMask(tempPos, spriteId, maskSpriteId, outfitColor);
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

	float minx = SDL_static_cast(float, x)-0.5f;
	float maxx = minx+SDL_static_cast(float, w);
	float miny = SDL_static_cast(float, y)-0.5f;
	float maxy = miny+SDL_static_cast(float, h);

	float minu = sx*tex->m_scaleW;
	float maxu = (sx+sw)*tex->m_scaleW;
	float minv = sy*tex->m_scaleH;
	float maxv = (sy+sh)*tex->m_scaleH;
	
	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexD3D9 vertices[4];
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
	IDirect3DDevice9_SetTexture(m_device, 0, SDL_reinterpret_cast(IDirect3DBaseTexture9*, tex->m_texture));
	IDirect3DDevice9_DrawPrimitiveUP(m_device, D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexD3D9));
}

Direct3D9Texture* SurfaceDirect3D9::createAutomapTile(Uint32 currentArea)
{
	if(m_automapTiles.size() >= HARDWARE_MAX_AUTOMAPTILES)
	{
		U32BD3D9Textures::iterator it = m_automapTiles.find(m_automapTilesBuff.front());
		if(it == m_automapTiles.end())//Something weird happen - let's erase the first one entry
			it = m_automapTiles.begin();

		releaseDirect3DTexture(it->second);
		m_automapTiles.erase(it);
		m_automapTilesBuff.pop_front();
	}
	Direct3D9Texture* s = createDirect3DTexture(256, 256, false);
	if(!s)
		return NULL;

	m_automapTiles[currentArea] = s;
	m_automapTilesBuff.push_back(currentArea);
	return s;
}

void SurfaceDirect3D9::uploadAutomapTile(Direct3D9Texture* texture, Uint8 color[256][256])
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
	updateTextureData(texture, pixels);
	SDL_free(pixels);
}

void SurfaceDirect3D9::drawAutomapTile(Uint32 currentArea, bool& recreate, Uint8 color[256][256], Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
{
	Direct3D9Texture* tex;
	U32BD3D9Textures::iterator it = m_automapTiles.find(currentArea);
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

	float minx = SDL_static_cast(float, x)-0.5f;
	float maxx = minx+SDL_static_cast(float, w);
	float miny = SDL_static_cast(float, y)-0.5f;
	float maxy = miny+SDL_static_cast(float, h);

	float minu = sx*tex->m_scaleW;
	float maxu = (sx+sw)*tex->m_scaleW;
	float minv = sy*tex->m_scaleH;
	float maxv = (sy+sh)*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexD3D9 vertices[4];
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
	IDirect3DDevice9_SetTexture(m_device, 0, SDL_reinterpret_cast(IDirect3DBaseTexture9*, tex->m_texture));
	IDirect3DDevice9_DrawPrimitiveUP(m_device, D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexD3D9));
}

SurfaceDirect3D9Comp::SurfaceDirect3D9Comp()
{
	m_sprites = NULL;
}

SurfaceDirect3D9Comp::~SurfaceDirect3D9Comp()
{
	if(m_sprites)
	{
		for(Uint32 i = 1; i <= m_spritesCache; ++i)
		{
			if(m_sprites[i])
				releaseDirect3DTexture(m_sprites[i]);
		}

		SDL_free(m_sprites);
	}
}

void SurfaceDirect3D9Comp::init()
{
	SurfaceDirect3D9::init();
	m_sprites = SDL_reinterpret_cast(Direct3D9Texture**, SDL_calloc(g_spriteCounts+1, sizeof(Direct3D9Texture*)));
	if(!m_sprites)
	{
		SDL_OutOfMemory();
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Surface::Init() Failed: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
	}
	m_spritesCache = g_spriteCounts;
}

void SurfaceDirect3D9Comp::spriteManagerReset()
{
	SurfaceDirect3D9::spriteManagerReset();
	if(m_sprites)
	{
		for(Uint32 i = 1; i <= m_spritesCache; ++i)
		{
			if(m_sprites[i])
				releaseDirect3DTexture(m_sprites[i]);
		}

		SDL_free(m_sprites);
	}
	m_sprites = SDL_reinterpret_cast(Direct3D9Texture**, SDL_calloc(g_spriteCounts+1, sizeof(Direct3D9Texture*)));
	if(!m_sprites)
	{
		SDL_OutOfMemory();
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Surface::spriteManagerReset() Failed: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
	}
	m_spritesCache = g_spriteCounts;
}

void SurfaceDirect3D9Comp::releaseTextures()
{
	SurfaceDirect3D9::releaseTextures();
	if(m_sprites)
	{
		for(Uint32 i = 1; i <= m_spritesCache; ++i)
		{
			if(m_sprites[i])
			{
				releaseDirect3DTexture(m_sprites[i]);
				m_sprites[i] = NULL;
			}
		}
	}
}

Direct3D9Texture* SurfaceDirect3D9Comp::loadSprite(Uint32 spriteId)
{
	unsigned char* pixels = g_engine.LoadSprite(spriteId, true);
	if(!pixels)
		return NULL;

	Direct3D9Texture* s = createDirect3DTexture(32, 32, false);
	if(!s)
	{
		SDL_free(pixels);
		return NULL;
	}
	m_sprites[spriteId] = s;
	updateTextureData(s, pixels);
	SDL_free(pixels);
	return s;
}

void SurfaceDirect3D9Comp::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y)
{
	if(spriteId > g_spriteCounts)
		return;

	Direct3D9Texture* tex = m_sprites[spriteId];
	if(!tex)
	{
		tex = loadSprite(spriteId);
		if(!tex)
			return;//load failed
	}

	float minx = SDL_static_cast(float, x)-0.5f;
	float maxx = minx+SDL_static_cast(float, tex->m_width);
	float miny = SDL_static_cast(float, y)-0.5f;
	float maxy = miny+SDL_static_cast(float, tex->m_height);

	float minu = 0.0f;
	float maxu = tex->m_width*tex->m_scaleW;
	float minv = 0.0f;
	float maxv = tex->m_height*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexD3D9 vertices[4];
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
	IDirect3DDevice9_SetTexture(m_device, 0, SDL_reinterpret_cast(IDirect3DBaseTexture9*, tex->m_texture));
	IDirect3DDevice9_DrawPrimitiveUP(m_device, D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexD3D9));
}

void SurfaceDirect3D9Comp::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
{
	if(spriteId > g_spriteCounts)
		return;

	Direct3D9Texture* tex = m_sprites[spriteId];
	if(!tex)
	{
		tex = loadSprite(spriteId);
		if(!tex)
			return;//load failed
	}

	float minx = SDL_static_cast(float, x)-0.5f;
	float maxx = minx+SDL_static_cast(float, w);
	float miny = SDL_static_cast(float, y)-0.5f;
	float maxy = miny+SDL_static_cast(float, h);

	float minu = sx*tex->m_scaleW;
	float maxu = (sx+sw)*tex->m_scaleW;
	float minv = sy*tex->m_scaleH;
	float maxv = (sy+sh)*tex->m_scaleH;
	
	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexD3D9 vertices[4];
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
	IDirect3DDevice9_SetTexture(m_device, 0, SDL_reinterpret_cast(IDirect3DBaseTexture9*, tex->m_texture));
	IDirect3DDevice9_DrawPrimitiveUP(m_device, D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexD3D9));
}

SurfaceDirect3D9Perf::SurfaceDirect3D9Perf()
{
	m_sprites = NULL;
	m_spriteAtlas = NULL;

	m_spriteAtlases = 0;
	m_spritesPerAtlas = 0;
	m_spritesPerModulo = 0;

	m_scheduleSpriteDraw = false;
}

SurfaceDirect3D9Perf::~SurfaceDirect3D9Perf()
{
	if(m_sprites)
		SDL_free(m_sprites);

	for(std::vector<Direct3D9Texture*>::iterator it = m_spritesAtlas.begin(), end = m_spritesAtlas.end(); it != end; ++it)
		releaseDirect3DTexture((*it));

	m_spritesAtlas.clear();
}

void SurfaceDirect3D9Perf::generateSpriteAtlases()
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
	for(std::vector<Direct3D9Texture*>::iterator it = m_spritesAtlas.begin(), end = m_spritesAtlas.end(); it != end; ++it)
		releaseDirect3DTexture((*it));

	m_spritesAtlas.clear();
	for(Uint32 i = 0; i < m_spriteAtlases; ++i)
	{
		Direct3D9Texture* texture = createDirect3DTexture(m_spritesPerModulo, m_spritesPerModulo, false);
		if(!texture)
		{
			UTIL_MessageBox(true, "Direct3D9: Out of video memory.");
			exit(-1);
			return;
		}
		m_spritesAtlas.push_back(texture);
	}
}

void SurfaceDirect3D9Perf::checkScheduledSprites()
{
	size_t vertices =  m_gameWindowVertices.size();
	if(vertices > 0)
	{
		updateTextureScaling(m_spriteAtlas);
		IDirect3DDevice9_SetTexture(m_device, 0, SDL_reinterpret_cast(IDirect3DBaseTexture9*, m_spriteAtlas->m_texture));
		drawTriangles(m_gameWindowVertices);

		m_gameWindowVertices.clear();
		m_gameWindowVertices.reserve(30720);
	}
}

void SurfaceDirect3D9Perf::renderTargetsRecreate()
{
	SurfaceDirect3D9::renderTargetsRecreate();
	generateSpriteAtlases();
}

void SurfaceDirect3D9Perf::init()
{
	SurfaceDirect3D9::init();
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

void SurfaceDirect3D9Perf::spriteManagerReset()
{
	SurfaceDirect3D9::spriteManagerReset();
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

void SurfaceDirect3D9Perf::releaseTextures()
{
	SurfaceDirect3D9::releaseTextures();
	if(m_sprites)
	{
		for(Uint32 i = 1; i <= m_spritesCache; ++i)
			m_sprites[i] = false;
	}
	for(std::vector<Direct3D9Texture*>::iterator it = m_spritesAtlas.begin(), end = m_spritesAtlas.end(); it != end; ++it)
		releaseDirect3DTexture((*it));

	m_spritesAtlas.clear();
}

void SurfaceDirect3D9Perf::beginScene()
{
	SurfaceDirect3D9::beginScene();
	m_scheduleSpriteDraw = false;
}

void SurfaceDirect3D9Perf::beginGameScene()
{
	SurfaceDirect3D9::beginGameScene();
	m_gameWindowVertices.clear();
	m_gameWindowVertices.reserve(30720);
	m_scheduleSpriteDraw = true;
}

void SurfaceDirect3D9Perf::endGameScene()
{
	checkScheduledSprites();
	m_scheduleSpriteDraw = false;
	SurfaceDirect3D9::endGameScene();
}

void SurfaceDirect3D9Perf::drawRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(m_scheduleSpriteDraw)
		checkScheduledSprites();
	SurfaceDirect3D9::drawRectangle(x, y, w, h, r, g, b, a);
}

void SurfaceDirect3D9Perf::fillRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(m_scheduleSpriteDraw)
		checkScheduledSprites();
	SurfaceDirect3D9::fillRectangle(x, y, w, h, r, g, b, a);
}

bool SurfaceDirect3D9Perf::loadSprite(Uint32 spriteId, Direct3D9Texture* texture, Uint32 xoff, Uint32 yoff)
{
	unsigned char* pixels = g_engine.LoadSprite(spriteId, true);
	if(!pixels)
		return false;

	IDirect3DTexture9* stagingTexture;
	HRESULT result = IDirect3DDevice9_CreateTexture(m_device, 32, 32, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &stagingTexture, NULL);
	if(FAILED(result))
	{
		SDL_free(pixels);
		return false;
	}

	D3DLOCKED_RECT locked;
	result = IDirect3DTexture9_LockRect(stagingTexture, 0, &locked, NULL, D3DLOCK_NOSYSLOCK);
	if(FAILED(result))
	{
		SDL_free(pixels);
		IDirect3DTexture9_Release(stagingTexture);
		return false;
	}

	unsigned char* dstData = SDL_reinterpret_cast(unsigned char*, locked.pBits);
	unsigned char* data = pixels;
	int length = 128;
	if(length == locked.Pitch)
		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, dstData), SDL_reinterpret_cast(const Uint8*, data), length*32);
	else
	{
		if(length > locked.Pitch)
			length = locked.Pitch;
		for(Uint32 row = 0; row < 32; ++row)
		{
			UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, dstData), SDL_reinterpret_cast(const Uint8*, data), length);
			data += length;
			dstData += locked.Pitch;
		}
	}
	SDL_free(pixels);

	result = IDirect3DTexture9_UnlockRect(stagingTexture, 0);
	if(FAILED(result))
	{
		IDirect3DTexture9_Release(stagingTexture);
		return false;
	}
	IDirect3DSurface9* destSurface;
	IDirect3DSurface9* stagingSurface;
	POINT destPT; destPT.x = xoff; destPT.y = yoff;
	IDirect3DTexture9_GetSurfaceLevel(stagingTexture, 0, &stagingSurface);
	IDirect3DTexture9_GetSurfaceLevel(texture->m_texture, 0, &destSurface);
	IDirect3DDevice9_UpdateSurface(m_device, stagingSurface, NULL, destSurface, &destPT);
	IDirect3DSurface9_Release(destSurface);
	IDirect3DSurface9_Release(stagingSurface);
	IDirect3DTexture9_Release(stagingTexture);
	return true;
}

void SurfaceDirect3D9Perf::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y)
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

	Direct3D9Texture* tex = m_spritesAtlas[spriteAtlas];
	if(!m_sprites[spriteId])
	{
		if(!loadSprite(spriteId, tex, xOffset, yOffset))
			return;//load failed
		m_sprites[spriteId] = true;
	}

	float minx = SDL_static_cast(float, x)-0.5f;
	float maxx = minx+32.0f;
	float miny = SDL_static_cast(float, y)-0.5f;
	float maxy = miny+32.0f;

	float minu = xOffset*tex->m_scaleW;
	float maxu = (xOffset+32)*tex->m_scaleW;
	float minv = yOffset*tex->m_scaleH;
	float maxv = (yOffset+32)*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexD3D9 vertice[4];
	vertice[0].x = minx;
	vertice[0].y = miny;
	vertice[0].z = 0.0f;
	vertice[0].color = texColor;
	vertice[0].u = minu;
	vertice[0].v = minv;

	vertice[1].x = minx;
	vertice[1].y = maxy;
	vertice[1].z = 0.0f;
	vertice[1].color = texColor;
	vertice[1].u = minu;
	vertice[1].v = maxv;

	vertice[2].x = maxx;
	vertice[2].y = miny;
	vertice[2].z = 0.0f;
	vertice[2].color = texColor;
	vertice[2].u = maxu;
	vertice[2].v = minv;

	vertice[3].x = maxx;
	vertice[3].y = maxy;
	vertice[3].z = 0.0f;
	vertice[3].color = texColor;
	vertice[3].u = maxu;
	vertice[3].v = maxv;
	if(m_scheduleSpriteDraw)
	{
		if(tex != m_spriteAtlas)
		{
			checkScheduledSprites();
			m_spriteAtlas = tex;
		}
		m_gameWindowVertices.push_back(vertice[0]);
		m_gameWindowVertices.push_back(vertice[1]);
		m_gameWindowVertices.push_back(vertice[2]);
		m_gameWindowVertices.push_back(vertice[3]);
		m_gameWindowVertices.push_back(vertice[2]);
		m_gameWindowVertices.push_back(vertice[1]);
	}
	else
	{
		updateTextureScaling(tex);
		IDirect3DDevice9_SetTexture(m_device, 0, SDL_reinterpret_cast(IDirect3DBaseTexture9*, tex->m_texture));
		IDirect3DDevice9_DrawPrimitiveUP(m_device, D3DPT_TRIANGLESTRIP, 2, vertice, sizeof(VertexD3D9));
	}
}

void SurfaceDirect3D9Perf::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
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

	Direct3D9Texture* tex = m_spritesAtlas[spriteAtlas];
	if(!m_sprites[spriteId])
	{
		if(!loadSprite(spriteId, tex, xOffset, yOffset))
			return;//load failed
		m_sprites[spriteId] = true;
	}

	float minx = SDL_static_cast(float, x)-0.5f;
	float maxx = minx+SDL_static_cast(float, w);
	float miny = SDL_static_cast(float, y)-0.5f;
	float maxy = miny+SDL_static_cast(float, h);
	
	sx += SDL_static_cast(Sint32, xOffset);
	sy += SDL_static_cast(Sint32, yOffset);

	float minu = sx*tex->m_scaleW;
	float maxu = (sx+sw)*tex->m_scaleW;
	float minv = sy*tex->m_scaleH;
	float maxv = (sy+sh)*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexD3D9 vertice[4];
	vertice[0].x = minx;
	vertice[0].y = miny;
	vertice[0].z = 0.0f;
	vertice[0].color = texColor;
	vertice[0].u = minu;
	vertice[0].v = minv;

	vertice[1].x = minx;
	vertice[1].y = maxy;
	vertice[1].z = 0.0f;
	vertice[1].color = texColor;
	vertice[1].u = minu;
	vertice[1].v = maxv;

	vertice[2].x = maxx;
	vertice[2].y = miny;
	vertice[2].z = 0.0f;
	vertice[2].color = texColor;
	vertice[2].u = maxu;
	vertice[2].v = minv;

	vertice[3].x = maxx;
	vertice[3].y = maxy;
	vertice[3].z = 0.0f;
	vertice[3].color = texColor;
	vertice[3].u = maxu;
	vertice[3].v = maxv;

	updateTextureScaling(tex);
	IDirect3DDevice9_SetTexture(m_device, 0, SDL_reinterpret_cast(IDirect3DBaseTexture9*, tex->m_texture));
	IDirect3DDevice9_DrawPrimitiveUP(m_device, D3DPT_TRIANGLESTRIP, 2, vertice, sizeof(VertexD3D9));
}

void SurfaceDirect3D9Perf::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor)
{
	if(m_scheduleSpriteDraw)
		checkScheduledSprites();
	SurfaceDirect3D9::drawSpriteMask(spriteId, maskSpriteId, x, y, outfitColor);
}
#endif
