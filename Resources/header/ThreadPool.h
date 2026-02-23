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
		ThreadPool(int nbThreads = std::thread::hardware_concurrency());
		~ThreadPool();

		template <class Func, typename... Args>
		void Enqueue(Func&& func, Args&&... args)
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_queue.emplace(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
		}

		template <class Func, typename Obj, typename... Args>
		void Enqueue(Func&& func, Obj* obj, Args&&... args)
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_queue.emplace(std::bind(std::forward<Func>(func), obj, std::forward<Args>(args)...));
		}


	private:
		void WorkerLoop();

		std::vector<std::thread> m_threads;
		std::queue<std::function<void()>> m_queue;

		std::condition_variable m_cv;
		std::mutex m_mutex;
		std::atomic<bool> m_stop = false;
	};
}

#endif // !THREAD_POOL