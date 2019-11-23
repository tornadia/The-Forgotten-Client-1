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

#include "surfaceVulkan.h"

#if defined(SDL_VIDEO_VULKAN)
#include <SDL2/SDL_vulkan.h>

const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

extern Engine g_engine;
extern Uint32 g_spriteCounts;
extern Uint16 g_pictureCounts;
extern bool g_inited;

SurfaceVulkan::SurfaceVulkan() : m_automapTilesBuff(HARDWARE_MAX_AUTOMAPTILES), m_spritesMaskIds(HARDWARE_MAX_SPRITEMASKS)
{
	m_haveSharpening = false;
	m_instance = VK_NULL_HANDLE;
	m_surface = VK_NULL_HANDLE;
	m_gpu = VK_NULL_HANDLE;
	m_device = VK_NULL_HANDLE;
	m_queue = VK_NULL_HANDLE;
	m_presentQueue = VK_NULL_HANDLE;
	m_commandPool = VK_NULL_HANDLE;
	m_commandDataBuffer = VK_NULL_HANDLE;
	m_descriptorSetLayout = VK_NULL_HANDLE;

	m_storeRenderPass = m_clearRenderPass = m_textureStoreRenderPass = m_textureClearRenderPass = VK_NULL_HANDLE;
	m_swapChain = VK_NULL_HANDLE;
	m_samplers[0] = m_samplers[1] = VK_NULL_HANDLE;
	for(Sint32 i = GRAPHIC_PIPELINE_RGBA; i < GRAPHIC_PIPELINE_LAST; ++i)
	{
		m_pipelineLayout[i] = VK_NULL_HANDLE;
		m_graphicsPipeline[i] = VK_NULL_HANDLE;
	}
	for(size_t i = 0; i < VULKAN_INFLIGHT_FRAMES; ++i)
	{
		m_presentCompleteSemaphores[i] = VK_NULL_HANDLE;
		m_renderCompleteSemaphores[i] = VK_NULL_HANDLE;
		m_renderFences[i] = VK_NULL_HANDLE;
		m_commandVertexBuffers[i] = VK_NULL_HANDLE;
		m_commandBuffers[i] = VK_NULL_HANDLE;

		m_staggingBuffer[i] = VK_NULL_HANDLE;
		m_staggingBufferMemory[i] = VK_NULL_HANDLE;
		m_vertexBuffer[i] = VK_NULL_HANDLE;
		m_vertexBufferMemory[i] = VK_NULL_HANDLE;
		m_vertexBufferSize[i] = 0;
	}

	m_vertexOffset = 0;
	m_lightVertices = 0;

	m_imageCount = 0;
	m_currentFrame = 0;
	m_currentGraphicPipeline = GRAPHIC_PIPELINE_LAST;

	m_totalVRAM = 0;
	m_spritesCache = 0;

	m_maxTextureSize = 1024;
	m_integer_scaling_width = 0;
	m_integer_scaling_height = 0;

	m_software = NULL;
	m_hardware = NULL;

	m_scaled_gameWindow = NULL;
	m_pictures = NULL;
	m_gameWindow = NULL;

	m_vulkanVertices.reserve(1048576);
	m_spriteMasks.reserve(HARDWARE_MAX_SPRITEMASKS);
	m_automapTiles.reserve(HARDWARE_MAX_AUTOMAPTILES);
}

SurfaceVulkan::~SurfaceVulkan()
{
	if(m_software)
		SDL_free(m_software);

	if(m_hardware)
		SDL_free(m_hardware);

	if(m_pictures)
	{
		for(Uint16 i = 0; i < g_pictureCounts; ++i)
		{
			if(m_pictures[i])
				releaseVulkanTexture(m_pictures[i]);
		}

		SDL_free(m_pictures);
	}

	for(U32BImages::iterator it = m_automapTiles.begin(), end = m_automapTiles.end(); it != end; ++it)
		releaseVulkanTexture(it->second);

	for(U64BImages::iterator it = m_spriteMasks.begin(), end = m_spriteMasks.end(); it != end; ++it)
		releaseVulkanTexture(it->second);

	for(size_t i = 0; i < VULKAN_INFLIGHT_FRAMES; ++i)
	{
		for(std::vector<VulkanTexture*>::iterator it = m_texturesToDelete[i].begin(), end = m_texturesToDelete[i].end(); it != end; ++it)
			releaseVulkanTexture((*it));

		m_texturesToDelete[i].clear();
	}

	m_spriteMasks.clear();
	m_spritesMaskIds.clear();
	m_automapTilesBuff.clear();
	m_automapTiles.clear();

	cleanupSwapChainResources();
	if(m_samplers[1])
		vkDestroySampler(m_device, m_samplers[1], NULL);

	if(m_samplers[0])
		vkDestroySampler(m_device, m_samplers[0], NULL);

	if(m_descriptorSetLayout)
		vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, NULL);

	if(m_commandPool)
	{
		vkFreeCommandBuffers(m_device, m_commandPool, VULKAN_INFLIGHT_FRAMES, m_commandVertexBuffers);
		vkFreeCommandBuffers(m_device, m_commandPool, VULKAN_INFLIGHT_FRAMES, m_commandBuffers);
		vkFreeCommandBuffers(m_device, m_commandPool, 1, &m_commandDataBuffer);
		vkDestroyCommandPool(m_device, m_commandPool, NULL);
	}

	for(size_t i = 0; i < VULKAN_INFLIGHT_FRAMES; ++i)
	{
		if(m_presentCompleteSemaphores[i])
			vkDestroySemaphore(m_device, m_presentCompleteSemaphores[i], NULL);

		if(m_renderCompleteSemaphores[i])
			vkDestroySemaphore(m_device, m_renderCompleteSemaphores[i], NULL);

		if(m_renderFences[i])
			vkDestroyFence(m_device, m_renderFences[i], NULL);

		if(m_staggingBufferMemory[i])
			vkFreeMemory(m_device, m_staggingBufferMemory[i], NULL);

		if(m_staggingBuffer[i])
			vkDestroyBuffer(m_device, m_staggingBuffer[i], NULL);

		if(m_vertexBufferMemory[i])
			vkFreeMemory(m_device, m_vertexBufferMemory[i], NULL);

		if(m_vertexBuffer[i])
			vkDestroyBuffer(m_device, m_vertexBuffer[i], NULL);
	}

	if(m_device)
		vkDestroyDevice(m_device, NULL);

	if(m_surface)
		vkDestroySurfaceKHR(m_instance, m_surface, NULL);

	if(m_instance)
		vkDestroyInstance(m_instance, NULL);
}

Uint32 SurfaceVulkan::rateDeviceSuitability(VkPhysicalDevice device)
{
	VkPhysicalDeviceFeatures deviceFeatures;
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	Uint32 score = 0;

	//Discrete GPUs have a significant performance advantage
	if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		score += 131072;

	//Higher texture support can mean better performance
	score += deviceProperties.limits.maxImageDimension2D;
	if(deviceProperties.limits.maxImageDimension2D < 1024)
		return 0;

	return score;
}

SwapChainSupportDetails SurfaceVulkan::querySwapChainSupport()
{
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_gpu, m_surface, &details.capabilities);

	Uint32 formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpu, m_surface, &formatCount, NULL);
	if(formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpu, m_surface, &formatCount, &details.formats[0]);
	}

	Uint32 presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_gpu, m_surface, &presentModeCount, NULL);
	if(presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(m_gpu, m_surface, &presentModeCount, &details.presentModes[0]);
	}
	return details;
}

