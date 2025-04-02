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
	MBASE_INLINE io_file(const mbase::wstring& in_filename, access_mode in_accmode = access_mode::RW_ACCESS, disposition in_disp = disposition::OVERWRITE) noexcept;
	MBASE_INLINE ~io_file() noexcept;
	/* ===== BUILDER METHODS END ===== */

	/* ===== OBSERVATION METHODS BEGIN ===== */
	MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE bool is_file_open() const noexcept;
	MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE mbase::wstring get_file_name() const noexcept;
	MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE size_type get_file_size() noexcept;
	/* ===== OBSERVATION METHODS END ===== */

	/* ===== STATE-MODIFIER METHODS BEGIN ===== */
	MBASE_INLINE os_file_handle open_file(const mbase::wstring& in_filename, access_mode in_accmode = access_mode::RW_ACCESS, disposition in_disp = disposition::OVERWRITE) noexcept;
	MBASE_INLINE os_file_handle open_file(const mbase::string& in_filename, access_mode in_accmode = access_mode::RW_ACCESS, disposition in_disp = disposition::OVERWRITE) noexcept;
	MBASE_INLINE GENERIC close_file() noexcept;
	MBASE_INLINE GENERIC clear_file() noexcept;
	MBASE_INLINE size_type write_data(CBYTEBUFFER in_src) override;
	MBASE_INLINE size_type write_data(CBYTEBUFFER in_src, size_type in_length) override;
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

