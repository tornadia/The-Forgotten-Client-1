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

#include "surfaceDirect3D11.h"

#if defined(SDL_VIDEO_RENDER_D3D11)
#include <d3d11_1.h>

//#define _USE_D3D_SHADER_COMPILER_
#ifdef SDL_VIDEO_DRIVER_WINRT
//The WinRT part of code are copied from SDL direct3d11 renderer so I have no idea if they work

#include <DXGI.h>
#if NTDDI_VERSION > NTDDI_WIN8
#include <DXGI1_3.h>
#endif

#include <windows.ui.core.h>
#include <windows.graphics.display.h>

#if WINAPI_FAMILY == WINAPI_FAMILY_APP
#include <windows.ui.xaml.media.dxinterop.h>
/* TODO, WinRT, XAML: get the ISwapChainBackgroundPanelNative from something other than a global var */
extern ISwapChainBackgroundPanelNative * WINRT_GlobalSwapChainBackgroundPanelNative;
#endif  /* WINAPI_FAMILY == WINAPI_FAMILY_APP */

using namespace Windows::UI::Core;
using namespace Windows::Graphics::Display;

#endif  /* SDL_VIDEO_DRIVER_WINRT */

#include <SDL2/SDL_syswm.h>

#ifdef _USE_D3D_SHADER_COMPILER_
#include <d3dcompiler.h>
#pragma comment(lib,"d3dcompiler.lib")
#endif

#if defined(__cplusplus) && !defined(CINTERFACE)
#define ID3D11Device_QueryInterface(This,riid,ppvObject) (This)->QueryInterface(riid,ppvObject)
#define ID3D11Device_CreateBuffer(This,pDesc,pInitialData,ppBuffer) (This)->CreateBuffer(pDesc,pInitialData,ppBuffer)
#define ID3D11Device_CreateSamplerState(This,pSamplerDesc,ppSamplerState) (This)->CreateSamplerState(pSamplerDesc,ppSamplerState)
#define ID3D11Device_CreateRasterizerState(This,pRasterizerDesc,ppRasterizerState) (This)->CreateRasterizerState(pRasterizerDesc,ppRasterizerState)
#define ID3D11Device_CreateBlendState(This,pBlendStateDesc,ppBlendState) (This)->CreateBlendState(pBlendStateDesc,ppBlendState)
#define ID3D11Device_CreateVertexShader(This,pShaderBytecode,BytecodeLength,pClassLinkage,ppVertexShader) (This)->CreateVertexShader(pShaderBytecode,BytecodeLength,pClassLinkage,ppVertexShader)
#define ID3D11Device_CreateInputLayout(This,pInputElementDescs,NumElements,pShaderBytecodeWithInputSignature,BytecodeLength,ppInputLayout) (This)->CreateInputLayout(pInputElementDescs,NumElements,pShaderBytecodeWithInputSignature,BytecodeLength,ppInputLayout)
#define ID3D11Device_CreatePixelShader(This,pShaderBytecode,BytecodeLength,pClassLinkage,ppPixelShader) (This)->CreatePixelShader(pShaderBytecode,BytecodeLength,pClassLinkage,ppPixelShader)
#define ID3D11Device_CreateRenderTargetView(This,pResource,pDesc,ppRTView) (This)->CreateRenderTargetView(pResource,pDesc,ppRTView)
#define ID3D11Device_CreateTexture2D(This,pDesc,pInitialData,ppTexture2D) (This)->CreateTexture2D(pDesc,pInitialData,ppTexture2D)
#define ID3D11Device_CreateShaderResourceView(This,pResource,pDesc,ppSRView) (This)->CreateShaderResourceView(pResource,pDesc,ppSRView)
#define ID3D11DeviceContext_QueryInterface(This,riid,ppvObject) (This)->QueryInterface(riid,ppvObject)
#define ID3D11DeviceContext_UpdateSubresource(This,pDstResource,DstSubresource,pDstBox,pSrcData,SrcRowPitch,SrcDepthPitch) (This)->UpdateSubresource(pDstResource,DstSubresource,pDstBox,pSrcData,SrcRowPitch,SrcDepthPitch)
#define ID3D11DeviceContext_Map(This,pResource,Subresource,MapType,MapFlags,pMappedResource) (This)->Map(pResource,Subresource,MapType,MapFlags,pMappedResource)
#define ID3D11DeviceContext_Unmap(This,pResource,Subresource) (This)->Unmap(pResource,Subresource)
#define ID3D11DeviceContext_CopySubresourceRegion(This,pDstResource,DstSubresource,DstX,DstY,DstZ,pSrcResource,SrcSubresource,pSrcBox) (This)->CopySubresourceRegion(pDstResource,DstSubresource,DstX,DstY,DstZ,pSrcResource,SrcSubresource,pSrcBox)
#define ID3D11DeviceContext_IASetVertexBuffers(This,StartSlot,NumBuffers,ppVertexBuffers,pStrides,pOffsets) (This)->IASetVertexBuffers(StartSlot,NumBuffers,ppVertexBuffers,pStrides,pOffsets)
#define ID3D11DeviceContext_IASetPrimitiveTopology(This,Topology) (This)->IASetPrimitiveTopology(Topology)
#define ID3D11DeviceContext_IASetInputLayout(This,pInputLayout) (This)->IASetInputLayout(pInputLayout)
#define ID3D11DeviceContext_Draw(This,VertexCount,StartVertexLocation) (This)->Draw(VertexCount,StartVertexLocation)
#define ID3D11DeviceContext_VSSetShader(This,pVertexShader,ppClassInstances,NumClassInstances) (This)->VSSetShader(pVertexShader,ppClassInstances,NumClassInstances)
#define ID3D11DeviceContext_VSSetConstantBuffers(This,StartSlot,NumBuffers,ppConstantBuffers) (This)->VSSetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers)
#define ID3D11DeviceContext_OMSetRenderTargets(This,NumViews,ppRenderTargetViews,pDepthStencilView) (This)->OMSetRenderTargets(NumViews,ppRenderTargetViews,pDepthStencilView)
#define ID3D11DeviceContext_OMSetBlendState(This,pBlendState,BlendFactor,SampleMask) (This)->OMSetBlendState(pBlendState,BlendFactor,SampleMask)
#define ID3D11DeviceContext_RSSetViewports(This,NumViewports,pViewports) (This)->RSSetViewports(NumViewports,pViewports)
#define ID3D11DeviceContext_RSSetScissorRects(This,NumRects,pRects) (This)->RSSetScissorRects(NumRects,pRects)
#define ID3D11DeviceContext_RSSetState(This,pRasterizerState) (This)->RSSetState(pRasterizerState)
#define ID3D11DeviceContext_PSSetShader(This,pPixelShader,ppClassInstances,NumClassInstances) (This)->PSSetShader(pPixelShader,ppClassInstances,NumClassInstances)
#define ID3D11DeviceContext_PSSetShaderResources(This,StartSlot,NumViews,ppShaderResourceViews) (This)->PSSetShaderResources(StartSlot,NumViews,ppShaderResourceViews)
#define ID3D11DeviceContext_PSSetSamplers(This,StartSlot,NumSamplers,ppSamplers) (This)->PSSetSamplers(StartSlot,NumSamplers,ppSamplers)
#define ID3D11DeviceContext_PSSetConstantBuffers(This,StartSlot,NumBuffers,ppConstantBuffers) (This)->PSSetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers)
#define ID3D11Buffer_GetDesc(This,pDesc) (This)->GetDesc(pDesc)
#define ID3D11Texture2D_GetDesc(This,pDesc) (This)->GetDesc(pDesc)
#define IDXGISwapChain_SetFullscreenState(This,Fullscreen,pTarget) (This)->SetFullscreenState(Fullscreen,pTarget)
#define IDXGISwapChain_ResizeBuffers(This,BufferCount,Width,Height,NewFormat,SwapChainFlags) (This)->ResizeBuffers(BufferCount,Width,Height,NewFormat,SwapChainFlags)
#define IDXGISwapChain_GetBuffer(This,Buffer,riid,ppSurface) (This)->GetBuffer(Buffer,riid,ppSurface)
#define IDXGISwapChain_Present(This,SyncInterval,Flags) (This)->Present(SyncInterval,Flags)
#define IDXGISwapChain1_SetRotation(This,Rotation) (This)->SetRotation(Rotation)
#define IDXGIAdapter_GetDesc(This,pDesc) (This)->GetDesc(pDesc)
#define IDXGIFactory_MakeWindowAssociation(This,WindowHandle,Flags) (This)->MakeWindowAssociation(WindowHandle,Flags)
#define IDXGIFactory_EnumAdapters(This,Adapter,ppAdapter) (This)->EnumAdapters(Adapter,ppAdapter)
#define IDXGIFactory_CreateSwapChain(This,pDevice,pDesc,ppSwapChain) (This)->CreateSwapChain(pDevice,pDesc,ppSwapChain)
#define IDXGIFactory2_CreateSwapChainForCoreWindow(This,pDevice,pWindow,pDesc,pRestrictToOutput,ppSwapChain) (This)->CreateSwapChainForCoreWindow(pDevice,pWindow,pDesc,pRestrictToOutput,ppSwapChain)
#define IDXGIFactory2_CreateSwapChainForComposition(This,pDevice,pDesc,pRestrictToOutput,ppSwapChain) (This)->CreateSwapChainForComposition(pDevice,pDesc,pRestrictToOutput,ppSwapChain)
#define IDXGIFactory2_CreateSwapChainForHwnd(This,pDevice,hWnd,pDesc,pFullscreenDesc,pRestrictToOutput,ppSwapChain) (This)->CreateSwapChainForHwnd(pDevice,hWnd,pDesc,pFullscreenDesc,pRestrictToOutput,ppSwapChain)
#define IDXGIDevice1_SetMaximumFrameLatency(This,MaxLatency) (This)->SetMaximumFrameLatency(MaxLatency)
#define IUnknown_Release(This) (This)->Release()
#endif /* C++ style interface */

#define SAFE_RELEASE(X) if ((X)) {IUnknown_Release(SDL_static_cast(IUnknown*, X)); X = NULL;}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-const-variable"
#endif

static const GUID D3D11_IID_IDXGIFactory = {0x7B7166EC, 0x21C7, 0x44AE,{0xB2, 0x1A, 0xC9, 0xAE, 0x32, 0x1A, 0xE3, 0x69}};
static const GUID D3D11_IID_IDXGIFactory2 = {0x50C83A1C, 0xE072, 0x4C48,{0x87, 0xB0, 0x36, 0x30, 0xFA, 0x36, 0xA6, 0xD0}};
static const GUID D3D11_IID_ID3D11Device1 = {0xA04BFB29, 0x08EF, 0x43D6,{0xA4, 0x9C, 0xA9, 0xBD, 0xBD, 0xCB, 0xE6, 0x86}};
static const GUID D3D11_IID_ID3D11DeviceContext1 = {0xBB2C6FAA, 0xB5FB, 0x4082,{0x8E, 0x6B, 0x38, 0x8B, 0x8C, 0xFA, 0x90, 0xE1}};
static const GUID D3D11_IID_IDXGIDevice1 = {0x77DB970F, 0x6276, 0x48BA,{0xBA, 0x28, 0x07, 0x01, 0x43, 0xB4, 0x39, 0x2C}};
static const GUID D3D11_IID_ID3D11Texture2D = {0x6F15AAF2, 0xD208, 0x4E89,{0x9A, 0xB4, 0x48, 0x95, 0x35, 0xD3, 0x4F, 0x9C}};

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

extern Engine g_engine;
extern Uint32 g_spriteCounts;
extern Uint16 g_pictureCounts;
extern bool g_inited;

typedef HRESULT (WINAPI *PFN_CREATE_DXGI_FACTORY)(REFIID riid, void **ppFactory);
typedef HRESULT (WINAPI *PFN_D3D11_CREATE_DEVICE)(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext);

