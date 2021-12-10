﻿#include "pch.h"
#include "VkRenderer.h"
#include "InstanceFactory.h"
#include "WindowHandler.h"
#include "PhysicalDeviceFactory.h"
#include "LogicalDeviceFactory.h"
#include "SwapChain.h"

namespace vi
{
	VkRenderer::VkRenderer(const Settings& settings) : _windowHandler(settings.windowHandler)
	{
		Debugger::Info info;
		info.settings = settings.debugger;
		info.instance = &_instance;
		_debugger = new Debugger(info);

		InstanceFactory::Info instanceInfo;
		instanceInfo.settings = settings.instance;
		instanceInfo.debugger = _debugger;
		instanceInfo.windowHandler = _windowHandler;
		_instance = InstanceFactory::Construct(instanceInfo);

		_debugger->CreateDebugMessenger();

		_surface = _windowHandler->CreateSurface(_instance);

		PhysicalDeviceFactory::Info physicalDeviceInfo;
		physicalDeviceInfo.instance = _instance;
		physicalDeviceInfo.surface = _surface;
		physicalDeviceInfo.deviceExtensions = settings.deviceExtensions;
		_physicalDevice = PhysicalDeviceFactory::Construct(physicalDeviceInfo);

		LogicalDeviceFactory::Info logicalDeviceInfo;
		logicalDeviceInfo.surface = _surface;
		logicalDeviceInfo.physicalDevice = _physicalDevice;
		logicalDeviceInfo.deviceExtensions = settings.deviceExtensions;
		logicalDeviceInfo.validationLayers = settings.debugger.validationLayers;
		logicalDeviceInfo.debugger = _debugger;
		const auto outLogicalDeviceFactory = LogicalDeviceFactory::Construct(logicalDeviceInfo);
		_device = outLogicalDeviceFactory.device;
		_queues = outLogicalDeviceFactory.queues;

		const auto families = PhysicalDeviceFactory::GetQueueFamilies(_surface, _physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = families.graphics;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		const auto result = vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool);
		assert(!result);

		SwapChain::Info swapChainInfo{};
		swapChainInfo.surface = _surface;
		swapChainInfo.physicalDevice = _physicalDevice;
		swapChainInfo.device = _device;
		swapChainInfo.queues = _queues;
		swapChainInfo.commandPool = _commandPool;
		swapChainInfo.windowHandler = _windowHandler;
		swapChainInfo.allocator = settings.allocator;
		swapChainInfo.renderer = this;
		_swapChain.Construct(swapChainInfo);

		_defaultSwapChainRenderPass = CreateRenderPass();
		_swapChain.SetRenderPass(_defaultSwapChainRenderPass);
	}

	VkRenderer::~VkRenderer()
	{
		DeviceWaitIdle();
		DestroyRenderPass(_defaultSwapChainRenderPass);
		_swapChain.Cleanup();
		vkDestroyCommandPool(_device, _commandPool, nullptr);
		vkDestroyDevice(_device, nullptr);
		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		delete _debugger;
		vkDestroyInstance(_instance, nullptr);
	}

	void VkRenderer::Draw(const uint32_t indexCount) const
	{
		vkCmdDrawIndexed(_currentCommandBuffer, indexCount, 1, 0, 0, 0);
	}

	void VkRenderer::Submit(VkCommandBuffer* buffers, 
		const uint32_t buffersCount, const VkSemaphore waitSemaphore,
		const VkSemaphore signalSemaphore, const VkFence fence) const
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submitInfo.waitSemaphoreCount = waitSemaphore ? 1 : 0;
		submitInfo.pWaitSemaphores = &waitSemaphore;
		submitInfo.pWaitDstStageMask = &waitStage;
		submitInfo.commandBufferCount = buffersCount;
		submitInfo.pCommandBuffers = buffers;
		submitInfo.signalSemaphoreCount = signalSemaphore ? 1 : 0;
		submitInfo.pSignalSemaphores = &signalSemaphore;

