#include "ThreadPool.h"

using namespace Apex::Core;

ThreadPool::ThreadPool(int nbThreads)
{
	nbThreads = std::max(nbThreads, 1);

	for (int i = 0; i < nbThreads; i++)
	{
		m_threads.emplace_back(&ThreadPool::WorkerLoop, this);
	}
}

ThreadPool::~ThreadPool()
{
	m_stop = true;
	m_cv.notify_all();

	for (std::thread& thread : m_threads)
	{
		thread.join();
	}
}

ThreadPool& ThreadPool::Get()
{
	static ThreadPool instance;
	return instance;
}

void ThreadPool::WorkerLoop()
{
	while (!m_stop)
	{
		std::function<void()> task;
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_cv.wait(lock, [this] 
			{
				return m_stop || !m_queue.empty();
			});
			if (m_stop) return;

			task = m_queue.front();
			m_queue.pop();
		}
		task();
	}
}
