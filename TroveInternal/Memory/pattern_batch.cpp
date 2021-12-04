#include "../pch.h"
#include "pattern_batch.h"
#include "range.h"

namespace memory
{
	void pattern_batch::add(std::string name, pattern pattern, std::function<void(handle)> callback)
	{
		m_entries.emplace_back(std::move(name), std::move(pattern), std::move(callback));
	}

	void pattern_batch::run(range region, bool report_found)
	{
		bool all_found = true;
		for (auto& entry : m_entries)
		{
			if (auto result = region.scan(entry.m_pattern))
			{
				if (entry.m_callback)
				{
					if (report_found)
						LOG(INFO) << "[SigScanner] Found '" << entry.m_name << "'Trove.exe"  << "+" << HEX_TO_UPPER(result.as<uintptr_t>() - region.begin().as<uintptr_t>());
					 
					std::invoke(entry.m_callback, result);
				}
				else
				{
					all_found = false;
					LOG(INFO) << "[SigScanner] Failed to find '" << entry.m_name << "'.";
				}
			}
			else
			{
				all_found = false;
				LOG(INFO) << "[SigScanner] Failed to find '" << entry.m_name << "'.";
			}
		}

		m_entries.clear();
		if (!all_found)
		{
			throw std::runtime_error("[SigScanner] Failed to find some patterns.");
		}
	}
}
