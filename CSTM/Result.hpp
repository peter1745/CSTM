#pragma once

#include <bit>

#include "Utility.hpp"

#include <functional>

namespace CSTM {

	enum class ResultStorageType
	{
		None,
		Value,
		Reference,
		Pointer
	};

	CSTM_TagType(ResultValueTag);
	CSTM_TagType(ResultErrorTag);

	template<typename T>
	class ResultValue
	{
	public:
		using Value = std::conditional_t<std::is_lvalue_reference_v<T>, std::add_pointer_t<std::remove_reference_t<T>>, T>;

		constexpr ResultValue() noexcept(std::is_nothrow_constructible_v<Value>)
			: m_bytes() {}

		constexpr ResultValue(const ResultValue& other)
			requires (std::is_copy_constructible_v<T>)
			: m_value(other.m_value)
		{
		}

		constexpr explicit ResultValue(T v) noexcept
			requires(std::is_lvalue_reference_v<T>)
			: m_value(&v)
		{
		}

		constexpr explicit ResultValue(T v) noexcept
			requires(!std::is_lvalue_reference_v<T>)
			: m_value(v)
		{
		}

		constexpr ~ResultValue()
		{
			if constexpr (!std::is_trivially_destructible_v<Value>)
			{
				m_value.~Value();
			}
		}

		~ResultValue() noexcept requires(std::is_trivially_destructible_v<T>) = default;

		[[nodiscard]]
		constexpr decltype(auto) value(this auto&& self)
		{
			using Self = decltype(self);

			if constexpr (std::is_lvalue_reference_v<T>)
			{
				return *std::forward_like<Self>(self.m_value);
			}
			else
			{
				return std::forward_like<Self>(self.m_value);
			}
		}

	private:
		union
		{
			std::byte m_bytes[sizeof(Value)];
			Value m_value;
		};

	};

	template<typename V, typename E>
	class Result;

	template<typename Fn, typename Arg>
	constexpr auto invoke_with_optional_arg(Fn&& fn, Arg&& arg)
	{
		if constexpr (std::is_invocable_v<Fn, Arg>)
		{
			return std::invoke(
				std::forward<Fn>(fn),
				std::forward<Arg>(arg)
			);
		}
		else
		{
			return std::invoke(std::forward<Fn>(fn));
		}
	}

	template<typename V, typename E>
	class BasicResult
	{
	public:
		constexpr BasicResult() noexcept(std::is_nothrow_constructible_v<V>)
			: m_value() {}

		constexpr BasicResult(const BasicResult& other) noexcept
			requires (std::is_copy_constructible_v<V> && std::is_copy_constructible_v<E>)
			: m_value(other.m_value), m_state(other.m_state) {}

		constexpr ~BasicResult()
		{
			if constexpr (!std::is_trivially_destructible_v<E>)
			{
				if (m_state == State::Error)
				{
					m_error.~E();
				}
			}
		}

		~BasicResult() noexcept requires(std::is_trivially_destructible_v<V> && std::is_trivially_destructible_v<E>) = default;

		[[nodiscard]]
		constexpr ResultStorageType storage_type() const noexcept
		{
			if constexpr (std::is_lvalue_reference_v<V>)
			{
				return ResultStorageType::Reference;
			}

			if constexpr (std::is_pointer_v<V>)
			{
				return ResultStorageType::Pointer;
			}

			return ResultStorageType::Value;
		}

		[[nodiscard]]
		constexpr bool has_value() const noexcept { return m_state == State::Value; }

		[[nodiscard]]
		constexpr bool has_error() const noexcept { return m_state == State::Error; }

		[[nodiscard]]
		constexpr bool is_empty() const noexcept { return m_state == State::Empty; }

		[[nodiscard]]
		constexpr decltype(auto) value(this auto&& self)
		{
			using Self = decltype(self);
			return std::forward_like<Self>(self.m_value).value();
		}

		[[nodiscard]]
		constexpr decltype(auto) value_or(this auto&& self, V defaultValue)
		{
			using Self = decltype(self);

			switch (std::forward_like<Self>(self.m_state))
			{
			case State::Value:
				return std::forward_like<Self>(self.m_value).value();
			default:
				return std::forward_like<Self>(defaultValue);
			}
		}

		[[nodiscard]]
		constexpr decltype(auto) error(this auto&& self)
		{
			using Self = decltype(self);
			return std::forward_like<Self>(self.m_error);
		}

		[[nodiscard]]
		constexpr decltype(auto) error_or(this auto&& self, E defaultError)
		{
			using Self = decltype(self);

			switch (std::forward_like<Self>(self.m_state))
			{
			case State::Error:
				return std::forward_like<Self>(self.m_error);
			default:
				return std::forward_like<Self>(defaultError);
			}
		}

