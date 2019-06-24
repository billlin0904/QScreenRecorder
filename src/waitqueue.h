#pragma once

#include <queue>
#include <condition_variable>
#include <mutex>

template <typename T>
class WaitQueue {
public:
	WaitQueue() {
	}

	~WaitQueue() {
	}

	WaitQueue(const WaitQueue&) = delete;
	WaitQueue& operator=(const WaitQueue&) = delete;

	size_t availableCount() const noexcept {
		std::lock_guard<std::mutex> guard{ mutex_ };
		return queue_.size();
	}

	void push(T && value) noexcept(std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_assignable<T>::value) {
		{
			std::lock_guard<std::mutex> guard{ mutex_ };
			queue_.push(std::move(value));
		}
		signal_.notify_one();
	}

	bool emptry() const noexcept(std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_assignable<T>::value) {
		std::lock_guard<std::mutex> guard{ mutex_ };
		return queue_.empty();
	}

	bool tryPop(T & value) noexcept(std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_assignable<T>::value) {
		std::lock_guard<std::mutex> guard{ mutex_ };
		if (queue_.empty()) {
			return false;
		}
		value = std::move(queue_.front());
		queue_.pop();
		return true;
	}

	bool tryWaitAndPop(T & value, int milli) noexcept(std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_assignable<T>::value) {
		std::unique_lock<std::mutex> lock{ mutex_ };
		const std::chrono::milliseconds timeout(milli);
		while (queue_.empty()) {
			if (!signal_.wait_for(lock, timeout, [this]() { return !queue_.empty(); })) {
				return false;
			}			
		}
		value = std::move(queue_.front());
		queue_.pop();
		return true;
	}
private:
	mutable std::mutex mutex_;
	std::queue<T> queue_;
	std::condition_variable signal_;
};