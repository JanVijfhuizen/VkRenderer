#include "pch.h"
#include "Rendering/DefaultMaterial.h"

DefaultMaterial::System::System(const uint32_t size) : SparseSet<DefaultMaterial>(size)
{
}

DefaultMaterial::System::~System()
{
}
