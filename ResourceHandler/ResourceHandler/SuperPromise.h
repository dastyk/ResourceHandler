#ifndef _UTILZ_SUPER_PROMISE_H_
#define _UTILZ_SUPER_PROMISE_H_
#include <condition_variable>
#include <memory>

namespace Utilz
{
	template<class T>
	struct SharedPromFut
	{
		bool set = false;
		T load;
		std::condition_variable c;
		std::mutex m;
	};

	template<class T>
	class SuperPromise
	{
		std::shared_ptr<SharedPromFut<T>> shared;
	public:
		SuperPromise() : shared(std::make_shared<SharedPromFut<T>>())
		{

		}
		void SetValue(T v)
		{
			shared->load = v;
			shared->set = true;
			shared->c.notify_all();
		}
		inline auto GetFuture()
		{
			return SuperFuture<T>(shared);
		}
	};

	template<class T>
	class SuperFuture
	{
		std::shared_ptr<SharedPromFut<T>> shared;
	public:

		SuperFuture(std::shared_ptr<SharedPromFut<T>> shared) : shared(shared)
		{}
		const T& Get()const
		{
			if (shared->set)
				return shared->load;
			else
			{
				std::unique_lock<std::mutex> lock(shared->m);
				shared->c.wait(lock);
				return shared->load;
			}
		}
	};
}


#endif