#ifdef _USE_D3D_SHADER_COMPILER_
HRESULT CompileShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob)
{
	if(!srcFile || !entryPoint || !profile || !blob)
		return E_INVALIDARG;

	*blob = nullptr;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS|D3DCOMPILE_OPTIMIZATION_LEVEL3;

	const D3D_SHADER_MACRO defines[] =
	{
		NULL, NULL
	};

	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(srcFile, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, profile, flags, 0, &shaderBlob, &errorBlob);
	if(FAILED(hr))
	{
		if(errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			UTIL_MessageBox(true, (char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}

		if(shaderBlob)
			shaderBlob->Release();

		return hr;
	}
	*blob = shaderBlob;
	return hr;
}
#endif

SurfaceDirect3D11::SurfaceDirect3D11() : m_automapTilesBuff(HARDWARE_MAX_AUTOMAPTILES), m_spritesMaskIds(HARDWARE_MAX_SPRITEMASKS)
{
	g_engine.RecreateWindow(false);

	m_pictures = NULL;
	m_software = "Direct3D";
	m_hardware = NULL;

	#ifdef SDL_VIDEO_DRIVER_WINRT
	m_dxgiHandle = NULL;
	m_d3d11Handle = NULL;
	#else
	m_dxgiHandle = SDL_LoadObject("dxgi.dll");
	if(m_dxgiHandle)
		m_d3d11Handle = SDL_LoadObject("d3d11.dll");
	#endif

	m_dxgiFactory = NULL;
	m_dxgiAdapter = NULL;
	m_swapChain = NULL;
	m_device = NULL;
	m_context = NULL;

	m_samplers[0] = m_samplers[1] = NULL;
	m_mainRasterizer = NULL;
	m_clippedRasterizer = NULL;
	m_inputLayout = NULL;
	m_vertexShader = NULL;
	m_pixelShaderSolid = NULL;
	m_pixelShaderTexture = NULL;

	m_gameWindow = NULL;
	m_scaled_gameWindow = NULL;
	m_pixelShaderSharpen = NULL;

	m_maxTextureSize = 0;
	m_integer_scaling_width = 0;
	m_integer_scaling_height = 0;

	m_blendBlend = NULL;
	m_blendAdd = NULL;
	m_blendMod = NULL;

	m_mainRenderTargetView = NULL;
	m_currentRenderTargetView = NULL;

	m_pixelShaderConstants = NULL;
	m_vertexShaderConstants = NULL;
	m_vertexBuffer = NULL;
	m_vertexBufferSize = 0;

	m_totalVRAM = 0;
	m_spritesCache = 0;

	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = 0;
	m_viewPortH = 0;

	m_usingLinearSample = false;
	m_needReset = true;
	m_haveSharpening = false;
	m_useOldDXGIinterface = false;

	#ifdef HAVE_CXX11_SUPPORT
	m_spriteMasks.reserve(HARDWARE_MAX_SPRITEMASKS);
	m_automapTiles.reserve(HARDWARE_MAX_AUTOMAPTILES);
	#endif
}

SurfaceDirect3D11::~SurfaceDirect3D11()
{
	if(m_hardware)
		SDL_free(m_hardware);

	if(m_pictures)
	{
		for(Uint16 i = 0; i < g_pictureCounts; ++i)
		{
			if(m_pictures[i])
				releaseDirect3DTexture(m_pictures[i]);
		}

		SDL_free(m_pictures);
	}

	for(U32BD3D11Textures::iterator it = m_automapTiles.begin(), end = m_automapTiles.end(); it != end; ++it)
		releaseDirect3DTexture(it->second);

	for(U64BD3D11Textures::iterator it = m_spriteMasks.begin(), end = m_spriteMasks.end(); it != end; ++it)
		releaseDirect3DTexture(it->second);

	m_spriteMasks.clear();
	m_spritesMaskIds.clear();
	m_automapTilesBuff.clear();
	m_automapTiles.clear();
	ID3D11Buffer* vertexBuffer = SDL_reinterpret_cast(ID3D11Buffer*, m_vertexBuffer);
	SAFE_RELEASE(vertexBuffer);

	IDXGISwapChain* swapChain = SDL_reinterpret_cast(IDXGISwapChain*, m_swapChain);
	SAFE_RELEASE(swapChain);

	ID3D11RenderTargetView* mainRenderTarget = SDL_reinterpret_cast(ID3D11RenderTargetView*, m_mainRenderTargetView);
	SAFE_RELEASE(mainRenderTarget);

	ID3D11BlendState* blendState = SDL_reinterpret_cast(ID3D11BlendState*, m_blendMod);
	SAFE_RELEASE(blendState);
	blendState = SDL_reinterpret_cast(ID3D11BlendState*, m_blendAdd);
	SAFE_RELEASE(blendState);
	blendState = SDL_reinterpret_cast(ID3D11BlendState*, m_blendBlend);
	SAFE_RELEASE(blendState);

	ID3D11RasterizerState* rasterizer = SDL_reinterpret_cast(ID3D11RasterizerState*, m_clippedRasterizer);
	SAFE_RELEASE(rasterizer);
	rasterizer = SDL_reinterpret_cast(ID3D11RasterizerState*, m_mainRasterizer);
	SAFE_RELEASE(rasterizer);

	ID3D11SamplerState* sampler = SDL_reinterpret_cast(ID3D11SamplerState*, m_samplers[1]);
	SAFE_RELEASE(sampler);
	sampler = SDL_reinterpret_cast(ID3D11SamplerState*, m_samplers[0]);
	SAFE_RELEASE(sampler);

	ID3D11PixelShader* pixelShader = SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderTexture);
	SAFE_RELEASE(pixelShader);
	pixelShader = SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderSolid);
	SAFE_RELEASE(pixelShader);
	pixelShader = SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderSharpen);
	SAFE_RELEASE(pixelShader);

	ID3D11Buffer* pixelShaderConstant = SDL_reinterpret_cast(ID3D11Buffer*, m_pixelShaderConstants);
	SAFE_RELEASE(pixelShaderConstant);
	ID3D11Buffer* vertexShaderConstant = SDL_reinterpret_cast(ID3D11Buffer*, m_vertexShaderConstants);
	SAFE_RELEASE(vertexShaderConstant);
	ID3D11InputLayout* inputLayout = SDL_reinterpret_cast(ID3D11InputLayout*, m_inputLayout);
	SAFE_RELEASE(inputLayout);
	ID3D11VertexShader* vertexShader = SDL_reinterpret_cast(ID3D11VertexShader*, m_vertexShader);
	SAFE_RELEASE(vertexShader);
	
	IDXGIAdapter* dxgiAdapter = SDL_reinterpret_cast(IDXGIAdapter*, m_dxgiAdapter);
	SAFE_RELEASE(dxgiAdapter);
	IDXGIFactory* dxgiFactory = SDL_reinterpret_cast(IDXGIFactory*, m_dxgiFactory);
	SAFE_RELEASE(dxgiFactory);
	ID3D11Device* d3dDevice = SDL_reinterpret_cast(ID3D11Device*, m_device);
	SAFE_RELEASE(d3dDevice);
	ID3D11DeviceContext* d3dContext = SDL_reinterpret_cast(ID3D11DeviceContext*, m_context);
	SAFE_RELEASE(d3dContext);
	if(m_scaled_gameWindow)
		releaseDirect3DTexture(m_scaled_gameWindow);

	if(m_gameWindow)
		releaseDirect3DTexture(m_gameWindow);

	if(m_d3d11Handle)
		SDL_UnloadObject(m_d3d11Handle);

	if(m_dxgiHandle)
		SDL_UnloadObject(m_dxgiHandle);
}

Direct3D11Texture* SurfaceDirect3D11::createDirect3DTexture(Uint32 width, Uint32 height, bool linearSampler, bool frameBuffer)
{
	Direct3D11Texture* texture = new Direct3D11Texture();
	if(!texture)
		return NULL;

	D3D11_TEXTURE2D_DESC textureDesc;
	SDL_zero(textureDesc);
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.MiscFlags = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.CPUAccessFlags = 0;
	if(frameBuffer)
		textureDesc.BindFlags = (D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_RENDER_TARGET);
	else
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	HRESULT result = ID3D11Device_CreateTexture2D(SDL_reinterpret_cast(ID3D11Device*, m_device), &textureDesc, NULL, SDL_reinterpret_cast(ID3D11Texture2D**, &texture->m_texture));
	if(FAILED(result))
	{
		releaseDirect3DTexture(texture);
		return NULL;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;
	resourceViewDesc.Format = textureDesc.Format;
	resourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resourceViewDesc.Texture2D.MostDetailedMip = 0;
	resourceViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
	result = ID3D11Device_CreateShaderResourceView(SDL_reinterpret_cast(ID3D11Device*, m_device), SDL_reinterpret_cast(ID3D11Resource*, texture->m_texture), &resourceViewDesc, SDL_reinterpret_cast(ID3D11ShaderResourceView**, &texture->m_resource));
	if(FAILED(result))
	{
		releaseDirect3DTexture(texture);
		return NULL;
	}

	if(frameBuffer)
	{
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;
		result = ID3D11Device_CreateRenderTargetView(SDL_reinterpret_cast(ID3D11Device*, m_device), SDL_reinterpret_cast(ID3D11Resource*, texture->m_texture), &renderTargetViewDesc, SDL_reinterpret_cast(ID3D11RenderTargetView**, &texture->m_frameBuffer));
		if(FAILED(result))
		{
			releaseDirect3DTexture(texture);
			return NULL;
		}
	}
	texture->m_width = width;
	texture->m_height = height;
	texture->m_scaleW = 1.0f/width;
	texture->m_scaleH = 1.0f/height;
	texture->m_linearSample = linearSampler;
	return texture;
}

bool SurfaceDirect3D11::updateTextureData(Direct3D11Texture* texture, unsigned char* data)
{
	ID3D11Texture2D* stagingTexture;
	D3D11_TEXTURE2D_DESC stagingTextureDesc;
	ID3D11Texture2D_GetDesc(SDL_reinterpret_cast(ID3D11Texture2D*, texture->m_texture), &stagingTextureDesc);
	stagingTextureDesc.Width = texture->m_width;
	stagingTextureDesc.Height = texture->m_height;
	stagingTextureDesc.BindFlags = 0;
	stagingTextureDesc.MiscFlags = 0;
	stagingTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	stagingTextureDesc.Usage = D3D11_USAGE_STAGING;

	HRESULT result = ID3D11Device_CreateTexture2D(SDL_reinterpret_cast(ID3D11Device*, m_device), &stagingTextureDesc, NULL, SDL_reinterpret_cast(ID3D11Texture2D**, &stagingTexture));
	if(FAILED(result))
		return false;

	D3D11_MAPPED_SUBRESOURCE textureMemory;
	result = ID3D11DeviceContext_Map(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, stagingTexture), 0, D3D11_MAP_WRITE, 0, &textureMemory);
	if(FAILED(result))
	{
		SAFE_RELEASE(stagingTexture);
		return false;
	}

	unsigned char* dstData = SDL_reinterpret_cast(unsigned char*, textureMemory.pData);
	UINT length = texture->m_width*4;
	if(length == textureMemory.RowPitch)
		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, dstData), SDL_reinterpret_cast(const Uint8*, data), length*texture->m_height);
	else
	{
		if(length > textureMemory.RowPitch)
			length = textureMemory.RowPitch;
		for(Uint32 row = 0; row < texture->m_height; ++row)
		{
			UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, dstData), SDL_reinterpret_cast(const Uint8*, data), length);
			data += length;
			dstData += textureMemory.RowPitch;
		}
	}
	ID3D11DeviceContext_Unmap(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, stagingTexture), 0);
	ID3D11DeviceContext_CopySubresourceRegion(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, texture->m_texture), 0, 0, 0, 0, SDL_reinterpret_cast(ID3D11Resource*, stagingTexture), 0, NULL);
	SAFE_RELEASE(stagingTexture);
	return true;
}

void SurfaceDirect3D11::releaseDirect3DTexture(Direct3D11Texture* texture)
{
	if(texture->m_texture)
	{
		ID3D11Texture2D* tex = SDL_reinterpret_cast(ID3D11Texture2D*, texture->m_texture);
		SAFE_RELEASE(tex);
	}
	if(texture->m_resource)
	{
		ID3D11ShaderResourceView* res = SDL_reinterpret_cast(ID3D11ShaderResourceView*, texture->m_resource);
		SAFE_RELEASE(res);
	}
	if(texture->m_frameBuffer)
	{
		ID3D11RenderTargetView* frameBuffer = SDL_reinterpret_cast(ID3D11RenderTargetView*, texture->m_frameBuffer);
		SAFE_RELEASE(frameBuffer);
	}
	delete texture;
}

void SurfaceDirect3D11::updateTextureScaling(Direct3D11Texture* texture)
{
	if(m_usingLinearSample != texture->m_linearSample)
	{
		m_usingLinearSample = texture->m_linearSample;
		ID3D11SamplerState* sampler = SDL_reinterpret_cast(ID3D11SamplerState*, (m_usingLinearSample ? m_samplers[1] : m_samplers[0]));
		ID3D11DeviceContext_PSSetSamplers(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, &sampler);
	}
}

void SurfaceDirect3D11::drawTriangles(std::vector<VertexD3D11>& vertices)
{
	UINT verticesSize = SDL_static_cast(UINT, vertices.size());
	if(verticesSize > 2046)
	{
		VertexD3D11* verticeData = &vertices[0];

		UINT verticeIndex = 0;
		ID3D11DeviceContext_IASetPrimitiveTopology(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		while(verticeIndex < verticesSize)
		{
			UINT drawSize = UTIL_min<UINT>(verticesSize-verticeIndex, 2046);
			if(!updateVertexBuffer(SDL_reinterpret_cast(void*, &verticeData[verticeIndex]), sizeof(VertexD3D11)*drawSize))
				return;

			ID3D11DeviceContext_Draw(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), drawSize, 0);
			verticeIndex += drawSize;
		}
	}
	else if(verticesSize > 0)
	{
		if(!updateVertexBuffer(&vertices[0], sizeof(VertexD3D11)*verticesSize))
			return;

		ID3D11DeviceContext_IASetPrimitiveTopology(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ID3D11DeviceContext_Draw(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), verticesSize, 0);
	}
}

#ifdef SDL_VIDEO_DRIVER_WINRT
IUnknown* D3D11_GetCoreWindowFromSDLWindow(SDL_Window* window)
{
	if(!window)
		return NULL;

	SDL_SysWMinfo sdlWindowInfo;
	SDL_VERSION(&sdlWindowInfo.version);
	if(!SDL_GetWindowWMInfo(window, &sdlWindowInfo))
		return NULL;

	if(sdlWindowInfo.subsystem != SDL_SYSWM_WINRT)
		return NULL;

	if(!sdlWindowInfo.info.winrt.window)
		return NULL;

	ABI::Windows::UI::Core::ICoreWindow* coreWindow = NULL;
	if(FAILED(sdlWindowInfo.info.winrt.window->QueryInterface(&coreWindow)))
		return NULL;

	IUnknown* coreWindowAsIUnknown = NULL;
	coreWindow->QueryInterface(&coreWindowAsIUnknown);
	coreWindow->Release();
	return coreWindowAsIUnknown;
}

