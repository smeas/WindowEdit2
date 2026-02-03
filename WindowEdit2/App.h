#pragma once

#include "sk/sk.h"
#include "sk/strings.h"
#include <Windows.h>
#include <imgui.h>
#include <memory>
#include <vector>
#include <SDL3/SDL_render.h>

#include "IconCache.h"
#include "WindowModel.h"

class App
{
	SDL_Renderer* m_renderer = nullptr;
	std::vector<std::unique_ptr<WindowModel>> m_windows;
	IconCache m_iconCache;
	u32 m_selectedIndex = 0;

	i32 m_posEditBuffer[2]{};
	i32 m_sizeEditBuffer[2]{};

public:
	void Init(SDL_Renderer* renderer)
	{
		m_renderer = renderer;
		m_iconCache.Init(renderer);
	}

	void Render();
	void RefreshWindowList();

private:
	static BOOL CALLBACK RefreshCallback(HWND hWnd, LPARAM lParam);

	static void RemoveWindowBorder(HWND hwnd);
	static void MoveWindowTopLeft(HWND hwnd);
	static void MakeWindowBorderlessFullscreen(HWND hwnd);
};
