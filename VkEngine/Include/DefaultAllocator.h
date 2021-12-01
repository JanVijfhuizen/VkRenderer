#pragma once
#include "pch.h"
#include "StackAllocator.h"

class DefaultAllocator final : public StackAllocator, public Singleton<DefaultAllocator>
{

};
