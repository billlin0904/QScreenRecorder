#pragma once

#include <cassert>

#include "module_win.h"

template <typename T>
class LibraryLoader;

template <typename R, typename ...Args>
class LibraryLoader<R (Args...)> final {
public:
	LibraryLoader(const ModuleHandle& module, const std::string& name) {
		func_ = reinterpret_cast<FunctionType>(GetModuleFunctionAddress(module, name));
		assert(func_ != nullptr);
	}

	LibraryLoader(const LibraryLoader&) = delete;
	LibraryLoader& operator=(const LibraryLoader&) = delete;

	inline R operator()(Args... args) const noexcept {
		return (*func_)(args...);
	}

	explicit operator bool() const noexcept {
		return !!func_;
	}
private:
	typedef R(*FunctionType)(Args...);
	FunctionType func_;
};

#define DEFINE_EXPORT_API(ApiName) \
	LibraryLoader<decltype(ApiName)>

