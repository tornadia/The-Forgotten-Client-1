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

#ifndef __FILE_SURFACE_VULKAN_h_
#define __FILE_SURFACE_VULKAN_h_

#include "engine.h"

#if defined(SDL_VIDEO_VULKAN)
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

enum GraphicsPipelines
{
	GRAPHIC_PIPELINE_RGBA = 0,
	GRAPHIC_PIPELINE_RGBA_LINES = 1,
	GRAPHIC_PIPELINE_LIGHT_MAP = 2,
	GRAPHIC_PIPELINE_TEXTURE = 3,
	GRAPHIC_PIPELINE_TEXTURE_NON_BLEND = 4,
	GRAPHIC_PIPELINE_TEXTURE_TRIANGLES = 5,
	GRAPHIC_PIPELINE_SHARPEN = 6,
	GRAPHIC_PIPELINE_LAST = 7
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct VulkanTexture
{
	VulkanTexture() : m_textureImage(NULL), m_textureView(NULL), m_textureImageMemory(NULL), m_framebuffer(NULL), m_descriptorPool(NULL), m_descriptorSet(NULL) {}

	VkImage m_textureImage;
	VkImageView m_textureView;
	VkDeviceMemory m_textureImageMemory;
	VkFramebuffer m_framebuffer;
	VkDescriptorPool m_descriptorPool;
	VkDescriptorSet m_descriptorSet;
	Uint32 m_width;
	Uint32 m_height;
	float m_scaleW;
	float m_scaleH;
};

struct VertexVulkan
{
	VertexVulkan(float x, float y, float u, float v, DWORD color) : x(x), y(y), u(u), v(v), color(color) {}

	float x, y;
	float u, v;
	DWORD color;
};

struct PushConstant_Projection
{
	float vkWidth;
	float vkHeight;
};

struct PushConstant_Sharpen
{
	float vkWidth;
	float vkHeight;
	float pxWidth;
	float pxHeight;
};

#ifdef HAVE_CXX11_SUPPORT
typedef std::unordered_map<Uint32, VulkanTexture*> U32BImages;
typedef std::unordered_map<Uint64, VulkanTexture*> U64BImages;
#else
typedef std::map<Uint32, VulkanTexture*> U32BImages;
typedef std::map<Uint64, VulkanTexture*> U64BImages;
#endif

class SurfaceVulkan : public Surface
{
	public:
		SurfaceVulkan();
		virtual ~SurfaceVulkan();

		Uint32 rateDeviceSuitability(VkPhysicalDevice device);
		SwapChainSupportDetails querySwapChainSupport();
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		Uint32 findMemoryType(Uint32 typeFilter, VkMemoryPropertyFlags properties);
		VkShaderModule createShaderModule(const char* code, size_t codesize);

		void setupTextureRendering(VulkanTexture* texture);
		VulkanTexture* createVulkanTexture(Uint32 width, Uint32 height, bool linearSampler, bool frameBuffer = false);
		bool createTextureFramebuffer(VulkanTexture* texture);
		bool updateTextureData(VulkanTexture* texture, unsigned char* data);
		void releaseVulkanTexture(VulkanTexture* texture);

		void setupGraphicPipeline(Sint32 pipelineId);
		bool CreateGraphicPipeline(Sint32 pipelineId);
		void cleanupSwapChainResources();
		void reinitSwapChainResources();

		void setupProjection();
		bool updateVertexBuffer(const void* vertexData, Uint32 dataSize);

		virtual bool isSupported();
		virtual const char* getName() {return "Vulkan";}
		virtual const char* getSoftware() {return m_software;}
		virtual const char* getHardware() {return m_hardware;}
		virtual Uint32 getVRAM() {return m_totalVRAM;}

		virtual void renderTargetsRecreate();
		virtual void init();
		virtual void doResize(Sint32 w, Sint32 h);
		virtual void spriteManagerReset();
		virtual unsigned char* getScreenPixels(Sint32& width, Sint32& height, bool& bgra);

		virtual void beginScene();
		virtual void endScene();

