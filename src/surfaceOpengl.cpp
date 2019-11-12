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

#include "surfaceOpengl.h"

#if defined(SDL_VIDEO_RENDER_OGL)
#define GL_VERSION 0x1F02
#define GL_BLEND 0x0BE2
#define GL_TEXTURE_2D 0x0DE1
#define GL_DEPTH_TEST 0x0B71
#define GL_CULL_FACE 0x0B44
#define GL_LIGHTING 0x0B50
#define GL_POLYGON_STIPPLE 0x0B42
#define GL_LINE_SMOOTH 0x0B20
#define GL_LINE_STIPPLE 0x0B24
#define GL_ALPHA_TEST 0x0BC0
#define GL_FOG 0x0B60
#define GL_LOGIC_OP 0x0BF1
#define GL_FRONT_AND_BACK 0x0408
#define GL_FILL 0x1B02
#define GL_TEXTURE_ENV 0x2300
#define GL_TEXTURE_ENV_MODE 0x2200
#define GL_MODULATE 0x2100
#define GL_LINE_LOOP 0x0002
#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_STRIP 0x0005
#define GL_MAX_TEXTURE_SIZE 0x0D33
#define GL_MODELVIEW 0x1700
#define GL_PROJECTION 0x1701
#define GL_ZERO 0
#define GL_ONE 1
#define GL_SRC_COLOR 0x0300
#define GL_ONE_MINUS_SRC_COLOR 0x0301
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_DST_ALPHA 0x0304
#define GL_ONE_MINUS_DST_ALPHA 0x0305
#define GL_DST_COLOR 0x0306
#define GL_ONE_MINUS_DST_COLOR 0x0307
#define GL_SRC_ALPHA_SATURATE 0x0308
#define GL_FUNC_ADD 0x8006
#define GL_SCISSOR_TEST 0x0C11
#define GL_PACK_ALIGNMENT 0x0D05
#define GL_PACK_ROW_LENGTH 0x0D02
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_UNPACK_ROW_LENGTH 0x0CF2
#define GL_FRONT 0x0404
#define GL_BACK 0x0405
#define GL_AUX0 0x0409
#define GL_AUX_BUFFERS 0x0C00
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_RGBA8 0x8058
#define GL_BGRA 0x80E1
#define GL_LUMINANCE 0x1909
#define GL_UNSIGNED_BYTE 0x1401
#define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367
#define GL_FRAMEBUFFER 0x8D40
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_FLOAT 0x1406
#define GL_RG16F 0x822F
#define GL_RG 0x8227
#define GL_HALF_FLOAT 0x140B
#define GL_TEXTURE0_ARB 0x84C0
#define GL_TEXTURE1_ARB 0x84C1

#define GL_OBJECT_COMPILE_STATUS_ARB 0x8B81
#define GL_VERTEX_SHADER_ARB 0x8B31
#define GL_PIXEL_SHADER_ARB 0x8B30

#define OPENGL_CONTEXT_MAJOR 2
#define OPENGL_CONTEXT_MINOR 0

static const float inv255f = (1.0f/255.0f);

extern Engine g_engine;
extern Uint32 g_spriteCounts;
extern Uint16 g_pictureCounts;

SurfaceOpengl::SurfaceOpengl() : m_automapTilesBuff(HARDWARE_MAX_AUTOMAPTILES), m_spritesMaskIds(HARDWARE_MAX_SPRITEMASKS)
{
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	m_oglContext = NULL;
	m_renderTarget = NULL;

	m_gameWindow = NULL;
	m_scaled_gameWindow = NULL;
	m_backup = NULL;

	m_pictures = NULL;
	m_software = NULL;
	m_hardware = NULL;

	m_maxTextureSize = 1024;
	m_integer_scaling_width = 0;
	m_integer_scaling_height = 0;
	m_sharpen_textureSize = -1;

	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = 0;
	m_viewPortH = 0;

	m_totalVRAM = 0;
	m_spritesCache = 0;

	m_useFBO = false;
	m_useAuxBuffer = false;
	m_useNonPower2 = false;
	m_haveSharpening = false;

	#ifdef HAVE_CXX11_SUPPORT
	m_spriteMasks.reserve(HARDWARE_MAX_SPRITEMASKS);
	m_automapTiles.reserve(HARDWARE_MAX_AUTOMAPTILES);
	#endif
}

SurfaceOpengl::~SurfaceOpengl()
{
	if(m_software)
		SDL_free(m_software);

	if(m_hardware)
		SDL_free(m_hardware);

	releaseOpenGLShader(m_program_sharpen);
	if(m_pictures)
	{
		for(Uint16 i = 0; i < g_pictureCounts; ++i)
		{
			if(m_pictures[i])
				releaseOpenGLTexture(m_pictures[i]);
		}

		SDL_free(m_pictures);
	}

	for(U32BGLTextures::iterator it = m_automapTiles.begin(), end = m_automapTiles.end(); it != end; ++it)
		releaseOpenGLTexture(it->second);

	for(U64BGLTextures::iterator it = m_spriteMasks.begin(), end = m_spriteMasks.end(); it != end; ++it)
		releaseOpenGLTexture(it->second);

	m_spriteMasks.clear();
	m_spritesMaskIds.clear();
	m_automapTilesBuff.clear();
	m_automapTiles.clear();
	if(m_backup)
		releaseOpenGLTexture(m_backup);

	if(m_scaled_gameWindow)
		releaseOpenGLTexture(m_scaled_gameWindow);
	
	if(m_gameWindow)
		releaseOpenGLTexture(m_gameWindow);

	SDL_GL_DeleteContext(m_oglContext);
}

bool SurfaceOpengl::createOpenGLShader(OpenglShader& program, const char* vertex_shader, const char* pixel_shader)
{
	while(OglGetError() != 0)
		continue;

	const char* vertex_program[2];
	vertex_program[0] = "";//defines
	vertex_program[1] = vertex_shader;

	const char* pixel_program[2];
	pixel_program[0] = "";//defines
	pixel_program[1] = pixel_shader;

	program.program = OglCreateProgramObjectARB();
	program.vert_shader = OglCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	program.pix_shader = OglCreateShaderObjectARB(GL_PIXEL_SHADER_ARB);

	int status;
	OglShaderSourceARB(program.vert_shader, SDL_arraysize(vertex_program), vertex_program, NULL);
	OglCompileShaderARB(program.vert_shader);
	OglGetObjectParameterivARB(program.vert_shader, GL_OBJECT_COMPILE_STATUS_ARB, &status);
	if(status == 0)
	{
		releaseOpenGLShader(program);
		return false;
	}

	OglShaderSourceARB(program.pix_shader, SDL_arraysize(pixel_program), pixel_program, NULL);
	OglCompileShaderARB(program.pix_shader);
	OglGetObjectParameterivARB(program.pix_shader, GL_OBJECT_COMPILE_STATUS_ARB, &status);
	if(status == 0)
	{
		releaseOpenGLShader(program);
		return false;
	}

	OglAttachObjectARB(program.program, program.vert_shader);
	OglAttachObjectARB(program.program, program.pix_shader);
	OglLinkProgramARB(program.program);

	OglUseProgramObjectARB(program.program);

	int location = OglGetUniformLocationARB(program.program, "tex0");
	if(location >= 0)
		OglUniform1iARB(location, 0);

	OglUseProgramObjectARB(0);
	if(OglGetError() == 0)
		return true;
	else
	{
		releaseOpenGLShader(program);
		return false;
	}
}

