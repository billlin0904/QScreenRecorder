#pragma once

#include <string>

#include <dlfcn.h>

#include "unique_handle.h"

struct ModuleHandleTraits {
    static void* invalid() noexcept {
        return nullptr;
    }
    static void close(void* value) noexcept {
        dlclose(value);
    }
};

using ModuleHandle = UniqueHandle<void*, ModuleHandleTraits>;

void* GetModuleFunctionAddress(const ModuleHandle& module, const std::string& name);

ModuleHandle LoadModuleLibrary(const std::string& name);
