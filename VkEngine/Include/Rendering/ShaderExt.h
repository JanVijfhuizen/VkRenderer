#pragma once
#include "VkRenderer/VkHandlers/VkHandler.h"
#include "VkRenderer/VkHandlers/VkPipelineHandler.h"

struct Shader final
{
	union
	{
		struct
		{
			vi::VkPipelineHandler::CreateInfo::Module vertex;
			vi::VkPipelineHandler::CreateInfo::Module geometry;
			vi::VkPipelineHandler::CreateInfo::Module fragment;
		};

		vi::VkPipelineHandler::CreateInfo::Module modules[3]
		{
			{{}, VK_SHADER_STAGE_VERTEX_BIT},
			{{}, VK_SHADER_STAGE_GEOMETRY_BIT},
			{{}, VK_SHADER_STAGE_FRAGMENT_BIT},
		};
	};
};

class ShaderExt : public vi::VkHandler
{
public:
	struct LoadInfo final
	{
		union
		{
			struct
			{
				bool vertex;
				bool geometry;
				bool fragment;
			};
			bool values[3]{true, false, true};
		};
	};

	explicit ShaderExt(vi::VkCore& core);

	[[nodiscard]] Shader Load(const char* name, const LoadInfo& info = {}) const;
	void DestroyShader(const Shader& shader);

private:
	[[nodiscard]] static vi::String ToCode(const vi::String& name, const vi::String& postFix);
};
