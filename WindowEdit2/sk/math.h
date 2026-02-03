#pragma once

#define SK_E_D        2.71828182845904523536   // e
#define SK_PI_D       3.14159265358979323846   // pi
#define SK_PI_2_D     1.57079632679489661923   // pi/2
#define SK_SQRT2_D    1.41421356237309504880   // sqrt(2)
#define SK_TWOPI_D (2.0 * SK_PI)
#define SK_TAU_D SK_TWOPI
#define SK_DEG2RAD_D (SK_PI / 180.0)
#define SK_RAD2DEG_D (180.0 / SK_PI)

#define SK_E        2.71828182845904523536f   // e
#define SK_PI       3.14159265358979323846f   // pi
#define SK_PI_2     1.57079632679489661923f   // pi/2
#define SK_SQRT2    1.41421356237309504880f   // sqrt(2)
#define SK_TWOPI ((f32)(2.0 * SK_PI_D))
#define SK_TAU SK_TWOPI
#define SK_DEG2RAD ((f32)(SK_PI_D / 180.0))
#define SK_RAD2DEG ((f32)(180.0 / SK_PI_D))

namespace sk
{
template <typename T>
T Clamp(T value, T low, T high)
{
	if (value < low) return low;
	if (value > high) return high;
	return value;
}
}
