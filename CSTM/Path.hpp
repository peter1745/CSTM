#pragma once

#include <ranges>

namespace KSTM {

	class Path
	{
	public:
		using ValueType = char;
		using StringType = std::string;
		using StringViewType = std::string_view;

		enum class Format { Generic, Windows, Native };

		static constexpr ValueType GenericSeparator = '/';
		static constexpr ValueType WindowsSeparator = '\\';

		Path() = default;
		Path(const Path&) = default;
		Path(Path&&) = default;
		~Path() = default;

		Path& operator=(const Path&) = default;
		Path& operator=(Path&&) noexcept = default;

		Path(const ValueType* str)
			: m_text(str), m_format(Format::Native) {}

		Path(StringType&& str)
			: m_text(std::move(str)), m_format(Format::Native) {}

		Path(StringType&& str, Format format)
			: m_text(str), m_format(format) {}

		Path& operator=(const ValueType* str)
		{
			m_text = str;
			return *this;
		}

		Path& operator=(StringType&& str) noexcept
		{
			m_text = std::move(str);
			return *this;
		}

		bool is_empty() const noexcept { return m_text.empty(); }

		template<typename Self>
		decltype(auto) str(this Self&& self) { return std::forward_like<Self>(self.m_text); }

		template<typename Self>
		decltype(auto) view(this Self&& self) { return StringViewType(std::forward_like<Self>(self.m_text)); }

		ValueType get_preferred_separator() const
		{
			switch (m_format)
			{
			case Format::Generic: return GenericSeparator;
			case Format::Windows: return WindowsSeparator;
#if defined(_WIN32)
			case Format::Native: return WindowsSeparator;
#else
			case Format::Native: return GenericSeparator;
#endif
			}

			return GenericSeparator;
		}

		Path LexicallyNormal() const
		{
			// 1. If the path is empty, stop (normal form of an empty path is an empty path).
			if (is_empty())
			{
				return {};
			}

			StringType normalized = m_text;

			auto separator = get_preferred_separator();

			// 2. Replace each directory-separator (which may consist of multiple slashes) with a single path::preferred_separator.
			// 3. Replace each slash character in the root-name with path::preferred_separator. (We technically don't do this)

			auto NotEmpty = [](auto w) { return !std::ranges::empty(w); };

#if defined(_WIN32)

			normalized = normalized
				| std::views::split(GenericSeparator)
				| std::views::filter(NotEmpty)
				| std::views::transform([&](auto w)
				{
					return w
						| std::views::split(WindowsSeparator)
						| std::views::filter(NotEmpty)
						| std::views::join_with(separator);
				})
				| std::views::join_with(separator)
				| std::ranges::to<StringType>();
#else
			normalized = normalized
				| std::views::split(GenericSeparator)
				| std::views::filter(NotEmpty)
				| std::views::join_with(separator)
				| std::ranges::to<StringType>();
#endif

			return {std::move(normalized)};

			/*
				Remove each dot and any immediately following directory-separator.
				Remove each non-dot-dot filename immediately followed by a directory-separator and a dot-dot, along with any immediately following directory-separator.
				If there is root-directory, remove all dot-dots and any directory-separators immediately following them.
				If the last filename is dot-dot, remove any trailing directory-separator.
				If the path is empty, add a dot (normal form of ./ is .).
			*/
		}

		bool has_root_name() const
		{
			return find_root_name_end() != m_text.data();
		}

		StringViewType get_root_name() const
		{
			return StringViewType{
				m_text.data(),
				static_cast<StringViewType::size_type>(std::distance(m_text.data(), find_root_name_end()))
			};
		}

	private:
		const ValueType* find_root_name_end() const
		{
			if (m_text.length() < 2)
			{
				// No root name
				return m_text.data();
			}

#if defined(_WIN32)
			constexpr bool IsWindowsPlatform = true;
#else
			constexpr bool IsWindowsPlatform = false;
#endif

			bool isWindowsFormat = m_format == Format::Windows || (m_format == Format::Native && IsWindowsPlatform);

			if (isWindowsFormat)
			{
				// NOTE(Peter): Implementation based on Microsofts
				// implementation "_Has_drive_letter_prefix" from filesystem header
				// except we only handle normal drives, no special paths (yet)
				auto hasDriveLetterPrefix = [&]
				{
					if (m_text.length() < 2)
					{
						return false;
					}

					auto f = std::toupper(m_text[0]);
					return f >= 65 && f <= 90 && m_text[1] == ':';
				};

				if (!hasDriveLetterPrefix())
				{
					return m_text.data();
				}

				return std::next(m_text.data(), 2);
			}
			else
			{
				if (m_text[0] != GenericSeparator)
				{
					return m_text.data();
				}

				if (m_text.length() < 2 || m_text[1] != m_text[0])
				{
					return m_text.data();
				}

				if (m_text.length() == 2)
				{
					return std::next(m_text.data(), 2);
				}

				if (m_text[2] == GenericSeparator)
				{
					return m_text.data();
				}

				auto separatorPos = m_text.find_first_of(GenericSeparator);

				if (separatorPos == StringType::npos)
				{
					return std::next(m_text.data(), m_text.length());
				}

				return std::next(m_text.data(), separatorPos);
			}
		}

	private:
		std::string m_text;
		Format m_format;
	};

}
