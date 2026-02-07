#include <Windows.h>
#include <cstdint>
#include <cstdio>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "sk/sk.h"
#include "App.h"

#pragma comment(lib, "SDL3.lib")

// TODO: We could go slower if we decouple window message processing from this delay.
#define THROTTLE_WHILE_INACTIVE_MS 100
#if _DEBUG
#define DEBUG_SHOW_FPS 1
#endif

// Embedded font
#include "unifont_compressed.h"
// For decompressing the embedded font
#include <zlib.h>

// Decompresses a deflate blob and returns new memory. You are responsible for calling delete[]!
SK_INTERNAL char* ZlibDecompress(const unsigned char* compressedData, u32 compressedSize, u32 uncompressedSize)
{
	SK_VERIFY(compressedData != nullptr);
	SK_VERIFY(uncompressedSize >= compressedSize);

	char* uncompressedData = new char[uncompressedSize];

	z_stream stream{};
	stream.next_in = (Bytef*)compressedData;
	stream.avail_in = compressedSize;
	stream.next_out = (Bytef*)uncompressedData;
	stream.avail_out = uncompressedSize;

	if (inflateInit(&stream) != Z_OK)
	{
		delete[] uncompressedData;
		return nullptr;
	}

	int ret = inflate(&stream, Z_FINISH);
	inflateEnd(&stream);

	// if (ret != Z_STREAM_END)
	// {
	// 	delete[] uncompressedData;
	// 	return nullptr;
	// }

	SK_VERIFY(ret == Z_STREAM_END);
	SK_VERIFY(stream.total_out == uncompressedSize);

	return uncompressedData;
}

SK_INTERNAL char* LoadEmbeddedFont(u32* outFontDataSize)
{
	char* data = ZlibDecompress((const unsigned char*)g_unifontTTF_compressed_data, g_unifontTTF_compressed_size, g_unifontTTF_uncompressed_size);
	*outFontDataSize = data ? g_unifontTTF_uncompressed_size : 0;
	return data;
}

int SDL_main(int argc, char* argv[])
{
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	float mainScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
	SDL_WindowFlags windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN |
		SDL_WINDOW_HIGH_PIXEL_DENSITY;
	SDL_Window* window = SDL_CreateWindow("WindowEdit 2.0", (i32)(800 * mainScale), (i32)(500 * mainScale),
	                                      windowFlags);
	if (window == NULL)
	{
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
	if (!renderer)
	{
		printf("Error: SDL_CreateRenderer(): %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	SDL_SetRenderVSync(renderer, 1);
	SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup scaling
	// TODO: ...
	// Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(mainScale);
	style.FontScaleDpi = mainScale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)

	// Setup Platform/Renderer backends
	ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer3_Init(renderer);

	// Load Fonts
	// - If fonts are not explicitly loaded, Dear ImGui will call AddFontDefault() to select an embedded font: either AddFontDefaultVector() or AddFontDefaultBitmap().
	//   This selection is based on (style.FontSizeBase * style.FontScaleMain * style.FontScaleDpi) reaching a small threshold.
	// - You can load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - If a file cannot be loaded, AddFont functions will return a nullptr. Please handle those errors in your code (e.g. use an assertion, display an error and quit).
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use FreeType for higher quality font rendering.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	// - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
	//style.FontSizeBase = 20.0f;
	//io.Fonts->AddFontDefaultVector();
	//io.Fonts->AddFontDefaultBitmap();
	//io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
	//IM_ASSERT(font != nullptr);

	// ImFont* font = io.Fonts->AddFontFromFileTTF("unifont-16.0.04.ttf");
	//ImFont* font = io.Fonts->AddFontFromMemoryCompressedTTF(g_unifontTTF_compressed_data, g_unifontTTF_compressed_size);

	u32 fontDataSize;
	char* fontData = LoadEmbeddedFont(&fontDataSize);
	ImFont* font = io.Fonts->AddFontFromMemoryTTF(fontData, (i32)fontDataSize);
	ImGui::PushFont(font, 13);


	// Show window after font loading, as that takes a few ms.
	SDL_ShowWindow(window);


	App app;
	HWND appHwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
	SK_VERIFY(appHwnd);
	app.Init(appHwnd, window, renderer);

	ImVec4 clearColor = {0, 0, 0, 1};

	u64 pfcFrequency = SDL_GetPerformanceFrequency();
	u64 pfcLastTime = SDL_GetPerformanceCounter();

	// sdl main loop
	bool quit = false;
	while (!quit)
	{
		u64 pfcCurrentTime = SDL_GetPerformanceCounter();
		f64 deltaTime = (f64)(pfcCurrentTime - pfcLastTime) / (f64)pfcFrequency;
		pfcLastTime = pfcCurrentTime;

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL3_ProcessEvent(&event);
			if (event.type == SDL_EVENT_QUIT)
				quit = true;
			if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
				quit = true;
		}

		SDL_WindowFlags flags = SDL_GetWindowFlags(window);
		bool isWindowActive = flags & (SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS);
		bool isWindowMinimized = flags & SDL_WINDOW_MINIMIZED;
		if (!isWindowActive || isWindowMinimized)
		{
			SDL_Delay(THROTTLE_WHILE_INACTIVE_MS);
		}

		if (isWindowMinimized)
		{
			// Do not render or update while minimized.
			continue;
		}

		// Start the Dear ImGui frame
		ImGui_ImplSDLRenderer3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

#ifdef DEBUG_SHOW_FPS
		char buffer[255]{};
		sprintf_s(buffer, "WindowEdit 2.0 %f", deltaTime);
		SDL_SetWindowTitle(window, buffer);
#endif

		app.Render();

		// Rendering
		ImGui::Render();
		SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
		SDL_SetRenderDrawColorFloat(renderer, clearColor.x, clearColor.y, clearColor.z, clearColor.w);
		SDL_RenderClear(renderer);
		ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
		SDL_RenderPresent(renderer);
	}

	app.Shutdown();

	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return SDL_APP_SUCCESS;
}
