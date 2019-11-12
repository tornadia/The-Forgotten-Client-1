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

#include "surfaceOpengles2.h"

#if defined(SDL_VIDEO_RENDER_OGL_ES2)
#define GL_TRUE 1
#define GL_FALSE 0
#define GL_VERSION 0x1F02
#define GL_BLEND 0x0BE2
#define GL_TEXTURE_2D 0x0DE1
#define GL_DEPTH_TEST 0x0B71
#define GL_CULL_FACE 0x0B44
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
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_RGBA 0x1908
#define GL_LUMINANCE 0x1909
#define GL_UNSIGNED_BYTE 0x1401
#define GL_FRAMEBUFFER 0x8D40
#define GL_FRAMEBUFFER_BINDING 0x8CA6
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_FLOAT 0x1406
#define GL_RG16F 0x822F
#define GL_RG 0x8227
#define GL_HALF_FLOAT 0x140B
#define GL_TEXTURE0 0x84C0
#define GL_ARRAY_BUFFER 0x8892
#define GL_STREAM_DRAW 0x88E0

#define GL_COMPILE_STATUS 0x8B81
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_PIXEL_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31

#define OPENGL_CONTEXT_MAJOR 2
#define OPENGL_CONTEXT_MINOR 0

static const float inv255f = (1.0f/255.0f);

extern Engine g_engine;
extern Uint32 g_spriteCounts;
extern Uint16 g_pictureCounts;

SurfaceOpenglES2::SurfaceOpenglES2() : m_automapTilesBuff(HARDWARE_MAX_AUTOMAPTILES), m_spritesMaskIds(HARDWARE_MAX_SPRITEMASKS)
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

	m_pictures = NULL;
	m_software = NULL;
	m_hardware = NULL;

	m_sharpen_textureSize = -1;
	window_framebuffer = 0;

	m_maxTextureSize = 1024;
	m_integer_scaling_width = 0;
	m_integer_scaling_height = 0;

	m_vertex_shader = 0;
	m_solidcolors_pixel_shader = 0;
	m_solidcolors2_pixel_shader = 0;
	m_texture_pixel_shader = 0;
	m_colortexture_pixel_shader = 0;
	m_sharpen_pixel_shader = 0;

	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = 0;
	m_viewPortH = 0;

	m_totalVRAM = 0;
	m_spritesCache = 0;
	m_haveSharpening = false;

	#ifdef GLES2_FORCE_VBOS
	for(Sint32 i = 0; i < GLES_ATTRIBUTES; ++i)
	{
		m_vertex_buffers[i] = 0;
		m_vertex_buffer_size[i] = 0;
	}
	#endif

	#ifdef HAVE_CXX11_SUPPORT
	m_spriteMasks.reserve(HARDWARE_MAX_SPRITEMASKS);
	m_automapTiles.reserve(HARDWARE_MAX_AUTOMAPTILES);
	#endif
}

SurfaceOpenglES2::~SurfaceOpenglES2()
{
	#ifdef GLES2_FORCE_VBOS
	for(Sint32 i = 0; i < GLES_ATTRIBUTES; ++i)
	{
		if(m_vertex_buffers[i])
			OglDeleteBuffers(1, &m_vertex_buffers[i]);
	}
	#endif

	if(m_software)
		SDL_free(m_software);

	if(m_hardware)
		SDL_free(m_hardware);

	releaseOpenGLES2Program(m_programSolidColors);
	releaseOpenGLES2Program(m_programSolidColors2);
	releaseOpenGLES2Program(m_programTexture);
	releaseOpenGLES2Program(m_programSharpen);
	releaseOpenGLES2Shader(m_sharpen_pixel_shader);
	releaseOpenGLES2Shader(m_colortexture_pixel_shader);
	releaseOpenGLES2Shader(m_texture_pixel_shader);
	releaseOpenGLES2Shader(m_solidcolors2_pixel_shader);
	releaseOpenGLES2Shader(m_solidcolors_pixel_shader);
	releaseOpenGLES2Shader(m_vertex_shader);
	if(m_pictures)
	{
		for(Uint16 i = 0; i < g_pictureCounts; ++i)
		{
			if(m_pictures[i])
				releaseOpenGLES2Texture(m_pictures[i]);
		}

		SDL_free(m_pictures);
	}

	for(U32BGLES2Textures::iterator it = m_automapTiles.begin(), end = m_automapTiles.end(); it != end; ++it)
		releaseOpenGLES2Texture(it->second);

	for(U64BGLES2Textures::iterator it = m_spriteMasks.begin(), end = m_spriteMasks.end(); it != end; ++it)
		releaseOpenGLES2Texture(it->second);

	m_spriteMasks.clear();
	m_spritesMaskIds.clear();
	m_automapTilesBuff.clear();
	m_automapTiles.clear();
	if(m_scaled_gameWindow)
		releaseOpenGLES2Texture(m_scaled_gameWindow);

	if(m_gameWindow)
		releaseOpenGLES2Texture(m_gameWindow);

	SDL_GL_DeleteContext(m_oglContext);
}

bool SurfaceOpenglES2::createOpenGLES2Shader(unsigned int& shader, const char* data, bool vertex_shader)
{
	const char* shader_data[2];
	shader_data[0] = "";//defines
	shader_data[1] = data;

	int status;
	shader = OglCreateShader(vertex_shader ? GL_VERTEX_SHADER : GL_PIXEL_SHADER);
	OglShaderSource(shader, SDL_arraysize(shader_data), shader_data, NULL);
	OglCompileShader(shader);
	OglGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if(status == 0)
	{
		/*char* info = NULL;
		int length;
		OglGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		if(length > 0)
		{
			info = SDL_stack_alloc(char, length);
			if(info)
				OglGetShaderInfoLog(shader, length, &length, info);
		}
		if(info)
		{
			SDL_snprintf(g_buffer, sizeof(g_buffer), "OpenGL ES: Failed to create %s shader:\n%s.", (vertex_shader ? "vertex" : "fragment"), info);
			UTIL_MessageBox(false, g_buffer);
		}
		else
		{
			SDL_snprintf(g_buffer, sizeof(g_buffer), "OpenGL ES: Failed to create %s shader.", (vertex_shader ? "vertex" : "fragment"));
			UTIL_MessageBox(false, g_buffer);
		}*/
		OglDeleteShader(shader);
		return false;
	}
	return true;
}

