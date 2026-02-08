#include "App.h"
#include <memory>
#include <imgui.h>

#include "common.h"
#include "WindowModel.h"

void App::Init(HWND ownWindowHandle, SDL_Window* window, SDL_Renderer* renderer)
{
	m_appWindowHandle = ownWindowHandle;
	m_appWindow = window;
	m_renderer = renderer;
	m_iconCache.Init(renderer);

	m_settingsManager.Init();
	m_settingsManager.Load();
	SDL_SetWindowSize(window, m_settingsManager.m_windowSize.x, m_settingsManager.m_windowSize.y);

	m_windowList.Init(*this);
}

void App::Shutdown()
{
	m_settingsManager.Save();
}

void App::Render()
{
	SDL_GetWindowSize(m_appWindow, &m_settingsManager.m_windowSize.x, &m_settingsManager.m_windowSize.y);

	SDL_WindowFlags flags = SDL_GetWindowFlags(m_appWindow);
	bool isWindowActive = flags & (SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS);

	i32 keyCount;
	const bool* keyboardState = SDL_GetKeyboardState(&keyCount);
	bool isHoldingCtrl = SDL_SCANCODE_LCTRL < keyCount && keyboardState[SDL_SCANCODE_LCTRL];

	if (!isHoldingCtrl)
	{
		m_windowList.Refresh(m_settingsManager.m_showAllWindows);
	}

	if (isWindowActive && // just a sanity check, keyboard focus is naturally required for input.
		isHoldingCtrl &&
		SDL_SCANCODE_Z < keyCount && keyboardState[SDL_SCANCODE_Z] && !m_oldKeyboardState[SDL_SCANCODE_Z])
	{
		PerformUndo();
	}

	ImGui::DockSpaceOverViewport(ImGui::GetID("fullscreenDockspace"), ImGui::GetMainViewport());

	DoWindowListWindow();
	DoInspectorWindow();
	DoProfilesWindow();

	// ImGui::Begin("Font Debug");
	// ImGui::ShowFontAtlas(ImGui::GetFont()->OwnerAtlas);
	// ImGui::End();

	std::memcpy(m_oldKeyboardState, keyboardState, min(_countof(m_oldKeyboardState), keyCount) * sizeof(bool));
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

void App::ActivateWindow(HWND hwnd)
{
	if (IsIconic(hwnd)) // is minimized?
	{
		ShowWindow(hwnd, SW_RESTORE);
	}

	SetForegroundWindow(hwnd);

	SetFocus(hwnd);
}

void App::MoveWindowToCenterOfPrimaryMonitor(HWND hwnd, bool activate)
{
	//HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	// Get primary monitor
	HMONITOR monitor = MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO monitorInfo{};
	monitorInfo.cbSize = sizeof(MONITORINFO);

	if (!GetMonitorInfo(monitor, &monitorInfo))
		return;

	Rect monitorRect = monitorInfo.rcMonitor;
	Rect windowRect;
	if (!GetWindowRect(hwnd, &windowRect))
		return;

	i32 px = monitorRect.X() + monitorRect.Width() / 2 - windowRect.Width() / 2;
	i32 py = monitorRect.Y() + monitorRect.Height() / 2 - windowRect.Height() / 2;

	u32 flags = SWP_NOSIZE;
	if (!activate)
		flags |= SWP_NOACTIVATE;

	SetWindowPos(hwnd, NULL, px, py, 0, 0, flags);
}

bool App::IsBorderless(HWND hwnd)
{
	i32 style = GetWindowLong(hwnd, GWL_STYLE);
	return (style & (WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU)) == 0;
}

void App::SetBorderless(WindowModel& window, bool state, bool updateSize)
{
	// TODO: Toggling borderless updates the size of some programs incorrectly, eg. Discord.
	//   this also breaks undo.

	HWND hwnd = window.GetHandle();
	i32 newStyle = 0;
	i32 newExStyle = 0;
	if (state)
	{
		i32 style = GetWindowLong(hwnd, GWL_STYLE);
		newStyle = style & ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
		if (style != newStyle)
		{
			SetWindowLong(hwnd, GWL_STYLE, newStyle);
			window.BorderlessRemovedStyleFlags |= style & ~newStyle;
		}

		i32 exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
		newExStyle = exStyle & ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
		if (exStyle != newExStyle)
		{
			SetWindowLong(hwnd, GWL_EXSTYLE, newExStyle);
			window.BorderlessRemovedExStyleFlags |= exStyle & ~newExStyle;
		}

		if (updateSize)
		{
			Rect clientRect;
			if (GetClientRect(hwnd, &clientRect))
			{
				SetWindowPos(hwnd, NULL, 0, 0, clientRect.Width(), clientRect.Height(), SWP_NOACTIVATE | SWP_NOMOVE);
			}
		}
	}
	else
	{
		Rect clientRect;
		bool hasClientRect = updateSize && GetClientRect(hwnd, &clientRect);

		// If we have stored the removed styles we can restore them.
		if (window.BorderlessRemovedStyleFlags != 0 && window.BorderlessRemovedExStyleFlags != 0)
		{
			i32 style = GetWindowLong(hwnd, GWL_STYLE);
			newStyle = style | window.BorderlessRemovedStyleFlags;
			SetWindowLong(hwnd, GWL_STYLE, newStyle);

			i32 exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
			newExStyle = exStyle | window.BorderlessRemovedExStyleFlags;
			SetWindowLong(hwnd, GWL_EXSTYLE, newExStyle);
		}
		else
		{
			// Apply the sensible "default" styles...
			i32 style = GetWindowLong(hwnd, GWL_STYLE);
			newStyle = style | (WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
			SetWindowLong(hwnd, GWL_STYLE, newStyle);

			newExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
			// Set WS_EX_WINDOWEDGE ?
		}

		if (updateSize && hasClientRect &&
			// Calculate the new size of the window to preserve the size of the client area.
			AdjustWindowRectEx(&clientRect, newStyle, GetMenu(hwnd) != NULL, newExStyle))
		{
			SetWindowPos(hwnd, NULL, 0, 0, clientRect.Width(), clientRect.Height(), SWP_NOACTIVATE | SWP_NOMOVE);
		}
	}
}

void App::MakeWindowFullscreen(HWND hwnd)
{
	HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO monitorInfo{};
	monitorInfo.cbSize = sizeof(monitorInfo);
	if (!GetMonitorInfo(monitor, &monitorInfo))
	{
		return;
	}

	Rect monitorRect = (Rect)monitorInfo.rcMonitor;
	SetWindowPos(hwnd, NULL,
	             monitorRect.X(), monitorRect.Y(), monitorRect.Width(), monitorRect.Height(),SWP_NOACTIVATE);
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
	if (!m_windowList.IsSelectedValid())
		return;

	Rect oldRect;
	bool hasOldRect = GetWindowRect(m_windowList.GetSelected()->GetHandle(), &oldRect);

	SetWindowPos(m_windowList.GetSelected()->GetHandle(), NULL,
	             profile.Position.x, profile.Position.y,
	             profile.Size.x, profile.Size.y,
	             SWP_NOACTIVATE);

	if (hasOldRect)
	{
		PushUndo({
			.Window = m_windowList.GetSelected(),
			.Type = HistoryItemType::SetPosition,
			.Position = oldRect.GetPos(),
		});

		PushUndo({
			.Window = m_windowList.GetSelected(),
			.Type = HistoryItemType::SetSize,
			.MergeWithPrevious = true,
			.Size = oldRect.GetSize(),
		});
	}
}

void App::DoWindowListWindow()
{
	ImGui::Begin("Window List");

	if (ImGui::Checkbox("Show All Windows", &m_settingsManager.m_showAllWindows))
	{
		m_settingsManager.Save();
	}

	if (ImGui::BeginTable("windowTable", 2,
	                      ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable))
	{
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 16 + 4);
		ImGui::TableSetupColumn("Title");
		ImGui::TableHeadersRow();

		u32 index = 0;
		for (auto& window : m_windowList.GetWindows())
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
				m_windowList.GetSelectedIndex() == index,
				ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap,
				ImVec2(0, 16)))
			{
				m_windowList.SetSelectedIndex(index);
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

	ImGui::End();
}

void App::DoInspectorWindow()
{
	ImGui::Begin("Inspector");

	if (!m_windowList.IsSelectedValid())
	{
		ImGui::Text("Select a window!");
		ImGui::End();
		return;
	}

	auto& window = m_windowList.GetSelected();
	HWND hwnd = m_windowList.GetSelected()->GetHandle();

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

		Rect oldRect;
		bool hasOldRect = GetWindowRect(hwnd, &oldRect);
		SetWindowPos(hwnd, NULL,
		             m_posEditBuffer[0], m_posEditBuffer[1], m_sizeEditBuffer[0], m_sizeEditBuffer[1], flags);

		// Record undo
		if (hasOldRect)
		{
			bool both = posChanged && sizeChanged;
			if (posChanged)
			{
				PushUndo({
					.Window = window,
					.Type = HistoryItemType::SetPosition,
					.Position = oldRect.GetPos(),
				});
			}
			if (sizeChanged)
			{
				PushUndo({
					.Window = window,
					.Type = HistoryItemType::SetSize,
					.MergeWithPrevious = both,
					.Size = oldRect.GetSize(),
				});
			}
		}
	}

	// TOPMOST
	i32 exStyle = GetWindowLongW(hwnd, GWL_EXSTYLE);
	bool isTopMost = (exStyle & WS_EX_TOPMOST) != 0;
	if (ImGui::Checkbox("Topmost", &isTopMost))
	{
		SetWindowPos(hwnd, isTopMost ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
		             SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

		PushUndo({
			.Window = window,
			.Type = HistoryItemType::SetTopmost,
			.Topmost = !isTopMost,
		});
	}

	// IS MINIMIZED
	WINDOWPLACEMENT windowPlacement{};
	windowPlacement.length = sizeof(WINDOWPLACEMENT);
	bool hasWindowPlacementInfo = GetWindowPlacement(hwnd, &windowPlacement);

	bool isMinimized = hasWindowPlacementInfo && windowPlacement.showCmd == SW_SHOWMINIMIZED;
	if (ImGui::Checkbox("Is Minimized", &isMinimized) && hasWindowPlacementInfo)
	{
		ShowWindow(hwnd, isMinimized ? SW_SHOWMINNOACTIVE : SW_SHOWNOACTIVATE);

		PushUndo({
			.Window = window,
			.Type = HistoryItemType::SetMinimized,
			.Minimized = !isMinimized,
		});
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

		PushUndo({
			.Window = window,
			.Type = HistoryItemType::SetMaximized,
			.Maximized = !isMaximized,
		});
	}

	// IS BORDERLESS
	bool isBorderless = IsBorderless(hwnd);
	if (ImGui::Checkbox("Is Borderless", &isBorderless))
	{
		SetBorderless(*window, isBorderless, true);

		PushUndo({
			.Window = window,
			.Type = HistoryItemType::SetBorderless,
			.Borderless = !isBorderless,
		});
	}


	//
	// ACTION BUTTONS
	//
	ImGui::SeparatorText("Actions");

	if (ImGui::Button("Make Borderless Fullscreen"))
	{
		Rect oldRect;
		bool hasOldRect = GetWindowRect(hwnd, &oldRect);

		SetBorderless(*window, true, false);
		MakeWindowFullscreen(hwnd);

		PushUndo({
			.Window = window,
			.Type = HistoryItemType::SetBorderless,
			.Borderless = false,
		});

		if (hasOldRect)
		{
			PushUndo({
				.Window = window,
				.Type = HistoryItemType::SetPosition,
				.MergeWithPrevious = true,
				.Position = oldRect.GetPos(),
			});

			PushUndo({
				.Window = window,
				.Type = HistoryItemType::SetSize,
				.MergeWithPrevious = true,
				.Size = oldRect.GetSize(),
			});
		}
	}

	// TODO: When doing this on a maximized window, the resulting position is offset.
	if (ImGui::Button("Bring to Center"))
	{
		bool wasMinimized = IsIconic(hwnd);
		if (wasMinimized)
		{
			ShowWindow(hwnd, SW_RESTORE);

			PushUndo({
				.Window = window,
				.Type = HistoryItemType::SetMinimized,
				.Minimized = true,
			});
		}

		Rect oldRect;
		bool hasOldRect = GetWindowRect(hwnd, &oldRect);

		MoveWindowToCenterOfPrimaryMonitor(hwnd, true);

		if (hasOldRect)
		{
			PushUndo({
				.Window = window,
				.Type = HistoryItemType::SetPosition,
				.MergeWithPrevious = wasMinimized,
				.Position = oldRect.GetPos(),
			});

			PushUndo({
				.Window = window,
				.Type = HistoryItemType::SetSize,
				.MergeWithPrevious = true,
				.Size = oldRect.GetSize(),
			});
		}
	}

	if (ImGui::Button("Move Top Left"))
	{
		Rect oldRect;
		bool hasOldRect = GetWindowRect(hwnd, &oldRect);

		MoveWindowTopLeft(hwnd);

		if (hasOldRect)
		{
			PushUndo({
				.Window = window,
				.Type = HistoryItemType::SetPosition,
				.Position = oldRect.GetPos(),
			});
		}
	}

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

		bool canSaveProfile = IsValidProfileName({m_profileSaveNameEditBuffer, strlen(m_profileSaveNameEditBuffer)});
		ImGui::BeginDisabled(!canSaveProfile);
		if ((ImGui::Button("OK") || ImGui::IsKeyPressed(ImGuiKey_Enter)) && canSaveProfile)
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

void App::PushUndo(const HistoryItem& undo)
{
	// Delete the oldest items to fit capacity.
	while (m_undoBuffer.size() >= k_maxUndoCapacity)
	{
		m_undoBuffer.pop_front();
	}

	m_undoBuffer.push_back(undo);
}

void App::PerformUndo()
{
	while (!m_undoBuffer.empty())
	{
		bool keepGoing = false;

		if (m_undoBuffer.back().Window->VerifyHandle())
		{
			ApplyUndo(m_undoBuffer.back());
			keepGoing |= m_undoBuffer.back().MergeWithPrevious;
		}
		else
		{
			keepGoing = true;
		}

		m_undoBuffer.pop_back();

		if (!keepGoing)
		{
			break;
		}
	}
}

void App::ApplyUndo(const HistoryItem& undo)
{
	HWND hwnd = undo.Window->GetHandle();
	switch (undo.Type)
	{
	case HistoryItemType::SetPosition:
		SetWindowPos(hwnd, NULL, undo.Position.x, undo.Position.y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE);
		break;
	case HistoryItemType::SetSize:
		SetWindowPos(hwnd, NULL, 0, 0, undo.Size.x, undo.Size.y, SWP_NOACTIVATE | SWP_NOMOVE);
		break;
	case HistoryItemType::SetTopmost:
		SetWindowPos(hwnd, undo.Topmost ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
		             SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		break;
	case HistoryItemType::SetMinimized:
		ShowWindow(hwnd, undo.Minimized ? SW_SHOWMINNOACTIVE : SW_SHOWNOACTIVATE);
		break;
	case HistoryItemType::SetMaximized:
		if (undo.Maximized)
		{
			// Workaround to set maximized without activating the window. (There is no SW_SHOWMXIMIZEDNOACTIVE constant...)
			// This seems to work in most cases...
			PostMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
		}
		else
		{
			ShowWindow(hwnd, SW_SHOWNOACTIVATE);
		}
		break;
	case HistoryItemType::SetBorderless:
		SetBorderless(*undo.Window, undo.Borderless, true);
		break;
	}
}
