#pragma once
#include <imgui.h>

// V3 theme v1.1
// - rlyeh, public domain
// Changes from v1.1:
// - Header (selected item) now uses 'lo' (hue color) at 45% alpha — clearly visible on any row bg
// - HeaderHovered uses 'lo' at 70% alpha
// - HeaderActive uses 'hi' at full alpha
// - Grey steps spread further apart for better row bg contrast
// - TableRowBg and TableRowBgAlt use new wider-spaced greys
// - Cell padding changes
// - Changed separator alignment
inline int Theme_igThemeV3(int hue07, int alt07, int nav07, int lit01 = 0, int compact01 = 0, int border01 = 1,
                           int shape0123 = 1)
{
	bool rounded = shape0123 == 2;

	ImGuiStyle& style = ImGui::GetStyle();

	const float _8 = compact01 ? 4.0f : 8.0f;
	const float _4 = compact01 ? 2.0f : 4.0f;
	const float _2 = compact01 ? 1.0f : 2.0f;
	// const float _2 = compact01 ? 0.5 : 1;

	style.Alpha = 1.0f;
	style.DisabledAlpha = 0.3f;

	style.WindowPadding = ImVec2(4, _8);
	style.FramePadding = ImVec2(4, _4);
	style.ItemSpacing = ImVec2(_8, _2 + _2);
	style.ItemInnerSpacing = ImVec2(4, 4);
	style.IndentSpacing = 16;
	style.ScrollbarSize = compact01 ? 12.0f : 18.0f;
	style.GrabMinSize = compact01 ? 16.0f : 20.0f;

	style.WindowBorderSize = (float)border01;
	style.ChildBorderSize = (float)border01;
	style.PopupBorderSize = (float)border01;
	style.FrameBorderSize = 0;

	style.WindowRounding = 4;
	style.ChildRounding = 6;
	style.FrameRounding = shape0123 == 0 ? 0 : (float)(shape0123 == 1 ? 4 : 12);
	style.PopupRounding = 4;
	style.ScrollbarRounding = (float)rounded * 8 + 4;
	style.GrabRounding = style.FrameRounding;

	style.TabBorderSize = 0;
	style.TabBarBorderSize = 2;
	style.TabBarOverlineSize = 2;
	style.TabCloseButtonMinWidthSelected = -1;
	style.TabCloseButtonMinWidthUnselected = -1;
	style.TabRounding = rounded;

	//style.CellPadding = ImVec2(8.0f, 4.0f); // this causes table selection higlight to look small
	style.CellPadding = ImVec2(8.0f, 3.0f);

	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_Right;

	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.5f, 0.5f);
	//style.SeparatorTextAlign.x = 1.00f;
	style.SeparatorTextAlign.x = 0.10f;
	style.SeparatorTextBorderSize = 2;
	style.SeparatorTextPadding = ImVec2(0, 0);

	style.WindowMinSize = ImVec2(32.0f, 16.0f);
	style.ColumnsMinSpacing = 6.0f;

	style.CircleTessellationMaxError = shape0123 == 3 ? 4.00f : 0.30f;

	auto lit = [&](ImVec4 hi)
	{
		float h, s, v;
		ImGui::ColorConvertRGBtoHSV(hi.x, hi.y, hi.z, h, s, v);
		return ImColor::HSV(h, s * 0.80f, v * 1.00f, hi.w).Value;
	};
	auto dim = [&](ImVec4 hi)
	{
		float h, s, v;
		ImGui::ColorConvertRGBtoHSV(hi.x, hi.y, hi.z, h, s, v);
		ImVec4 d = ImColor::HSV(h, s, lit01 ? v * 0.65f : v * 0.65f, hi.w).Value;
		if (hi.z > hi.x && hi.z > hi.y) return ImVec4(d.x, d.y, hi.z, d.w);
		return d;
	};
	// Helper: colorize with alpha
	auto tint = [](ImVec4 c, float a) { return ImVec4(c.x, c.y, c.z, a); };

	const ImVec4 cyan = ImVec4(000 / 255.f, 192 / 255.f, 255 / 255.f, 1.00f);
	const ImVec4 red = ImVec4(230 / 255.f, 000 / 255.f, 000 / 255.f, 1.00f);
	const ImVec4 yellow = ImVec4(240 / 255.f, 210 / 255.f, 000 / 255.f, 1.00f);
	const ImVec4 orange = ImVec4(255 / 255.f, 144 / 255.f, 000 / 255.f, 1.00f);
	const ImVec4 lime = ImVec4(192 / 255.f, 255 / 255.f, 000 / 255.f, 1.00f);
	const ImVec4 aqua = ImVec4(000 / 255.f, 255 / 255.f, 192 / 255.f, 1.00f);
	const ImVec4 magenta = ImVec4(255 / 255.f, 000 / 255.f, 88 / 255.f, 1.00f);
	const ImVec4 purple = ImVec4(192 / 255.f, 000 / 255.f, 255 / 255.f, 1.00f);

	ImVec4 alt = cyan;
	/**/
	if (alt07 == 0 || alt07 == 'C') alt = cyan;
	else if (alt07 == 1 || alt07 == 'R') alt = red;
	else if (alt07 == 2 || alt07 == 'Y') alt = yellow;
	else if (alt07 == 3 || alt07 == 'O') alt = orange;
	else if (alt07 == 4 || alt07 == 'L') alt = lime;
	else if (alt07 == 5 || alt07 == 'A') alt = aqua;
	else if (alt07 == 6 || alt07 == 'M') alt = magenta;
	else if (alt07 == 7 || alt07 == 'P') alt = purple;
	if (lit01) alt = dim(alt);

	ImVec4 hi = cyan, lo = dim(cyan);
	/**/
	if (hue07 == 0 || hue07 == 'C') lo = dim(hi = cyan);
	else if (hue07 == 1 || hue07 == 'R') lo = dim(hi = red);
	else if (hue07 == 2 || hue07 == 'Y') lo = dim(hi = yellow);
	else if (hue07 == 3 || hue07 == 'O') lo = dim(hi = orange);
	else if (hue07 == 4 || hue07 == 'L') lo = dim(hi = lime);
	else if (hue07 == 5 || hue07 == 'A') lo = dim(hi = aqua);
	else if (hue07 == 6 || hue07 == 'M') lo = dim(hi = magenta);
	else if (hue07 == 7 || hue07 == 'P') lo = dim(hi = purple);
	// if( lit01 ) { ImVec4 tmp = hi; hi = lo; lo = lit(tmp); }

	ImVec4 nav = orange;
	/**/
	if (nav07 == 0 || nav07 == 'C') nav = cyan;
	else if (nav07 == 1 || nav07 == 'R') nav = red;
	else if (nav07 == 2 || nav07 == 'Y') nav = yellow;
	else if (nav07 == 3 || nav07 == 'O') nav = orange;
	else if (nav07 == 4 || nav07 == 'L') nav = lime;
	else if (nav07 == 5 || nav07 == 'A') nav = aqua;
	else if (nav07 == 6 || nav07 == 'M') nav = magenta;
	else if (nav07 == 7 || nav07 == 'P') nav = purple;
	if (lit01) nav = dim(nav);

	const ImVec4
		link = ImVec4(0.26f, 0.59f, 0.98f, 1.00f),
		// Greys spread further apart for visible row contrast
		grey0 = ImVec4(0.03f, 0.03f, 0.05f, 1.00f), // darkest (was 0.04/0.05/0.07)
		grey1 = ImVec4(0.07f, 0.08f, 0.10f, 1.00f), // window bg (was 0.08/0.09/0.11)
		grey2 = ImVec4(0.10f, 0.11f, 0.13f, 1.00f), // row alt bg — unchanged
		grey3 = ImVec4(0.15f, 0.16f, 0.19f, 1.00f), // row bg, was 0.12/0.13/0.15 — brighter
		grey4 = ImVec4(0.20f, 0.21f, 0.24f, 1.00f), // hovered frame (was 0.16/0.17/0.19)
		grey5 = ImVec4(0.24f, 0.25f, 0.28f, 1.00f); // active frame (was 0.18/0.19/0.21)

