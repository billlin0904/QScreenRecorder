#pragma once

template <typename Type, typename Traits>
class UniqueHandle {
public:
	struct boolean_struct { int member; };
	typedef int boolean_struct::* boolean_type;

	explicit UniqueHandle(Type value = Traits::invalid()) noexcept 
		: value_(value) {
	}

	UniqueHandle(UniqueHandle&& other) noexcept
		: value_(other.release()) {
	}

	UniqueHandle& operator=(UniqueHandle&& other) noexcept {
		reset(other.release());
		return *this;
	}

	~UniqueHandle() {
		close();
	}

	UniqueHandle(const UniqueHandle&) = delete;
	UniqueHandle& operator=(const UniqueHandle&) = delete;

	Type get() const noexcept {
		return value_;
	}

	void reset(Type value = Traits::invalid()) noexcept {
		if (value_ != value) {
			close();
			value_ = value;
		}
	}

	Type release() noexcept {
		auto value = value_;
		value_ = Traits::invalid();
		return value;
	}

	operator boolean_type() const noexcept {
		return Traits::invalid() != value_ ? &boolean_struct::member : nullptr;
	}

	void close() noexcept {
		if (value_ != Traits::invalid()) {
			Traits::close(value_);
		}
	}
private:
	bool operator==(UniqueHandle const &);
	bool operator!=(UniqueHandle const &);	

	Type value_;
};

template <typename Type, typename Traits>
auto swap(UniqueHandle<Type, Traits> & left, UniqueHandle<Type, Traits> & right) throw() -> void {
	left.swap(right);
}

template <typename Type, typename Traits>
auto operator==(UniqueHandle<Type, Traits> const & left, UniqueHandle<Type, Traits> const & right) noexcept -> bool {
	return left.get() == right.get();
}

template <typename Type, typename Traits>
auto operator!=(UniqueHandle<Type, Traits> const & left, UniqueHandle<Type, Traits> const & right) noexcept -> bool {
	return left.get() != right.get();
}

template <typename Type, typename Traits>
auto operator<(UniqueHandle<Type, Traits> const & left, UniqueHandle<Type, Traits> const & right) noexcept -> bool {
	return left.get() < right.get();
}

template <typename Type, typename Traits>
auto operator>=(UniqueHandle<Type, Traits> const & left, UniqueHandle<Type, Traits> const & right) noexcept -> bool {
	return left.get() >= right.get();
}

template <typename Type, typename Traits>
auto operator>(UniqueHandle<Type, Traits> const & left, UniqueHandle<Type, Traits> const & right) noexcept -> bool {
	return left.get() > right.get();
}

template <typename Type, typename Traits>
auto operator<=(UniqueHandle<Type, Traits> const & left, UniqueHandle<Type, Traits> const & right) noexcept -> bool {
	return left.get() <= right.get();
}
