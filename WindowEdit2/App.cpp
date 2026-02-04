#include "App.h"

struct IntVec2
{
	i32 x;
	i32 y;
	IntVec2() = default;
	IntVec2(i32 x, i32 y) : x(x), y(y) {}
};

struct Rect : RECT
{
	i32 X() const { return left; }
	i32 Y() const { return top; }
	i32 Width() const { return right - left; }
	i32 Height() const { return bottom - top; }
	IntVec2 GetPos() const { return {X(), Y()}; }
	IntVec2 GetSize() const { return {Width(), Height()}; }
};

void App::Render()
{
	i32 keyCount;
	const bool* keyboardState = SDL_GetKeyboardState(&keyCount);
	bool isHoldingCtrl = SDL_SCANCODE_LCTRL < keyCount && keyboardState[SDL_SCANCODE_LCTRL];

	if (!isHoldingCtrl)
	{
		RefreshWindowList();
	}

	ImGui::DockSpaceOverViewport(ImGui::GetID("fullscreenDockspace"), ImGui::GetMainViewport());

	ImGui::Begin("Window List");

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
			ImGui::PushID(index + 1);

			bool useFallbackTitle = window->GetTitle().size() == 0;
			const std::string& title = !useFallbackTitle ? window->GetTitle() : window->GetExecutableName();
			if (useFallbackTitle)
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(127, 127, 127, 255));

			if (ImGui::Selectable(
				title.size() > 0 ? title.c_str() : "???",
				m_selectedIndex == index,
				ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap,
				ImVec2(0, 16)))
			{
				m_selectedIndex = index;
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



	ImGui::Begin("Inspector");
	if (m_selectedIndex < m_windows.size())
	{
		HWND hwnd = m_windows[m_selectedIndex]->GetHandle();

		// Position and Size
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


		// Topmost
		i32 exStyle = GetWindowLongW(hwnd, GWL_EXSTYLE);
		bool isTopMost = (exStyle & WS_EX_TOPMOST) != 0;
		if (ImGui::Checkbox("Topmost", &isTopMost))
		{
			SetWindowPos(hwnd, isTopMost ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
			             SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}


		// Is Minimized
		WINDOWPLACEMENT windowPlacement{};
		windowPlacement.length = sizeof(WINDOWPLACEMENT);
		bool hasWindowPlacementInfo = GetWindowPlacement(hwnd, &windowPlacement);

		bool isMinimized = hasWindowPlacementInfo && windowPlacement.showCmd == SW_SHOWMINIMIZED;
		if (ImGui::Checkbox("Is Minimized", &isMinimized) && hasWindowPlacementInfo)
		{
			ShowWindow(hwnd, isMinimized ? SW_SHOWMINNOACTIVE : SW_SHOWNOACTIVATE);
		}

		// Is Maximized
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


		// Buttons
		ImGui::SeparatorText("Actions");

		if (ImGui::Button("Make Borderless Fullscreen"))
			MakeWindowBorderlessFullscreen(hwnd);

		if (ImGui::Button("Move Top Left"))
			MoveWindowTopLeft(hwnd);

		// Not very useful on its own...
		// if (ImGui::Button("Remove Border"))
		// 	RemoveWindowBorder(hwnd);
	}
	else
	{
		ImGui::Text("Select a window!");
	}
	ImGui::End();
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
	app->m_windows.emplace_back(std::make_unique<WindowModel>(hWnd));
	app->m_windows.back()->FetchMetadata(app->m_iconCache);
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
