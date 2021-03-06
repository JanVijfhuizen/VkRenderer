#pragma once
#include "VkHandler.h"

namespace vi
{
	/// <summary>
	/// Contains some core functionality for vulkan descriptor layouts.
	/// </summary>
	class VkLayoutHandler final : public VkHandler
	{
	public:
		/// <summary>
		/// Struct that contains information for creating a layout.
		/// </summary>
		struct CreateInfo final
		{
			/// <summary>
			/// Binding information that is used when creating a new descriptor set layout.
			/// </summary>
			struct Binding final
			{
				// What type the binding is (image, buffer etc).
				VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				size_t size = sizeof(int32_t);
				uint32_t count = 1;
				// Where the binding is used during rendering (vertex, fragment etc).
				VkShaderStageFlagBits flag;
				// Binding index for the shader binding. If not set, will default to this struct's index. 
				int32_t binding = -1;
			};

			Vector<Binding> bindings{4, GMEM_TEMP};
		};

		explicit VkLayoutHandler(VkCore& core);

		/// <returns>Object that can be used to create a render pipeline and to create descriptor sets from.</returns>
		[[nodiscard]] VkDescriptorSetLayout CreateLayout(const CreateInfo& info) const;
		void DestroyLayout(VkDescriptorSetLayout layout) const;
	};
}
