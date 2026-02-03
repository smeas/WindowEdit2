#include "core.h"
#include <corecrt_wstdio.h>
#include <cstdlib>
#include <Windows.h>

SK_INTERNAL void TriggerAssertFail(wchar_t const* file, u32 line, wchar_t const* message) {
	wchar_t buffer[512];
	_snwprintf_s(buffer, _countof(buffer),
				 L"Assertion failed!\n"
				 L"File: %s\n"
				 L"Line: %u\n"
				 L"\n"
				 L"Message: %s", file, line, message);

	int result = MessageBoxW(nullptr, buffer, L"Fatal error", MB_ICONERROR | MB_ABORTRETRYIGNORE | MB_TASKMODAL);
	switch (result)
	{
	case IDRETRY:
		__debugbreak();
		break;
	case IDIGNORE:
		break;
	case IDABORT:
	default:
		std::abort();
		break;
	}
}

SK_NOINLINE void sk::AssertFailW(wchar_t const* file, u32 line, wchar_t const* message)
{
	// #ifdef _DEBUG
	// 	_wassert(message, file, line);
	// #endif
	// 	// Retry does not trap the debugger?
	// 	//_CrtDbgReport(_CRT_ASSERT, file, (i32)line, nullptr, message);

	TriggerAssertFail(file, line, message);
}

SK_NOINLINE void sk::AssertFailFW(wchar_t const* file, u32 line, wchar_t const* format, ...)
{
	va_list args;
	va_start(args, format);

	wchar_t buffer[256];
	_vsnwprintf_s(buffer, _countof(buffer), format, args);
	TriggerAssertFail(file, line, buffer);

	va_end(args);
}

// SK_NOINLINE void sk::VerifyFailW(wchar_t const* file, u32 line, wchar_t const* message)
// {
// 	TriggerAssertFail(message, file, line);
// }
//
// SK_NOINLINE void sk::VerifyFailFW(wchar_t const* file, u32 line, wchar_t const* format, ...)
// {
// 	va_list args;
// 	va_start(args, format);
//
// 	wchar_t buffer[256];
// 	_vsnwprintf_s(buffer, _countof(buffer), format, args);
// 	TriggerAssertFail(buffer, file, line);
//
// 	va_end(args);
// }
