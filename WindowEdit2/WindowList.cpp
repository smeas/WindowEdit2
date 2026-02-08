#include "WindowList.h"
#include <algorithm>

#include "WindowModel.h"
#include "sk/strings.h"
#include "App.h"

void WindowList::Init(App& app)
{
	m_app = &app;
}

void WindowList::Refresh(bool showAllWindows)
{
	m_refreshShowAllWindows = showAllWindows;
	HWND oldSelection = m_selectedIndex < m_windows.size() ? m_windows[m_selectedIndex]->GetHandle() : NULL;

	m_windows.clear();
	m_newHandleMap.clear(); // should already be cleared...

	EnumWindows(RefreshCallback, (LPARAM)(void*)this);  // NOLINT(bugprone-casting-through-void)

	std::swap(m_handleMap, m_newHandleMap);
	m_newHandleMap.clear();

	// Restore the selection since the list might have been reordered.
	if (oldSelection != NULL)
	{
		for (u32 i = 0; i < m_windows.size(); ++i)
		{
			if (m_windows[i]->GetHandle() == oldSelection)
				m_selectedIndex = i;
		}
	}
}

BOOL WindowList::RefreshCallback(HWND hWnd, LPARAM lParam)
{
	WindowList* self = (WindowList*)(void*)lParam;  // NOLINT(performance-no-int-to-ptr, bugprone-casting-through-void)

	if (hWnd != nullptr && IsWindow(hWnd) && IsWindowVisible(hWnd))
	{
		self->RefreshAddWindow(hWnd);
		return true; // continue EnumWindows
	}

	return true;
}

void WindowList::RefreshAddWindow(HWND hwnd)
{
	bool hasTitle = false;
	wchar_t titleBuffer[256];
	if (!m_refreshShowAllWindows)
	{
		if (hwnd == m_app->GetAppWindowHandle())
			return;

		i32 length = GetWindowTextW(hwnd, titleBuffer, _countof(titleBuffer));
		if (length == 0)
			return;

		hasTitle = true;
	}

	SK_ASSERT(!std::ranges::any_of(m_windows, [&](const auto& elem){ return elem->GetHandle() == hwnd; }));

	if (const auto pair = m_handleMap.find(hwnd); pair == m_handleMap.end())
	{
		// new window
		auto newModel = std::make_shared<WindowModel>(hwnd);
		// This can be invalid for various reasons. The process that owns the window could be a high privileged system
		// process that can not be accessed, or the window or process disappeared while processing (race condition).
		// Since we can't get any useful information out of this window, let's just discard it.
		if (newModel->IsValid())
		{
			m_windows.push_back(newModel);
			newModel->FetchStaticMetadata(m_app->GetIconCache());

			if (hasTitle)
				newModel->m_title = sk::ConvertWStringToUtf8(titleBuffer);
			else
				newModel->RefreshTitle();

			m_newHandleMap[hwnd] = std::move(newModel);
		}
	}
	else
	{
		DWORD processId;
		DWORD threadId = GetWindowThreadProcessId(hwnd, &processId);

		const auto& existingModel = pair->second;

		// This helps ensure that the HWND has not been recycled and is in fact referring to the same window. The only
		// way this could fail is if the same thread (or a new thread in the same process with the same id) in the same
		// process recycles the same HWND, which is exceedingly unlikely.
		if (existingModel->m_owningProcessId == processId &&
			existingModel->m_owningThreadId == threadId &&
			existingModel->m_owningProcessHandle.IsProcessAlive())
		{
			// update window
			if (hasTitle)
				existingModel->m_title = sk::ConvertWStringToUtf8(titleBuffer);
			else
				existingModel->RefreshTitle();

			m_windows.push_back(existingModel);
			m_newHandleMap[hwnd] = existingModel;
		}
		else
		{
			// to be erased
		}
	}
}
