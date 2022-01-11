#include "pch.h"
#include "Engine/Engine.h"

int main()
{
	Engine::Info info{};
	vi::UniquePtr<Engine> engine{GMEM};
	return engine->Run(info);
}
