#include "WindowModel.h"

#include <psapi.h>

#include "sk/strings.h"

#define MAX_TITLE_LENGTH 256

WindowModel::~WindowModel()
{
	if (m_owningProcessHandle != NULL)
		CloseHandle(m_owningProcessHandle);
}

void WindowModel::FetchMetadata(IconCache& iconCache)
{
	if (m_windowHandle == nullptr)
	{
		m_title.clear();
		return;
	}

	// TODO: Later, we could check if the handle is stale by checking that the process ids matches.
	if (m_owningProcessId == 0)
	{
		DWORD processId;
		m_owningThreadId = GetWindowThreadProcessId(m_windowHandle, &processId);
		m_owningProcessId = processId;
	}

	if (m_owningProcessHandle == NULL)
	{
		m_owningProcessHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, m_owningProcessId);
		m_processFileName = GetProcessNameW(m_owningProcessHandle);
	}


	{
		size_t splitIndex = m_processFileName.find_last_of(L"\\/");
		size_t offset = splitIndex != std::wstring::npos ? splitIndex + 1 : 0;
		m_executableName = sk::ConvertWStringToUtf8(&m_processFileName.c_str()[offset]);
	}


	wchar_t titleBuffer[MAX_TITLE_LENGTH];
	GetWindowTextW(m_windowHandle, titleBuffer, _countof(titleBuffer));
	sk::ConvertWStringToUtf8(titleBuffer, m_title);

	m_icon = iconCache.GetIconForProcessFileName(m_processFileName);
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
