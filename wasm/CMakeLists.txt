cmake_minimum_required(VERSION 3.18)
project(garuda_wasm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(garuda_wasm
    garuda_wasm.cpp
    bindings.cpp
)

target_include_directories(garuda_wasm PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

if(EMSCRIPTEN)
    target_link_options(garuda_wasm PRIVATE
        -s WASM=1
        -s MODULARIZE=1
        -s EXPORT_NAME=createGarudaModule
        -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']
        -s ALLOW_MEMORY_GROWTH=1
        -s INITIAL_MEMORY=32MB
        -s MAXIMUM_MEMORY=256MB
        -s ENVIRONMENT=web
        -s FILESYSTEM=0
        -s ASSERTIONS=0
        -O3
        --bind
    )

    set_target_properties(garuda_wasm PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../site/wasm
        OUTPUT_NAME "garuda"
    )
endif()
