#ifndef MBASE_IOFILE_H
#define MBASE_IOFILE_H

#include <mbase/io_base.h> // mbase::io_base
#include <mbase/string.h> // mbase::string
#include <mbase/behaviors.h> // mbase::non_copymovable
#include <mbase/char_stream.h> // mbase::char_stream

#ifdef MBASE_PLATFORM_WINDOWS
#include <Windows.h> // CreateFileA, SetFilePointer, WriteFile, ReadFile, CreateIoCompletionPort
#endif

#ifdef MBASE_PLATFORM_UNIX
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#endif

MBASE_STD_BEGIN

/*

	--- CLASS INFORMATION ---
Identification: S0C17-OBJ-UD-ST

Name: io_file

Parent: S0C16-SAB-UD-ST, S0C6-STR-NA-ST 

Behaviour List:
- Default Constructible
- Destructible

Description:
io_file is a class that implements the io_base abstract class.
It is providing file io operations through it's corresponding methods such as:
open_file, close_file, get_file_size, get_file_name etc.

The read/write operation which are through read_data/write_data methods are synchronized,
blocking operations. Attempting to write to an invalid file will result in a OS dependant behavior.
io_file does not make further control on read/write operations whether the file handle is valid or not.

To achieve async io behavior, refer to the section Async I/O in MBASE.

*/

class io_file : public io_base, public non_copymovable {
public:

	enum class access_mode : U32 {
		#ifdef MBASE_PLATFORM_WINDOWS
		READ_ACCESS = GENERIC_READ,
		WRITE_ACCESS = GENERIC_WRITE,
		RW_ACCESS = READ_ACCESS | WRITE_ACCESS
		#endif
		#ifdef MBASE_PLATFORM_UNIX
		READ_ACCESS = O_RDONLY,
		WRITE_ACCESS = O_WRONLY,
		RW_ACCESS = O_RDWR
		#endif
	};

	enum class disposition : U32 {
		#ifdef MBASE_PLATFORM_WINDOWS
		OVERWRITE = CREATE_ALWAYS, // always succeeds
		APPEND = OPEN_EXISTING, // sometimes fail
		OPEN = OPEN_ALWAYS // always succeeds
		#endif
		#ifdef MBASE_PLATFORM_UNIX
		OVERWRITE = O_TRUNC | O_CREAT,
		APPEND = O_APPEND,
		OPEN = 0
		#endif
	};

	/* ===== BUILDER METHODS BEGIN ===== */
	MBASE_INLINE io_file() noexcept;
	MBASE_INLINE io_file(const mbase::wstring& in_filename, access_mode in_accmode = access_mode::RW_ACCESS, disposition in_disp = disposition::OVERWRITE, bool in_async = false) noexcept;
	MBASE_INLINE ~io_file() noexcept;
	/* ===== BUILDER METHODS END ===== */

	/* ===== OBSERVATION METHODS BEGIN ===== */
	MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE bool is_file_open() const noexcept;
	MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE mbase::wstring get_file_name() const noexcept;
	MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE size_type get_file_size() noexcept;
	/* ===== OBSERVATION METHODS END ===== */

	/* ===== STATE-MODIFIER METHODS BEGIN ===== */
	MBASE_INLINE os_file_handle open_file(const mbase::wstring& in_filename, access_mode in_accmode = access_mode::RW_ACCESS, disposition in_disp = disposition::OVERWRITE, bool in_async = false) noexcept;
	MBASE_INLINE GENERIC close_file() noexcept;
	MBASE_INLINE GENERIC clear_file() noexcept;
	MBASE_INLINE size_type write_data(const IBYTEBUFFER in_src) override;
	MBASE_INLINE size_type write_data(const IBYTEBUFFER in_src, size_type in_length) override;
	MBASE_INLINE size_type write_data(const mbase::string& in_src) override;
	MBASE_INLINE size_type write_data(char_stream& in_src) override;
	MBASE_INLINE size_type write_data(char_stream& in_src, size_type in_length) override;
	MBASE_INLINE size_type read_data(IBYTEBUFFER in_src, size_type in_length) override;
	MBASE_INLINE size_type read_data(char_stream& in_src) override;
	MBASE_INLINE size_type read_data(char_stream& in_src, size_type in_length) override;
	/* ===== STATE-MODIFIER METHODS END ===== */

private:
	mbase::wstring mFileName;
};

