#pragma once

#include "sk/sk.h"
#include <Windows.h>
#include <SDL3/SDL_render.h>

#include "IconCache.h"
#include "ProcessHandle.h"
#include "SettingsManager.h"
#include "WindowList.h"

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

	ProcessHandle m_test;
	ProcessHandle m_test2;

	WindowList m_windowList;

public:
	void Init(HWND ownWindowHandle, SDL_Window* window, SDL_Renderer* renderer);
	void Shutdown();

	HWND GetAppWindowHandle() const { return m_appWindowHandle; }
	IconCache& GetIconCache() { return m_iconCache; }

	void Render();

private:
	static void RemoveWindowBorder(HWND hwnd);
	static void MoveWindowTopLeft(HWND hwnd);
	static void MakeWindowBorderlessFullscreen(HWND hwnd);
	static void ActivateWindow(HWND hwnd);
	static void MoveWindowToCenterOfPrimaryMonitor(HWND hwnd, bool activate);

	bool IsValidProfileName(std::string_view name) const;

	// Get a profile from the current state.
	GlobalProfile ReadProfile();

	void ApplyProfile(const GlobalProfile& profile);

	void DoWindowListWindow();
	void DoInspectorWindow();
	void DoProfilesWindow();
};
