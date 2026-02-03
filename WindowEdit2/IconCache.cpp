#include "IconCache.h"

#include <vector>

#include "sk/strings.h"
#include <SDL3/SDL.h>
#include <psapi.h>

// static std::wstring GetProcessNameW(/*u32 processId*/ HWND windowHandle)
// {
// 	std::wstring result;
//
// 	DWORD processId;
// 	DWORD threadId = GetWindowThreadProcessId(windowHandle, &processId);
// 	if (threadId == 0)
// 	{
// 		SK_LOG_D("Failed to get process id for window %p\n", windowHandle);
// 		return result;
// 	}
//
// 	HANDLE processHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, processId);
// 	if (processHandle == nullptr)
// 	{
// 		SK_LOG_D("Failed to open process %u\n", processId);
// 		return result;
// 	}
// 	SK_DEFER(CloseHandle(processHandle));
//
// 	// GetWindowModuleFileName only works on windows in the same process.
// 	// u32 stringSize = GetWindowModuleFileName(windowHandle, nameBuffer, _countof(nameBuffer));
//
// 	wchar_t nameBuffer[256];
// 	u32 stringSize = GetModuleFileNameEx(processHandle, NULL, nameBuffer, _countof(nameBuffer));
// 	if (stringSize == 0)
// 		return result;
//
// 	result.resize(stringSize); // skip null terminator
// 	std::memcpy(result.data(), nameBuffer, stringSize * sizeof(wchar_t));
// 	return result;
// }
//
// static std::shared_ptr<IconTexture> GetExeIcon(
// 	SDL_Renderer* renderer,
// 	const wchar_t* fileName)
// {
// 	HICON iconHandle;
// 	u32 iconCount = ExtractIconExW(fileName, 0, nullptr, &iconHandle, 1);
// 	if (iconCount != 1)
// 	{
// 		SK_LOG_D("Failed to get icon for exe: '%s'\n", sk::ConvertWStringToUtf8(fileName).c_str());
// 		return {};
// 	}
//
// 	SK_DEFER(DestroyIcon(iconHandle));
//
// 	ICONINFOEXW iconInfo;
// 	iconInfo.cbSize = sizeof(iconInfo);
// 	if (!GetIconInfoExW(iconHandle, &iconInfo))
// 	{
// 		SK_LOG_D("Failed to get icon info for exe: '%s'\n", sk::ConvertWStringToUtf8(fileName).c_str());
// 		return {};
// 	}
//
// 	SK_DEFER(DeleteObject(iconInfo.hbmMask));
// 	SK_DEFER(DeleteObject(iconInfo.hbmColor));
//
// 	if (!iconInfo.hbmColor)
// 	{
// 		SK_LOG_D("Failed to get icon bitmap for exe: '%s'\n", sk::ConvertWStringToUtf8(fileName).c_str());
// 		return {};
// 	}
//
// 	BITMAP iconBitmapHeader;
// 	if (!GetObject(iconInfo.hbmColor, sizeof(BITMAP), &iconBitmapHeader))
// 	{
// 		SK_LOG_D("Failed to get icon bitmap for exe: '%s'\n", sk::ConvertWStringToUtf8(fileName).c_str());
// 		return {};
// 	}
//
// 	//SK_LOG("Icon size for exe '%s': %i x %i\n", sk::ConvertWStringToUtf8(fileName).c_str(), iconBitmapHeader.bmWidth, iconBitmapHeader.bmHeight);
//
// 	HDC gdiContext = CreateCompatibleDC(nullptr);
// 	SK_DEFER(DeleteDC(gdiContext));
//
// 	LONG size = 16; // TODO:
//
// 	BITMAPINFO bitmapInfo = {
// 		.bmiHeader = {
// 			.biSize = sizeof(BITMAPINFO),
// 			.biWidth = size,
// 			.biHeight = size,
// 			.biPlanes = 1,
// 			.biBitCount = 32,
// 			.biCompression = BI_RGB,
// 			.biSizeImage = 0,
// 			.biXPelsPerMeter = 0,
// 			.biYPelsPerMeter = 0,
// 			.biClrUsed = 0,
// 			.biClrImportant = 0,
// 		},
// 		.bmiColors = {}
// 	};
// 	std::vector<u8> rawBuffer(size * size * 4);
// 	i32 linesCopied = GetDIBits(gdiContext, iconInfo.hbmColor, 0, size, rawBuffer.data(), &bitmapInfo, 0);
// 	if (linesCopied != size)
// 	{
// 		SK_LOG_E("Failed to copy bitmap for exe: '%s'\n", sk::ConvertWStringToUtf8(fileName).c_str());
// 		return {};
// 	}
//
// 	// TODO: Ensure color formate from DI bits is always BGRA32, or handle this.
// 	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STATIC, size, size);
// 	if (!texture)
// 	{
// 		SK_LOG_E("Failed to create texture\n");
// 		return {};
// 	}
// 	SDL_Rect rect = {0, 0, size, size};
// 	SDL_UpdateTexture(texture, &rect, rawBuffer.data(), size * 4);
//
// 	return std::make_shared<IconTexture>(texture);
// }

