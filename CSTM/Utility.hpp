#pragma once

#include "Types.hpp"

#include <array>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <string>
#include <cstring>
#include <optional>

#ifdef _WIN32
	#define CSTM_NoUniqueAddr [[msvc::no_unique_address]]
#else
	#define CSTM_NoUniqueAddr [[no_unique_address]]
#endif

#define CSTM_TagType(Name) struct Name { constexpr explicit Name() = default; }

namespace CSTM {

	CSTM_TagType(NullType);

	constexpr NullType Null = NullType{};

	template<typename Func, typename Return, typename... Args>
	concept SameReturn = requires(Func func, Args&&... args)
	{
		{ func(std::forward<Args>(args)...) } -> std::same_as<Return>;
	};

	template<typename Func, typename... Args>
	constexpr auto conditional_invoke_result()
	{
		if constexpr (std::is_invocable_v<Func, Args...>)
		{
			return std::invoke_result_t<Func, Args...>{};
		}
		else
		{
			return std::invoke_result_t<Func>{};
		}
	}

	// NOTE(Peter): This is a "hack" to get around std::conditional_v<...> evaluating
	// both std::invoke_result_t instantiations regardless of the condition being
	// true or false.
	// It would be nice if this could be done generically
	template<typename Func, typename... Args>
	using conditional_invoke_result_t = decltype(conditional_invoke_result<Func, Args...>());

	template<typename T>
	using Optional = std::optional<T>;
	constexpr std::nullopt_t NullOpt = std::nullopt;

	template<typename T>
	concept SizedContainer = requires(T t)
	{
		{ t.element_count() } -> std::same_as<size_t>;
		{ t.byte_count() } -> std::same_as<size_t>;
	};

	template<typename T>
	struct IsString : std::false_type {};

	template<>
	struct IsString<std::string> : std::true_type {};

	template<>
	struct IsString<std::string_view> : std::true_type {};

	template<>
	struct IsString<const char*> : std::true_type {};

	template<size_t S>
	struct IsString<char[S]> : std::true_type {};

	template<typename T>
	constexpr bool IsStringV = IsString<std::remove_cvref_t<T>>::value;

	template<SizedContainer T>
	constexpr size_t element_count(T&& t) noexcept { return std::forward<T>(t).element_count(); }

	template<SizedContainer T>
	constexpr size_t element_count(const T& t) noexcept { return t.element_count(); }

	template<SizedContainer T>
	constexpr size_t byte_count(const T& t) noexcept { return t.byte_count(); }

	template<SizedContainer T>
	constexpr size_t byte_count(T&& t) noexcept { return std::forward<T>(t).byte_count(); }

	constexpr size_t element_count(const std::string& str) noexcept { return str.length(); }
	constexpr size_t element_count(std::string_view str) noexcept { return str.length(); }
	inline size_t element_count(const char* str) noexcept { return strlen(str); }

	constexpr size_t byte_count(const std::string& str) noexcept { return str.length(); }
	constexpr size_t byte_count(std::string_view str) noexcept { return str.length(); }
	inline size_t byte_count(const char* str) noexcept { return strlen(str); }

	template<typename Type, template<typename...> typename Template>
	inline constexpr bool IsSpecializationV = false;

	template<template<typename...> typename Template, typename... Types>
	inline constexpr bool IsSpecializationV<Template<Types...>, Template> = true;

	template<typename Type, template<typename...> typename Template>
	struct IsSpecialization : std::bool_constant<IsSpecializationV<Type, Template>> {};

	inline bool is_hexadecimal(byte b) noexcept
	{
		return std::isxdigit(b) > 0;
	}

	constexpr byte from_hex(byte c)
	{
		if (c >= 'a') return 10 + c - 'a';
		if (c >= 'A') return 10 + c - 'A';
		return c - '0';
	}

	constexpr byte from_hex2(byte c0, byte c1)
	{
		return (from_hex(c0) << 4) + from_hex(c1);
	}

	constexpr uint32_t from_little_endian(const std::array<byte, 4> bytes) noexcept
	{
		const union Result
		{
			std::array<byte, 4> Bytes;
			uint32_t Value;
		} result{ bytes };

		return result.Value;
	};

}