VkSurfaceFormatKHR SurfaceVulkan::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	if(availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
		return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

	for(std::vector<VkSurfaceFormatKHR>::const_iterator it = availableFormats.begin(), end = availableFormats.end(); it != end; ++it)
	{
		VkSurfaceFormatKHR availableFormat = (*it);
		if(availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;
	}
	return availableFormats[0];
}

VkPresentModeKHR SurfaceVulkan::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	VkPresentModeKHR actualPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	if(g_engine.isVsync())
		return actualPresentMode;

	for(std::vector<VkPresentModeKHR>::const_iterator it = availablePresentModes.begin(), end = availablePresentModes.end(); it != end; ++it)
	{
		VkPresentModeKHR presentMode = (*it);
		if(presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return presentMode;
		else if(presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			actualPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	}
	return actualPresentMode;
}

VkExtent2D SurfaceVulkan::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if(capabilities.currentExtent.width != SDL_MAX_UINT32)
		return capabilities.currentExtent;
	else
	{
		VkExtent2D actualExtent = {SDL_static_cast(Uint32, g_engine.getWindowWidth()), SDL_static_cast(Uint32, g_engine.getWindowHeight())};
		actualExtent.width = UTIL_max<Uint32>(capabilities.minImageExtent.width, UTIL_min<Uint32>(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = UTIL_max<Uint32>(capabilities.minImageExtent.height, UTIL_min<Uint32>(capabilities.maxImageExtent.height, actualExtent.height));
		return actualExtent;
	}
}

Uint32 SurfaceVulkan::findMemoryType(Uint32 typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(m_gpu, &memProperties);
	for(Uint32 i = 0; i < memProperties.memoryTypeCount; ++i)
	{
		if((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}
	return 0;//Throw here
}

VkShaderModule SurfaceVulkan::createShaderModule(const char* code, size_t codesize)
{
	VkShaderModuleCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = NULL;
	createInfo.flags = 0;
	createInfo.codeSize = codesize;
	createInfo.pCode = SDL_reinterpret_cast(const Uint32*, code);

	VkShaderModule shaderModule;
	if(vkCreateShaderModule(m_device, &createInfo, NULL, &shaderModule) != VK_SUCCESS)
		return NULL;
	return shaderModule;
}

void SurfaceVulkan::setupTextureRendering(VulkanTexture* texture)
{
	vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout[m_currentGraphicPipeline], 0, 1, &texture->m_descriptorSet, 0, NULL);
}

VulkanTexture* SurfaceVulkan::createVulkanTexture(Uint32 width, Uint32 height, bool linearSampler, bool frameBuffer)
{
	VulkanTexture* texture = new VulkanTexture();
	if(!texture)
		return NULL;

	texture->m_width = width;
	texture->m_height = height;
	texture->m_scaleW = 1.0f/width;
	texture->m_scaleH = 1.0f/height;

	VkImageCreateInfo imageInfo;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	if(frameBuffer)
		imageInfo.usage = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_SAMPLED_BIT);
	else
		imageInfo.usage = (VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_SAMPLED_BIT);
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.pNext = NULL;
	imageInfo.flags = 0;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.queueFamilyIndexCount = 0;
	imageInfo.pQueueFamilyIndices = NULL;
	if(vkCreateImage(m_device, &imageInfo, NULL, &texture->m_textureImage) != VK_SUCCESS)
	{
		releaseVulkanTexture(texture);
		return NULL;
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_device, texture->m_textureImage, &memRequirements);

	VkMemoryAllocateInfo allocInfo;
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if(vkAllocateMemory(m_device, &allocInfo, NULL, &texture->m_textureImageMemory) != VK_SUCCESS)
	{
		releaseVulkanTexture(texture);
		return NULL;
	}

	if(vkBindImageMemory(m_device, texture->m_textureImage, texture->m_textureImageMemory, 0) != VK_SUCCESS)
	{
		releaseVulkanTexture(texture);
		return NULL;
	}

	VkImageViewCreateInfo viewInfo;
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.pNext = NULL;
	viewInfo.flags = 0;
	viewInfo.image = texture->m_textureImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	if(vkCreateImageView(m_device, &viewInfo, NULL, &texture->m_textureView) != VK_SUCCESS)
	{
		releaseVulkanTexture(texture);
		return NULL;
	}

	VkDescriptorPoolSize poolSize;
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo;
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.pNext = NULL;
	poolInfo.flags = 0;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 1;
	if(vkCreateDescriptorPool(m_device, &poolInfo, NULL, &texture->m_descriptorPool) != VK_SUCCESS)
	{
		releaseVulkanTexture(texture);
		return NULL;
	}

	VkDescriptorSetAllocateInfo allocSetInfo;
	allocSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocSetInfo.pNext = NULL;
	allocSetInfo.descriptorPool = texture->m_descriptorPool;
	allocSetInfo.descriptorSetCount = 1;
	allocSetInfo.pSetLayouts = &m_descriptorSetLayout;
	if(vkAllocateDescriptorSets(m_device, &allocSetInfo, &texture->m_descriptorSet) != VK_SUCCESS)
	{
		releaseVulkanTexture(texture);
		return NULL;
	}

	VkDescriptorImageInfo imageDescInfo;
	imageDescInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageDescInfo.imageView = texture->m_textureView;
	imageDescInfo.sampler = m_samplers[(linearSampler ? 1 : 0)];

	VkWriteDescriptorSet descriptorWrite;
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.pNext = NULL;
	descriptorWrite.dstSet = texture->m_descriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageDescInfo;
	descriptorWrite.pBufferInfo = NULL;
	descriptorWrite.pTexelBufferView = NULL;
	vkUpdateDescriptorSets(m_device, 1, &descriptorWrite, 0, NULL);
	return texture;
}

bool SurfaceVulkan::createTextureFramebuffer(VulkanTexture* texture)
{
	VkFramebufferCreateInfo framebufferInfo;
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.pNext = NULL;
	framebufferInfo.flags = 0;
	framebufferInfo.renderPass = m_textureStoreRenderPass;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.pAttachments = &texture->m_textureView;
	framebufferInfo.width = texture->m_width;
	framebufferInfo.height = texture->m_height;
	framebufferInfo.layers = 1;
	if(vkCreateFramebuffer(m_device, &framebufferInfo, NULL, &texture->m_framebuffer) != VK_SUCCESS)
		return false;
	return true;
}

bool SurfaceVulkan::updateTextureData(VulkanTexture* texture, unsigned char* pixels)
{
	VkCommandBufferBeginInfo beginInfo;
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = NULL;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = NULL;
	vkResetCommandBuffer(m_commandDataBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	vkBeginCommandBuffer(m_commandDataBuffer, &beginInfo);

	VkDeviceSize imageSize = (texture->m_width * texture->m_height * 4);
	VkBuffer stagingBuffer = NULL;
	VkDeviceMemory stagingBufferMemory = NULL;
	VkBufferCreateInfo bufferInfo;
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = NULL;
	bufferInfo.flags = 0;
	bufferInfo.size = imageSize;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.queueFamilyIndexCount = 0;
	bufferInfo.pQueueFamilyIndices = NULL;
	if(vkCreateBuffer(m_device, &bufferInfo, NULL, &stagingBuffer) != VK_SUCCESS)
		return false;
		
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_device, stagingBuffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo;
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
	if(vkAllocateMemory(m_device, &allocInfo, NULL, &stagingBufferMemory) != VK_SUCCESS)
	{
		vkDestroyBuffer(m_device, stagingBuffer, NULL);
		return false;
	}
		
	vkBindBufferMemory(m_device, stagingBuffer, stagingBufferMemory, 0);

	void* data;
	vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &data);
	UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, data), SDL_reinterpret_cast(const Uint8*, pixels), SDL_static_cast(size_t, imageSize));
	vkUnmapMemory(m_device, stagingBufferMemory);

	VkImageMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = NULL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.image = texture->m_textureImage;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	vkCmdPipelineBarrier(m_commandDataBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

	VkBufferImageCopy region;
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = {0, 0, 0};
	region.imageExtent = {texture->m_width, texture->m_height, 1};
	vkCmdCopyBufferToImage(m_commandDataBuffer, stagingBuffer, texture->m_textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(m_commandDataBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = NULL;
	submitInfo.pWaitDstStageMask = NULL;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandDataBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = NULL;
	vkEndCommandBuffer(m_commandDataBuffer);
	vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_queue);
	if(stagingBufferMemory)
		vkFreeMemory(m_device, stagingBufferMemory, NULL);
	if(stagingBuffer)
		vkDestroyBuffer(m_device, stagingBuffer, NULL);
	return true;
}

void SurfaceVulkan::releaseVulkanTexture(VulkanTexture* texture)
{
	if(texture->m_descriptorSet)
		vkFreeDescriptorSets(m_device, texture->m_descriptorPool, 1, &texture->m_descriptorSet);
	if(texture->m_descriptorPool)
		vkDestroyDescriptorPool(m_device, texture->m_descriptorPool, NULL);
	if(texture->m_framebuffer)
		vkDestroyFramebuffer(m_device, texture->m_framebuffer, NULL);
	if(texture->m_textureView)
		vkDestroyImageView(m_device, texture->m_textureView, NULL);
	if(texture->m_textureImageMemory)
		vkFreeMemory(m_device, texture->m_textureImageMemory, NULL);
	if(texture->m_textureImage)
		vkDestroyImage(m_device, texture->m_textureImage, NULL);
	delete texture;
}

void SurfaceVulkan::setupGraphicPipeline(Sint32 pipelineId)
{
	if(m_currentGraphicPipeline != pipelineId)
	{
		m_currentGraphicPipeline = pipelineId;
		vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline[m_currentGraphicPipeline]);
	}
}

bool SurfaceVulkan::CreateGraphicPipeline(Sint32 pipelineId)
{
	VkShaderModule vertShaderModule = VK_NULL_HANDLE;
	VkShaderModule fragShaderModule = VK_NULL_HANDLE;
	if(pipelineId == GRAPHIC_PIPELINE_RGBA || pipelineId == GRAPHIC_PIPELINE_RGBA_LINES || pipelineId == GRAPHIC_PIPELINE_LIGHT_MAP)
	{
		vertShaderModule = createShaderModule(RGB_VERTEX_SHADER, sizeof(RGB_VERTEX_SHADER)-1);
		fragShaderModule = createShaderModule(RGB_FRAGMENT_SHADER, sizeof(RGB_FRAGMENT_SHADER)-1);
	}
	else if(pipelineId == GRAPHIC_PIPELINE_SHARPEN)
	{
		vertShaderModule = createShaderModule(SHARPEN_VERTEX_SHADER, sizeof(SHARPEN_VERTEX_SHADER)-1);
		fragShaderModule = createShaderModule(SHARPEN_FRAGMENT_SHADER, sizeof(SHARPEN_FRAGMENT_SHADER)-1);
	}
	else
	{
		vertShaderModule = createShaderModule(TEXTURE_VERTEX_SHADER, sizeof(TEXTURE_VERTEX_SHADER)-1);
		fragShaderModule = createShaderModule(TEXTURE_FRAGMENT_SHADER, sizeof(TEXTURE_FRAGMENT_SHADER)-1);
	}
	if(!vertShaderModule || !fragShaderModule)
	{
		if(vertShaderModule)
			vkDestroyShaderModule(m_device, vertShaderModule, NULL);
		if(fragShaderModule)
			vkDestroyShaderModule(m_device, fragShaderModule, NULL);
		return false;
	}

	VkPipelineShaderStageCreateInfo vertShaderStageInfo;
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.pNext = NULL;
	vertShaderStageInfo.flags = 0;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";
	vertShaderStageInfo.pSpecializationInfo = NULL;

	VkPipelineShaderStageCreateInfo fragShaderStageInfo;
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.pNext = NULL;
	fragShaderStageInfo.flags = 0;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";
	fragShaderStageInfo.pSpecializationInfo = NULL;

	VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

	VkVertexInputBindingDescription vertexBindingDescription;
	vertexBindingDescription.binding = 0;
	vertexBindingDescription.stride = sizeof(VertexVulkan);
	vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
	VkVertexInputAttributeDescription vertexAttributeDescription;
	vertexAttributeDescription.binding = 0;
	vertexAttributeDescription.location = 0;
	vertexAttributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
	vertexAttributeDescription.offset = 0;
	vertexAttributeDescriptions.push_back(vertexAttributeDescription);

	vertexAttributeDescription.binding = 0;
	vertexAttributeDescription.location = 1;
	vertexAttributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
	vertexAttributeDescription.offset = 8;
	vertexAttributeDescriptions.push_back(vertexAttributeDescription);

	vertexAttributeDescription.binding = 0;
	vertexAttributeDescription.location = 2;
	vertexAttributeDescription.format = VK_FORMAT_B8G8R8A8_UNORM;
	vertexAttributeDescription.offset = 16;
	vertexAttributeDescriptions.push_back(vertexAttributeDescription);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo;
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.pNext = NULL;
	vertexInputInfo.flags = 0;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = SDL_static_cast(Uint32, vertexAttributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = &vertexAttributeDescriptions[0];

	VkPipelineInputAssemblyStateCreateInfo inputAssembly;
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.pNext = NULL;
	inputAssembly.flags = 0;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState;
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext = NULL;
	viewportState.flags = 0;
	viewportState.viewportCount = 1;
	viewportState.pViewports = NULL;
	viewportState.scissorCount = 1;
	viewportState.pScissors = NULL;

	VkPipelineRasterizationStateCreateInfo rasterizer;
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.pNext = NULL;
	rasterizer.flags = 0;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling;
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.pNext = NULL;
	multisampling.flags = 0;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 0.0f;
	multisampling.pSampleMask = NULL;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPushConstantRange pushConstantInfo;
	pushConstantInfo.stageFlags = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
	pushConstantInfo.offset = 0;
	pushConstantInfo.size = sizeof(struct PushConstant_Projection);

	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	colorBlendAttachment.colorWriteMask = (VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT);
	switch(pipelineId)
	{
		case GRAPHIC_PIPELINE_RGBA_LINES:
		{
			colorBlendAttachment.blendEnable = VK_TRUE;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		}
		break;
		case GRAPHIC_PIPELINE_LIGHT_MAP:
		{
			colorBlendAttachment.blendEnable = VK_TRUE;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		}
		break;
		case GRAPHIC_PIPELINE_TEXTURE:
		{
			colorBlendAttachment.blendEnable = VK_TRUE;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		}
		break;
		case GRAPHIC_PIPELINE_TEXTURE_NON_BLEND:
		{
			colorBlendAttachment.blendEnable = VK_FALSE;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		}
		break;
		case GRAPHIC_PIPELINE_TEXTURE_TRIANGLES:
		{
			colorBlendAttachment.blendEnable = VK_TRUE;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}
		break;
		case GRAPHIC_PIPELINE_SHARPEN:
		{
			colorBlendAttachment.blendEnable = VK_FALSE;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			pushConstantInfo.size = sizeof(struct PushConstant_Sharpen);
		}
		break;
		default:
		{
			colorBlendAttachment.blendEnable = VK_TRUE;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		}
		break;
	}

	VkPipelineColorBlendStateCreateInfo colorBlending;
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.pNext = NULL;
	colorBlending.flags = 0;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo;
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.pNext = NULL;
	pipelineLayoutInfo.flags = 0;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantInfo;
	if(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, NULL, &m_pipelineLayout[pipelineId]) != VK_SUCCESS)
	{
		vkDestroyShaderModule(m_device, fragShaderModule, NULL);
		vkDestroyShaderModule(m_device, vertShaderModule, NULL);
		return false;
	}

	VkDynamicState dynEnable[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicState;
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pNext = NULL;
	dynamicState.flags = 0;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynEnable;

	VkGraphicsPipelineCreateInfo pipelineInfo;
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = NULL;
	pipelineInfo.flags = 0;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pTessellationState = NULL;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = NULL;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = m_pipelineLayout[pipelineId];
	pipelineInfo.renderPass = m_storeRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = 0;
	VkResult res = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &m_graphicsPipeline[pipelineId]);
	if(res != VK_SUCCESS)
	{
		vkDestroyShaderModule(m_device, fragShaderModule, NULL);
		vkDestroyShaderModule(m_device, vertShaderModule, NULL);
		return false;
	}
	vkDestroyShaderModule(m_device, fragShaderModule, NULL);
	vkDestroyShaderModule(m_device, vertShaderModule, NULL);
	return true;
}

void SurfaceVulkan::cleanupSwapChainResources()
{
	if(m_scaled_gameWindow)
	{
		releaseVulkanTexture(m_scaled_gameWindow);
		m_scaled_gameWindow = NULL;
	}
	if(m_gameWindow)
	{
		releaseVulkanTexture(m_gameWindow);
		m_gameWindow = NULL;
	}

	for(std::vector<VkFramebuffer>::iterator it = m_swapChainFrameBuffer.begin(), end = m_swapChainFrameBuffer.end(); it != end; ++it)
		vkDestroyFramebuffer(m_device, (*it), NULL);

	m_swapChainFrameBuffer.clear();
	for(Sint32 i = GRAPHIC_PIPELINE_RGBA; i < GRAPHIC_PIPELINE_LAST; ++i)
	{
		if(m_graphicsPipeline[i])
		{
			vkDestroyPipeline(m_device, m_graphicsPipeline[i], NULL);
			m_graphicsPipeline[i] = NULL;
		}
		if(m_pipelineLayout[i])
		{
			vkDestroyPipelineLayout(m_device, m_pipelineLayout[i], NULL);
			m_pipelineLayout[i] = NULL;
		}
	}

	if(m_textureClearRenderPass)
	{
		vkDestroyRenderPass(m_device, m_textureClearRenderPass, NULL);
		m_textureClearRenderPass = NULL;
	}

	if(m_textureStoreRenderPass)
	{
		vkDestroyRenderPass(m_device, m_textureStoreRenderPass, NULL);
		m_textureStoreRenderPass = NULL;
	}

	if(m_clearRenderPass)
	{
		vkDestroyRenderPass(m_device, m_clearRenderPass, NULL);
		m_clearRenderPass = NULL;
	}

	if(m_storeRenderPass)
	{
		vkDestroyRenderPass(m_device, m_storeRenderPass, NULL);
		m_storeRenderPass = NULL;
	}

	for(std::vector<VkImageView>::iterator it = m_swapChainImageViews.begin(), end = m_swapChainImageViews.end(); it != end; ++it)
		vkDestroyImageView(m_device, (*it), NULL);

	m_swapChainImageViews.clear();
	if(m_swapChain)
	{
		vkDestroySwapchainKHR(m_device, m_swapChain, NULL);
		m_swapChain = NULL;
	}
}

void SurfaceVulkan::reinitSwapChainResources()
{
	cleanupSwapChainResources();

	SwapChainSupportDetails swapChainSupport = querySwapChainSupport();
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
	
	m_imageCount = swapChainSupport.capabilities.minImageCount;
	if(m_imageCount < VULKAN_INFLIGHT_FRAMES)
		m_imageCount = VULKAN_INFLIGHT_FRAMES;
	if(swapChainSupport.capabilities.maxImageCount > 0 && m_imageCount > swapChainSupport.capabilities.maxImageCount)
		m_imageCount = swapChainSupport.capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.flags = 0;
	createInfo.surface = m_surface;
	createInfo.minImageCount = m_imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	Uint32 queueFamilyIndices[] = {m_graphicsQueueNodeIndex, m_graphicsQueuePresentIndex};
	if(m_graphicsQueueNodeIndex != m_graphicsQueuePresentIndex)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = NULL;
	}

	if(swapChainSupport.capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	else
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	if(vkCreateSwapchainKHR(m_device, &createInfo, NULL, &m_swapChain) != VK_SUCCESS)
	{
		UTIL_MessageBox(true, "Vulkan: Failed to create swapchain.");
		exit(-1);
		return;
	}

	vkGetSwapchainImagesKHR(m_device, m_swapChain, &m_imageCount, NULL);
	if(m_imageCount == 0)
	{
		UTIL_MessageBox(true, "Vulkan: Failed to create swapchain.");
		exit(-1);
		return;
	}

	size_t imageCount = SDL_static_cast(size_t, m_imageCount);
	m_swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapChain, &m_imageCount, &m_swapChainImages[0]);

	m_swapChainImageFormat = surfaceFormat.format;
	m_swapChainExtent = extent;

	m_swapChainImageViews.resize(imageCount);
	for(size_t i = 0; i < imageCount; ++i)
	{
		VkImageViewCreateInfo viewInfo;
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.pNext = NULL;
		viewInfo.flags = 0;
		viewInfo.image = m_swapChainImages[i];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = m_swapChainImageFormat;
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		if(vkCreateImageView(m_device, &viewInfo, NULL, &m_swapChainImageViews[i]) != VK_SUCCESS)
		{
			UTIL_MessageBox(true, "Vulkan: Failed to create imageview.");
			exit(-1);
			return;
		}
	}

	VkAttachmentDescription colorAttachment;
	colorAttachment.flags = 0;
	colorAttachment.format = m_swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef;
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass;
	subpass.flags = 0;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = NULL;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pResolveAttachments = NULL;
	subpass.pDepthStencilAttachment = NULL;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = NULL;

	VkSubpassDependency dependency;
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo;
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext = NULL;
	renderPassInfo.flags = 0;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;
	if(vkCreateRenderPass(m_device, &renderPassInfo, NULL, &m_storeRenderPass) != VK_SUCCESS)
	{
		UTIL_MessageBox(true, "Vulkan: Failed to create render pass.");
		exit(-1);
		return;
	}
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	if(vkCreateRenderPass(m_device, &renderPassInfo, NULL, &m_clearRenderPass) != VK_SUCCESS)
	{
		UTIL_MessageBox(true, "Vulkan: Failed to create render pass.");
		exit(-1);
		return;
	}
	colorAttachment.format = VK_FORMAT_B8G8R8A8_UNORM;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_GENERAL;
	dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	if(vkCreateRenderPass(m_device, &renderPassInfo, NULL, &m_textureStoreRenderPass) != VK_SUCCESS)
	{
		UTIL_MessageBox(true, "Vulkan: Failed to create render pass.");
		exit(-1);
		return;
	}
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	if(vkCreateRenderPass(m_device, &renderPassInfo, NULL, &m_textureClearRenderPass) != VK_SUCCESS)
	{
		UTIL_MessageBox(true, "Vulkan: Failed to create render pass.");
		exit(-1);
		return;
	}

	if(!CreateGraphicPipeline(GRAPHIC_PIPELINE_RGBA) || !CreateGraphicPipeline(GRAPHIC_PIPELINE_RGBA_LINES) || !CreateGraphicPipeline(GRAPHIC_PIPELINE_LIGHT_MAP))
	{
		UTIL_MessageBox(true, "Vulkan: Failed to create graphic pipeline(SOLID COLORS).");
		exit(-1);
		return;
	}
	
	if(!CreateGraphicPipeline(GRAPHIC_PIPELINE_TEXTURE) || !CreateGraphicPipeline(GRAPHIC_PIPELINE_TEXTURE_NON_BLEND) || !CreateGraphicPipeline(GRAPHIC_PIPELINE_TEXTURE_TRIANGLES))
	{
		UTIL_MessageBox(true, "Vulkan: Failed to create graphic pipeline(RGBA TEXTURES).");
		exit(-1);
		return;
	}

	if(CreateGraphicPipeline(GRAPHIC_PIPELINE_SHARPEN))
		m_haveSharpening = true;
	
	m_swapChainFrameBuffer.resize(imageCount);
	for(size_t i = 0; i < imageCount; ++i)
	{
		VkFramebufferCreateInfo framebufferInfo;
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.pNext = NULL;
		framebufferInfo.flags = 0;
		framebufferInfo.renderPass = m_storeRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &m_swapChainImageViews[i];
		framebufferInfo.width = m_swapChainExtent.width;
		framebufferInfo.height = m_swapChainExtent.height;
		framebufferInfo.layers = 1;
		if(vkCreateFramebuffer(m_device, &framebufferInfo, NULL, &m_swapChainFrameBuffer[i]) != VK_SUCCESS)
		{
			UTIL_MessageBox(true, "Vulkan: Failed to create framebuffers.");
			exit(-1);
			return;
		}
	}
}

void SurfaceVulkan::setupProjection()
{
	VkMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	barrier.pNext = NULL;
	barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(m_commandBuffer, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 1, &barrier, 0, NULL, 0, NULL);

	PushConstant_Projection projection;
	projection.vkWidth = m_vkWidth;
	projection.vkHeight = m_vkHeight;
	for(Sint32 i = GRAPHIC_PIPELINE_RGBA; i <= GRAPHIC_PIPELINE_TEXTURE_TRIANGLES; ++i)
		vkCmdPushConstants(m_commandBuffer, m_pipelineLayout[i], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(struct PushConstant_Projection), SDL_reinterpret_cast(void*, &projection));

	vkCmdPipelineBarrier(m_commandBuffer, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 1, &barrier, 0, NULL, 0, NULL);
}

bool SurfaceVulkan::updateVertexBuffer(const void* vertexData, Uint32 dataSize)
{
	VkDeviceSize deviceSize = SDL_static_cast(VkDeviceSize, dataSize);
	if(m_vertexBufferSize[m_currentFrame] < dataSize || !m_vertexBuffer[m_currentFrame] || !m_vertexBufferMemory[m_currentFrame] || !m_staggingBuffer[m_currentFrame] || !m_staggingBufferMemory[m_currentFrame])
	{
		if(m_staggingBufferMemory[m_currentFrame])
		{
			vkFreeMemory(m_device, m_staggingBufferMemory[m_currentFrame], NULL);
			m_staggingBufferMemory[m_currentFrame] = VK_NULL_HANDLE;
		}
		if(m_staggingBuffer[m_currentFrame])
		{
			vkDestroyBuffer(m_device, m_staggingBuffer[m_currentFrame], NULL);
			m_staggingBuffer[m_currentFrame] = VK_NULL_HANDLE;
		}
		if(m_vertexBufferMemory[m_currentFrame])
		{
			vkFreeMemory(m_device, m_vertexBufferMemory[m_currentFrame], NULL);
			m_vertexBufferMemory[m_currentFrame] = VK_NULL_HANDLE;
		}
		if(m_vertexBuffer[m_currentFrame])
		{
			vkDestroyBuffer(m_device, m_vertexBuffer[m_currentFrame], NULL);
			m_vertexBuffer[m_currentFrame] = VK_NULL_HANDLE;
		}

		VkBufferCreateInfo bufferInfo;
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.pNext = NULL;
		bufferInfo.flags = 0;
		bufferInfo.size = deviceSize;
		bufferInfo.usage = (VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.queueFamilyIndexCount = 0;
		bufferInfo.pQueueFamilyIndices = NULL;
		if(vkCreateBuffer(m_device, &bufferInfo, NULL, &m_vertexBuffer[m_currentFrame]) != VK_SUCCESS)
			return false;

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_device, m_vertexBuffer[m_currentFrame], &memRequirements);

		VkMemoryAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = NULL;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if(vkAllocateMemory(m_device, &allocInfo, NULL, &m_vertexBufferMemory[m_currentFrame]) != VK_SUCCESS)
			return false;
		vkBindBufferMemory(m_device, m_vertexBuffer[m_currentFrame], m_vertexBufferMemory[m_currentFrame], 0);

		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.pNext = NULL;
		bufferInfo.flags = 0;
		bufferInfo.size = deviceSize;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.queueFamilyIndexCount = 0;
		bufferInfo.pQueueFamilyIndices = NULL;
		if(vkCreateBuffer(m_device, &bufferInfo, NULL, &m_staggingBuffer[m_currentFrame]) != VK_SUCCESS)
			return false;

		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = NULL;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
		if(vkAllocateMemory(m_device, &allocInfo, NULL, &m_staggingBufferMemory[m_currentFrame]) != VK_SUCCESS)
			return false;

		vkBindBufferMemory(m_device, m_staggingBuffer[m_currentFrame], m_staggingBufferMemory[m_currentFrame], 0);
		m_vertexBufferSize[m_currentFrame] = dataSize;
	}

	void* mappedMem;
	if(vkMapMemory(m_device, m_staggingBufferMemory[m_currentFrame], 0, deviceSize, 0, &mappedMem) != VK_SUCCESS)
		return false;

	UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, mappedMem), SDL_reinterpret_cast(const Uint8*, vertexData), SDL_static_cast(size_t, dataSize));
	vkUnmapMemory(m_device, m_staggingBufferMemory[m_currentFrame]);

	VkBufferCopy copyRegion;
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = deviceSize;
	vkCmdCopyBuffer(m_commandVertexBuffers[m_currentFrame], m_staggingBuffer[m_currentFrame], m_vertexBuffer[m_currentFrame], 1, &copyRegion);
	return true;
}