MBASE_INLINE io_file::io_file(const mbase::wstring& in_filename, access_mode in_accmode, disposition in_disp) noexcept : mFileName(in_filename)
{
	open_file(in_filename, in_accmode, in_disp);
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

MBASE_INLINE io_base::os_file_handle io_file::open_file(const mbase::wstring& in_filename, access_mode in_accmode, disposition in_disp) noexcept
{
	close_file();
#ifdef MBASE_PLATFORM_WINDOWS
	DWORD fileAttrs = FILE_ATTRIBUTE_NORMAL;
	
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

MBASE_INLINE io_base::os_file_handle io_file::open_file(const mbase::string& in_filename, access_mode in_accmode, disposition in_disp) noexcept
{
	close_file();
#ifdef MBASE_PLATFORM_WINDOWS
	DWORD fileAttrs = FILE_ATTRIBUTE_NORMAL;
	
	mFileName = mbase::from_utf8(in_filename);
	PTRGENERIC rawHandle = CreateFileA(in_filename.c_str(), (DWORD)in_accmode, FILE_SHARE_READ, nullptr, (DWORD)in_disp, fileAttrs, nullptr);	
	
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
	I32 fHandle = open(in_filename.c_str(), (U32)in_disp | (U32)in_accmode, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
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

MBASE_INLINE typename io_file::size_type io_file::write_data(CBYTEBUFFER in_src, size_type in_length)
{
	if(!is_file_open())
	{
		return 0;		
	}
	size_type totalBytesWritten = 0;
#ifdef MBASE_PLATFORM_WINDOWS
	DWORD dataWritten = 0;
	DWORD maxChunkSize = 128 * (1024);
	while(totalBytesWritten < in_length)
	{
		size_type remainingBytes = in_length - totalBytesWritten;
		DWORD bytesToWriteEach = (remainingBytes < maxChunkSize) ? static_cast<DWORD>(remainingBytes) : maxChunkSize;
		BOOL writeResult = WriteFile(mRawContext.raw_handle, in_src + totalBytesWritten, bytesToWriteEach, &dataWritten, nullptr);
		if (!writeResult)
		{
			_set_last_error(GetLastError());
			close_file();
			break;
		}
		else
		{
			if(!dataWritten)
			{
				break;
			}
			totalBytesWritten += dataWritten;
		}
	}
	
#endif
#ifdef MBASE_PLATFORM_UNIX
	size_type maxChunkSize = 128 * (1024);
	while(totalBytesWritten < in_length)
	{
		size_type remainingBytes = in_length - totalBytesWritten;
		size_type bytesToWriteEach = (remainingBytes < maxChunkSize) ? remainingBytes : maxChunkSize;
		ssize_t writeResult = write(mRawContext.raw_handle, in_src + totalBytesWritten, bytesToWriteEach);
		if(writeResult == -1)
		{
			_set_last_error(errno);
			close_file();
			break;
		}
		else
		{
			if(!writeResult)
			{
				break;
			}
			totalBytesWritten += writeResult;
		}
	}
#endif
	return totalBytesWritten;
}

MBASE_INLINE typename io_file::size_type io_file::write_data(CBYTEBUFFER in_src)
{
	return this->write_data(in_src, type_sequence<IBYTE>::length_bytes(in_src));
}

typename io_file::size_type io_file::write_data(const mbase::string& in_src)
{
	return this->write_data(in_src.c_str(), in_src.size());
}

MBASE_INLINE typename io_file::size_type io_file::write_data(char_stream& in_src)
{
	PTRDIFF cursorPos = in_src.get_pos();
	SIZE_T bytesToWrite = in_src.buffer_length() - cursorPos;
	IBYTEBUFFER tmpBuffer = in_src.get_bufferc();

	return this->write_data(tmpBuffer, bytesToWrite);
}

MBASE_INLINE typename io_file::size_type io_file::write_data(char_stream& in_src, size_type in_length)
{
	return this->write_data(in_src.get_bufferc(), in_length);
}

MBASE_INLINE typename io_file::size_type io_file::read_data(IBYTEBUFFER in_src, size_type in_length)
{
	if(!is_file_open())
	{
		return 0;		
	}
	size_type totalBytesRead = 0;
#ifdef MBASE_PLATFORM_WINDOWS
	DWORD dataRead = 0;
	DWORD maxChunkSize = 128 * (1024);
	while(totalBytesRead < in_length)
	{	
		size_type remainingBytes = in_length - totalBytesRead;
		DWORD bytesToReadEach = (remainingBytes < maxChunkSize) ? static_cast<DWORD>(remainingBytes) : maxChunkSize;
		BOOL readResult = ReadFile(mRawContext.raw_handle, in_src + totalBytesRead, bytesToReadEach, &dataRead, nullptr);
		if (!readResult)
		{
			_set_last_error(GetLastError());
			close_file();
			break;
		}
		else
		{
			if(!dataRead)
			{
				break;
			}
			totalBytesRead += dataRead;
		}
	}
	
#endif
#ifdef MBASE_PLATFORM_UNIX
	size_type maxChunkSize = 128 * (1024);
	while(totalBytesRead < in_length)
	{
		size_type remainingBytes = in_length - totalBytesRead;
		size_type bytesToReadEach = (remainingBytes < maxChunkSize) ? remainingBytes : maxChunkSize;
		ssize_t readResult = read(mRawContext.raw_handle, in_src + totalBytesRead, bytesToReadEach);

		if(readResult == -1)
		{
			_set_last_error(errno);
			close_file();
			break;
		}
		else
		{
			if(!readResult)
			{
				break;
			}
			totalBytesRead += readResult;
		}
	}	
#endif
	return totalBytesRead;
}

MBASE_INLINE typename io_file::size_type io_file::read_data(char_stream& in_src)
{
	PTRDIFF cursorPos = in_src.get_pos();
	SIZE_T bytesToRead = in_src.buffer_length() - cursorPos;
	return this->read_data(in_src.get_bufferc(), bytesToRead);
}

MBASE_INLINE typename io_file::size_type io_file::read_data(char_stream& in_src, size_type in_length)
{
	return this->read_data(in_src.get_bufferc(), in_length);
}

MBASE_INLINE mbase::string read_file_as_string(mbase::io_file& in_iof)
{
	mbase::string fileContent;

	while(true)
	{
		IBYTE fileData[512] = {0};
		SIZE_T bytesRead = in_iof.read_data(fileData, 512);
		if(!bytesRead)
		{
			break;
		}

		fileContent.append(fileData, bytesRead);
	}

	return fileContent;
}

MBASE_INLINE mbase::string read_file_as_string(const mbase::wstring& in_path)
{
	mbase::io_file iof;
	iof.open_file(in_path, mbase::io_file::access_mode::READ_ACCESS, mbase::io_file::disposition::OPEN);
	if(!iof.is_file_open())
	{
		return mbase::string();
	}

	return read_file_as_string(iof);
}

MBASE_INLINE mbase::string read_file_as_string(const mbase::string& in_path)
{
	mbase::io_file iof;
	iof.open_file(in_path, mbase::io_file::access_mode::READ_ACCESS, mbase::io_file::disposition::OPEN);
	if(!iof.is_file_open())
	{
		return mbase::string();
	}

	return read_file_as_string(iof);
}

MBASE_INLINE bool write_string_to_file(const mbase::wstring& in_path, const mbase::string& in_string)
{
	mbase::io_file iof;
	iof.open_file(in_path, mbase::io_file::access_mode::RW_ACCESS, mbase::io_file::disposition::OVERWRITE);
	if(!iof.is_file_open())
	{
		return false;
	}

	iof.write_data(in_string);
	return true;
}

MBASE_INLINE bool append_string_to_file(const mbase::wstring& in_path, const mbase::string& in_string)
{
	mbase::io_file iof;
	iof.open_file(in_path, mbase::io_file::access_mode::WRITE_ACCESS, mbase::io_file::disposition::APPEND);
	if(!iof.is_file_open())
	{
		iof.open_file(in_path);
	}

	if(!iof.is_file_open())
	{
		return false;
	}

	iof.write_data(in_string);
	return true;
}


MBASE_INLINE bool write_string_to_file(const mbase::string& in_path, const mbase::string& in_string)
{
	mbase::io_file iof;
	iof.open_file(in_path, mbase::io_file::access_mode::RW_ACCESS, mbase::io_file::disposition::OVERWRITE);
	if(!iof.is_file_open())
	{
		return false;
	}
	iof.write_data(in_string);
	return true;
}

MBASE_INLINE bool append_string_to_file(const mbase::string& in_path, const mbase::string& in_string)
{
	mbase::io_file iof;
	iof.open_file(in_path, mbase::io_file::access_mode::WRITE_ACCESS, mbase::io_file::disposition::APPEND);
	if(!iof.is_file_open())
	{
		iof.open_file(in_path);
	}

	if(!iof.is_file_open())
	{
		return false;
	}
	iof.write_data(in_string);
	return true;
}

MBASE_STD_END

#endif // MBASE_IOFILE_H