bool SurfaceOpenglES2::createOpenGLES2Program(OpenglES2Program& program, unsigned int vertex_shader, unsigned int pixel_shader)
{
	program.program = OglCreateProgram();
	program.vert_shader = vertex_shader;
	program.pix_shader = pixel_shader;

	int status;
	OglAttachShader(program.program, program.vert_shader);
	OglAttachShader(program.program, program.pix_shader);
	OglBindAttribLocation(program.program, GLES_ATTRIBUTE_POSITION, "a_position");
	OglBindAttribLocation(program.program, GLES_ATTRIBUTE_TEXCOORD, "a_texCoord");
	OglBindAttribLocation(program.program, GLES_ATTRIBUTE_COLORS, "a_color");
	OglLinkProgram(program.program);
	OglGetProgramiv(program.program, 0x8B82, &status);
	if(status == 0)
	{
		/*char* info = NULL;
		int length;
		OglGetProgramiv(program.program, GL_INFO_LOG_LENGTH, &length);
		if(length > 0)
		{
			info = SDL_stack_alloc(char, length);
			if(info)
				OglGetProgramInfoLog(program.program, length, &length, info);
		}
		if(info)
		{
			SDL_snprintf(g_buffer, sizeof(g_buffer), "OpenGL ES: Failed to create program:\n%s.", info);
			UTIL_MessageBox(false, g_buffer);
		}
		else
		{
			SDL_snprintf(g_buffer, sizeof(g_buffer), "OpenGL ES: Failed to create program.");
			UTIL_MessageBox(false, g_buffer);
		}*/
		releaseOpenGLES2Program(program);
		return false;
	}
	OglUseProgram(program.program);

	int location = OglGetUniformLocation(program.program, "u_texture");
	if(location >= 0)
		OglUniform1i(location, 0);

	program.modulationLocation = OglGetUniformLocation(program.program, "u_modulation");
	if(program.modulationLocation >= 0)
		OglUniform4f(program.modulationLocation, 1.0f, 1.0f, 1.0f, 1.0f);

	float projection[16];//4*4 matrix
	for(int i = 0; i < 16; ++i)
		projection[i] = 0.0f;

	program.projectionLocation = OglGetUniformLocation(program.program, "u_projection");
	if(program.projectionLocation >= 0)
		OglUniformMatrix4fv(program.projectionLocation, 1, GL_FALSE, projection);

	return true;
}

void SurfaceOpenglES2::selectShader(OpenglES2Program& program)
{
	OglUseProgram(program.program);
}

void SurfaceOpenglES2::releaseOpenGLES2Program(OpenglES2Program& program)
{
	if(program.vert_shader)
		program.vert_shader = 0;

	if(program.pix_shader)
		program.pix_shader = 0;

	if(program.program)
	{
		OglDeleteProgram(program.program);
		program.program = 0;
	}
}

void SurfaceOpenglES2::releaseOpenGLES2Shader(unsigned int& shader)
{
	if(shader)
	{
		OglDeleteShader(shader);
		shader = 0;
	}
}

OpenglES2Texture* SurfaceOpenglES2::createOpenGLES2Texture(Uint32 width, Uint32 height, bool linearSampler, bool frameBuffer)
{
	OpenglES2Texture* texture = new OpenglES2Texture();
	if(!texture)
		return NULL;

	while(OglGetError() != 0)
		continue;

	texture->m_width = width;
	texture->m_height = height;

	OglGenTextures(1, &texture->m_texture);
	texture->m_scaleW = 1.0f/width;
	texture->m_scaleH = 1.0f/height;

	OglBindTexture(GL_TEXTURE_2D, texture->m_texture);
	OglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (linearSampler ? GL_LINEAR : GL_NEAREST));
	OglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (linearSampler ? GL_LINEAR : GL_NEAREST));
	OglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	OglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	OglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	if(frameBuffer)
	{
		OglGenFramebuffers(1, &texture->m_framebuffer);
		OglBindFramebuffer(GL_FRAMEBUFFER, texture->m_framebuffer);
		OglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->m_texture, 0);
		unsigned int status = OglCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(status != GL_FRAMEBUFFER_COMPLETE)
		{
			releaseOpenGLES2Texture(texture);
			return NULL;
		}
		OglBindFramebuffer(GL_FRAMEBUFFER, window_framebuffer);
	}
	
	if(OglGetError() == 0)
		return texture;
	else
	{
		releaseOpenGLES2Texture(texture);
		return NULL;
	}
}

bool SurfaceOpenglES2::updateTextureData(OpenglES2Texture* texture, unsigned char* data)
{
	OglBindTexture(GL_TEXTURE_2D, texture->m_texture);
	OglPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	OglTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->m_width, texture->m_height, GL_RGBA, GL_UNSIGNED_BYTE, data);
	return true;
}

void SurfaceOpenglES2::releaseOpenGLES2Texture(OpenglES2Texture* texture)
{
	if(texture->m_framebuffer)
		OglDeleteFramebuffers(1, &texture->m_framebuffer);
	if(texture->m_texture)
		OglDeleteTextures(1, &texture->m_texture);
	delete texture;
}

void SurfaceOpenglES2::updateVertexBuffer(Sint32 attribute, Sint32 size, const void* vertexData, ptrdiff_t dataSizeInBytes)
{
	#ifdef GLES2_FORCE_VBOS
	if(!m_vertex_buffers[attribute])
		OglGenBuffers(1, &m_vertex_buffers[attribute]);

	OglBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffers[attribute]);
	if(m_vertex_buffer_size[attribute] < dataSizeInBytes)
	{
		OglBufferData(GL_ARRAY_BUFFER, dataSizeInBytes, vertexData, GL_STREAM_DRAW);
		m_vertex_buffer_size[attribute] = dataSizeInBytes;
	}
	else
		OglBufferSubData(GL_ARRAY_BUFFER, 0, dataSizeInBytes, vertexData);

	OglVertexAttribPointer(attribute, size, GL_FLOAT, GL_FALSE, 0, NULL);
	#else
	OglVertexAttribPointer(attribute, size, GL_FLOAT, GL_FALSE, 0, vertexData);
	#endif
}

void SurfaceOpenglES2::drawTriangles(std::vector<float>& vertices, std::vector<float>& texCoords)
{
	int verticesSize = SDL_static_cast(int, vertices.size()/2);
	if(verticesSize > 2046)
	{
		int verticeIndex = 0;
		while(verticeIndex < verticesSize)
		{
			int drawSize = UTIL_min<int>(verticesSize-verticeIndex, 2046);
			updateVertexBuffer(GLES_ATTRIBUTE_POSITION, 2, &vertices[verticeIndex*2], (drawSize*2)*sizeof(float));
			updateVertexBuffer(GLES_ATTRIBUTE_TEXCOORD, 2, &texCoords[verticeIndex*2], (drawSize*2)*sizeof(float));
			OglDrawArrays(GL_TRIANGLES, 0, drawSize);
			verticeIndex += drawSize;
		}
	}
	else if(verticesSize > 0)
	{
		updateVertexBuffer(GLES_ATTRIBUTE_POSITION, 2, &vertices[0], vertices.size()*sizeof(float));
		updateVertexBuffer(GLES_ATTRIBUTE_TEXCOORD, 2, &texCoords[0], texCoords.size()*sizeof(float));
		OglDrawArrays(GL_TRIANGLES, 0, verticesSize);
	}
}

