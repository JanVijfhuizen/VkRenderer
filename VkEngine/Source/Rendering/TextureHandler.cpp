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
	const auto texStagingBuffer = shaderHandler.CreateBuffer(sizeof(unsigned char) * w * h * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	const auto texStagingMem = memoryHandler.Allocate(texStagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	memoryHandler.Bind(texStagingBuffer, texStagingMem);
	memoryHandler.Map(texStagingMem, tex, 0, w * h * d);

	const auto img = imageHandler.Create({ w, h });
	const auto imgMem = memoryHandler.Allocate(img, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	memoryHandler.Bind(img, imgMem);

	auto imgCmd = commandBufferHandler.Create();
	commandBufferHandler.BeginRecording(imgCmd);
	imageHandler.TransitionLayout(img, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	commandBufferHandler.EndRecording();

	const auto imgFence = syncHandler.CreateFence();
	commandBufferHandler.Submit(&imgCmd, 1, VK_NULL_HANDLE, VK_NULL_HANDLE, imgFence);
	syncHandler.WaitForFence(imgFence);

	commandBufferHandler.BeginRecording(imgCmd);
	shaderHandler.CopyBuffer(texStagingBuffer, img, {w, h});
	commandBufferHandler.EndRecording();
	commandBufferHandler.Submit(&imgCmd, 1, VK_NULL_HANDLE, VK_NULL_HANDLE, imgFence);
	syncHandler.WaitForFence(imgFence);

	commandBufferHandler.BeginRecording(imgCmd);
	imageHandler.TransitionLayout(img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	commandBufferHandler.EndRecording();
	commandBufferHandler.Submit(&imgCmd, 1, VK_NULL_HANDLE, VK_NULL_HANDLE, imgFence);
	syncHandler.WaitForFence(imgFence);

	syncHandler.DestroyFence(imgFence);
	commandBufferHandler.Destroy(imgCmd);
	shaderHandler.DestroyBuffer(texStagingBuffer);
	memoryHandler.Free(texStagingMem);

	Free(tex);

	Texture texture{};
	texture.resolution = { w, h };
	texture.channels = d;
	texture.image = img;
	texture.memory = imgMem;
	texture.imageView = imageHandler.CreateView(img);

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
