#pragma once
#include "VkHandler.h"

namespace vi
{
	class VkShaderHandler final : VkHandler
	{
		friend VkCore;

	public:
		void Draw(uint32_t indexCount) const;

		[[nodiscard]] VkShaderModule CreateModule(const String& data) const;
		void DestroyModule(VkShaderModule module) const;

		[[nodiscard]] VkSampler CreateSampler(VkFilter magFilter = VK_FILTER_LINEAR,
			VkFilter minFilter = VK_FILTER_LINEAR, VkBorderColor borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
			VkSamplerAddressMode adressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT) const;
		void BindSampler(VkDescriptorSet set, VkImageView imageView, VkImageLayout layout, VkSampler sampler, uint32_t bindingIndex, uint32_t arrayIndex) const;
		void DestroySampler(VkSampler sampler) const;

		[[nodiscard]] VkBuffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags flags) const;
		void BindVertexBuffer(VkBuffer buffer) const;
		void BindIndicesBuffer(VkBuffer buffer) const;
		void BindBuffer(VkDescriptorSet set, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t bindingIndex, uint32_t arrayIndex) const;
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0) const;
		void CopyBuffer(VkBuffer srcBuffer, VkImage dstImage, glm::ivec2 resolution) const;
		void DestroyBuffer(VkBuffer buffer) const;

		template <typename T>
		void UpdatePushConstant(VkPipelineLayout layout, VkFlags flag, const T& input);

	private:
		explicit VkShaderHandler(VkCore& core);
	};
}
