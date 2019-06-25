#pragma once

#include <atomic>
#include <vector>

template <typename T, size_t Capacity>
class SPSCQueue {
public:
	SPSCQueue() 
		: head_(0)
		, tail_(0)
		, queue_(Capacity + 1) {
	}

	constexpr size_t capacity() const {
		return Capacity;
	}

	bool emptry() const noexcept {
		return (head_.load() == tail_.load());
	}

	size_t availableCount() const noexcept {
		return (head_.load(std::memory_order_acquire) - tail_.load(std::memory_order_relaxed)) % Capacity;
	}

	bool tryEnqueue(T&& value) noexcept(std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_assignable<T>::value) {
		const auto head = head_.load(std::memory_order_relaxed);
		auto next_head = (head + 1) % Capacity;
		if (next_head == tail_.load(std::memory_order_acquire)) {
			return false;
		}
		queue_[head] = std::forward<T>(value);
		head_.store(next_head, std::memory_order_release);
		return true;
	}

	bool tryPop(T& value) noexcept(std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_assignable<T>::value) {
		const auto tail = tail_.load(std::memory_order_relaxed);
		if (tail == head_.load(std::memory_order_acquire)) {
			return false;
		}
		value = std::move(queue_[tail]);
		tail_.store((tail + 1) % Capacity, std::memory_order_release);
		return true;
	}
private:
	std::atomic<size_t> head_;
	uint8_t padding1_[128]{};
	std::atomic<size_t> tail_;
	uint8_t padding2_[128]{};
	std::vector<T> queue_;
};
