#pragma once
#include "VkHandler.h"

namespace vi
{
	class VkShaderHandler final : VkHandler
	{
	public:
		struct SamplerCreateInfo final
		{
			float minLod = 0;
			float maxLod = 0;
			VkFilter minFilter = VK_FILTER_LINEAR;
			VkFilter maxFilter = VK_FILTER_LINEAR;
			VkBorderColor borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			VkSamplerAddressMode adressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		};

		explicit VkShaderHandler(VkCore& core);

		/// <summary> Draws a list of vertices based on the given pipeline.</summary>
		void Draw(uint32_t indexCount) const;

		/// <returns>Shader module based on compiled spv file.</returns>
		[[nodiscard]] VkShaderModule CreateModule(const String& data) const;
		void DestroyModule(VkShaderModule module) const;

		/// <returns>Object that can be used to use images as attachments during shader stages.</returns>
		[[nodiscard]] VkSampler CreateSampler(const SamplerCreateInfo& info = {}) const;
		void BindSampler(VkDescriptorSet set, VkImageView imageView, VkImageLayout layout, VkSampler sampler, uint32_t bindingIndex, uint32_t arrayIndex) const;
		void DestroySampler(VkSampler sampler) const;

		/// <returns>Object that can be used to attach memory data during shader stages.</returns>
		[[nodiscard]] VkBuffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags flags) const;
		void BindVertexBuffer(VkBuffer buffer) const;
		void BindIndicesBuffer(VkBuffer buffer) const;
		void BindBuffer(VkDescriptorSet set, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t bindingIndex, uint32_t arrayIndex) const;
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0) const;
		void CopyBuffer(VkBuffer srcBuffer, VkImage dstImage, glm::ivec2 resolution) const;
		void DestroyBuffer(VkBuffer buffer) const;

		/// <summary>
		/// Very cheap but limited way to transfer memory to the GPU.
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="layout">Target pipeline layout.</param>
		/// <param name="flag">Target shader stage(s).</param>
		/// <param name="input">Data to transfer tot the GPU.</param>
		template <typename T>
		void UpdatePushConstant(VkPipelineLayout layout, VkFlags flag, const T& input);

	private:
		void IntUpdatePushConstant(VkPipelineLayout layout, VkFlags flag, void const* input, size_t size);
	};

	template <typename T>
	void VkShaderHandler::UpdatePushConstant(
		const VkPipelineLayout layout,
		const VkFlags flag, const T& input)
	{
		IntUpdatePushConstant(layout, flag, &input, sizeof(T));
	}
}
