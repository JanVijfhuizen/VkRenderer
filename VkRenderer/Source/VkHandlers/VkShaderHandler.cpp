#include "pch.h"
#include "VkHandlers/VkShaderHandler.h"
#include "VkCore/VkCore.h"

namespace vi
{
	void VkShaderHandler::Draw(const uint32_t indexCount) const
	{
		vkCmdDrawIndexed(core.GetCommandBufferHandler().GetCurrent(), indexCount, 1, 0, 0, 0);
	}

	VkShaderModule VkShaderHandler::CreateModule(const String& data) const
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = data.GetLength();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(data.GetData());

		VkShaderModule vkModule;
		const auto result = vkCreateShaderModule(core.GetLogicalDevice(), &createInfo, nullptr, &vkModule);
		assert(!result);
		return vkModule;
	}

	void VkShaderHandler::DestroyModule(const VkShaderModule module) const
	{
		vkDestroyShaderModule(core.GetLogicalDevice(), module, nullptr);
	}

	VkSampler VkShaderHandler::CreateSampler(
		const VkFilter magFilter, 
		const VkFilter minFilter, 
		const VkBorderColor borderColor,
		const VkSamplerAddressMode adressMode) const
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(core.GetPhysicalDevice(), &properties);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = magFilter;
		samplerInfo.minFilter = minFilter;
		samplerInfo.addressModeU = adressMode;
		samplerInfo.addressModeV = adressMode;
		samplerInfo.addressModeW = adressMode;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = borderColor;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		VkSampler sampler;
		const auto result = vkCreateSampler(core.GetLogicalDevice(), &samplerInfo, nullptr, &sampler);
		assert(!result);
		return sampler;
	}

	void VkShaderHandler::BindSampler(
		const VkDescriptorSet set, 
		const VkImageView imageView, 
		const VkImageLayout layout,
		const VkSampler sampler, 
		const uint32_t bindingIndex, 
		const uint32_t arrayIndex) const
	{
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = layout;
		imageInfo.imageView = imageView;
		imageInfo.sampler = sampler;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = set;
		descriptorWrite.dstBinding = bindingIndex;
		descriptorWrite.dstArrayElement = arrayIndex;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(core.GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
	}

	void VkShaderHandler::DestroySampler(const VkSampler sampler) const
	{
		vkDestroySampler(core.GetLogicalDevice(), sampler, nullptr);
	}

	VkBuffer VkShaderHandler::CreateBuffer(const VkDeviceSize size, const VkBufferUsageFlags flags) const
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = flags;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkBuffer vertexBuffer;
		const auto result = vkCreateBuffer(core.GetLogicalDevice(), &bufferInfo, nullptr, &vertexBuffer);
		assert(!result);
		return vertexBuffer;
	}

	void VkShaderHandler::BindVertexBuffer(const VkBuffer buffer) const
	{
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(core.GetCommandBufferHandler().GetCurrent(), 0, 1, &buffer, &offset);
	}

	void VkShaderHandler::BindIndicesBuffer(const VkBuffer buffer) const
	{
		vkCmdBindIndexBuffer(core.GetCommandBufferHandler().GetCurrent(), buffer, 0, VK_INDEX_TYPE_UINT16);
	}

	void VkShaderHandler::BindBuffer(
		const VkDescriptorSet set, 
		const VkBuffer buffer, 
		const VkDeviceSize offset, 
		const VkDeviceSize range,
		const uint32_t bindingIndex, 
		const uint32_t arrayIndex) const
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = buffer;
		bufferInfo.offset = offset;
		bufferInfo.range = range;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = set;
		descriptorWrite.dstBinding = bindingIndex;
		descriptorWrite.dstArrayElement = arrayIndex;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(core.GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
	}

	void VkShaderHandler::CopyBuffer(
		const VkBuffer srcBuffer, 
		const VkBuffer dstBuffer, 
		const VkDeviceSize size, 
		const VkDeviceSize srcOffset,
		const VkDeviceSize dstOffset) const
	{
		VkBufferCopy region{};
		region.srcOffset = srcOffset;
		region.dstOffset = dstOffset;
		region.size = size;
		vkCmdCopyBuffer(core.GetCommandBufferHandler().GetCurrent(), srcBuffer, dstBuffer, 1, &region);
	}

	void VkShaderHandler::CopyBuffer(
		const VkBuffer srcBuffer, 
		const VkImage dstImage, 
		const glm::ivec2 resolution) const
	{
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent =
		{
			static_cast<uint32_t>(resolution.x),
			static_cast<uint32_t>(resolution.y),
			1
		};

		auto& subResource = region.imageSubresource;
		subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subResource.mipLevel = 0;
		subResource.baseArrayLayer = 0;
		subResource.layerCount = 1;

		vkCmdCopyBufferToImage(core.GetCommandBufferHandler().GetCurrent(), srcBuffer, dstImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}

	void VkShaderHandler::DestroyBuffer(const VkBuffer buffer) const
	{
		vkDestroyBuffer(core.GetLogicalDevice(), buffer, nullptr);
	}

	VkShaderHandler::VkShaderHandler(VkCore& core) : VkHandler(core)
	{

	}

	void VkShaderHandler::IntUpdatePushConstant(const VkPipelineLayout layout, 
		const VkFlags flag, void const * input, const size_t size)
	{
		vkCmdPushConstants(core.GetCommandBufferHandler().GetCurrent(), layout, flag, 0, size, input);
	}
}
