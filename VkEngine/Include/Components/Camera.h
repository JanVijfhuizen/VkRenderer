#pragma once

struct Camera final
{
	struct Ubo final
	{
		glm::vec3 position;
		float scale;
	};
};
