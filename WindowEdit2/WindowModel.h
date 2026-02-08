#pragma once
#include <Windows.h>
#include <string>
#include <memory>
#include "IconCache.h"
#include "ProcessHandle.h"

class WindowModel
{
	friend class WindowList;
	HWND m_windowHandle = nullptr;
	ProcessHandle m_owningProcessHandle;
	u32 m_owningProcessId = 0;
	u32 m_owningThreadId = 0;
	std::wstring m_processFileName;

	std::string m_title;
	std::string m_executablePathName;
	std::string m_executableName; // trimmed version of m_executablePathName
	IconTextureRef m_icon;

public:
	WindowModel(HWND handle);
	~WindowModel();

	bool IsValid() const { return m_owningProcessHandle.IsValid(); }
	HWND GetHandle() const { return m_windowHandle; }

	const std::string& GetTitle() const { return m_title; }
	const std::wstring& GetExecutablePathNameW() const { return m_processFileName; }
	const std::string& GetExecutablePathName() const { return m_executablePathName; }
	const std::string& GetExecutableName() const { return m_executableName; }
	const IconTexture* GetIcon() const { return m_icon.get(); }



	void FetchStaticMetadata(IconCache& iconCache);
	void RefreshTitle();

	static std::wstring GetProcessNameW(HANDLE processHandle);

private:
	// TODO: For reference
	// bool IsOwningProcessStillAlive() const
	// {
	// 	if (m_owningProcessHandle == NULL)
	// 		return false;
	//
	// 	DWORD result = WaitForSingleObject(m_owningProcessHandle, 0);
	// 	// If the process does not exist anymore, the handle will be signaled.
	// 	if (result == WAIT_OBJECT_0)
	// 		return false;
	//
	// 	return true;
	// }
};
