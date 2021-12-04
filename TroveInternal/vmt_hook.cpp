#include "pch.h"
#include "vmt_hook.h"

vmt_hook::vmt_hook(void* obj, std::size_t num_funcs) :
	m_object(static_cast<void***>(obj)),
	m_num_funcs(num_funcs + dynamic_cast_length),
	m_original_table(*m_object),
	m_new_table(std::make_unique<void* []>(m_num_funcs))
{
	std::copy_n(m_original_table - dynamic_cast_length, m_num_funcs, m_new_table.get());
}

void vmt_hook::hook(std::size_t index, void* func)
{
	m_new_table[index + dynamic_cast_length] = func;
}

void vmt_hook::unhook(std::size_t index)
{
	m_new_table[index + dynamic_cast_length] = m_original_table[index];
}

void vmt_hook::enable()
{
	*m_object = m_new_table.get() + dynamic_cast_length;
}

void vmt_hook::disable()
{
	*m_object = m_original_table;
}