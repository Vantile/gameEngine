#include <engine/Job.h>

#include <string>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void SetThreadAffinity(std::thread& thread, uint32_t coreIndex)
{
	DWORD_PTR mask = 1ull << coreIndex;
	HANDLE handle = static_cast<HANDLE>(thread.native_handle());
	SetThreadAffinityMask(handle, mask);
}

void SetThreadName(HANDLE thread, const wchar_t* name)
{
	SetThreadDescription(thread, name);
}

void SetThreadName(std::thread& thread, const wchar_t* name)
{
	SetThreadDescription(static_cast<HANDLE>(thread.native_handle()), name);
}

void JobQueue::Push(Job job)
{
	{
		std::lock_guard<LockableBase(std::mutex)> lock(m_Mutex);
		LockMark(m_Mutex);
		m_Jobs.push(std::move(job));
	}

	m_CV.notify_one();
}

bool JobQueue::Pop(Job& out)
{
	std::unique_lock<LockableBase(std::mutex)> lock(m_Mutex);
	LockMark(m_Mutex);
	m_CV.wait(lock, [&] {
		return !m_Jobs.empty() || m_Shutdown;
		});

	if (m_Shutdown && m_Jobs.empty())
	{
		return false;
	}

	out = std::move(m_Jobs.front());
	m_Jobs.pop();
	return true;
}

void JobQueue::Shutdown()
{
	{
		std::lock_guard<LockableBase(std::mutex)> lock(m_Mutex);
		LockMark(m_Mutex);
		m_Shutdown = true;
	}

	m_CV.notify_all();
}

void JobCounter::Increment()
{
	count.fetch_add(1, std::memory_order_relaxed);
}

void JobCounter::Decrement()
{
	if (count.fetch_sub(1, std::memory_order_release) == 1)
	{
		std::lock_guard<LockableBase(std::mutex)> lock(mutex);
		LockMark(mutex);
		cv.notify_all();
	}
}

void JobCounter::Wait()
{
	std::unique_lock<LockableBase(std::mutex)> lock(mutex);
	LockMark(mutex);
	cv.wait(lock, [&] {
		return count.load(std::memory_order_acquire) == 0;
		});
}

bool JobCounter::IsDone() const
{
	return count.load(std::memory_order_acquire) == 0;
}

void JobSystem::Init(uint32_t threadCount)
{
	SetThreadAffinityMask(GetCurrentThread(), 1ull << 0);
	SetThreadName(GetCurrentThread(), L"Main Thread");
	if (threadCount == 0)
	{
		threadCount = std::max(1u, std::thread::hardware_concurrency() - 1);
	}

	for (uint32_t i = 0; i < threadCount; ++i)
	{
		m_Workers.emplace_back([this] { WorkerLoop(); });
		SetThreadAffinity(m_Workers[i], i + 1);
		SetThreadName(m_Workers[i], (L"Worker Thread " + std::to_wstring(i)).c_str());
	}
}

void JobSystem::Submit(Job job, JobCounter* counter)
{
	if (counter != nullptr)
	{
		counter->Increment();
	}

	Job wrapped;
	wrapped.func = [job, counter]() {
		job.func();
		if (counter != nullptr)
		{
			counter->Decrement();
		}
		};

	m_Queue.Push(std::move(wrapped));
}

void JobSystem::Shutdown()
{
	m_Queue.Shutdown();
	for (std::thread& thread : m_Workers)
	{
		thread.join();
	}
}

void JobSystem::WorkerLoop()
{
	Job job;
	while (m_Queue.Pop(job))
	{
		job.func();
	}
}