		bool integer_scaling(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void drawLightMap(LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height);
		virtual void drawGameScene(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void beginGameScene();
		virtual void endGameScene();

		virtual void setClipRect(Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void disableClipRect();
		virtual void drawRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
		virtual void fillRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

		VulkanTexture* loadPicture(Uint16 pictureId, bool linearSampler);
		virtual void drawFont(Uint16 pictureId, Sint32 x, Sint32 y, const std::string& text, size_t pos, size_t len, Uint8 r, Uint8 g, Uint8 b, Sint32 cX[256], Sint32 cY[256], Sint32 cW[256], Sint32 cH[256]);
		virtual void drawBackground(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void drawPictureRepeat(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void drawPicture(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 x, Sint32 y, Sint32 w, Sint32 h);

		VulkanTexture* loadSpriteMask(Uint64 tempPos, Uint32 spriteId, Uint32 maskSpriteId, Uint32 outfitColor);
		virtual void drawSprite(Uint32, Sint32, Sint32) {;}
		virtual void drawSprite(Uint32, Sint32, Sint32, Sint32, Sint32, Sint32, Sint32, Sint32, Sint32) {;}
		virtual void drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor);
		virtual void drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Uint32 outfitColor);

		VulkanTexture* createAutomapTile(Uint32 currentArea);
		void uploadAutomapTile(VulkanTexture* texture, Uint8 color[256][256]);
		virtual void drawAutomapTile(Uint32 m_currentArea, bool& m_recreate, Uint8 m_color[256][256], Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh);

	protected:
		U32BImages m_automapTiles;
		U64BImages m_spriteMasks;
		std::circular_buffer<Uint32> m_automapTilesBuff;
		std::circular_buffer<Uint64> m_spritesMaskIds;

		VulkanTexture** m_pictures;
		VulkanTexture* m_gameWindow;
		VulkanTexture* m_scaled_gameWindow;

		char* m_software;
		char* m_hardware;

		VkInstance m_instance;
		VkSurfaceKHR m_surface;
		VkPhysicalDevice m_gpu;
		VkDevice m_device;
		VkQueue m_queue;
		VkQueue m_presentQueue;
		VkDescriptorSetLayout m_descriptorSetLayout;

		VkRenderPass m_storeRenderPass;
		VkRenderPass m_clearRenderPass;
		VkRenderPass m_textureStoreRenderPass;
		VkRenderPass m_textureClearRenderPass;
		VkSampler m_samplers[2];
		VkPipelineLayout m_pipelineLayout[GRAPHIC_PIPELINE_LAST];
		VkPipeline m_graphicsPipeline[GRAPHIC_PIPELINE_LAST];
		VkSwapchainKHR m_swapChain;
		VkFormat m_swapChainImageFormat;
		VkExtent2D m_swapChainExtent;
		std::vector<VkImage> m_swapChainImages;
		std::vector<VkImageView> m_swapChainImageViews;
		std::vector<VkFramebuffer> m_swapChainFrameBuffer;
		std::vector<VulkanTexture*> m_texturesToDelete[VULKAN_INFLIGHT_FRAMES];

		VkBuffer m_staggingBuffer[VULKAN_INFLIGHT_FRAMES];
		VkDeviceMemory m_staggingBufferMemory[VULKAN_INFLIGHT_FRAMES];
		VkBuffer m_vertexBuffer[VULKAN_INFLIGHT_FRAMES];
		VkDeviceMemory m_vertexBufferMemory[VULKAN_INFLIGHT_FRAMES];
		Uint32 m_vertexBufferSize[VULKAN_INFLIGHT_FRAMES];

		std::vector<VertexVulkan> m_vulkanVertices;
		Uint32 m_vertexOffset;
		Uint32 m_lightVertices;

		VkSemaphore m_presentCompleteSemaphores[VULKAN_INFLIGHT_FRAMES];
		VkSemaphore m_renderCompleteSemaphores[VULKAN_INFLIGHT_FRAMES];
		VkFence m_renderFences[VULKAN_INFLIGHT_FRAMES];
		VkCommandPool m_commandPool;
		VkCommandBuffer m_commandBuffer;
		VkCommandBuffer m_commandDataBuffer;
		VkCommandBuffer m_commandVertexBuffers[VULKAN_INFLIGHT_FRAMES];
		VkCommandBuffer m_commandBuffers[VULKAN_INFLIGHT_FRAMES];

		Uint32 m_totalVRAM;
		Uint32 m_spritesCache;

		Uint32 m_graphicsQueuePresentIndex;
		Uint32 m_graphicsQueueNodeIndex;
		Uint32 m_imageCount;
		Uint32 m_imageIndex;
		Uint32 m_currentFrame;

		float m_vkWidth;
		float m_vkHeight;

		Sint32 m_scissorWidth;
		Sint32 m_scissorHeight;

		Sint32 m_currentGraphicPipeline;

		Sint32 m_maxTextureSize;
		Sint32 m_integer_scaling_width;
		Sint32 m_integer_scaling_height;

		PFN_vkCreateInstance vkCreateInstance;
		PFN_vkDestroyInstance vkDestroyInstance;
		PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
		PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
		PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
		PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
		PFN_vkCreateDevice vkCreateDevice;
		PFN_vkDestroyDevice vkDestroyDevice;
		PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
		PFN_vkGetDeviceQueue vkGetDeviceQueue;
		PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;

		PFN_vkCreateSemaphore vkCreateSemaphore;
		PFN_vkDestroySemaphore vkDestroySemaphore;
		PFN_vkCreateFence vkCreateFence;
		PFN_vkWaitForFences vkWaitForFences;
		PFN_vkResetFences vkResetFences;
		PFN_vkDestroyFence vkDestroyFence;
		PFN_vkResetCommandBuffer vkResetCommandBuffer;
		PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
		PFN_vkCreateCommandPool vkCreateCommandPool;
		PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
		PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
		PFN_vkCmdSetViewport vkCmdSetViewport;
		PFN_vkCmdSetScissor vkCmdSetScissor;
		PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
		PFN_vkEndCommandBuffer vkEndCommandBuffer;
		PFN_vkCmdExecuteCommands vkCmdExecuteCommands;
		PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
		PFN_vkQueueSubmit vkQueueSubmit;
		PFN_vkQueueWaitIdle vkQueueWaitIdle;
		PFN_vkQueuePresentKHR vkQueuePresentKHR;
		PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
		PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
		PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
		PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
		PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
		PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
		PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
		PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
		PFN_vkCreateFramebuffer vkCreateFramebuffer;
		PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
		PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
		PFN_vkDestroyCommandPool vkDestroyCommandPool;
		PFN_vkCreateRenderPass vkCreateRenderPass;
		PFN_vkDestroyRenderPass vkDestroyRenderPass;
		PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
		PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
		PFN_vkMapMemory vkMapMemory;
		PFN_vkUnmapMemory vkUnmapMemory;
		PFN_vkCreateBuffer vkCreateBuffer;
		PFN_vkDestroyBuffer vkDestroyBuffer;
		PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
		PFN_vkCreateSampler vkCreateSampler;
		PFN_vkDestroySampler vkDestroySampler;
		PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
		PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
		PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
		PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
		PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
		PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
		PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
		PFN_vkFreeDescriptorSets vkFreeDescriptorSets;
		PFN_vkCreateImage vkCreateImage;
		PFN_vkDestroyImage vkDestroyImage;
		PFN_vkCreateImageView vkCreateImageView;
		PFN_vkDestroyImageView vkDestroyImageView;
		PFN_vkAllocateMemory vkAllocateMemory;
		PFN_vkBindBufferMemory vkBindBufferMemory;
		PFN_vkBindImageMemory vkBindImageMemory;
		PFN_vkFreeMemory vkFreeMemory;
		PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
		PFN_vkCreateShaderModule vkCreateShaderModule;
		PFN_vkDestroyShaderModule vkDestroyShaderModule;
		PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
		PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
		PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
		PFN_vkDestroyPipeline vkDestroyPipeline;

		PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
		PFN_vkCmdBindPipeline vkCmdBindPipeline;
		PFN_vkCmdPushConstants vkCmdPushConstants;
		PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
		PFN_vkCmdDraw vkCmdDraw;
		PFN_vkCmdCopyImage vkCmdCopyImage;
		PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
		PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;

		bool m_haveSharpening;
};

class SurfaceVulkanComp : public SurfaceVulkan
{
	public:
		SurfaceVulkanComp();
		virtual ~SurfaceVulkanComp();

		virtual void init();
		virtual void spriteManagerReset();

		VulkanTexture* loadSprite(Uint32 spriteId);
		virtual void drawSprite(Uint32 spriteId, Sint32 x, Sint32 y);
		virtual void drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh);

	protected:
		VulkanTexture** m_sprites;
};

class SurfaceVulkanPerf : public SurfaceVulkan
{
	public:
		SurfaceVulkanPerf();
		virtual ~SurfaceVulkanPerf();

		void generateSpriteAtlases();
		void checkScheduledSprites();

		virtual void init();
		virtual void spriteManagerReset();

		virtual void beginScene();
		virtual void beginGameScene();
		virtual void endGameScene();

		virtual void drawRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
		virtual void fillRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

		bool loadSprite(Uint32 spriteId, VulkanTexture* texture, Uint32 xoff, Uint32 yoff);
		virtual void drawSprite(Uint32 spriteId, Sint32 x, Sint32 y);
		virtual void drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh);
		virtual void drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor);

