#pragma once
#include <memory>
#include <unordered_map>

#include "sk/sk.h"
#include <Windows.h>
#include <SDL3/SDL_render.h>

class IconTexture
{
	SDL_Texture* m_texture = nullptr;
	f32 m_width = 0;
	f32 m_height = 0;

public:
	IconTexture(SDL_Texture* texture) : m_texture(texture)
	{
		SK_ASSERT(texture);
		SDL_GetTextureSize(texture, &m_width, &m_height);
	}

	~IconTexture()
	{
		SDL_DestroyTexture(m_texture);
	}

	operator bool() const { return m_texture; }

	f32 GetWidth() const { return m_width; }
	f32 GetHeight() const { return m_height; }
	SDL_Texture* GetTexture() const { return m_texture; }
};

using IconTextureRef = std::shared_ptr<IconTexture>;

/*
 * Cache for process icons.
 */
class IconCache
{
	SDL_Renderer* m_renderer = nullptr;
	std::unordered_map<std::wstring, IconTextureRef> m_cache;

public:
	void Init(SDL_Renderer* renderer) { m_renderer = renderer; }
	IconTextureRef GetIconForProcessFileName(const std::wstring& fileName);
};
