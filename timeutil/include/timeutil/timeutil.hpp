#include <cstddef>
#include <chrono>
#include <ctime>
#include <thread>

namespace timeutil
{
	class Timer
	{
		public:
			std::chrono::high_resolution_clock::time_point tick;

			void tic() noexcept
			{ tick = std::chrono::high_resolution_clock::now(); }
			double toc() noexcept
			{
				auto duration = std::chrono::high_resolution_clock::now() - tick;
				return std::chrono::duration_cast<std::chrono::duration<double>>(
					duration).count();
			}
	};
	//use clock_t (so affected by clock cycles but not by sleeps etc)
	class Clocker
	{
		public:
			std::clock_t tick;
			void tic() noexcept
			{
				tick = std::clock();
			}
			double toc() noexcept
			{
				auto diff = std::clock() - tick;
				return static_cast<double>(diff) / CLOCKS_PER_SEC;
			}
	};

	//time wall clock time
	inline double time()
	{
		return std::chrono::duration_cast<std::chrono::duration<double>>(
			std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	}

	inline void sleep(double secs)
	{
		std::this_thread::sleep_for(std::chrono::duration<double>(secs));
	}
}
