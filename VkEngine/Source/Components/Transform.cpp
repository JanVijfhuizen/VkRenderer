#include "pch.h"
#include "Components/Transform.h"

glm::vec3 Transform::GetForwardVector() const
{
	const float sinX = sin(rotation.x);
	const float sinY = sin(rotation.y);
	const float cosY = cos(rotation.y);

	return {sinY, sinX, cosY};
}

void Transform::CreateModelMatrix(glm::mat4& outMat) const
{
	outMat = glm::mat4{ 1 };
	outMat = glm::translate(outMat, position);

	const auto euler = glm::eulerAngleXYZ(
		glm::radians(rotation.x),
		glm::radians(rotation.y),
		glm::radians(rotation.z));

	outMat *= euler;
	outMat = glm::scale(outMat, scale);
}

TransformSystem::TransformSystem(ce::Cecsar& cecsar) : System<Transform>(cecsar)
{

}