bool SurfaceVulkan::isSupported()
{
	if(!g_engine.RecreateWindow(true))
	{
		UTIL_MessageBox(false, "Vulkan: Failed to create Vulkan window.");
		return false;
	}

	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = SDL_reinterpret_cast(PFN_vkGetInstanceProcAddr, SDL_Vulkan_GetVkGetInstanceProcAddr());
	if(!vkGetInstanceProcAddr)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to fetch 'vkGetInstanceProcAddr' function.");
		return false;
	}
	vkCreateInstance = SDL_reinterpret_cast(PFN_vkCreateInstance, vkGetInstanceProcAddr(NULL, "vkCreateInstance"));
	if(!vkCreateInstance)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to fetch 'vkCreateInstance' function.");
		return false;
	}

	Uint32 extensionCount;
	if(!SDL_Vulkan_GetInstanceExtensions(g_engine.m_window, &extensionCount, NULL))
	{
		UTIL_MessageBox(false, "Vulkan: Failed to fetch Vulkan extensions.");
		return false;
	}

	const char** extensions = SDL_reinterpret_cast(const char**, malloc(sizeof(const char *) * extensionCount));
	if(!extensions)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to fetch Vulkan extensions.");
		return false;
	}
	if(!SDL_Vulkan_GetInstanceExtensions(g_engine.m_window, &extensionCount, extensions))
	{
		UTIL_MessageBox(false, "Vulkan: Failed to fetch Vulkan extensions.");
		return false;
	}

	VkApplicationInfo app;
	app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app.pNext = NULL;
	app.pApplicationName = "Tibia";
	app.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app.pEngineName = "Vulkan Renderer";
	app.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo inst_info;
	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_info.pNext = NULL;
	inst_info.flags = 0;
	inst_info.pApplicationInfo = &app;
	inst_info.enabledLayerCount = 0;
	inst_info.ppEnabledLayerNames = SDL_reinterpret_cast(const char *const *, NULL);
	inst_info.enabledExtensionCount = extensionCount;
	inst_info.ppEnabledExtensionNames = SDL_reinterpret_cast(const char *const *, extensions);
	if(vkCreateInstance(&inst_info, NULL, &m_instance) != VK_SUCCESS)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to create Vulkan instance.");
		return false;
	}

	#define VKLoadFunction(func) \
    do { \
		##func = SDL_reinterpret_cast(PFN_##func, vkGetInstanceProcAddr(m_instance, #func)); \
		if(!##func) \
		{ \
			UTIL_MessageBox(false, "Vulkan: Failed to fetch '" #func "' function."); \
			return false; \
		} \
    } while(0)

	VKLoadFunction(vkDestroyInstance);
	VKLoadFunction(vkDestroySurfaceKHR);
	VKLoadFunction(vkEnumeratePhysicalDevices);
	VKLoadFunction(vkGetPhysicalDeviceProperties);
	VKLoadFunction(vkGetPhysicalDeviceFeatures);
	VKLoadFunction(vkGetPhysicalDeviceQueueFamilyProperties);
	VKLoadFunction(vkCreateDevice);
	VKLoadFunction(vkGetPhysicalDeviceSurfaceSupportKHR);
	VKLoadFunction(vkDestroyDevice);
	VKLoadFunction(vkGetDeviceQueue);
	VKLoadFunction(vkCreateSemaphore);
	VKLoadFunction(vkDestroySemaphore);
	VKLoadFunction(vkCreateFence);
	VKLoadFunction(vkWaitForFences);
	VKLoadFunction(vkResetFences);
	VKLoadFunction(vkDestroyFence);
	VKLoadFunction(vkResetCommandBuffer);
	VKLoadFunction(vkBeginCommandBuffer);
	VKLoadFunction(vkCreateCommandPool);
	VKLoadFunction(vkAllocateCommandBuffers);
	VKLoadFunction(vkCmdBeginRenderPass);
	VKLoadFunction(vkCmdSetViewport);
	VKLoadFunction(vkCmdSetScissor);
	VKLoadFunction(vkCmdEndRenderPass);
	VKLoadFunction(vkEndCommandBuffer);
	VKLoadFunction(vkCmdExecuteCommands);
	VKLoadFunction(vkDeviceWaitIdle);
	VKLoadFunction(vkQueueSubmit);
	VKLoadFunction(vkQueueWaitIdle);
	VKLoadFunction(vkQueuePresentKHR);
	VKLoadFunction(vkAcquireNextImageKHR);
	VKLoadFunction(vkGetBufferMemoryRequirements);
	VKLoadFunction(vkGetImageMemoryRequirements);
	VKLoadFunction(vkGetPhysicalDeviceMemoryProperties);
	VKLoadFunction(vkGetPhysicalDeviceFormatProperties);
	VKLoadFunction(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
	VKLoadFunction(vkGetPhysicalDeviceSurfaceFormatsKHR);
	VKLoadFunction(vkGetPhysicalDeviceSurfacePresentModesKHR);
	VKLoadFunction(vkCreateFramebuffer);
	VKLoadFunction(vkDestroyFramebuffer);
	VKLoadFunction(vkFreeCommandBuffers);
	VKLoadFunction(vkDestroyCommandPool);
	VKLoadFunction(vkCreateRenderPass);
	VKLoadFunction(vkDestroyRenderPass);
	VKLoadFunction(vkCreateSwapchainKHR);
	VKLoadFunction(vkGetSwapchainImagesKHR);
	VKLoadFunction(vkMapMemory);
	VKLoadFunction(vkUnmapMemory);
	VKLoadFunction(vkCreateBuffer);
	VKLoadFunction(vkDestroyBuffer);
	VKLoadFunction(vkCmdCopyBuffer);
	VKLoadFunction(vkCreateSampler);
	VKLoadFunction(vkDestroySampler);
	VKLoadFunction(vkCreateDescriptorPool);
	VKLoadFunction(vkDestroyDescriptorPool);
	VKLoadFunction(vkCreateDescriptorSetLayout);
	VKLoadFunction(vkDestroyDescriptorSetLayout);
	VKLoadFunction(vkAllocateDescriptorSets);
	VKLoadFunction(vkUpdateDescriptorSets);
	VKLoadFunction(vkCmdBindDescriptorSets);
	VKLoadFunction(vkFreeDescriptorSets);
	VKLoadFunction(vkCreateImage);
	VKLoadFunction(vkDestroyImage);
	VKLoadFunction(vkCreateImageView);
	VKLoadFunction(vkDestroyImageView);
	VKLoadFunction(vkAllocateMemory);
	VKLoadFunction(vkBindBufferMemory);
	VKLoadFunction(vkBindImageMemory);
	VKLoadFunction(vkFreeMemory);
	VKLoadFunction(vkDestroySwapchainKHR);
	VKLoadFunction(vkCreateShaderModule);
	VKLoadFunction(vkDestroyShaderModule);
	VKLoadFunction(vkCreatePipelineLayout);
	VKLoadFunction(vkDestroyPipelineLayout);
	VKLoadFunction(vkCreateGraphicsPipelines);
	VKLoadFunction(vkDestroyPipeline);
	VKLoadFunction(vkCmdPipelineBarrier);
	VKLoadFunction(vkCmdBindPipeline);
	VKLoadFunction(vkCmdPushConstants);
	VKLoadFunction(vkCmdBindVertexBuffers);
	VKLoadFunction(vkCmdDraw);
	VKLoadFunction(vkCmdCopyImage);
	VKLoadFunction(vkCmdCopyBufferToImage);
	VKLoadFunction(vkFlushMappedMemoryRanges);
	#undef VKLoadFunction
	if(!SDL_Vulkan_CreateSurface(g_engine.m_window, m_instance, &m_surface))
	{
		UTIL_MessageBox(false, "Vulkan: Failed to create surface.");
		return false;
	}

	Uint32 gpu_count;
	vkEnumeratePhysicalDevices(m_instance, &gpu_count, NULL);
	if(gpu_count == 0)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to get graphical device.");
		return false;
	}

	std::vector<VkPhysicalDevice> physical_devices(gpu_count);
	vkEnumeratePhysicalDevices(m_instance, &gpu_count, &physical_devices[0]);

	std::multimap<Uint32, VkPhysicalDevice> candidates;
	for(Uint32 i = 0; i < gpu_count; ++i)
	{
		VkPhysicalDevice device = physical_devices[i];
		candidates.insert(std::make_pair(rateDeviceSuitability(device), device));
	}

	if(candidates.rbegin()->first > 0)
		m_gpu = candidates.rbegin()->second;
	else
	{
		UTIL_MessageBox(false, "Vulkan: Failed to get sufficient graphical device.");
		return false;
	}
	
	Uint32 queue_count;
	vkGetPhysicalDeviceQueueFamilyProperties(m_gpu, &queue_count, NULL);
	if(queue_count == 0)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to get graphical device.");
		return false;
	}

	std::vector<VkQueueFamilyProperties> queue_props(queue_count);
	vkGetPhysicalDeviceQueueFamilyProperties(m_gpu, &queue_count, &queue_props[0]);

	m_graphicsQueuePresentIndex = SDL_MAX_UINT32;
	m_graphicsQueueNodeIndex = SDL_MAX_UINT32;
	for(Uint32 i = 0; i < queue_count; ++i)
	{
		if(queue_props[i].queueCount > 0)
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(m_gpu, i, m_surface, &presentSupport);
			if(presentSupport)
				m_graphicsQueuePresentIndex = i;
			if(queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				m_graphicsQueueNodeIndex = i;
		}
		if(m_graphicsQueuePresentIndex != SDL_MAX_UINT32 && m_graphicsQueueNodeIndex != SDL_MAX_UINT32)
			break;
	}
	if(m_graphicsQueueNodeIndex == SDL_MAX_UINT32 || m_graphicsQueuePresentIndex == SDL_MAX_UINT32)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to get sufficient graphical device.");
		return false;
	}

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::vector<Uint32> uniqueQueueFamilies = {m_graphicsQueueNodeIndex, m_graphicsQueuePresentIndex};

	float queue_priorities = 1.0f;
	for(std::vector<Uint32>::iterator it = uniqueQueueFamilies.begin(), end = uniqueQueueFamilies.end(); it != end; ++it)
	{
		VkDeviceQueueCreateInfo queue;
		queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue.pNext = NULL;
		queue.flags = 0;
		queue.queueFamilyIndex = (*it);
		queue.queueCount = 1;
		queue.pQueuePriorities = &queue_priorities;
		queueCreateInfos.push_back(queue);
	}
	
	VkDeviceCreateInfo deviceInfo;
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext = NULL;
	deviceInfo.flags = 0;
	deviceInfo.queueCreateInfoCount = SDL_static_cast(Uint32, queueCreateInfos.size());
	deviceInfo.pQueueCreateInfos = &queueCreateInfos[0];
	deviceInfo.enabledLayerCount = 0;
	deviceInfo.ppEnabledLayerNames = SDL_reinterpret_cast(const char* const*, NULL);
	deviceInfo.enabledExtensionCount = SDL_static_cast(Uint32, deviceExtensions.size());
	deviceInfo.ppEnabledExtensionNames = &deviceExtensions[0];
	deviceInfo.pEnabledFeatures = NULL;
	if(vkCreateDevice(m_gpu, &deviceInfo, NULL, &m_device) != VK_SUCCESS)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to create graphics device.");
		return false;
	}
	
	vkGetDeviceQueue(m_device, m_graphicsQueueNodeIndex, 0, &m_queue);
	vkGetDeviceQueue(m_device, m_graphicsQueuePresentIndex, 0, &m_presentQueue);

	VkFormatProperties props;
	vkGetPhysicalDeviceFormatProperties(m_gpu, VK_FORMAT_B8G8R8A8_UNORM, &props);
	if(!(props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
	{
		UTIL_MessageBox(false, "Vulkan: Failed to get sufficient texture support.");
		return false;
	}

	VkDescriptorSetLayoutBinding samplerLayoutBinding;
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = NULL;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo;
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.pNext = NULL;
	layoutInfo.flags = 0;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &samplerLayoutBinding;
	if(vkCreateDescriptorSetLayout(m_device, &layoutInfo, NULL, &m_descriptorSetLayout) != VK_SUCCESS)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to create descriptor set layout.");
		return false;
	}

	VkCommandPoolCreateInfo poolInfo;
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.pNext = NULL;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = m_graphicsQueueNodeIndex;
	if(vkCreateCommandPool(m_device, &poolInfo, NULL, &m_commandPool) != VK_SUCCESS)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to create command pool.");
		return false;
	}

	VkCommandBufferAllocateInfo allocInfo;
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.commandPool = m_commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
	if(vkAllocateCommandBuffers(m_device, &allocInfo, &m_commandDataBuffer) != VK_SUCCESS)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to allocate commandbuffers.");
		return false;
	}
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.commandPool = m_commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = VULKAN_INFLIGHT_FRAMES;
	if(vkAllocateCommandBuffers(m_device, &allocInfo, m_commandVertexBuffers) != VK_SUCCESS)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to allocate commandbuffers.");
		return false;
	}
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.commandPool = m_commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	allocInfo.commandBufferCount = VULKAN_INFLIGHT_FRAMES;
	if(vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers) != VK_SUCCESS)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to allocate commandbuffers.");
		return false;
	}
	
	VkSamplerCreateInfo samplerInfo;
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.pNext = NULL;
	samplerInfo.flags = 0;
	samplerInfo.magFilter = VK_FILTER_NEAREST;
	samplerInfo.minFilter = VK_FILTER_NEAREST;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 1.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	if(vkCreateSampler(m_device, &samplerInfo, NULL, &m_samplers[0]) != VK_SUCCESS)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to create texture sampler(NEAREST).");
		return false;
	}
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	if(vkCreateSampler(m_device, &samplerInfo, NULL, &m_samplers[1]) != VK_SUCCESS)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to create texture sampler(LINEAR).");
		return false;
	}
	for(size_t i = 0; i < VULKAN_INFLIGHT_FRAMES; ++i)
	{
		VkSemaphoreCreateInfo semaphoreInfo;
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreInfo.pNext = NULL;
		semaphoreInfo.flags = 0;
		if(vkCreateSemaphore(m_device, &semaphoreInfo, NULL, &m_presentCompleteSemaphores[i]) != VK_SUCCESS)
		{
			UTIL_MessageBox(false, "Vulkan: Failed to create synchronization object.");
			return false;
		}
		if(vkCreateSemaphore(m_device, &semaphoreInfo, NULL, &m_renderCompleteSemaphores[i]) != VK_SUCCESS)
		{
			UTIL_MessageBox(false, "Vulkan: Failed to create synchronization object.");
			return false;
		}
		VkFenceCreateInfo fenceInfo;
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.pNext = NULL;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		if(vkCreateFence(m_device, &fenceInfo, NULL, &m_renderFences[i]) != VK_SUCCESS)
		{
			UTIL_MessageBox(false, "Vulkan: Failed to create synchronization object.");
			return false;
		}
	}
	m_currentFrame = 0;
	m_commandBuffer = m_commandBuffers[m_currentFrame];

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(m_gpu, &deviceProperties);
	m_maxTextureSize = SDL_static_cast(Sint32, deviceProperties.limits.maxImageDimension2D);
	if(m_maxTextureSize < 1024)
	{
		UTIL_MessageBox(false, "Vulkan: Your device doesn't meet minimum requirement for texture dimension.");
		return false;
	}
	SDL_snprintf(g_buffer, sizeof(g_buffer), "Vulkan %u.%u.%u", VK_VERSION_MAJOR(deviceProperties.apiVersion), VK_VERSION_MINOR(deviceProperties.apiVersion), VK_VERSION_PATCH(deviceProperties.apiVersion));
	m_software = SDL_strdup(g_buffer);
	m_hardware = SDL_strdup(deviceProperties.deviceName);

	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(m_gpu, &memProperties);
	m_totalVRAM = 0;
	for(size_t i = 0; i < memProperties.memoryHeapCount; ++i)
	{
		if(memProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
			m_totalVRAM += UTIL_power_of_2(SDL_static_cast(Uint32, memProperties.memoryHeaps[i].size/1048576));
	}
	return true;
}