void SurfaceOpengl::releaseOpenGLShader(OpenglShader& program)
{
	if(program.vert_shader)
	{
		OglDeleteObjectARB(program.vert_shader);
		program.vert_shader = NULL;
	}

	if(program.pix_shader)
	{
		OglDeleteObjectARB(program.pix_shader);
		program.pix_shader = NULL;
	}

	if(program.program)
	{
		OglDeleteObjectARB(program.program);
		program.program = NULL;
	}
}

OpenglTexture* SurfaceOpengl::createOpenGLTexture(Uint32 width, Uint32 height, bool linearSampler, bool frameBuffer)
{
	OpenglTexture* texture = new OpenglTexture();
	if(!texture)
		return NULL;

	while(OglGetError() != 0)
		continue;

	texture->m_width = width;
	texture->m_height = height;

	OglGenTextures(1, &texture->m_texture);
	if(m_useNonPower2)
	{
		texture->m_scaleW = 1.0f/width;
		texture->m_scaleH = 1.0f/height;
	}
	else
	{
		width = UTIL_power_of_2(width);
		height = UTIL_power_of_2(height);
		texture->m_scaleW = 1.0f/width;
		texture->m_scaleH = 1.0f/height;
	}

	OglBindTexture(GL_TEXTURE_2D, texture->m_texture);
	OglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (linearSampler ? GL_LINEAR : GL_NEAREST));
	OglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (linearSampler ? GL_LINEAR : GL_NEAREST));
	OglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	OglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	#ifdef __MACOSX__
	glTexParameteri(GL_TEXTURE_2D, 0x85BC, 0x85BE);
	#endif
	OglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	if(frameBuffer && m_useFBO)
	{
		OglGenFramebuffers(1, &texture->m_framebuffer);
		OglBindFramebuffer(GL_FRAMEBUFFER, texture->m_framebuffer);
		OglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->m_texture, 0);
		unsigned int status = OglCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(status != GL_FRAMEBUFFER_COMPLETE)
		{
			OglBindFramebuffer(GL_FRAMEBUFFER, 0);
			releaseOpenGLTexture(texture);
			return NULL;
		}
		OglBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	
	if(OglGetError() == 0)
		return texture;
	else
	{
		releaseOpenGLTexture(texture);
		return NULL;
	}
}

bool SurfaceOpengl::updateTextureData(OpenglTexture* texture, unsigned char* data)
{
	OglBindTexture(GL_TEXTURE_2D, texture->m_texture);
	OglPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	OglPixelStorei(GL_UNPACK_ROW_LENGTH, texture->m_width);
	OglTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->m_width, texture->m_height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, data);
	return true;
}

void SurfaceOpengl::releaseOpenGLTexture(OpenglTexture* texture)
{
	if(texture->m_framebuffer)
		OglDeleteFramebuffers(1, &texture->m_framebuffer);
	if(texture->m_texture)
		OglDeleteTextures(1, &texture->m_texture);
	delete texture;
}

void SurfaceOpengl::updateViewport()
{
	if(m_renderTarget)
		OglViewport(m_viewPortX, m_viewPortY, m_viewPortW, m_viewPortH);
	else
	{
		Sint32 w_w, w_h;
		SDL_GL_GetDrawableSize(g_engine.m_window, &w_w, &w_h);
		OglViewport(m_viewPortX, (w_h-m_viewPortY-m_viewPortH), m_viewPortW, m_viewPortH);
	}

	OglMatrixMode(GL_PROJECTION);
	OglLoadIdentity();
	if(m_renderTarget)
		OglOrtho(0.0, SDL_static_cast(double, m_viewPortW), 0.0, SDL_static_cast(double, m_viewPortH), 0.0, 1.0);
	else
		OglOrtho(0.0, SDL_static_cast(double, m_viewPortW), SDL_static_cast(double, m_viewPortH), 0.0, 0.0, 1.0);
	OglMatrixMode(GL_MODELVIEW);
	OglLoadIdentity();
}

void SurfaceOpengl::updateRenderer()
{
	updateViewport();

	OglDisable(GL_POLYGON_STIPPLE);
	OglDisable(GL_LINE_SMOOTH);
	OglDisable(GL_LINE_STIPPLE);
	OglDisable(GL_ALPHA_TEST);
	OglDisable(GL_FOG);
	OglDisable(GL_LOGIC_OP);
	OglDisable(GL_SCISSOR_TEST);

	OglDisable(GL_DEPTH_TEST);
	OglDisable(GL_CULL_FACE);
	OglDisable(GL_LIGHTING);
	OglColorMask(1, 1, 1, 0);

	OglEnable(GL_TEXTURE_2D);
	OglEnable(GL_BLEND);

	OglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	OglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	OglBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	OglBlendEquation(GL_FUNC_ADD);
}

