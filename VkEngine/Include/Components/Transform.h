#pragma once

/// <summary>
/// Component that handles the world transformation for an entity.
/// </summary>
struct Transform final
{
	glm::vec3 position{0};
	// Euler angle rotation.
	glm::vec3 rotation{0};
	glm::vec3 scale{1};

	// Get the forward directional vector based on the rotation.
	[[nodiscard]] glm::vec3 GetForwardVector() const;
	// Create model matrix in world space.
	void CreateModelMatrix(glm::mat4& outMat) const;
};

/// <summary>
/// System that handles the transform components.
/// </summary>
class TransformSystem final : public ce::System<Transform>
{
public:
	explicit TransformSystem(ce::Cecsar& cecsar);
};
