#pragma once
constexpr unsigned SWAPCHAIN_MAX_FRAMES = 3;

#define GLFW_INCLUDE_VULKAN
#include "vulkan/vulkan.h"
#include <GLFW/glfw3.h>
#include "Utilities.h"
#include "FreeListAllocator.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "glm/gtx/euler_angles.hpp"

#include <string>
#include <vector>
#include <iostream>
#include <cassert>
#include <map>
#include <set>

inline vi::FreeListAllocator GMEM{ 4096 };
inline vi::FreeListAllocator GMEM_TEMP{ 4096 };