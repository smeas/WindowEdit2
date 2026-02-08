#pragma once
#include <Windows.h>

#include "sk/core.h"

class ProcessHandle
{
	HANDLE m_handle = NULL; // 0 is not a valid process handle, see OpenProcess return value.

public:
	ProcessHandle() = default;

	ProcessHandle(HANDLE handle) : m_handle(handle) {}

	ProcessHandle(const ProcessHandle& other)
	{
		if (other.m_handle != NULL)
		{
			auto currentProcess = GetCurrentProcess();
			bool ok = DuplicateHandle(
				currentProcess, other.m_handle,
				currentProcess, &m_handle,
				0, false, DUPLICATE_SAME_ACCESS);
			SK_VERIFYF(ok, "duplicate handle failed");
		}
		else
		{
			m_handle = NULL;
		}
	}

	ProcessHandle(ProcessHandle&& other) noexcept
	{
		m_handle = other.m_handle;
		other.m_handle = NULL;
	}

	~ProcessHandle()
	{
		if (m_handle == NULL)
			return;

		CloseHandle(m_handle);
		m_handle = NULL;
	}

	ProcessHandle& operator=(const ProcessHandle& other)
	{
		ProcessHandle(other).Swap(*this);
		return *this;
	}

	ProcessHandle& operator=(ProcessHandle&& other) noexcept
	{
		ProcessHandle(std::move(other)).Swap(*this);
		return *this;
	}

	void Swap(ProcessHandle& other)
	{
		std::swap(m_handle, other.m_handle);
	}

	void Reset()
	{
		ProcessHandle().Swap(*this);
	}

	HANDLE Get() const { return m_handle; }

	// HANDLE* operator&()
	// {
	// 	Reset();
	// 	return &m_handle;
	// }

	bool IsValid() const
	{
		return m_handle != NULL;
	}

	bool IsProcessAlive() const
	{
		if (m_handle == NULL)
			return false;

		DWORD result = WaitForSingleObject(m_handle, 0);
		// If the process does not exist anymore, the handle will be signaled.
		if (result == WAIT_OBJECT_0)
			return false;

		SK_VERIFYF(result != WAIT_FAILED, "process requires the SYNCHRONIZE access flag");

		return true;
	}
};
