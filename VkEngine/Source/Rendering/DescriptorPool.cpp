#include "pch.h"
#include "Rendering/DescriptorPool.h"
#include "Rendering/RenderSystem.h"
#include "VkRenderer/VkRenderer.h"

DescriptorPool::DescriptorPool() = default;

DescriptorPool::DescriptorPool(const VkDescriptorSetLayout layout, 
	const ArrayPtr<VkDescriptorType> types, const uint32_t blockSize) : _layout(layout), _blockSize(blockSize), _open(blockSize)
{
	const size_t size = sizeof(VkDescriptorType) * types.GetSize();
	const auto typePtr = reinterpret_cast<VkDescriptorType*>(GMEM.MAlloc(size));
	memcpy(typePtr, types.GetData(), size);
	_types = ArrayPtr( typePtr, types.GetSize());

	AddBlock();
}

DescriptorPool::~DescriptorPool()
{
	GMEM.MFree(_types.GetData());

	auto& renderSystem = RenderSystem::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	for (auto& subPool : _subPools)
		renderer.DestroyDescriptorPool(subPool);
}

VkDescriptorSet DescriptorPool::Get()
{
	if (_open.GetCount() == 0)
		AddBlock();
	return _open[0];
}

void DescriptorPool::Add(const VkDescriptorSet set)
{
	_open.Add(set);
}

void DescriptorPool::AddBlock()
{
	auto& renderSystem = RenderSystem::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	const auto capacities = reinterpret_cast<uint32_t*>(GMEM_TEMP.MAlloc(sizeof(uint32_t) * _blockSize));
	for (uint32_t i = 0; i < _blockSize; ++i)
		capacities[i] = _blockSize;
	const auto subPool = renderer.CreateDescriptorPool(_types.GetData(), capacities, 1);
	GMEM_TEMP.MFree(capacities);

	_subPools.Add(subPool);

	const auto sets = reinterpret_cast<VkDescriptorSet*>(GMEM_TEMP.MAlloc(sizeof(VkDescriptorSet) * _blockSize));
	renderer.CreateDescriptorSets(subPool, _layout, _blockSize, sets);
	for (uint32_t i = 0; i < _blockSize; ++i)
		_open.Add(sets[i]);
	GMEM_TEMP.MFree(sets);
}
