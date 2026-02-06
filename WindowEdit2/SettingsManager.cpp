#include "SettingsManager.h"
#include <fstream>

#include "vendor/json.hpp"

void to_json(nlohmann::json& json, const GlobalProfile& profile)
{
	json = {
		{"name", profile.Name},
		{"x", profile.Position.x},
		{"y", profile.Position.y},
		{"w", profile.Size.x},
		{"h", profile.Size.y},
	};
}

void from_json(const nlohmann::json& json, GlobalProfile& profile)
{
	json.at("name").get_to(profile.Name);
	json.at("x").get_to(profile.Position.x);
	json.at("y").get_to(profile.Position.y);
	json.at("w").get_to(profile.Size.x);
	json.at("h").get_to(profile.Size.y);
}

void SettingsManager::Init() {}

void SettingsManager::Load()
{
	m_globalProfiles.clear();

	std::ifstream file("WindowEdit2.json");
	if (!file)
	{
		return;
	}

	try
	{
		auto data = nlohmann::json::parse(file);

		data["windowSize"]["x"].get_to(m_windowSize.x);
		data["windowSize"]["y"].get_to(m_windowSize.y);
		data["showAllWindows"].get_to(m_showAllWindows);

		for (auto& item : data["profiles"])
		{
			m_globalProfiles.push_back(item.get<GlobalProfile>());
		}
	}
	catch (std::exception&)
	{
		// Failed to read config. Reset all settings.
		Reset();
	}
}

void SettingsManager::Save()
{
	nlohmann::json data;

	data["windowSize"]["x"] = m_windowSize.x;
	data["windowSize"]["y"] = m_windowSize.y;
	data["showAllWindows"] = m_showAllWindows;

	for (GlobalProfile& profile : m_globalProfiles)
	{
		data["profiles"].push_back(profile);
	}

	std::ofstream file("WindowEdit2.json");
	file << data;
}
