#include "pch.h"
#include "Rendering/DescriptorPool.h"
#include "Rendering/VulkanRenderer.h"
#include "VkRenderer/VkHandlers/VkDescriptorPoolHandler.h"

void DescriptorPool::Construct(
    VulkanRenderer& renderer, const VkDescriptorSetLayout layout, VkDescriptorType* types,
	uint32_t* capacities, const uint32_t typeCount, const uint32_t blockSize)
{
    _renderer = &renderer;
    _layout = layout;
    // Copy types and capacities to volatile memory.
    // Even though these arrays are not volatile at all, it's better to have them close to the actual volatile part of the pool.
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

    // Create a new descriptor based on the requirements.
    vi::VkDescriptorPoolHandler::PoolCreateInfo poolCreateInfo{};
    poolCreateInfo.types = _types.GetData();
    poolCreateInfo.capacities = _capacities.GetData();
    poolCreateInfo.typeCount = _types.GetLength();
    const auto pool = handler.Create(poolCreateInfo);
    _pools.Add(pool);

    const uint32_t startIndex = _open.GetCount();
    _open.Resize(startIndex + _blockSize);

    vi::VkDescriptorPoolHandler::SetCreateInfo setCreateInfo{};
    setCreateInfo.pool = pool;
    setCreateInfo.layout = _layout;
    setCreateInfo.setCount = _blockSize;
    setCreateInfo.outSets = &_open[startIndex];
    handler.CreateSets(setCreateInfo);
}
