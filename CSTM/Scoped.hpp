#pragma once

#include "Utility.hpp"

namespace KSTM {

	template<typename T>
	struct ScopedValue
	{
		T value;
	};

	template<>
	struct ScopedValue<NullTag> {};

	template<typename T, typename D>
	class ScopedT
	{
	public:
		ScopedT() = default;
		ScopedT(ScopedT&&) = delete;
		ScopedT(const ScopedT&) = delete;

		~ScopedT()
		{
			if (m_dismissed)
			{
				return;
			}

			if constexpr (!std::same_as<D, NullTag>)
			{
				if constexpr (!std::same_as<T, NullTag>)
				{
					m_deferred.value(m_value.value);
				}
				else
				{
					m_deferred.value();
				}
			}
		}

		template<typename Func>
			requires std::invocable<Func>
		auto init_with(Func&& func)
		{
			auto value = std::invoke(std::forward<Func>(func));
			return ScopedT<decltype(value), D>{
				ScopedValue{value},
				std::move(m_deferred)
			};
		}

		template<typename T2>
		auto init_with(T2&& value)
		{
			return ScopedT<decltype(value), D>{
				ScopedValue{value},
				std::move(m_deferred)
			};
		}

		template<typename D2>
		auto defer(D2&& func)
		{
			return ScopedT<T, D2>{
				std::move(m_value),
				ScopedValue{std::forward<D2>(func)}
			};
		}

		void dismiss()
		{
			m_dismissed = true;
		}

		template<typename Self>
			requires(!std::same_as<T, NullTag>)
		decltype(auto) value(this Self&& self)
		{
			return std::forward_like<Self>(self.m_value.value);
		}

	private:
		ScopedT(const ScopedValue<T>& value, const ScopedValue<D>& deferred)
			: m_value(value), m_deferred(deferred) {}

	private:
		HeliosNoUniqueAddr ScopedValue<T> m_value;
		HeliosNoUniqueAddr ScopedValue<D> m_deferred;
		bool m_dismissed = false;

		friend struct Scoped;

		template<typename T2, typename D2>
		friend class ScopedT;
	};

	struct Scoped
	{
		Scoped() = default;
		Scoped(Scoped&&) = delete;
		Scoped(const Scoped&) = delete;

		template<typename Func>
			requires std::invocable<Func>
		auto init_with(Func&& func)
		{
			auto value = std::invoke(std::forward<Func>(func));
			return ScopedT<decltype(value), NullTag>{
				ScopedValue{value},
				ScopedValue<NullTag>{}
			};
		}

		template<typename T>
		auto init_with(T&& value)
		{
			return ScopedT<T, NullTag>{
				ScopedValue{value},
				ScopedValue<NullTag>{}
			};
		}

		template<typename Func>
		auto defer(Func&& func)
		{
			return ScopedT<NullTag, Func>{
				ScopedValue<NullTag>{},
				ScopedValue{func}
			};
		}
	};

}
