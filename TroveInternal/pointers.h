#pragma once
#include "pch.h"

class pointers
{
public:
	explicit pointers();
	~pointers();
public:
	HWND m_hwnd{};
	using present_t = HRESULT(__stdcall*)(IDXGISwapChain* this_, UINT sync_interval, UINT flags);
	present_t m_present{};
	using resizebuffers_t = HRESULT(__stdcall*)(IDXGISwapChain* this_, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swapchain_flags);
	resizebuffers_t m_resizebuffers{};

	void* m_infinity_dodge{};
	using channel_say_t = DWORD*(__thiscall*)(DWORD* this_, int a2);
	channel_say_t m_channel_say{};

	using add_message_t = void(__thiscall*)(LPCRITICAL_SECTION* this_, const char* a2, int a3, DWORD* a4);
	add_message_t m_add_message{};
};

inline std::unique_ptr<pointers> g_pointers{};
