#pragma once
#include "pch.h"
#include <imgui.h>


class renderer
{
public:
	explicit renderer();
	~renderer();

	void on_present();

	void pre_reset();
	void post_reset();

	void wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
public:
	ImFont* m_font;
	ImFont* m_monospace_font;
private:
};

inline  std::unique_ptr<renderer> g_renderer{};