bool SurfaceOpengl::isSupported()
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, OPENGL_CONTEXT_MAJOR);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_CONTEXT_MINOR);
	if(!g_engine.RecreateWindow(false, true))
	{
		UTIL_MessageBox(false, "OpenGL: Failed to create OpenGL window.");
		return false;
	}

	#define OGLLoadFunction(func) \
    do { \
		O##func = SDL_reinterpret_cast(PFN_O##func, SDL_GL_GetProcAddress(#func)); \
		if(!O##func) \
		{ \
			UTIL_MessageBox(false, "OpenGL: Failed to fetch '" #func "' function."); \
			return false; \
		} \
    } while(0)

	m_oglContext = SDL_GL_CreateContext(g_engine.m_window);
	if(!m_oglContext)
	{
		UTIL_MessageBox(false, "OpenGL: Failed to create device context.");
		return false;
	}
	if(SDL_GL_MakeCurrent(g_engine.m_window, m_oglContext) < 0)
	{
		UTIL_MessageBox(false, "OpenGL: Failed to create device context.");
		return false;
	}
	if(g_engine.isVsync())
		SDL_GL_SetSwapInterval(1);
	else
		SDL_GL_SetSwapInterval(0);
	OGLLoadFunction(glBindTexture);
	OGLLoadFunction(glGenTextures);
	OGLLoadFunction(glDeleteTextures);
	OGLLoadFunction(glTexImage2D);
	OGLLoadFunction(glTexSubImage2D);
	OGLLoadFunction(glTexParameteri);
	OGLLoadFunction(glTexEnvi);
	OGLLoadFunction(glPixelStorei);
	OGLLoadFunction(glDrawBuffer);
	OGLLoadFunction(glReadBuffer);
	OGLLoadFunction(glCopyTexSubImage2D);
	OGLLoadFunction(glPolygonMode);
	OGLLoadFunction(glEnable);
	OGLLoadFunction(glDisable);
	OGLLoadFunction(glBegin);
	OGLLoadFunction(glBlendFuncSeparate);
	OGLLoadFunction(glBlendEquation);
	OGLLoadFunction(glColorMask);
	OGLLoadFunction(glColor4f);
	OGLLoadFunction(glTexCoord2f);
	OGLLoadFunction(glVertex2f);
	OGLLoadFunction(glEnd);
	OGLLoadFunction(glReadPixels);
	OGLLoadFunction(glGetIntegerv);
	OGLLoadFunction(glLoadIdentity);
	OGLLoadFunction(glMatrixMode);
	OGLLoadFunction(glOrtho);
	OGLLoadFunction(glViewport);
	OGLLoadFunction(glScissor);
	OGLLoadFunction(glGetString);
	OGLLoadFunction(glGetError);
	OGLLoadFunction(glFinish);
	if(SDL_GL_ExtensionSupported("GL_ARB_framebuffer_object"))
	{
		OGLLoadFunction(glGenFramebuffers);
		OGLLoadFunction(glDeleteFramebuffers);
		OGLLoadFunction(glFramebufferTexture2D);
		OGLLoadFunction(glBindFramebuffer);
		OGLLoadFunction(glCheckFramebufferStatus);
		m_useFBO = true;
	}
	else if(SDL_GL_ExtensionSupported("GL_EXT_framebuffer_object"))
	{
		OglGenFramebuffers = SDL_reinterpret_cast(PFN_OglGenFramebuffers, SDL_GL_GetProcAddress("glGenFramebuffersEXT"));
		if(!OglGenFramebuffers)
		{
			UTIL_MessageBox(false, "OpenGL: Failed to fetch 'glGenFramebuffersEXT' function.");
			return false;
		}
		OglDeleteFramebuffers = SDL_reinterpret_cast(PFN_OglDeleteFramebuffers, SDL_GL_GetProcAddress("glDeleteFramebuffersEXT"));
		if(!OglDeleteFramebuffers)
		{
			UTIL_MessageBox(false, "OpenGL: Failed to fetch 'glDeleteFramebuffersEXT' function.");
			return false;
		}
		OglFramebufferTexture2D = SDL_reinterpret_cast(PFN_OglFramebufferTexture2D, SDL_GL_GetProcAddress("glFramebufferTexture2DEXT"));
		if(!OglFramebufferTexture2D)
		{
			UTIL_MessageBox(false, "OpenGL: Failed to fetch 'glFramebufferTexture2DEXT' function.");
			return false;
		}
		OglBindFramebuffer = SDL_reinterpret_cast(PFN_OglBindFramebuffer, SDL_GL_GetProcAddress("glBindFramebufferEXT"));
		if(!OglBindFramebuffer)
		{
			UTIL_MessageBox(false, "OpenGL: Failed to fetch 'glBindFramebufferEXT' function.");
			return false;
		}
		OglCheckFramebufferStatus = SDL_reinterpret_cast(PFN_OglCheckFramebufferStatus, SDL_GL_GetProcAddress("glCheckFramebufferStatusEXT"));
		if(!OglCheckFramebufferStatus)
		{
			UTIL_MessageBox(false, "OpenGL: Failed to fetch 'glCheckFramebufferStatusEXT' function.");
			return false;
		}
		m_useFBO = true;
	}
	else
	{
		#if RENDERTARGET_WIDTH > GAME_MINIMUM_WIDTH || RENDERTARGET_HEIGHT > GAME_MINIMUM_HEIGHT
		{
			UTIL_MessageBox(false, "OpenGL: Your device doesn't support rendering to texture.");
			return false;
		}
		#endif

		Sint32 auxBuffers;
		OglGetIntegerv(GL_AUX_BUFFERS, &auxBuffers);
		if(auxBuffers > 0)
		{
			//If possible do offscreen rendering with aux buffers
			//instead of backuping portion of screen to texture
			m_useAuxBuffer = true;
		}
	}
	#undef OGLLoadFunction
	if(SDL_GL_ExtensionSupported("GL_ARB_texture_non_power_of_two"))
		m_useNonPower2 = true;

	OglGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_maxTextureSize);
	if(m_maxTextureSize < 1024)
	{
		UTIL_MessageBox(false, "OpenGL: Your device doesn't meet minimum requirement for texture dimension.");
		return false;
	}
	if(SDL_GL_ExtensionSupported("GL_ARB_shader_objects") && SDL_GL_ExtensionSupported("GL_ARB_shading_language_100") && SDL_GL_ExtensionSupported("GL_ARB_vertex_shader") && SDL_GL_ExtensionSupported("GL_ARB_fragment_shader"))
	{
		OglAttachObjectARB = SDL_reinterpret_cast(PFN_OglAttachObjectARB, SDL_GL_GetProcAddress("glAttachObjectARB"));
		OglCompileShaderARB = SDL_reinterpret_cast(PFN_OglCompileShaderARB, SDL_GL_GetProcAddress("glCompileShaderARB"));
		OglCreateProgramObjectARB = SDL_reinterpret_cast(PFN_OglCreateProgramObjectARB, SDL_GL_GetProcAddress("glCreateProgramObjectARB"));
		OglCreateShaderObjectARB = SDL_reinterpret_cast(PFN_OglCreateShaderObjectARB, SDL_GL_GetProcAddress("glCreateShaderObjectARB"));
		OglDeleteObjectARB = SDL_reinterpret_cast(PFN_OglDeleteObjectARB, SDL_GL_GetProcAddress("glDeleteObjectARB"));
		OglGetObjectParameterivARB = SDL_reinterpret_cast(PFN_OglGetObjectParameterivARB, SDL_GL_GetProcAddress("glGetObjectParameterivARB"));
		OglGetUniformLocationARB = SDL_reinterpret_cast(PFN_OglGetUniformLocationARB, SDL_GL_GetProcAddress("glGetUniformLocationARB"));
		OglLinkProgramARB = SDL_reinterpret_cast(PFN_OglLinkProgramARB, SDL_GL_GetProcAddress("glLinkProgramARB"));
		OglShaderSourceARB = SDL_reinterpret_cast(PFN_OglShaderSourceARB, SDL_GL_GetProcAddress("glShaderSourceARB"));
		OglUniform1iARB = SDL_reinterpret_cast(PFN_OglUniform1iARB, SDL_GL_GetProcAddress("glUniform1iARB"));
		OglUniform2fARB = SDL_reinterpret_cast(PFN_OglUniform2fARB, SDL_GL_GetProcAddress("glUniform2fARB")); 
		OglUseProgramObjectARB = SDL_reinterpret_cast(PFN_OglUseProgramObjectARB, SDL_GL_GetProcAddress("glUseProgramObjectARB"));
		if(OglAttachObjectARB && OglCompileShaderARB && OglCreateProgramObjectARB && OglCreateShaderObjectARB && OglDeleteObjectARB && OglGetObjectParameterivARB && OglGetUniformLocationARB && OglLinkProgramARB && OglShaderSourceARB && OglUniform1iARB && OglUniform2fARB && OglUseProgramObjectARB)
		{
			if(createOpenGLShader(m_program_sharpen, OGL_TEXTURE_VERTEX_SHADER, OGL_SHARPEN_PIXEL_SHADER))
			{
				m_sharpen_textureSize = OglGetUniformLocationARB(m_program_sharpen.program, "textureSize");
				if(m_sharpen_textureSize >= 0)
					m_haveSharpening = true;
				else
					releaseOpenGLShader(m_program_sharpen);
			}
		}
	}
	char* version = SDL_reinterpret_cast(char*, OglGetString(GL_VERSION));
	SDL_snprintf(g_buffer, sizeof(g_buffer), "OpenGL %c.%c", version[0], version[2]);
	m_software = SDL_strdup(g_buffer);
	m_hardware = SDL_strdup(SDL_reinterpret_cast(char*, OglGetString(GL_VERSION-1)));
	if(SDL_GL_ExtensionSupported("GL_NVX_gpu_memory_info"))
	{
		int vram = 0;
		OglGetIntegerv(0x9047, &vram);
		m_totalVRAM = UTIL_power_of_2(SDL_static_cast(Uint32, vram)/1024);
	}
	#ifdef SDL_VIDEO_DRIVER_WINDOWS
	else if(SDL_GL_ExtensionSupported("WGL_AMD_gpu_association"))
	{
		typedef UINT (APIENTRY *PFN_OwglGetGPUIDsAMD)(UINT maxCount, UINT* ids);
		typedef INT (APIENTRY *PFN_OwglGetGPUInfoAMD)(UINT id, INT prop, unsigned int dataType, UINT size, void* data);
		PFN_OwglGetGPUIDsAMD OwglGetGPUIDsAMD = SDL_reinterpret_cast(PFN_OwglGetGPUIDsAMD, SDL_GL_GetProcAddress("wglGetGPUIDsAMD"));
		PFN_OwglGetGPUInfoAMD OwglGetGPUInfoAMD = SDL_reinterpret_cast(PFN_OwglGetGPUInfoAMD, SDL_GL_GetProcAddress("wglGetGPUInfoAMD"));
		if(OwglGetGPUIDsAMD && OwglGetGPUInfoAMD)
		{
			UINT n = OwglGetGPUIDsAMD(0, 0);
			UINT* ids = new UINT[n];
			size_t vram = 0;
			OwglGetGPUIDsAMD(n, ids);
			OwglGetGPUInfoAMD(ids[0], 0x21A3, 0x1405, sizeof(size_t), &vram);
			delete[] ids;
			m_totalVRAM = UTIL_power_of_2(SDL_static_cast(Uint32, vram));
		}
	}
	#else
	else if(SDL_GL_ExtensionSupported("GLX_AMD_gpu_association"))
	{
		typedef unsigned int (APIENTRY *PFN_OglXGetGPUIDsAMD)(unsigned int maxCount, unsigned int* ids);
		typedef int (APIENTRY *PFN_OglXGetGPUInfoAMD)(unsigned int id, int prop, unsigned int dataType, unsigned int size, void* data);
		PFN_OglXGetGPUIDsAMD OglXGetGPUIDsAMD = SDL_reinterpret_cast(PFN_OglXGetGPUIDsAMD, SDL_GL_GetProcAddress("glXGetGPUIDsAMD"));
		PFN_OglXGetGPUInfoAMD OglXGetGPUInfoAMD = SDL_reinterpret_cast(PFN_OglXGetGPUInfoAMD, SDL_GL_GetProcAddress("glXGetGPUInfoAMD"));
		if(OglXGetGPUIDsAMD && OglXGetGPUInfoAMD)
		{
			unsigned int n = OglXGetGPUIDsAMD(0, 0);
			unsigned int* ids = new unsigned int[n];
			size_t vram = 0;
			OglXGetGPUIDsAMD(n, ids);
			OglXGetGPUInfoAMD(ids[0], 0x21A3, 0x1405, sizeof(size_t), &vram);
			delete[] ids;
			m_totalVRAM = UTIL_power_of_2(SDL_static_cast(Uint32, vram));
		}
	}
	#endif
	else if(SDL_GL_ExtensionSupported("GL_ATI_meminfo"))
	{
		int vram = 0;
		OglGetIntegerv(0x87FC, &vram);
		m_totalVRAM = UTIL_power_of_2(SDL_static_cast(Uint32, vram)/1024);
	}
	return true;
}

