#include "pch.h"
#include "Rendering/Vertex.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

Vertex::Data Vertex::Load(const std::string& fileName)
{
	// It's far from the best method, but it's not the end of the world since I'm only planning to load in very simple models.

	Data data{};

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;

	std::string path = "Meshes/" + fileName;
	const bool success = LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());
	assert(success);

	if (!warn.empty())
		std::cout << warn << std::endl;

	if (!err.empty())
		std::cerr << err << std::endl;

	auto& vertices = attrib.vertices;
	auto& normals = attrib.normals;
	auto& texCoords = attrib.texcoords;

	for (const auto& shape : shapes)
	{
		size_t index_offset = 0;

		auto& faces = shape.mesh.num_face_vertices;
		auto& inds = shape.mesh.indices;

		for (const auto& face : faces)
		{
			const auto fv = size_t(face);

			for (size_t v = 0; v < fv; v++)
			{
				Vertex vertex{};
				tinyobj::index_t idx = inds[index_offset + v];

				auto& position = vertex.position;
				const auto vertSize = 3 * size_t(idx.vertex_index);

				position.x = vertices[vertSize];
				position.y = vertices[vertSize + 1];
				position.z = vertices[vertSize + 2];

				if (idx.normal_index >= 0)
				{
					auto& normal = vertex.normal;
					const auto normSize = 3 * size_t(idx.normal_index);

					normal.x = normals[normSize];
					normal.y = normals[normSize + 1];
					normal.z = normals[normSize + 2];
				}

				if (idx.texcoord_index >= 0)
				{
					auto& texCoord = vertex.textureCoordinates;
					const auto texSize = 2 * size_t(idx.texcoord_index);

					texCoord.x = texCoords[texSize];
					texCoord.y = texCoords[texSize + 1];
				}

				data.vertices.push_back(vertex);
			}

			index_offset += fv;
		}
	}

	data.indices.resize(data.vertices.size());
	for (uint32_t i = 0; i < data.vertices.size(); ++i)
		data.indices[i] = i;

	return data;
}

VkVertexInputBindingDescription Vertex::GetBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> Vertex::GetAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
	attributeDescriptions.resize(3);

	auto& position = attributeDescriptions[0];
	position.binding = 0;
	position.location = 0;
	position.format = VK_FORMAT_R32G32B32_SFLOAT;
	position.offset = offsetof(Vertex, position);

	auto& normal = attributeDescriptions[1];
	normal.binding = 0;
	normal.location = 1;
	normal.format = VK_FORMAT_R32G32B32_SFLOAT;
	normal.offset = offsetof(Vertex, normal);

	auto& texCoords = attributeDescriptions[2];
	texCoords.binding = 0;
	texCoords.location = 2;
	texCoords.format = VK_FORMAT_R32G32_SFLOAT;
	texCoords.offset = offsetof(Vertex, textureCoordinates);

	return attributeDescriptions;
}
