#pragma once

struct Vertex final
{
	glm::vec3 position{0};
	glm::vec3 normal{0, 0, 1};
	glm::vec2 textureCoordinates{0};

	[[nodiscard]] static VkVertexInputBindingDescription GetBindingDescription();
	[[nodiscard]] static vi::Vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
};