void SurfaceOpengl::init()
{
	m_pictures = SDL_reinterpret_cast(OpenglTexture**, SDL_calloc(g_pictureCounts, sizeof(OpenglTexture*)));
	if(!m_pictures)
	{
		SDL_OutOfMemory();
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Surface::Init() Failed: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
	}

	m_gameWindow = createOpenGLTexture(RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT, (g_engine.getAntialiasing() == CLIENT_ANTIALIASING_NORMAL), true);
	if(!m_gameWindow)
	{
		UTIL_MessageBox(true, "OpenGL: Out of video memory.");
		exit(-1);
		return;
	}
}

void SurfaceOpengl::doResize(Sint32 w, Sint32 h)
{
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = w;
	m_viewPortH = h;
	if(SDL_GL_GetCurrentContext() == m_oglContext)
		updateRenderer();
}

void SurfaceOpengl::spriteManagerReset()
{
	for(U32BGLTextures::iterator it = m_automapTiles.begin(), end = m_automapTiles.end(); it != end; ++it)
		releaseOpenGLTexture(it->second);

	for(U64BGLTextures::iterator it = m_spriteMasks.begin(), end = m_spriteMasks.end(); it != end; ++it)
		releaseOpenGLTexture(it->second);

	m_spriteMasks.clear();
	m_spritesMaskIds.clear();
	m_automapTilesBuff.clear();
	m_automapTiles.clear();
}

unsigned char* SurfaceOpengl::getScreenPixels(Sint32& width, Sint32& height, bool& bgra)
{
	if(SDL_GL_GetCurrentContext() != m_oglContext)
		return NULL;

	SDL_GL_GetDrawableSize(g_engine.m_window, &width, &height);
	while(OglGetError() != 0)
		continue;

	unsigned char* pixels = SDL_reinterpret_cast(unsigned char*, SDL_malloc(width*height*4));
	if(!pixels)
		return NULL;

	OglPixelStorei(GL_PACK_ALIGNMENT, 1);
	OglPixelStorei(GL_PACK_ROW_LENGTH, width);
	OglReadBuffer(GL_FRONT);
	OglFinish();
	OglReadPixels(0, 0, width, height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
	if(OglGetError() != 0)
	{
		SDL_free(pixels);
		return NULL;
	}
	OglReadBuffer(GL_BACK);

	//Flip y-axis of screen as fast as possible
	Sint32 temp_pitch = width*4;
	Uint8* src = SDL_reinterpret_cast(Uint8*, pixels+(height-1)*temp_pitch);
	Uint8* dst = SDL_reinterpret_cast(Uint8*, pixels);
	Uint8* tmp = SDL_stack_alloc(Uint8, temp_pitch);
	Sint32 rows = height/2;
	while(rows--)
	{
		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, tmp), SDL_reinterpret_cast(const Uint8*, dst), SDL_static_cast(size_t, temp_pitch));
		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, dst), SDL_reinterpret_cast(const Uint8*, src), SDL_static_cast(size_t, temp_pitch));
		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, src), SDL_reinterpret_cast(const Uint8*, tmp), SDL_static_cast(size_t, temp_pitch));
		dst += temp_pitch;
		src -= temp_pitch;
	}
	SDL_stack_free(tmp);
	bgra = true;
	return pixels;
}

void SurfaceOpengl::beginScene()
{
	if(SDL_GL_GetCurrentContext() != m_oglContext)
	{
		SDL_GL_MakeCurrent(g_engine.m_window, m_oglContext);
		updateRenderer();
	}
}

void SurfaceOpengl::endScene()
{
	SDL_GL_SwapWindow(g_engine.m_window);
}

void SurfaceOpengl::copyFromScreen(Sint32 x, Sint32 y, Uint32 w, Uint32 h)
{
	if(!m_backup || m_backup->m_width < w || m_backup->m_height < h)
	{
		if(m_backup)
		{
			releaseOpenGLTexture(m_backup);
			m_backup = NULL;
		}

		m_backup = createOpenGLTexture(w, h, false);
		if(!m_backup)
			return;
	}
	OglBindTexture(GL_TEXTURE_2D, m_backup->m_texture);
	OglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, x, m_viewPortH-y-SDL_static_cast(Sint32, h), SDL_static_cast(Sint32, w), SDL_static_cast(Sint32, h));
}

