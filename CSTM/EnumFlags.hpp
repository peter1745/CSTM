#pragma once

namespace Theia::Core {

	template<typename T>
	constexpr inline std::false_type enum_flags(T) { return {}; }

	template<typename T>
	concept EnumFlags = std::is_scoped_enum_v<T> && !std::same_as<std::false_type, decltype(enum_flags(std::declval<T>()))>;
}

template<Theia::Core::EnumFlags TEnum>
constexpr TEnum operator|(TEnum lhs, TEnum rhs) noexcept
{
	return static_cast<TEnum>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

template <Theia::Core::EnumFlags TEnum>
constexpr bool operator&(TEnum lhs, TEnum rhs) noexcept
{
	return (std::to_underlying(lhs) & std::to_underlying(rhs)) != 0;
}

template <Theia::Core::EnumFlags TEnum>
constexpr TEnum operator^(TEnum lhs, TEnum rhs) noexcept
{
	return static_cast<TEnum>(std::to_underlying(lhs) ^ std::to_underlying(rhs));
}

template <Theia::Core::EnumFlags TEnum>
constexpr TEnum operator~(TEnum value) noexcept
{
	return static_cast<TEnum>(~std::to_underlying(value));
}

template <Theia::Core::EnumFlags TEnum>
constexpr bool operator!(TEnum value) noexcept
{
	return !std::to_underlying(value);
}

template <Theia::Core::EnumFlags TEnum>
constexpr TEnum& operator|=(TEnum& lhs, const TEnum& rhs) noexcept
{
	return (lhs = (lhs | rhs));
}

template <Theia::Core::EnumFlags TEnum>
constexpr TEnum& operator&=(TEnum& lhs, const TEnum& rhs) noexcept
{
	return (lhs = (lhs & rhs));
}

template <Theia::Core::EnumFlags TEnum>
constexpr TEnum& operator^=(TEnum& lhs, const TEnum& rhs) noexcept
{
	return (lhs = (lhs ^ rhs));
}
