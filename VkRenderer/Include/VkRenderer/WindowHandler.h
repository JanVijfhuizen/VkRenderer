#pragma once

namespace vi
{
	/// <summary>
	/// Handles the windowing of the application. Can open and close windows.
	/// </summary>
	class WindowHandler
	{
		friend class VkRenderer;
		friend class InstanceFactory;

	public:
		struct Info final
		{
			glm::ivec2 resolution{ 800, 600 };
			String name{"My Window", GMEM_TEMP};

			Info();
			Info(const Info& other);
		};

		explicit WindowHandler(const Info& info = {});

		/// <summary>Checks if the window resized since the previous frame.</summary>
		[[nodiscard]] virtual bool QueryHasResized() = 0;
		/// <returns>Constructor info.</returns>
		[[nodiscard]] virtual const Info& GetInfo() const;

	protected:
		Info info;

		virtual VkSurfaceKHR CreateSurface(VkInstance instance) = 0;
		[[nodiscard]] virtual ArrayPtr<const char*> GetRequiredExtensions() = 0;
	};
}