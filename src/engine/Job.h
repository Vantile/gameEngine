#pragma once

#include <functional>
#include <mutex>
#include <queue>
#include <xutility>

struct Job
{
	std::function<void()> func;
};

class JobQueue
{
public:
	void Push(Job job)
	{
		{
			std::lock_guard<std::mutex> lock(m_Mutex);
			m_Jobs.push(std::move(job));
		}

		m_CV.notify_one();
	}

	bool Pop(Job& out)
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
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

	void Shutdown()
	{
		{
			std::lock_guard<std::mutex> lock(m_Mutex);
			m_Shutdown = true;
		}

		m_CV.notify_all();
	}

private:
	std::queue<Job> m_Jobs;
	std::mutex m_Mutex;
	std::condition_variable m_CV;
	bool m_Shutdown = false;
};

struct JobCounter
{
	void Increment()
	{
		count.fetch_add(1, std::memory_order_relaxed);
	}

	void Decrement()
	{
		if (count.fetch_sub(1, std::memory_order_release) == 1)
		{
			std::lock_guard<std::mutex> lock(mutex);
			cv.notify_all();
		}
	}

	void Wait()
	{
		std::unique_lock<std::mutex> lock(mutex);
		cv.wait(lock, [&] {
			return count.load(std::memory_order_acquire) == 0;
		});
	}

	bool IsDone() const
	{
		return count.load(std::memory_order_acquire) == 0;
	}

	std::atomic<uint32_t> count{ 0 };
	std::mutex mutex;
	std::condition_variable cv;
};

class JobSystem
{
public:
	void Init(uint32_t threadCount = 0)
	{
		if (threadCount == 0)
		{
			threadCount = std::max(1u, std::thread::hardware_concurrency() - 1);
		}

		for (uint32_t i = 0; i < threadCount; ++i)
		{
			m_Workers.emplace_back([this] { WorkerLoop(); });
		}
	}

	void Submit(Job job, JobCounter* counter)
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

	void Shutdown()
	{
		m_Queue.Shutdown();
		for (std::thread& thread : m_Workers)
		{
			thread.join();
		}
	}

private:
	void WorkerLoop()
	{
		Job job;
		while (m_Queue.Pop(job))
		{
			job.func();
		}
	}

private:
	std::vector<std::thread> m_Workers;
	JobQueue m_Queue;
};