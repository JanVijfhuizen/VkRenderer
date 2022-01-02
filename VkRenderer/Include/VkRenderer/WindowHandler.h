#pragma once

namespace vi
{
	class WindowHandler
	{
		friend class VkRenderer;
		friend class InstanceFactory;

	public:
		struct VkInfo final
		{
			glm::ivec2 resolution{ 800, 600 };
			std::string name{ "My Window" };
		};

		[[nodiscard]] virtual bool QueryHasResized() = 0;
		[[nodiscard]] virtual const VkInfo& GetVkInfo() const = 0;

	protected:
		virtual VkSurfaceKHR CreateSurface(VkInstance instance) = 0;
		[[nodiscard]] virtual ArrayPtr<const char*> GetRequiredExtensions() = 0;
	};
}