void SurfaceOpenglES2::updateViewport()
{
	if(m_renderTarget)
		OglViewport(m_viewPortX, m_viewPortY, m_viewPortW, m_viewPortH);
	else
	{
		Sint32 w_w, w_h;
		SDL_GL_GetDrawableSize(g_engine.m_window, &w_w, &w_h);
		OglViewport(m_viewPortX, (w_h-m_viewPortY-m_viewPortH), m_viewPortW, m_viewPortH);
	}

	float projection[4][4];
	projection[0][0] = 2.0f/m_viewPortW;
	projection[0][1] = 0.0f;
	projection[0][2] = 0.0f;
	projection[0][3] = 0.0f;
	projection[1][0] = 0.0f;
	if(m_renderTarget)
		projection[1][1] = 2.0f/m_viewPortH;
	else
		projection[1][1] = -2.0f/m_viewPortH;
	projection[1][2] = 0.0f;
	projection[1][3] = 0.0f;
	projection[2][0] = 0.0f;
	projection[2][1] = 0.0f;
	projection[2][2] = 0.0f;
	projection[2][3] = 0.0f;
	projection[3][0] = -1.0f;
	if(m_renderTarget)
		projection[3][1] = -1.0f;
	else
		projection[3][1] = 1.0f;
	projection[3][2] = 0.0f;
	projection[3][3] = 1.0f;

	selectShader(m_programSolidColors);
	OglUniformMatrix4fv(m_programSolidColors.projectionLocation, 1, GL_FALSE, SDL_reinterpret_cast(float*, projection));
	selectShader(m_programSolidColors2);
	OglUniformMatrix4fv(m_programSolidColors2.projectionLocation, 1, GL_FALSE, SDL_reinterpret_cast(float*, projection));
	if(m_haveSharpening)
	{
		selectShader(m_programSharpen);
		OglUniformMatrix4fv(m_programSharpen.projectionLocation, 1, GL_FALSE, SDL_reinterpret_cast(float*, projection));
	}
	selectShader(m_programTexture);
	OglUniformMatrix4fv(m_programTexture.projectionLocation, 1, GL_FALSE, SDL_reinterpret_cast(float*, projection));
}

void SurfaceOpenglES2::updateRenderer()
{
	updateViewport();

	OglDisable(GL_SCISSOR_TEST);
	OglDisable(GL_DEPTH_TEST);
	OglDisable(GL_CULL_FACE);
	OglColorMask(1, 1, 1, 0);

	OglActiveTexture(GL_TEXTURE0);
	OglEnable(GL_BLEND);
	OglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	OglBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	OglBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

	OglEnableVertexAttribArray(GLES_ATTRIBUTE_POSITION);
	OglEnableVertexAttribArray(GLES_ATTRIBUTE_TEXCOORD);
}

