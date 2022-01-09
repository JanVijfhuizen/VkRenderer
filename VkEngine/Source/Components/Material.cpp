#include "pch.h"
#include "Components/Material.h"
#include "VkRenderer/VkCore/VkCore.h"

Material::System::System(ce::Cecsar& cecsar, vi::VkCore& core) : 
	ce::System<Material>(cecsar), _core(core)
{
	
}
