#include "pch.h"
#include "VkHandlers/VkLayoutHandler.h"
#include "VkCore/VkCore.h"

namespace vi
{
	VkDescriptorSetLayout VkLayoutHandler::CreateLayout(const CreateInfo& info) const
	{
		const uint32_t bindingsCount = info.bindings.GetCount();
		const ArrayPtr<VkDescriptorSetLayoutBinding> layoutBindings(bindingsCount, GMEM_TEMP);

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
		layoutInfo.pBindings = layoutBindings.GetData();

		VkDescriptorSetLayout layout;
		const auto result = vkCreateDescriptorSetLayout(core.GetLogicalDevice(), &layoutInfo, nullptr, &layout);
		assert(!result);
		return layout;
	}

	void VkLayoutHandler::DestroyLayout(const VkDescriptorSetLayout layout) const
	{
		vkDestroyDescriptorSetLayout(core.GetLogicalDevice(), layout, nullptr);
	}

	VkLayoutHandler::VkLayoutHandler(VkCore& core) : VkHandler(core)
	{

	}
}