void SurfaceVulkan::renderTargetsRecreate()
{
	if(m_scaled_gameWindow)
	{
		releaseVulkanTexture(m_scaled_gameWindow);
		m_scaled_gameWindow = NULL;
	}
	if(m_gameWindow)
	{
		releaseVulkanTexture(m_gameWindow);
		m_gameWindow = NULL;
	}

	m_gameWindow = createVulkanTexture(RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT, (g_engine.getAntialiasing() == CLIENT_ANTIALIASING_NORMAL), true);
	if(!m_gameWindow || !createTextureFramebuffer(m_gameWindow))
	{
		UTIL_MessageBox(true, "Vulkan: Out of video memory.");
		exit(-1);
		return;
	}
}

void SurfaceVulkan::init()
{
	m_pictures = SDL_reinterpret_cast(VulkanTexture**, SDL_calloc(g_pictureCounts, sizeof(VulkanTexture*)));
	if(!m_pictures)
	{
		SDL_OutOfMemory();
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Surface::Init() Failed: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
	}
	renderTargetsRecreate();
}

void SurfaceVulkan::doResize(Sint32, Sint32)
{
	vkDeviceWaitIdle(m_device);
	reinitSwapChainResources();
	renderTargetsRecreate();
}

void SurfaceVulkan::spriteManagerReset()
{
	for(U32BImages::iterator it = m_automapTiles.begin(), end = m_automapTiles.end(); it != end; ++it)
		releaseVulkanTexture(it->second);

	for(U64BImages::iterator it = m_spriteMasks.begin(), end = m_spriteMasks.end(); it != end; ++it)
		releaseVulkanTexture(it->second);

	m_spriteMasks.clear();
	m_spritesMaskIds.clear();
	m_automapTilesBuff.clear();
	m_automapTiles.clear();
}

