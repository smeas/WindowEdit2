#pragma once
#include <string>

namespace sk
{
std::string ConvertWStringToUtf8(const wchar_t* input);
void ConvertWStringToUtf8(const wchar_t* input, std::string& output);
}
