#pragma once
#include "pch.h"
#include "VkRenderer/StackAllocator.h"

class DefaultAllocator final : public vi::StackAllocator, public Singleton<DefaultAllocator>
{

};
