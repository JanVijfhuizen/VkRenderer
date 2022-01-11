#pragma once

struct Camera final
{
	struct Ubo final
	{
		glm::vec3 position;
		float scale;
		float depth = 1;
	};
};

constexpr auto MAX_CAMERAS = 8;

class CameraSystem final : public ce::SmallSystem<Camera>
{
public:
	explicit CameraSystem(ce::Cecsar& cecsar);
};
