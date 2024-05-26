#pragma once

#include "Utility.hpp"

#include <concepts>

namespace KSTM {

	template<typename... Rest>
	class Tuple;

}

namespace std {

	template<typename... Types>
	struct tuple_size<Theia::Core::Tuple<Types...>> : std::integral_constant<size_t, sizeof...(Types)> {};

	template<typename First, typename... Rest>
	struct tuple_element<0, Theia::Core::Tuple<First, Rest...>>
	{
		using type = First;
	};

	template<size_t Idx, typename First, typename... Rest>
	struct tuple_element<Idx, Theia::Core::Tuple<First, Rest...>> : tuple_element<Idx - 1, Theia::Core::Tuple<Rest...>> {};

}

namespace Theia::Core {

	struct ExactArgsTag
	{
		explicit ExactArgsTag() = default;
	};

	struct UnpackTupleTag
	{
		explicit UnpackTupleTag() = default;
	};

	template<size_t Idx, typename T>
	struct TupleElement;

	template<size_t Idx, typename T>
	struct TupleElement<Idx, const T>
	{
		using Type = std::add_const_t<typename TupleElement<Idx, T>::Type>;
	};

	template<size_t Idx, typename T>
	using TupleElementT = typename TupleElement<Idx, T>::Type;

	template<size_t Idx>
	struct TupleElement<Idx, Tuple<>> {};

	template<typename First, typename... Rest>
	struct TupleElement<0, Tuple<First, Rest...>>
	{
		using Type = First;
		using TupleType = Tuple<First, Rest...>;
	};

	template<size_t Idx, typename First, typename... Rest>
	struct TupleElement<Idx, Tuple<First, Rest...>> : TupleElement<Idx - 1, Tuple<Rest...>> {};

	template<>
	class Tuple<>
	{
	public:
		constexpr Tuple() noexcept = default;

		template<std::same_as<ExactArgsTag> Tag>
		constexpr Tuple(Tag) noexcept {}

		constexpr size_t element_count() const noexcept { return 0; }
		constexpr size_t byte_count() const noexcept { return 0; }
	};

	template<typename First, typename... Rest>
	class Tuple<First, Rest...> : Tuple<Rest...>
	{
		using Base = Tuple<Rest...>;

	public:
		constexpr Tuple() noexcept = default;

		template<std::same_as<ExactArgsTag> Tag, typename First2, typename... Rest2>
		constexpr Tuple(Tag, First2&& first, Rest2&&... rest) noexcept
			: Base(ExactArgsTag{}, std::forward<Rest2>(rest)...), m_value(std::forward<First2>(first)) {}

		template<std::same_as<UnpackTupleTag> Tag, typename T, size_t... Indices>
		constexpr Tuple(Tag, T&& other, std::index_sequence<Indices...>);

		template<std::same_as<UnpackTupleTag> Tag, typename T>
		constexpr Tuple(Tag, T&& other)
			: Tuple(UnpackTupleTag{}, std::forward<T>(other), std::make_index_sequence<other.element_count()>{}) {}

		constexpr explicit Tuple(const First& first, const Rest&... rest) noexcept
			: Tuple(ExactArgsTag{}, first, rest...) {}

		template<typename First2, typename... Rest2>
		constexpr Tuple(const First2& first, const Rest2&... rest) noexcept
			: Tuple(ExactArgsTag{}, first, rest...) {}

		Tuple(const Tuple&) = default;
		Tuple(Tuple&&) = default;

		template<typename... Others>
		constexpr Tuple(const Tuple<Others...>& other) noexcept
			: Tuple(UnpackTupleTag{}, other) {}

		constexpr size_t element_count() const noexcept { return 1 + Base::element_count(); }

		constexpr size_t byte_count() const noexcept
		{
			if constexpr (IsStringV<First>)
			{
				return Theia::Core::byte_count(m_value) + Base::byte_count();
			}
			else
			{
				return sizeof(First) + Base::byte_count();
			}
		}

