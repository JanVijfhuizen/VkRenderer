#pragma once
#include "VkHandler.h"

namespace vi
{
	class VkPipelineHandler final : public VkHandler
	{
		friend VkCore;

	public:
		struct Info final
		{
			struct Module final
			{
				VkShaderModule module;
				VkShaderStageFlagBits flags;
			};

			struct PushConstant final
			{
				size_t size;
				VkShaderStageFlags flag;
			};

			VkRenderPass renderPass;
			Vector<Module> modules{2, GMEM_TEMP};
			Vector<VkDescriptorSetLayout> setLayouts{4, GMEM_TEMP};
			Vector<PushConstant> pushConstants{1, GMEM_TEMP};

			VkVertexInputBindingDescription bindingDescription;
			Vector<VkVertexInputAttributeDescription> attributeDescriptions{4, GMEM_TEMP};

			VkPrimitiveTopology primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			VkBool32 primitiveRestartEnable = VK_FALSE;
			VkExtent2D extent;
			VkBool32 depthClampEnable = VK_FALSE;
			VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
			float lineWidth = 1;
			VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
			VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			bool depthBufferEnabled = true;
			VkCompareOp depthBufferCompareOp = VK_COMPARE_OP_LESS;

			VkPipeline basePipeline = VK_NULL_HANDLE;
			int32_t basePipelineIndex = -1;
		};

		void Create(const Info& info, VkPipeline& outPipeline, VkPipelineLayout& outLayout) const;
		void Bind(VkPipeline pipeline, VkPipelineLayout layout);
		void Destroy(VkPipeline pipeline, VkPipelineLayout layout) const;

		[[nodiscard]] VkPipeline GetCurrent() const;
		[[nodiscard]] VkPipelineLayout GetCurrentLayout() const;

	private:
		VkPipeline _current;
		VkPipelineLayout _currentLayout;

		explicit VkPipelineHandler(VkCore& core);
	};
}
