cmake_minimum_required(VERSION 4.3)

if(NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# ccache for standalone builds (no-op in monorepo: parent sets it first); before project().
if(NOT DEFINED CMAKE_CXX_COMPILER_LAUNCHER)
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
        set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    endif()
endif()

project(cs2-kit VERSION 1.0.0 LANGUAGES CXX)

option(CS2KIT_BUILD_STANDALONE "Build CS2Kit as a standalone project" ${PROJECT_IS_TOP_LEVEL})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CS2KitThirdParty)
include(CS2KitSdk)

cs2kit_configure_third_party()
cs2kit_configure_sdk()
cs2kit_generate_sdk_protobuf(CS2KIT_PROTO_SOURCES CS2KIT_PROTO_INCLUDE_DIRS)

file(GLOB_RECURSE CS2KIT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(cs2-kit STATIC
    ${CS2KIT_SOURCES}
    ${CS2KIT_PROTO_SOURCES}
)
add_library(CS2Kit::CS2Kit ALIAS cs2-kit)

target_compile_features(cs2-kit PUBLIC cxx_std_23)
set_target_properties(cs2-kit PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
    OUTPUT_NAME cs2-kit
)

target_include_directories(cs2-kit
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        ${CS2KIT_PROTO_INCLUDE_DIRS}
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(cs2-kit
    PUBLIC
        CS2Kit::Metamod
        CS2Kit::HL2SDK
        thirdparty::cpr
        thirdparty::nlohmann_json
)

if(WIN32)
    target_link_libraries(cs2-kit PUBLIC psapi)
endif()