MBASE_INLINE io_file::io_file() noexcept : mFileName()
{
}

MBASE_INLINE io_file::io_file(const mbase::wstring& in_filename, access_mode in_accmode, disposition in_disp, bool in_async) noexcept : mFileName(in_filename)
{
	open_file(in_filename, in_accmode, in_disp, in_async);
}

MBASE_INLINE io_file::~io_file() noexcept
{
	close_file();
}

MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE bool io_file::is_file_open() const noexcept 
{
	return mOperateReady;
}

MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE mbase::wstring io_file::get_file_name() const noexcept 
{
	return mFileName;
}

MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE typename io_file::size_type io_file::get_file_size() noexcept 
{
	if(!mOperateReady)
	{
		return 0;
	}
	
#ifdef MBASE_PLATFORM_WINDOWS
	LARGE_INTEGER lInt;
	GetFileSizeEx(mRawContext.raw_handle, &lInt);
	return lInt.QuadPart;
#endif
#ifdef MBASE_PLATFORM_UNIX
	size_type oldPointerPos = get_file_pointer_pos();
	set_file_pointer(0, mbase::io_base::move_method::MV_END);

	size_type fileSize = get_file_pointer_pos();
	set_file_pointer(oldPointerPos, move_method::MV_BEGIN); // back to old position
	
	return fileSize;
#endif
}

