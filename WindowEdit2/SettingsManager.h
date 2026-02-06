#pragma once
#include <optional>
#include <string>
#include <vector>

#include "common.h"

struct GlobalProfile
{
	std::string Name;
	IntVec2 Position;
	IntVec2 Size;
};

class SettingsManager
{
	std::vector<GlobalProfile> m_globalProfiles;

public:
	bool m_showAllWindows;
	IntVec2 m_windowSize;

	SettingsManager() { Reset(); }

	void Reset()
	{
		m_globalProfiles.clear();
		m_showAllWindows = false;
		m_windowSize = IntVec2(800, 500);
	}

	void Init();
	void Load();
	void Save();

	std::vector<GlobalProfile>& GetProfiles() { return m_globalProfiles; }

	const GlobalProfile* FindProfile(std::string_view name) const
	{
		for (auto& profile : m_globalProfiles)
		{
			if (profile.Name == name)
				return &profile;
		}

		return nullptr;
	}

	i32 FindProfileIndex(std::string_view name) const
	{
		for (u32 i = 0; i < m_globalProfiles.size(); ++i)
		{
			if (m_globalProfiles[i].Name == name)
				return i;
		}

		return -1;
	}

	void DeleteProfile(std::string_view name)
	{
		i32 index = FindProfileIndex(name);
		if (index == -1)
			return;

		m_globalProfiles.erase(m_globalProfiles.begin() + index);
	}
};
