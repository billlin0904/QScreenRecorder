#pragma once

#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif

#include <Windows.h>
#include <stdexcept>

#include "unique_handle.h"

struct ModuleHandleTraits {
	static HMODULE invalid() noexcept {
		return nullptr;
	}
	static void close(HMODULE value) noexcept {
		FreeLibrary(value);
	}
};

using ModuleHandle = UniqueHandle<HMODULE, ModuleHandleTraits>;

void* GetModuleFunctionAddress(const ModuleHandle& module, const std::string& name);

ModuleHandle LoadModuleLibrary(const std::string& name);