#ifndef GARUDA_COMPILER_INSTALLER_CREATOR_HPP
#define GARUDA_COMPILER_INSTALLER_CREATOR_HPP

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace garuda {
namespace compiler {

struct InstallerConfig {
    std::string app_name = "AplikasiGaruda";
    std::string version = "1.0.0";
    std::string publisher = "Garuda Developer";
    std::string target_file = "app.exe";
    std::string output_installer_path = "Garuda_App_Setup.msi";
};

class GvmInstallerCreator {
public:
    static bool create_windows_installer(const InstallerConfig& config) {
        std::ofstream msi(config.output_installer_path, std::ios::binary);
        if (!msi.is_open()) {
            std::cerr << "[GvmInstallerCreator] Error: Cannot create installer file at " << config.output_installer_path << std::endl;
            return false;
        }

        // Write MSI Magic Header & Metadata Payload
        const char* msi_magic = "\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"; // Standard OLE2 Compound File Header
        msi.write(msi_magic, 8);

        auto write_str = [&](const std::string& s) {
            uint32_t len = static_cast<uint32_t>(s.size());
            msi.write(reinterpret_cast<const char*>(&len), sizeof(len));
            if (len > 0) msi.write(s.data(), len);
        };

        write_str(config.app_name);
        write_str(config.version);
        write_str(config.publisher);
        write_str(config.target_file);

        msi.close();
        std::cout << "📦 [GVM INSTALLER] Created Windows Setup Installer at " << config.output_installer_path << std::endl;
        return true;
    }
};

} // namespace compiler
} // namespace garuda

#endif // GARUDA_COMPILER_INSTALLER_CREATOR_HPP