static DXGI_MODE_ROTATION D3D11_GetCurrentRotation()
{
	const DisplayOrientations currentOrientation = WINRT_DISPLAY_PROPERTY(CurrentOrientation);
	switch(currentOrientation)
	{
		#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
		case DisplayOrientations::Landscape:
			return DXGI_MODE_ROTATION_ROTATE90;
		case DisplayOrientations::Portrait:
			return DXGI_MODE_ROTATION_IDENTITY;
		case DisplayOrientations::LandscapeFlipped:
			return DXGI_MODE_ROTATION_ROTATE270;
		case DisplayOrientations::PortraitFlipped:
			return DXGI_MODE_ROTATION_ROTATE180;
		#else
		case DisplayOrientations::Landscape:
			return DXGI_MODE_ROTATION_IDENTITY;
		case DisplayOrientations::Portrait:
			return DXGI_MODE_ROTATION_ROTATE270;
		case DisplayOrientations::LandscapeFlipped:
			return DXGI_MODE_ROTATION_ROTATE180;
		case DisplayOrientations::PortraitFlipped:
			return DXGI_MODE_ROTATION_ROTATE90;
		#endif
	}

	return DXGI_MODE_ROTATION_IDENTITY;
}
#else
static DXGI_MODE_ROTATION D3D11_GetCurrentRotation()
{
	return DXGI_MODE_ROTATION_IDENTITY;
}
#endif

static bool D3D11_IsDisplayRotated90Degrees(DXGI_MODE_ROTATION rotation)
{
	switch(rotation)
	{
		case DXGI_MODE_ROTATION_ROTATE90:
		case DXGI_MODE_ROTATION_ROTATE270:
			return true;
		default:
			return false;
	}
}

HRESULT SurfaceDirect3D11::createSwapChain(int w, int h)
{
	if(m_useOldDXGIinterface)
	{
		#ifdef SDL_VIDEO_DRIVER_WINDOWS
		SDL_SysWMinfo windowinfo;
		SDL_VERSION(&windowinfo.version);
		SDL_GetWindowWMInfo(g_engine.m_window, &windowinfo);

		SDL_DisplayMode fullscreen_mode;
		//Uint32 window_flags = SDL_GetWindowFlags(g_engine.m_window);
		SDL_GetWindowDisplayMode(g_engine.m_window, &fullscreen_mode);

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		swapChainDesc.BufferDesc.Width = w;
		swapChainDesc.BufferDesc.Height = h;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = fullscreen_mode.refresh_rate;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;
		swapChainDesc.OutputWindow = windowinfo.info.win.window;
		swapChainDesc.Windowed = TRUE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
		swapChainDesc.Flags = 0;

		HRESULT result = IDXGIFactory_CreateSwapChain(SDL_reinterpret_cast(IDXGIFactory*, m_dxgiFactory), SDL_reinterpret_cast(IUnknown*, m_device), &swapChainDesc, SDL_reinterpret_cast(IDXGISwapChain**, &m_swapChain));
		if(FAILED(result))
			return result;

		/*For some reason making the swapchain going fullscreen state make the whole system unresponsible so I disabled it
		It doesn't matter if I use SetFullscreenState or the Windowed variable in swapchain creation
		if(window_flags & SDL_WINDOW_FULLSCREEN && (window_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != SDL_WINDOW_FULLSCREEN_DESKTOP)
		{
			IDXGISwapChain_SetFullscreenState(SDL_reinterpret_cast(IDXGISwapChain*, m_swapChain), TRUE, NULL);
		}*/
		IDXGIFactory_MakeWindowAssociation(SDL_reinterpret_cast(IDXGIFactory*, m_dxgiFactory), windowinfo.info.win.window, DXGI_MWA_NO_WINDOW_CHANGES);
		return S_OK;
		#else
		return E_NOINTERFACE;
		#endif
	}

	#ifdef SDL_VIDEO_DRIVER_WINRT
	IUnknown* coreWindow = D3D11_GetCoreWindowFromSDLWindow(g_engine.m_window);
	const bool usingXAML = (coreWindow == NULL);
	#else
	IUnknown* coreWindow = NULL;
	const bool usingXAML = false;
	#endif

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	swapChainDesc.Width = w;
	swapChainDesc.Height = h;
	swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
	#else
	if(usingXAML)
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	else
		swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	#endif
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = 0;

	if(coreWindow)
	{
		HRESULT result = IDXGIFactory2_CreateSwapChainForCoreWindow(SDL_reinterpret_cast(IDXGIFactory2*, m_dxgiFactory), SDL_reinterpret_cast(IUnknown*, m_device), coreWindow, &swapChainDesc, NULL, SDL_reinterpret_cast(IDXGISwapChain1**, &m_swapChain));
		if(FAILED(result))
			return result;
	}
	else if(usingXAML)
	{
		HRESULT result = IDXGIFactory2_CreateSwapChainForComposition(SDL_reinterpret_cast(IDXGIFactory2*, m_dxgiFactory), SDL_reinterpret_cast(IUnknown*, m_device), &swapChainDesc, NULL, SDL_reinterpret_cast(IDXGISwapChain1**, &m_swapChain));
		if(FAILED(result))
			return result;

		#if WINAPI_FAMILY == WINAPI_FAMILY_APP
		result = ISwapChainBackgroundPanelNative_SetSwapChain(WINRT_GlobalSwapChainBackgroundPanelNative, SDL_reinterpret_cast(IDXGISwapChain*, m_swapChain));
		if(FAILED(result))
			return result;
		#else
		return E_NOINTERFACE;
		#endif
	}
	else
	{
		#ifdef SDL_VIDEO_DRIVER_WINDOWS
		SDL_SysWMinfo windowinfo;
		SDL_VERSION(&windowinfo.version);
		SDL_GetWindowWMInfo(g_engine.m_window, &windowinfo);

		HRESULT result = IDXGIFactory2_CreateSwapChainForHwnd(SDL_reinterpret_cast(IDXGIFactory2*, m_dxgiFactory), SDL_reinterpret_cast(IUnknown*, m_device), windowinfo.info.win.window, &swapChainDesc, NULL, NULL, SDL_reinterpret_cast(IDXGISwapChain1**, &m_swapChain));
		if(FAILED(result))
			return result;
		IDXGIFactory_MakeWindowAssociation(SDL_reinterpret_cast(IDXGIFactory2*, m_dxgiFactory), windowinfo.info.win.window, DXGI_MWA_NO_WINDOW_CHANGES);
		#else
		return E_NOINTERFACE;
		#endif
	}
	SAFE_RELEASE(coreWindow);
	return S_OK;
}

void SurfaceDirect3D11::createWindowSizeDependentResources()
{
	ID3D11DeviceContext_OMSetRenderTargets(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, NULL, NULL);

	ID3D11RenderTargetView* mainRenderTarget = SDL_reinterpret_cast(ID3D11RenderTargetView*, m_mainRenderTargetView);
	SAFE_RELEASE(mainRenderTarget);

	int w_width, w_height;
	SDL_GetWindowSize(g_engine.m_window, &w_width, &w_height);

	DXGI_MODE_ROTATION rotation = D3D11_GetCurrentRotation();
	if(D3D11_IsDisplayRotated90Degrees(rotation))
	{
		int tmp = w_width;
		w_width = w_height;
		w_height = tmp;
	}

	HRESULT result;
	if(m_swapChain)
	{
		#if !defined(SDL_VIDEO_DRIVER_WINRT) || (WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP)
		result = IDXGISwapChain_ResizeBuffers(SDL_reinterpret_cast(IDXGISwapChain*, m_swapChain), 0, w_width, w_height, DXGI_FORMAT_UNKNOWN, 0);
		if(result == DXGI_ERROR_DEVICE_RESET || result == DXGI_ERROR_DEVICE_REMOVED)
		{
			//g_inited = false should recreate the whole d3d11 interface in next frame
			g_inited = false;
			return;
		}
		else if(FAILED(result))
		{
			UTIL_MessageBox(true, "Direct3D11: Failed to resize swapchain.");
			exit(-1);
			return;
		}
		#endif
	}
	else
	{
		result = createSwapChain(w_width, w_height);
		if(FAILED(result))
		{
			UTIL_MessageBox(true, "Direct3D11: Failed to create swapchain.");
			exit(-1);
			return;
		}
	}

	#if WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP
	if(!m_useOldDXGIinterface)
	{
		result = IDXGISwapChain1_SetRotation(SDL_reinterpret_cast(IDXGISwapChain1*, m_swapChain), rotation);
		if(FAILED(result))
		{
			UTIL_MessageBox(true, "Direct3D11: Failed to set rotation.");
			exit(-1);
			return;
		}
	}
	#endif

	ID3D11Texture2D* backBuffer;
	result = IDXGISwapChain_GetBuffer(SDL_reinterpret_cast(IDXGISwapChain*, m_swapChain), 0, IID_ID3D11Texture2D, SDL_reinterpret_cast(void**, &backBuffer));
	if(FAILED(result))
	{
		UTIL_MessageBox(true, "Direct3D11: Failed to get backBuffer.");
		exit(-1);
		return;
	}
	
	result = ID3D11Device_CreateRenderTargetView(SDL_reinterpret_cast(ID3D11Device*, m_device), SDL_static_cast(ID3D11Resource*, backBuffer), NULL, SDL_reinterpret_cast(ID3D11RenderTargetView**, &m_mainRenderTargetView));
	if(FAILED(result))
	{
		SAFE_RELEASE(backBuffer);
		UTIL_MessageBox(true, "Direct3D11: Failed to create backBuffer frameBuffer.");
		exit(-1);
		return;
	}
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = g_engine.getWindowWidth();
	m_viewPortH = g_engine.getWindowHeight();
	updateViewport();
	SAFE_RELEASE(backBuffer);
	m_needReset = false;
}

