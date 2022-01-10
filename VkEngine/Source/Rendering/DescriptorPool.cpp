#include "pch.h"
#include "Rendering/DescriptorPool.h"
#include "Rendering/Renderer.h"

DescriptorPool::DescriptorPool(Renderer& renderer,
    const VkDescriptorSetLayout layout, 
    vi::ArrayPtr<VkDescriptorType>& types, 
    vi::ArrayPtr<uint32_t>& capacities,
    const uint32_t blockSize) :
	_renderer(renderer), _layout(layout), _types(types, GMEM_VOL), 
	_capacities(capacities, GMEM_VOL), _blockSize(blockSize)
{
    assert(blockSize > 0);
    AddBlock();
}

DescriptorPool::~DescriptorPool()
{
    auto& handler = _renderer.GetDescriptorPoolHandler();
	for (auto& pool : _pools)
		handler.Destroy(pool);
}

VkDescriptorSet DescriptorPool::Get()
{
    if (_open.GetCount() == 0)
        AddBlock();
    return _open.Pop();
}

void DescriptorPool::Add(const VkDescriptorSet set)
{
    _open.Add(set);
}

void DescriptorPool::AddBlock()
{
    auto& handler = _renderer.GetDescriptorPoolHandler();
    const auto pool = handler.Create(_types.GetData(), _capacities.GetData(), _types.GetLength());
    _pools.Add(pool);

    const uint32_t startIndex = _open.GetCount();
    _open.Resize(startIndex + _blockSize);
    handler.CreateSets(pool, _layout, _blockSize, &_open[startIndex]);
}