	protected:
		std::vector<VulkanTexture*> m_spritesAtlas;
		VulkanTexture* m_spriteAtlas;

		VkBuffer m_stagingBuffer;
		VkDeviceMemory m_stagingBufferMemory;
		Uint32 m_gameVertices;

		Uint32 m_spriteAtlases;
		Uint32 m_spritesPerAtlas;
		Uint32 m_spritesPerModulo;

		bool* m_sprites;
		bool m_scheduleSpriteDraw;
};

#define RGB_FRAGMENT_SHADER                                                  \
"#version 400\n"                                                             \
"#extension GL_ARB_separate_shader_objects : enable\n\n"                     \
"layout(location = 0) in vec4 fragColor;\n\n"                                \
"layout(location = 0) out vec4 outColor;\n\n"                                \
"void main() {\n"                                                            \
"	outColor = fragColor;\n"                                                 \
"}"                                                                          \

#define RGB_VERTEX_SHADER                                                    \
"#version 400\n"                                                             \
"#extension GL_ARB_separate_shader_objects : enable\n\n"                     \
"layout(push_constant) uniform UniformBufferObject {\n"                      \
"	float vkWidth;\n"                                                        \
"	float vkHeight;\n"                                                       \
"} ubo;\n\n"                                                                 \
"layout(location = 0) in vec2 inPosition;\n"                                 \
"layout(location = 1) in vec2 inTexcoord;\n"                                 \
"layout(location = 2) in vec4 inColor;\n\n"                                  \
"layout(location = 0) out vec4 fragColor;\n\n"                               \
"void main() {\n"                                                            \
"	float x = fma(inPosition.x, ubo.vkWidth, -1.0);\n"                       \
"	float y = fma(inPosition.y, ubo.vkHeight, -1.0);\n"                      \
"	gl_Position = vec4(x, y, 0.0, 1.0);\n"                                   \
"	fragColor = inColor;\n"                                                  \
"}"                                                                          \

#define TEXTURE_FRAGMENT_SHADER                                              \
"#version 400\n"                                                             \
"#extension GL_ARB_separate_shader_objects : enable\n"                       \
"#extension GL_ARB_shading_language_420pack : enable\n\n"                    \
"layout(binding = 0) uniform sampler2D texSampler;\n\n"                      \
"layout(location = 0) in vec4 fragColor;\n"                                  \
"layout(location = 1) in vec2 fragTexCoord;\n\n"                             \
"layout(location = 0) out vec4 outColor;\n\n"                                \
"void main() {\n"                                                            \
"	outColor = texture(texSampler, fragTexCoord) * fragColor;\n"             \
"}"                                                                          \

#define TEXTURE_VERTEX_SHADER                                                \
"#version 400\n"                                                             \
"#extension GL_ARB_separate_shader_objects : enable\n\n"                     \
"layout(push_constant) uniform UniformBufferObject {\n"                      \
"	float vkWidth;\n"                                                        \
"	float vkHeight;\n"                                                       \
"} ubo;\n\n"                                                                 \
"layout(location = 0) in vec2 inPosition;\n"                                 \
"layout(location = 1) in vec2 inTexcoord;\n"                                 \
"layout(location = 2) in vec4 inColor;\n\n"                                  \
"layout(location = 0) out vec4 fragColor;\n"                                 \
"layout(location = 1) out vec2 fragTexcoord;\n\n"                            \
"void main() {\n"                                                            \
"	float x = fma(inPosition.x, ubo.vkWidth, -1.0);\n"                       \
"	float y = fma(inPosition.y, ubo.vkHeight, -1.0);\n"                      \
"	gl_Position = vec4(x, y, 0.0, 1.0);\n"                                   \
"	fragColor = inColor;\n"                                                  \
"	fragTexcoord = inTexcoord;\n"                                            \
"}"                                                                          \

#define SHARPEN_FRAGMENT_SHADER                                                               \
"#version 400\n"                                                                              \
"#extension GL_ARB_separate_shader_objects : enable\n"                                        \
"#extension GL_ARB_shading_language_420pack : enable\n\n"                                     \
"layout(binding = 0) uniform sampler2D texSampler;\n\n"                                       \
"layout(location = 0) in vec2 textureSize;\n"                                                 \
"layout(location = 1) in vec2 fragTexCoord;\n\n"                                              \
"layout(location = 0) out vec4 outColor;\n\n"                                                 \
"#define sharp_clamp ( 0.050000 )\n"                                                          \
"#define sharp_strength ( 2.000000 )\n"                                                       \
"#define CoefLuma vec3(0.2126, 0.7152, 0.0722)\n\n"                                           \
"void main() {\n"                                                                             \
"	vec3 origset = texture(texSampler, fragTexCoord).rgb;\n"                                  \
"	vec3 sharp_strength_luma = (CoefLuma * sharp_strength);\n"                                \
"	vec2 textureSize2 = vec2(textureSize.x, -textureSize.y);\n"                               \
"	vec3 blur_sample;\n"                                                                      \
"	blur_sample  = texture(texSampler, fragTexCoord + textureSize2).rgb;\n"                   \
"	blur_sample += texture(texSampler, fragTexCoord - textureSize).rgb;\n"                    \
"	blur_sample += texture(texSampler, fragTexCoord + textureSize).rgb;\n"                    \
"	blur_sample += texture(texSampler, fragTexCoord - textureSize2).rgb;\n"                   \
"	blur_sample *= 0.25;\n"                                                                   \
"	vec3 sharp = origset - blur_sample;\n"                                                    \
"	vec4 sharp_strength_luma_clamp = vec4(sharp_strength_luma * (0.5 / sharp_clamp), 0.5);\n" \
"	float sharp_luma = clamp(dot(vec4(sharp, 1.0), sharp_strength_luma_clamp), 0.0, 1.0);\n"  \
"	sharp_luma = (sharp_clamp * 2.0) * sharp_luma - sharp_clamp;\n"                           \
"	vec3 outputcolor = origset + sharp_luma;\n"                                               \
"	outColor = vec4(clamp(outputcolor, 0.0, 1.0), 1.0);\n"                                    \
"}"                                                                                           \

#define SHARPEN_VERTEX_SHADER                                                \
"#version 400\n"                                                             \
"#extension GL_ARB_separate_shader_objects : enable\n\n"                     \
"layout(push_constant) uniform UniformBufferObject {\n"                      \
"	float vkWidth;\n"                                                        \
"	float vkHeight;\n"                                                       \
"	float pxWidth;\n"                                                        \
"	float pxHeight;\n"                                                       \
"} ubo;\n\n"                                                                 \
"layout(location = 0) in vec2 inPosition;\n"                                 \
"layout(location = 1) in vec2 inTexcoord;\n"                                 \
"layout(location = 2) in vec4 inColor;\n\n"                                  \
"layout(location = 0) out vec2 textureSize;\n"                               \
"layout(location = 1) out vec2 fragTexCoord;\n\n"                            \
"void main() {\n"                                                            \
"	float x = fma(inPosition.x, ubo.vkWidth, -1.0);\n"                       \
"	float y = fma(inPosition.y, ubo.vkHeight, -1.0);\n"                      \
"	gl_Position = vec4(x, y, 0.0, 1.0);\n"                                   \
"	textureSize = vec2(ubo.pxWidth, ubo.pxHeight);\n"                        \
"	fragTexCoord = inTexcoord;\n"                                            \
"}"                                                                          \

#endif /* SDL_VIDEO_VULKAN */

#endif /* __FILE_SURFACE_HARDWARE_h_ */