void SurfaceOpengl::drawBackup(Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	if(!m_backup)
		return;

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float maxy = SDL_static_cast(float, y);
	float miny = SDL_static_cast(float, y+h);

	float minu = 0.0f;
	float maxu = w*m_backup->m_scaleW;
	float minv = 0.0f;
	float maxv = h*m_backup->m_scaleH;

	OglBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
	OglBindTexture(GL_TEXTURE_2D, m_backup->m_texture);
	OglBegin(GL_TRIANGLE_STRIP);
	OglTexCoord2f(minu, minv);
	OglVertex2f(minx, miny);
	OglTexCoord2f(minu, maxv);
	OglVertex2f(minx, maxy);
	OglTexCoord2f(maxu, minv);
	OglVertex2f(maxx, miny);
	OglTexCoord2f(maxu, maxv);
	OglVertex2f(maxx, maxy);
	OglEnd();
	OglBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

bool SurfaceOpengl::integer_scaling(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	Sint32 width = RENDERTARGET_WIDTH;
	Sint32 height = RENDERTARGET_HEIGHT;
	UTIL_integer_scale(width, height, w, h, m_maxTextureSize, m_maxTextureSize);
	if(m_integer_scaling_width != width || m_integer_scaling_height != height || !m_scaled_gameWindow)
	{
		if(m_scaled_gameWindow)
		{
			releaseOpenGLTexture(m_scaled_gameWindow);
			m_scaled_gameWindow = NULL;
		}

		m_scaled_gameWindow = createOpenGLTexture(width, height, true, true);
		if(!m_scaled_gameWindow)
			return false;

		m_integer_scaling_width = width;
		m_integer_scaling_height = height;
	}

	OglBindFramebuffer(GL_FRAMEBUFFER, m_scaled_gameWindow->m_framebuffer);
	m_renderTarget = m_scaled_gameWindow;
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = m_scaled_gameWindow->m_width;
	m_viewPortH = m_scaled_gameWindow->m_height;
	updateViewport();

	float minx = 0.0f;
	float maxx = SDL_static_cast(float, width);
	float miny = 0.0f;
	float maxy = SDL_static_cast(float, height);

	float minu = sx*m_gameWindow->m_scaleW;
	float maxu = (sx+sw)*m_gameWindow->m_scaleW;
	float minv = sy*m_gameWindow->m_scaleH;
	float maxv = (sy+sh)*m_gameWindow->m_scaleH;

	OglBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
	OglBindTexture(GL_TEXTURE_2D, m_gameWindow->m_texture);
	OglBegin(GL_TRIANGLE_STRIP);
	OglTexCoord2f(minu, minv);
	OglVertex2f(minx, miny);
	OglTexCoord2f(minu, maxv);
	OglVertex2f(minx, maxy);
	OglTexCoord2f(maxu, minv);
	OglVertex2f(maxx, miny);
	OglTexCoord2f(maxu, maxv);
	OglVertex2f(maxx, maxy);
	OglEnd();

	OglBindFramebuffer(GL_FRAMEBUFFER, 0);
	m_renderTarget = NULL;
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = g_engine.getWindowWidth();
	m_viewPortH = g_engine.getWindowHeight();
	updateViewport();

	minx = SDL_static_cast(float, x);
	maxx = SDL_static_cast(float, x+w);
	miny = SDL_static_cast(float, y);
	maxy = SDL_static_cast(float, y+h);

	minu = 0.0f;
	maxu = m_scaled_gameWindow->m_width*m_scaled_gameWindow->m_scaleW;
	minv = 0.0f;
	maxv = m_scaled_gameWindow->m_height*m_scaled_gameWindow->m_scaleH;

	if(g_engine.isSharpening() && m_haveSharpening)
	{
		OglUseProgramObjectARB(m_program_sharpen.program);
		OglUniform2fARB(m_sharpen_textureSize, 1.0f / w, 1.0f / h);
	}
	OglBindTexture(GL_TEXTURE_2D, m_scaled_gameWindow->m_texture);
	OglBegin(GL_TRIANGLE_STRIP);
	OglTexCoord2f(minu, minv);
	OglVertex2f(minx, miny);
	OglTexCoord2f(minu, maxv);
	OglVertex2f(minx, maxy);
	OglTexCoord2f(maxu, minv);
	OglVertex2f(maxx, miny);
	OglTexCoord2f(maxu, maxv);
	OglVertex2f(maxx, maxy);
	OglEnd();
	OglBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	if(g_engine.isSharpening() && m_haveSharpening)
		OglUseProgramObjectARB(0);
	return true;
}

void SurfaceOpengl::drawGameScene(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	Uint8 antialiasing = g_engine.getAntialiasing();
	if(antialiasing == CLIENT_ANTIALIASING_INTEGER)
	{
		if(m_useFBO && integer_scaling(sx, sy, sw, sh, x, y, w, h))
			return;
		else
		{
			antialiasing = CLIENT_ANTIALIASING_NORMAL;
			g_engine.setAntialiasing(antialiasing);
		}
	}
	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny;
	float maxy;
	if(m_useFBO)
	{
		miny = SDL_static_cast(float, y);
		maxy = SDL_static_cast(float, y+h);
	}
	else
	{
		maxy = SDL_static_cast(float, y);
		miny = SDL_static_cast(float, y+h);
	}

	float minu = sx*m_gameWindow->m_scaleW;
	float maxu = (sx+sw)*m_gameWindow->m_scaleW;
	float minv = sy*m_gameWindow->m_scaleH;
	float maxv = (sy+sh)*m_gameWindow->m_scaleH;

	if(g_engine.isSharpening() && m_haveSharpening)
	{
		OglUseProgramObjectARB(m_program_sharpen.program);
		OglUniform2fARB(m_sharpen_textureSize, 1.0f / w, 1.0f / h);
	}
	OglBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
	OglBindTexture(GL_TEXTURE_2D, m_gameWindow->m_texture);
	OglBegin(GL_TRIANGLE_STRIP);
	OglTexCoord2f(minu, minv);
	OglVertex2f(minx, miny);
	OglTexCoord2f(minu, maxv);
	OglVertex2f(minx, maxy);
	OglTexCoord2f(maxu, minv);
	OglVertex2f(maxx, miny);
	OglTexCoord2f(maxu, maxv);
	OglVertex2f(maxx, maxy);
	OglEnd();
	OglBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	if(g_engine.isSharpening() && m_haveSharpening)
		OglUseProgramObjectARB(0);
}

void SurfaceOpengl::beginGameScene()
{
	if(m_useFBO)
	{
		OglBindFramebuffer(GL_FRAMEBUFFER, m_gameWindow->m_framebuffer);
		m_renderTarget = m_gameWindow;
		m_viewPortX = 0;
		m_viewPortY = 0;
		m_viewPortW = m_gameWindow->m_width;
		m_viewPortH = m_gameWindow->m_height;
		updateViewport();
	}
	else
	{
		setClipRect(0, 0, RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT);
		if(m_useAuxBuffer)
		{
			OglDrawBuffer(GL_AUX0);
			OglReadBuffer(GL_AUX0);
		}
		else
			copyFromScreen(0, 0, RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT);
	}
	fillRectangle(0, 0, RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT, 0x00, 0x00, 0x00, 255);
}

void SurfaceOpengl::endGameScene()
{
	if(m_useFBO)
	{
		OglBindFramebuffer(GL_FRAMEBUFFER, 0);
		m_renderTarget = NULL;
		m_viewPortX = 0;
		m_viewPortY = 0;
		m_viewPortW = g_engine.getWindowWidth();
		m_viewPortH = g_engine.getWindowHeight();
		updateViewport();
	}
	else
	{
		OglBindTexture(GL_TEXTURE_2D, m_gameWindow->m_texture);
		OglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, m_viewPortH-RENDERTARGET_HEIGHT, RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT);
		if(m_useAuxBuffer)
		{
			OglReadBuffer(GL_BACK);
			OglDrawBuffer(GL_BACK);
		}
		else
			drawBackup(0, 0, RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT);
		disableClipRect();
	}
}