		template<typename SuccessFunc, typename FailureFunc>
		auto match(this auto&& self, SuccessFunc&& successFunc, FailureFunc&& failureFunc)
		{
			using Self = decltype(self);

			auto invoke = [&]<typename Func, typename Arg>(Func&& func, Arg&& arg)
			{
				using FnRet = std::remove_cv_t<conditional_invoke_result_t<Func, Arg>>;

				if constexpr (std::is_void_v<FnRet>)
				{
					invoke_with_optional_arg(std::forward<Func>(func), std::forward<Arg>(arg));
					return std::forward<Self>(self);
				}
				else
				{
					return invoke_with_optional_arg(std::forward<Func>(func), std::forward<Arg>(arg));
				}
			};

			switch (std::forward_like<Self>(self.m_state))
			{
			case State::Value:
				return invoke(std::forward<SuccessFunc>(successFunc), std::forward_like<Self>(self.m_value).value());
			case State::Error:
				return invoke(std::forward<FailureFunc>(failureFunc), std::forward_like<Self>(self.m_error));
			default:
				return std::forward<Self>(self);
			}
		}

		template<typename Func>
		constexpr auto and_then(this auto&& self, Func&& func)
		{
			using Self = decltype(self);
			using ValueType = typename ResultValue<V>::Value;
			using FnRet = std::remove_cv_t<conditional_invoke_result_t<Func, ValueType>>;
			using Ret = std::conditional_t<std::is_void_v<FnRet>, NullType, FnRet>;

			switch (std::forward_like<Self>(self.m_state))
			{
			case State::Value:
			{
				if constexpr (std::same_as<Ret, NullType>)
				{
					invoke_with_optional_arg(
						std::forward<Func>(func),
						std::forward_like<Self>(self.m_value).value()
					);

					return Result<Ret, E>{ Null };
				}
				else
				{
					return Result<Ret, E>{
						invoke_with_optional_arg(
							std::forward<Func>(func),
							std::forward_like<Self>(self.m_value).value()
						)
					};
				}
			}
			case State::Error:
				return Result<Ret, E>{ ResultErrorTag{}, std::forward_like<Self>(self.m_error) };
			default:
				return Result<Ret, E>{};
			}
		}

		template<typename Func>
		constexpr auto or_else(this auto&& self, Func&& func)
		{
			using Self = decltype(self);
			using FnRet = std::remove_cv_t<conditional_invoke_result_t<Func, E>>;
			using Ret = std::conditional_t<std::is_void_v<FnRet>, NullType, FnRet>;

			switch (std::forward_like<Self>(self.m_state))
			{
			case State::Value:
				return Result<V, Ret>{ std::forward_like<Self>(self.m_value).value() };
			case State::Error:
			{
				if constexpr (std::same_as<Ret, NullType>)
				{
					invoke_with_optional_arg(
						std::forward<Func>(func),
						std::forward_like<Self>(self.m_error)
					);

					return Result<V, Ret>{ ResultErrorTag{}, Null };
				}
				else
				{
					return Result<V, Ret>{
						ResultErrorTag{},
						invoke_with_optional_arg(
							std::forward<Func>(func),
							std::forward_like<Self>(self.m_error)
						)
					};
				}
			}
			default:
				return Result<V, Ret>{};
			}
		}

		template<typename Exception, typename... ExceptionArgs>
		requires std::constructible_from<Exception, ExceptionArgs...>
		constexpr auto throw_on_error(this auto&& self, ExceptionArgs&&... args)
		{
			using Self = decltype(self);

			switch (std::forward_like<Self>(self.m_state))
			{
			case State::Value:
				return Result<V, E>{ std::forward_like<Self>(self.m_value).value() };
			case State::Error:
				throw Exception(std::forward<ExceptionArgs>(args)...);
			default:
				return Result<V, E>{};
			}
		}

		template<typename Exception, typename... ExceptionArgs>
		requires std::constructible_from<Exception, ExceptionArgs...>
		constexpr auto throw_on_value(this auto&& self, ExceptionArgs&&... args)
		{
			using Self = decltype(self);

			switch (std::forward_like<Self>(self.m_state))
			{
			case State::Value:
				throw Exception(std::forward<ExceptionArgs>(args)...);
			case State::Error:
				return Result<V, E>{ ResultErrorTag{}, std::forward_like<Self>(self.m_error) };
			default:
				return Result<V, E>{};
			}
		}

	protected:
		constexpr BasicResult(ResultValueTag, V value) noexcept
			: m_value(value), m_state(State::Value) {}

		constexpr BasicResult(ResultErrorTag, const E& err) noexcept
			: m_error(err), m_state(State::Error) {}

	private:
		union
		{
			ResultValue<V> m_value;
			E m_error;
		};

		enum class State : uint8_t
		{
			Empty, Value, Error
		};

		State m_state = State::Empty;
	};

	template<typename V, typename E>
	class Result : public BasicResult<V, E>
	{
		using Base = BasicResult<V, E>;

	public:
		constexpr Result() noexcept(std::is_nothrow_constructible_v<V>)
			: Base() {}

		constexpr Result(V value) noexcept
			: Base(ResultValueTag{}, value) {}

		constexpr Result(const E& error) noexcept
			: Base(ResultErrorTag{}, error) {}

		constexpr Result(ResultErrorTag, const E& error) noexcept
			: Base(ResultErrorTag{}, error) {}
	};

	template<typename T>
	class Result<T, T> : public BasicResult<T, T>
	{
		using Base = BasicResult<T, T>;

	public:
		constexpr Result() noexcept(std::is_nothrow_constructible_v<T>)
			: Base() {}

		constexpr Result(T value) noexcept
			: Base(ResultValueTag{}, value) {}

		constexpr Result(ResultErrorTag, const T& error) noexcept
			: Base(ResultErrorTag{}, error) {}
	};


}
