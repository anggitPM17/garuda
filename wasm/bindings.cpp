// =====================================================================
// FASE D.3.2 - Embind: JS ↔ C++ WebAssembly Bindings
// =====================================================================
#include <string>

#if defined(__EMSCRIPTEN__)
#include <emscripten/bind.h>

extern "C" const char* garuda_version();
extern "C" const char* garuda_run(const char* code);

using namespace emscripten;

EMSCRIPTEN_BINDINGS(garuda_module) {
    function("version", &garuda_version);
    function("run", &garuda_run);
}
#endif
