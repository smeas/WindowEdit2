#pragma once

#include "sk/sk.h"
#include "sk/strings.h"
#include <Windows.h>
#include <imgui.h>
#include <memory>
#include <vector>
#include <SDL3/SDL_render.h>

#include "IconCache.h"
#include "SettingsManager.h"
#include "WindowModel.h"

class App
{
	HWND m_appWindowHandle = nullptr;
	SDL_Window* m_appWindow = nullptr;
	SDL_Renderer* m_renderer = nullptr;
	std::vector<std::unique_ptr<WindowModel>> m_windows;
	IconCache m_iconCache;
	SettingsManager m_settingsManager;
	u32 m_selectedIndex = 0;

	i32 m_posEditBuffer[2]{};
	i32 m_sizeEditBuffer[2]{};

	char m_profileSaveNameEditBuffer[256]{};
	GlobalProfile m_profileToSave{};

public:
	void Init(HWND ownWindowHandle, SDL_Window* window, SDL_Renderer* renderer);
	void Shutdown();

	void Render();
	void RefreshWindowList();


private:
	static BOOL CALLBACK RefreshCallback(HWND hWnd, LPARAM lParam);

	static void RemoveWindowBorder(HWND hwnd);
	static void MoveWindowTopLeft(HWND hwnd);
	static void MakeWindowBorderlessFullscreen(HWND hwnd);
	static void ActivateWindow(HWND hwnd);

	bool IsValidProfileName(std::string_view name) const;

	// Get a profile from the current state.
	GlobalProfile ReadProfile();

	void ApplyProfile(const GlobalProfile& profile);

	void DoWindowListWindow();
	void DoInspectorWindow();
	void DoProfilesWindow();
};
