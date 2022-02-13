#include "pch.h"
#include "Rendering/ShaderExt.h"
#include "Utils/FileReader.h"
#include "VkRenderer/VkCore/VkCore.h"
#include "VkRenderer/VkHandlers/VkShaderHandler.h"

ShaderExt::ShaderExt(vi::VkCore& core) : VkHandler(core)
{
}

Shader ShaderExt::Load(const char* name, const LoadInfo& info) const
{
	auto& handler = core.GetShaderHandler();

	const vi::String middle{ name, GMEM_TEMP };
	vi::String postFixes[3]
	{
		{"vert.spv", GMEM_TEMP },
		{"geom.spv", GMEM_TEMP },
		{"frag.spv", GMEM_TEMP }
	};
	VkShaderStageFlagBits shaderStageFlags[3]
	{
		VK_SHADER_STAGE_VERTEX_BIT,
		VK_SHADER_STAGE_GEOMETRY_BIT,
		VK_SHADER_STAGE_FRAGMENT_BIT
	};

	Shader shader{};

	// Check for vertex, geometry and fragment shaders, if any.
	for (uint32_t i = 0; i < 3; ++i)
	{
		if (!info.values[i])
			continue;

		vi::String postFix{ postFixes[i], GMEM_TEMP };
		const auto code = ToCode(middle, postFix);

		auto& module = shader.modules.Add();
		module.module = handler.CreateModule(code);
		module.flags = shaderStageFlags[i];
	}

	return shader;
}

void ShaderExt::DestroyShader(const Shader& shader)
{
	auto& handler = core.GetShaderHandler();

	for (auto& module : shader.modules)
		if (module.module)
			handler.DestroyModule(module.module);
}

vi::String ShaderExt::ToCode(const vi::String& name, const vi::String& postFix)
{
	vi::String path{ "Shaders/", GMEM_TEMP };
	path.Append(name);
	path.Append(postFix);
	return FileReader::Read(path);
}