		template<size_t Idx>
		constexpr TupleElementT<Idx, Tuple>& at() & noexcept
		{
			using TupleType = typename TupleElement<Idx, Tuple>::TupleType;
			return static_cast<TupleType&>(*this).m_value;
		}

		template<size_t Idx>
		constexpr const TupleElementT<Idx, Tuple>& at() const & noexcept
		{
			using TupleType = typename TupleElement<Idx, Tuple>::TupleType;
			return static_cast<const TupleType&>(*this).m_value;
		}

		template<size_t Idx>
		constexpr TupleElementT<Idx, Tuple>&& at() && noexcept
		{
			using ElemType = TupleElementT<Idx, Tuple>;
			using TupleType = typename TupleElement<Idx, Tuple>::TupleType;
			return static_cast<ElemType&&>(static_cast<TupleType&>(*this).m_value);
		}

		template<size_t Idx>
		constexpr const TupleElementT<Idx, Tuple>&& at() const && noexcept
		{
			using ElemType = TupleElementT<Idx, Tuple>;
			using TupleType = typename TupleElement<Idx, Tuple>::TupleType;
			return static_cast<const ElemType&&>(static_cast<TupleType&>(*this).m_value);
		}

		template<size_t Idx>
		constexpr TupleElementT<Idx, Tuple>& get() & noexcept
		{
			return at<Idx>();
		}

		template<size_t Idx>
		constexpr const TupleElementT<Idx, Tuple>& get() const & noexcept
		{
			return at<Idx>();
		}

		template<size_t Idx>
		constexpr TupleElementT<Idx, Tuple>&& get() && noexcept
		{
			return std::forward<TupleElementT<Idx, Tuple>>(at<Idx>());
		}

		template<size_t Idx>
		constexpr const TupleElementT<Idx, Tuple>&& get() const && noexcept
		{
			return std::forward<TupleElementT<Idx, Tuple>>(at<Idx>());
		}

	private:
		First m_value;

		template<typename... Types>
		friend class Tuple;
	};

	template<typename... Types>
	constexpr auto tuple_indices(const Tuple<Types...>&) noexcept
	{
		return std::make_index_sequence<sizeof...(Types)>{};
	}

	template<typename... Types>
	Tuple<std::unwrap_reference_t<Types>...> make_tuple(Types&&... values)
	{
		return Tuple<std::unwrap_reference_t<Types>...>(std::forward<Types>(values)...);
	}

	template<typename... Types>
	Tuple<Types&&...> forward_as_tuple(Types&&... values)
	{
		return Tuple<Types&&...>(std::forward<Types>(values)...);
	}

	template<size_t Idx, typename... Types>
	constexpr TupleElementT<Idx, Tuple<Types...>>& at(Tuple<Types...>& tuple) noexcept
	{
		return tuple.template at<Idx>();
	}

	template<size_t Idx, typename... Types>
	constexpr const TupleElementT<Idx, Tuple<Types...>>& at(const Tuple<Types...>& tuple) noexcept
	{
		return tuple.template at<Idx>();
	}

	template<size_t Idx, typename... Types>
	constexpr TupleElementT<Idx, Tuple<Types...>>&& at(Tuple<Types...>&& tuple) noexcept
	{
		return std::forward(tuple).template at<Idx>();
	}

	template<size_t Idx, typename... Types>
	constexpr const TupleElementT<Idx, Tuple<Types...>>&& at(const Tuple<Types...>&& tuple) noexcept
	{
		return std::forward(tuple).template at<Idx>();
	}

	template<typename First, typename... Rest>
	template<std::same_as<UnpackTupleTag> Tag, typename T, size_t... Indices>
	constexpr Tuple<First, Rest...>::Tuple(Tag, T&& other, std::index_sequence<Indices...>)
		: Tuple(ExactArgsTag{}, Theia::Core::at<Indices>(std::forward<T>(other))...) {}
}
