include_guard(GLOBAL)

function(cs2kit_pick_target out_var)
    foreach(candidate IN LISTS ARGN)
        if(TARGET "${candidate}")
            set("${out_var}" "${candidate}" PARENT_SCOPE)
            return()
        endif()
    endforeach()

    message(FATAL_ERROR "None of the expected CMake targets exist: ${ARGN}")
endfunction()

function(cs2kit_alias_interface alias_name target_name)
    string(REPLACE "::" "_" local_name "${alias_name}")

    if(NOT TARGET "${alias_name}")
        add_library("${local_name}" INTERFACE)
        add_library("${alias_name}" ALIAS "${local_name}")
        target_link_libraries("${local_name}" INTERFACE "${target_name}")
    endif()
endfunction()

function(cs2kit_configure_third_party)
    find_package(cpr CONFIG REQUIRED)
    find_package(nlohmann_json CONFIG REQUIRED)

    cs2kit_pick_target(CS2KIT_CPR_TARGET cpr::cpr cpr)
    cs2kit_pick_target(CS2KIT_NLOHMANN_JSON_TARGET
        nlohmann_json::nlohmann_json
        nlohmann_json
    )

    cs2kit_alias_interface(thirdparty::cpr "${CS2KIT_CPR_TARGET}")
    cs2kit_alias_interface(thirdparty::nlohmann_json "${CS2KIT_NLOHMANN_JSON_TARGET}")
endfunction()
