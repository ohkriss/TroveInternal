#include "pch.h"
#include "pointers.h"
#include "renderer.h"
#include "hooking.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) 
    {
		DisableThreadLibraryCalls(hModule);

		g_hmodule = hModule;
		g_main_thread = CreateThread(nullptr, 0, [](PVOID) -> DWORD
			{
				auto logger_instance = std::make_unique<logger>();
				try
				{
					g_pointers = std::make_unique<pointers>();
					LOG(INFO) << "Pointers initialized.";

					g_hooking = std::make_unique<hooking>();
					LOG(INFO) << "Hooking initialized.";

					while (g_running)
					{
						if (GetAsyncKeyState(VK_END) & 0x8000)
							g_running = false;

						std::this_thread::sleep_for(10ms);
					}

					g_hooking->disable();
					LOG(INFO) << "Hooking disabled.";

					std::this_thread::sleep_for(1s);

					g_hooking.reset();
					LOG(INFO) << "Hooking uninitialized.";

					g_renderer.reset();
					LOG(INFO) << "Renderer uninitialized.";

					g_pointers.reset();
					LOG(INFO) << "Pointers uninitialized.";
				}
				catch (std::exception const& ex)
				{
					LOG(WARNING) << ex.what();
					MessageBoxA(nullptr, ex.what(), nullptr, MB_OK | MB_ICONEXCLAMATION);
				}

				logger_instance.reset();

				CloseHandle(g_main_thread);
				FreeLibraryAndExitThread(g_hmodule, 0);
			}, nullptr, 0, &g_main_thread_id);
    }
    
    return TRUE;
}

