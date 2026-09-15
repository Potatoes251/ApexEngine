#ifndef THREAD_POOL
#define THREAD_POOL

#include <queue>
#include <thread>
#include <mutex>
#include <functional>

namespace Apex::Core
{
	class ThreadPool
	{
	public:
		ThreadPool(ThreadPool const&) = delete;
		ThreadPool& operator=(ThreadPool const&) = delete;

		~ThreadPool();

		static ThreadPool& Get();

		template <class Func, typename... Args>
		void Enqueue(Func&& func, Args&&... args)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_queue.emplace(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
			m_cv.notify_one();
		}


	private:
		ThreadPool(int nbThreads = std::thread::hardware_concurrency() - 1);

		void WorkerLoop();

		std::vector<std::thread> m_threads;
		std::queue<std::function<void()>> m_queue;

		std::condition_variable m_cv;
		std::mutex m_mutex;
		std::atomic<bool> m_stop = false;
	};
}

#endif // !THREAD_POOL