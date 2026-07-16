#pragma once

#include <functional>
#include <mutex>
#include <queue>
#include <tracy/Tracy.hpp>

struct Job
{
	std::function<void()> func;
};

class JobQueue
{
public:
	void Push(Job job);
	bool Pop(Job& out);
	void Shutdown();

private:
	std::queue<Job> m_Jobs;
	TracyLockable(std::mutex, m_Mutex);
	std::condition_variable_any m_CV;
	bool m_Shutdown = false;
};

struct JobCounter
{
	void Increment();
	void Decrement();
	void Wait();
	bool IsDone() const;

	std::atomic<uint32_t> count{ 0 };
	TracyLockable(std::mutex, mutex);
	std::condition_variable_any cv;
};

class JobSystem
{
public:
	void Init(uint32_t threadCount = 0);
	void Submit(Job job, JobCounter* counter);
	void Shutdown();

private:
	void WorkerLoop();

private:
	std::vector<std::thread> m_Workers;
	JobQueue m_Queue;
};