bool SurfaceOpenglES2::isSupported()
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, OPENGL_CONTEXT_MAJOR);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_CONTEXT_MINOR);
	if(!g_engine.RecreateWindow(false, true))
	{
		UTIL_MessageBox(false, "OpenGL ES: Failed to create OpenGL window.");
		return false;
	}

	#define OGLLoadFunction(func) \
    do { \
		O##func = SDL_reinterpret_cast(PFN_O##func, SDL_GL_GetProcAddress(#func)); \
		if(!O##func) \
		{ \
			UTIL_MessageBox(false, "OpenGL ES: Failed to fetch '" #func "' function."); \
			return false; \
		} \
    } while(0)

	m_oglContext = SDL_GL_CreateContext(g_engine.m_window);
	if(!m_oglContext)
	{
		UTIL_MessageBox(false, "OpenGL ES: Failed to create device context.");
		return false;
	}
	if(SDL_GL_MakeCurrent(g_engine.m_window, m_oglContext) < 0)
	{
		UTIL_MessageBox(false, "OpenGL ES: Failed to create device context.");
		return false;
	}
	if(g_engine.isVsync())
		SDL_GL_SetSwapInterval(1);
	else
		SDL_GL_SetSwapInterval(0);
	OGLLoadFunction(glActiveTexture);
	OGLLoadFunction(glBindTexture);
	OGLLoadFunction(glGenTextures);
	OGLLoadFunction(glDeleteTextures);
	OGLLoadFunction(glTexImage2D);
	OGLLoadFunction(glTexSubImage2D);
	OGLLoadFunction(glTexParameteri);
	OGLLoadFunction(glTexEnvi);
	OGLLoadFunction(glPixelStorei);
	OGLLoadFunction(glCopyTexSubImage2D);
	OGLLoadFunction(glEnable);
	OGLLoadFunction(glDisable);
	OGLLoadFunction(glEnableVertexAttribArray);
	OGLLoadFunction(glDisableVertexAttribArray);
	OGLLoadFunction(glDrawArrays);
	OGLLoadFunction(glVertexAttribPointer);
	OGLLoadFunction(glBlendFuncSeparate);
	OGLLoadFunction(glBlendEquationSeparate);
	OGLLoadFunction(glColorMask);
	OGLLoadFunction(glReadPixels);
	OGLLoadFunction(glGetIntegerv);
	OGLLoadFunction(glGetBooleanv);
	OGLLoadFunction(glViewport);
	OGLLoadFunction(glScissor);
	OGLLoadFunction(glGetString);
	OGLLoadFunction(glGetError);
	OGLLoadFunction(glFinish);
	OGLLoadFunction(glGenFramebuffers);
	OGLLoadFunction(glDeleteFramebuffers);
	OGLLoadFunction(glFramebufferTexture2D);
	OGLLoadFunction(glBindFramebuffer);
	OGLLoadFunction(glCheckFramebufferStatus);

	OGLLoadFunction(glGenBuffers);
	OGLLoadFunction(glDeleteBuffers);
	OGLLoadFunction(glBindBuffer);
	OGLLoadFunction(glBufferData);
	OGLLoadFunction(glBufferSubData);

	OGLLoadFunction(glCreateShader);
	OGLLoadFunction(glDeleteShader);
	OGLLoadFunction(glCompileShader);
	OGLLoadFunction(glCreateProgram);
	OGLLoadFunction(glDeleteProgram);
	OGLLoadFunction(glUseProgram);
	OGLLoadFunction(glLinkProgram);
	OGLLoadFunction(glAttachShader);
	OGLLoadFunction(glShaderSource);
	OGLLoadFunction(glGetProgramiv);
	OGLLoadFunction(glGetShaderiv);
	OGLLoadFunction(glGetProgramInfoLog);
	OGLLoadFunction(glGetShaderInfoLog);
	OGLLoadFunction(glBindAttribLocation);
	OGLLoadFunction(glGetUniformLocation);
	OGLLoadFunction(glUniform1i);
	OGLLoadFunction(glUniform4f);
	OGLLoadFunction(glUniformMatrix4fv);
	#undef OGLLoadFunction

	int value = 0;
	OglGetIntegerv(GL_FRAMEBUFFER_BINDING, &value);
	window_framebuffer = SDL_static_cast(unsigned int, value);
	OglGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_maxTextureSize);
	if(m_maxTextureSize < 1024)
	{
		UTIL_MessageBox(false, "OpenGL ES: Your device doesn't meet minimum requirement for texture dimension.");
		return false;
	}
	if(!createOpenGLES2Shader(m_vertex_shader, GLES_VERTEX_SHADER, true) || !createOpenGLES2Shader(m_solidcolors_pixel_shader, GLES_COLOR_PIXEL_SHADER, false)
		|| !createOpenGLES2Shader(m_texture_pixel_shader, GLES_TEXTURE_PIXEL_SHADER, false) || !createOpenGLES2Shader(m_colortexture_pixel_shader, GLES_COLOR_TEXTURE_PIXEL_SHADER, false)
		|| !createOpenGLES2Shader(m_solidcolors2_pixel_shader, GLES_COLORS_PIXEL_SHADER, false))
	{
		UTIL_MessageBox(false, "OpenGL ES: Failed to create shaders.");
		return false;
	}
	else
	{
		if(!createOpenGLES2Program(m_programSolidColors, m_vertex_shader, m_solidcolors_pixel_shader) || !createOpenGLES2Program(m_programTexture, m_vertex_shader, m_texture_pixel_shader)
			|| !createOpenGLES2Program(m_programSolidColors2, m_vertex_shader, m_solidcolors2_pixel_shader))
		{
			UTIL_MessageBox(false, "OpenGL ES: Failed to create shaders.");
			return false;
		}

		if(createOpenGLES2Shader(m_sharpen_pixel_shader, GLES_SHARPEN_PIXEL_SHADER, false) && createOpenGLES2Program(m_programSharpen, m_vertex_shader, m_sharpen_pixel_shader))
			m_haveSharpening = true;
	}
	char* version = SDL_reinterpret_cast(char*, OglGetString(GL_VERSION));
	SDL_snprintf(g_buffer, sizeof(g_buffer), "OpenGL ES %c.%c", version[0], version[2]);
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

void SurfaceOpenglES2::init()
{
	m_pictures = SDL_reinterpret_cast(OpenglES2Texture**, SDL_calloc(g_pictureCounts, sizeof(OpenglES2Texture*)));
	if(!m_pictures)
	{
		SDL_OutOfMemory();
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Surface::Init() Failed: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
	}

	m_gameWindow = createOpenGLES2Texture(RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT, (g_engine.getAntialiasing() == CLIENT_ANTIALIASING_NORMAL), true);
	if(!m_gameWindow)
	{
		UTIL_MessageBox(true, "OpenGL ES: Out of video memory.");
		exit(-1);
		return;
	}
}

void SurfaceOpenglES2::doResize(Sint32 w, Sint32 h)
{
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = w;
	m_viewPortH = h;
	if(SDL_GL_GetCurrentContext() == m_oglContext)
		updateRenderer();
}

void SurfaceOpenglES2::spriteManagerReset()
{
	for(U32BGLES2Textures::iterator it = m_automapTiles.begin(), end = m_automapTiles.end(); it != end; ++it)
		releaseOpenGLES2Texture(it->second);

	for(U64BGLES2Textures::iterator it = m_spriteMasks.begin(), end = m_spriteMasks.end(); it != end; ++it)
		releaseOpenGLES2Texture(it->second);

	m_spriteMasks.clear();
	m_spritesMaskIds.clear();
	m_automapTilesBuff.clear();
	m_automapTiles.clear();
}

unsigned char* SurfaceOpenglES2::getScreenPixels(Sint32& width, Sint32& height, bool& bgra)
{
	if(SDL_GL_GetCurrentContext() != m_oglContext)
		return NULL;

	SDL_GL_GetDrawableSize(g_engine.m_window, &width, &height);
	while(OglGetError() != 0)
		continue;

	unsigned char* pixels = SDL_reinterpret_cast(unsigned char*, SDL_malloc(width*height * 4));
	if(!pixels)
		return NULL;

	OglPixelStorei(GL_PACK_ALIGNMENT, 1);
	OglFinish();
	OglReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	if(OglGetError() != 0)
	{
		SDL_free(pixels);
		return NULL;
	}

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
	bgra = false;
	return pixels;
}

void SurfaceOpenglES2::beginScene()
{
	if(SDL_GL_GetCurrentContext() != m_oglContext)
	{
		SDL_GL_MakeCurrent(g_engine.m_window, m_oglContext);
		updateRenderer();
	}
	selectShader(m_programTexture);
}

void SurfaceOpenglES2::endScene()
{
	SDL_GL_SwapWindow(g_engine.m_window);
}

bool SurfaceOpenglES2::integer_scaling(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	Sint32 width = RENDERTARGET_WIDTH;
	Sint32 height = RENDERTARGET_HEIGHT;
	UTIL_integer_scale(width, height, w, h, m_maxTextureSize, m_maxTextureSize);
	if(m_integer_scaling_width != width || m_integer_scaling_height != height || !m_scaled_gameWindow)
	{
		if(m_scaled_gameWindow)
		{
			releaseOpenGLES2Texture(m_scaled_gameWindow);
			m_scaled_gameWindow = NULL;
		}

		m_scaled_gameWindow = createOpenGLES2Texture(width, height, true, true);
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
	float maxu = (sx + sw)*m_gameWindow->m_scaleW;
	float minv = sy*m_gameWindow->m_scaleH;
	float maxv = (sy + sh)*m_gameWindow->m_scaleH;

	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	float texCoords[8];
	texCoords[0] = minu; texCoords[1] = minv;
	texCoords[2] = minu; texCoords[3] = maxv;
	texCoords[4] = maxu; texCoords[5] = minv;
	texCoords[6] = maxu; texCoords[7] = maxv;

	OglBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
	OglBindTexture(GL_TEXTURE_2D, m_gameWindow->m_texture);
	updateVertexBuffer(GLES_ATTRIBUTE_POSITION, 2, vertices, 8*sizeof(float));
	updateVertexBuffer(GLES_ATTRIBUTE_TEXCOORD, 2, texCoords, 8*sizeof(float));
	OglDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	OglBindFramebuffer(GL_FRAMEBUFFER, 0);
	m_renderTarget = NULL;
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = g_engine.getWindowWidth();
	m_viewPortH = g_engine.getWindowHeight();
	updateViewport();

	minx = SDL_static_cast(float, x);
	maxx = SDL_static_cast(float, x + w);
	miny = SDL_static_cast(float, y);
	maxy = SDL_static_cast(float, y + h);

	minu = 0.0f;
	maxu = m_scaled_gameWindow->m_width*m_scaled_gameWindow->m_scaleW;
	minv = 0.0f;
	maxv = m_scaled_gameWindow->m_height*m_scaled_gameWindow->m_scaleH;

	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	texCoords[0] = minu; texCoords[1] = minv;
	texCoords[2] = minu; texCoords[3] = maxv;
	texCoords[4] = maxu; texCoords[5] = minv;
	texCoords[6] = maxu; texCoords[7] = maxv;
	
	if(g_engine.isSharpening() && m_haveSharpening)
	{
		selectShader(m_programSharpen);
		OglUniform4f(m_programSharpen.modulationLocation, 1.0f / w, 1.0f / h, 1.0f, 1.0f);
	}
	OglBindTexture(GL_TEXTURE_2D, m_scaled_gameWindow->m_texture);
	updateVertexBuffer(GLES_ATTRIBUTE_POSITION, 2, vertices, 8*sizeof(float));
	updateVertexBuffer(GLES_ATTRIBUTE_TEXCOORD, 2, texCoords, 8*sizeof(float));
	OglDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	OglBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	if(g_engine.isSharpening() && m_haveSharpening)
		selectShader(m_programTexture);
	return true;
}

void SurfaceOpenglES2::drawGameScene(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
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

	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	float texCoords[8];
	texCoords[0] = minu; texCoords[1] = minv;
	texCoords[2] = minu; texCoords[3] = maxv;
	texCoords[4] = maxu; texCoords[5] = minv;
	texCoords[6] = maxu; texCoords[7] = maxv;
	
	if(g_engine.isSharpening() && m_haveSharpening)
	{
		selectShader(m_programSharpen);
		OglUniform4f(m_programSharpen.modulationLocation, 1.0f / w, 1.0f / h, 1.0f, 1.0f);
	}
	OglBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
	OglBindTexture(GL_TEXTURE_2D, m_gameWindow->m_texture);
	updateVertexBuffer(GLES_ATTRIBUTE_POSITION, 2, vertices, 8*sizeof(float));
	updateVertexBuffer(GLES_ATTRIBUTE_TEXCOORD, 2, texCoords, 8*sizeof(float));
	OglDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	OglBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	if(g_engine.isSharpening() && m_haveSharpening)
		selectShader(m_programTexture);
}

void SurfaceOpenglES2::beginGameScene()
{
	OglBindFramebuffer(GL_FRAMEBUFFER, m_gameWindow->m_framebuffer);
	m_renderTarget = m_gameWindow;
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = m_gameWindow->m_width;
	m_viewPortH = m_gameWindow->m_height;
	updateViewport();
	fillRectangle(0, 0, RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT, 0x00, 0x00, 0x00, 255);
}

void SurfaceOpenglES2::endGameScene()
{
	OglBindFramebuffer(GL_FRAMEBUFFER, window_framebuffer);
	m_renderTarget = NULL;
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = g_engine.getWindowWidth();
	m_viewPortH = g_engine.getWindowHeight();
	updateViewport();
}

void SurfaceOpenglES2::drawLightMap(LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height)
{
	float *vertices, *colores;
	vertices = SDL_reinterpret_cast(float*, SDL_malloc(sizeof(float)*(width+1)*4));
	colores = SDL_reinterpret_cast(float*, SDL_malloc(sizeof(float)*(width+1)*6));

	selectShader(m_programSolidColors2);
	OglBlendFuncSeparate(GL_ZERO, GL_SRC_COLOR, GL_ZERO, GL_ONE);
	OglEnableVertexAttribArray(GLES_ATTRIBUTE_COLORS);
	OglDisableVertexAttribArray(GLES_ATTRIBUTE_TEXCOORD);

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
			Uint32 offset3 = verticeCount * 2;
			Uint32 offset4 = verticeCount * 3;
			vertices[offset3] = SDL_static_cast(float, drawX);
			vertices[offset3+1] = SDL_static_cast(float, drawY+scale);
			colores[offset4] = lightmap[offset1+offset].r*inv255f;
			colores[offset4+1] = lightmap[offset1+offset].g*inv255f;
			colores[offset4+2] = lightmap[offset1+offset].b*inv255f;
			++verticeCount;
			
			offset3 += 2;
			offset4 += 3;
			vertices[offset3] = SDL_static_cast(float, drawX);
			vertices[offset3+1] = SDL_static_cast(float, drawY);
			colores[offset4] = lightmap[offset2+offset].r*inv255f;
			colores[offset4+1] = lightmap[offset2+offset].g*inv255f;
			colores[offset4+2] = lightmap[offset2+offset].b*inv255f;
			++verticeCount;
			drawX += scale;
		}

		drawY += scale;
		updateVertexBuffer(GLES_ATTRIBUTE_POSITION, 2, vertices, (verticeCount*2)*sizeof(float));
		updateVertexBuffer(GLES_ATTRIBUTE_COLORS, 3, colores, (verticeCount*3)*sizeof(float));
		OglDrawArrays(GL_TRIANGLE_STRIP, 0, verticeCount);
	}

	OglEnableVertexAttribArray(GLES_ATTRIBUTE_TEXCOORD);
	OglDisableVertexAttribArray(GLES_ATTRIBUTE_COLORS);
	OglBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	selectShader(m_programTexture);

	SDL_free(colores);
	SDL_free(vertices);
}

