#pragma once
#pragma once

#include <string>
//-------------------------------------------------------------------//
class CFileHandle
{
	FILE* p;
public:
	CFileHandle(const wchar_t* pp, const wchar_t* r)
	{
		errno_t  e = _wfopen_s(&p, pp, r);// if (p == 0) throw CFileError(pp, r);
	}
	CFileHandle(const std::wstring& s, const wchar_t* r)
	{
		errno_t  e = _wfopen_s(&p, s.c_str(), r);// if (p == 0) throw CFileError(s, r);
	}
	~CFileHandle()
	{
		if (p) {
			fclose(p);
		}
	}
	FILE* GetFile()
	{
		return p;
	} 
	void Close()
	{
		if (p) {
			fclose(p);
			p = 0;
		}
	}
};
//-------------------------------------------------------------------//
class CFileHelper
{
public:
	//helper function
	static bool DoesFileExist(CString strFile)
	{
		GetFileAttributes(strFile); // from winbase.h
		DWORD dwError = GetLastError();
		if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(strFile) && (
			dwError == ERROR_FILE_NOT_FOUND ||
			dwError == ERROR_PATH_NOT_FOUND ||
			dwError == ERROR_INVALID_NAME ||
			dwError == ERROR_INVALID_DRIVE ||
			dwError == ERROR_NOT_READY ||
			dwError == ERROR_INVALID_PARAMETER ||
			dwError == ERROR_BAD_PATHNAME ||
			dwError == ERROR_BAD_NETPATH))
		{
			//File not found
			return false;
		}
		return true;
	};
};