void SurfaceOpengl::drawLightMap(LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height)
{
	OglDisable(GL_TEXTURE_2D);
	OglBlendFuncSeparate(GL_ZERO, GL_SRC_COLOR, GL_ZERO, GL_ONE);

	Sint32 drawY = y-scale-(scale/2);
	height -= 1;
	for(Sint32 j = -1; j < height; ++j)
	{
		Sint32 offset1 = (j + 1) * width;
		Sint32 offset2 = UTIL_max<Sint32>(j, 0) * width;
		Sint32 drawX = x-scale-(scale/2);
		OglBegin(GL_TRIANGLE_STRIP);
		for(Sint32 k = -1; k < width; ++k)
		{
			Sint32 offset = UTIL_max<Sint32>(k, 0);
			OglColor4f(lightmap[offset1+offset].r*inv255f, lightmap[offset1+offset].g*inv255f, lightmap[offset1+offset].b*inv255f, 1.0f);
			OglVertex2f(SDL_static_cast(float, drawX), SDL_static_cast(float, drawY+scale));
			OglColor4f(lightmap[offset2+offset].r*inv255f, lightmap[offset2+offset].g*inv255f, lightmap[offset2+offset].b*inv255f, 1.0f);
			OglVertex2f(SDL_static_cast(float, drawX), SDL_static_cast(float, drawY));
			drawX += scale;
		}

		drawY += scale;
		OglEnd();
	}

	OglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	OglBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	OglEnable(GL_TEXTURE_2D);
}

void SurfaceOpengl::setClipRect(Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	OglEnable(GL_SCISSOR_TEST);
	if(m_renderTarget)
		OglScissor((m_viewPortX+x), (m_viewPortY+y), w, h);
	else
	{
		Sint32 w_w, w_h;
		SDL_GL_GetDrawableSize(g_engine.m_window, &w_w, &w_h);
		OglScissor((m_viewPortX+x), (w_h-m_viewPortY-y-h), w, h);
	}
}

void SurfaceOpengl::disableClipRect()
{
	OglDisable(GL_SCISSOR_TEST);
}

void SurfaceOpengl::drawRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	float minx = SDL_static_cast(float, x)+0.5f;
	float maxx = SDL_static_cast(float, x+w)+0.5f;
	float miny = SDL_static_cast(float, y)+0.5f;
	float maxy = SDL_static_cast(float, y+h)+0.5f;

	OglDisable(GL_TEXTURE_2D);
	OglColor4f(r*inv255f, g*inv255f, b*inv255f, a*inv255f);
	OglBegin(GL_LINE_LOOP);
	OglVertex2f(minx, miny);
	OglVertex2f(maxx, miny);
	OglVertex2f(maxx, maxy);
	OglVertex2f(minx, maxy);
	OglEnd();
	OglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	OglEnable(GL_TEXTURE_2D);
}

void SurfaceOpengl::fillRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	OglDisable(GL_TEXTURE_2D);
	OglColor4f(r*inv255f, g*inv255f, b*inv255f, a*inv255f);
	OglBegin(GL_TRIANGLE_STRIP);
	OglVertex2f(minx, miny);
	OglVertex2f(minx, maxy);
	OglVertex2f(maxx, miny);
	OglVertex2f(maxx, maxy);
	OglEnd();
	OglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	OglEnable(GL_TEXTURE_2D);
}

OpenglTexture* SurfaceOpengl::loadPicture(Uint16 pictureId, bool linear)
{
	Sint32 width, height;
	unsigned char* pixels = g_engine.LoadPicture(pictureId, true, width, height);
	if(!pixels)
		return NULL;

	OpenglTexture* s = createOpenGLTexture(width, height, linear);
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

void SurfaceOpengl::drawFont(Uint16 pictureId, Sint32 x, Sint32 y, const std::string& text, size_t pos, size_t len, Uint8 r, Uint8 g, Uint8 b, Sint32 cX[256], Sint32 cY[256], Sint32 cW[256], Sint32 cH[256])
{
	OpenglTexture* tex = m_pictures[pictureId];
	if(!tex)
	{
		tex = loadPicture(pictureId, false);
		if(!tex)
			return;//load failed
	}

	OglBindTexture(GL_TEXTURE_2D, tex->m_texture);
	OglColor4f(r*inv255f, g*inv255f, b*inv255f, 1.0f);
	OglBegin(GL_TRIANGLES);

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
				rx += cW[character] + cX[0];
				break;
			case 0x0E://Special case - change rendering color
			{
				if(i+4 < len)//First check if we have the color bytes
				{
					Uint8 red = SDL_static_cast(Uint8, text[i+1]);
					Uint8 green = SDL_static_cast(Uint8, text[i+2]);
					Uint8 blue = SDL_static_cast(Uint8, text[i+3]);
					OglColor4f(red*inv255f, green*inv255f, blue*inv255f, 1.0f);
					i += 3;
				}
			}
			break;
			case 0x0F://Special case - change back standard color
				OglColor4f(r*inv255f, g*inv255f, b*inv255f, 1.0f);
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

				OglTexCoord2f(minu, minv);
				OglVertex2f(minx, miny);
				OglTexCoord2f(minu, maxv);
				OglVertex2f(minx, maxy);
				OglTexCoord2f(maxu, minv);
				OglVertex2f(maxx, miny);
				OglTexCoord2f(maxu, maxv);
				OglVertex2f(maxx, maxy);
				OglTexCoord2f(maxu, minv);
				OglVertex2f(maxx, miny);
				OglTexCoord2f(minu, maxv);
				OglVertex2f(minx, maxy);
				rx += cW[character] + cX[0];
			}
			break;
		}
	}
	OglEnd();
	OglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void SurfaceOpengl::drawBackground(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	OpenglTexture* tex = m_pictures[pictureId];
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

	OglBindTexture(GL_TEXTURE_2D, tex->m_texture);
	OglBegin(GL_TRIANGLE_STRIP);
	OglTexCoord2f(minu, minv);
	OglVertex2f(minx, miny);
	OglTexCoord2f(minu, maxv);
	OglVertex2f(minx, maxy);
	OglTexCoord2f(maxu, minv);
	OglVertex2f(maxx, miny);
	OglTexCoord2f(maxu, maxv);
	OglVertex2f(maxx, maxy);
	OglEnd();
}

void SurfaceOpengl::drawPictureRepeat(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	OpenglTexture* tex = m_pictures[pictureId];
	if(!tex)
	{
		tex = loadPicture(pictureId, false);
		if(!tex)
			return;//load failed
	}

	OglBindTexture(GL_TEXTURE_2D, tex->m_texture);
	OglBegin(GL_TRIANGLES);

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

			OglTexCoord2f(minu, minv);
			OglVertex2f(minx, miny);
			OglTexCoord2f(minu, maxv);
			OglVertex2f(minx, maxy);
			OglTexCoord2f(maxu, minv);
			OglVertex2f(maxx, miny);
			OglTexCoord2f(maxu, maxv);
			OglVertex2f(maxx, maxy);
			OglTexCoord2f(maxu, minv);
			OglVertex2f(maxx, miny);
			OglTexCoord2f(minu, maxv);
			OglVertex2f(minx, maxy);
			cx += sw;
		}
		y += sh;
	}
	OglEnd();
}

void SurfaceOpengl::drawPicture(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	OpenglTexture* tex = m_pictures[pictureId];
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

	OglBindTexture(GL_TEXTURE_2D, tex->m_texture);
	OglBegin(GL_TRIANGLE_STRIP);
	OglTexCoord2f(minu, minv);
	OglVertex2f(minx, miny);
	OglTexCoord2f(minu, maxv);
	OglVertex2f(minx, maxy);
	OglTexCoord2f(maxu, minv);
	OglVertex2f(maxx, miny);
	OglTexCoord2f(maxu, maxv);
	OglVertex2f(maxx, maxy);
	OglEnd();
}

