#pragma once

#include <atomic>

#include "Types.hpp"
#include "HashMap.hpp"
#include "Result.hpp"
#include "StringBase.hpp"

namespace CSTM {

	class StringView;

	enum class StringError
	{
		InvalidOffset,
		InvalidLength
	};

	class String : public StringBase
	{
		struct LargeStorage
		{
			byte* data;
			std::atomic_size_t ref_count;
			size_t hash_code;
		};

		static constexpr size_t SmallStringLength = 16 * sizeof(byte);
		inline static HashMap<size_t, LargeStorage*> StringPool;

	public:
		static String create(const char* str);

	public:
		[[nodiscard]]
		bool is_empty() const noexcept { return m_byte_count == 0; }

		[[nodiscard]]
		bool is_large_string() const noexcept { return m_byte_count > SmallStringLength; }

		[[nodiscard]]
		const byte* data() const noexcept override { return is_large_string() ? m_large_storage->data : m_small_storage; }

		[[nodiscard]]
		size_t byte_count() const noexcept override { return m_byte_count; }

		[[nodiscard]]
		bool operator==(const String& other) const noexcept;

		[[nodiscard]]
		size_t ref_count() const noexcept { return is_large_string() ? m_large_storage->ref_count.load() : 1uz; }

		[[nodiscard]]
		Result<StringView, StringError> view(size_t offset = 0, size_t length = ~0uz) const noexcept;

	public:
		String() noexcept = default;
		String(const String& other) noexcept;
		String(String&& other) noexcept;
		~String() noexcept override;

		String& operator=(const String& other) noexcept;
		String& operator=(String&& other) noexcept;

	private:
		void try_decrease_ref_count() const noexcept;
		void allocate_from(const byte* data, size_t byteCount);

		[[nodiscard]]
		byte* data_mut() { return is_large_string() ? m_large_storage->data : m_small_storage; }

	private:
		union
		{
			byte m_small_storage[SmallStringLength]{};
			LargeStorage* m_large_storage;
		};

		size_t m_byte_count = 0;
	};

}
