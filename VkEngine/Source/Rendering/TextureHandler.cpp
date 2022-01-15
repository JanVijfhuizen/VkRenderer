#include "pch.h"
#include "Rendering/TextureHandler.h"
#include "VkRenderer/VkCore/VkCore.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

TextureHandler::TextureHandler(vi::VkCore& core) : VkHandler(core)
{
}

Texture TextureHandler::Create(const char* name, const char* extension) const
{
	auto& commandBufferHandler = core.GetCommandBufferHandler();
	auto& imageHandler = core.GetImageHandler();
	auto& memoryHandler = core.GetMemoryHandler();
	auto& shaderHandler = core.GetShaderHandler();
	auto& syncHandler = core.GetSyncHandler();

	vi::String path{ "Textures/", GMEM_TEMP };
	vi::String postFix{ extension, GMEM_TEMP };

	path.Append(name);
	path.Append(".");
	path.Append(extension);

	int32_t w, h, d;
	const auto tex = Load(path.GetData(), w, h, d);
	const uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(w, h)))) + 1;

	const auto texStagingBuffer = shaderHandler.CreateBuffer(sizeof(unsigned char) * w * h * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	const auto texStagingMem = memoryHandler.Allocate(texStagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	memoryHandler.Bind(texStagingBuffer, texStagingMem);
	memoryHandler.Map(texStagingMem, tex, 0, w * h * d);

	const auto img = imageHandler.Create({ w, h }, mipLevels, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, 
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	const auto imgMem = memoryHandler.Allocate(img, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	memoryHandler.Bind(img, imgMem);

	auto imgCmd = commandBufferHandler.Create();
	commandBufferHandler.BeginRecording(imgCmd);
	imageHandler.TransitionLayout(img, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
	commandBufferHandler.EndRecording();

	const auto imgFence = syncHandler.CreateFence();

	vi::VkCommandBufferHandler::SubmitInfo submitInfo{};
	submitInfo.buffers = &imgCmd;
	submitInfo.buffersCount = 1;
	submitInfo.fence = imgFence;
	commandBufferHandler.Submit(submitInfo);
	syncHandler.WaitForFence(imgFence);

	commandBufferHandler.BeginRecording(imgCmd);
	shaderHandler.CopyBuffer(texStagingBuffer, img, {w, h});
	commandBufferHandler.EndRecording();
	commandBufferHandler.Submit(submitInfo);
	syncHandler.WaitForFence(imgFence);

	syncHandler.DestroyFence(imgFence);
	commandBufferHandler.Destroy(imgCmd);
	shaderHandler.DestroyBuffer(texStagingBuffer);
	memoryHandler.Free(texStagingMem);

	Free(tex);

	GenerateMipMaps(img, { w, h }, mipLevels, VK_FORMAT_R8G8B8A8_SRGB);

	Texture texture{};
	texture.resolution = { w, h };
	texture.channels = d;
	texture.mipLevels = mipLevels;
	texture.image = img;
	texture.memory = imgMem;
	texture.imageView = imageHandler.CreateView(img, mipLevels);

	return texture;
}

void TextureHandler::Destroy(const Texture& texture) const
{
	auto& imageHandler = core.GetImageHandler();
	auto& memoryHandler = core.GetMemoryHandler();

	imageHandler.DestroyView(texture.imageView);
	imageHandler.Destroy(texture.image);
	memoryHandler.Free(texture.memory);
}

void TextureHandler::GenerateMipMaps(
	const VkImage image, 
	const glm::ivec2 resolution,
	const uint32_t mipLevels,
	const VkFormat imageFormat) const
{
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(core.GetPhysicalDevice(), imageFormat, &formatProperties);
	assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT);

	auto& commandBufferHandler = core.GetCommandBufferHandler();
	auto& syncHandler = core.GetSyncHandler();

	auto commandBuffer = commandBufferHandler.Create();
	commandBufferHandler.BeginRecording(commandBuffer);

	const auto fence = syncHandler.CreateFence();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = resolution.x;
	int32_t mipHeight = resolution.y;

	for (uint32_t i = 1; i < mipLevels; i++) 
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		mipWidth = mipWidth == 1 ? mipWidth : mipWidth / 2;
		mipHeight = mipHeight == 1 ? mipHeight : mipHeight / 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	commandBufferHandler.EndRecording();

	vi::VkCommandBufferHandler::SubmitInfo submitInfo{};
	submitInfo.buffers = &commandBuffer;
	submitInfo.buffersCount = 1;
	submitInfo.fence = fence;
	commandBufferHandler.Submit(submitInfo);
	syncHandler.WaitForFence(fence);

	commandBufferHandler.Destroy(commandBuffer);
	syncHandler.DestroyFence(fence);
}

unsigned char* TextureHandler::Load(const std::string& path, int32_t& width, int32_t& height, int32_t& numChannels)
{
	const auto pixels = stbi_load(path.c_str(), &width, &height, &numChannels, STBI_rgb_alpha);
	assert(pixels);
	return pixels;
}

void TextureHandler::Free(unsigned char* pixels)
{
	stbi_image_free(pixels);
}
