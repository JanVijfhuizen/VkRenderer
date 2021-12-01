#pragma once
#include "pch.h"
#include "VkRenderer/StackAllocator.h"

class DefaultAllocator final : public StackAllocator, public Singleton<DefaultAllocator>
{

};