void SurfaceOpenglES2::setClipRect(Sint32 x, Sint32 y, Sint32 w, Sint32 h)
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

void SurfaceOpenglES2::disableClipRect()
{
	OglDisable(GL_SCISSOR_TEST);
}

void SurfaceOpenglES2::drawRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	float minx = SDL_static_cast(float, x)+0.5f;
	float maxx = SDL_static_cast(float, x+w)-0.5f;
	float miny = SDL_static_cast(float, y)+0.5f;
	float maxy = SDL_static_cast(float, y+h)-0.5f;

	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = maxx; vertices[3] = miny;
	vertices[4] = maxx; vertices[5] = maxy;
	vertices[6] = minx; vertices[7] = maxy;

	selectShader(m_programSolidColors);
	OglDisableVertexAttribArray(GLES_ATTRIBUTE_TEXCOORD);
	OglUniform4f(m_programSolidColors.modulationLocation, r*inv255f, g*inv255f, b*inv255f, a*inv255f);
	updateVertexBuffer(GLES_ATTRIBUTE_POSITION, 2, vertices, 8*sizeof(float));
	OglDrawArrays(GL_LINE_LOOP, 0, 4);
	OglEnableVertexAttribArray(GLES_ATTRIBUTE_TEXCOORD);
	selectShader(m_programTexture);
}