OpenglTexture* SurfaceOpengl::loadSpriteMask(Uint64 tempPos, Uint32 spriteId, Uint32 maskSpriteId, Uint32 outfitColor)
{
	unsigned char* pixels = g_engine.LoadSpriteMask(spriteId, maskSpriteId, outfitColor, true);
	if(!pixels)
		return NULL;

	if(m_spriteMasks.size() >= HARDWARE_MAX_SPRITEMASKS)
	{
		U64BGLTextures::iterator it = m_spriteMasks.find(m_spritesMaskIds.front());
		if(it == m_spriteMasks.end())//Something weird happen - let's erase the first one entry
			it = m_spriteMasks.begin();

		releaseOpenGLTexture(it->second);
		m_spriteMasks.erase(it);
		m_spritesMaskIds.pop_front();
	}
	OpenglTexture* s = createOpenGLTexture(32, 32, false);
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

void SurfaceOpengl::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor)
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
	OpenglTexture* tex;
	U64BGLTextures::iterator it = m_spriteMasks.find(tempPos);
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

	OglBindTexture(GL_TEXTURE_2D, tex->m_texture);
	OglBegin(GL_TRIANGLE_STRIP);
	OglTexCoord2f(minu, minv);
	OglVertex2f(minx, miny);
	OglTexCoord2f(minu, maxv);
	OglVertex2f(minx, maxy);
	OglTexCoord2f(maxu, minv);
	OglVertex2f(maxx, miny);
	OglTexCoord2f(maxu, maxv);
	OglVertex2f(maxx, maxy);
	OglEnd();
}

void SurfaceOpengl::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Uint32 outfitColor)
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
	OpenglTexture* tex;
	U64BGLTextures::iterator it = m_spriteMasks.find(tempPos);
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

	OglBindTexture(GL_TEXTURE_2D, tex->m_texture);
	OglBegin(GL_TRIANGLE_STRIP);
	OglTexCoord2f(minu, minv);
	OglVertex2f(minx, miny);
	OglTexCoord2f(minu, maxv);
	OglVertex2f(minx, maxy);
	OglTexCoord2f(maxu, minv);
	OglVertex2f(maxx, miny);
	OglTexCoord2f(maxu, maxv);
	OglVertex2f(maxx, maxy);
	OglEnd();
}

OpenglTexture* SurfaceOpengl::createAutomapTile(Uint32 currentArea)
{
	if(m_automapTiles.size() >= HARDWARE_MAX_AUTOMAPTILES)
	{
		U32BGLTextures::iterator it = m_automapTiles.find(m_automapTilesBuff.front());
		if(it == m_automapTiles.end())//Something weird happen - let's erase the first one entry
			it = m_automapTiles.begin();

		releaseOpenGLTexture(it->second);
		m_automapTiles.erase(it);
		m_automapTilesBuff.pop_front();
	}
	OpenglTexture* s = createOpenGLTexture(256, 256, false);
	if(!s)
		return NULL;

	m_automapTiles[currentArea] = s;
	m_automapTilesBuff.push_back(currentArea);
	return s;
}

void SurfaceOpengl::uploadAutomapTile(OpenglTexture* texture, Uint8 color[256][256])
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

void SurfaceOpengl::drawAutomapTile(Uint32 currentArea, bool& recreate, Uint8 color[256][256], Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
{
	OpenglTexture* tex;
	U32BGLTextures::iterator it = m_automapTiles.find(currentArea);
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

	OglBindTexture(GL_TEXTURE_2D, tex->m_texture);
	OglBegin(GL_TRIANGLE_STRIP);
	OglTexCoord2f(minu, minv);
	OglVertex2f(minx, miny);
	OglTexCoord2f(minu, maxv);
	OglVertex2f(minx, maxy);
	OglTexCoord2f(maxu, minv);
	OglVertex2f(maxx, miny);
	OglTexCoord2f(maxu, maxv);
	OglVertex2f(maxx, maxy);
	OglEnd();
}

SurfaceOpenglComp::SurfaceOpenglComp()
{
	m_sprites = NULL;
}

SurfaceOpenglComp::~SurfaceOpenglComp()
{
	if(m_sprites)
	{
		for(Uint32 i = 1; i <= m_spritesCache; ++i)
		{
			if(m_sprites[i])
				releaseOpenGLTexture(m_sprites[i]);
		}

		SDL_free(m_sprites);
	}
}

void SurfaceOpenglComp::init()
{
	SurfaceOpengl::init();
	m_sprites = SDL_reinterpret_cast(OpenglTexture**, SDL_calloc(g_spriteCounts+1, sizeof(OpenglTexture*)));
	if(!m_sprites)
	{
		SDL_OutOfMemory();
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Surface::Init() Failed: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
	}
	m_spritesCache = g_spriteCounts;
}

void SurfaceOpenglComp::spriteManagerReset()
{
	SurfaceOpengl::spriteManagerReset();
	if(m_sprites)
	{
		for(Uint32 i = 1; i <= m_spritesCache; ++i)
		{
			if(m_sprites[i])
				releaseOpenGLTexture(m_sprites[i]);
		}

		SDL_free(m_sprites);
	}
	m_sprites = SDL_reinterpret_cast(OpenglTexture**, SDL_calloc(g_spriteCounts+1, sizeof(OpenglTexture*)));
	if(!m_sprites)
	{
		SDL_OutOfMemory();
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Surface::spriteManagerReset() Failed: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
	}
	m_spritesCache = g_spriteCounts;
}

OpenglTexture* SurfaceOpenglComp::loadSprite(Uint32 spriteId)
{
	unsigned char* pixels = g_engine.LoadSprite(spriteId, true);
	if(!pixels)
		return NULL;

	OpenglTexture* s = createOpenGLTexture(32, 32, false);
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

void SurfaceOpenglComp::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y)
{
	if(spriteId > g_spriteCounts)
		return;

	OpenglTexture* tex = m_sprites[spriteId];
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

	OglBindTexture(GL_TEXTURE_2D, tex->m_texture);
	OglBegin(GL_TRIANGLE_STRIP);
	OglTexCoord2f(minu, minv);
	OglVertex2f(minx, miny);
	OglTexCoord2f(minu, maxv);
	OglVertex2f(minx, maxy);
	OglTexCoord2f(maxu, minv);
	OglVertex2f(maxx, miny);
	OglTexCoord2f(maxu, maxv);
	OglVertex2f(maxx, maxy);
	OglEnd();
}

void SurfaceOpenglComp::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
{
	if(spriteId > g_spriteCounts)
		return;

	OpenglTexture* tex = m_sprites[spriteId];
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
	
	OglBindTexture(GL_TEXTURE_2D, tex->m_texture);
	OglBegin(GL_TRIANGLE_STRIP);
	OglTexCoord2f(minu, minv);
	OglVertex2f(minx, miny);
	OglTexCoord2f(minu, maxv);
	OglVertex2f(minx, maxy);
	OglTexCoord2f(maxu, minv);
	OglVertex2f(maxx, miny);
	OglTexCoord2f(maxu, maxv);
	OglVertex2f(maxx, maxy);
	OglEnd();
}

SurfaceOpenglPerf::SurfaceOpenglPerf()
{
	m_sprites = NULL;
	m_spriteAtlas = NULL;

	m_spriteAtlases = 0;
	m_spritesPerAtlas = 0;
	m_spritesPerModulo = 0;

	m_scheduleSpriteDraw = false;
}

SurfaceOpenglPerf::~SurfaceOpenglPerf()
{
	if(m_sprites)
		SDL_free(m_sprites);

	for(std::vector<OpenglTexture*>::iterator it = m_spritesAtlas.begin(), end = m_spritesAtlas.end(); it != end; ++it)
		releaseOpenGLTexture((*it));

	m_spritesAtlas.clear();
}

void SurfaceOpenglPerf::drawTriangles(std::vector<float>& vertices, std::vector<float>& texCoords)
{
	OglBegin(GL_TRIANGLES);
	for(size_t i = 0; i < vertices.size(); i += 2)
	{
		OglTexCoord2f(texCoords[i], texCoords[i + 1]);
		OglVertex2f(vertices[i], vertices[i + 1]);
	}
	OglEnd();
}

void SurfaceOpenglPerf::generateSpriteAtlases()
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
	for(std::vector<OpenglTexture*>::iterator it = m_spritesAtlas.begin(), end = m_spritesAtlas.end(); it != end; ++it)
		releaseOpenGLTexture((*it));

	m_spritesAtlas.clear();
	for(Uint32 i = 0; i < m_spriteAtlases; ++i)
	{
		OpenglTexture* texture = createOpenGLTexture(m_spritesPerModulo, m_spritesPerModulo, false);
		if(!texture)
		{
			UTIL_MessageBox(true, "OpenGL: Out of video memory.");
			exit(-1);
			return;
		}
		m_spritesAtlas.push_back(texture);
	}
}

