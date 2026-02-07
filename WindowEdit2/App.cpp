#include "App.h"
#include "common.h"
//#include "imgui_internal.h"

void App::Init(HWND ownWindowHandle, SDL_Window* window, SDL_Renderer* renderer)
{
	m_appWindowHandle = ownWindowHandle;
	m_appWindow = window;
	m_renderer = renderer;
	m_iconCache.Init(renderer);

	m_settingsManager.Init();
	m_settingsManager.Load();
	SDL_SetWindowSize(window, m_settingsManager.m_windowSize.x, m_settingsManager.m_windowSize.y);
}

void App::Shutdown()
{
	m_settingsManager.Save();
}

void App::Render()
{
	SDL_GetWindowSize(m_appWindow, &m_settingsManager.m_windowSize.x, &m_settingsManager.m_windowSize.y);

	i32 keyCount;
	const bool* keyboardState = SDL_GetKeyboardState(&keyCount);
	bool isHoldingCtrl = SDL_SCANCODE_LCTRL < keyCount && keyboardState[SDL_SCANCODE_LCTRL];

	if (!isHoldingCtrl)
	{
		RefreshWindowList();
	}

	ImGui::DockSpaceOverViewport(ImGui::GetID("fullscreenDockspace"), ImGui::GetMainViewport());

	DoWindowListWindow();
	DoInspectorWindow();
	DoProfilesWindow();

	// ImGui::Begin("Font Debug");
	// ImGui::ShowFontAtlas(ImGui::GetFont()->OwnerAtlas);
	// ImGui::End();
}

void App::RefreshWindowList()
{
	HWND oldSelection = m_selectedIndex < m_windows.size() ? m_windows[m_selectedIndex]->GetHandle() : NULL;

	m_windows.clear();
	EnumWindows(RefreshCallback, (LPARAM)(void*)this);

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

BOOL CALLBACK App::RefreshCallback(HWND hWnd, LPARAM lParam)
{
	if (hWnd == nullptr || !IsWindow(hWnd) || !IsWindowVisible(hWnd))
		return true;

	App* app = (App*)(void*)lParam;

	auto window = std::make_unique<WindowModel>(hWnd);
	// TODO: We don't need to fetch all metadata to determine not to show the window, this is unnecessary.
	window->FetchMetadata(app->m_iconCache);

	bool showWindow = true;
	if (!app->m_settingsManager.m_showAllWindows)
	{
		showWindow =
			!window->GetTitle().empty() && // has a title
			window->GetHandle() != app->m_appWindowHandle; // not the app window
	}

	if (showWindow)
	{
		app->m_windows.push_back(std::move(window));
	}

	return true; // continue EnumWindows
}


//
// Operations
//

void App::RemoveWindowBorder(HWND hwnd)
{
	i32 style = GetWindowLong(hwnd, GWL_STYLE);
	style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
	SetWindowLongW(hwnd, GWL_STYLE, style);

	i32 exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
	exStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
	SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);
}

void App::MoveWindowTopLeft(HWND hwnd)
{
	HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

	Rect windowRect;
	Rect clientRect;
	MONITORINFO monitorInfo{};
	monitorInfo.cbSize = sizeof(monitorInfo);

	if (!GetWindowRect(hwnd, &windowRect) ||
		!GetClientRect(hwnd, &clientRect) ||
		!GetMonitorInfo(monitor, &monitorInfo))
	{
		return;
	}

	Rect monitorRect = (Rect)monitorInfo.rcMonitor;

	IntVec2 windowSize = windowRect.GetSize();
	IntVec2 clientSize = clientRect.GetSize();

	i32 border = (windowSize.x - clientSize.x) / 2;
	i32 capbar = (windowSize.y - clientSize.y) - (windowSize.x - clientSize.x);

	i32 newX = monitorRect.X() - border;
	i32 newY = monitorRect.Y() - (border + capbar);

	if (windowRect.X() == newX && windowRect.Y() == newY)
	{
		newX = monitorRect.X() + 2 - border;
		newY = monitorRect.Y();
	}

	SetWindowPos(hwnd, NULL, newX, newY, 0, 0,
	             SWP_NOSIZE | SWP_NOACTIVATE);
}

void App::MakeWindowBorderlessFullscreen(HWND hwnd)
{
	HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

	RemoveWindowBorder(hwnd);

	// Rect windowRect;
	// Rect clientRect;
	MONITORINFO monitorInfo{};
	monitorInfo.cbSize = sizeof(monitorInfo);

	if ( /*!GetWindowRect(hwnd, &windowRect) ||
		!GetClientRect(hwnd, &clientRect) ||*/
		!GetMonitorInfo(monitor, &monitorInfo))
	{
		return;
	}

	Rect monitorRect = (Rect)monitorInfo.rcMonitor;

	// if (windowRect.Width() == monitorRect.Width() &&
	// 	windowRect.Height() == monitorRect.Height())
	// {
	// 	SetWindowPos(hwnd, NULL, 0, 0, windowRect.Width(), windowRect.Height(),
	// 		SWP_NOMOVE | SWP_NOACTIVATE);
	// }

	SetWindowPos(hwnd, NULL, monitorRect.X(), monitorRect.Y(), monitorRect.Width(), monitorRect.Height(),
	             SWP_NOACTIVATE);
}

