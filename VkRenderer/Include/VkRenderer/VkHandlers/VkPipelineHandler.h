#pragma once
#include "VkHandler.h"

namespace vi
{
	/// <summary>
	/// Contains some core functionality for vulkan pipelines.
	/// </summary>
	class VkPipelineHandler final : public VkHandler
	{
	public:
		/// <summary>
		/// Struct that contains information for creating a pipeline.
		/// </summary>
		struct CreateInfo final
		{
			// Struct used to define a shader module used by a pipeline.
			struct Module final
			{
				VkShaderModule module;
				VkShaderStageFlagBits flags;
			};

			// Struct used to define a push constant used by a pipeline.
			struct PushConstant final
			{
				size_t size;
				VkShaderStageFlags flag;
			};

			// Render pass used by the pipeline.
			VkRenderPass renderPass;
			Vector<Module> modules{2, GMEM_TEMP};
			Vector<VkDescriptorSetLayout> setLayouts{4, GMEM_TEMP};
			Vector<PushConstant> pushConstants{1, GMEM_TEMP};

			// Vertex data description.
			VkVertexInputBindingDescription bindingDescription;
			Vector<VkVertexInputAttributeDescription> attributeDescriptions{4, GMEM_TEMP};

			// Shape of the drawn topology. Examples can be triangles, points, lines.
			VkPrimitiveTopology primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			// Whether or not there can be gaps in the topology (multiple separate meshes).
			VkBool32 primitiveRestartEnable = VK_FALSE;
			// Shape of the render target. Usually the same size as the screen.
			glm::ivec2 extent;
			// Clamps depth between 0 and 1.
			VkBool32 depthClampEnable = VK_FALSE;
			// Shape of the polygons. Can support things like wireframe.
			VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
			float lineWidth = 1;
			// Culls backfaces by default. Examples like a skybox might want to inverse this.
			VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
			// What counts as a front face.
			VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			// Does the pipeline use a depth buffer.
			bool depthBufferEnabled = true;
			// Changes the way values are stored in the depth buffer. By default it stores the lowest value.
			VkCompareOp depthBufferCompareOp = VK_COMPARE_OP_LESS;
			// Optional pipeline this one is inspired by. Useful when recreating assets, since construction can be faster.
			VkPipeline basePipeline = VK_NULL_HANDLE;
			int32_t basePipelineIndex = -1;
			// Multisampling.
			VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
			// Shader sampling.
			bool shaderSamplingEnabled = true;
			float minSampleShading = .2f;
		};

		explicit VkPipelineHandler(VkCore& core);

		void Create(const CreateInfo& info, VkPipeline& outPipeline, VkPipelineLayout& outLayout) const;
		void Bind(VkPipeline pipeline, VkPipelineLayout layout);
		void Destroy(VkPipeline pipeline, VkPipelineLayout layout) const;

		[[nodiscard]] VkPipeline GetCurrent() const;
		[[nodiscard]] VkPipelineLayout GetCurrentLayout() const;

	private:
		VkPipeline _current;
		VkPipelineLayout _currentLayout;
	};
}
