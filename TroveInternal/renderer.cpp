#include "pch.h"

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_dx11.h>

#include "gui.h"
#include "pointers.h"
#include "renderer.h"

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

renderer::renderer()
{
	auto file_path = std::filesystem::path(std::getenv("appdata"));
	file_path /= "TroveInternal";
	if (!std::filesystem::exists(file_path))
	{
		std::filesystem::create_directory(file_path);
	}
	else if (!std::filesystem::is_directory(file_path))
	{
		std::filesystem::remove(file_path);
		std::filesystem::create_directory(file_path);
	}
	file_path /= "imgui.ini";

	ImGuiContext* ctx = ImGui::CreateContext();

	static std::string path = file_path.make_preferred().string();
	ctx->IO.IniFilename = path.c_str();

	m_monospace_font = ImGui::GetIO().Fonts->AddFontDefault();

	g_gui.dx_init();
}

renderer::~renderer()
{
	ImGui_ImplWin32_Shutdown();
	ImGui_ImplDX11_Shutdown();
	ImGui::DestroyContext();

	g_renderer = nullptr;
}

void renderer::on_present()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (g_gui.m_opened)
	{
		ImGui::GetIO().MouseDrawCursor = true;
		ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;

	}
	else
	{
		ImGui::GetIO().MouseDrawCursor = false;
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
	}

	if (const auto& displaySize = ImGui::GetIO().DisplaySize; displaySize.x > 0.0f && displaySize.y > 0.0f) 
	{
		if (g_gui.m_opened)
		{
			g_gui.dx_on_tick();
		}
	}

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void renderer::pre_reset()
{
	ImGui_ImplDX11_InvalidateDeviceObjects();
}

void renderer::post_reset()
{
	ImGui_ImplDX11_CreateDeviceObjects();
}

void renderer::wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_KEYUP && wparam == VK_INSERT)
	{
		static POINT cursor_coords{};
		if (g_gui.m_opened)
		{
			GetCursorPos(&cursor_coords);
		}
		else if (cursor_coords.x + cursor_coords.y != 0)
		{
			SetCursorPos(cursor_coords.x, cursor_coords.y);
		}

		g_gui.m_opened ^= true;
	}

	ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
}