#pragma once
#include "VkHandler.h"

namespace vi
{
	class VkLayoutHandler final : public VkHandler
	{
		friend VkCore;

	public:
		struct Info final
		{
			struct Binding final
			{
				VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				size_t size = sizeof(int32_t);
				uint32_t count = 1;
				VkShaderStageFlagBits flag;
			};

			Vector<Binding> bindings{4, GMEM_TEMP};
		};

		[[nodiscard]] VkDescriptorSetLayout CreateLayout(const Info& info) const;
		void DestroyLayout(VkDescriptorSetLayout layout) const;

	private:
		explicit VkLayoutHandler(VkCore& core);
	};
}
