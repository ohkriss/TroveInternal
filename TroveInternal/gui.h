#pragma once

class gui
{
public:
	void dx_init();
	void dx_on_tick();
	void render_main_tab();
public:
	bool m_opened{};
};

inline gui g_gui;