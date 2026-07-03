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
option(CS2KIT_ENABLE_POSTGRES "Build the CS2Kit::Database Postgres client (requires libpqxx)" OFF)

# Conan imported targets are directory-scoped; make them visible to sibling
# plugin directories when cs2-kit is vendored into a monorepo.
set(CMAKE_FIND_PACKAGE_TARGETS_GLOBAL ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CS2KitSdk)

find_package(cpr CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
if(CS2KIT_ENABLE_POSTGRES)
    find_package(libpqxx CONFIG REQUIRED)
endif()

cs2kit_configure_sdk()
cs2kit_generate_sdk_protobuf(CS2KIT_PROTO_SOURCES CS2KIT_PROTO_INCLUDE_DIRS)

file(GLOB_RECURSE CS2KIT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
if(NOT CS2KIT_ENABLE_POSTGRES)
    list(FILTER CS2KIT_SOURCES EXCLUDE REGEX "src/Database/")
endif()

# CEntityKeyValues/KeyValues3 method bodies live in these SDK sources - they are
# neither header-inline nor in the prebuilt tier1 libs - so entity spawn keyvalues
# (EntityKeyValues/EntityOpsService) need them compiled into the kit.
set(CS2KIT_HL2SDK_SOURCES
    "${CS2KIT_HL2SDK_DIR}/entity2/entitykeyvalues.cpp"
    "${CS2KIT_HL2SDK_DIR}/entity2/entitysystem.cpp"
    "${CS2KIT_HL2SDK_DIR}/tier1/keyvalues3.cpp"
)

add_library(cs2-kit STATIC
    ${CS2KIT_SOURCES}
    ${CS2KIT_HL2SDK_SOURCES}
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
        cpr::cpr
        nlohmann_json::nlohmann_json
)

if(CS2KIT_ENABLE_POSTGRES)
    # PUBLIC: PostgresDatabase.hpp exposes pqxx types in its templates.
    target_link_libraries(cs2-kit PUBLIC libpqxx::pqxx)
endif()

if(WIN32)
    target_link_libraries(cs2-kit PUBLIC psapi)
endif()

if(CS2KIT_BUILD_STANDALONE)
    include(CTest)
endif()

# SDK-free utils tests: recompile the pure-logic sources instead of linking
# cs2-kit, so the test binary needs no HL2SDK/Metamod link inputs.
if(BUILD_TESTING)
    add_executable(cs2kit-utils-tests
        tests/MicroTestMain.cpp
        tests/EffectManagerTests.cpp
        tests/MemoryAccessTests.cpp
        tests/MigratorTests.cpp
        tests/ParseDurationTests.cpp
        tests/SlotThrottleTests.cpp
        tests/StringUtilsTests.cpp
        tests/SteamIdTests.cpp
        tests/TimeUtilsTests.cpp
        src/Core/EffectManager.cpp
        src/Core/Scheduler.cpp
        src/Utils/StringUtils.cpp
        src/Utils/SteamId.cpp
        src/Utils/TimeUtils.cpp
    )

    target_compile_features(cs2kit-utils-tests PRIVATE cxx_std_23)
    target_include_directories(cs2kit-utils-tests PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    )

    add_test(NAME cs2kit-utils COMMAND cs2kit-utils-tests)
endif()
