#pragma once

struct Vertex final
{
	glm::vec3 position{};
	glm::vec3 normal{ 0, 0, 1 };
	glm::vec2 textureCoordinates{};

	struct Data final
	{
		std::vector<Vertex> vertices{};
		std::vector<uint16_t> indices{};
	};

	[[nodiscard]] static Data Load(const std::string& fileName);

	[[nodiscard]] static VkVertexInputBindingDescription GetBindingDescription();
	[[nodiscard]] static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
};