#define Luma(v,a) ImVec4((v)/100.f,(v)/100.f,(v)/100.f,(a)/100.f)

	style.Colors[ImGuiCol_Text] = Luma(100, 100);
	style.Colors[ImGuiCol_TextDisabled] = Luma(39, 100);
	style.Colors[ImGuiCol_WindowBg] = grey1;
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.10f, 0.12f, 1.00f);
	style.Colors[ImGuiCol_PopupBg] = grey1;
	style.Colors[ImGuiCol_Border] = grey4;
	style.Colors[ImGuiCol_BorderShadow] = grey1;
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.11f, 0.13f, 0.15f, 1.00f);
	style.Colors[ImGuiCol_FrameBgHovered] = grey4;
	style.Colors[ImGuiCol_FrameBgActive] = grey4;
	style.Colors[ImGuiCol_TitleBg] = grey0;
	style.Colors[ImGuiCol_TitleBgActive] = grey0;
	style.Colors[ImGuiCol_TitleBgCollapsed] = grey1;
	style.Colors[ImGuiCol_MenuBarBg] = grey2;
	style.Colors[ImGuiCol_ScrollbarBg] = grey0;
	style.Colors[ImGuiCol_ScrollbarGrab] = grey3;
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = lo;
	style.Colors[ImGuiCol_ScrollbarGrabActive] = hi;
	style.Colors[ImGuiCol_CheckMark] = alt;
	style.Colors[ImGuiCol_SliderGrab] = lo;
	style.Colors[ImGuiCol_SliderGrabActive] = hi;
	style.Colors[ImGuiCol_Button] = ImVec4(0.11f, 0.13f, 0.15f, 1.00f); //ImVec4(0.10f, 0.11f, 0.14f, 1.00f);
	style.Colors[ImGuiCol_ButtonHovered] = lo;
	style.Colors[ImGuiCol_ButtonActive] = grey5;
	// Selection is now clearly visible using hue color
	style.Colors[ImGuiCol_Header] = tint(lo, 0.45f); // selected, not hovered — hue color at 45% alpha
	style.Colors[ImGuiCol_HeaderHovered] = tint(lo, 0.70f); // hovered — brighter
	style.Colors[ImGuiCol_HeaderActive] = hi; // clicked — full hue
	style.Colors[ImGuiCol_Separator] = ImVec4(0.13f, 0.15f, 0.19f, 1.00f);
	style.Colors[ImGuiCol_SeparatorHovered] = lo;
	style.Colors[ImGuiCol_SeparatorActive] = hi;
	style.Colors[ImGuiCol_ResizeGrip] = Luma(15, 100);
	style.Colors[ImGuiCol_ResizeGripHovered] = lo;
	style.Colors[ImGuiCol_ResizeGripActive] = hi;
	style.Colors[ImGuiCol_InputTextCursor] = Luma(100, 100);
	style.Colors[ImGuiCol_TabHovered] = grey3;
	style.Colors[ImGuiCol_Tab] = grey1;
	style.Colors[ImGuiCol_TabSelected] = grey3;
	style.Colors[ImGuiCol_TabSelectedOverline] = hi;
	style.Colors[ImGuiCol_TabDimmed] = grey1;
	style.Colors[ImGuiCol_TabDimmedSelected] = grey1;
	style.Colors[ImGuiCol_TabDimmedSelectedOverline] = lo;
	style.Colors[ImGuiCol_DockingPreview] = grey1;
	style.Colors[ImGuiCol_DockingEmptyBg] = Luma(20, 100);
	style.Colors[ImGuiCol_PlotLines] = grey5;
	style.Colors[ImGuiCol_PlotLinesHovered] = lo;
	style.Colors[ImGuiCol_PlotHistogram] = grey5;
	style.Colors[ImGuiCol_PlotHistogramHovered] = lo;
	style.Colors[ImGuiCol_TableHeaderBg] = grey0;
	style.Colors[ImGuiCol_TableBorderStrong] = grey0;
	style.Colors[ImGuiCol_TableBorderLight] = grey0;
	// Wider gap between row bg and alt for visible striping
	style.Colors[ImGuiCol_TableRowBg] = grey3; // brighter base row
	style.Colors[ImGuiCol_TableRowBgAlt] = grey2; // noticeably darker alt row
	style.Colors[ImGuiCol_TextLink] = link;
	style.Colors[ImGuiCol_TextSelectedBg] = Luma(39, 100);
	style.Colors[ImGuiCol_TreeLines] = Luma(39, 100);
	style.Colors[ImGuiCol_DragDropTarget] = nav;
	style.Colors[ImGuiCol_NavCursor] = nav;
	style.Colors[ImGuiCol_NavWindowingHighlight] = lo;
	style.Colors[ImGuiCol_NavWindowingDimBg] = Luma(0, 63);
	style.Colors[ImGuiCol_ModalWindowDimBg] = Luma(0, 63);

	if (lit01)
	{
		for (int i = 0; i < ImGuiCol_COUNT; i++)
		{
			float H, S, V;
			ImVec4& col = style.Colors[i];
			ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);
			if (S < 0.5f)
			{
				V = 1.0f - V;
				S *= 0.15f;
			}

			ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
		}
	}

