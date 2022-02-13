#pragma once

// Standard (optional) struct that can be used to define renderable triangles in shaders.
struct Vertex final
{
	// Using uint16_t for this, because the models for this game won't be that large. 
	typedef uint16_t Index;

	glm::vec3 position{0};
	// The forward vector for this vertex.
	glm::vec3 normal{0, 1, 0};
	// Sample coordinates for texture attachments.
	glm::vec2 textureCoordinates{0};

	// Returns a vulkan description for the vertex binding.
	[[nodiscard]] static VkVertexInputBindingDescription GetBindingDescription();
	// Returns a vulkan description for the vertex attributes.
	[[nodiscard]] static vi::Vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
};
