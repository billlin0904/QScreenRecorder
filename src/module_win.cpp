#include <stdexcept>
#include <cassert>

#include "module_win.h"

void* GetModuleFunctionAddress(const ModuleHandle& module, const std::string& name) {
	auto addr = GetProcAddress(module.get(), name.c_str());
	assert(addr != nullptr);
	return addr;
}

ModuleHandle LoadModuleLibrary(const std::string& name) {
	auto module = LoadLibraryA(name.c_str());
	assert(module != nullptr);
	if (!module) {
		throw std::invalid_argument("Not found module.");
	}
	return ModuleHandle(module);
}
