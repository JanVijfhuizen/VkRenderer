#include "pch.h"
#include "Transform.h"

Transform::System::System(const uint32_t size) : SparseSet<Transform>(size)
{
	_bakedArr = new Baked[size];
}

Transform::System::~System()
{
	delete[] _bakedArr;
}

void Transform::System::Update()
{
	for (const auto [transform, sparseId] : *this)
	{
		const uint32_t denseId = GetDenseId(sparseId);
		if (transform.manualBake)
			continue;

		auto& baked = _bakedArr[denseId];
		auto& model = baked.model;

		model = glm::mat4{ 1 };
		model = glm::translate(model, transform.position);

		auto& rotation = transform.rotation;
		const auto euler = glm::eulerAngleXYZ(
			glm::radians(rotation.x) * .5f,
			glm::radians(rotation.y) * .5f,
			glm::radians(rotation.z) * .5f);

		model *= euler;
		model = glm::scale(model, transform.scale);
	}
}

void Transform::System::Swap(const uint32_t aDenseId, const uint32_t bDenseId)
{
	SparseSet<Transform>::Swap(aDenseId, bDenseId);
	const auto mat4 = _bakedArr[aDenseId];
	_bakedArr[aDenseId] = _bakedArr[bDenseId];
	_bakedArr[bDenseId] = mat4;
}

glm::vec3 Transform::System::GetForwardVector(const glm::vec3 eulerAngles)
{
	const float sinX = sin(eulerAngles.x);
	const float sinY = sin(eulerAngles.y);
	const float cosY = cos(eulerAngles.y);

	return { sinY, sinX, cosY };
}

Transform::Baked* Transform::System::GetBakedTransforms() const
{
	return _bakedArr;
}
