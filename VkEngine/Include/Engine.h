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

		Awake awake = nullptr;
		Start start = nullptr;
		Update update = nullptr;
	};

	static void Run(const Info& info);
};
