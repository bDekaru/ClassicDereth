#pragma once


#include <chrono>
#include <future>
#include <type_traits>
#include <list>

namespace taskex
{
	/// simple task manager that accepts callables with a time to execute
	class task_manager
	{
	public:
		void clear()
		{
			m_tasks.clear();
		}

		/// execute pending tasks that should occur after the given time
		void exec(double ts)
		{
			auto itr = m_tasks.begin();
			while (itr != m_tasks.end())
			{
				if (itr->time <= ts)
				{
					itr->delgate();
					itr = m_tasks.erase(itr);
				}
				else
					itr++;
			}
		}

		/// queue task to execute after the given time
		void queue(double ts, std::function<void(void)> fn)
		{
			delay_run_t entry = { ts, fn };

			auto itr = m_tasks.begin();
			while (itr != m_tasks.end())
			{
				if (itr->time > ts)
				{
					m_tasks.insert(itr, entry);
					return;
				}
				itr++;
			}
			m_tasks.push_back(entry);
		}

	protected:

		struct delay_run_t
		{
			double time;
			std::function<void(void)> delgate;
		};

		std::list<delay_run_t> m_tasks;
	};

#if 0
	namespace experimental
	{
		/**
			auto task = taskex::runtask([]()
				{
					// i run on the thread pool
					return somevalfromdb;
				},
				[](dbresult expected)
				{
					// i don't actually run until something else calls resume on the task
				});

			m_tasks.push_back(task);

			// ...

			void CWeenieObject::Tick()
			{
				// ..

				for (auto itr = m_tasks.begin(); itr != m_tasks.end();)
				{
					if (itr->ready())
					{
						itr->resume();
						itr = m_tasks.erase(itr);
					}
					else
						itr++;
				}

				// ..
			}
		*/

		/// wrap std::async calls in such a way that another task can be
		/// resumed on a chosen thread after the initial task/promise has
		/// completed. the future result is passed onto the resumed callable
		template<typename ResultType>
		class task_resumed
		{
		public:
			using result_t = ResultType;
			using future_t = std::future<result_t>;
			using resume_fn_t = std::function<void(result_t&)>;
			using future_status = std::future_status;
			using milliseconds = std::chrono::milliseconds;

			task_resumed(future_t future) : m_future(future);
			{
			}
			task_resumed(future_t future, resume_fn_t resume) : m_future(future), m_resume(resume)
			{
			}

			bool valid() { return m_future.valid(); }

			bool ready()
			{
				return valid() && m_future.wait_for(milliseconds(0)) == future_status::ready;
			}

			void resume()
			{
				if (m_resume)
					m_resume(m_future.get());
			}

		protected:
			future_t m_future;
			resume_fn_t m_resume;

		private:
		};


#if __cpp_lib_is_invocable >= 201703
		template<typename Fn>
		using fn_res_t = std::invoke_result_t<Fn, void>;
#else
		template<typename Fn, typename... FnArgs>
		using fn_res_t = std::result_of_t<Fn, void>;
#endif

		template<class Fn>
		inline task_resumed<fn_res_t<Fn>> runtask(Fn&& fn)
		{
			return task_resumed<fn_res_t<Fn>>(std::async(std::launch::async, fn));
		}

		template<class Fn, class ResumeFn>
		inline task_resumed<fn_res_t<Fn>> runtask(Fn&& fn, ResumeFn&& resume)
		{
			return task_resumed<fn_res_t<Fn>>(std::async(std::launch::async, fn), resume);
		}

	};
#endif
};
