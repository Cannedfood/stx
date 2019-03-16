#pragma once

#include <atomic>


struct counted {
	std::atomic<int>& m_count;

	counted(std::atomic<int>* count) noexcept : m_count(*count) { ++m_count; }
	~counted() noexcept { --m_count; }
};