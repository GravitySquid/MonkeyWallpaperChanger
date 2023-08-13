#pragma once
#include <thread>
#include <chrono>
#include <functional>
#include <atomic>
#include <condition_variable>

class Timer {
public:
	Timer(std::function<void()> func, std::chrono::milliseconds interval);
	~Timer();

	void start();
	void stop();

private:
	std::function<void()> func_; // The function to execute
	std::chrono::milliseconds interval_; // The interval between executions
	std::atomic<bool> running_; // The flag to indicate if the timer is running
	std::thread thread_; // The thread that runs the timer
};
