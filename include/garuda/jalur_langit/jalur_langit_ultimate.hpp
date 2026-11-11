#pragma once
#include <string>
#include <vector>

namespace garuda::jalur_langit {
    struct ColumnDef {
        std::string name;
        std::string type;
        bool is_primary{false};
    };

    class JalurLangitUltimate {
    public:
        explicit JalurLangitUltimate(const std::string& dir) : dir_(dir) {}
        void create_table(const std::string& name, const std::vector<ColumnDef>& cols) {}
    private:
        std::string dir_;
    };
}
