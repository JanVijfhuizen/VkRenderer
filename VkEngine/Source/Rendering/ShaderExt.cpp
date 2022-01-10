#include "pch.h"
#include "Rendering/ShaderExt.h"
#include "Utils/FileReader.h"
#include "VkRenderer/VkCore/VkCore.h"

ShaderExt::ShaderExt(vi::VkCore& core) : VkHandler(core)
{
}

ShaderExt::Shader ShaderExt::Load(const char* name) const
{
	vi::String vertPostFix{ "vert.spv", GMEM_TEMP };
	vi::String fragPostFix{ "frag.spv", GMEM_TEMP };
	vi::String middle{ name, GMEM_TEMP };

	const auto vertCode = ToCode(middle, vertPostFix);
	const auto fragCode = ToCode(middle, fragPostFix);

	auto& handler = core.GetShaderHandler();

	Shader shader{};
	shader.vertex.module = handler.CreateModule(vertCode);
	shader.fragment.module = handler.CreateModule(fragCode);
	return shader;
}

void ShaderExt::DestroyShader(const Shader& shader)
{
	auto& handler = core.GetShaderHandler();
	handler.DestroyModule(shader.fragment.module);
	handler.DestroyModule(shader.vertex.module);
}

vi::String ShaderExt::ToCode(vi::String& name, vi::String& postFix)
{
	vi::String path{ "Shaders/", GMEM_TEMP };
	path.Append(name);
	path.Append(postFix);
	return FileReader::Read(path);
}
