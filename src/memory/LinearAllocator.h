#pragma once

#include <cstdint>
#include <memory/Memory.h>

class LinearAllocator
{
public:
	LinearAllocator(size_t capacity)
	{
		m_Buffer = static_cast<uint8_t*>(malloc(capacity));
		m_Capacity = capacity;
	}

	~LinearAllocator() { free(m_Buffer); }

	void* Alloc(size_t size, size_t alignment = 16)
	{
		size_t aligned = (m_Offset + alignment - 1) & ~(alignment - 1);

		assert(aligned + size <= m_Capacity && "Linear allocator out of memory");

		void* ptr = m_Buffer + aligned;
		m_Offset = aligned + size;
		return ptr;
	}

	void Reset() { m_Offset = 0; }

	size_t Used() const { return m_Offset; }
	size_t Remaining() const { return m_Capacity - m_Offset; }

private:
	uint8_t* m_Buffer = nullptr;
	size_t m_Capacity = 0;
	size_t m_Offset = 0;
};