void SurfaceOpenglES2::fillRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	selectShader(m_programSolidColors);
	OglDisableVertexAttribArray(GLES_ATTRIBUTE_TEXCOORD);
	OglUniform4f(m_programSolidColors.modulationLocation, r*inv255f, g*inv255f, b*inv255f, a*inv255f);
	updateVertexBuffer(GLES_ATTRIBUTE_POSITION, 2, vertices, 8*sizeof(float));
	OglDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	OglEnableVertexAttribArray(GLES_ATTRIBUTE_TEXCOORD);
	selectShader(m_programTexture);
}

OpenglES2Texture* SurfaceOpenglES2::loadPicture(Uint16 pictureId, bool linear)
{
	Sint32 width, height;
	unsigned char* pixels = g_engine.LoadPicture(pictureId, false, width, height);
	if(!pixels)
		return NULL;

	OpenglES2Texture* s = createOpenGLES2Texture(width, height, linear);
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

void SurfaceOpenglES2::drawFont(Uint16 pictureId, Sint32 x, Sint32 y, const std::string& text, size_t pos, size_t len, Uint8 r, Uint8 g, Uint8 b, Sint32 cX[256], Sint32 cY[256], Sint32 cW[256], Sint32 cH[256])
{
	OpenglES2Texture* tex = m_pictures[pictureId];
	if(!tex)
	{
		tex = loadPicture(pictureId, false);
		if(!tex)
			return;//load failed
	}

	std::vector<float> vertices;
	std::vector<float> texCoords;
	vertices.reserve(3072);
	texCoords.reserve(3072);

	OglBindTexture(GL_TEXTURE_2D, tex->m_texture);
	OglUniform4f(m_programTexture.modulationLocation, r*inv255f, g*inv255f, b*inv255f, 1.0f);

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
					drawTriangles(vertices, texCoords);
					OglUniform4f(m_programTexture.modulationLocation, red*inv255f, green*inv255f, blue*inv255f, 1.0f);
					vertices.clear();
					texCoords.clear();
					i += 3;
				}
			}
			break;
			case 0x0F://Special case - change back standard color
			{
				drawTriangles(vertices, texCoords);
				OglUniform4f(m_programTexture.modulationLocation, r*inv255f, g*inv255f, b*inv255f, 1.0f);
				vertices.clear();
				texCoords.clear();
			}
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

				vertices.push_back(minx); vertices.push_back(miny);
				vertices.push_back(minx); vertices.push_back(maxy);
				vertices.push_back(maxx); vertices.push_back(miny);
				vertices.push_back(maxx); vertices.push_back(maxy);
				vertices.push_back(maxx); vertices.push_back(miny);
				vertices.push_back(minx); vertices.push_back(maxy);

				texCoords.push_back(minu); texCoords.push_back(minv);
				texCoords.push_back(minu); texCoords.push_back(maxv);
				texCoords.push_back(maxu); texCoords.push_back(minv);
				texCoords.push_back(maxu); texCoords.push_back(maxv);
				texCoords.push_back(maxu); texCoords.push_back(minv);
				texCoords.push_back(minu); texCoords.push_back(maxv);
				rx += cW[character] + cX[0];
			}
			break;
		}
	}
	drawTriangles(vertices, texCoords);
	OglUniform4f(m_programTexture.modulationLocation, 1.0f, 1.0f, 1.0f, 1.0f);
}

void SurfaceOpenglES2::drawBackground(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	OpenglES2Texture* tex = m_pictures[pictureId];
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

	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	float texCoords[8];
	texCoords[0] = minu; texCoords[1] = minv;
	texCoords[2] = minu; texCoords[3] = maxv;
	texCoords[4] = maxu; texCoords[5] = minv;
	texCoords[6] = maxu; texCoords[7] = maxv;

	OglBindTexture(GL_TEXTURE_2D, tex->m_texture);
	updateVertexBuffer(GLES_ATTRIBUTE_POSITION, 2, vertices, 8*sizeof(float));
	updateVertexBuffer(GLES_ATTRIBUTE_TEXCOORD, 2, texCoords, 8*sizeof(float));
	OglDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void SurfaceOpenglES2::drawPictureRepeat(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	OpenglES2Texture* tex = m_pictures[pictureId];
	if(!tex)
	{
		tex = loadPicture(pictureId, false);
		if(!tex)
			return;//load failed
	}

	std::vector<float> vertices;
	std::vector<float> texCoords;
	vertices.reserve(3072);
	texCoords.reserve(3072);

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

			vertices.push_back(minx); vertices.push_back(miny);
			vertices.push_back(minx); vertices.push_back(maxy);
			vertices.push_back(maxx); vertices.push_back(miny);
			vertices.push_back(maxx); vertices.push_back(maxy);
			vertices.push_back(maxx); vertices.push_back(miny);
			vertices.push_back(minx); vertices.push_back(maxy);

			texCoords.push_back(minu); texCoords.push_back(minv);
			texCoords.push_back(minu); texCoords.push_back(maxv);
			texCoords.push_back(maxu); texCoords.push_back(minv);
			texCoords.push_back(maxu); texCoords.push_back(maxv);
			texCoords.push_back(maxu); texCoords.push_back(minv);
			texCoords.push_back(minu); texCoords.push_back(maxv);
			cx += sw;
		}
		y += sh;
	}
	OglBindTexture(GL_TEXTURE_2D, tex->m_texture);
	drawTriangles(vertices, texCoords);
}

