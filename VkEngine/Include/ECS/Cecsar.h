#pragma once
#include "SparseSet.h"
#include "HashSet.h"

namespace ce
{
	class Cecsar;

	/// <summary>
	/// ECS structure. An entity can have any number of components.
	/// </summary>
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

	/// <summary>
	/// ECS Interface. Can be used to link a component system to the ECS.
	/// </summary>
	class ISystem
	{
	public:
		explicit ISystem(Cecsar& cecsar);
		virtual void RemoveAt(uint32_t index) = 0;

	protected:
		[[nodiscard]] Cecsar& GetCecsar() const;

	private:
		Cecsar& _cecsar;
	};

	/// <summary>
	/// Cecsar is an Entity Component System (ECS) that focuses on avoiding data fragmentation as much as possible.
	/// </summary>
	class Cecsar final : public SparseSet<Entity>
	{
	public:
		explicit Cecsar(size_t capacity);

		/// <summary>
		/// When a system is subscribed, they can properly function in the ECS environment.<br>
		/// The main upside to subscribing is that when an entity is destroyed, the subscribed systems automatically remove any related component.
		/// </summary>
		void SubscribeSystem(ISystem* system);
		void RemoveAt(uint32_t sparseIndex) override;

	private:
		vi::Vector<ISystem*> _systems{ 32, GMEM_VOL };
	};

	/// <summary>
	/// The default component system used in Cecsar.
	/// </summary>
	template <typename T>
	class System : public ISystem, public SparseSet<T>
	{
	public:
		explicit System(Cecsar& cecsar);
		virtual void RemoveAt(uint32_t index);
	};

	/// <summary>
	/// An alternative component system used in Cecsar.<br> 
	/// Takes up less space than the standard system but lookup is slower.<br>
	/// Very useful for components that are rare (think: camera, boss behaviour, etc.)
	/// </summary>
	template <typename T>
	class SmallSystem : public ISystem, public HashSet<T>
	{
		explicit SmallSystem(Cecsar& cecsar, size_t size);
		virtual void RemoveAt(uint32_t index);
	};

	template <typename T>
	System<T>::System(Cecsar& cecsar) : ISystem(cecsar), SparseSet<T>(cecsar.GetLength())
	{
	}

	template <typename T>
	void System<T>::RemoveAt(const uint32_t index)
	{
		SparseSet<T>::RemoveAt(index);
	}

	template <typename T>
	SmallSystem<T>::SmallSystem(Cecsar& cecsar, const size_t size) : ISystem(cecsar), HashSet<T>(size)
	{

	}

	template <typename T>
	void SmallSystem<T>::RemoveAt(const uint32_t index)
	{
		HashSet<T>::RemoveAt(index);
	}
}