void SurfaceDirect3D11::updateViewport()
{
	if(m_viewPortW == 0 || m_viewPortH == 0)
		return;

	DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_IDENTITY;
	if(!m_currentRenderTargetView || m_currentRenderTargetView == m_mainRenderTargetView)
		rotation = D3D11_GetCurrentRotation();

	float p_11, p_12, p_13, p_14;
	float p_21, p_22, p_23, p_24;
	float p_31, p_32, p_33, p_34;
	float p_41, p_42, p_43, p_44;
	switch(rotation)
	{
		//Calculate projection
		case DXGI_MODE_ROTATION_IDENTITY:
		{
			p_11 = p_22 = p_33 = p_44 = 1.0f;
			p_12 = p_21 = p_32 = p_43 = 0.0f;
			p_13 = p_24 = p_31 = p_42 = 0.0f;
			p_14 = p_23 = p_34 = p_41 = 0.0f;
		}
		break;
		case DXGI_MODE_ROTATION_ROTATE270:
		{
			float r = SDL_static_cast(float, M_PI*0.5f);
			float sinR = SDL_sinf(r);
			float cosR = SDL_cosf(r);

			p_11 = p_22 = cosR;
			p_12 = sinR; p_21 = -sinR;
			p_13 = p_14 = 0.0f;
			p_23 = p_24 = 0.0f;
			p_31 = p_32 = p_34 = 0.0f;
			p_33 = p_44 = 1.0f;
			p_41 = p_42 = p_43 = 0.0f;
		}
		break;
		case DXGI_MODE_ROTATION_ROTATE180:
		{
			float r = SDL_static_cast(float, M_PI);
			float sinR = SDL_sinf(r);
			float cosR = SDL_cosf(r);

			p_11 = p_22 = cosR;
			p_12 = sinR; p_21 = -sinR;
			p_13 = p_14 = 0.0f;
			p_23 = p_24 = 0.0f;
			p_31 = p_32 = p_34 = 0.0f;
			p_33 = p_44 = 1.0f;
			p_41 = p_42 = p_43 = 0.0f;
		}
		break;
		case DXGI_MODE_ROTATION_ROTATE90:
		{
			float r = SDL_static_cast(float, -M_PI*0.5f);
			float sinR = SDL_sinf(r);
			float cosR = SDL_cosf(r);

			p_11 = p_22 = cosR;
			p_12 = sinR; p_21 = -sinR;
			p_13 = p_14 = 0.0f;
			p_23 = p_24 = 0.0f;
			p_31 = p_32 = p_34 = 0.0f;
			p_33 = p_44 = 1.0f;
			p_41 = p_42 = p_43 = 0.0f;
		}
		break;
		default: return;
	}

	//Calculate view
	float v_11, v_12, v_13, v_14;
	float v_21, v_22, v_23, v_24;
	float v_31, v_32, v_33, v_34;
	float v_41, v_42, v_43, v_44;
	v_11 = 2.0f/m_viewPortW; v_12 = 0.0f; v_13 = 0.0f; v_14 = 0.0f;
	v_21 = 0.0f; v_22 = -2.0f/m_viewPortH; v_23 = 0.0f; v_24 = 0.0f;
	v_31 = 0.0f; v_32 = 0.0f; v_33 = 1.0f; v_34 = 0.0f;
	v_41 = -1.0f; v_42 = 1.0f; v_43 = 0.0f; v_44 = 1.0f;

	//Calculate projection and view
	m_vertexShaderConstantsData.pv_11 = v_11 * p_11 + v_12 * p_21 + v_13 * p_31 + v_14 * p_41;
	m_vertexShaderConstantsData.pv_12 = v_11 * p_12 + v_12 * p_22 + v_13 * p_32 + v_14 * p_42;
	m_vertexShaderConstantsData.pv_13 = v_11 * p_13 + v_12 * p_23 + v_13 * p_33 + v_14 * p_43;
	m_vertexShaderConstantsData.pv_14 = v_11 * p_14 + v_12 * p_24 + v_13 * p_34 + v_14 * p_44;
	m_vertexShaderConstantsData.pv_21 = v_21 * p_11 + v_22 * p_21 + v_23 * p_31 + v_24 * p_41;
	m_vertexShaderConstantsData.pv_22 = v_21 * p_12 + v_22 * p_22 + v_23 * p_32 + v_24 * p_42;
	m_vertexShaderConstantsData.pv_23 = v_21 * p_13 + v_22 * p_23 + v_23 * p_33 + v_24 * p_43;
	m_vertexShaderConstantsData.pv_24 = v_21 * p_14 + v_22 * p_24 + v_23 * p_34 + v_24 * p_44;
	m_vertexShaderConstantsData.pv_31 = v_31 * p_11 + v_32 * p_21 + v_33 * p_31 + v_34 * p_41;
	m_vertexShaderConstantsData.pv_32 = v_31 * p_12 + v_32 * p_22 + v_33 * p_32 + v_34 * p_42;
	m_vertexShaderConstantsData.pv_33 = v_31 * p_13 + v_32 * p_23 + v_33 * p_33 + v_34 * p_43;
	m_vertexShaderConstantsData.pv_34 = v_31 * p_14 + v_32 * p_24 + v_33 * p_34 + v_34 * p_44;
	m_vertexShaderConstantsData.pv_41 = v_41 * p_11 + v_42 * p_21 + v_43 * p_31 + v_44 * p_41;
	m_vertexShaderConstantsData.pv_42 = v_41 * p_12 + v_42 * p_22 + v_43 * p_32 + v_44 * p_42;
	m_vertexShaderConstantsData.pv_43 = v_41 * p_13 + v_42 * p_23 + v_43 * p_33 + v_44 * p_43;
	m_vertexShaderConstantsData.pv_44 = v_41 * p_14 + v_42 * p_24 + v_43 * p_34 + v_44 * p_44;
	ID3D11DeviceContext_UpdateSubresource(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, m_vertexShaderConstants), 0, NULL, &m_vertexShaderConstantsData, 0, 0);

	D3D11_VIEWPORT viewport;
	if(D3D11_IsDisplayRotated90Degrees(rotation))
	{
		viewport.TopLeftX = SDL_static_cast(float, m_viewPortY);
		viewport.TopLeftY = SDL_static_cast(float, m_viewPortX);
		viewport.Width = SDL_static_cast(float, m_viewPortH);
		viewport.Height = SDL_static_cast(float, m_viewPortW);
	}
	else
	{
		viewport.TopLeftX = SDL_static_cast(float, m_viewPortX);
		viewport.TopLeftY = SDL_static_cast(float, m_viewPortY);
		viewport.Width = SDL_static_cast(float, m_viewPortW);
		viewport.Height = SDL_static_cast(float, m_viewPortH);
	}
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	ID3D11DeviceContext_RSSetViewports(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 1, &viewport);
}

bool SurfaceDirect3D11::updateVertexBuffer(const void* vertexData, size_t dataSize)
{
	if(m_vertexBuffer && m_vertexBufferSize >= dataSize)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT result = ID3D11DeviceContext_Map(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, m_vertexBuffer), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if(FAILED(result))
			return false;

		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, mappedResource.pData), SDL_reinterpret_cast(const Uint8*, vertexData), dataSize);
		ID3D11DeviceContext_Unmap(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, m_vertexBuffer), 0);
	}
	else
	{
		D3D11_BUFFER_DESC vertexBufferDesc;
		if(m_vertexBuffer)
			ID3D11Buffer_GetDesc(SDL_reinterpret_cast(ID3D11Buffer*, m_vertexBuffer), &vertexBufferDesc);
		else
			SDL_zero(vertexBufferDesc);

		ID3D11Buffer* vertexBuffer = SDL_reinterpret_cast(ID3D11Buffer*, m_vertexBuffer);
		SAFE_RELEASE(vertexBuffer);

		vertexBufferDesc.ByteWidth = SDL_static_cast(UINT, dataSize);
		vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA vertexBufferData;
		SDL_zero(vertexBufferData);
		vertexBufferData.pSysMem = vertexData;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;

		HRESULT result = ID3D11Device_CreateBuffer(SDL_reinterpret_cast(ID3D11Device*, m_device), &vertexBufferDesc, &vertexBufferData, SDL_reinterpret_cast(ID3D11Buffer**, &m_vertexBuffer));
		if(FAILED(result))
			return false;

		const UINT stride = sizeof(VertexD3D11);
		const UINT offset = 0;
		ID3D11DeviceContext_IASetVertexBuffers(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, SDL_reinterpret_cast(ID3D11Buffer**, &m_vertexBuffer), &stride, &offset);
		m_vertexBufferSize = dataSize;
	}
	return true;
}

bool SurfaceDirect3D11::createVertexShader(void* d3dDevice, void** vertexShader, void** inputLayout)
{
	const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	
	HRESULT result = ID3D11Device_CreateVertexShader(SDL_reinterpret_cast(ID3D11Device*, d3dDevice), D3D11_VertexShader, sizeof(D3D11_VertexShader), NULL, SDL_reinterpret_cast(ID3D11VertexShader**, vertexShader));
	if(FAILED(result))
		 return false;
	result = ID3D11Device_CreateInputLayout(SDL_reinterpret_cast(ID3D11Device*, d3dDevice), vertexDesc, ARRAYSIZE(vertexDesc), D3D11_VertexShader, sizeof(D3D11_VertexShader), SDL_reinterpret_cast(ID3D11InputLayout**, inputLayout));
	if(FAILED(result))
		return false;
	return true;
}

bool SurfaceDirect3D11::createPixelShader(void* d3dDevice, const void* shader_data, size_t shader_size, void** pixelShader)
{
	HRESULT result = ID3D11Device_CreatePixelShader(SDL_reinterpret_cast(ID3D11Device*, d3dDevice), shader_data, shader_size, NULL, SDL_reinterpret_cast(ID3D11PixelShader**, pixelShader));
	if(FAILED(result))
		return false;
	return true;
}

bool SurfaceDirect3D11::isSupported()
{
	PFN_CREATE_DXGI_FACTORY CreateDXGIFactoryFunc;
	PFN_D3D11_CREATE_DEVICE D3D11CreateDeviceFunc;

	#ifdef SDL_VIDEO_DRIVER_WINRT
	CreateDXGIFactoryFunc = CreateDXGIFactory1;
	D3D11CreateDeviceFunc = D3D11CreateDevice;
	#else
	if(!m_dxgiHandle || !m_d3d11Handle)
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create Direct3D11 interface.");
		return false;
	}

	CreateDXGIFactoryFunc = SDL_reinterpret_cast(PFN_CREATE_DXGI_FACTORY, SDL_LoadFunction(m_dxgiHandle, "CreateDXGIFactory"));
	if(!CreateDXGIFactoryFunc)
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to fetch 'CreateDXGIFactory' function.");
		return false;
	}

	D3D11CreateDeviceFunc = SDL_reinterpret_cast(PFN_D3D11_CREATE_DEVICE, SDL_LoadFunction(m_d3d11Handle, "D3D11CreateDevice"));
	if(!D3D11CreateDeviceFunc)
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to fetch 'D3D11CreateDevice' function.");
		return false;
	}
	#endif
	
	HRESULT result = CreateDXGIFactoryFunc(D3D11_IID_IDXGIFactory2, &m_dxgiFactory);
	if(FAILED(result))
	{
		m_useOldDXGIinterface = true;
		result = CreateDXGIFactoryFunc(D3D11_IID_IDXGIFactory, &m_dxgiFactory);
		if(FAILED(result))
		{
			UTIL_MessageBox(false, "Direct3D11: Failed to create DXGI Factory.");
			return false;
		}
	}

	result = IDXGIFactory_EnumAdapters(SDL_reinterpret_cast(IDXGIFactory*, m_dxgiFactory), 0, SDL_reinterpret_cast(IDXGIAdapter**, &m_dxgiAdapter));
	if(FAILED(result))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to get device adapter.");
		return false;
	}

	D3D_FEATURE_LEVEL featureLevel_11_1 = D3D_FEATURE_LEVEL_11_1;
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	D3D_FEATURE_LEVEL featureLevel;
	result = D3D11CreateDeviceFunc(SDL_reinterpret_cast(IDXGIAdapter*, m_dxgiAdapter), D3D_DRIVER_TYPE_UNKNOWN, NULL, (D3D11_CREATE_DEVICE_SINGLETHREADED|D3D11_CREATE_DEVICE_BGRA_SUPPORT), &featureLevel_11_1, 1, D3D11_SDK_VERSION, SDL_reinterpret_cast(ID3D11Device**, &m_device), &featureLevel, SDL_reinterpret_cast(ID3D11DeviceContext**, &m_context));
	if(FAILED(result))
	{
		result = D3D11CreateDeviceFunc(SDL_reinterpret_cast(IDXGIAdapter*, m_dxgiAdapter), D3D_DRIVER_TYPE_UNKNOWN, NULL, (D3D11_CREATE_DEVICE_SINGLETHREADED|D3D11_CREATE_DEVICE_BGRA_SUPPORT), featureLevels, SDL_arraysize(featureLevels), D3D11_SDK_VERSION, SDL_reinterpret_cast(ID3D11Device**, &m_device), &featureLevel, SDL_reinterpret_cast(ID3D11DeviceContext**, &m_context));
		if(FAILED(result))
		{
			UTIL_MessageBox(false, "Direct3D11: Failed to create device.");
			return false;
		}
	}

	void* d3device1;
	result = ID3D11Device_QueryInterface(SDL_reinterpret_cast(ID3D11Device*, m_device), D3D11_IID_ID3D11Device1, &d3device1);
	if(SUCCEEDED(result))
	{
		ID3D11Device* device = SDL_reinterpret_cast(ID3D11Device*, m_device);
		SAFE_RELEASE(device);
		m_device = d3device1;
	}

	void* d3dContext1;
	result = ID3D11DeviceContext_QueryInterface(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), D3D11_IID_ID3D11DeviceContext1, &d3dContext1);
	if(SUCCEEDED(result))
	{
		ID3D11DeviceContext* context = SDL_reinterpret_cast(ID3D11DeviceContext*, m_context);
		SAFE_RELEASE(context);
		m_context = d3dContext1;
	}
	
	IDXGIDevice1* dxgiDevice;
	result = ID3D11Device_QueryInterface(SDL_reinterpret_cast(ID3D11Device*, m_device), D3D11_IID_IDXGIDevice1, SDL_reinterpret_cast(void**, &dxgiDevice));
	if(SUCCEEDED(result))
	{
		IDXGIDevice1_SetMaximumFrameLatency(dxgiDevice, 1);
		SAFE_RELEASE(dxgiDevice);
	}

	switch(featureLevel)
	{
		case D3D_FEATURE_LEVEL_11_1:
		{
			m_software = "Direct3D 11.1";
			m_maxTextureSize = 16384;
		}
		break;
		case D3D_FEATURE_LEVEL_11_0:
		{
			m_software = "Direct3D 11.0";
			m_maxTextureSize = 16384;
		}
		break;
		case D3D_FEATURE_LEVEL_10_1:
		{
			m_software = "Direct3D 10.1";
			m_maxTextureSize = 8192;
		}
		break;
		case D3D_FEATURE_LEVEL_10_0:
		{
			m_software = "Direct3D 10.0";
			m_maxTextureSize = 8192;
		}
		break;
		case D3D_FEATURE_LEVEL_9_3:
		{
			m_software = "Direct3D 9.0c";
			m_maxTextureSize = 4096;
		}
		break;
		case D3D_FEATURE_LEVEL_9_2:
		{
			m_software = "Direct3D 9.0b";
			m_maxTextureSize = 2048;
		}
		break;
		case D3D_FEATURE_LEVEL_9_1:
		{
			m_software = "Direct3D 9.0";
			m_maxTextureSize = 2048;
		}
		break;
		default:
		{
			SDL_snprintf(g_buffer, sizeof(g_buffer), "Direct3D11: Unexpected feature level: %d", featureLevel);
			UTIL_MessageBox(false, g_buffer);
			return false;
		}
	}

	if(!createVertexShader(m_device, &m_vertexShader, &m_inputLayout))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create vertex shader.");
		return false;
	}

	if(!createPixelShader(m_device, D3D11_PixelShader_Solid, sizeof(D3D11_PixelShader_Solid), &m_pixelShaderSolid))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create pixel shader(SOLID COLORS).");
		return false;
	}

	if(!createPixelShader(m_device, D3D11_PixelShader_Texture, sizeof(D3D11_PixelShader_Texture), &m_pixelShaderTexture))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create pixel shader(RGBA TEXTURES).");
		return false;
	}
	
	if(createPixelShader(m_device, D3D11_sharpen, sizeof(D3D11_sharpen), &m_pixelShaderSharpen))
		m_haveSharpening = true;
	
	D3D11_BUFFER_DESC constantBufferDesc;
	SDL_zero(constantBufferDesc);
	constantBufferDesc.ByteWidth = sizeof(VertexShaderConstants);
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	result = ID3D11Device_CreateBuffer(SDL_reinterpret_cast(ID3D11Device*, m_device), &constantBufferDesc, NULL, SDL_reinterpret_cast(ID3D11Buffer**, &m_pixelShaderConstants));
	if(FAILED(result))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create pixel shader constants.");
		return false;
	}
	result = ID3D11Device_CreateBuffer(SDL_reinterpret_cast(ID3D11Device*, m_device), &constantBufferDesc, NULL, SDL_reinterpret_cast(ID3D11Buffer**, &m_vertexShaderConstants));
	if(FAILED(result))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create vertex shader constants.");
		return false;
	}
	
	D3D11_SAMPLER_DESC samplerDesc;
	SDL_zero(samplerDesc);
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	result = ID3D11Device_CreateSamplerState(SDL_reinterpret_cast(ID3D11Device*, m_device), &samplerDesc, SDL_reinterpret_cast(ID3D11SamplerState**, &m_samplers[0]));
	if(FAILED(result))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create texture sampler(NEAREST).");
		return false;
	}
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	result = ID3D11Device_CreateSamplerState(SDL_reinterpret_cast(ID3D11Device*, m_device), &samplerDesc, SDL_reinterpret_cast(ID3D11SamplerState**, &m_samplers[1]));
	if(FAILED(result))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create texture sampler(LINEAR).");
		return false;
	}

	D3D11_RASTERIZER_DESC rasterDesc;
	SDL_zero(rasterDesc);
	rasterDesc.AntialiasedLineEnable = FALSE;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = TRUE;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = FALSE;
	rasterDesc.MultisampleEnable = FALSE;
	rasterDesc.ScissorEnable = FALSE;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	result = ID3D11Device_CreateRasterizerState(SDL_reinterpret_cast(ID3D11Device*, m_device), &rasterDesc, SDL_reinterpret_cast(ID3D11RasterizerState**, &m_mainRasterizer));
	if(FAILED(result))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create rasterizer(MAIN).");
		return false;
	}
	rasterDesc.ScissorEnable = TRUE;
	result = ID3D11Device_CreateRasterizerState(SDL_reinterpret_cast(ID3D11Device*, m_device), &rasterDesc, SDL_reinterpret_cast(ID3D11RasterizerState**, &m_clippedRasterizer));
	if(FAILED(result))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create rasterizer(CLIPPED).");
		return false;
	}

	D3D11_BLEND_DESC blendDesc;
	SDL_zero(blendDesc);
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	result = ID3D11Device_CreateBlendState(SDL_reinterpret_cast(ID3D11Device*, m_device), &blendDesc, SDL_reinterpret_cast(ID3D11BlendState**, &m_blendBlend));
	if(FAILED(result))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create graphic pipeline.");
		return false;
	}
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	result = ID3D11Device_CreateBlendState(SDL_reinterpret_cast(ID3D11Device*, m_device), &blendDesc, SDL_reinterpret_cast(ID3D11BlendState**, &m_blendAdd));
	if(FAILED(result))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create graphic pipeline.");
		return false;
	}
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_COLOR;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	result = ID3D11Device_CreateBlendState(SDL_reinterpret_cast(ID3D11Device*, m_device), &blendDesc, SDL_reinterpret_cast(ID3D11BlendState**, &m_blendMod));
	if(FAILED(result))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create graphic pipeline.");
		return false;
	}

	ID3D11DeviceContext_IASetInputLayout(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11InputLayout*, m_inputLayout));
	ID3D11DeviceContext_VSSetShader(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11VertexShader*, m_vertexShader), NULL, 0);
	ID3D11DeviceContext_VSSetConstantBuffers(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, SDL_reinterpret_cast(ID3D11Buffer**, &m_vertexShaderConstants));
	
	#ifdef _USE_D3D_SHADER_COMPILER_
	ID3DBlob *vsBlob = nullptr;
	HRESULT hr = CompileShader(L"DX11_vertex_shader_vs.hlsl", "main", "vs_4_0_level_9_3", &vsBlob);
	if(SUCCEEDED(hr))
	{
		SDL_RWops* src = SDL_RWFromFile("test.ini", "wb");
		if(src)
		{
			Uint32* data = SDL_reinterpret_cast(Uint32*, vsBlob->GetBufferPointer());
			size_t len = vsBlob->GetBufferSize();
			bool started = false;
			size_t counter = 0;
			for(size_t i = 0; i < len; i += 4)
			{
				if(++counter == 6)
				{
					if(started)
					{
						started = false;
						int texSize = SDL_snprintf(g_buffer, sizeof(g_buffer), ", 0x%08x,\n", *data);
						SDL_RWwrite(src, g_buffer, 1, texSize);
					}
					else
					{
						started = true;
						int texSize = SDL_snprintf(g_buffer, sizeof(g_buffer), "0x%08x", *data);
						SDL_RWwrite(src, g_buffer, 1, texSize);
					}
					counter = 0;
				}
				else
				{
					if(started)
					{
						int texSize = SDL_snprintf(g_buffer, sizeof(g_buffer), ", 0x%08x", *data);
						SDL_RWwrite(src, g_buffer, 1, texSize);
					}
					else
					{
						started = true;
						int texSize = SDL_snprintf(g_buffer, sizeof(g_buffer), "0x%08x", *data);
						SDL_RWwrite(src, g_buffer, 1, texSize);
					}
				}
				++data;
			}
			SDL_RWclose(src);
		}
		SAFE_RELEASE(vsBlob);
	}
	#endif
	DXGI_ADAPTER_DESC adapterDesc;
	IDXGIAdapter_GetDesc(SDL_reinterpret_cast(IDXGIAdapter*, m_dxgiAdapter), &adapterDesc);
	sprintf_s(g_buffer, "%ws", adapterDesc.Description);
	m_hardware = SDL_strdup(g_buffer);
	m_totalVRAM = UTIL_power_of_2(SDL_static_cast(Uint32, adapterDesc.DedicatedVideoMemory/1048576));
	return true;
}