void SurfaceOpenglES2::drawPicture(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	OpenglES2Texture* tex = m_pictures[pictureId];
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

	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	float texCoords[8];
	texCoords[0] = minu; texCoords[1] = minv;
	texCoords[2] = minu; texCoords[3] = maxv;
	texCoords[4] = maxu; texCoords[5] = minv;
	texCoords[6] = maxu; texCoords[7] = maxv;

	OglBindTexture(GL_TEXTURE_2D, tex->m_texture);
	updateVertexBuffer(GLES_ATTRIBUTE_POSITION, 2, vertices, 8*sizeof(float));
	updateVertexBuffer(GLES_ATTRIBUTE_TEXCOORD, 2, texCoords, 8*sizeof(float));
	OglDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

OpenglES2Texture* SurfaceOpenglES2::loadSpriteMask(Uint64 tempPos, Uint32 spriteId, Uint32 maskSpriteId, Uint32 outfitColor)
{
	unsigned char* pixels = g_engine.LoadSpriteMask(spriteId, maskSpriteId, outfitColor, false);
	if(!pixels)
		return NULL;

	if(m_spriteMasks.size() >= HARDWARE_MAX_SPRITEMASKS)
	{
		U64BGLES2Textures::iterator it = m_spriteMasks.find(m_spritesMaskIds.front());
		if(it == m_spriteMasks.end())//Something weird happen - let's erase the first one entry
			it = m_spriteMasks.begin();

		releaseOpenGLES2Texture(it->second);
		m_spriteMasks.erase(it);
		m_spritesMaskIds.pop_front();
	}
	OpenglES2Texture* s = createOpenGLES2Texture(32, 32, false);
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

void SurfaceOpenglES2::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor)
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
	OpenglES2Texture* tex;
	U64BGLES2Textures::iterator it = m_spriteMasks.find(tempPos);
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

	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	float texCoords[8];
	texCoords[0] = minu; texCoords[1] = minv;
	texCoords[2] = minu; texCoords[3] = maxv;
	texCoords[4] = maxu; texCoords[5] = minv;
	texCoords[6] = maxu; texCoords[7] = maxv;

	OglBindTexture(GL_TEXTURE_2D, tex->m_texture);
	updateVertexBuffer(GLES_ATTRIBUTE_POSITION, 2, vertices, 8*sizeof(float));
	updateVertexBuffer(GLES_ATTRIBUTE_TEXCOORD, 2, texCoords, 8*sizeof(float));
	OglDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void SurfaceOpenglES2::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Uint32 outfitColor)
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
	OpenglES2Texture* tex;
	U64BGLES2Textures::iterator it = m_spriteMasks.find(tempPos);
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

	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	float texCoords[8];
	texCoords[0] = minu; texCoords[1] = minv;
	texCoords[2] = minu; texCoords[3] = maxv;
	texCoords[4] = maxu; texCoords[5] = minv;
	texCoords[6] = maxu; texCoords[7] = maxv;

	OglBindTexture(GL_TEXTURE_2D, tex->m_texture);
	updateVertexBuffer(GLES_ATTRIBUTE_POSITION, 2, vertices, 8*sizeof(float));
	updateVertexBuffer(GLES_ATTRIBUTE_TEXCOORD, 2, texCoords, 8*sizeof(float));
	OglDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

OpenglES2Texture* SurfaceOpenglES2::createAutomapTile(Uint32 currentArea)
{
	if(m_automapTiles.size() >= HARDWARE_MAX_AUTOMAPTILES)
	{
		U32BGLES2Textures::iterator it = m_automapTiles.find(m_automapTilesBuff.front());
		if(it == m_automapTiles.end())//Something weird happen - let's erase the first one entry
			it = m_automapTiles.begin();

		releaseOpenGLES2Texture(it->second);
		m_automapTiles.erase(it);
		m_automapTilesBuff.pop_front();
	}
	OpenglES2Texture* s = createOpenGLES2Texture(256, 256, false);
	if(!s)
		return NULL;

	m_automapTiles[currentArea] = s;
	m_automapTilesBuff.push_back(currentArea);
	return s;
}

void SurfaceOpenglES2::uploadAutomapTile(OpenglES2Texture* texture, Uint8 color[256][256])
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
			*ptrpixels++ = r;
			*ptrpixels++ = g;
			*ptrpixels++ = b;
			*ptrpixels++ = 255;
		}
	}
	updateTextureData(texture, pixels);
	SDL_free(pixels);
}

