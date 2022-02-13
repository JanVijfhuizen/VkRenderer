#pragma once
#include "VkRenderer/VkHandlers/VkHandler.h"
#include "VkRenderer/VkHandlers/VkPipelineHandler.h"

// Struct that contains relevant shader data.
struct Shader final
{
	vi::Vector<vi::VkPipelineHandler::CreateInfo::Module> modules{3, GMEM_TEMP};
};

/// <summary>
/// Contains engine specific shader methods.
/// </summary>
class ShaderExt : public vi::VkHandler
{
public:
	// Struct used for loading shaders.
	struct LoadInfo final
	{
		union
		{
			struct
			{
				// Load vertex shader?
				bool vertex;
				// Load geometry shader?
				bool geometry;
				// Load fragment shader?
				bool fragment;
			};
			bool values[3]{true, false, true};
		};
	};

	explicit ShaderExt(vi::VkCore& core);

	// Load shader. Will throw an exception if the selected modules are not found.
	[[nodiscard]] Shader Load(const char* name, const LoadInfo& info = {}) const;
	// Destroy shader assets.
	void DestroyShader(const Shader& shader);

private:
	[[nodiscard]] static vi::String ToCode(const vi::String& name, const vi::String& postFix);
};
