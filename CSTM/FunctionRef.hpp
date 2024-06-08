#pragma once

#include <variant>

namespace CSTM {

	using FunctionStorageType = void(*)();
	using FunctionRefStorage = std::variant<std::monostate, void*, const void*, FunctionStorageType>;

	template<typename T>
	constexpr static decltype(auto) get_function_ref_target(FunctionRefStorage& storage)
	{
		if constexpr(std::is_const_v<T>)
		{
			return static_cast<T*>(*std::get_if<const void*>(&storage));
		}
		else if constexpr (std::is_object_v<T>)
		{
			return static_cast<T*>(*std::get_if<void*>(&storage));
		}
		else if constexpr (std::is_function_v<T>)
		{
			return reinterpret_cast<T*>(*std::get_if<FunctionStorageType>(&storage));
		}
		else
		{
			return reinterpret_cast<T*>(*std::get_if<void*>(&storage));
		}
	}

	template<typename T>
	class FunctionRef;

	template<auto>
	struct nontype_t { explicit nontype_t() = default; };

	template<auto func>
	constexpr nontype_t<func> nontype{};

	template<typename Ret, typename... Args>
	class FunctionRef<Ret(Args...)>
	{
	public:
		FunctionRef() = default;

		template<typename F>
		FunctionRef(F* func) noexcept
		requires(std::is_function_v<F>)
		{
			assign<F>(func);
		}

		template<typename T, typename U = std::remove_reference_t<T>>
		requires(!std::is_member_pointer_v<T>)
		FunctionRef(T&& lambda)
		{
			assign<T>(std::forward<T>(lambda));
		}

		template<auto func, typename T, typename U = std::remove_reference_t<T>>
		FunctionRef(nontype_t<func>, T&& instance)
		{
			assign<func, T>(std::forward<T>(instance));
		}

		[[nodiscard]]
		bool is_empty() const { return std::holds_alternative<std::monostate>(m_target); }

		[[nodiscard]]
		operator bool() const { return !is_empty(); }

		template<typename F>
		requires(std::is_function_v<F>)
		void assign(F* func)
		{
			m_target = reinterpret_cast<FunctionStorageType>(func);
			m_callable = [](FunctionRefStorage& obj, Args&&... args)
			{
				if constexpr (std::is_void_v<Ret>)
				{
					get_function_ref_target<F>(obj)(std::forward<Args>(args)...);
				}
				else
				{
					return get_function_ref_target<F>(obj)(std::forward<Args>(args)...);
				}
			};
		}

		template<typename T, typename U = std::remove_reference_t<T>>
		requires(!std::is_member_pointer_v<T>)
		void assign(T&& lambda)
		{
			m_target = std::addressof(lambda);

			m_callable = [](FunctionRefStorage& obj, Args&&... args)
			{
				decltype(auto) i = *get_function_ref_target<U>(obj);

				if constexpr (std::is_void_v<Ret>)
				{
					i(std::forward<Args>(args)...);
				}
				else
				{
					return i(std::forward<Args>(args)...);
				}
			};
		}

		template<auto func, typename T, typename U = std::remove_reference_t<T>>
		void assign(T&& instance)
		{
			m_target = std::addressof(instance);
			m_callable = [](FunctionRefStorage& obj, Args&&... args)
			{
				decltype(auto) i = *get_function_ref_target<U>(obj);

				if constexpr (std::is_void_v<Ret>)
				{
					std::invoke(func, i, std::forward<Args>(args)...);
				}
				else
				{
					return std::invoke(func, i, std::forward<Args>(args)...);
				}
			};
		}

		template<auto func, typename T, typename U = std::remove_reference_t<T>>
		void assign(nontype_t<func>, T&& instance)
		{
			m_target = std::addressof(instance);
			m_callable = [](FunctionRefStorage& obj, Args&&... args)
			{
				decltype(auto) i = *get_function_ref_target<U>(obj);

				if constexpr (std::is_void_v<Ret>)
				{
					std::invoke(func, i, std::forward<Args>(args)...);
				}
				else
				{
					return std::invoke(func, i, std::forward<Args>(args)...);
				}
			};
		}

		[[nodiscard]]
		Ret operator()(Args&&... args)
		{
			return m_callable(m_target, std::forward<Args>(args)...);
		}

	private:
		FunctionRefStorage m_target{};

		using Callable = Ret(*)(FunctionRefStorage&, Args&&...);
		Callable m_callable;
	};

}