void App::ActivateWindow(HWND hwnd)
{
	if (IsIconic(hwnd))
	{
		ShowWindow(hwnd, SW_RESTORE);
	}

	SetForegroundWindow(hwnd);

	SetFocus(hwnd);
}

bool App::IsValidProfileName(std::string_view name) const
{
	return !name.empty() && !m_settingsManager.FindProfile(name);
}

GlobalProfile App::ReadProfile()
{
	// TODO: Maybe not the best to grab values from the edit buffer?
	return {
		.Name = "",
		.Position = {m_posEditBuffer[0], m_posEditBuffer[1]},
		.Size = {m_sizeEditBuffer[0], m_sizeEditBuffer[1]}
	};
}

void App::ApplyProfile(const GlobalProfile& profile)
{
	if (m_selectedIndex >= m_windows.size())
		return;

	SetWindowPos(m_windows[m_selectedIndex]->GetHandle(), NULL,
	             profile.Position.x, profile.Position.y,
	             profile.Size.x, profile.Size.y,
	             SWP_NOACTIVATE);
}

void App::DoWindowListWindow()
{
	ImGui::Begin("Window List");

	if (ImGui::Checkbox("Show All Windows", &m_settingsManager.m_showAllWindows))
	{
		m_settingsManager.Save();
	}

	// TODO: start out with separate docked windows for simplicity while building the GUI and features, can migrate to a
	//   table layout later if needed.
	// if (ImGui::BeginTable("layouttable", 2,
	//                       ImGuiTableFlags_Resizable))
	// {
	// 	ImGui::TableNextRow();
	// 	ImGui::TableSetColumnIndex(0);
	//
	// 	ImGui::BeginChild("windowScroll", ImVec2(0, 0), 0, ImGuiWindowFlags_AlwaysVerticalScrollbar);
	if (ImGui::BeginTable("windowTable", 2,
	                      ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable))
	{
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 16 + 4);
		ImGui::TableSetupColumn("Title");
		ImGui::TableHeadersRow();

		u32 index = 0;
		for (auto& window : m_windows)
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			if (const auto icon = window->GetIcon(); icon)
			{
				ImGui::Image((ImTextureID)icon->GetTexture(), ImVec2{icon->GetWidth(), icon->GetHeight()},
				             ImVec2(0, 1), ImVec2(1, 0));
			}

			ImGui::TableNextColumn();
			ImGui::PushID((i32)index + 1);

			bool useFallbackTitle = window->GetTitle().empty();
			const std::string& title = !useFallbackTitle ? window->GetTitle() : window->GetExecutableName();
			if (useFallbackTitle)
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(127, 127, 127, 255));

			// Here is a bit of a hack to get a stable id per index on the selectable in order to prevent flickering
			// when hovering a window that changes its title text rapidly. For some reason the selectable adds a small
			// padding to the column, so it should come *after* the text in order to avoid shifting the text.
			ImGui::TextUnformatted(!title.empty() ? title.c_str() : "???");
			ImGui::SameLine();
			if (ImGui::Selectable(
				"##sel",
				m_selectedIndex == index,
				ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap,
				ImVec2(0, 16)))
			{
				m_selectedIndex = index;
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetItemTooltip("%s", window->GetExecutablePathName().c_str());
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				ActivateWindow(window->GetHandle());
			}

			// CONTEXT MENU
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Open Executable Path"))
				{
					ShellOpenFolderAndSelectFile(window->GetExecutablePathNameW().c_str());
				}

				ImGui::EndPopup();
			}

			if (useFallbackTitle)
				ImGui::PopStyleColor();
			ImGui::PopID();

			index++;
		}

		ImGui::EndTable();
	}
	// 	ImGui::EndChild();
	//
	// 	ImGui::TableSetColumnIndex(1);
	// 	ImGui::Text("hello");
	// 	ImGui::EndTable();
	// }

	ImGui::End();
}