unsigned char* SurfaceVulkan::getScreenPixels(Sint32& width, Sint32& height, bool& bgra)
{
	vkQueueWaitIdle(m_queue);
	vkQueueWaitIdle(m_presentQueue);

	width = g_engine.getWindowWidth();
	height = g_engine.getWindowHeight();

	VkImage stagingTexture;
	VkDeviceMemory stagingTextureMemory;
	VkImageCreateInfo imageInfo;
	imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
	imageInfo.usage = (VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_SAMPLED_BIT);
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.pNext = NULL;
	imageInfo.flags = 0;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = SDL_static_cast(Uint32, width);
	imageInfo.extent.height = SDL_static_cast(Uint32, height);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.queueFamilyIndexCount = 0;
	imageInfo.pQueueFamilyIndices = NULL;
	if(vkCreateImage(m_device, &imageInfo, NULL, &stagingTexture) != VK_SUCCESS)
		return NULL;

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_device, stagingTexture, &memRequirements);

	VkMemoryAllocateInfo allocInfo;
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	if(vkAllocateMemory(m_device, &allocInfo, NULL, &stagingTextureMemory) != VK_SUCCESS)
	{
		vkDestroyImage(m_device, stagingTexture, NULL);
		return NULL;
	}

	vkBindImageMemory(m_device, stagingTexture, stagingTextureMemory, 0);
	vkResetCommandBuffer(m_commandDataBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

	VkCommandBufferBeginInfo beginInfo;
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = NULL;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = NULL;
	vkBeginCommandBuffer(m_commandDataBuffer, &beginInfo);

	VkImageMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = NULL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.image = stagingTexture;
	barrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	vkCmdPipelineBarrier(m_commandDataBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

	VkImageCopy copyRegion;
	copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.srcSubresource.mipLevel = 0;
	copyRegion.srcSubresource.baseArrayLayer = 0;
	copyRegion.srcSubresource.layerCount = 1;
	copyRegion.srcOffset.x = 0;
	copyRegion.srcOffset.y = 0;
	copyRegion.srcOffset.z = 0;
	copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.dstSubresource.mipLevel = 0;
	copyRegion.dstSubresource.baseArrayLayer = 0;
	copyRegion.dstSubresource.layerCount = 1;
	copyRegion.dstOffset.x = 0;
	copyRegion.dstOffset.y = 0;
	copyRegion.dstOffset.z = 0;
	copyRegion.extent.width = SDL_static_cast(Uint32, width);
	copyRegion.extent.height = SDL_static_cast(Uint32, height);
	copyRegion.extent.depth = 1;
	vkCmdCopyImage(m_commandDataBuffer, m_swapChainImages[m_imageIndex], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingTexture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(m_commandDataBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = NULL;
	submitInfo.pWaitDstStageMask = NULL;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandDataBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = NULL;
	vkEndCommandBuffer(m_commandDataBuffer);
	vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_queue);

	unsigned char* pixels = SDL_reinterpret_cast(unsigned char*, SDL_malloc(width*height*4));
	if(!pixels)
	{
		vkFreeMemory(m_device, stagingTextureMemory, NULL);
		vkDestroyImage(m_device, stagingTexture, NULL);
		return NULL;
	}

	void* data;
	if(vkMapMemory(m_device, stagingTextureMemory, 0, VK_WHOLE_SIZE, 0, &data) != VK_SUCCESS)
	{
		vkFreeMemory(m_device, stagingTextureMemory, NULL);
		vkDestroyImage(m_device, stagingTexture, NULL);
		return NULL;
	}
	UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, pixels), SDL_reinterpret_cast(const Uint8*, data), SDL_static_cast(size_t, width*height*4));
	vkUnmapMemory(m_device, stagingTextureMemory);
	vkFreeMemory(m_device, stagingTextureMemory, NULL);
	vkDestroyImage(m_device, stagingTexture, NULL);
	bgra = true;
	return pixels;
}

