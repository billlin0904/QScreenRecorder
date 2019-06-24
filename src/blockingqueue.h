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

	void clear() {
		std::lock_guard<std::mutex> guard{ mutex_ };
		queue_.clear();
	}

	void push(T && value) noexcept(std::is_nothrow_move_constructible<T>::value&& std::is_nothrow_move_assignable<T>::value) {
		{
			std::lock_guard<std::mutex> guard{ mutex_ };
			queue_.push(std::forward<T>(value));
		}
		signal_.notify_one();
	}

	bool emptry() const noexcept {
		std::lock_guard<std::mutex> guard{ mutex_ };
		return queue_.empty();
	}

	bool tryPop(T& value) noexcept(std::is_nothrow_move_constructible<T>::value&& std::is_nothrow_move_assignable<T>::value) {
		std::lock_guard<std::mutex> guard{ mutex_ };
		if (queue.empty()) {
			return false;
		}
		value = std::move(queue_.front());
		queue_.pop();
		return true;
	}

	bool tryWaitAndPop(T& value, int milli) noexcept(std::is_nothrow_move_constructible<T>::value&& std::is_nothrow_move_assignable<T>::value) {
		std::unique_lock<std::mutex> guard{ mutex_ };
		while (queue_.empty()) {
			if (!signal_.wait_for(lock, std::chrono::milliseconds(milli))) {
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