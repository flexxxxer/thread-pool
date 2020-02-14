#pragma once

#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <functional>
#include <future>

using namespace std::chrono_literals;
using namespace std;

class thread_pool
{
private:
	std::condition_variable event_obj_;
	std::mutex lock_mutex_;
	bool is_thread_pool_in_destruction_ = false;

	// fixed-size threads array
	std::vector<std::thread> threads_;

	// queue of tasks
	std::queue<std::function<void()>> tasks_queue_;

	// init thread pool
	explicit thread_pool(const size_t threads_count = std::thread::hardware_concurrency())
	{
		this->threads_.resize(threads_count); // create array of threads

		for (std::thread& th : this->threads_)
			th = std::thread(thread_pool::task_consumer, this); // init every thread
	}

	// destruct thread pool
	~thread_pool()
	{
		{
			std::unique_lock<std::mutex> locker(this->lock_mutex_); // lock pool
			this->is_thread_pool_in_destruction_ = true; // set flag

			while (!this->tasks_queue_.empty()) this->tasks_queue_.pop(); // remove all tasks
		}

		this->event_obj_.notify_all(); // notify all thread about closing

		for (std::thread& th : this->threads_)
			th.join(); // bind eof of threads to main thread
	}

	// thread consume function
	static void task_consumer(thread_pool* pool)
	{
		// infinity consume and execute tasks
		while (true)
		{
			std::function<void()> task = []() {}; // create empty task

			{
				std::unique_lock<std::mutex> locker(pool->lock_mutex_); // lock guard

				// wait event after inserting to queue
				pool->event_obj_.wait(locker, [pool]()
					{
						// if queue have items or pool is in destruction we exit from wait state
						return pool->is_thread_pool_in_destruction_ || !pool->tasks_queue_.empty();
					});

				// if pool is in destruction leave end current thread
				if (pool->is_thread_pool_in_destruction_)
					return;

				// if we have items in queue, pop task
				task = pool->tasks_queue_.front(); pool->tasks_queue_.pop();
			}

			task(); // execute task
		}
	}

public:

	/**
	 * \return thread pool instance
	 */
	static thread_pool& instance()
	{
		static thread_pool pool; // classic singleton
		return pool;
	}
	/**
	 * \return thread count in pool
	 */
	size_t thread_capacity()
	{
		unique_lock<mutex> locker(this->lock_mutex_); // lock guard

		return this->threads_.size();
	}
	/**
	 * \return queue task count
	 */
	size_t thread_queue_task_count()
	{
		unique_lock<mutex> locker(this->lock_mutex_); // lock guard

		return this->tasks_queue_.size();
	}

	/**
	 * \brief Run function in parallel using thread from pool
	 * \tparam F function type
	 * \tparam Args function args list
	 * \param func function for parallel execution
	 * \param launch_type async or deferred execution
	 * \param args function arguments
	 * \return result of executing function
	 */
	template<typename F, class... Args>
	auto execute(F&& func, std::launch launch_type, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
	{
		using return_type = typename std::result_of<F(Args...)>::type;

		// create task
		auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(func), std::forward<Args>(args)...));

		// get future result
		std::future<return_type> res = task->get_future();

		std::mutex async_mutex; // mutex for async locking
		std::unique_lock<std::mutex> async_lock; // async locking

		// for notification about async complete between threads
		std::shared_ptr<std::condition_variable> async_event_obj = std::make_shared<std::condition_variable>();

		{
			// lock guard
			std::unique_lock<std::mutex> locker(this->lock_mutex_);

			// add task do queue
			this->tasks_queue_.emplace([task, launch_type, async_event_obj]()
				{
					(*task)(); // execute task

					if (launch_type == std::launch::async) // if something wait async
						async_event_obj->notify_one(); // notify him
				});
		}

		// notify one thread
		this->event_obj_.notify_one();

		if (launch_type == std::launch::async) // if we must to wait async
			async_event_obj->wait(async_lock); // wait async

		return res; // return future result
	}

	/**
	 * \brief Asynchronously execute function using thread from pool
	 * \tparam F function type
	 * \tparam Args function args list
	 * \param func function for async execution
	 * \param args function arguments
	 * \return
	 */
	template<typename F, class... Args>
	auto async(F&& func, Args&&... args) -> typename std::result_of<F(Args...)>::type
	{
		// get future result from existing
		std::future<typename std::result_of<F(Args...)>::type> future_value = this->execute(func, std::launch::async, args...);

		// return value contained in future value object
		return future_value.get();
	}

	/**
	 * \brief Performs function in parallel using thread from pool
	 * \tparam F function type
	 * \tparam Args function args list
	 * \param func function for parallel execution
	 * \param args function arguments
	 * \return
	 */
	template<typename F, class... Args>
	auto parallel(F&& func, Args&&... args) -> future<typename std::result_of<F(Args...)>::type>
	{
		// get future result from existing
		std::future<typename std::result_of<F(Args...)>::type> future_value = this->execute(func, std::launch::deferred, args...);

		// return future value
		return future_value;
	}
};