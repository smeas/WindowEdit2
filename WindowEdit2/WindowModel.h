#pragma once
#include <Windows.h>
#include <string>
#include <memory>
#include "IconCache.h"

class WindowModel
{
	HWND m_windowHandle = nullptr;
	HANDLE m_owningProcessHandle = NULL;
	u32 m_owningProcessId = 0;
	u32 m_owningThreadId = 0;
	std::wstring m_processFileName;

	std::string m_title;
	std::string m_executableName;
	IconTextureRef m_icon;

public:
	WindowModel(HWND handle) : m_windowHandle(handle) {}
	~WindowModel();

	HWND GetHandle() const { return m_windowHandle; }
	const std::string& GetTitle() const { return m_title; }
	const std::string& GetExecutableName() const { return m_executableName; }
	const IconTexture* GetIcon() const { return m_icon.get(); }

	void FetchMetadata(IconCache& iconCache);

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
