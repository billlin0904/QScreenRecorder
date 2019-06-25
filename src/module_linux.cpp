#include <cassert>
#include <stdexcept>
#include "module_linux.h"

void* GetModuleFunctionAddress(const ModuleHandle& module, const std::string& name) {
    auto symbol = dlsym(module.get(), name.c_str());
    return symbol;
}

ModuleHandle LoadModuleLibrary(const std::string& name) {
    auto module = dlopen(name.c_str(), RTLD_GLOBAL | RTLD_LAZY);
    assert(module != nullptr);
    if (!module) {
        throw std::invalid_argument("Not found module.");
    }
    return ModuleHandle(module);
}