void SurfaceDirect3D11::init()
{
	m_pictures = SDL_reinterpret_cast(Direct3D11Texture**, SDL_calloc(g_pictureCounts, sizeof(Direct3D11Texture*)));
	if(!m_pictures)
	{
		SDL_OutOfMemory();
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Surface::Init() Failed: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
	}

	m_gameWindow = createDirect3DTexture(RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT, (g_engine.getAntialiasing() == CLIENT_ANTIALIASING_NORMAL), true);
	if(!m_gameWindow)
	{
		UTIL_MessageBox(false, "Direct3D11: Out of video memory.");
		exit(-1);
		return;
	}
}

void SurfaceDirect3D11::doResize(Sint32, Sint32)
{
	m_needReset = true;
}

void SurfaceDirect3D11::spriteManagerReset()
{
	for(U32BD3D11Textures::iterator it = m_automapTiles.begin(), end = m_automapTiles.end(); it != end; ++it)
		releaseDirect3DTexture(it->second);

	for(U64BD3D11Textures::iterator it = m_spriteMasks.begin(), end = m_spriteMasks.end(); it != end; ++it)
		releaseDirect3DTexture(it->second);

	m_spriteMasks.clear();
	m_spritesMaskIds.clear();
	m_automapTilesBuff.clear();
	m_automapTiles.clear();
}

unsigned char* SurfaceDirect3D11::getScreenPixels(Sint32& width, Sint32& height, bool& bgra)
{
	if(m_needReset)
		return NULL;

	ID3D11Texture2D* backBuffer;
	HRESULT result = IDXGISwapChain_GetBuffer(SDL_reinterpret_cast(IDXGISwapChain*, m_swapChain), 0, IID_ID3D11Texture2D, SDL_reinterpret_cast(void**, &backBuffer));
	if(FAILED(result))
		return NULL;
	ID3D11Texture2D* stagingTexture;
	D3D11_TEXTURE2D_DESC stagingTextureDesc;
	ID3D11Texture2D_GetDesc(backBuffer, &stagingTextureDesc);
	width = SDL_static_cast(Sint32, stagingTextureDesc.Width);
	height = SDL_static_cast(Sint32, stagingTextureDesc.Height);
	stagingTextureDesc.BindFlags = 0;
	stagingTextureDesc.MiscFlags = 0;
	stagingTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	stagingTextureDesc.Usage = D3D11_USAGE_STAGING;
	unsigned char* pixels = SDL_reinterpret_cast(unsigned char*, SDL_malloc(width*height*4));
	if(!pixels)
	{
		SAFE_RELEASE(backBuffer);
		return NULL;
	}
	result = ID3D11Device_CreateTexture2D(SDL_reinterpret_cast(ID3D11Device*, m_device), &stagingTextureDesc, NULL, SDL_reinterpret_cast(ID3D11Texture2D**, &stagingTexture));
	if(FAILED(result))
	{
		SAFE_RELEASE(backBuffer);
		SDL_free(pixels);
		return NULL;
	}
	DXGI_MODE_ROTATION rotation = D3D11_GetCurrentRotation();
	D3D11_BOX srcBox;
	switch(rotation)
	{
		case DXGI_MODE_ROTATION_ROTATE90:
		{
			srcBox.left = m_viewPortH-height;
			srcBox.right = m_viewPortH;
			srcBox.top = 0;
			srcBox.bottom = height;
		}
		break;
		case DXGI_MODE_ROTATION_ROTATE180:
		{
			srcBox.left = m_viewPortW-width;
			srcBox.right = m_viewPortW;
			srcBox.top = m_viewPortH-height;
			srcBox.bottom = m_viewPortH;
		}
		break;
		case DXGI_MODE_ROTATION_ROTATE270:
		{
			srcBox.left = 0;
			srcBox.right = height;
			srcBox.top = m_viewPortW-width;
			srcBox.bottom = m_viewPortW;
		}
		break;
		default:
		{
			srcBox.left = m_viewPortX;
			srcBox.right = m_viewPortX+width;
			srcBox.top = m_viewPortY;
			srcBox.bottom = m_viewPortY+height;
		}
		break;
	}
	srcBox.front = 0;
	srcBox.back = 1;
	ID3D11DeviceContext_CopySubresourceRegion(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, stagingTexture), 0, 0, 0, 0, SDL_reinterpret_cast(ID3D11Resource*, backBuffer), 0, &srcBox);
	SAFE_RELEASE(backBuffer);

	D3D11_MAPPED_SUBRESOURCE textureMemory;
	result = ID3D11DeviceContext_Map(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, stagingTexture), 0, D3D11_MAP_READ, 0, &textureMemory);
	if(FAILED(result))
	{
		SAFE_RELEASE(stagingTexture);
		SDL_free(pixels);
		return false;
	}
	if(stagingTextureDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM || stagingTextureDesc.Format == DXGI_FORMAT_B8G8R8X8_UNORM)
	{
		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, pixels), SDL_reinterpret_cast(const Uint8*, textureMemory.pData), SDL_static_cast(size_t, width*height*4));
		bgra = true;
	}
	else
	{
		ID3D11DeviceContext_Unmap(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, stagingTexture), 0);
		SAFE_RELEASE(stagingTexture);
		SDL_free(pixels);
		return NULL;
	}
	ID3D11DeviceContext_Unmap(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, stagingTexture), 0);
	SAFE_RELEASE(stagingTexture);
	return pixels;
}

void SurfaceDirect3D11::beginScene()
{
	if(m_needReset)
	{
		m_currentRenderTargetView = NULL;
		createWindowSizeDependentResources();
		m_currentRenderTargetView = m_mainRenderTargetView;
	}

	ID3D11DeviceContext_OMSetRenderTargets(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 1, SDL_reinterpret_cast(ID3D11RenderTargetView**, &m_mainRenderTargetView), NULL);
	if(m_currentRenderTargetView != m_mainRenderTargetView)
	{
		m_viewPortX = 0;
		m_viewPortY = 0;
		m_viewPortW = g_engine.getWindowWidth();
		m_viewPortH = g_engine.getWindowHeight();
		m_currentRenderTargetView = m_mainRenderTargetView;
		updateViewport();
	}

	ID3D11DeviceContext_RSSetState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11RasterizerState*, m_mainRasterizer));
	ID3D11DeviceContext_OMSetBlendState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11BlendState*, m_blendBlend), 0, 0xFFFFFFFF);

	ID3D11SamplerState* sampler = SDL_reinterpret_cast(ID3D11SamplerState*, m_samplers[0]);
	ID3D11DeviceContext_PSSetShader(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderTexture), NULL, 0);
	ID3D11DeviceContext_PSSetSamplers(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, &sampler);
	m_usingLinearSample = false;
}

