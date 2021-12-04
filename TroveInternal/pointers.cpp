#include "pch.h"
#include "pointers.h"
#include <tchar.h>
#include "Memory/pattern_batch.h"
#include "Memory/handle.h"
#include "Memory/module.h"

static HWND GetMSWindowHandle() {
    HWND msHandle = nullptr;
    TCHAR buf[256] = { 0 };
    ULONG procid;
    for (HWND hwnd = GetTopWindow(nullptr); hwnd != nullptr; hwnd = GetNextWindow(hwnd, GW_HWNDNEXT)) 
    {
        GetWindowThreadProcessId(hwnd, &procid);
        if (procid != GetCurrentProcessId()) continue;
        if (!GetClassNameW(hwnd, buf, 256)) continue;
        if (wcscmp(buf, L"SDL_app") != 0) continue;
        msHandle = hwnd;
    }
    if (!msHandle) return nullptr;
    return msHandle;
}

pointers::pointers()
{
	memory::pattern_batch main_batch;
    main_batch.add("Infinity jump instruction", "DF F1 DD D8 73 12 56", [&](memory::handle ptr)
    {
        m_infinity_dodge = ptr.add(1).as<void*>();
    });
    main_batch.add("Channel say", "50 E8 ? ? ? ? 8D 4C 24 20 E8 ? ? ? ? 5F 5E 5B 8B E5 5D C2 08 00", [&](memory::handle ptr)
    {
        m_channel_say = ptr.add(2).as<decltype(m_channel_say)>();
    });
    main_batch.add("Add message", "E8 ? ? ? ? FF 74 24 14 33 D2", [&](memory::handle ptr)
    {
        m_add_message = ptr.add(0x62).as<decltype(m_add_message)>();
    });
    /*
     IggyPlayerCallFunctionRS int (__stdcall *IggyPlayerCallFunctionRS)(_DWORD, _DWORD, _DWORD, _DWORD, _DWORD)
      E8 ? ? ? ? 8B 7C 24 08 56 (+0x9F)
    */
    memory::pattern_batch renderer_batch;
    renderer_batch.add("Swapchain present", "FF 15 ? ? ? ? 8B D8 85 FF 74 25 8B 97 ? ? ? ? 85 D2 74 1B 80 7F 53 00 74 15 8B 87 ? ? ? ? 68 ? ? ? ? 52 50 8B 08 FF 91 ? ? ? ? 85 F6 74 29", [&](memory::handle ptr)
    {
        m_present = ptr.add(2).as<decltype(m_present)>();
    });
    renderer_batch.add("Swapchain resizebuffers", "53 FF 15 ? ? ? ? 5F 5E 5B 5D", [&](memory::handle ptr)
    {
        m_resizebuffers = ptr.as<decltype(m_resizebuffers)>();
    });

    main_batch.run(memory::module(nullptr));
    renderer_batch.run(memory::module("gameoverlayrenderer.dll"));

    m_hwnd = GetMSWindowHandle();
    if (!m_hwnd)
        throw std::runtime_error("Failed to find the game's window.");
}

pointers::~pointers()
{
}