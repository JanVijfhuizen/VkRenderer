#include "pch.h"
#include "Rendering/Texture.h"
#include "Rendering/RenderSystem.h"
#include "VkRenderer/VkRenderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Texture::Manager::Manager()
{
	// ReSharper disable once CppNoDiscardExpression
	CreateTexture("Test.png");
}

Texture::Manager::~Manager()
{
	auto& renderSystem = RenderManager::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	for (auto& data : _data)
	{
		renderer.DestroyImageView(data.imageView);
		renderer.DestroyImage(data.image);
		renderer.FreeMemory(data.imageMemory);
	}
}

uint32_t Texture::Manager::CreateTexture(const std::string& fileName)
{
	auto& renderSystem = RenderManager::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	int32_t w, h, d;
	const auto tex = Load("Textures/" + fileName, w, h, d);
	const auto texStagingBuffer = renderer.CreateBuffer(sizeof(unsigned char) * w * h * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	const auto texStagingMem = renderer.AllocateMemory(texStagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	renderer.BindMemory(texStagingBuffer, texStagingMem);
	renderer.MapMemory(texStagingMem, tex, 0, w * h * 4);
	Free(tex);

	const auto img = renderer.CreateImage({ w, h });
	const auto imgMem = renderer.AllocateMemory(img, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	renderer.BindMemory(img, imgMem);

	auto imgCmd = renderer.CreateCommandBuffer();
	renderer.BeginCommandBufferRecording(imgCmd);
	renderer.TransitionImageLayout(img, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	renderer.EndCommandBufferRecording();

	const auto imgFence = renderer.CreateFence();
	renderer.Submit(&imgCmd, 1, VK_NULL_HANDLE, VK_NULL_HANDLE, imgFence);
	renderer.WaitForFence(imgFence);

	renderer.BeginCommandBufferRecording(imgCmd);
	renderer.CopyBuffer(texStagingBuffer, img, w, h);
	renderer.EndCommandBufferRecording();
	renderer.Submit(&imgCmd, 1, VK_NULL_HANDLE, VK_NULL_HANDLE, imgFence);
	renderer.WaitForFence(imgFence);

	renderer.BeginCommandBufferRecording(imgCmd);
	renderer.TransitionImageLayout(img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	renderer.EndCommandBufferRecording();
	renderer.Submit(&imgCmd, 1, VK_NULL_HANDLE, VK_NULL_HANDLE, imgFence);
	renderer.WaitForFence(imgFence);

	renderer.DestroyFence(imgFence);
	renderer.DestroyCommandBuffer(imgCmd);
	renderer.DestroyBuffer(texStagingBuffer);
	renderer.FreeMemory(texStagingMem);

	const auto imgView = renderer.CreateImageView(img);

	Texture texture{};
	texture.resolution = { w, h };
	texture.channels = d;
	texture.image = img;
	texture.imageMemory = imgMem;
	texture.imageView = imgView;
	_data.Add(texture);

	return _data.GetCount() - 1;
}

const Texture& Texture::Manager::GetData(const uint32_t handle)
{
	return _data[handle];
}

unsigned char* Texture::Manager::Load(const std::string& path, int32_t& width, int32_t& height, int32_t& numChannels)
{
	const auto pixels = stbi_load(path.c_str(), &width, &height, &numChannels, STBI_rgb_alpha);
	assert(pixels);
	return pixels;
}

void Texture::Manager::Free(unsigned char* pixels)
{
	stbi_image_free(pixels);
}
