#pragma once

struct Transform final
{
public:
	glm::vec3 position{0};
	float rotation = 0;
	float scale = 1;

	class System final : public ce::System<Transform>
	{
	public:
		explicit System(ce::Cecsar& cecsar);
	};
};
