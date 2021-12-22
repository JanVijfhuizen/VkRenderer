#pragma once

class Engine final
{
public:
	struct Info final
	{
		size_t capacity = 1e3f;

		typedef void (*Awake)();
		typedef void (*Start)();
		typedef void (*Update)(bool& outQuit);
		typedef void (*Cleanup)();

		Awake awake = nullptr;
		Start start = nullptr;
		Update update = nullptr;
		Cleanup cleanup = nullptr;
	};

	static void Run(const Info& info);
};
