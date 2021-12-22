#include "pch.h"
#include "Rendering/DescriptorPool.h"
#include "Rendering/RenderManager.h"
#include "VkRenderer/VkRenderer.h"

DescriptorPool::DescriptorPool() = default;

DescriptorPool::DescriptorPool(const VkDescriptorSetLayout layout, 
	VkDescriptorType* types, uint32_t* sizes,
	const uint32_t typeCount, const uint32_t blockSize) :
	_layout(layout), _typeCount(typeCount), _blockSize(blockSize), _open(blockSize)
{
	const size_t typesSize = sizeof(VkDescriptorType) * typeCount;
	_types = reinterpret_cast<VkDescriptorType*>(GMEM.MAlloc(typesSize));
	memcpy(_types, types, typesSize);

	const size_t sizesSize = sizeof(uint32_t) * typeCount;
	_sizes = reinterpret_cast<uint32_t*>(GMEM.MAlloc(sizesSize));
	memcpy(_sizes, sizes, sizesSize);

	AddBlock();
}

DescriptorPool& DescriptorPool::operator=(DescriptorPool&& other) noexcept
{
	this->~DescriptorPool();

	_layout = other._layout;
	_typeCount = other._typeCount;
	_blockSize = other._blockSize;
	_open = UVector<VkDescriptorSet>(other._open.GetCount());
	for (auto& open : other._open)
		_open.Add(open);

	_types = other._types;
	other._types = nullptr;
	_sizes = other._sizes;
	other._sizes = nullptr;
	for (auto& subPool : other._subPools)
		_subPools.Add(subPool);
	other._subPools.Clear();

	return *this;
}

DescriptorPool::~DescriptorPool()
{
	GMEM.MFree(_types);
	GMEM.MFree(_sizes);

	auto& renderManager = RenderManager::Get();
	auto& renderer = renderManager.GetVkRenderer();

	for (auto& subPool : _subPools)
		renderer.DestroyDescriptorPool(subPool);
}

VkDescriptorSet DescriptorPool::Get()
{
	if (_open.GetCount() == 0)
		AddBlock();
	const auto set = _open[0];
	_open.EraseAt(0);
	return set;
}

void DescriptorPool::Add(const VkDescriptorSet set)
{
	_open.Add(set);
}

void DescriptorPool::AddBlock()
{
	auto& renderManager = RenderManager::Get();
	auto& renderer = renderManager.GetVkRenderer();

	const auto subPool = renderer.CreateDescriptorPool(_types, _sizes, _typeCount);
	_subPools.Add(subPool);

	const auto sets = reinterpret_cast<VkDescriptorSet*>(GMEM_TEMP.MAlloc(sizeof(VkDescriptorSet) * _blockSize));
	renderer.CreateDescriptorSets(subPool, _layout, _blockSize, sets);
	for (uint32_t i = 0; i < _blockSize; ++i)
		_open.Add(sets[i]);
	GMEM_TEMP.MFree(sets);
}
