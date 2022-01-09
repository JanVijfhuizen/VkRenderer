#pragma once
#include "VkRenderer/VkHandlers/VkHandler.h"
#include "VkRenderer/VkHandlers/VkPipelineHandler.h"

class ShaderExt : public vi::VkHandler
{
public:
	struct Shader final
	{
		vi::VkPipelineHandler::Info::Module vertex
		{
			{}, VK_SHADER_STAGE_VERTEX_BIT
		};
		vi::VkPipelineHandler::Info::Module fragment
		{
			{}, VK_SHADER_STAGE_FRAGMENT_BIT
		};
	};

	explicit ShaderExt(vi::VkCore& core);

	[[nodiscard]] Shader Load(const char* name) const;

private:
	[[nodiscard]] static vi::String ToCode(vi::String& name, vi::String& postFix);
};