void SurfaceOpenglES2::drawAutomapTile(Uint32 currentArea, bool& recreate, Uint8 color[256][256], Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
{
	OpenglES2Texture* tex;
	U32BGLES2Textures::iterator it = m_automapTiles.find(currentArea);
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

	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	float texCoords[8];
	texCoords[0] = minu; texCoords[1] = minv;
	texCoords[2] = minu; texCoords[3] = maxv;
	texCoords[4] = maxu; texCoords[5] = minv;
	texCoords[6] = maxu; texCoords[7] = maxv;

	OglBindTexture(GL_TEXTURE_2D, tex->m_texture);
	updateVertexBuffer(GLES_ATTRIBUTE_POSITION, 2, vertices, 8*sizeof(float));
	updateVertexBuffer(GLES_ATTRIBUTE_TEXCOORD, 2, texCoords, 8*sizeof(float));
	OglDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

SurfaceOpenglES2Comp::SurfaceOpenglES2Comp()
{
	m_sprites = NULL;
}

SurfaceOpenglES2Comp::~SurfaceOpenglES2Comp()
{
	if(m_sprites)
	{
		for(Uint32 i = 1; i <= m_spritesCache; ++i)
		{
			if(m_sprites[i])
				releaseOpenGLES2Texture(m_sprites[i]);
		}

		SDL_free(m_sprites);
	}
}

void SurfaceOpenglES2Comp::init()
{
	SurfaceOpenglES2::init();
	m_sprites = SDL_reinterpret_cast(OpenglES2Texture**, SDL_calloc(g_spriteCounts+1, sizeof(OpenglES2Texture*)));
	if(!m_sprites)
	{
		SDL_OutOfMemory();
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Surface::Init() Failed: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
	}
	m_spritesCache = g_spriteCounts;
}

void SurfaceOpenglES2Comp::spriteManagerReset()
{
	SurfaceOpenglES2::spriteManagerReset();
	if(m_sprites)
	{
		for(Uint32 i = 1; i <= m_spritesCache; ++i)
		{
			if(m_sprites[i])
				releaseOpenGLES2Texture(m_sprites[i]);
		}

		SDL_free(m_sprites);
	}
	m_sprites = SDL_reinterpret_cast(OpenglES2Texture**, SDL_calloc(g_spriteCounts+1, sizeof(OpenglES2Texture*)));
	if(!m_sprites)
	{
		SDL_OutOfMemory();
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Surface::spriteManagerReset() Failed: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
	}
	m_spritesCache = g_spriteCounts;
}

OpenglES2Texture* SurfaceOpenglES2Comp::loadSprite(Uint32 spriteId)
{
	unsigned char* pixels = g_engine.LoadSprite(spriteId, false);
	if(!pixels)
		return NULL;

	OpenglES2Texture* s = createOpenGLES2Texture(32, 32, false);
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

void SurfaceOpenglES2Comp::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y)
{
	if(spriteId > g_spriteCounts)
		return;

	OpenglES2Texture* tex = m_sprites[spriteId];
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

	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	float texCoords[8];
	texCoords[0] = minu; texCoords[1] = minv;
	texCoords[2] = minu; texCoords[3] = maxv;
	texCoords[4] = maxu; texCoords[5] = minv;
	texCoords[6] = maxu; texCoords[7] = maxv;

	OglBindTexture(GL_TEXTURE_2D, tex->m_texture);
	updateVertexBuffer(GLES_ATTRIBUTE_POSITION, 2, vertices, 8*sizeof(float));
	updateVertexBuffer(GLES_ATTRIBUTE_TEXCOORD, 2, texCoords, 8*sizeof(float));
	OglDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void SurfaceOpenglES2Comp::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
{
	if(spriteId > g_spriteCounts)
		return;

	OpenglES2Texture* tex = m_sprites[spriteId];
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
	
	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	float texCoords[8];
	texCoords[0] = minu; texCoords[1] = minv;
	texCoords[2] = minu; texCoords[3] = maxv;
	texCoords[4] = maxu; texCoords[5] = minv;
	texCoords[6] = maxu; texCoords[7] = maxv;

	OglBindTexture(GL_TEXTURE_2D, tex->m_texture);
	updateVertexBuffer(GLES_ATTRIBUTE_POSITION, 2, vertices, 8*sizeof(float));
	updateVertexBuffer(GLES_ATTRIBUTE_TEXCOORD, 2, texCoords, 8*sizeof(float));
	OglDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

SurfaceOpenglES2Perf::SurfaceOpenglES2Perf()
{
	m_sprites = NULL;
	m_spriteAtlas = NULL;

	m_spriteAtlases = 0;
	m_spritesPerAtlas = 0;
	m_spritesPerModulo = 0;

	m_scheduleSpriteDraw = false;
}

SurfaceOpenglES2Perf::~SurfaceOpenglES2Perf()
{
	if(m_sprites)
		SDL_free(m_sprites);

	for(std::vector<OpenglES2Texture*>::iterator it = m_spritesAtlas.begin(), end = m_spritesAtlas.end(); it != end; ++it)
		releaseOpenGLES2Texture((*it));

	m_spritesAtlas.clear();
}

void SurfaceOpenglES2Perf::generateSpriteAtlases()
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
	for(std::vector<OpenglES2Texture*>::iterator it = m_spritesAtlas.begin(), end = m_spritesAtlas.end(); it != end; ++it)
		releaseOpenGLES2Texture((*it));

	m_spritesAtlas.clear();
	for(Uint32 i = 0; i < m_spriteAtlases; ++i)
	{
		OpenglES2Texture* texture = createOpenGLES2Texture(m_spritesPerModulo, m_spritesPerModulo, false);
		if(!texture)
		{
			UTIL_MessageBox(true, "OpenGL ES: Out of video memory.");
			exit(-1);
			return;
		}
		m_spritesAtlas.push_back(texture);
	}
}

void SurfaceOpenglES2Perf::checkScheduledSprites()
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

void SurfaceOpenglES2Perf::init()
{
	SurfaceOpenglES2::init();
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

void SurfaceOpenglES2Perf::spriteManagerReset()
{
	SurfaceOpenglES2::spriteManagerReset();
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

void SurfaceOpenglES2Perf::beginScene()
{
	SurfaceOpenglES2::beginScene();
	m_scheduleSpriteDraw = false;
}

void SurfaceOpenglES2Perf::beginGameScene()
{
	SurfaceOpenglES2::beginGameScene();
	m_gameWindowVertices.clear();
	m_gameWindowTexCoords.clear();
	m_gameWindowVertices.reserve(61440);
	m_gameWindowTexCoords.reserve(61440);
	m_scheduleSpriteDraw = true;
}

void SurfaceOpenglES2Perf::endGameScene()
{
	checkScheduledSprites();
	m_scheduleSpriteDraw = false;
	SurfaceOpenglES2::endGameScene();
}

void SurfaceOpenglES2Perf::drawRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(m_scheduleSpriteDraw)
		checkScheduledSprites();
	SurfaceOpenglES2::drawRectangle(x, y, w, h, r, g, b, a);
}

void SurfaceOpenglES2Perf::fillRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(m_scheduleSpriteDraw)
		checkScheduledSprites();
	SurfaceOpenglES2::fillRectangle(x, y, w, h, r, g, b, a);
}

bool SurfaceOpenglES2Perf::loadSprite(Uint32 spriteId, OpenglES2Texture* texture, Uint32 xoff, Uint32 yoff)
{
	unsigned char* pixels = g_engine.LoadSprite(spriteId, false);
	if(!pixels)
		return false;

	OglBindTexture(GL_TEXTURE_2D, texture->m_texture);
	OglPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	OglTexSubImage2D(GL_TEXTURE_2D, 0, SDL_static_cast(int, xoff), SDL_static_cast(int, yoff), 32, 32, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	SDL_free(pixels);
	return true;
}

void SurfaceOpenglES2Perf::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y)
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

	OpenglES2Texture* tex = m_spritesAtlas[spriteAtlas];
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
		float vertices[8];
		vertices[0] = minx; vertices[1] = miny;
		vertices[2] = minx; vertices[3] = maxy;
		vertices[4] = maxx; vertices[5] = miny;
		vertices[6] = maxx; vertices[7] = maxy;

		float texCoords[8];
		texCoords[0] = minu; texCoords[1] = minv;
		texCoords[2] = minu; texCoords[3] = maxv;
		texCoords[4] = maxu; texCoords[5] = minv;
		texCoords[6] = maxu; texCoords[7] = maxv;

		OglBindTexture(GL_TEXTURE_2D, tex->m_texture);
		updateVertexBuffer(GLES_ATTRIBUTE_POSITION, 2, vertices, 8*sizeof(float));
		updateVertexBuffer(GLES_ATTRIBUTE_TEXCOORD, 2, texCoords, 8*sizeof(float));
		OglDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
}

void SurfaceOpenglES2Perf::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
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

	OpenglES2Texture* tex = m_spritesAtlas[spriteAtlas];
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

	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	float texCoords[8];
	texCoords[0] = minu; texCoords[1] = minv;
	texCoords[2] = minu; texCoords[3] = maxv;
	texCoords[4] = maxu; texCoords[5] = minv;
	texCoords[6] = maxu; texCoords[7] = maxv;

	OglBindTexture(GL_TEXTURE_2D, tex->m_texture);
	updateVertexBuffer(GLES_ATTRIBUTE_POSITION, 2, vertices, 8*sizeof(float));
	updateVertexBuffer(GLES_ATTRIBUTE_TEXCOORD, 2, texCoords, 8*sizeof(float));
	OglDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void SurfaceOpenglES2Perf::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor)
{
	if(m_scheduleSpriteDraw)
		checkScheduledSprites();
	SurfaceOpenglES2::drawSpriteMask(spriteId, maskSpriteId, x, y, outfitColor);
}
#endif
