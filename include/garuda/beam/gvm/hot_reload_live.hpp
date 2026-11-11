#ifndef GARUDA_BEAM_GVM_HOT_RELOAD_LIVE_HPP
#define GARUDA_BEAM_GVM_HOT_RELOAD_LIVE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <mutex>
#include <iostream>
#include <cstdint>

namespace garuda {
namespace beam {
namespace gvm {

class GvmLiveHotReloader {
private:
    std::unordered_map<std::string, size_t> active_function_table_;
    std::map<std::string, uint32_t> function_versions_;
    mutable std::mutex reload_mutex_;

public:
    GvmLiveHotReloader() = default;

    void register_function(const std::string& func_name, size_t instruction_offset) {
        std::lock_guard<std::mutex> lock(reload_mutex_);
        active_function_table_[func_name] = instruction_offset;
        function_versions_[func_name] = 1;
    }

    bool hot_swap_function(const std::string& func_name, size_t new_instruction_offset) {
        std::lock_guard<std::mutex> lock(reload_mutex_);
        auto it = active_function_table_.find(func_name);
        if (it != active_function_table_.end()) {
            it->second = new_instruction_offset;
            function_versions_[func_name]++;
            std::cout << "⚡ [GVM HOT-RELOAD] Live code swapped for function '" << func_name
                      << "' ➔ New Offset: " << new_instruction_offset
                      << " (Version: " << function_versions_[func_name] << ")" << std::endl;
            return true;
        }
        return false;
    }

    bool get_function_offset(const std::string& func_name, size_t& out_offset) const {
        std::lock_guard<std::mutex> lock(reload_mutex_);
        auto it = active_function_table_.find(func_name);
        if (it != active_function_table_.end()) {
            out_offset = it->second;
            return true;
        }
        return false;
    }

    uint32_t get_function_version(const std::string& func_name) const {
        std::lock_guard<std::mutex> lock(reload_mutex_);
        auto it = function_versions_.find(func_name);
        if (it != function_versions_.end()) {
            return it->second;
        }
        return 0;
    }

    size_t active_function_count() const {
        std::lock_guard<std::mutex> lock(reload_mutex_);
        return active_function_table_.size();
    }
};

} // namespace gvm
} // namespace beam
} // namespace garuda

#endif // GARUDA_BEAM_GVM_HOT_RELOAD_LIVE_HPP
