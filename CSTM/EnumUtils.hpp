#pragma once

#include <type_traits>
#include <utility>

namespace CSTM {

	template<typename E>
	concept scoped_enum = std::is_scoped_enum_v<E>;

	template<scoped_enum E, typename = void>
	struct EnumHasMaxMember : std::false_type {};
	template<scoped_enum E>
	struct EnumHasMaxMember<E, std::void_t<decltype(E::Max)>> : std::true_type {};

	template<scoped_enum E, typename = void>
	struct EnumHasMinMember : std::false_type {};
	template<scoped_enum E>
	struct EnumHasMinMember<E, std::void_t<decltype(E::Min)>> : std::true_type {};

	template<scoped_enum E>
	struct EnumTraits
	{
		static constexpr std::underlying_type_t<E> min()
		{
			static_assert(EnumHasMinMember<E>::value, "No 'Min' member in enum");
			return std::to_underlying(E::Min);
		}

		static constexpr std::underlying_type_t<E> max()
		{
			static_assert(EnumHasMaxMember<E>::value, "No 'Max' member in enum");
			return std::to_underlying(E::Max);
		}
	};

}
