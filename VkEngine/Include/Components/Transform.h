#pragma once

struct Transform final
{
	struct PushConstant final
	{
		glm::mat4 model;
	};

	glm::vec3 position{0};
	glm::vec3 rotation{0};
	glm::vec3 scale{1};

	[[nodiscard]] glm::vec3 GetForwardVector() const;
	void CreateModelMatrix(glm::mat4& outMat) const;
};

class TransformSystem final : public ce::System<Transform>
{
public:
	explicit TransformSystem(ce::Cecsar& cecsar);
};
