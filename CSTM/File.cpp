#include "File.hpp"

#if !defined(_WIN32)
	#include <fcntl.h>
	#include <unistd.h>
	#include <cerrno>
#endif

namespace CSTM {

#if defined(_WIN32)
	constexpr FileError win32_file_error(DWORD error)
	{
		switch (error)
		{
		case ERROR_FILE_NOT_FOUND: return FileError::FileNotFound;
		case ERROR_PATH_NOT_FOUND: return FileError::FileNotFound;
		case ERROR_ACCESS_DENIED: return FileError::AccessDenied;
		case ERROR_OPEN_FAILED: return FileError::OpenFailed;
		case ERROR_INVALID_HANDLE: return FileError::InvalidFile;
		default: return FileError::Unknown;
		}
	}

	constexpr DWORD win32_access_flags(FileAccess access)
	{
		switch (access)
		{
		case FileAccess::ReadOnly: return GENERIC_READ;
		case FileAccess::ReadWrite: return GENERIC_READ | GENERIC_WRITE;
		default: return 0;
		}
	}

	constexpr DWORD win32_file_share_mode(FileAccess access)
	{
		switch (access)
		{
		case FileAccess::ReadOnly: return FILE_SHARE_READ | FILE_SHARE_WRITE;
		case FileAccess::ReadWrite: return FILE_SHARE_READ;
		default: return 0;
		}
	}
#else
	constexpr FileError linux_file_error(int error)
	{
		switch (error)
		{
		case ENOENT: return FileError::FileNotFound;
		case EACCES: return FileError::AccessDenied;
		case EBADF: return FileError::InvalidFile;
		default: return FileError::Unknown;
		}
	}

	constexpr int linux_access_flags(FileAccess access)
	{
		switch (access)
		{
		case FileAccess::ReadOnly: return O_RDONLY;
		case FileAccess::ReadWrite: return O_RDWR;
		default: return 0;
		}
	}
#endif

	constexpr FileError last_file_error()
	{
#if defined(_WIN32)
			return win32_file_error(GetLastError());
#else
		return linux_file_error(errno);
#endif
	}

	Result<uint64_t, FileError> File::size() const
	{
#if defined(_WIN32)
		LARGE_INTEGER size;

		if (!GetFileSizeEx(m_handle, &size))
		{
			return last_file_error();
		}

		return static_cast<uint64_t>(size.QuadPart);
#else
		off_t size = lseek(m_handle, 0, SEEK_END);

		if (size == -1)
		{
			return last_file_error();
		}

		lseek(m_handle, 0, SEEK_SET);

		return static_cast<uint64_t>(size);
#endif
	}

	Result<std::string, FileError> File::read_all_text() const
	{
		// Get file size and allocate string large enough to hold the file contents
		auto result = size()
			.and_then([](const uint64_t size)
			{
				return std::string(size, '\0');
			});

#if defined(_WIN32)
		if (result.has_value())
		{
			DWORD bytesRead = 0;
			const BOOL readSome = ReadFile(
				m_handle,
				result.value().data(),
				result.value().size(),
				&bytesRead,
				nullptr
			);

			if (!readSome)
			{
				return FileError::InvalidRead;
			}
		}
#else
		if (result.has_value())
		{
			ssize_t bytesRead = read(
				m_handle,
				result.value().data(),
				result.value().size()
			);

			if (bytesRead == -1)
			{
				return FileError::InvalidRead;
			}
		}
#endif

		return result;
	}

	Result<File, FileError> File::open(std::string_view filepath, FileAccess access)
	{
#if defined(_WIN32)
		const auto handle = CreateFileA(
			filepath.data(),
			win32_access_flags(access),
			win32_file_share_mode(access),
			nullptr,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			nullptr
		);
#else
		int flags = 0;

		// Enable reading large files (>4gb)
		flags |= O_LARGEFILE;

		// Read-Write flags
		flags |= linux_access_flags(access);

		const auto handle = ::open(filepath.data(), flags);
#endif

		if (handle == InvalidFileHandle)
		{
			return last_file_error();
		}

		File file;
		file.m_handle = handle;
		return file;
	}
}