void SurfaceDirect3D11::endScene()
{
	UINT syncInterval;
	UINT presentFlags;
	#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
	syncInterval = 1;
	presentFlags = 0;
	#else
	if(g_engine.isVsync())
	{
		syncInterval = 1;
		presentFlags = 0;
	}
	else
	{
		syncInterval = 0;
		presentFlags = 0;
	}
	#endif

	HRESULT result = IDXGISwapChain_Present(SDL_reinterpret_cast(IDXGISwapChain*, m_swapChain), syncInterval, presentFlags);
	if(FAILED(result))
	{
		if(result == DXGI_ERROR_DEVICE_RESET || result == DXGI_ERROR_DEVICE_REMOVED)
		{
			//g_inited = false should recreate the whole d3d11 interface in next frame
			g_inited = false;
		}
		else if(result == DXGI_ERROR_INVALID_CALL)
		{
			//Schedule swapchain reset
			m_needReset = true;
		}
		else
		{
			UTIL_MessageBox(true, "Direct3D11: Failed to present swapchain.");
			exit(-1);
		}
	}
}

bool SurfaceDirect3D11::integer_scaling(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
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

	ID3D11DeviceContext_RSSetState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11RasterizerState*, m_mainRasterizer));
	ID3D11DeviceContext_OMSetRenderTargets(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 1, SDL_reinterpret_cast(ID3D11RenderTargetView**, &m_scaled_gameWindow->m_frameBuffer), NULL);
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = m_scaled_gameWindow->m_width;
	m_viewPortH = m_scaled_gameWindow->m_height;
	m_currentRenderTargetView = m_scaled_gameWindow->m_frameBuffer;
	updateViewport();

	float minx = 0.0f;
	float maxx = SDL_static_cast(float, width);
	float miny = 0.0f;
	float maxy = SDL_static_cast(float, height);

	float minu = sx*m_gameWindow->m_scaleW;
	float maxu = (sx+sw)*m_gameWindow->m_scaleW;
	float minv = sy*m_gameWindow->m_scaleH;
	float maxv = (sy+sh)*m_gameWindow->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexD3D11 vertices[4];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].u = minu;
	vertices[0].v = minv;
	vertices[0].color = texColor;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].u = minu;
	vertices[1].v = maxv;
	vertices[1].color = texColor;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].u = maxu;
	vertices[2].v = minv;
	vertices[2].color = texColor;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].u = maxu;
	vertices[3].v = maxv;
	vertices[3].color = texColor;
	if(!updateVertexBuffer(vertices, sizeof(vertices)))
		return false;

	updateTextureScaling(m_gameWindow);
	ID3D11DeviceContext_OMSetBlendState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), NULL, 0, 0xFFFFFFFF);
	ID3D11DeviceContext_PSSetShaderResources(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, SDL_reinterpret_cast(ID3D11ShaderResourceView**, &m_gameWindow->m_resource));
	ID3D11DeviceContext_IASetPrimitiveTopology(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	ID3D11DeviceContext_Draw(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 4, 0);
	ID3D11DeviceContext_OMSetRenderTargets(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 1, SDL_reinterpret_cast(ID3D11RenderTargetView**, &m_mainRenderTargetView), NULL);
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = g_engine.getWindowWidth();
	m_viewPortH = g_engine.getWindowHeight();
	m_currentRenderTargetView = m_mainRenderTargetView;
	updateViewport();

	minx = SDL_static_cast(float, x);
	maxx = SDL_static_cast(float, x+w);
	miny = SDL_static_cast(float, y);
	maxy = SDL_static_cast(float, y+h);

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
	if(!updateVertexBuffer(vertices, sizeof(vertices)))
		return false;

	updateTextureScaling(m_scaled_gameWindow);
	ID3D11DeviceContext_RSSetState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11RasterizerState*, m_clippedRasterizer));
	if(g_engine.isSharpening() && m_haveSharpening)
	{
		float textureDimensions[2] = {1.0f / w, 1.0f / h};
		ID3D11DeviceContext_PSSetShader(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderSharpen), NULL, 0);
		ID3D11DeviceContext_PSSetConstantBuffers(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, SDL_reinterpret_cast(ID3D11Buffer**, &m_pixelShaderConstants));
		ID3D11DeviceContext_UpdateSubresource(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, m_pixelShaderConstants), 0, NULL, &textureDimensions, 0, 0);
	}
	ID3D11DeviceContext_PSSetShaderResources(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, SDL_reinterpret_cast(ID3D11ShaderResourceView**, &m_scaled_gameWindow->m_resource));
	ID3D11DeviceContext_IASetPrimitiveTopology(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	ID3D11DeviceContext_Draw(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 4, 0);
	if(g_engine.isSharpening() && m_haveSharpening)
	{
		ID3D11Buffer* resources = NULL;
		ID3D11DeviceContext_PSSetShader(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderTexture), NULL, 0);
		ID3D11DeviceContext_PSSetConstantBuffers(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 0, &resources);
	}
	ID3D11DeviceContext_OMSetBlendState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11BlendState*, m_blendBlend), 0, 0xFFFFFFFF);
	return true;
}

void SurfaceDirect3D11::drawGameScene(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
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
	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	float minu = sx*m_gameWindow->m_scaleW;
	float maxu = (sx+sw)*m_gameWindow->m_scaleW;
	float minv = sy*m_gameWindow->m_scaleH;
	float maxv = (sy+sh)*m_gameWindow->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexD3D11 vertices[4];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].u = minu;
	vertices[0].v = minv;
	vertices[0].color = texColor;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].u = minu;
	vertices[1].v = maxv;
	vertices[1].color = texColor;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].u = maxu;
	vertices[2].v = minv;
	vertices[2].color = texColor;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].u = maxu;
	vertices[3].v = maxv;
	vertices[3].color = texColor;
	if(!updateVertexBuffer(vertices, sizeof(vertices)))
		return;

	updateTextureScaling(m_gameWindow);
	ID3D11DeviceContext_OMSetBlendState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), NULL, 0, 0xFFFFFFFF);
	if(g_engine.isSharpening() && m_haveSharpening)
	{
		float textureDimensions[2] = {1.0f / w, 1.0f / h};
		ID3D11DeviceContext_PSSetShader(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderSharpen), NULL, 0);
		ID3D11DeviceContext_PSSetConstantBuffers(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, SDL_reinterpret_cast(ID3D11Buffer**, &m_pixelShaderConstants));
		ID3D11DeviceContext_UpdateSubresource(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, m_pixelShaderConstants), 0, NULL, &textureDimensions, 0, 0);
	}
	ID3D11DeviceContext_PSSetShaderResources(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, SDL_reinterpret_cast(ID3D11ShaderResourceView**, &m_gameWindow->m_resource));
	ID3D11DeviceContext_IASetPrimitiveTopology(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	ID3D11DeviceContext_Draw(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 4, 0);
	if(g_engine.isSharpening() && m_haveSharpening)
	{
		ID3D11Buffer* resources = NULL;
		ID3D11DeviceContext_PSSetShader(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderTexture), NULL, 0);
		ID3D11DeviceContext_PSSetConstantBuffers(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 0, &resources);
	}
	ID3D11DeviceContext_OMSetBlendState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11BlendState*, m_blendBlend), 0, 0xFFFFFFFF);
}

void SurfaceDirect3D11::beginGameScene()
{
	ID3D11DeviceContext_OMSetRenderTargets(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 1, SDL_reinterpret_cast(ID3D11RenderTargetView**, &m_gameWindow->m_frameBuffer), NULL);
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = m_gameWindow->m_width;
	m_viewPortH = m_gameWindow->m_height;
	m_currentRenderTargetView = m_gameWindow->m_frameBuffer;
	updateViewport();
	fillRectangle(0, 0, RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT, 0x00, 0x00, 0x00, 0xFF);
}

void SurfaceDirect3D11::endGameScene()
{
	ID3D11DeviceContext_OMSetRenderTargets(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 1, SDL_reinterpret_cast(ID3D11RenderTargetView**, &m_mainRenderTargetView), NULL);
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = g_engine.getWindowWidth();
	m_viewPortH = g_engine.getWindowHeight();
	m_currentRenderTargetView = m_mainRenderTargetView;
	updateViewport();
}

void SurfaceDirect3D11::drawLightMap(LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height)
{
	VertexD3D11* vertices = SDL_reinterpret_cast(VertexD3D11*, SDL_malloc(sizeof(VertexD3D11)*(width+1)*2));
	ID3D11DeviceContext_OMSetBlendState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11BlendState*, m_blendMod), 0, 0xFFFFFFFF);

	ID3D11ShaderResourceView* resources = NULL;
	ID3D11DeviceContext_PSSetShader(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderSolid), NULL, 0);
	ID3D11DeviceContext_PSSetShaderResources(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 0, &resources);
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
			vertices[verticeCount].x = SDL_static_cast(float, drawX);
			vertices[verticeCount].y = SDL_static_cast(float, drawY+scale);
			vertices[verticeCount].u = 0.0f;
			vertices[verticeCount].v = 0.0f;
			vertices[verticeCount].color = MAKE_RGBA_COLOR(lightmap[offset1+offset].r, lightmap[offset1+offset].g, lightmap[offset1+offset].b, 255);
			++verticeCount;

			vertices[verticeCount].x = SDL_static_cast(float, drawX);
			vertices[verticeCount].y = SDL_static_cast(float, drawY);
			vertices[verticeCount].u = 0.0f;
			vertices[verticeCount].v = 0.0f;
			vertices[verticeCount].color = MAKE_RGBA_COLOR(lightmap[offset2+offset].r, lightmap[offset2+offset].g, lightmap[offset2+offset].b, 255);
			++verticeCount;
			drawX += scale;
		}

		drawY += scale;
		if(!updateVertexBuffer(vertices, sizeof(VertexD3D11)*verticeCount))
			continue;

		ID3D11DeviceContext_IASetPrimitiveTopology(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		ID3D11DeviceContext_Draw(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), verticeCount, 0);
	}

	ID3D11DeviceContext_OMSetBlendState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11BlendState*, m_blendBlend), 0, 0xFFFFFFFF);
	ID3D11DeviceContext_PSSetShader(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderTexture), NULL, 0);
	SDL_free(vertices);
}

void SurfaceDirect3D11::setClipRect(Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_IDENTITY;
	if(!m_currentRenderTargetView || m_currentRenderTargetView == m_mainRenderTargetView)
		rotation = D3D11_GetCurrentRotation();
	D3D11_RECT scissorRect;
	switch(rotation)
	{
		case DXGI_MODE_ROTATION_ROTATE90:
		{
			scissorRect.left = m_viewPortH-y-h;
			scissorRect.right = m_viewPortH-y;
			scissorRect.top = x;
			scissorRect.bottom = x+h;
		}
		break;
		case DXGI_MODE_ROTATION_ROTATE180:
		{
			scissorRect.left = m_viewPortW-x-w;
			scissorRect.right = m_viewPortW-x;
			scissorRect.top = m_viewPortH-y-h;
			scissorRect.bottom = m_viewPortH-y;
		}
		break;
		case DXGI_MODE_ROTATION_ROTATE270:
		{
			scissorRect.left = y;
			scissorRect.right = y+h;
			scissorRect.top = m_viewPortW-x-w;
			scissorRect.bottom = m_viewPortW-x;
		}
		break;
		default:
		{
			scissorRect.left = m_viewPortX+x;
			scissorRect.right = m_viewPortX+x+w;
			scissorRect.top = m_viewPortY+y;
			scissorRect.bottom = m_viewPortY+y+h;
		}
		break;
	}
	ID3D11DeviceContext_RSSetScissorRects(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 1, &scissorRect);
	ID3D11DeviceContext_RSSetState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11RasterizerState*, m_clippedRasterizer));
}

void SurfaceDirect3D11::disableClipRect()
{
	ID3D11DeviceContext_RSSetScissorRects(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, NULL);
	ID3D11DeviceContext_RSSetState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11RasterizerState*, m_mainRasterizer));
}

void SurfaceDirect3D11::drawRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	float minx = SDL_static_cast(float, x)+0.5f;
	float maxx = SDL_static_cast(float, x+w)-0.5f;
	float miny = SDL_static_cast(float, y)+0.5f;
	float maxy = SDL_static_cast(float, y+h)-0.5f;

	DWORD texColor = MAKE_RGBA_COLOR(r, g, b, a);

	VertexD3D11 vertices[5];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].u = 0.0f;
	vertices[0].v = 0.0f;
	vertices[0].color = texColor;

	vertices[1].x = maxx;
	vertices[1].y = miny;
	vertices[1].u = 0.0f;
	vertices[1].v = 0.0f;
	vertices[1].color = texColor;

	vertices[2].x = maxx;
	vertices[2].y = maxy;
	vertices[2].u = 0.0f;
	vertices[2].v = 0.0f;
	vertices[2].color = texColor;

	vertices[3].x = minx;
	vertices[3].y = maxy;
	vertices[3].u = 0.0f;
	vertices[3].v = 0.0f;
	vertices[3].color = texColor;

	vertices[4].x = minx;
	vertices[4].y = miny;
	vertices[4].u = 0.0f;
	vertices[4].v = 0.0f;
	vertices[4].color = texColor;
	if(!updateVertexBuffer(vertices, sizeof(vertices)))
		return;

	ID3D11ShaderResourceView* resources = NULL;
	ID3D11DeviceContext_PSSetShader(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderSolid), NULL, 0);
	ID3D11DeviceContext_PSSetShaderResources(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 0, &resources);

	ID3D11DeviceContext_IASetPrimitiveTopology(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	ID3D11DeviceContext_Draw(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 5, 0);

	ID3D11DeviceContext_PSSetShader(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderTexture), NULL, 0);
}

