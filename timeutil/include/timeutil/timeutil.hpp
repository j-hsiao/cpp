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
				std::chrono::duration<double> duration = 
					std::chrono::high_resolution_clock::now() - tick;
				return duration.count();
			}
	};
	//use clock_t (so affected by clock cycles but not by sleeps etc)
	class Clocker
	{
		public:
			std::clock_t tick;
			void tic() noexcept
			{ tick = std::clock(); }
			double toc() noexcept
			{ return static_cast<double>(std::clock() - tick) / CLOCKS_PER_SEC; }
	};

	//wall clock time (python time.time())
	inline double time()
	{
		std::chrono::duration<double> duration =
			std::chrono::high_resolution_clock::now().time_since_epoch();
		return duration.count();
	}

	inline void sleep(double secs)
	{
		std::this_thread::sleep_for(std::chrono::duration<double>(secs));
	}
}
