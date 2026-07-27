#pragma once

#include <mutex>
#include <tracy/Tracy.hpp>
#include <unordered_map>

//struct AllocationHeader
//{
//	size_t size;
//	const char* category;
//	const char* file;
//	uint32_t line;
//};
//
//struct MemoryTracker
//{
//	struct Stats
//	{
//		size_t totalAllocated = 0;
//		size_t totalFreed = 0;
//		uint32_t allocationCount = 0;
//
//		size_t Live() const { return totalAllocated - totalFreed; }
//	};
//
//	std::unordered_map<const char*, Stats> categoryStats;
//	std::mutex mutex;
//
//	void TrackAlloc(size_t size, const char* category)
//	{
//		std::lock_guard lock(mutex);
//		Stats& stat = categoryStats[category];
//		stat.totalAllocated += size;
//		++stat.allocationCount;
//	}
//
//	void TrackFree(size_t size, const char* category)
//	{
//		std::lock_guard lock(mutex);
//		Stats& stat = categoryStats[category];
//		stat.totalFreed += size;
//	}
//
//	void PrintStats()
//	{
//		for (auto& [category, stat] : categoryStats)
//		{
//			std::printf("[%s] Live: Bytes: %zu, Allocations: %u\n", category, stat.Live(), stat.allocationCount);
//		}
//	}
//};
//
//inline MemoryTracker g_MemoryTracker;

#define engineNew(T, ...)\
	[&]()\
	{\
		T* ptr = new T(__VA_ARGS__);\
		TracyAlloc(ptr, sizeof(T));\
		return ptr;\
	}()

#define engineDelete(ptr)\
	[&]()\
	{\
		TracyFree(ptr);\
		delete ptr;\
	}()

#define engineAlloc(allocator, T, ...)\
	[&]()\
	{\
		void* mem = (allocator).Alloc(sizeof(T), alignof(T));\
		T* ptr = new(mem) T(__VA_ARGS__);\
		TracyAllocN(ptr, sizeof(T), #T);\
		return ptr;\
	}()

#define engineFree(allocator, T, ptr)\
	[&]()\
	{\
		TracyFreeN(ptr, #T);\
		(ptr)->~T();\
		(allocator).Free(ptr);\
	}()