void SurfaceDirect3D11::fillRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	DWORD texColor = MAKE_RGBA_COLOR(r, g, b, a);

	VertexD3D11 vertices[4];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].u = 0.0f;
	vertices[0].v = 0.0f;
	vertices[0].color = texColor;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].u = 0.0f;
	vertices[1].v = 0.0f;
	vertices[1].color = texColor;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].u = 0.0f;
	vertices[2].v = 0.0f;
	vertices[2].color = texColor;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].u = 0.0f;
	vertices[3].v = 0.0f;
	vertices[3].color = texColor;
	if(!updateVertexBuffer(vertices, sizeof(vertices)))
		return;

	ID3D11ShaderResourceView* resources = NULL;
	ID3D11DeviceContext_PSSetShader(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderSolid), NULL, 0);
	ID3D11DeviceContext_PSSetShaderResources(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 0, &resources);

	ID3D11DeviceContext_IASetPrimitiveTopology(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	ID3D11DeviceContext_Draw(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 4, 0);

	ID3D11DeviceContext_PSSetShader(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderTexture), NULL, 0);
}

Direct3D11Texture* SurfaceDirect3D11::loadPicture(Uint16 pictureId, bool linear)
{
	Sint32 width, height;
	unsigned char* pixels = g_engine.LoadPicture(pictureId, true, width, height);
	if(!pixels)
		return NULL;

	Direct3D11Texture* s = createDirect3DTexture(width, height, linear);
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

void SurfaceDirect3D11::drawFont(Uint16 pictureId, Sint32 x, Sint32 y, const std::string& text, size_t pos, size_t len, Uint8 r, Uint8 g, Uint8 b, Sint32 cX[256], Sint32 cY[256], Sint32 cW[256], Sint32 cH[256])
{
	Direct3D11Texture* tex = m_pictures[pictureId];
	if(!tex)
	{
		tex = loadPicture(pictureId, false);
		if(!tex)
			return;//load failed
	}

	std::vector<VertexD3D11> vertices;
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
				float minx = SDL_static_cast(float, rx);
				float maxx = SDL_static_cast(float, rx+cW[character]);
				float miny = SDL_static_cast(float, ry);
				float maxy = SDL_static_cast(float, ry+cH[character]);

				float minu = cX[character]*tex->m_scaleW;
				float maxu = (cX[character]+cW[character])*tex->m_scaleW;
				float minv = cY[character]*tex->m_scaleH;
				float maxv = (cY[character]+cH[character])*tex->m_scaleH;

				VertexD3D11 vertice[4];
				vertice[0].x = minx;
				vertice[0].y = miny;
				vertice[0].u = minu;
				vertice[0].v = minv;
				vertice[0].color = texColor;

				vertice[1].x = minx;
				vertice[1].y = maxy;
				vertice[1].u = minu;
				vertice[1].v = maxv;
				vertice[1].color = texColor;

				vertice[2].x = maxx;
				vertice[2].y = miny;
				vertice[2].u = maxu;
				vertice[2].v = minv;
				vertice[2].color = texColor;

				vertice[3].x = maxx;
				vertice[3].y = maxy;
				vertice[3].u = maxu;
				vertice[3].v = maxv;
				vertice[3].color = texColor;

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
	ID3D11DeviceContext_PSSetShaderResources(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, SDL_reinterpret_cast(ID3D11ShaderResourceView**, &tex->m_resource));
	drawTriangles(vertices);
}

void SurfaceDirect3D11::drawBackground(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	Direct3D11Texture* tex = m_pictures[pictureId];
	if(!tex)
	{
		tex = loadPicture(pictureId, g_engine.hasAntialiasing());
		if(!tex)
			return;//load failed
	}

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	float minu = sx*tex->m_scaleW;
	float maxu = (sx+sw)*tex->m_scaleW;
	float minv = sy*tex->m_scaleH;
	float maxv = (sy+sh)*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexD3D11 vertices[4];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].u = minu;
	vertices[0].v = minv;
	vertices[0].color = texColor;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].u = minu;
	vertices[1].v = maxv;
	vertices[1].color = texColor;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].u = maxu;
	vertices[2].v = minv;
	vertices[2].color = texColor;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].u = maxu;
	vertices[3].v = maxv;
	vertices[3].color = texColor;
	if(!updateVertexBuffer(vertices, sizeof(vertices)))
		return;

	updateTextureScaling(tex);
	ID3D11DeviceContext_OMSetBlendState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), NULL, 0, 0xFFFFFFFF);
	ID3D11DeviceContext_PSSetShaderResources(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, SDL_reinterpret_cast(ID3D11ShaderResourceView**, &tex->m_resource));
	ID3D11DeviceContext_IASetPrimitiveTopology(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	ID3D11DeviceContext_Draw(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 4, 0);
	ID3D11DeviceContext_OMSetBlendState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11BlendState*, m_blendBlend), 0, 0xFFFFFFFF);
}

void SurfaceDirect3D11::drawPictureRepeat(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	Direct3D11Texture* tex = m_pictures[pictureId];
	if(!tex)
	{
		tex = loadPicture(pictureId, false);
		if(!tex)
			return;//load failed
	}

	std::vector<VertexD3D11> vertices;
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
			float minx = SDL_static_cast(float, cx);
			float maxx = SDL_static_cast(float, cx+curW);
			float miny = SDL_static_cast(float, y);
			float maxy = SDL_static_cast(float, y+curH);

			float minu = sx*tex->m_scaleW;
			float maxu = (sx+curW)*tex->m_scaleW;
			float minv = sy*tex->m_scaleH;
			float maxv = (sy+curH)*tex->m_scaleH;

			VertexD3D11 vertice[4];
			vertice[0].x = minx;
			vertice[0].y = miny;
			vertice[0].u = minu;
			vertice[0].v = minv;
			vertice[0].color = texColor;

			vertice[1].x = minx;
			vertice[1].y = maxy;
			vertice[1].u = minu;
			vertice[1].v = maxv;
			vertice[1].color = texColor;

			vertice[2].x = maxx;
			vertice[2].y = miny;
			vertice[2].u = maxu;
			vertice[2].v = minv;
			vertice[2].color = texColor;

			vertice[3].x = maxx;
			vertice[3].y = maxy;
			vertice[3].u = maxu;
			vertice[3].v = maxv;
			vertice[3].color = texColor;

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
	ID3D11DeviceContext_PSSetShaderResources(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, SDL_reinterpret_cast(ID3D11ShaderResourceView**, &tex->m_resource));
	drawTriangles(vertices);
}

void SurfaceDirect3D11::drawPicture(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	Direct3D11Texture* tex = m_pictures[pictureId];
	if(!tex)
	{
		tex = loadPicture(pictureId, false);
		if(!tex)
			return;//load failed
	}

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	float minu = sx*tex->m_scaleW;
	float maxu = (sx+w)*tex->m_scaleW;
	float minv = sy*tex->m_scaleH;
	float maxv = (sy+h)*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexD3D11 vertices[4];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].u = minu;
	vertices[0].v = minv;
	vertices[0].color = texColor;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].u = minu;
	vertices[1].v = maxv;
	vertices[1].color = texColor;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].u = maxu;
	vertices[2].v = minv;
	vertices[2].color = texColor;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].u = maxu;
	vertices[3].v = maxv;
	vertices[3].color = texColor;
	if(!updateVertexBuffer(vertices, sizeof(vertices)))
		return;

	updateTextureScaling(tex);
	ID3D11DeviceContext_PSSetShaderResources(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, SDL_reinterpret_cast(ID3D11ShaderResourceView**, &tex->m_resource));
	ID3D11DeviceContext_IASetPrimitiveTopology(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	ID3D11DeviceContext_Draw(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 4, 0);
}

Direct3D11Texture* SurfaceDirect3D11::loadSpriteMask(Uint64 tempPos, Uint32 spriteId, Uint32 maskSpriteId, Uint32 outfitColor)
{
	unsigned char* pixels = g_engine.LoadSpriteMask(spriteId, maskSpriteId, outfitColor, true);
	if(!pixels)
		return NULL;

	if(m_spriteMasks.size() >= HARDWARE_MAX_SPRITEMASKS)
	{
		U64BD3D11Textures::iterator it = m_spriteMasks.find(m_spritesMaskIds.front());
		if(it == m_spriteMasks.end())//Something weird happen - let's erase the first one entry
			it = m_spriteMasks.begin();

		releaseDirect3DTexture(it->second);
		m_spriteMasks.erase(it);
		m_spritesMaskIds.pop_front();
	}
	Direct3D11Texture* s = createDirect3DTexture(32, 32, false);
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

void SurfaceDirect3D11::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor)
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
	Direct3D11Texture* tex;
	U64BD3D11Textures::iterator it = m_spriteMasks.find(tempPos);
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

	float minx = SDL_static_cast(float, x);
	float maxx = minx+SDL_static_cast(float, tex->m_width);
	float miny = SDL_static_cast(float, y);
	float maxy = miny+SDL_static_cast(float, tex->m_height);

	float minu = 0.0f;
	float maxu = tex->m_width*tex->m_scaleW;
	float minv = 0.0f;
	float maxv = tex->m_height*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexD3D11 vertices[4];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].u = minu;
	vertices[0].v = minv;
	vertices[0].color = texColor;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].u = minu;
	vertices[1].v = maxv;
	vertices[1].color = texColor;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].u = maxu;
	vertices[2].v = minv;
	vertices[2].color = texColor;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].u = maxu;
	vertices[3].v = maxv;
	vertices[3].color = texColor;
	if(!updateVertexBuffer(vertices, sizeof(vertices)))
		return;

	updateTextureScaling(tex);
	ID3D11DeviceContext_PSSetShaderResources(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, SDL_reinterpret_cast(ID3D11ShaderResourceView**, &tex->m_resource));
	ID3D11DeviceContext_IASetPrimitiveTopology(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	ID3D11DeviceContext_Draw(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 4, 0);
}

void SurfaceDirect3D11::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Uint32 outfitColor)
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
	Direct3D11Texture* tex;
	U64BD3D11Textures::iterator it = m_spriteMasks.find(tempPos);
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

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	float minu = sx*tex->m_scaleW;
	float maxu = (sx+sw)*tex->m_scaleW;
	float minv = sy*tex->m_scaleH;
	float maxv = (sy+sh)*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexD3D11 vertices[4];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].u = minu;
	vertices[0].v = minv;
	vertices[0].color = texColor;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].u = minu;
	vertices[1].v = maxv;
	vertices[1].color = texColor;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].u = maxu;
	vertices[2].v = minv;
	vertices[2].color = texColor;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].u = maxu;
	vertices[3].v = maxv;
	vertices[3].color = texColor;
	if(!updateVertexBuffer(vertices, sizeof(vertices)))
		return;

	updateTextureScaling(tex);
	ID3D11DeviceContext_PSSetShaderResources(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, SDL_reinterpret_cast(ID3D11ShaderResourceView**, &tex->m_resource));
	ID3D11DeviceContext_IASetPrimitiveTopology(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	ID3D11DeviceContext_Draw(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 4, 0);
}

Direct3D11Texture* SurfaceDirect3D11::createAutomapTile(Uint32 currentArea)
{
	if(m_automapTiles.size() >= HARDWARE_MAX_AUTOMAPTILES)
	{
		U32BD3D11Textures::iterator it = m_automapTiles.find(m_automapTilesBuff.front());
		if(it == m_automapTiles.end())//Something weird happen - let's erase the first one entry
			it = m_automapTiles.begin();

		releaseDirect3DTexture(it->second);
		m_automapTiles.erase(it);
		m_automapTilesBuff.pop_front();
	}
	Direct3D11Texture* s = createDirect3DTexture(256, 256, false);
	if(!s)
		return NULL;
	
	m_automapTiles[currentArea] = s;
	m_automapTilesBuff.push_back(currentArea);
	return s;
}

void SurfaceDirect3D11::uploadAutomapTile(Direct3D11Texture* texture, Uint8 color[256][256])
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

void SurfaceDirect3D11::drawAutomapTile(Uint32 currentArea, bool& recreate, Uint8 color[256][256], Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
{
	Direct3D11Texture* tex;
	U32BD3D11Textures::iterator it = m_automapTiles.find(currentArea);
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

	float minu = sx*tex->m_scaleW;
	float maxu = (sx+sw)*tex->m_scaleW;
	float minv = sy*tex->m_scaleH;
	float maxv = (sy+sh)*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexD3D11 vertices[4];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].u = minu;
	vertices[0].v = minv;
	vertices[0].color = texColor;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].u = minu;
	vertices[1].v = maxv;
	vertices[1].color = texColor;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].u = maxu;
	vertices[2].v = minv;
	vertices[2].color = texColor;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].u = maxu;
	vertices[3].v = maxv;
	vertices[3].color = texColor;
	if(!updateVertexBuffer(vertices, sizeof(vertices)))
		return;

	updateTextureScaling(tex);
	ID3D11DeviceContext_PSSetShaderResources(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, SDL_reinterpret_cast(ID3D11ShaderResourceView**, &tex->m_resource));
	ID3D11DeviceContext_IASetPrimitiveTopology(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	ID3D11DeviceContext_Draw(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 4, 0);
}

SurfaceDirect3D11Comp::SurfaceDirect3D11Comp()
{
	m_sprites = NULL;
}

SurfaceDirect3D11Comp::~SurfaceDirect3D11Comp()
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

