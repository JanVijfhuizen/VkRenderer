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
		Bake(transform, baked);
	}
}

void Transform::System::Swap(const uint32_t aDenseId, const uint32_t bDenseId)
{
	SparseSet<Transform>::Swap(aDenseId, bDenseId);
	const auto mat4 = _bakedArr[aDenseId];
	_bakedArr[aDenseId] = _bakedArr[bDenseId];
	_bakedArr[bDenseId] = mat4;
}

void Transform::System::Bake(const uint32_t sparseId)
{
	const uint32_t denseId = GetDenseId(sparseId);
	Bake(operator[](sparseId), _bakedArr[denseId]);
}

Transform::Baked* Transform::System::GetBakedTransforms() const
{
	return _bakedArr;
}

void Transform::System::Bake(Transform& transform, Baked& baked) const
{
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

glm::vec3 Transform::GetForwardVector() const
{
	const float sinX = sin(rotation.x);
	const float sinY = sin(rotation.y);
	const float cosY = cos(rotation.y);

	return { sinY, sinX, cosY };
}
