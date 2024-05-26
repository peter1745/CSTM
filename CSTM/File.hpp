#pragma once

#include "Result.hpp"

#include <cstdint>

namespace CSTM {

#if defined(_WIN32)
	#define NOMINMAX
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	using FileHandle = HANDLE;
	inline const FileHandle InvalidFileHandle = INVALID_HANDLE_VALUE;
#else
	using FileHandle = int;
	constexpr int InvalidFileHandle = -1;
#endif

	enum class FileError
	{
		Unknown,

		FileNotFound,
		AccessDenied,
		OpenFailed,
		InvalidFile,
		InvalidRead,
	};

	enum class FileAccess { ReadOnly, ReadWrite };

	class File
	{
	public:
		[[nodiscard]]
		Result<uint64_t, FileError> size() const;

		[[nodiscard]]
		Result<std::string, FileError> read_all_text() const;

		[[nodiscard]]
		static Result<File, FileError> open(std::string_view filepath, FileAccess access = FileAccess::ReadOnly);

	private:
		FileHandle m_handle = InvalidFileHandle;

	};

}
