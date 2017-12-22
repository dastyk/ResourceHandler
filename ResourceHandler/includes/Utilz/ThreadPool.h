//Courtesy of: https://github.com/progschj/ThreadPool
#ifndef SE_UTILZ_THREADPOOL_H
#define SE_UTILZ_THREADPOOL_H
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

namespace Utilz
{

	class ThreadPool {
	public:
		ThreadPool(size_t);

		/*
		 *@brief Adds a job to the queue. A thread will be assigned to the job as soon as a free thread is available.
		 *@param[in] f The function the thread will call.
		 *@param[in] args The arguments that will be passed to the function.
		 *@retval Returns a future of the return type of f. The future is moveconstructable but not copyconstructable.
		 *@warning The function f must have a finite running time. No while(true) or similar is allowed.
		 *@code
		 *ThreadPool tp(4);
		 *auto result = tp.Enqueue([](int a){return a + 5;}, 10);
		 *printf("%d", result.get()); //Prints 15 once a thread has finished the task.
		 *@endcode
		 */
		template<class F, class... Args>
		auto Enqueue(F&& f, Args&&... args)
			->std::future<typename std::result_of<F(Args...)>::type>;

		/*
		*@brief Adds a job to the queue. A thread will be assigned to the job as soon as a free thread is available.
		*@param[in] instance The instance of the class
		*@param[in] TM The method pointer
		*@param[in] args The arguments that will be passed to the function.
		*@retval Returns a future of the return type of f. The future is moveconstructable but not copyconstructable.
		*@warning The function f must have a finite running time. No while(true) or similar is allowed.
		*@code
		*ThreadPool tp(4);
		* X x;
		*auto result = tp.Enqueue(this, &X::foo, 10);
		*printf("%d", result.get()); //Prints 15 once a thread has finished the task.
		*@endcode
		*/
		template<class RET, class T, class... Args>
		auto Enqueue(T* instance, RET(T::*TM)(Args...), Args&&... args)->std::future<RET>;

		/*
		*@brief Adds a job to the queue. A thread will be assigned to the job as soon as a free thread is available.
		*@param[in] instance The instance of the class
		*@param[in] TM The method pointer
		*@param[in] args The arguments that will be passed to the function.
		*@retval Returns a future of the return type of f. The future is moveconstructable but not copyconstructable.
		*@warning The function f must have a finite running time. No while(true) or similar is allowed.
		*@code
		*ThreadPool tp(4);
		* const X x;
		*auto result = tp.Enqueue(this, &X::foo, 10);
		*printf("%d", result.get()); //Prints 15 once a thread has finished the task.
		*@endcode
		*/
		template<class RET, class T, class... Args>
		auto Enqueue(const T* instance, RET(T::*TM)(Args...)const, Args&&... args)->std::future<RET>;
		~ThreadPool();
	private:
		// need to keep track of threads so we can join them
		std::vector< std::thread > workers;
		// the task queue
		std::queue< std::function<void()> > tasks;

		// synchronization
		std::mutex queue_mutex;
		std::condition_variable condition;
		bool stop;
	};

	// the constructor just launches some amount of workers
	inline ThreadPool::ThreadPool(size_t threads)
		: stop(false)
	{
		for (size_t i = 0; i < threads; ++i)
			workers.emplace_back(
				[this]
		{
			for (;;)
			{
				std::function<void()> task;

				{
					std::unique_lock<std::mutex> lock(this->queue_mutex);
					this->condition.wait(lock,
						[this] { return this->stop || !this->tasks.empty(); });
					if (this->stop && this->tasks.empty())
						return;
					task = std::move(this->tasks.front());
					this->tasks.pop();
				}

				task();
			}
		}
		);
	}

	// add new work item to the pool
	template<class F, class... Args>
	auto ThreadPool::Enqueue(F&& f, Args&&... args)
		-> std::future<typename std::result_of<F(Args...)>::type>
	{
		using return_type = typename std::result_of<F(Args...)>::type;

		auto task = std::make_shared< std::packaged_task<return_type()> >(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
			);

		std::future<return_type> res = task->get_future();
		{
			std::unique_lock<std::mutex> lock(queue_mutex);

			// don't allow enqueueing after stopping the pool
			if (stop)
				throw std::runtime_error("enqueue on stopped ThreadPool");

			tasks.emplace([task]() { (*task)(); });
		}
		condition.notify_one();
		return res;
	}

	// add new work item to the pool
	template<class RET, class T, class... Args>
	auto ThreadPool::Enqueue(T* instance, RET(T::*TM)(Args...), Args&&... args) -> std::future<RET>
	{
		auto task = std::make_shared< std::packaged_task<RET()> >(
			std::bind(TM, instance, std::forward<Args>(args)...)
			);

		auto res = task->get_future();
		{
			std::unique_lock<std::mutex> lock(queue_mutex);

			// don't allow enqueueing after stopping the pool
			if (stop)
				throw std::runtime_error("enqueue on stopped ThreadPool");

			tasks.emplace([task]() { (*task)(); });
		}
		condition.notify_one();
		return res;
	}

	// add new work item to the pool
	template<class RET, class T, class... Args>
	auto ThreadPool::Enqueue(const T* instance, RET(T::*TM)(Args...)const, Args&&... args) -> std::future<RET>
	{
		auto task = std::make_shared< std::packaged_task<RET()> >(
			std::bind(TM, instance, std::forward<Args>(args)...)
			);

		auto res = task->get_future();
		{
			std::unique_lock<std::mutex> lock(queue_mutex);

			// don't allow enqueueing after stopping the pool
			if (stop)
				throw std::runtime_error("enqueue on stopped ThreadPool");

			tasks.emplace([task]() { (*task)(); });
		}
		condition.notify_one();
		return res;
	}

	// the destructor joins all threads
	inline ThreadPool::~ThreadPool()
	{
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			stop = true;
		}
		condition.notify_all();
		for (std::thread &worker : workers)
			worker.join();
	}
}
#endif
