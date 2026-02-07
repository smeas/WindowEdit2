#pragma once
#include <Windows.h>
#include "sk/sk.h"

struct IntVec2
{
	i32 x;
	i32 y;
	IntVec2() = default;
	IntVec2(i32 x, i32 y) : x(x), y(y) {}
};

struct Rect : RECT
{
	Rect() = default;
	Rect(RECT other) : RECT(other) {}
	Rect(i32 x, i32 y, i32 w, i32 h) : RECT{x, y, x + w, y + h} {}
	Rect(IntVec2 position, IntVec2 size) : RECT{position.x,position.y, position.x + size.x, position.y + size.y} {}

	i32 X() const { return left; }
	i32 Y() const { return top; }
	i32 Width() const { return right - left; }
	i32 Height() const { return bottom - top; }
	IntVec2 GetPos() const { return {X(), Y()}; }
	IntVec2 GetSize() const { return {Width(), Height()}; }
};

bool ShellOpenFolderAndSelectFile(const wchar_t* filePath);
