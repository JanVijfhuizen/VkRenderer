#pragma once
#include "VkRenderer/VkHandlers/VkHandler.h"
#include "VkRenderer/VkHandlers/VkPipelineHandler.h"

struct Shader final
{
	vi::VkPipelineHandler::CreateInfo::Module vertex
	{
		{}, VK_SHADER_STAGE_VERTEX_BIT
	};
	vi::VkPipelineHandler::CreateInfo::Module fragment
	{
		{}, VK_SHADER_STAGE_FRAGMENT_BIT
	};
};

class ShaderExt : public vi::VkHandler
{
public:
	explicit ShaderExt(vi::VkCore& core);

	[[nodiscard]] Shader Load(const char* name) const;
	void DestroyShader(const Shader& shader);

private:
	[[nodiscard]] static vi::String ToCode(vi::String& name, vi::String& postFix);
};