void App::DoInspectorWindow()
{
	ImGui::Begin("Inspector");

	if (m_selectedIndex >= m_windows.size())
	{
		ImGui::Text("Select a window!");
		ImGui::End();
		return;
	}

	HWND hwnd = m_windows[m_selectedIndex]->GetHandle();

	// POSITION AND SIZE
	if (!ImGui::IsAnyItemActive())
	{
		RECT rect{};
		GetWindowRect(hwnd, &rect);

		m_posEditBuffer[0] = rect.left;
		m_posEditBuffer[1] = rect.top;
		m_sizeEditBuffer[0] = rect.right - rect.left;
		m_sizeEditBuffer[1] = rect.bottom - rect.top;
	}

	ImGui::DragInt2("Position", m_posEditBuffer);
	bool posChanged = ImGui::IsItemDeactivatedAfterEdit();
	ImGui::DragInt2("Size", m_sizeEditBuffer);
	bool sizeChanged = ImGui::IsItemDeactivatedAfterEdit();

	if (posChanged || sizeChanged)
	{
		u32 flags = SWP_NOACTIVATE;
		flags |= posChanged ? 0 : SWP_NOMOVE;
		flags |= sizeChanged ? 0 : SWP_NOSIZE;
		SetWindowPos(hwnd, NULL,
		             m_posEditBuffer[0], m_posEditBuffer[1], m_sizeEditBuffer[0], m_sizeEditBuffer[1], flags);
	}

	// TOPMOST
	i32 exStyle = GetWindowLongW(hwnd, GWL_EXSTYLE);
	bool isTopMost = (exStyle & WS_EX_TOPMOST) != 0;
	if (ImGui::Checkbox("Topmost", &isTopMost))
	{
		SetWindowPos(hwnd, isTopMost ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
		             SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	// IS MINIMIZED
	WINDOWPLACEMENT windowPlacement{};
	windowPlacement.length = sizeof(WINDOWPLACEMENT);
	bool hasWindowPlacementInfo = GetWindowPlacement(hwnd, &windowPlacement);

	bool isMinimized = hasWindowPlacementInfo && windowPlacement.showCmd == SW_SHOWMINIMIZED;
	if (ImGui::Checkbox("Is Minimized", &isMinimized) && hasWindowPlacementInfo)
	{
		ShowWindow(hwnd, isMinimized ? SW_SHOWMINNOACTIVE : SW_SHOWNOACTIVATE);
	}

	// IS MAXIMIZED
	bool isMaximized = hasWindowPlacementInfo && windowPlacement.showCmd == SW_SHOWMAXIMIZED;
	if (ImGui::Checkbox("Is Maximized", &isMaximized) && hasWindowPlacementInfo)
	{
		// ShowWindow(hwnd, isMaximized ? SW_SHOWMAXIMIZED : SW_SHOWNOACTIVATE);
		if (isMaximized)
		{
			// Workaround to set maximized without activating the window. (There is no SW_SHOWMXIMIZEDNOACTIVE constant...)
			// This seems to work in most cases...
			PostMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
		}
		else
		{
			ShowWindow(hwnd, SW_SHOWNOACTIVATE);
		}
	}

	//
	// ACTION BUTTONS
	//
	ImGui::SeparatorText("Actions");

	if (ImGui::Button("Make Borderless Fullscreen"))
		MakeWindowBorderlessFullscreen(hwnd);

	if (ImGui::Button("Move Top Left"))
		MoveWindowTopLeft(hwnd);

	// Not very useful on its own...
	// if (ImGui::Button("Remove Border"))
	// 	RemoveWindowBorder(hwnd);

	ImGui::End();
}

void App::DoProfilesWindow()
{
	ImGui::Begin("Profiles");

	//
	// HEADER
	//
	if (ImGui::Button("Create Profile"))
	{
		m_profileToSave = ReadProfile();
		ImGui::OpenPopup("Save Profile");
	}

	//
	// PROFILE LIST
	//
	if (ImGui::BeginTable("profileList", 1,
	                      ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders))
	{
		for (auto& profile : m_settingsManager.GetProfiles())
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Selectable(profile.Name.c_str(), false, ImGuiSelectableFlags_SpanAllColumns);
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Pos(%i %i) Size(%i %i)",
				                  profile.Position.x, profile.Position.y,
				                  profile.Size.x, profile.Size.y);
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				ApplyProfile(profile);
			}

			// CONTEXT MENU
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Delete"))
				{
					m_settingsManager.DeleteProfile(profile.Name);
					m_settingsManager.Save();
				}

				ImGui::EndPopup();
			}
		}

		ImGui::EndTable();
	}

	//
	// SAVE MODAL
	//
	if (ImGui::BeginPopupModal("Save Profile"))
	{
		ImGui::Text("Save profile?");

		if (ImGui::IsWindowAppearing())
		{
			m_profileSaveNameEditBuffer[0] = '\0'; // set to empty string
			ImGui::SetKeyboardFocusHere();
		}
		ImGui::InputText("", m_profileSaveNameEditBuffer, _countof(m_profileSaveNameEditBuffer));

		ImGui::BeginDisabled(!IsValidProfileName({m_profileSaveNameEditBuffer, strlen(m_profileSaveNameEditBuffer)}));
		if (ImGui::Button("OK") || ImGui::IsKeyPressed(ImGuiKey_Enter))
		{
			m_profileToSave.Name = m_profileSaveNameEditBuffer;
			m_settingsManager.GetProfiles().push_back(m_profileToSave);
			m_settingsManager.Save();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();

		ImGui::SameLine();
		if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	ImGui::End();
}
