// Anchor TU untuk STATIC library garuda.
// JANGAN include header berat di sini (script_engine.hpp, httplib.h, dll).
namespace garuda { void _core_link_anchor() {} }
