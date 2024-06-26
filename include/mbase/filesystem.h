#ifndef MBASE_FILESYSTEM_H
#define MBASE_FILESYSTEM_H

#include <mbase/common.h>
#include <mbase/string.h> // mbase::string
#include <mbase/vector.h> // mbase::vector

#ifdef MBASE_PLATFORM_WINDOWS
#include <Windows.h>
/*
ERROR_ALREADY_EXISTS,
ERROR_PATH_NOT_FOUND,
ERROR_ACCESS_DENIED,
CreateDirectoryA,
FindFirstFileA,
FindNextFileA,
FindClose,
CopyFileA,
DeleteFileA,
GetTempPathA,
GetCurrentDirectoryA,
GetTempFileNameA
*/
#endif

#ifdef MBASE_PLATFORM_UNIX
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#endif

MBASE_STD_BEGIN

// THIS INFORMATION STRUCT WILL BE EXTENDED LATER
struct FS_FILE_INFORMATION {
	mbase::string fileName;
	SIZE_T fileSize = 0;
};

enum class FS_ERROR : I32{
	FS_SUCCESS = 0,
	FS_DIRECTORY_EXISTS = MBASE_FS_FLAGS_MIN,
	FS_PATH_NOT_FOUND,
	FS_ACCESS_DENIED,
	FS_UNKNOWN_ERROR = MBASE_FS_FLAGS_MAX
};

MBASE_INLINE FS_ERROR err_convert(I16 in_err) noexcept;
MBASE_INLINE FS_ERROR create_directory(const mbase::string_view& in_path) noexcept;
MBASE_INLINE FS_ERROR copy_file(const mbase::string_view& in_path, const mbase::string_view& in_copypath) noexcept;
MBASE_INLINE FS_ERROR delete_file(const mbase::string_view& in_path) noexcept;
MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE mbase::string get_temp_path() noexcept;
MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE mbase::string get_current_path() noexcept;
MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE mbase::string get_temp_file(const mbase::string_view& in_prefix = "") noexcept;
template<typename ContainerType = mbase::vector<FS_FILE_INFORMATION>>
MBASE_INLINE GENERIC get_directory(const mbase::string_view& in_path, ContainerType& out_files) noexcept
{
#ifdef MBASE_PLATFORM_WINDOWS
	WIN32_FIND_DATAA findData;
	HANDLE findHandle = FindFirstFileA(in_path.c_str(), &findData);
	do {
		FS_FILE_INFORMATION ffi;
		ffi.fileName = findData.cFileName;
		ffi.fileSize = findData.nFileSizeHigh | findData.nFileSizeLow;
		out_files.push_back(ffi);
	} while (FindNextFileA(findHandle, &findData));
	FindClose(findHandle);
#endif
#ifdef MBASE_PLATFORM_UNIX
	DIR *d = nullptr;
	struct dirent* dir = nullptr;
	d = opendir(in_path.c_str());
	if(d)
	{
		while((dir = readdir(d)) != NULL)
		{
			FS_FILE_INFORMATION ffi;
			struct stat st = {0};
			
			ffi.fileName = dir->d_name;

			if(!stat(dir->d_name, &st))
			{
				ffi.fileSize = st.st_size;
			}
			out_files.push_back(ffi);
		}
		closedir(d);
	}
#endif
}

/* IMPLEMENTATIONS */

MBASE_INLINE FS_ERROR err_convert(I16 in_err) noexcept
{
#ifdef MBASE_PLATFORM_WINDOWS
	switch (in_err)
	{
	case ERROR_ALREADY_EXISTS:
		return FS_ERROR::FS_DIRECTORY_EXISTS;
	case ERROR_PATH_NOT_FOUND:
		return FS_ERROR::FS_PATH_NOT_FOUND;
	case ERROR_ACCESS_DENIED:
		return FS_ERROR::FS_ACCESS_DENIED;
	default:
		return FS_ERROR::FS_UNKNOWN_ERROR;
	}
#endif
	return FS_ERROR::FS_SUCCESS;
}

MBASE_INLINE FS_ERROR create_directory(const mbase::string_view& in_path) noexcept 
{
#ifdef MBASE_PLATFORM_WINDOWS
	if (!CreateDirectoryA(in_path.c_str(), nullptr))
	{
		return err_convert(GetLastError());
	}
#endif
#ifdef MBASE_PLATFORM_UNIX
	if(mkdir(in_path.c_str(), 0600))
	{

	}
#endif
	return FS_ERROR::FS_SUCCESS;
}

MBASE_INLINE FS_ERROR copy_file(const mbase::string_view& in_path, const mbase::string_view& in_copypath) noexcept 
{
#ifdef MBASE_PLATFORM_WINDOWS
	if (!CopyFileA(in_path.c_str(), in_copypath.c_str(), false))
	{
		return err_convert(GetLastError());
	}
#endif
#ifdef MBASE_PLATFORM_UNIX

#endif
	return FS_ERROR::FS_SUCCESS;
}

MBASE_INLINE FS_ERROR delete_file(const mbase::string_view& in_path) noexcept 
{
#ifdef MBASE_PLATFORM_WINDOWS
	if (!DeleteFileA(in_path.c_str()))
	{
		return err_convert(GetLastError());
	}
#endif
#ifdef MBASE_PLATFORM_UNIX
	remove(in_path.c_str());
#endif
	return FS_ERROR::FS_SUCCESS;
}

MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE mbase::string get_temp_path() noexcept 
{
#ifdef MBASE_PLATFORM_WINDOWS
	IBYTE pathString[MAX_PATH + 1] = { 0 };
	GetTempPathA(MAX_PATH + 1, pathString);
	return mbase::string(pathString);
#endif
#ifdef MBASE_PLATFORM_UNIX
	return mbase::string("/tmp/");
#endif
}

MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE mbase::string get_current_path() noexcept 
{
#ifdef MBASE_PLATFORM_WINDOWS
	IBYTE pathString[MAX_PATH + 1] = { 0 };
	GetCurrentDirectoryA(MAX_PATH + 1, pathString);
	return mbase::string(pathString);
#endif
#ifdef MBASE_PLATFORM_UNIX
	return mbase::string(get_current_dir_name());
#endif
}

MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE mbase::string get_temp_file(const mbase::string_view& in_prefix) noexcept 
{
#ifdef MBASE_PLATFORM_WINDOWS
	IBYTE pathString[MAX_PATH + 1] = { 0 };
	GetTempFileNameA(".", in_prefix.c_str(), 0, pathString);
	return mbase::string(pathString);
#endif
#ifdef MBASE_PLATFORM_UNIX
	mbase::string tempName = in_prefix + "XXXXXX";
	mkstemp(tempName.data());
	return tempName;
#endif
}


MBASE_STD_END

#endif // MBASE_FILESYSTEM_H