#pragma once

#include "sk/sk.h"
#include <Windows.h>
#include <SDL3/SDL_render.h>
#include <deque>

#include "IconCache.h"
#include "ProcessHandle.h"
#include "SettingsManager.h"
#include "WindowList.h"

enum class HistoryItemType
{
	SetPosition,
	SetSize,
	SetTopmost,
	SetMinimized,
	SetMaximized,
	SetBorderless,
};

struct HistoryItem
{
	std::shared_ptr<WindowModel> Window;
	HistoryItemType Type;
	bool MergeWithPrevious = false;
	union  // NOLINT(clang-diagnostic-padded)
	{
		IntVec2 Position;
		IntVec2 Size;
		bool Topmost;
		bool Minimized;
		bool Maximized;
		bool Borderless;
	};
};

class App
{
	HWND m_appWindowHandle = nullptr;
	SDL_Window* m_appWindow = nullptr;
	SDL_Renderer* m_renderer = nullptr;
	IconCache m_iconCache;
	SettingsManager m_settingsManager;

	i32 m_posEditBuffer[2]{};
	i32 m_sizeEditBuffer[2]{};

	char m_profileSaveNameEditBuffer[256]{};
	GlobalProfile m_profileToSave{};

	bool m_oldKeyboardState[256]{};

	WindowList m_windowList;
	static constexpr u32 k_maxUndoCapacity = 64;
	std::deque<HistoryItem> m_undoBuffer;

public:
	void Init(HWND ownWindowHandle, SDL_Window* window, SDL_Renderer* renderer);
	void Shutdown();

	HWND GetAppWindowHandle() const { return m_appWindowHandle; }
	IconCache& GetIconCache() { return m_iconCache; }

	void Render();

private:
	static void RemoveWindowBorder(HWND hwnd);
	static void MoveWindowTopLeft(HWND hwnd);
	static void ActivateWindow(HWND hwnd);
	static void MoveWindowToCenterOfPrimaryMonitor(HWND hwnd, bool activate);

	static bool IsBorderless(HWND hwnd);
	static void SetBorderless(WindowModel& window, bool state, bool updateSize);
	static void MakeWindowFullscreen(HWND hwnd);

	bool IsValidProfileName(std::string_view name) const;

	// Get a profile from the current state.
	GlobalProfile ReadProfile();

	void ApplyProfile(const GlobalProfile& profile);

	void DoWindowListWindow();
	void DoInspectorWindow();
	void DoProfilesWindow();

	void PushUndo(const HistoryItem& undo);
	void PerformUndo();
	void ApplyUndo(const HistoryItem& undo);
};