void SurfaceVulkan::beginScene()
{
	Sint32 width = g_engine.getWindowWidth();
	Sint32 height = g_engine.getWindowHeight();

	m_vkWidth = 2.0f / width;
	m_vkHeight = 2.0f / height;

	vkWaitForFences(m_device, 1, &m_renderFences[m_currentFrame], VK_TRUE, SDL_MAX_UINT64);
	vkResetFences(m_device, 1, &m_renderFences[m_currentFrame]);

	VkResult result = vkAcquireNextImageKHR(m_device, m_swapChain, SDL_MAX_UINT64, m_presentCompleteSemaphores[m_currentFrame], VK_NULL_HANDLE, &m_imageIndex);
	if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		vkDeviceWaitIdle(m_device);
		reinitSwapChainResources();
		renderTargetsRecreate();
		vkAcquireNextImageKHR(m_device, m_swapChain, SDL_MAX_UINT64, m_presentCompleteSemaphores[m_currentFrame], VK_NULL_HANDLE, &m_imageIndex);
	}
	else if(result != VK_SUCCESS)
	{
		return;
	}
	std::vector<VulkanTexture*>& texturesToDelete = m_texturesToDelete[m_currentFrame];
	if(!texturesToDelete.empty())
	{
		for(std::vector<VulkanTexture*>::iterator it = texturesToDelete.begin(), end = texturesToDelete.end(); it != end; ++it)
			releaseVulkanTexture((*it));
	}

	m_vulkanVertices.clear();
	m_vertexOffset = 0;

	VkCommandBufferBeginInfo beginInfo;
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = NULL;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = NULL;
	vkResetCommandBuffer(m_commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
	if(m_graphicsQueueNodeIndex != m_graphicsQueuePresentIndex)
	{
		VkImageMemoryBarrier barrier;
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext = NULL;
		barrier.srcQueueFamilyIndex = m_graphicsQueuePresentIndex;
		barrier.dstQueueFamilyIndex = m_graphicsQueueNodeIndex;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.image = m_swapChainImages[m_imageIndex];
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vkCmdPipelineBarrier(m_commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
	}
	
	VkRenderPassBeginInfo rpBeginInfo;
	rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBeginInfo.pNext = NULL;
	rpBeginInfo.renderPass = m_storeRenderPass;
	rpBeginInfo.framebuffer = m_swapChainFrameBuffer[m_imageIndex];
	rpBeginInfo.renderArea.offset = {0, 0};
	rpBeginInfo.renderArea.extent.width = width;
	rpBeginInfo.renderArea.extent.height = height;
	rpBeginInfo.clearValueCount = 0;
	rpBeginInfo.pClearValues = NULL;
	vkCmdBeginRenderPass(m_commandBuffer, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	m_currentGraphicPipeline = GRAPHIC_PIPELINE_LAST;

	VkViewport viewport;
	viewport.x = viewport.y = 0.0f;
	viewport.width = SDL_static_cast(float, width);
	viewport.height = SDL_static_cast(float, height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

	VkRect2D scissor;
	scissor.offset = {0, 0};
	scissor.extent.width = width;
	scissor.extent.height = height;
	m_scissorWidth = width;
	m_scissorHeight = height;
	vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
	setupProjection();
}

void SurfaceVulkan::endScene()
{
	vkCmdEndRenderPass(m_commandBuffer);
	if(m_graphicsQueueNodeIndex != m_graphicsQueuePresentIndex)
	{
		VkImageMemoryBarrier barrier;
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext = NULL;
		barrier.srcQueueFamilyIndex = m_graphicsQueueNodeIndex;
		barrier.dstQueueFamilyIndex = m_graphicsQueuePresentIndex;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.image = m_swapChainImages[m_imageIndex];
		barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		vkCmdPipelineBarrier(m_commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
	}
	vkEndCommandBuffer(m_commandBuffer);

	VkCommandBufferBeginInfo beginInfo;
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = NULL;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = NULL;
	vkResetCommandBuffer(m_commandVertexBuffers[m_currentFrame], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	vkBeginCommandBuffer(m_commandVertexBuffers[m_currentFrame], &beginInfo);
	if(!updateVertexBuffer(&m_vulkanVertices[0], SDL_static_cast(Uint32, sizeof(VertexVulkan)*m_vulkanVertices.size())))
	{
		UTIL_MessageBox(true, "SurfaceVulkan::updateVertexBuffer: Failed to upload vbo to gpu.");
		exit(-1);
	}
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(m_commandVertexBuffers[m_currentFrame], 0, 1, &m_vertexBuffer[m_currentFrame], &offset);
	vkCmdExecuteCommands(m_commandVertexBuffers[m_currentFrame], 1, &m_commandBuffer);
	vkEndCommandBuffer(m_commandVertexBuffers[m_currentFrame]);

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	
	VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &m_presentCompleteSemaphores[m_currentFrame];
	submitInfo.pWaitDstStageMask = &waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandVertexBuffers[m_currentFrame];

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_renderCompleteSemaphores[m_currentFrame];
	vkQueueSubmit(m_queue, 1, &submitInfo, m_renderFences[m_currentFrame]);

	VkPresentInfoKHR presentInfo;
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = NULL;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_renderCompleteSemaphores[m_currentFrame];

	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_swapChain;
	presentInfo.pImageIndices = &m_imageIndex;
	presentInfo.pResults = NULL;

	VkResult result = vkQueuePresentKHR(m_presentQueue, &presentInfo);
	if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		vkDeviceWaitIdle(m_device);
		reinitSwapChainResources();
		renderTargetsRecreate();
	}
	if(++m_currentFrame >= VULKAN_INFLIGHT_FRAMES)
		m_currentFrame = 0;
	m_commandBuffer = m_commandBuffers[m_currentFrame];
}

bool SurfaceVulkan::integer_scaling(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	Sint32 width = RENDERTARGET_WIDTH;
	Sint32 height = RENDERTARGET_HEIGHT;
	UTIL_integer_scale(width, height, w, h, m_maxTextureSize, m_maxTextureSize);
	if(m_integer_scaling_width != width || m_integer_scaling_height != height || !m_scaled_gameWindow)
	{
		if(m_scaled_gameWindow)
		{
			releaseVulkanTexture(m_scaled_gameWindow);
			m_scaled_gameWindow = NULL;
		}

		m_scaled_gameWindow = createVulkanTexture(width, height, true, true);
		if(!m_scaled_gameWindow || !createTextureFramebuffer(m_scaled_gameWindow))
			return false;

		m_integer_scaling_width = width;
		m_integer_scaling_height = height;
	}

	float cvkw = m_vkWidth;
	float cvkh = m_vkHeight;

	m_vkWidth = 2.0f / width;
	m_vkHeight = 2.0f / height;

	VkRenderPassBeginInfo rpBeginInfo;
	rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBeginInfo.pNext = NULL;
	rpBeginInfo.renderPass = m_storeRenderPass;
	rpBeginInfo.framebuffer = m_scaled_gameWindow->m_framebuffer;
	rpBeginInfo.renderArea.offset = {0, 0};
	rpBeginInfo.renderArea.extent.width = width;
	rpBeginInfo.renderArea.extent.height = height;
	rpBeginInfo.clearValueCount = 0;
	rpBeginInfo.pClearValues = NULL;
	vkCmdEndRenderPass(m_commandBuffer);
	vkCmdBeginRenderPass(m_commandBuffer, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	m_currentGraphicPipeline = GRAPHIC_PIPELINE_LAST;

	VkViewport viewport;
	viewport.x = viewport.y = 0.0f;
	viewport.width = SDL_static_cast(float, width);
	viewport.height = SDL_static_cast(float, height);
	viewport.minDepth = 0;
	viewport.maxDepth = 1;
	vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

	VkRect2D scissor;
	scissor.offset = {0, 0};
	scissor.extent.width = width;
	scissor.extent.height = height;
	vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);

	VkMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	barrier.pNext = NULL;
	barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(m_commandBuffer, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 1, &barrier, 0, NULL, 0, NULL);

	PushConstant_Projection projection;
	projection.vkWidth = m_vkWidth;
	projection.vkHeight = m_vkHeight;
	setupGraphicPipeline(GRAPHIC_PIPELINE_TEXTURE_NON_BLEND);
	vkCmdPushConstants(m_commandBuffer, m_pipelineLayout[m_currentGraphicPipeline], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(struct PushConstant_Projection), SDL_reinterpret_cast(void*, &projection));
	vkCmdPipelineBarrier(m_commandBuffer, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 1, &barrier, 0, NULL, 0, NULL);

	float minx = 0.0f;
	float maxx = SDL_static_cast(float, width);
	float miny = 0.0f;
	float maxy = SDL_static_cast(float, height);

	float minu = sx*m_gameWindow->m_scaleW;
	float maxu = (sx+sw)*m_gameWindow->m_scaleW;
	float minv = sy*m_gameWindow->m_scaleH;
	float maxv = (sy+sh)*m_gameWindow->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);
	m_vulkanVertices.emplace_back(minx, miny, minu, minv, texColor);
	m_vulkanVertices.emplace_back(minx, maxy, minu, maxv, texColor);
	m_vulkanVertices.emplace_back(maxx, miny, maxu, minv, texColor);
	m_vulkanVertices.emplace_back(maxx, maxy, maxu, maxv, texColor);

	setupTextureRendering(m_gameWindow);
	vkCmdDraw(m_commandBuffer, 4, 1, m_vertexOffset, 0);
	m_vertexOffset += 4;

	width = g_engine.getWindowWidth();
	height = g_engine.getWindowHeight();

	m_vkWidth = cvkw;
	m_vkHeight = cvkh;

	rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBeginInfo.pNext = NULL;
	rpBeginInfo.renderPass = m_storeRenderPass;
	rpBeginInfo.framebuffer = m_swapChainFrameBuffer[m_imageIndex];
	rpBeginInfo.renderArea.offset = {0, 0};
	rpBeginInfo.renderArea.extent.width = width;
	rpBeginInfo.renderArea.extent.height = height;
	rpBeginInfo.clearValueCount = 0;
	rpBeginInfo.pClearValues = NULL;
	vkCmdEndRenderPass(m_commandBuffer);
	vkCmdBeginRenderPass(m_commandBuffer, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	m_currentGraphicPipeline = GRAPHIC_PIPELINE_LAST;

	viewport.x = viewport.y = 0.0f;
	viewport.width = SDL_static_cast(float, width);
	viewport.height = SDL_static_cast(float, height);
	viewport.minDepth = 0;
	viewport.maxDepth = 1;
	vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

	scissor.offset = {0, 0};
	scissor.extent.width = width;
	scissor.extent.height = width;
	vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);

	vkCmdPipelineBarrier(m_commandBuffer, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 1, &barrier, 0, NULL, 0, NULL);
	if(g_engine.isSharpening() && m_haveSharpening)
	{
		PushConstant_Sharpen prj;
		prj.vkWidth = m_vkWidth;
		prj.vkHeight = m_vkHeight;
		prj.pxWidth = 1.0f / w;
		prj.pxHeight = 1.0f / h;
		setupGraphicPipeline(GRAPHIC_PIPELINE_SHARPEN);
		vkCmdPushConstants(m_commandBuffer, m_pipelineLayout[m_currentGraphicPipeline], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(struct PushConstant_Sharpen), SDL_reinterpret_cast(void*, &prj));
	}
	else
	{
		projection.vkWidth = m_vkWidth;
		projection.vkHeight = m_vkHeight;
		setupGraphicPipeline(GRAPHIC_PIPELINE_TEXTURE_NON_BLEND);
		vkCmdPushConstants(m_commandBuffer, m_pipelineLayout[m_currentGraphicPipeline], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(struct PushConstant_Projection), SDL_reinterpret_cast(void*, &projection));
	}
	vkCmdPipelineBarrier(m_commandBuffer, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 1, &barrier, 0, NULL, 0, NULL);

	minx = SDL_static_cast(float, x);
	maxx = SDL_static_cast(float, x+w);
	miny = SDL_static_cast(float, y);
	maxy = SDL_static_cast(float, y+h);

	minu = 0.0f;
	maxu = m_scaled_gameWindow->m_width*m_scaled_gameWindow->m_scaleW;
	minv = 0.0f;
	maxv = m_scaled_gameWindow->m_height*m_scaled_gameWindow->m_scaleH;

	m_vulkanVertices.emplace_back(minx, miny, minu, minv, texColor);
	m_vulkanVertices.emplace_back(minx, maxy, minu, maxv, texColor);
	m_vulkanVertices.emplace_back(maxx, miny, maxu, minv, texColor);
	m_vulkanVertices.emplace_back(maxx, maxy, maxu, maxv, texColor);

	setupTextureRendering(m_scaled_gameWindow);
	vkCmdDraw(m_commandBuffer, 4, 1, m_vertexOffset, 0);
	m_vertexOffset += 4;
	setupProjection();
	return true;
}

void SurfaceVulkan::drawGameScene(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
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

	if(g_engine.isSharpening() && m_haveSharpening)
	{
		VkMemoryBarrier barrier;
		barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		barrier.pNext = NULL;
		barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(m_commandBuffer, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 1, &barrier, 0, NULL, 0, NULL);

		PushConstant_Sharpen prj;
		prj.vkWidth = m_vkWidth;
		prj.vkHeight = m_vkHeight;
		prj.pxWidth = 1.0f / w;
		prj.pxHeight = 1.0f / h;
		setupGraphicPipeline(GRAPHIC_PIPELINE_SHARPEN);
		vkCmdPushConstants(m_commandBuffer, m_pipelineLayout[m_currentGraphicPipeline], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(struct PushConstant_Sharpen), SDL_reinterpret_cast(void*, &prj));
		vkCmdPipelineBarrier(m_commandBuffer, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 1, &barrier, 0, NULL, 0, NULL);
	}
	else
		setupGraphicPipeline(GRAPHIC_PIPELINE_TEXTURE_NON_BLEND);
	setupTextureRendering(m_gameWindow);

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	float minu = sx*m_gameWindow->m_scaleW;
	float maxu = (sx+sw)*m_gameWindow->m_scaleW;
	float minv = sy*m_gameWindow->m_scaleH;
	float maxv = (sy+sh)*m_gameWindow->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);
	m_vulkanVertices.emplace_back(minx, miny, minu, minv, texColor);
	m_vulkanVertices.emplace_back(minx, maxy, minu, maxv, texColor);
	m_vulkanVertices.emplace_back(maxx, miny, maxu, minv, texColor);
	m_vulkanVertices.emplace_back(maxx, maxy, maxu, maxv, texColor);

	vkCmdDraw(m_commandBuffer, 4, 1, m_vertexOffset, 0);
	m_vertexOffset += 4;
}

void SurfaceVulkan::beginGameScene()
{
	Sint32 width = m_gameWindow->m_width;
	Sint32 height = m_gameWindow->m_height;

	m_vkWidth = 2.0f / width;
	m_vkHeight = 2.0f / height;

	VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
	VkRenderPassBeginInfo rpBeginInfo;
	rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBeginInfo.pNext = NULL;
	rpBeginInfo.renderPass = m_textureClearRenderPass;
	rpBeginInfo.framebuffer = m_gameWindow->m_framebuffer;
	rpBeginInfo.renderArea.offset = {0, 0};
	rpBeginInfo.renderArea.extent.width = width;
	rpBeginInfo.renderArea.extent.height = height;
	rpBeginInfo.clearValueCount = 1;
	rpBeginInfo.pClearValues = &clearColor;
	vkCmdEndRenderPass(m_commandBuffer);
	vkCmdBeginRenderPass(m_commandBuffer, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	m_currentGraphicPipeline = GRAPHIC_PIPELINE_LAST;

	VkViewport viewport;
	viewport.x = viewport.y = 0.0f;
	viewport.width = SDL_static_cast(float, width);
	viewport.height = SDL_static_cast(float, height);
	viewport.minDepth = 0;
	viewport.maxDepth = 1;
	vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

	VkRect2D scissor;
	scissor.offset = {0, 0};
	scissor.extent.width = width;
	scissor.extent.height = height;
	m_scissorWidth = width;
	m_scissorHeight = height;
	vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
	setupProjection();
}

void SurfaceVulkan::endGameScene()
{
	Sint32 width = g_engine.getWindowWidth();
	Sint32 height = g_engine.getWindowHeight();

	m_vkWidth = 2.0f / width;
	m_vkHeight = 2.0f / height;

	VkRenderPassBeginInfo rpBeginInfo;
	rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBeginInfo.pNext = NULL;
	rpBeginInfo.renderPass = m_storeRenderPass;
	rpBeginInfo.framebuffer = m_swapChainFrameBuffer[m_imageIndex];
	rpBeginInfo.renderArea.offset = {0, 0};
	rpBeginInfo.renderArea.extent.width = width;
	rpBeginInfo.renderArea.extent.height = height;
	rpBeginInfo.clearValueCount = 0;
	rpBeginInfo.pClearValues = NULL;
	vkCmdEndRenderPass(m_commandBuffer);
	vkCmdBeginRenderPass(m_commandBuffer, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	m_currentGraphicPipeline = GRAPHIC_PIPELINE_LAST;

	VkViewport viewport;
	viewport.x = viewport.y = 0.0f;
	viewport.width = SDL_static_cast(float, width);
	viewport.height = SDL_static_cast(float, height);
	viewport.minDepth = 0;
	viewport.maxDepth = 1;
	vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

	VkRect2D scissor;
	scissor.offset = {0, 0};
	scissor.extent.width = width;
	scissor.extent.height = height;
	m_scissorWidth = width;
	m_scissorHeight = height;
	vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
	setupProjection();
}

void SurfaceVulkan::drawLightMap(LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height)
{
	setupGraphicPipeline(GRAPHIC_PIPELINE_LIGHT_MAP);

	Sint32 drawY = y-scale-(scale/2);
	height -= 1;
	for(Sint32 j = -1; j < height; ++j)
	{
		Sint32 offset1 = (j + 1) * width;
		Sint32 offset2 = UTIL_max<Sint32>(j, 0) * width;
		Uint32 verticeCount = 0;
		Sint32 drawX = x-scale-(scale/2);
		for(Sint32 k = -1; k < width; ++k)
		{
			Sint32 offset = UTIL_max<Sint32>(k, 0);
			float minx = SDL_static_cast(float, drawX);
			float miny = SDL_static_cast(float, drawY);
			float maxy = miny+SDL_static_cast(float, scale);
			m_vulkanVertices.emplace_back(minx, maxy, 0.0f, 0.0f, MAKE_RGBA_COLOR(lightmap[offset1+offset].r, lightmap[offset+offset1].g, lightmap[offset1+offset].b, 255));
			m_vulkanVertices.emplace_back(minx, miny, 0.0f, 0.0f, MAKE_RGBA_COLOR(lightmap[offset2+offset].r, lightmap[offset2+offset].g, lightmap[offset2+offset].b, 255));

			verticeCount += 2;
			drawX += scale;
		}
		drawY += scale;
		vkCmdDraw(m_commandBuffer, verticeCount, 1, m_vertexOffset, 0);
		m_vertexOffset += verticeCount;
	}
}

void SurfaceVulkan::setClipRect(Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	VkRect2D scissor;
	scissor.offset.x = x;
	scissor.offset.y = y;
	scissor.extent.width = w;
	scissor.extent.height = h;
	vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
}

void SurfaceVulkan::disableClipRect()
{
	VkRect2D scissor;
	scissor.offset = {0, 0};
	scissor.extent.width = m_scissorWidth;
	scissor.extent.height = m_scissorHeight;
	vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
}

void SurfaceVulkan::drawRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	setupGraphicPipeline(GRAPHIC_PIPELINE_RGBA_LINES);

	float minx = SDL_static_cast(float, x)+0.5f;
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y)+0.5f;
	float maxy = SDL_static_cast(float, y+h);

	DWORD texColor = MAKE_RGBA_COLOR(r, g, b, a);
	m_vulkanVertices.emplace_back(minx, miny, 0.0f, 0.0f, texColor);
	m_vulkanVertices.emplace_back(maxx, miny, 0.0f, 0.0f, texColor);
	m_vulkanVertices.emplace_back(maxx, maxy, 0.0f, 0.0f, texColor);
	m_vulkanVertices.emplace_back(minx, maxy, 0.0f, 0.0f, texColor);
	m_vulkanVertices.emplace_back(minx, miny, 0.0f, 0.0f, texColor);

	vkCmdDraw(m_commandBuffer, 5, 1, m_vertexOffset, 0);
	m_vertexOffset += 5;
}

void SurfaceVulkan::fillRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	setupGraphicPipeline(GRAPHIC_PIPELINE_RGBA);

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	DWORD texColor = MAKE_RGBA_COLOR(r, g, b, a);
	m_vulkanVertices.emplace_back(minx, miny, 0.0f, 0.0f, texColor);
	m_vulkanVertices.emplace_back(minx, maxy, 0.0f, 0.0f, texColor);
	m_vulkanVertices.emplace_back(maxx, miny, 0.0f, 0.0f, texColor);
	m_vulkanVertices.emplace_back(maxx, maxy, 0.0f, 0.0f, texColor);

	vkCmdDraw(m_commandBuffer, 4, 1, m_vertexOffset, 0);
	m_vertexOffset += 4;
}

VulkanTexture* SurfaceVulkan::loadPicture(Uint16 pictureId, bool linearSampler)
{
	Sint32 width, height;
	unsigned char* pixels = g_engine.LoadPicture(pictureId, true, width, height);
	if(!pixels)
		return NULL;

	VulkanTexture* s = createVulkanTexture(width, height, linearSampler);
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

void SurfaceVulkan::drawFont(Uint16 pictureId, Sint32 x, Sint32 y, const std::string& text, size_t pos, size_t len, Uint8 r, Uint8 g, Uint8 b, Sint32 cX[256], Sint32 cY[256], Sint32 cW[256], Sint32 cH[256])
{
	VulkanTexture* tex = m_pictures[pictureId];
	if(!tex)
	{
		tex = loadPicture(pictureId, false);
		if(!tex)
			return;//load failed
	}

	DWORD texColor = MAKE_RGBA_COLOR(r, g, b, 255);
	Uint32 drawTriangles = 0;

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

				m_vulkanVertices.emplace_back(minx, miny, minu, minv, texColor);
				m_vulkanVertices.emplace_back(minx, maxy, minu, maxv, texColor);
				m_vulkanVertices.emplace_back(maxx, miny, maxu, minv, texColor);

				m_vulkanVertices.emplace_back(maxx, maxy, maxu, maxv, texColor);
				m_vulkanVertices.emplace_back(maxx, miny, maxu, minv, texColor);
				m_vulkanVertices.emplace_back(minx, maxy, minu, maxv, texColor);

				drawTriangles += 6;
				rx += cW[character] + cX[0];
			}
			break;
		}
	}

	if(drawTriangles > 0)
	{
		setupGraphicPipeline(GRAPHIC_PIPELINE_TEXTURE_TRIANGLES);
		setupTextureRendering(tex);
		vkCmdDraw(m_commandBuffer, drawTriangles, 1, m_vertexOffset, 0);
		m_vertexOffset += drawTriangles;
	}
}

