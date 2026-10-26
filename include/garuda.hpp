#ifndef GARUDA_HPP
#define GARUDA_HPP

#include "garuda/version.hpp"
#include <iostream>

class GarudaUltraKernel {
public:
    GarudaUltraKernel() {
        std::cout << "🌌 [ULTRA KERNEL] " << GARUDA_ENGINE_NAME << " v" << GARUDA_VERSION_STRING << " online." << std::endl;
    }
};

#endif
