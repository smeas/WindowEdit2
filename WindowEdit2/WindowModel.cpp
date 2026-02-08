#include "WindowModel.h"

#include <psapi.h>

#include "sk/strings.h"

#define MAX_TITLE_LENGTH 256

WindowModel::WindowModel(HWND handle) : m_windowHandle(handle)
{
	DWORD processId;
	m_owningThreadId = GetWindowThreadProcessId(m_windowHandle, &processId);
	m_owningProcessId = processId;

	if (m_owningProcessId != 0)
	{
		// There are some high privilege processes that on which this call will always fail. One example is the system
		// process csrss.exe (Client Server Runtime Process). This process seems to be responsible for some system
		// tooltips, like when hovering the minimize/maximize/close buttons on a normal window. Accessing this process
		// will always fail.
		m_owningProcessHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, false, m_owningProcessId);

		// Handle rare edge case where window and process ids could get are recycled between calls. This ensures that the
		// process handle we opened really corresponds to the window.
		if (m_owningProcessHandle.IsValid())
		{
			DWORD nextProcessId;
			DWORD nextThreadId = GetWindowThreadProcessId(handle, &nextProcessId);

			// Process/thread ids changed between calls, possibly due to id recycling.
			// The window handle is not valid anymore!
			if (m_owningThreadId != nextThreadId || m_owningProcessId != nextProcessId)
			{
				m_owningProcessHandle = NULL;
			}
		}
	}

	// Window handle is not valid, or access is denied.
	if (!m_owningProcessHandle.IsValid())
		return;

	m_processFileName = GetProcessNameW(m_owningProcessHandle.Get());

	m_executablePathName = sk::ConvertWStringToUtf8(m_processFileName.c_str());
	size_t splitIndex = m_executablePathName.find_last_of("\\/");
	size_t offset = splitIndex != std::wstring::npos ? splitIndex + 1 : 0;
	m_executableName = m_executablePathName.substr(offset);
}

WindowModel::~WindowModel()
{
}

void WindowModel::FetchStaticMetadata(IconCache& iconCache)
{
	m_icon = iconCache.GetIconForProcessFileName(m_processFileName);
}

void WindowModel::RefreshTitle()
{
	wchar_t titleBuffer[MAX_TITLE_LENGTH];
	GetWindowTextW(m_windowHandle, titleBuffer, _countof(titleBuffer));
	sk::ConvertWStringToUtf8(titleBuffer, m_title);
}

std::wstring WindowModel::GetProcessNameW(HANDLE processHandle)
{
	std::wstring result;
	if (processHandle == NULL)
		return result;

	// GetWindowModuleFileName only works on windows in the same process.
	// u32 stringSize = GetWindowModuleFileName(windowHandle, nameBuffer, _countof(nameBuffer));

	wchar_t nameBuffer[256];
	u32 stringSize = GetModuleFileNameEx(processHandle, NULL, nameBuffer, _countof(nameBuffer));
	if (stringSize == 0)
		return result;

	result.resize(stringSize); // skip null terminator
	std::memcpy(result.data(), nameBuffer, stringSize * sizeof(wchar_t));
	return result;
}