#undef Luma
	return 0;
}

inline void Theme_ShowDebugWindow()
{
	ImGui::Begin("THEME");
	bool theme = false;
	static int hue = 0, alt = 0, nav = 0, shape = 1;  // NOLINT(clang-diagnostic-unique-object-duplication)
	static bool light = 0, compact = 0, borders = 0;  // NOLINT(clang-diagnostic-unique-object-duplication)
	theme |= ImGui::SliderInt("Hue", &hue, 0, 7);
	theme |= ImGui::SliderInt("Alt", &alt, 0, 7);
	theme |= ImGui::SliderInt("Nav", &nav, 0, 7);
	theme |= ImGui::SliderInt("Shape", &shape, 0, 3);
	theme |= ImGui::Checkbox("Light", &light);
	theme |= ImGui::Checkbox("Compact", &compact);
	theme |= ImGui::Checkbox("Borders", &borders);
	if (theme) Theme_igThemeV3(hue["CRYOLAMP"], alt["CRYOLAMP"], nav["CRYOLAMP"], light, compact, borders, shape);
	ImGui::End();
}

/// Theme index 0..=7
inline void Theme_ApplyAppTheme(i32 index = 3, bool light = false)
{
	int color = sk::Clamp(index, 0, 7);
	Theme_igThemeV3(color, color, color, light ? 1 : 0, 0, 1, 1);
}
