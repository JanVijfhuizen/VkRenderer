#include "pch.h"
#include "Rendering/DescriptorPool.h"
#include "Rendering/Renderer.h"

void DescriptorPool::Construct(
    Renderer& renderer, const VkDescriptorSetLayout layout, VkDescriptorType* types,
	uint32_t* capacities, const uint32_t typeCount, const uint32_t blockSize)
{
    _renderer = &renderer;
    _layout = layout;
    _types = vi::ArrayPtr<VkDescriptorType>{ types, typeCount, GMEM_VOL };
    _capacities = vi::ArrayPtr<uint32_t>{ capacities, typeCount, GMEM_VOL };
    _blockSize = blockSize;

    assert(blockSize > 0);
    AddBlock();
}

void DescriptorPool::Cleanup()
{
    auto& handler = _renderer->GetDescriptorPoolHandler();
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
    auto& handler = _renderer->GetDescriptorPoolHandler();
    const auto pool = handler.Create(_types.GetData(), _capacities.GetData(), _types.GetLength());
    _pools.Add(pool);

    const uint32_t startIndex = _open.GetCount();
    _open.Resize(startIndex + _blockSize);
    handler.CreateSets(pool, _layout, _blockSize, &_open[startIndex]);
}
