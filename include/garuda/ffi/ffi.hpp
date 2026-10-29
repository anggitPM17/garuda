#pragma once
// =====================================================================
// FASE D.2.1 - C-ABI FFI: Type-safe, zero-copy, RAII
// =====================================================================
#include <string>
#include <vector>
#include <functional>
#include <optional>
#include <stdexcept>
#include <cstdint>
#include <memory>
#include <utility>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace garuda::ffi {

class FfiError : public std::runtime_error {
public:
    explicit FfiError(const std::string& msg) : std::runtime_error(msg) {}
};

class DlHandle {
private:
    void* handle_ = nullptr;

public:
    DlHandle() = default;

    explicit DlHandle(const std::string& path) {
#if defined(_WIN32)
        handle_ = static_cast<void*>(LoadLibraryA(path.c_str()));
#else
        handle_ = dlopen(path.c_str(), RTLD_LAZY | RTLD_LOCAL);
#endif
    }

    ~DlHandle() {
        if (handle_) {
#if defined(_WIN32)
            FreeLibrary(static_cast<HMODULE>(handle_));
#else
            dlclose(handle_);
#endif
            handle_ = nullptr;
        }
    }

    DlHandle(const DlHandle&) = delete;
    DlHandle& operator=(const DlHandle&) = delete;

    DlHandle(DlHandle&& other) noexcept : handle_(other.handle_) {
        other.handle_ = nullptr;
    }

    DlHandle& operator=(DlHandle&& other) noexcept {
        if (this != &other) {
            if (handle_) {
#if defined(_WIN32)
                FreeLibrary(static_cast<HMODULE>(handle_));
#else
                dlclose(handle_);
#endif
            }
            handle_ = other.handle_;
            other.handle_ = nullptr;
        }
        return *this;
    }

    void* get() const { return handle_; }

    void* symbol(const std::string& name) const {
        if (!handle_) return nullptr;
#if defined(_WIN32)
        return reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(handle_), name.c_str()));
#else
        return dlsym(handle_, name.c_str());
#endif
    }

    bool valid() const { return handle_ != nullptr; }
};

class CFunction {
private:
    std::shared_ptr<DlHandle> handle_;
    void* symbol_ = nullptr;
    std::string name_;

public:
    CFunction() = default;

    CFunction(std::shared_ptr<DlHandle> handle, void* symbol, std::string name)
        : handle_(std::move(handle)), symbol_(symbol), name_(std::move(name)) {}

    static std::optional<CFunction> load(const std::string& library_path, const std::string& symbol_name) {
        auto handle = std::make_shared<DlHandle>(library_path);
        if (!handle->valid()) return std::nullopt;

        void* sym = handle->symbol(symbol_name);
        if (!sym) return std::nullopt;

        return CFunction(handle, sym, symbol_name);
    }

    template<typename Ret, typename... Args>
    Ret call(Args... args) const {
        if (!symbol_) throw FfiError("CFunction symbol not loaded");
        using FnType = Ret(*)(Args...);
        auto fn = reinterpret_cast<FnType>(symbol_);
        return fn(std::forward<Args>(args)...);
    }

    bool is_loaded() const { return symbol_ != nullptr; }
    std::string name() const { return name_; }
};

} // namespace garuda::ffi
