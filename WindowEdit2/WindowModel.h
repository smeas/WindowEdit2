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
	i32 BorderlessRemovedStyleFlags = 0;
	i32 BorderlessRemovedExStyleFlags = 0;

	WindowModel(HWND handle);
	~WindowModel();

	bool IsValid() const { return m_owningProcessHandle.IsValid(); }
	HWND GetHandle() const { return m_windowHandle; }
	ProcessHandle& GetOwningProcessHandle() { return m_owningProcessHandle; }

	const std::string& GetTitle() const { return m_title; }
	const std::wstring& GetExecutablePathNameW() const { return m_processFileName; }
	const std::string& GetExecutablePathName() const { return m_executablePathName; }
	const std::string& GetExecutableName() const { return m_executableName; }
	const IconTexture* GetIcon() const { return m_icon.get(); }

	// Checks if the window handle is still valid, and has not been recycled (points to the same window as before).
	bool VerifyHandle();

	void FetchStaticMetadata(IconCache& iconCache);
	void RefreshTitle();

	static std::wstring GetProcessNameW(HANDLE processHandle);
};
