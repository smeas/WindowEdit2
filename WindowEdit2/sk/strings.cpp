#include "strings.h"
#include "core.h"
#include <Windows.h>

std::string sk::ConvertWStringToUtf8(const wchar_t* input)
{
	std::string result;
	ConvertWStringToUtf8(input, result);
	return result;
}

void sk::ConvertWStringToUtf8(const wchar_t* input, std::string& output)
{
	i32 inputLength = (i32)wcslen(input);
	i32 sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, input, inputLength, NULL, 0, NULL, NULL);
	output.resize(sizeNeeded);
	if (sizeNeeded == 0)
		return;

	// NOTE: WideCharToMultiByte does not write a null terminator if the input string lenght is explicitly specified.
	// https://learn.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-widechartomultibyte#remarks
	i32 realSize = WideCharToMultiByte(CP_UTF8, 0, input, inputLength, output.data(), (i32)output.size(),  NULL, NULL);
	SK_ASSERT(realSize == sizeNeeded);
}