MBASE_INLINE io_base::os_file_handle io_file::open_file(const mbase::wstring& in_filename, access_mode in_accmode, disposition in_disp, bool is_async) noexcept
{
	close_file();
#ifdef MBASE_PLATFORM_WINDOWS
	DWORD fileAttrs = FILE_ATTRIBUTE_NORMAL;
	if (is_async) 
	{
		fileAttrs |= FILE_FLAG_OVERLAPPED;
	}
	
	mFileName = in_filename;
	PTRGENERIC rawHandle = CreateFileW(mFileName.c_str(), (DWORD)in_accmode, FILE_SHARE_READ, nullptr, (DWORD)in_disp, fileAttrs, nullptr);	
	
	if (rawHandle == INVALID_HANDLE_VALUE)
	{
		_set_last_error(GetLastError());
	}
	else
	{
		
		mOperateReady = true;
		_set_raw_context(rawHandle);
		if (in_disp == disposition::APPEND)
		{
			SetFilePointer(mRawContext.raw_handle, 0, nullptr, FILE_END);
		}
	}
#endif
#ifdef MBASE_PLATFORM_UNIX
	I32 fHandle = open(mbase::to_utf8(in_filename).c_str(), (U32)in_disp | (U32)in_accmode, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if(fHandle == -1)
	{
		_set_last_error(errno);
	}
	else
	{
		mOperateReady = true;
		_set_raw_context(fHandle);
	}
#endif

	return mRawContext.raw_handle;
}

MBASE_INLINE GENERIC io_file::close_file() noexcept 
{
	if (mRawContext.raw_handle)
	{
#ifdef MBASE_PLATFORM_WINDOWS

		CloseHandle(mRawContext.raw_handle);
#endif
#ifdef MBASE_PLATFORM_UNIX
		close(mRawContext.raw_handle);
#endif
	}
	mOperateReady = false;
	mRawContext.raw_handle = 0;
}

MBASE_INLINE GENERIC io_file::clear_file() noexcept
{
	#ifdef MBASE_PLATFORM_WINDOWS
	if(mRawContext.raw_handle)
	{
		set_file_pointer(0, mbase::io_base::move_method::MV_BEGIN);
		SetEndOfFile(mRawContext.raw_handle);	
	}
	#endif

	#ifdef MBASE_PLATFORM_UNIX
	if (mRawContext.raw_handle != -1) // In Linux we typically check against -1 for invalid fd
    {
        lseek(mRawContext.raw_handle, 0, SEEK_SET);  // Move to beginning
        ftruncate(mRawContext.raw_handle, 0);        // Truncate to 0 bytes
    }
	#endif
}

MBASE_INLINE typename io_file::size_type io_file::write_data(const IBYTEBUFFER in_src)
{
	if(!is_file_open())
	{
		return 0;		
	}

	SIZE_T dataLength = type_sequence<IBYTE>::length_bytes(in_src);
#ifdef MBASE_PLATFORM_WINDOWS
	DWORD dataWritten = 0;
	size_type writeResult = WriteFile(mRawContext.raw_handle, in_src, dataLength + 1, &dataWritten, nullptr);

	if(!writeResult)
	{
		_set_last_error(GetLastError());
		close_file();
	}
	return dataWritten;
#endif
#ifdef MBASE_PLATFORM_UNIX
	ssize_t writeResult = write(mRawContext.raw_handle, in_src, dataLength + 1);
	if(writeResult == -1)
	{
		_set_last_error(errno);
		close_file();
		return 0;
	}

	return writeResult;
#endif
}

MBASE_INLINE typename io_file::size_type io_file::write_data(const IBYTEBUFFER in_src, size_type in_length)
{
	if(!is_file_open())
	{
		return 0;		
	}

#ifdef MBASE_PLATFORM_WINDOWS
	DWORD dataWritten = 0;
	size_type writeResult = WriteFile(mRawContext.raw_handle, in_src, in_length, &dataWritten, nullptr);

	if (!writeResult)
	{
		_set_last_error(GetLastError());
		close_file();
	}
	
	return dataWritten;
#endif
#ifdef MBASE_PLATFORM_UNIX
	ssize_t writeResult = write(mRawContext.raw_handle, in_src, in_length);
	if(writeResult == -1)
	{
		_set_last_error(errno);
		close_file();
		return 0;
	}

	return writeResult;
#endif
}

typename io_file::size_type io_file::write_data(const mbase::string& in_src)
{
	if(!is_file_open())
	{
		return 0;		
	}

#ifdef MBASE_PLATFORM_WINDOWS
	DWORD dataWritten = 0;
	size_type writeResult = WriteFile(mRawContext.raw_handle, in_src.c_str(), in_src.size() + 1, &dataWritten, nullptr);

	if (!writeResult)
	{
		_set_last_error(GetLastError());
		close_file();
	}

	return dataWritten;
#endif
#ifdef MBASE_PLATFORM_UNIX
	ssize_t writeResult = write(mRawContext.raw_handle, in_src.c_str(), in_src.size() + 1);
	if(writeResult == -1)
	{
		_set_last_error(errno);
		close_file();
		return 0;
	}

	return writeResult;
#endif
}

MBASE_INLINE typename io_file::size_type io_file::write_data(char_stream& in_src)
{
	if(!is_file_open())
	{
		return 0;		
	}

	PTRDIFF cursorPos = in_src.get_pos();
	SIZE_T bytesToWrite = in_src.buffer_length() - cursorPos;
	IBYTEBUFFER tmpBuffer = in_src.get_bufferc();
#ifdef MBASE_PLATFORM_WINDOWS
	DWORD dataWritten = 0;
	size_type writeResult = WriteFile(mRawContext.raw_handle, tmpBuffer, bytesToWrite, &dataWritten, nullptr);

	if (!writeResult)
	{
		_set_last_error(GetLastError());
		close_file();
	}

	return dataWritten;
#endif
#ifdef MBASE_PLATFORM_UNIX
	ssize_t writeResult = write(mRawContext.raw_handle, tmpBuffer, bytesToWrite);
	if(writeResult == -1)
	{
		_set_last_error(errno);
		close_file();
		return 0;
	}

	return writeResult;
#endif
}

MBASE_INLINE typename io_file::size_type io_file::write_data(char_stream& in_src, size_type in_length)
{
	if(!is_file_open())
	{
		return 0;		
	}
	IBYTEBUFFER tmpBuffer = in_src.get_bufferc();
#ifdef MBASE_PLATFORM_WINDOWS
	DWORD dataWritten = 0;
	size_type writeResult = WriteFile(mRawContext.raw_handle, tmpBuffer, in_length, &dataWritten, nullptr);
	
	if (!writeResult)
	{
		_set_last_error(GetLastError());
		close_file();
	}

	in_src.advance(dataWritten);
	return dataWritten;
#endif
#ifdef MBASE_PLATFORM_UNIX
	ssize_t writeResult = write(mRawContext.raw_handle, tmpBuffer, in_length);
	if(writeResult == -1)
	{
		_set_last_error(errno);
		close_file();
		return 0;
	}

	return writeResult;
#endif
}

MBASE_INLINE typename io_file::size_type io_file::read_data(IBYTEBUFFER in_src, size_type in_length)
{
	if(!is_file_open())
	{
		return 0;		
	}

#ifdef MBASE_PLATFORM_WINDOWS
	DWORD dataRead = 0;
	size_type readResult = ReadFile(mRawContext.raw_handle, in_src, in_length, &dataRead, nullptr);
	if (!readResult)
	{
		_set_last_error(GetLastError());
		close_file();
	}
	return dataRead;
#endif
#ifdef MBASE_PLATFORM_UNIX
	ssize_t readResult = read(mRawContext.raw_handle, in_src, in_length);
	if(readResult == -1)
	{
		_set_last_error(errno);
		close_file();
		return 0;
	}
	return readResult;
#endif
}

MBASE_INLINE typename io_file::size_type io_file::read_data(char_stream& in_src)
{
	if(!is_file_open())
	{
		return 0;		
	}

	PTRDIFF cursorPos = in_src.get_pos();
	IBYTEBUFFER tmpBuffer = in_src.get_bufferc();
	SIZE_T bytesToRead = in_src.buffer_length() - cursorPos;
#ifdef MBASE_PLATFORM_WINDOWS
	DWORD dataRead = 0;
	size_type readResult = ReadFile(mRawContext.raw_handle, tmpBuffer, bytesToRead, &dataRead, nullptr);
	if (!readResult)
	{
		_set_last_error(GetLastError());
		close_file();
	}
	return dataRead;
#endif
#ifdef MBASE_PLATFORM_UNIX
	ssize_t readResult = read(mRawContext.raw_handle, tmpBuffer, bytesToRead);
	if(readResult == -1)
	{
		_set_last_error(errno);
		close_file();
		return 0;
	}
	return readResult;
#endif
}

MBASE_INLINE typename io_file::size_type io_file::read_data(char_stream& in_src, size_type in_length)
{
	if(!is_file_open())
	{
		return 0;		
	}

	IBYTEBUFFER tmpBuffer = in_src.get_bufferc();
#ifdef MBASE_PLATFORM_WINDOWS
	DWORD dataRead = 0;
	size_type readResult = ReadFile(mRawContext.raw_handle, tmpBuffer, in_length, &dataRead, nullptr);
	if (!readResult)
	{
		_set_last_error(GetLastError());
		close_file();
	}
	in_src.advance(dataRead);
	return dataRead;
#endif
#ifdef MBASE_PLATFORM_UNIX
	ssize_t readResult = read(mRawContext.raw_handle, tmpBuffer, in_length);
	if(readResult == -1)
	{
		_set_last_error(errno);
		close_file();
		return 0;
	}
	return readResult;
#endif
}

MBASE_STD_END

#endif // MBASE_IOFILE_H