// IconTextureRef IconCache::GetIconForWindow(HWND hwnd)
// {
// 	// DWORD processId;
// 	// DWORD threadId = GetWindowThreadProcessId(hwnd, &processId);
// 	//
// 	// if (threadId == 0)
// 	// {
// 	// 	SK_LOG_E("Failed to GetWindowThreadProcessId(%p)", hwnd);
// 	// 	return;
// 	// }
//
// 	std::wstring processName = GetProcessNameW(hwnd);
// 	if (!processName.empty())
// 	{
// 		return GetExeIcon(m_renderer, processName.data());
// 	}
//
// 	return {};
// }

IconTextureRef IconCache::GetIconForProcessFileName(const std::wstring& fileName)
{
	if (fileName.empty())
	{
		return {};
	}

	// Lookup in the cache.
	auto cacheEntry = m_cache.find(fileName);
	if (cacheEntry != m_cache.end())
	{
		return cacheEntry->second;
	}

	HICON iconHandle;
	u32 iconCount = ExtractIconExW(fileName.c_str(), 0, nullptr, &iconHandle, 1);
	if (iconCount != 1)
	{
		SK_LOG_D("Failed to get icon for exe: '%s'\n", sk::ConvertWStringToUtf8(fileName.c_str()).c_str());
		return {};
	}

	SK_DEFER(DestroyIcon(iconHandle));

	ICONINFOEXW iconInfo;
	iconInfo.cbSize = sizeof(iconInfo);
	if (!GetIconInfoExW(iconHandle, &iconInfo))
	{
		SK_LOG_D("Failed to get icon info for exe: '%s'\n", sk::ConvertWStringToUtf8(fileName.c_str()).c_str());
		return {};
	}

	SK_DEFER(DeleteObject(iconInfo.hbmMask));
	SK_DEFER(DeleteObject(iconInfo.hbmColor));

	if (!iconInfo.hbmColor)
	{
		SK_LOG_D("Failed to get icon bitmap for exe: '%s'\n", sk::ConvertWStringToUtf8(fileName.c_str()).c_str());
		return {};
	}

	BITMAP iconBitmapHeader;
	if (!GetObject(iconInfo.hbmColor, sizeof(BITMAP), &iconBitmapHeader))
	{
		SK_LOG_D("Failed to get icon bitmap for exe: '%s'\n", sk::ConvertWStringToUtf8(fileName.c_str()).c_str());
		return {};
	}

	//SK_LOG("Icon size for exe '%s': %i x %i\n", sk::ConvertWStringToUtf8(fileName.c_str()).c_str(), iconBitmapHeader.bmWidth, iconBitmapHeader.bmHeight);

	HDC gdiContext = CreateCompatibleDC(nullptr);
	SK_DEFER(DeleteDC(gdiContext));

	const LONG size = 16; // TODO:

	BITMAPINFO bitmapInfo = {
		.bmiHeader = {
			.biSize = sizeof(BITMAPINFO),
			.biWidth = size,
			.biHeight = size,
			.biPlanes = 1,
			.biBitCount = 32,
			.biCompression = BI_RGB,
			.biSizeImage = 0,
			.biXPelsPerMeter = 0,
			.biYPelsPerMeter = 0,
			.biClrUsed = 0,
			.biClrImportant = 0,
		},
		.bmiColors = {}
	};
	std::vector<u8> rawBuffer((size_t)size * size * 4);
	i32 linesCopied = GetDIBits(gdiContext, iconInfo.hbmColor, 0, size, rawBuffer.data(), &bitmapInfo, 0);
	if (linesCopied != size)
	{
		SK_LOG_E("Failed to copy bitmap for exe: '%s'\n", sk::ConvertWStringToUtf8(fileName.c_str()).c_str());
		return {};
	}

	// TODO: Ensure color formate from DI bits is always BGRA32, or handle this.
	SDL_Texture* texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STATIC, size, size);
	if (!texture)
	{
		SK_LOG_E("Failed to create texture\n");
		return {};
	}
	SDL_Rect rect = {0, 0, size, size};
	SDL_UpdateTexture(texture, &rect, rawBuffer.data(), size * 4);

	IconTextureRef iconRef = std::make_shared<IconTexture>(texture);
	m_cache.insert({fileName, iconRef});

	return iconRef;
}
