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
	i32 X() const { return left; }
	i32 Y() const { return top; }
	i32 Width() const { return right - left; }
	i32 Height() const { return bottom - top; }
	IntVec2 GetPos() const { return {X(), Y()}; }
	IntVec2 GetSize() const { return {Width(), Height()}; }
};
