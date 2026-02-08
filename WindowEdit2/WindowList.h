#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
#include <Windows.h>

#include "sk/core.h"

class App;
class IconCache;
class WindowModel;

class WindowList
{
	App* m_app = nullptr;
	std::vector<std::shared_ptr<WindowModel>> m_windows;
	std::unordered_map<HWND, std::shared_ptr<WindowModel>> m_handleMap;
	std::unordered_map<HWND, std::shared_ptr<WindowModel>> m_newHandleMap; // swap with m_handleMap while refreshing
	bool m_refreshShowAllWindows = false;
	u32 m_selectedIndex = 0;

public:
	void Init(App& app);

	void Refresh(bool showAllWindows);

	std::vector<std::shared_ptr<WindowModel>>& GetWindows()
	{
		return m_windows;
	}

	bool IsSelectedValid() const { return m_selectedIndex < m_windows.size(); }
	u32 GetSelectedIndex() const { return m_selectedIndex; }

	void SetSelectedIndex(u32 index)
	{
		m_selectedIndex = index;
		SK_VERIFY(index == 0 || IsSelectedValid());
	}

	// Must call IsSelectedValid first!
	std::shared_ptr<WindowModel>& GetSelected()
	{
		SK_VERIFY(IsSelectedValid());
		return m_windows[m_selectedIndex];
	}

private:
	static BOOL CALLBACK RefreshCallback(HWND hWnd, LPARAM lParam);
	void RefreshAddWindow(HWND hwnd);
};