void SurfaceDirect3D11Comp::init()
{
	SurfaceDirect3D11::init();
	m_sprites = SDL_reinterpret_cast(Direct3D11Texture**, SDL_calloc(g_spriteCounts+1, sizeof(Direct3D11Texture*)));
	if(!m_sprites)
	{
		SDL_OutOfMemory();
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Surface::Init() Failed: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
	}
	m_spritesCache = g_spriteCounts;
}

void SurfaceDirect3D11Comp::spriteManagerReset()
{
	SurfaceDirect3D11::spriteManagerReset();
	if(m_sprites)
	{
		for(Uint32 i = 1; i <= m_spritesCache; ++i)
		{
			if(m_sprites[i])
				releaseDirect3DTexture(m_sprites[i]);
		}

		SDL_free(m_sprites);
	}
	m_sprites = SDL_reinterpret_cast(Direct3D11Texture**, SDL_calloc(g_spriteCounts+1, sizeof(Direct3D11Texture*)));
	if(!m_sprites)
	{
		SDL_OutOfMemory();
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Surface::spriteManagerReset() Failed: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
	}
	m_spritesCache = g_spriteCounts;
}

Direct3D11Texture* SurfaceDirect3D11Comp::loadSprite(Uint32 spriteId)
{
	unsigned char* pixels = g_engine.LoadSprite(spriteId, true);
	if(!pixels)
		return NULL;

	Direct3D11Texture* s = createDirect3DTexture(32, 32, false);
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

void SurfaceDirect3D11Comp::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y)
{
	if(spriteId > g_spriteCounts)
		return;

	Direct3D11Texture* tex = m_sprites[spriteId];
	if(!tex)
	{
		tex = loadSprite(spriteId);
		if(!tex)
			return;//load failed
	}

	float minx = SDL_static_cast(float, x);
	float maxx = minx+SDL_static_cast(float, tex->m_width);
	float miny = SDL_static_cast(float, y);
	float maxy = miny+SDL_static_cast(float, tex->m_height);

	float minu = 0.0f;
	float maxu = tex->m_width*tex->m_scaleW;
	float minv = 0.0f;
	float maxv = tex->m_height*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexD3D11 vertices[4];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].u = minu;
	vertices[0].v = minv;
	vertices[0].color = texColor;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].u = minu;
	vertices[1].v = maxv;
	vertices[1].color = texColor;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].u = maxu;
	vertices[2].v = minv;
	vertices[2].color = texColor;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].u = maxu;
	vertices[3].v = maxv;
	vertices[3].color = texColor;
	if(!updateVertexBuffer(vertices, sizeof(vertices)))
		return;

	updateTextureScaling(tex);
	ID3D11DeviceContext_PSSetShaderResources(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, SDL_reinterpret_cast(ID3D11ShaderResourceView**, &tex->m_resource));
	ID3D11DeviceContext_IASetPrimitiveTopology(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	ID3D11DeviceContext_Draw(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 4, 0);
}

void SurfaceDirect3D11Comp::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
{
	if(spriteId > g_spriteCounts)
		return;

	Direct3D11Texture* tex = m_sprites[spriteId];
	if(!tex)
	{
		tex = loadSprite(spriteId);
		if(!tex)
			return;//load failed
	}

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	float minu = sx*tex->m_scaleW;
	float maxu = (sx+sw)*tex->m_scaleW;
	float minv = sy*tex->m_scaleH;
	float maxv = (sy+sh)*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);
	
	VertexD3D11 vertices[4];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].u = minu;
	vertices[0].v = minv;
	vertices[0].color = texColor;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].u = minu;
	vertices[1].v = maxv;
	vertices[1].color = texColor;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].u = maxu;
	vertices[2].v = minv;
	vertices[2].color = texColor;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].u = maxu;
	vertices[3].v = maxv;
	vertices[3].color = texColor;
	if(!updateVertexBuffer(vertices, sizeof(vertices)))
		return;

	updateTextureScaling(tex);
	ID3D11DeviceContext_PSSetShaderResources(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, SDL_reinterpret_cast(ID3D11ShaderResourceView**, &tex->m_resource));
	ID3D11DeviceContext_IASetPrimitiveTopology(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	ID3D11DeviceContext_Draw(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 4, 0);
}

SurfaceDirect3D11Perf::SurfaceDirect3D11Perf()
{
	m_sprites = NULL;
	m_spriteAtlas = NULL;

	m_spriteAtlases = 0;
	m_spritesPerAtlas = 0;
	m_spritesPerModulo = 0;

	m_scheduleSpriteDraw = false;
}

SurfaceDirect3D11Perf::~SurfaceDirect3D11Perf()
{
	if(m_sprites)
		SDL_free(m_sprites);

	for(std::vector<Direct3D11Texture*>::iterator it = m_spritesAtlas.begin(), end = m_spritesAtlas.end(); it != end; ++it)
		releaseDirect3DTexture((*it));

	m_spritesAtlas.clear();
}

void SurfaceDirect3D11Perf::generateSpriteAtlases()
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
	else
	{
		m_spriteAtlases = (g_spriteCounts+4095)/4096;
		m_spritesPerAtlas = 4096;
		m_spritesPerModulo = 2048;
	}
	for(std::vector<Direct3D11Texture*>::iterator it = m_spritesAtlas.begin(), end = m_spritesAtlas.end(); it != end; ++it)
		releaseDirect3DTexture((*it));

	m_spritesAtlas.clear();
	for(Uint32 i = 0; i < m_spriteAtlases; ++i)
	{
		Direct3D11Texture* texture = createDirect3DTexture(m_spritesPerModulo, m_spritesPerModulo, false);
		if(!texture)
		{
			UTIL_MessageBox(true, "Direct3D11: Out of video memory.");
			exit(-1);
			return;
		}
		m_spritesAtlas.push_back(texture);
	}
}

void SurfaceDirect3D11Perf::checkScheduledSprites()
{
	size_t vertices = m_gameWindowVertices.size();
	if(vertices > 0)
	{
		updateTextureScaling(m_spriteAtlas);
		ID3D11DeviceContext_PSSetShaderResources(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, SDL_reinterpret_cast(ID3D11ShaderResourceView**, &m_spriteAtlas->m_resource));
		drawTriangles(m_gameWindowVertices);

		m_gameWindowVertices.clear();
		m_gameWindowVertices.reserve(30720);
	}
}

void SurfaceDirect3D11Perf::init()
{
	SurfaceDirect3D11::init();
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

void SurfaceDirect3D11Perf::spriteManagerReset()
{
	SurfaceDirect3D11::spriteManagerReset();
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

void SurfaceDirect3D11Perf::beginScene()
{
	SurfaceDirect3D11::beginScene();
	m_scheduleSpriteDraw = false;
}

void SurfaceDirect3D11Perf::beginGameScene()
{
	SurfaceDirect3D11::beginGameScene();
	m_gameWindowVertices.clear();
	m_gameWindowVertices.reserve(30720);
	m_scheduleSpriteDraw = true;
}

void SurfaceDirect3D11Perf::endGameScene()
{
	checkScheduledSprites();
	m_scheduleSpriteDraw = false;
	SurfaceDirect3D11::endGameScene();
}

void SurfaceDirect3D11Perf::drawRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(m_scheduleSpriteDraw)
		checkScheduledSprites();
	SurfaceDirect3D11::drawRectangle(x, y, w, h, r, g, b, a);
}

void SurfaceDirect3D11Perf::fillRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(m_scheduleSpriteDraw)
		checkScheduledSprites();
	SurfaceDirect3D11::fillRectangle(x, y, w, h, r, g, b, a);
}

bool SurfaceDirect3D11Perf::loadSprite(Uint32 spriteId, Direct3D11Texture* texture, Uint32 xoff, Uint32 yoff)
{
	unsigned char* pixels = g_engine.LoadSprite(spriteId, true);
	if(!pixels)
		return false;

	ID3D11Texture2D* stagingTexture;
	D3D11_TEXTURE2D_DESC stagingTextureDesc;
	ID3D11Texture2D_GetDesc(SDL_reinterpret_cast(ID3D11Texture2D*, texture->m_texture), &stagingTextureDesc);
	stagingTextureDesc.Width = 32;
	stagingTextureDesc.Height = 32;
	stagingTextureDesc.BindFlags = 0;
	stagingTextureDesc.MiscFlags = 0;
	stagingTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	stagingTextureDesc.Usage = D3D11_USAGE_STAGING;

	HRESULT result = ID3D11Device_CreateTexture2D(SDL_reinterpret_cast(ID3D11Device*, m_device), &stagingTextureDesc, NULL, SDL_reinterpret_cast(ID3D11Texture2D**, &stagingTexture));
	if(FAILED(result))
	{
		SDL_free(pixels);
		return false;
	}

	D3D11_MAPPED_SUBRESOURCE textureMemory;
	result = ID3D11DeviceContext_Map(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, stagingTexture), 0, D3D11_MAP_WRITE, 0, &textureMemory);
	if(FAILED(result))
	{
		SAFE_RELEASE(stagingTexture);
		SDL_free(pixels);
		return false;
	}

	unsigned char* dstData = SDL_reinterpret_cast(unsigned char*, textureMemory.pData);
	UINT length = 32*4;
	if(length == textureMemory.RowPitch)
		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, dstData), SDL_reinterpret_cast(const Uint8*, pixels), length*32);
	else
	{
		unsigned char* srcPixels = pixels;
		if(length > textureMemory.RowPitch)
			length = textureMemory.RowPitch;
		for(Uint32 row = 0; row < texture->m_height; ++row)
		{
			UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, dstData), SDL_reinterpret_cast(const Uint8*, srcPixels), length);
			srcPixels += length;
			dstData += textureMemory.RowPitch;
		}
	}
	ID3D11DeviceContext_Unmap(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, stagingTexture), 0);
	ID3D11DeviceContext_CopySubresourceRegion(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, texture->m_texture), 0, xoff, yoff, 0, SDL_reinterpret_cast(ID3D11Resource*, stagingTexture), 0, NULL);
	SAFE_RELEASE(stagingTexture);
	SDL_free(pixels);
	return true;
}

void SurfaceDirect3D11Perf::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y)
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

	Direct3D11Texture* tex = m_spritesAtlas[spriteAtlas];
	if(!m_sprites[spriteId])
	{
		if(!loadSprite(spriteId, tex, xOffset, yOffset))
			return;//load failed
		m_sprites[spriteId] = true;
	}

	float minx = SDL_static_cast(float, x);
	float maxx = minx+32.0f;
	float miny = SDL_static_cast(float, y);
	float maxy = miny+32.0f;

	float minu = xOffset*tex->m_scaleW;
	float maxu = (xOffset+32)*tex->m_scaleW;
	float minv = yOffset*tex->m_scaleH;
	float maxv = (yOffset+32)*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexD3D11 vertices[4];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].u = minu;
	vertices[0].v = minv;
	vertices[0].color = texColor;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].u = minu;
	vertices[1].v = maxv;
	vertices[1].color = texColor;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].u = maxu;
	vertices[2].v = minv;
	vertices[2].color = texColor;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].u = maxu;
	vertices[3].v = maxv;
	vertices[3].color = texColor;
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
		if(!updateVertexBuffer(vertices, sizeof(vertices)))
			return;

		updateTextureScaling(tex);
		ID3D11DeviceContext_PSSetShaderResources(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, SDL_reinterpret_cast(ID3D11ShaderResourceView**, &tex->m_resource));
		ID3D11DeviceContext_IASetPrimitiveTopology(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		ID3D11DeviceContext_Draw(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 4, 0);
	}
}

void SurfaceDirect3D11Perf::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
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

	Direct3D11Texture* tex = m_spritesAtlas[spriteAtlas];
	if(!m_sprites[spriteId])
	{
		if(!loadSprite(spriteId, tex, xOffset, yOffset))
			return;//load failed
		m_sprites[spriteId] = true;
	}

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	sx += SDL_static_cast(Sint32, xOffset);
	sy += SDL_static_cast(Sint32, yOffset);

	float minu = sx*tex->m_scaleW;
	float maxu = (sx+sw)*tex->m_scaleW;
	float minv = sy*tex->m_scaleH;
	float maxv = (sy+sh)*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);

	VertexD3D11 vertices[4];
	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].u = minu;
	vertices[0].v = minv;
	vertices[0].color = texColor;

	vertices[1].x = minx;
	vertices[1].y = maxy;
	vertices[1].u = minu;
	vertices[1].v = maxv;
	vertices[1].color = texColor;

	vertices[2].x = maxx;
	vertices[2].y = miny;
	vertices[2].u = maxu;
	vertices[2].v = minv;
	vertices[2].color = texColor;

	vertices[3].x = maxx;
	vertices[3].y = maxy;
	vertices[3].u = maxu;
	vertices[3].v = maxv;
	vertices[3].color = texColor;
	if(!updateVertexBuffer(vertices, sizeof(vertices)))
		return;

	updateTextureScaling(tex);
	ID3D11DeviceContext_PSSetShaderResources(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, SDL_reinterpret_cast(ID3D11ShaderResourceView**, &tex->m_resource));
	ID3D11DeviceContext_IASetPrimitiveTopology(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	ID3D11DeviceContext_Draw(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 4, 0);
}

void SurfaceDirect3D11Perf::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor)
{
	if(m_scheduleSpriteDraw)
		checkScheduledSprites();
	SurfaceDirect3D11::drawSpriteMask(spriteId, maskSpriteId, x, y, outfitColor);
}
#endif
