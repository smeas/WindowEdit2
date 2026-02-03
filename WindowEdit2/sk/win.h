#pragma once
#include <string>
#include <stdexcept>
#include "core.h"

namespace sk
{
// As defined in winnt.h
typedef long HRESULT;

inline std::string HrToString(HRESULT hr)
{
	char str[64];
	(void)sprintf_s(str, "HRESULT of 0x%08X", (u32)hr);
	return std::string(str);
}

inline bool HrSucceeded(HRESULT hr)
{
	// As defined in winerror.h
	return hr >= 0;
}

inline bool HrFailed(HRESULT hr)
{
	// As defined in winerror.h
	return hr < 0;
}

class HrException : public std::runtime_error
{
	HRESULT m_hresult;

public:
	HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hresult(hr)
	{
	}

	HRESULT Result() const { return m_hresult; }
};

inline void ThrowIfFailed(HRESULT hr)
{
	if (HrFailed(hr))
	{
		throw HrException(hr);
	}
}
};