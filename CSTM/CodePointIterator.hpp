#pragma once

#include "Types.hpp"
#include "Utility.hpp"

#include <concepts>
#include <vector>

namespace CSTM {

	class StringBase;

	enum class IterAction
	{
		Break, Continue
	};

	class CodePointIteratorBase
	{
	public:
		CodePointIteratorBase(const byte* current, const byte* end);

		virtual ~CodePointIteratorBase() = default;

		[[nodiscard]]
		uint32_t current() const noexcept { return m_code_point.value; }

		virtual bool advance() = 0;

		void each(std::invocable<uint32_t> auto&& func)
		{
			using ReturnType = std::invoke_result_t<decltype(func), uint32_t>;

			if constexpr (std::same_as<ReturnType, IterAction>)
			{
				while (advance())
				{
					if (func(current()) == IterAction::Break)
					{
						break;
					}
				}
			}
			else
			{
				while (advance())
				{
					func(current());
				}
			}
		}

		void each(std::invocable<size_t, uint32_t> auto&& func)
		{
			using ReturnType = std::invoke_result_t<decltype(func), size_t, uint32_t>;

			size_t i = 0;

			if constexpr (std::same_as<ReturnType, IterAction>)
			{
				while (advance())
				{
					if (func(i, current()) == IterAction::Break)
					{
						break;
					}

					i++;
				}
			}
			else
			{
				while (advance())
				{
					func(i, current());
					i++;
				}
			}
		}

		void store(std::vector<uint32_t>& container)
		{
			each([&](const uint32_t codePoint)
			{
				container.push_back(codePoint);
			});
		}

	protected:
		void compute_current_code_point();

	protected:
		const byte* m_current;
		const byte* m_end;

		struct CodePoint
		{
			uint32_t value = ~0u;
			uint32_t byteCount = 0;
		} m_code_point{};
	};

	class CodePointIterator final : public CodePointIteratorBase
	{
	public:
		explicit CodePointIterator(const StringBase& str);

		bool advance() override;
	};

	class CodePointReverseIterator final : public CodePointIteratorBase
	{
	public:
		explicit CodePointReverseIterator(const StringBase& str);

		bool advance() override;
	};

}
