#pragma once

#include <iostream>
#include <cassert>
#include <cstdint>

#define GLFW_INCLUDE_VULKAN
#include "vulkan/vulkan.h"
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "glm/gtx/euler_angles.hpp"

#include "FreeListAllocator.h"

const size_t GMEM_SIZE = 65536;

/// <summary>
/// Memory allocator used for long term allocations.
/// </summary>
inline vi::FreeListAllocator GMEM{ GMEM_SIZE };
/// <summary>
/// Memory allocator used for long term volatile allocations.
/// </summary>
inline vi::FreeListAllocator GMEM_VOL{ GMEM_SIZE };
/// <summary>
/// Memory allocator used for quick temporary allocations.
/// </summary>
inline vi::FreeListAllocator GMEM_TEMP{ GMEM_SIZE };

#include "Iterator.h"
#include "Utilities.h"
#include "ArrayPtr.h"
#include "ViVector.h"
#include "BinTree.h"
#include "HashMap.h"
#include "ViString.h"
#include "CStrRef.h"