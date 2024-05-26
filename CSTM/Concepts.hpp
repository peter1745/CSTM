#pragma once

namespace CSTM {

	template<typename Func, typename Return, typename... Args>
	concept HasReturnType = requires(Func func, Args&&... args)
	{
		{ func(std::forward<Args>(args)...) } -> std::same_as<Return>;
	};

	template<typename T>
	concept SizedContainer = requires(T t)
	{
		{ t.element_count() } -> std::same_as<size_t>;
		{ t.byte_count() } -> std::same_as<size_t>;
	};

}