void SurfaceVulkan::drawBackground(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	VulkanTexture* tex = m_pictures[pictureId];
	if(!tex)
	{
		tex = loadPicture(pictureId, g_engine.hasAntialiasing());
		if(!tex)
			return;//load failed
	}

	setupGraphicPipeline(GRAPHIC_PIPELINE_TEXTURE_NON_BLEND);
	setupTextureRendering(tex);

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	float minu = sx*tex->m_scaleW;
	float maxu = (sx+sw)*tex->m_scaleW;
	float minv = sy*tex->m_scaleH;
	float maxv = (sy+sh)*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);
	m_vulkanVertices.emplace_back(minx, miny, minu, minv, texColor);
	m_vulkanVertices.emplace_back(minx, maxy, minu, maxv, texColor);
	m_vulkanVertices.emplace_back(maxx, miny, maxu, minv, texColor);
	m_vulkanVertices.emplace_back(maxx, maxy, maxu, maxv, texColor);

	vkCmdDraw(m_commandBuffer, 4, 1, m_vertexOffset, 0);
	m_vertexOffset += 4;
}

void SurfaceVulkan::drawPictureRepeat(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	VulkanTexture* tex = m_pictures[pictureId];
	if(!tex)
	{
		tex = loadPicture(pictureId, false);
		if(!tex)
			return;//load failed
	}

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);
	Uint32 drawTriangles = 0;
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

			m_vulkanVertices.emplace_back(minx, miny, minu, minv, texColor);
			m_vulkanVertices.emplace_back(minx, maxy, minu, maxv, texColor);
			m_vulkanVertices.emplace_back(maxx, miny, maxu, minv, texColor);

			m_vulkanVertices.emplace_back(maxx, maxy, maxu, maxv, texColor);
			m_vulkanVertices.emplace_back(maxx, miny, maxu, minv, texColor);
			m_vulkanVertices.emplace_back(minx, maxy, minu, maxv, texColor);

			drawTriangles += 6;
			cx += sw;
		}
		y += sh;
	}

	if(drawTriangles > 0)
	{
		setupGraphicPipeline(GRAPHIC_PIPELINE_TEXTURE_TRIANGLES);
		setupTextureRendering(tex);
		vkCmdDraw(m_commandBuffer, drawTriangles, 1, m_vertexOffset, 0);
		m_vertexOffset += drawTriangles;
	}
}

void SurfaceVulkan::drawPicture(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	VulkanTexture* tex = m_pictures[pictureId];
	if(!tex)
	{
		tex = loadPicture(pictureId, false);
		if(!tex)
			return;//load failed
	}

	setupGraphicPipeline(GRAPHIC_PIPELINE_TEXTURE);
	setupTextureRendering(tex);

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	float minu = sx*tex->m_scaleW;
	float maxu = (sx+w)*tex->m_scaleW;
	float minv = sy*tex->m_scaleH;
	float maxv = (sy+h)*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);
	m_vulkanVertices.emplace_back(minx, miny, minu, minv, texColor);
	m_vulkanVertices.emplace_back(minx, maxy, minu, maxv, texColor);
	m_vulkanVertices.emplace_back(maxx, miny, maxu, minv, texColor);
	m_vulkanVertices.emplace_back(maxx, maxy, maxu, maxv, texColor);

	vkCmdDraw(m_commandBuffer, 4, 1, m_vertexOffset, 0);
	m_vertexOffset += 4;
}

VulkanTexture* SurfaceVulkan::loadSpriteMask(Uint64 tempPos, Uint32 spriteId, Uint32 maskSpriteId, Uint32 outfitColor)
{
	unsigned char* pixels = g_engine.LoadSpriteMask(spriteId, maskSpriteId, outfitColor, true);
	if(!pixels)
		return NULL;

	if(m_spriteMasks.size() >= HARDWARE_MAX_SPRITEMASKS)
	{
		U64BImages::iterator it = m_spriteMasks.find(m_spritesMaskIds.front());
		if(it == m_spriteMasks.end())//Something weird happen - let's erase the first one entry
			it = m_spriteMasks.begin();

		m_texturesToDelete[m_currentFrame].push_back(it->second);
		m_spriteMasks.erase(it);
		m_spritesMaskIds.pop_front();
	}
	VulkanTexture* s = createVulkanTexture(32, 32, false);
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

void SurfaceVulkan::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor)
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
	VulkanTexture* tex;
	U64BImages::iterator it = m_spriteMasks.find(tempPos);
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

	setupGraphicPipeline(GRAPHIC_PIPELINE_TEXTURE);
	setupTextureRendering(tex);

	float minx = SDL_static_cast(float, x);
	float maxx = minx+SDL_static_cast(float, tex->m_width);
	float miny = SDL_static_cast(float, y);
	float maxy = miny+SDL_static_cast(float, tex->m_height);

	float minu = 0.0f;
	float maxu = tex->m_width*tex->m_scaleW;
	float minv = 0.0f;
	float maxv = tex->m_height*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);
	m_vulkanVertices.emplace_back(minx, miny, minu, minv, texColor);
	m_vulkanVertices.emplace_back(minx, maxy, minu, maxv, texColor);
	m_vulkanVertices.emplace_back(maxx, miny, maxu, minv, texColor);
	m_vulkanVertices.emplace_back(maxx, maxy, maxu, maxv, texColor);

	vkCmdDraw(m_commandBuffer, 4, 1, m_vertexOffset, 0);
	m_vertexOffset += 4;
}

void SurfaceVulkan::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Uint32 outfitColor)
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
	VulkanTexture* tex;
	U64BImages::iterator it = m_spriteMasks.find(tempPos);
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

	setupGraphicPipeline(GRAPHIC_PIPELINE_TEXTURE);
	setupTextureRendering(tex);

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	float minu = sx*tex->m_scaleW;
	float maxu = (sx+sw)*tex->m_scaleW;
	float minv = sy*tex->m_scaleH;
	float maxv = (sy+sh)*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);
	m_vulkanVertices.emplace_back(minx, miny, minu, minv, texColor);
	m_vulkanVertices.emplace_back(minx, maxy, minu, maxv, texColor);
	m_vulkanVertices.emplace_back(maxx, miny, maxu, minv, texColor);
	m_vulkanVertices.emplace_back(maxx, maxy, maxu, maxv, texColor);

	vkCmdDraw(m_commandBuffer, 4, 1, m_vertexOffset, 0);
	m_vertexOffset += 4;
}

VulkanTexture* SurfaceVulkan::createAutomapTile(Uint32 currentArea)
{
	if(m_automapTiles.size() >= HARDWARE_MAX_AUTOMAPTILES)
	{
		U32BImages::iterator it = m_automapTiles.find(m_automapTilesBuff.front());
		if(it == m_automapTiles.end())//Something weird happen - let's erase the first one entry
			it = m_automapTiles.begin();

		m_texturesToDelete[m_currentFrame].push_back(it->second);
		m_automapTiles.erase(it);
		m_automapTilesBuff.pop_front();
	}
	VulkanTexture* s = createVulkanTexture(256, 256, false);
	if(!s)
		return NULL;

	m_automapTiles[currentArea] = s;
	m_automapTilesBuff.push_back(currentArea);
	return s;
}

void SurfaceVulkan::uploadAutomapTile(VulkanTexture* texture, Uint8 color[256][256])
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
	vkQueueWaitIdle(m_queue);
	updateTextureData(texture, pixels);
	SDL_free(pixels);
}

void SurfaceVulkan::drawAutomapTile(Uint32 currentArea, bool& recreate, Uint8 color[256][256], Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
{
	VulkanTexture* tex;
	U32BImages::iterator it = m_automapTiles.find(currentArea);
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

	setupGraphicPipeline(GRAPHIC_PIPELINE_TEXTURE);
	setupTextureRendering(tex);

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	float minu = sx*tex->m_scaleW;
	float maxu = (sx+sw)*tex->m_scaleW;
	float minv = sy*tex->m_scaleW;
	float maxv = (sy+sh)*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);
	m_vulkanVertices.emplace_back(minx, miny, minu, minv, texColor);
	m_vulkanVertices.emplace_back(minx, maxy, minu, maxv, texColor);
	m_vulkanVertices.emplace_back(maxx, miny, maxu, minv, texColor);
	m_vulkanVertices.emplace_back(maxx, maxy, maxu, maxv, texColor);

	vkCmdDraw(m_commandBuffer, 4, 1, m_vertexOffset, 0);
	m_vertexOffset += 4;
}

