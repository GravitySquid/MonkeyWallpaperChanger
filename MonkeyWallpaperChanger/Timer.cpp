#include "Timer.h"

Timer::Timer(std::function<void()> func, std::chrono::milliseconds interval)
	: func_(func), interval_(interval), running_(false) {}

// Destructor that stops the timer if it is running
Timer::~Timer() {
	stop();
}

// Start the timer on a separate thread
void Timer::start() {
	running_ = true;
	thread_ = std::thread([this]() {
		// Loop until the timer is stopped
		std::chrono::steady_clock::time_point prevChangedTime = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point currentTime = prevChangedTime;
		while (running_) {
			// Wait for the specified interval
			currentTime = std::chrono::steady_clock::now();
			int elapsedSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - prevChangedTime).count();
			// Execute the function if the timer is still running
			if (running_ && elapsedSeconds > interval_.count()) {
				func_();
				prevChangedTime = std::chrono::steady_clock::now();
			}
			//* Sleep 10% of interval, then check in. Dont want to churn CPU constantly.
			//* Timer maybe out +/- 10% 
			std::this_thread::sleep_for(interval_ * 0.1);
		}
		});
}

// Stop the timer and join the thread
void Timer::stop() {
	running_ = false;
	if (thread_.joinable()) {
		thread_.join();
	}
}