void SurfaceOpenglPerf::checkScheduledSprites()
{
	size_t vertices = m_gameWindowVertices.size();
	if(vertices > 0)
	{
		OglBindTexture(GL_TEXTURE_2D, m_spriteAtlas->m_texture);
		drawTriangles(m_gameWindowVertices, m_gameWindowTexCoords);

		m_gameWindowVertices.clear();
		m_gameWindowTexCoords.clear();
		m_gameWindowVertices.reserve(61440);
		m_gameWindowTexCoords.reserve(61440);
	}
}

void SurfaceOpenglPerf::init()
{
	SurfaceOpengl::init();
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

void SurfaceOpenglPerf::spriteManagerReset()
{
	SurfaceOpengl::spriteManagerReset();
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

void SurfaceOpenglPerf::beginScene()
{
	SurfaceOpengl::beginScene();
	m_scheduleSpriteDraw = false;
}

void SurfaceOpenglPerf::beginGameScene()
{
	SurfaceOpengl::beginGameScene();
	m_gameWindowVertices.clear();
	m_gameWindowTexCoords.clear();
	m_gameWindowVertices.reserve(61440);
	m_gameWindowTexCoords.reserve(61440);
	m_scheduleSpriteDraw = true;
}

void SurfaceOpenglPerf::endGameScene()
{
	checkScheduledSprites();
	m_scheduleSpriteDraw = false;
	SurfaceOpengl::endGameScene();
}

void SurfaceOpenglPerf::drawRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(m_scheduleSpriteDraw)
		checkScheduledSprites();
	SurfaceOpengl::drawRectangle(x, y, w, h, r, g, b, a);
}

void SurfaceOpenglPerf::fillRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(m_scheduleSpriteDraw)
		checkScheduledSprites();
	SurfaceOpengl::fillRectangle(x, y, w, h, r, g, b, a);
}

bool SurfaceOpenglPerf::loadSprite(Uint32 spriteId, OpenglTexture* texture, Uint32 xoff, Uint32 yoff)
{
	unsigned char* pixels = g_engine.LoadSprite(spriteId, true);
	if(!pixels)
		return false;

	OglBindTexture(GL_TEXTURE_2D, texture->m_texture);
	OglPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	OglPixelStorei(GL_UNPACK_ROW_LENGTH, 32);
	OglTexSubImage2D(GL_TEXTURE_2D, 0, SDL_static_cast(int, xoff), SDL_static_cast(int, yoff), 32, 32, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
	SDL_free(pixels);
	return true;
}

void SurfaceOpenglPerf::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y)
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

	OpenglTexture* tex = m_spritesAtlas[spriteAtlas];
	if(!m_sprites[spriteId])
	{
		if(!loadSprite(spriteId, tex, xOffset, yOffset))
			return;//load failed
		m_sprites[spriteId] = true;
	}

	float minx = SDL_static_cast(float, x);
	float maxx = minx+32.f;
	float miny = SDL_static_cast(float, y);
	float maxy = miny+32.f;

	float minu = xOffset*tex->m_scaleW;
	float maxu = (xOffset+32)*tex->m_scaleW;
	float minv = yOffset*tex->m_scaleH;
	float maxv = (yOffset+32)*tex->m_scaleH;
	if(m_scheduleSpriteDraw)
	{
		if(tex != m_spriteAtlas)
		{
			checkScheduledSprites();
			m_spriteAtlas = tex;
		}

		m_gameWindowVertices.push_back(minx); m_gameWindowVertices.push_back(miny);
		m_gameWindowVertices.push_back(minx); m_gameWindowVertices.push_back(maxy);
		m_gameWindowVertices.push_back(maxx); m_gameWindowVertices.push_back(miny);
		m_gameWindowVertices.push_back(maxx); m_gameWindowVertices.push_back(maxy);
		m_gameWindowVertices.push_back(maxx); m_gameWindowVertices.push_back(miny);
		m_gameWindowVertices.push_back(minx); m_gameWindowVertices.push_back(maxy);

		m_gameWindowTexCoords.push_back(minu); m_gameWindowTexCoords.push_back(minv);
		m_gameWindowTexCoords.push_back(minu); m_gameWindowTexCoords.push_back(maxv);
		m_gameWindowTexCoords.push_back(maxu); m_gameWindowTexCoords.push_back(minv);
		m_gameWindowTexCoords.push_back(maxu); m_gameWindowTexCoords.push_back(maxv);
		m_gameWindowTexCoords.push_back(maxu); m_gameWindowTexCoords.push_back(minv);
		m_gameWindowTexCoords.push_back(minu); m_gameWindowTexCoords.push_back(maxv);
	}
	else
	{
		OglBindTexture(GL_TEXTURE_2D, tex->m_texture);
		OglBegin(GL_TRIANGLE_STRIP);
		OglTexCoord2f(minu, minv);
		OglVertex2f(minx, miny);
		OglTexCoord2f(minu, maxv);
		OglVertex2f(minx, maxy);
		OglTexCoord2f(maxu, minv);
		OglVertex2f(maxx, miny);
		OglTexCoord2f(maxu, maxv);
		OglVertex2f(maxx, maxy);
		OglEnd();
	}
}

void SurfaceOpenglPerf::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
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

	OpenglTexture* tex = m_spritesAtlas[spriteAtlas];
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

	OglBindTexture(GL_TEXTURE_2D, tex->m_texture);
	OglBegin(GL_TRIANGLE_STRIP);
	OglTexCoord2f(minu, minv);
	OglVertex2f(minx, miny);
	OglTexCoord2f(minu, maxv);
	OglVertex2f(minx, maxy);
	OglTexCoord2f(maxu, minv);
	OglVertex2f(maxx, miny);
	OglTexCoord2f(maxu, maxv);
	OglVertex2f(maxx, maxy);
	OglEnd();
}

void SurfaceOpenglPerf::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor)
{
	if(m_scheduleSpriteDraw)
		checkScheduledSprites();
	SurfaceOpengl::drawSpriteMask(spriteId, maskSpriteId, x, y, outfitColor);
}
#endif
