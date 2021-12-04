#pragma once
#include "detour_hook.h"
#include "vmt_hook.h"

struct hooks
{
	static void* __stdcall convert_thread_to_fiber(void* param) noexcept;
	static HRESULT __stdcall swapchain_present(IDXGISwapChain* this_, UINT sync_interval, UINT flags) noexcept;
	static HRESULT __stdcall swapchain_resizebuffers(IDXGISwapChain* this_, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swapchain_flags) noexcept;
	static LRESULT __stdcall wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept;
	static BOOL __stdcall set_cursor_pos(int x, int y) noexcept;
	static DWORD* __fastcall channel_say(DWORD *this_, int edx, int a2) noexcept;
};

struct minhook_keepalive
{
	minhook_keepalive();
	~minhook_keepalive();
};

class hooking
{
	friend hooks;
public:
	explicit hooking();
	~hooking();

	void enable();
	void disable();
	std::add_pointer_t<HRESULT __stdcall(IDXGISwapChain* this_, UINT sync_interval, UINT flags)> m_originalPresent;
	std::add_pointer_t<HRESULT __stdcall(IDXGISwapChain* this_, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swapchain_flags)> m_originalResizeBuffers;

private:
	bool m_enabled{};
	minhook_keepalive m_minhook_keepalive{};

	WNDPROC m_og_wndproc;
	
	detour_hook m_set_cursor_pos_hook;
	detour_hook m_convert_thread_to_fiber_hook;
	detour_hook m_channel_send_hook;
};

inline std::unique_ptr<hooking> g_hooking{};