#include "pch.h"
#include <MinHook.h>
#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

#include "hooking.h"
#include "pointers.h"
#include "renderer.h"
#include "gui.h"
#include "Memory/module.h"
#include "Memory/handle.h"

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

hooking::hooking() :
	m_set_cursor_pos_hook("SetCursorPos", memory::module("user32.dll").get_export("SetCursorPos").as<void*>(), &hooks::set_cursor_pos),
	m_convert_thread_to_fiber_hook("ConvertThreadToFiber", memory::module("kernel32.dll").get_export("ConvertThreadToFiber").as<void*>(), &hooks::convert_thread_to_fiber),
	m_channel_send_hook("channelSend", g_pointers->m_channel_say, &hooks::channel_say)
{
	m_originalPresent = **reinterpret_cast<decltype(m_originalPresent)**>(g_pointers->m_present);
	m_originalResizeBuffers = **reinterpret_cast<decltype(m_originalResizeBuffers)**>(g_pointers->m_resizebuffers);

	m_og_wndproc = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(g_pointers->m_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&hooks::wndproc)));
}

hooking::~hooking()
{
	if (m_enabled)
		disable();
}

void hooking::enable()
{
	**reinterpret_cast<decltype(hooks::swapchain_present)***>(g_pointers->m_present) = hooks::swapchain_present;
	LOG(INFO_TO_FILE) << "Enabled hook 'Present'.";
	**reinterpret_cast<decltype(hooks::swapchain_resizebuffers)***>(g_pointers->m_resizebuffers) = hooks::swapchain_resizebuffers;
	LOG(INFO_TO_FILE) << "Enabled hook 'Resizebuffers'.";

	m_set_cursor_pos_hook.enable();
	m_convert_thread_to_fiber_hook.enable();

	m_channel_send_hook.enable();

	m_enabled = true;
}

void hooking::disable()
{
	m_enabled = false;

	m_convert_thread_to_fiber_hook.disable();
	m_channel_send_hook.disable();
	m_set_cursor_pos_hook.disable();

	**reinterpret_cast<void***>(g_pointers->m_present) = m_originalPresent;
	**reinterpret_cast<void***>(g_pointers->m_resizebuffers) = m_originalResizeBuffers;

	SetWindowLongPtrW(g_pointers->m_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_og_wndproc));
}

minhook_keepalive::minhook_keepalive()
{
	MH_Initialize();
}

minhook_keepalive::~minhook_keepalive()
{
	MH_Uninitialize();
}

HRESULT __stdcall hooks::swapchain_present(IDXGISwapChain* this_, UINT sync_interval, UINT flags) noexcept
{
	[[maybe_unused]] static const auto imguiInit = [](IDXGISwapChain* swapchain) noexcept 
	{
		comptr<IDXGISwapChain> m_dxgi_swapchain = swapchain;
		comptr<ID3D11DeviceContext> m_d3d_device_context;
		comptr<ID3D11Device> m_dx_device;
		void* d3d_device{};

		m_dxgi_swapchain->GetDevice(__uuidof(ID3D11Device), &d3d_device);
		m_dx_device.Attach(static_cast<ID3D11Device*>(d3d_device));
		m_dx_device->GetImmediateContext(m_d3d_device_context.GetAddressOf());

		ImGui_ImplDX11_Init(m_dx_device.Get(), m_d3d_device_context.Get());
		return true;
	}(this_);

	if (g_running)
	{
		g_renderer->on_present();
	}

	return g_hooking->m_originalPresent(this_, sync_interval, flags);
}


HRESULT __stdcall hooks::swapchain_resizebuffers(IDXGISwapChain* this_, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swapchain_flags) noexcept
{
	if (g_running) 
	{
		g_renderer->pre_reset();

		auto result = g_hooking->m_originalResizeBuffers(this_, buffer_count, width, height, new_format, swapchain_flags);
		if (SUCCEEDED(result))
		{
			g_renderer->post_reset();
		}

		return result;
	}

	return g_hooking->m_originalResizeBuffers(this_, buffer_count, width, height, new_format, swapchain_flags);
}

void* __stdcall hooks::convert_thread_to_fiber(void* param) noexcept
{
	if (g_running)
	{
		if (IsThreadAFiber())
		{
			return GetCurrentFiber();
		}
	}

	return g_hooking->m_convert_thread_to_fiber_hook.get_original<decltype(&convert_thread_to_fiber)>()(param);
}


LRESULT __stdcall hooks::wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept
{
	[[maybe_unused]] static const auto imguiInit = [](HWND window) noexcept 
	{
		ImGui::CreateContext();
		ImGui_ImplWin32_Init(window);

		g_renderer = std::make_unique<renderer>();
		LOG(INFO) << "Renderer initialized.";

		g_hooking->enable();
		LOG(INFO) << "Hooking enabled.";

		return true;
	}(hwnd);

	if (g_running)
	{
		g_renderer->wndproc(hwnd, msg, wparam, lparam);
	}
	return CallWindowProcW(g_hooking->m_og_wndproc, hwnd, msg, wparam, lparam);
}

BOOL __stdcall hooks::set_cursor_pos(int x, int y) noexcept
{
	if (g_running)
	{
		if (g_gui.m_opened)
			return true;
	}

	return g_hooking->m_set_cursor_pos_hook.get_original<decltype(&set_cursor_pos)>()(x, y);
}

DWORD* __fastcall hooks::channel_say(DWORD* this_, int edx, int a2) noexcept
{
	if (g_running)
	{
		LOG(INFO) << this_ << " ## " << a2;
		if (this_)
			LOG(INFO) << *this_;
	}

	return g_hooking->m_channel_send_hook.get_original<decltype(g_pointers->m_channel_say)>()(this_, a2);
}