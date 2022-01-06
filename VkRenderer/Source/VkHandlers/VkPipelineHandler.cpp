#include "pch.h"
#include "VkHandlers/VkPipelineHandler.h"
#include "VkCore/VkCore.h"

namespace vi
{
	void VkPipelineHandler::Create(const Info& info, 
		VkPipeline& outPipeline, VkPipelineLayout& outLayout) const
	{
		const auto logicalDevice = core.GetLogicalDevice();
		const uint32_t modulesCount = info.modules.GetCount();
		const ArrayPtr<VkPipelineShaderStageCreateInfo> modules{ modulesCount, GMEM_TEMP };

		for (uint32_t i = 0; i < modulesCount; ++i)
		{
			const auto& moduleInfo = info.modules[i];
			auto& vertShaderStageInfo = modules[i];

			vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageInfo.stage = moduleInfo.flags;
			vertShaderStageInfo.module = moduleInfo.module;
			vertShaderStageInfo.pName = "main";
		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(info.attributeDescriptions.GetCount());
		vertexInputInfo.pVertexBindingDescriptions = &info.bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = info.attributeDescriptions.GetData();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = info.primitiveTopology;
		inputAssembly.primitiveRestartEnable = info.primitiveRestartEnable;

		VkViewport viewport{};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = static_cast<float>(info.extent.width);
		viewport.height = static_cast<float>(info.extent.height);
		viewport.minDepth = 0;
		viewport.maxDepth = 1;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = info.extent;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = info.depthClampEnable;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = info.polygonMode;
		rasterizer.lineWidth = info.lineWidth;
		rasterizer.cullMode = info.cullMode;
		rasterizer.frontFace = info.frontFace;
		rasterizer.depthBiasEnable = VK_FALSE;

		// Todo multisampling.
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 0;
		dynamicState.pDynamicStates = nullptr;

		const uint32_t pushConstantRangeCount = info.pushConstants.GetCount();
		const ArrayPtr<VkPushConstantRange> pushConstantRanges{ pushConstantRangeCount, GMEM_TEMP };

		for (uint32_t i = 0; i < pushConstantRangeCount; ++i)
		{
			auto& original = info.pushConstants[i];
			auto& pushConstantRange = pushConstantRanges[i];

			pushConstantRange.offset = 0;
			pushConstantRange.size = original.size;
			pushConstantRange.stageFlags = original.flag;
		}

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = info.depthBufferCompareOp;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = info.setLayouts.GetCount();
		pipelineLayoutInfo.pSetLayouts = info.setLayouts.GetData();
		pipelineLayoutInfo.pushConstantRangeCount = info.pushConstants.GetCount();
		pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.GetData();

		auto result = vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &outLayout);
		assert(!result);

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = modulesCount;
		pipelineInfo.pStages = modules.GetData();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = info.depthBufferEnabled ? &depthStencil : nullptr;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr;
		pipelineInfo.layout = outLayout;
		pipelineInfo.renderPass = info.renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = info.basePipeline;
		pipelineInfo.basePipelineIndex = info.basePipelineIndex;

		result = vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &outPipeline);
		assert(!result);
	}

	void VkPipelineHandler::Bind(const VkPipeline pipeline, const VkPipelineLayout layout)
	{
		_current = pipeline;
		_currentLayout = layout;
		vkCmdBindPipeline(core.GetCommandBufferHandler().GetCurrent(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}

	void VkPipelineHandler::Destroy(const VkPipeline pipeline, const VkPipelineLayout layout) const
	{
		const auto logicalDevice = core.GetLogicalDevice();
		vkDestroyPipeline(logicalDevice, pipeline, nullptr);
		vkDestroyPipelineLayout(logicalDevice, layout, nullptr);
	}

	VkPipeline VkPipelineHandler::GetCurrent() const
	{
		return _current;
	}

	VkPipelineLayout VkPipelineHandler::GetCurrentLayout() const
	{
		return _currentLayout;
	}

	VkPipelineHandler::VkPipelineHandler(VkCore& core) : VkHandler(core)
	{

	}
}
