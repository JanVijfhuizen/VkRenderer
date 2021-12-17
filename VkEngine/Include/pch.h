#pragma once
#include "VkRenderer/pch.h"
#include <fstream>
#include <array>
#include <unordered_map>

#include "Cecsar.h"
#include "Singleton.h"
#include "VkRenderer/FreeListAllocator.h"

inline vi::FreeListAllocator GMEM{ 4096 };
inline vi::FreeListAllocator GMEM_TEMP{ 4096 };