		vkResetFences(_device, 1, &fence);
		const auto result = vkQueueSubmit(_queues.graphics, 1, &submitInfo, fence);
		assert(!result);
	}

	void VkRenderer::DeviceWaitIdle() const
	{
		const auto result = vkDeviceWaitIdle(_device);
		assert(!result);
	}

	VkDescriptorSetLayout VkRenderer::CreateLayout(const LayoutInfo& info) const
	{
		const uint32_t bindingsCount = info.bindings.size();
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings{};
		layoutBindings.resize(bindingsCount);

		for (uint32_t i = 0; i < bindingsCount; ++i)
		{
			const auto& binding = info.bindings[i];
			auto& uboLayoutBinding = layoutBindings[i];

			uboLayoutBinding.binding = i;
			uboLayoutBinding.descriptorType = binding.type;
			uboLayoutBinding.descriptorCount = binding.count;
			uboLayoutBinding.stageFlags = binding.flag;
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = nullptr;
		layoutInfo.flags = 0;
		layoutInfo.bindingCount = bindingsCount;
		layoutInfo.pBindings = layoutBindings.data();

		VkDescriptorSetLayout layout;
		const auto result = vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &layout);
		assert(!result);
		return layout;
	}

	void VkRenderer::DestroyLayout(const VkDescriptorSetLayout layout) const
	{
		vkDestroyDescriptorSetLayout(_device, layout, nullptr);
	}

	VkDescriptorPool VkRenderer::CreateDescriptorPool(const VkDescriptorType* types, const uint32_t* capacities, const uint32_t count) const
	{
		std::vector<VkDescriptorPoolSize> sizes{};
		sizes.resize(count);

		uint32_t maxSets = 0;

		for (uint32_t i = 0; i < count; ++i)
		{
			auto& size = sizes[i];
			size.type = types[i];
			size.descriptorCount = capacities[i];
			maxSets += size.descriptorCount;
		}

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = count;
		poolInfo.pPoolSizes = sizes.data();
		poolInfo.maxSets = maxSets;

		VkDescriptorPool pool;
		const auto result = vkCreateDescriptorPool(_device, &poolInfo, nullptr, &pool);
		assert(!result);
		return pool;
	}

	void VkRenderer::DestroyDescriptorPool(const VkDescriptorPool pool) const
	{
		vkDestroyDescriptorPool(_device, pool, nullptr);
	}

	void VkRenderer::CreatePipeline(const PipelineInfo& info, VkPipeline& outPipeline, VkPipelineLayout& outLayout)
	{
		std::vector<VkPipelineShaderStageCreateInfo> modules{};

		const uint32_t modulesCount = info.modules.size();
		modules.resize(modulesCount);

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
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(info.attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &info.bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = info.attributeDescriptions.data();

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

		std::vector<VkPushConstantRange> pushConstantRanges{};
		for (const auto& pushConstant : info.pushConstants)
		{
			VkPushConstantRange pushConstantRange;
			pushConstantRange.offset = 0;
			pushConstantRange.size = pushConstant.size;
			pushConstantRange.stageFlags = pushConstant.flag;
			pushConstantRanges.push_back(pushConstantRange);
		}

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = info.setLayouts.size();
		pipelineLayoutInfo.pSetLayouts = info.setLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = info.pushConstants.size();
		pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

		auto result = vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &outLayout);
		assert(!result);

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = modulesCount;
		pipelineInfo.pStages = modules.data();
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

		result = vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &outPipeline);
		assert(!result);
	}

	void VkRenderer::DestroyPipeline(const VkPipeline pipeline, const VkPipelineLayout layout) const
	{
		vkDestroyPipeline(_device, pipeline, nullptr);
		vkDestroyPipelineLayout(_device, layout, nullptr);
	}

	void VkRenderer::CreateDescriptorSets(const VkDescriptorPool pool, 
		const VkDescriptorSetLayout layout, const uint32_t setCount,
		VkDescriptorSet* outSets) const
	{
		std::vector<VkDescriptorSetLayout> layouts(setCount, layout);

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pool;
		allocInfo.descriptorSetCount = setCount;
		allocInfo.pSetLayouts = layouts.data();

		const auto result = vkAllocateDescriptorSets(_device, &allocInfo, outSets);
		assert(!result);
	}

	VkRenderPass VkRenderer::CreateRenderPass(const RenderPassInfo& info) const
	{
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = info.useColorAttachment;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = info.useColorAttachment;
		subpass.pColorAttachments = &colorAttachmentRef;
		if(info.useDepthAttachment)
			subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = 0;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = 0;
		dependency.dstAccessMask = 0;

		std::vector<VkAttachmentDescription> descriptions{};

		if(info.useColorAttachment)
		{
			const auto format = _swapChain.GetFormat();

			VkAttachmentDescription colorDescription{};
			colorDescription.format = format;
			colorDescription.samples = VK_SAMPLE_COUNT_1_BIT;
			colorDescription.loadOp = info.colorLoadOp;
			colorDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorDescription.initialLayout = info.colorInitialLayout;
			colorDescription.finalLayout = info.colorFinalLayout;
			descriptions.push_back(colorDescription);

			dependency.srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}

		if(info.useDepthAttachment)
		{
			VkAttachmentDescription depthDescription{};
			depthDescription.format = GetDepthBufferFormat();
			depthDescription.samples = VK_SAMPLE_COUNT_1_BIT;
			depthDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthDescription.storeOp = info.depthStoreOp;
			depthDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			descriptions.push_back(depthDescription);

			dependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = descriptions.size();
		renderPassInfo.pAttachments = descriptions.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VkRenderPass renderPass;
		const auto result = vkCreateRenderPass(_device, &renderPassInfo, nullptr, &renderPass);
		assert(!result);
		return renderPass;
	}

	void VkRenderer::DestroyRenderPass(const VkRenderPass renderPass) const
	{
		vkDestroyRenderPass(_device, renderPass, nullptr);
	}

	void VkRenderer::BindVertexBuffer(const VkBuffer buffer) const
	{
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(_currentCommandBuffer, 0, 1, &buffer, &offset);
	}

	void VkRenderer::BindIndicesBuffer(const VkBuffer buffer) const
	{
		vkCmdBindIndexBuffer(_currentCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT16);
	}

	void VkRenderer::BindBuffer(const VkDescriptorSet set, const VkBuffer buffer, 
		const VkDeviceSize offset, const VkDeviceSize range, 
		const uint32_t bindingIndex, const uint32_t arrayIndex) const
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

		vkUpdateDescriptorSets(_device, 1, &descriptorWrite, 0, nullptr);
	}

	void VkRenderer::BindSampler(const VkDescriptorSet set, const VkImageView imageView, const VkImageLayout layout,
		const VkSampler sampler, const uint32_t bindingIndex, const uint32_t arrayIndex) const
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

		vkUpdateDescriptorSets(_device, 1, &descriptorWrite, 0, nullptr);
	}

	VkShaderModule VkRenderer::CreateShaderModule(const std::vector<char>& data) const
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = data.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(data.data());

		VkShaderModule vkModule;
		const auto result = vkCreateShaderModule(_device, &createInfo, nullptr, &vkModule);
		assert(!result);
		return vkModule;
	}

	void VkRenderer::DestroyShaderModule(const VkShaderModule module) const
	{
		vkDestroyShaderModule(_device, module, nullptr);
	}

	VkImage VkRenderer::CreateImage(
		const glm::ivec2 resolution, 
		const VkFormat format, 
		const VkImageTiling tiling,
		const VkImageUsageFlags usage) const
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<uint32_t>(resolution.x);
		imageInfo.extent.height = static_cast<uint32_t>(resolution.y);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkImage image;
		const auto result = vkCreateImage(_device, &imageInfo, nullptr, &image);
		assert(!result);
		return image;
	}

	void VkRenderer::TransitionImageLayout(const VkImage image, 
		const VkImageLayout oldLayout, const VkImageLayout newLayout) const
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags srcStage = 0;
		VkPipelineStageFlags dstStage = 0;

		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			const auto format = GetDepthBufferFormat();
			if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT)
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		GetLayoutMasks(oldLayout, barrier.srcAccessMask, srcStage);
		GetLayoutMasks(newLayout, barrier.dstAccessMask, dstStage);

		vkCmdPipelineBarrier(_currentCommandBuffer,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}

	void VkRenderer::DestroyImage(const VkImage image) const
	{
		vkDestroyImage(_device, image, nullptr);
	}

	VkImageView VkRenderer::CreateImageView(const VkImage image, 
		const VkFormat format, const VkImageAspectFlags aspectFlags) const
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = image;

		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = format;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = aspectFlags;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		const auto result = vkCreateImageView(_device, &createInfo, nullptr, &imageView);
		assert(!result);
		return imageView;
	}

	void VkRenderer::DestroyImageView(const VkImageView imageView) const
	{
		vkDestroyImageView(_device, imageView, nullptr);
	}

	VkSampler VkRenderer::CreateSampler(const VkFilter magFilter, const VkFilter minFilter) const
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(_physicalDevice, &properties);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = magFilter;
		samplerInfo.minFilter = minFilter;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		VkSampler sampler;
		const auto result = vkCreateSampler(_device, &samplerInfo, nullptr, &sampler);
		assert(!result);
		return sampler;
	}

	void VkRenderer::DestroySampler(const VkSampler sampler) const
	{
		vkDestroySampler(_device, sampler, nullptr);
	}

	VkFramebuffer VkRenderer::CreateFrameBuffer(const VkImageView* imageViews, const uint32_t imageViewCount,
		const VkRenderPass renderPass, const VkExtent2D extent) const
	{
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = imageViewCount;
		framebufferInfo.pAttachments = imageViews;
		framebufferInfo.width = extent.width;
		framebufferInfo.height = extent.height;
		framebufferInfo.layers = 1;

		VkFramebuffer frameBuffer;
		const auto result = vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &frameBuffer);
		assert(!result);
		return frameBuffer;
	}

	void VkRenderer::DestroyFrameBuffer(const VkFramebuffer frameBuffer) const
	{
		vkDestroyFramebuffer(_device, frameBuffer, nullptr);
	}

	SwapChain& VkRenderer::GetSwapChain()
	{
		return _swapChain;
	}

	void VkRenderer::BeginRenderPass(const VkFramebuffer frameBuffer, 
		const VkRenderPass renderPass, const glm::ivec2 offset,
		const glm::ivec2 extent, VkClearValue* clearColors, const uint32_t clearColorsCount) const
	{
		const VkExtent2D extentVk
		{
			static_cast<uint32_t>(extent.x),
			static_cast<uint32_t>(extent.y)
		};

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = frameBuffer;
		renderPassInfo.renderArea.offset = { offset.x, offset.y };
		renderPassInfo.renderArea.extent = extentVk;

		renderPassInfo.clearValueCount = clearColorsCount;
		renderPassInfo.pClearValues = clearColors;

		vkCmdBeginRenderPass(_currentCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VkRenderer::EndRenderPass() const
	{
		vkCmdEndRenderPass(_currentCommandBuffer);
	}

	VkCommandBuffer VkRenderer::CreateCommandBuffer() const
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = _commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		const auto result = vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer);
		assert(!result);

		return commandBuffer;
	}

	void VkRenderer::BeginCommandBufferRecording(const VkCommandBuffer commandBuffer)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		_currentCommandBuffer = commandBuffer;
	}

	void VkRenderer::EndCommandBufferRecording() const
	{
		const auto result = vkEndCommandBuffer(_currentCommandBuffer);
		assert(!result);
	}

	void VkRenderer::DestroyCommandBuffer(const VkCommandBuffer commandBuffer) const
	{
		vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
	}

	VkSemaphore VkRenderer::CreateSemaphore() const
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkSemaphore semaphore;
		const auto result = vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &semaphore);
		assert(!result);
		return semaphore;
	}

	void VkRenderer::DestroySemaphore(const VkSemaphore semaphore) const
	{
		vkDestroySemaphore(_device, semaphore, nullptr);
	}

	VkFence VkRenderer::CreateFence() const
	{
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VkFence fence;
		const auto result = vkCreateFence(_device, &fenceInfo, nullptr, &fence);
		assert(!result);
		return fence;
	}

	void VkRenderer::WaitForFence(const VkFence fence) const
	{
		vkWaitForFences(_device, 1, &fence, VK_TRUE, UINT64_MAX);
	}

	void VkRenderer::DestroyFence(const VkFence fence) const
	{
		vkDestroyFence(_device, fence, nullptr);
	}

	VkBuffer VkRenderer::CreateBuffer(const VkDeviceSize size, const VkBufferUsageFlags flags) const
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = flags;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkBuffer vertexBuffer;
		const auto result = vkCreateBuffer(_device, &bufferInfo, nullptr, &vertexBuffer);
		assert(!result);
		return vertexBuffer;
	}

	void VkRenderer::CopyBuffer(const VkBuffer srcBuffer, const VkBuffer dstBuffer, 
		const VkDeviceSize size, const VkDeviceSize srcOffset, const VkDeviceSize dstOffset) const
	{
		VkBufferCopy region{};
		region.srcOffset = srcOffset;
		region.dstOffset = dstOffset;
		region.size = size;
		vkCmdCopyBuffer(_currentCommandBuffer, srcBuffer, dstBuffer, 1, &region);
	}

	void VkRenderer::CopyBuffer(const VkBuffer srcBuffer, const VkImage dstImage, 
		const uint32_t width, const uint32_t height) const
	{
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent =
		{
			width,
			height,
			1
		};

		auto& subResource = region.imageSubresource;
		subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subResource.mipLevel = 0;
		subResource.baseArrayLayer = 0;
		subResource.layerCount = 1;

		vkCmdCopyBufferToImage(_currentCommandBuffer, srcBuffer, dstImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}

	void VkRenderer::DestroyBuffer(const VkBuffer buffer) const
	{
		vkDestroyBuffer(_device, buffer, nullptr);
	}

	VkDeviceMemory VkRenderer::AllocateMemory(const VkImage image, const VkMemoryPropertyFlags flags) const
	{
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(_device, image, &memRequirements);
		return AllocateMemory(memRequirements, flags);
	}

	VkDeviceMemory VkRenderer::AllocateMemory(const VkBuffer buffer, const VkMemoryPropertyFlags flags) const
	{
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(_device, buffer, &memRequirements);
		return AllocateMemory(memRequirements, flags);
	}

	VkDeviceMemory VkRenderer::AllocateMemory(const VkMemoryRequirements memRequirements, const VkMemoryPropertyFlags flags) const
	{
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, flags);

		VkDeviceMemory memory;
		const auto result = vkAllocateMemory(_device, &allocInfo, nullptr, &memory);
		assert(!result);
		return memory;
	}

	void VkRenderer::BindMemory(const VkImage image, const VkDeviceMemory memory, const VkDeviceSize offset) const
	{
		vkBindImageMemory(_device, image, memory, offset);
	}

	void VkRenderer::BindMemory(const VkBuffer buffer, const VkDeviceMemory memory, const VkDeviceSize offset) const
	{
		vkBindBufferMemory(_device, buffer, memory, offset);
	}

	void VkRenderer::FreeMemory(VkDeviceMemory memory) const
	{
		vkFreeMemory(_device, memory, nullptr);
	}

	VkFormat VkRenderer::FindSupportedFormat(const std::vector<VkFormat>& candidates, 
		const VkImageTiling tiling, const VkFormatFeatureFlags features) const
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
				return format;
			if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
				return format;
		}

		throw std::exception("Format not available!");
	}

	uint32_t VkRenderer::FindMemoryType(const uint32_t typeFilter, const VkMemoryPropertyFlags properties) const
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			if (typeFilter & 1 << i)
			{
				const bool requiredPropertiesPresent = (memProperties.memoryTypes[i].propertyFlags & properties) == properties;
				if (!requiredPropertiesPresent)
					continue;

				return i;
			}

		throw std::exception("Memory type not available!");
	}

	void VkRenderer::GetLayoutMasks(const VkImageLayout layout, 
		VkAccessFlags& outAccessFlags, VkPipelineStageFlags& outPipelineStageFlags)
	{
		switch (layout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			outAccessFlags = 0;
			outPipelineStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			outAccessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
			outPipelineStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			outAccessFlags = VK_ACCESS_SHADER_READ_BIT;
			outPipelineStageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			outAccessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			outPipelineStageFlags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			break;
		default:
			throw std::exception("Layout transition not supported!");
		}
	}

	VkFormat VkRenderer::GetDepthBufferFormat() const
	{
		return FindSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}
}
