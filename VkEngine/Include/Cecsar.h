#pragma once

class Cecsar final
{
public:
	class ISystem
	{
	public:
		explicit ISystem(Cecsar& cecsar);
		virtual void EraseAt(uint32_t index) = 0;

	protected:
		[[nodiscard]] Cecsar* GetCecsar() const;

	private:
		Cecsar& _cecsar;
	};

	struct Entity final
	{
		friend Cecsar;

		[[nodiscard]] operator bool() const;
		[[nodiscard]] bool operator ==(const Entity& other) const;
		[[nodiscard]] operator uint32_t() const;

	private:
		uint32_t _identifier = 0;
		uint32_t _index;
	};

	void SubscribeSystem(ISystem* system);

private:
	vi::Vector<ISystem*> _systems{4, GMEM_VOL};
};