SurfaceVulkanComp::SurfaceVulkanComp()
{
	m_sprites = NULL;
}

SurfaceVulkanComp::~SurfaceVulkanComp()
{
	if(m_sprites)
	{
		for(Uint32 i = 1; i <= m_spritesCache; ++i)
		{
			if(m_sprites[i])
				releaseVulkanTexture(m_sprites[i]);
		}

		SDL_free(m_sprites);
	}
}

void SurfaceVulkanComp::init()
{
	SurfaceVulkan::init();
	m_sprites = SDL_reinterpret_cast(VulkanTexture**, SDL_calloc(g_spriteCounts+1, sizeof(VulkanTexture*)));
	if(!m_sprites)
	{
		SDL_OutOfMemory();
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Surface::Init() Failed: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
	}
	m_spritesCache = g_spriteCounts;
}

void SurfaceVulkanComp::spriteManagerReset()
{
	SurfaceVulkan::spriteManagerReset();
	if(m_sprites)
	{
		for(Uint32 i = 1; i <= m_spritesCache; ++i)
		{
			if(m_sprites[i])
				releaseVulkanTexture(m_sprites[i]);
		}

		SDL_free(m_sprites);
	}
	m_sprites = SDL_reinterpret_cast(VulkanTexture**, SDL_calloc(g_spriteCounts+1, sizeof(VulkanTexture*)));
	if(!m_sprites)
	{
		SDL_OutOfMemory();
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Surface::spriteManagerReset() Failed: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
	}
	m_spritesCache = g_spriteCounts;
}

VulkanTexture* SurfaceVulkanComp::loadSprite(Uint32 spriteId)
{
	unsigned char* pixels = g_engine.LoadSprite(spriteId, true);
	if(!pixels)
		return NULL;

	VulkanTexture* s = createVulkanTexture(32, 32, false);
	if(!s)
	{
		SDL_free(pixels);
		return NULL;
	}
	updateTextureData(s, pixels);
	SDL_free(pixels);
	m_sprites[spriteId] = s;
	return s;
}

void SurfaceVulkanComp::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y)
{
	if(spriteId > g_spriteCounts)
		return;

	VulkanTexture* tex = m_sprites[spriteId];
	if(!tex)
	{
		tex = loadSprite(spriteId);
		if(!tex)
			return;//load failed
	}

	setupGraphicPipeline(GRAPHIC_PIPELINE_TEXTURE);
	setupTextureRendering(tex);

	float minx = SDL_static_cast(float, x);
	float maxx = minx+SDL_static_cast(float, tex->m_width);
	float miny = SDL_static_cast(float, y);
	float maxy = miny+SDL_static_cast(float, tex->m_height);

	float minu = 0.0f;
	float maxu = tex->m_width*tex->m_scaleW;
	float minv = 0.0f;
	float maxv = tex->m_height*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);
	m_vulkanVertices.emplace_back(minx, miny, minu, minv, texColor);
	m_vulkanVertices.emplace_back(minx, maxy, minu, maxv, texColor);
	m_vulkanVertices.emplace_back(maxx, miny, maxu, minv, texColor);
	m_vulkanVertices.emplace_back(maxx, maxy, maxu, maxv, texColor);

	vkCmdDraw(m_commandBuffer, 4, 1, m_vertexOffset, 0);
	m_vertexOffset += 4;
}

void SurfaceVulkanComp::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
{
	if(spriteId > g_spriteCounts)
		return;

	VulkanTexture* tex = m_sprites[spriteId];
	if(!tex)
	{
		tex = loadSprite(spriteId);
		if(!tex)
			return;//load failed
	}

	setupGraphicPipeline(GRAPHIC_PIPELINE_TEXTURE);
	setupTextureRendering(tex);

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x+w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y+h);

	float minu = sx*tex->m_scaleW;
	float maxu = (sx+sw)*tex->m_scaleW;
	float minv = sy*tex->m_scaleH;
	float maxv = (sy+sh)*tex->m_scaleH;

	DWORD texColor = MAKE_RGBA_COLOR(255, 255, 255, 255);
	m_vulkanVertices.emplace_back(minx, miny, minu, minv, texColor);
	m_vulkanVertices.emplace_back(minx, maxy, minu, maxv, texColor);
	m_vulkanVertices.emplace_back(maxx, miny, maxu, minv, texColor);
	m_vulkanVertices.emplace_back(maxx, maxy, maxu, maxv, texColor);

	vkCmdDraw(m_commandBuffer, 4, 1, m_vertexOffset, 0);
	m_vertexOffset += 4;
}

SurfaceVulkanPerf::SurfaceVulkanPerf()
{
	m_sprites = NULL;
	m_spriteAtlas = NULL;

	m_spriteAtlases = 0;
	m_spritesPerAtlas = 0;
	m_spritesPerModulo = 0;

	m_stagingBuffer = VK_NULL_HANDLE;
	m_stagingBufferMemory = VK_NULL_HANDLE;

	m_gameVertices = 0;
}

SurfaceVulkanPerf::~SurfaceVulkanPerf()
{
	if(m_sprites)
		SDL_free(m_sprites);

	for(std::vector<VulkanTexture*>::iterator it = m_spritesAtlas.begin(), end = m_spritesAtlas.end(); it != end; ++it)
		releaseVulkanTexture((*it));

	m_spritesAtlas.clear();
	if(m_stagingBufferMemory)
		vkFreeMemory(m_device, m_stagingBufferMemory, NULL);

	if(m_stagingBuffer)
		vkDestroyBuffer(m_device, m_stagingBuffer, NULL);
}

void SurfaceVulkanPerf::generateSpriteAtlases()
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
	for(std::vector<VulkanTexture*>::iterator it = m_spritesAtlas.begin(), end = m_spritesAtlas.end(); it != end; ++it)
		releaseVulkanTexture((*it));

	m_spritesAtlas.clear();
	for(Uint32 i = 0; i < m_spriteAtlases; ++i)
	{
		VulkanTexture* texture = createVulkanTexture(m_spritesPerModulo, m_spritesPerModulo, false);
		if(!texture)
		{
			UTIL_MessageBox(true, "Vulkan: Out of video memory.");
			exit(-1);
			return;
		}
		m_spritesAtlas.push_back(texture);
	}
}

void SurfaceVulkanPerf::checkScheduledSprites()
{
	if(m_gameVertices > 0)
	{
		setupGraphicPipeline(GRAPHIC_PIPELINE_TEXTURE_TRIANGLES);
		setupTextureRendering(m_spriteAtlas);

		vkCmdDraw(m_commandBuffer, m_gameVertices, 1, m_vertexOffset, 0);
		m_vertexOffset += m_gameVertices;
		m_gameVertices = 0;
	}
}

void SurfaceVulkanPerf::init()
{
	SurfaceVulkan::init();
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

void SurfaceVulkanPerf::spriteManagerReset()
{
	SurfaceVulkan::spriteManagerReset();
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

void SurfaceVulkanPerf::beginScene()
{
	SurfaceVulkan::beginScene();
	m_scheduleSpriteDraw = false;
}

void SurfaceVulkanPerf::beginGameScene()
{
	SurfaceVulkan::beginGameScene();
	m_gameVertices = 0;
	m_scheduleSpriteDraw = true;
}

void SurfaceVulkanPerf::endGameScene()
{
	checkScheduledSprites();
	m_scheduleSpriteDraw = false;
	SurfaceVulkan::endGameScene();
}

void SurfaceVulkanPerf::drawRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(m_scheduleSpriteDraw)
		checkScheduledSprites();
	SurfaceVulkan::drawRectangle(x, y, w, h, r, g, b, a);
}

void SurfaceVulkanPerf::fillRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(m_scheduleSpriteDraw)
		checkScheduledSprites();
	SurfaceVulkan::fillRectangle(x, y, w, h, r, g, b, a);
}

bool SurfaceVulkanPerf::loadSprite(Uint32 spriteId, VulkanTexture* texture, Uint32 xoff, Uint32 yoff)
{
	static const VkDeviceSize imageSize = (32*32*4);
	if(!m_stagingBuffer)
	{
		VkBufferCreateInfo bufferInfo;
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.pNext = NULL;
		bufferInfo.flags = 0;
		bufferInfo.size = imageSize;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.queueFamilyIndexCount = 0;
		bufferInfo.pQueueFamilyIndices = NULL;
		if(vkCreateBuffer(m_device, &bufferInfo, NULL, &m_stagingBuffer) != VK_SUCCESS)
			return false;

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_device, m_stagingBuffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = NULL;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
		if(vkAllocateMemory(m_device, &allocInfo, NULL, &m_stagingBufferMemory) != VK_SUCCESS)
		{
			vkDestroyBuffer(m_device, m_stagingBuffer, NULL);
			m_stagingBuffer = NULL;
			return false;
		}
		vkBindBufferMemory(m_device, m_stagingBuffer, m_stagingBufferMemory, 0);
	}

	unsigned char* pixels = g_engine.LoadSprite(spriteId, true);
	if(!pixels)
		return false;

	VkCommandBufferBeginInfo beginInfo;
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = NULL;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = NULL;
	vkResetCommandBuffer(m_commandDataBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	vkBeginCommandBuffer(m_commandDataBuffer, &beginInfo);

	void* data;
	vkMapMemory(m_device, m_stagingBufferMemory, 0, imageSize, 0, &data);
	UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, data), SDL_reinterpret_cast(const Uint8*, pixels), SDL_static_cast(size_t, imageSize));
	vkUnmapMemory(m_device, m_stagingBufferMemory);

	VkImageMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = NULL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.image = texture->m_textureImage;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	vkCmdPipelineBarrier(m_commandDataBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

	VkBufferImageCopy region;
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = {SDL_static_cast(Sint32, xoff), SDL_static_cast(Sint32, yoff), 0};
	region.imageExtent = {32, 32, 1};
	vkCmdCopyBufferToImage(m_commandDataBuffer, m_stagingBuffer, texture->m_textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(m_commandDataBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = NULL;
	submitInfo.pWaitDstStageMask = NULL;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandDataBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = NULL;
	vkEndCommandBuffer(m_commandDataBuffer);
	vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_queue);
	SDL_free(pixels);
	return true;
}

void SurfaceVulkanPerf::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y)
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

	VulkanTexture* tex = m_spritesAtlas[spriteAtlas];
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
	if(m_scheduleSpriteDraw)
	{
		if(tex != m_spriteAtlas)
		{
			checkScheduledSprites();
			m_spriteAtlas = tex;
		}

		m_vulkanVertices.emplace_back(minx, miny, minu, minv, texColor);
		m_vulkanVertices.emplace_back(minx, maxy, minu, maxv, texColor);
		m_vulkanVertices.emplace_back(maxx, miny, maxu, minv, texColor);

		m_vulkanVertices.emplace_back(maxx, maxy, maxu, maxv, texColor);
		m_vulkanVertices.emplace_back(maxx, miny, maxu, minv, texColor);
		m_vulkanVertices.emplace_back(minx, maxy, minu, maxv, texColor);

		m_gameVertices += 6;
	}
	else
	{
		setupGraphicPipeline(GRAPHIC_PIPELINE_TEXTURE);
		setupTextureRendering(tex);

		m_vulkanVertices.emplace_back(minx, miny, minu, minv, texColor);
		m_vulkanVertices.emplace_back(minx, maxy, minu, maxv, texColor);
		m_vulkanVertices.emplace_back(maxx, miny, maxu, minv, texColor);
		m_vulkanVertices.emplace_back(maxx, maxy, maxu, maxv, texColor);

		vkCmdDraw(m_commandBuffer, 4, 1, m_vertexOffset, 0);
		m_vertexOffset += 4;
	}
}

void SurfaceVulkanPerf::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
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

	VulkanTexture* tex = m_spritesAtlas[spriteAtlas];
	if(!m_sprites[spriteId])
	{
		if(!loadSprite(spriteId, tex, xOffset, yOffset))
			return;//load failed
		m_sprites[spriteId] = true;
	}

	setupGraphicPipeline(GRAPHIC_PIPELINE_TEXTURE);
	setupTextureRendering(tex);

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
	m_vulkanVertices.emplace_back(minx, miny, minu, minv, texColor);
	m_vulkanVertices.emplace_back(minx, maxy, minu, maxv, texColor);
	m_vulkanVertices.emplace_back(maxx, miny, maxu, minv, texColor);
	m_vulkanVertices.emplace_back(maxx, maxy, maxu, maxv, texColor);

	vkCmdDraw(m_commandBuffer, 4, 1, m_vertexOffset, 0);
	m_vertexOffset += 4;
}

void SurfaceVulkanPerf::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor)
{
	if(m_scheduleSpriteDraw)
		checkScheduledSprites();
	SurfaceVulkan::drawSpriteMask(spriteId, maskSpriteId, x, y, outfitColor);
}
#endif
