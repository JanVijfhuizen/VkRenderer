#pragma once

struct Transform final
{
	glm::vec3 position{0};
	float rotation = 0;
	float scale = 1;
};

class TransformSystem final : public ce::System<Transform>
{
public:
	explicit TransformSystem(ce::Cecsar& cecsar);
};
