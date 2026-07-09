# Run via `cmake -P` each build (see CS2KitBuildInfo.cmake). All values derive
# from committed state so the header is only rewritten when a commit changes.
# Inputs: TEMPLATE_FILE, OUTPUT_FILE, VERSION_FILE, REPO_DIR, KIT_DIR.

find_program(GIT_EXECUTABLE git)

function(_cs2kit_git out_var default)
    if(NOT GIT_EXECUTABLE)
        set("${out_var}" "${default}" PARENT_SCOPE)
        return()
    endif()
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" ${ARGN}
        OUTPUT_VARIABLE output
        RESULT_VARIABLE result
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(result EQUAL 0)
        set("${out_var}" "${output}" PARENT_SCOPE)
    else()
        set("${out_var}" "${default}" PARENT_SCOPE)
    endif()
endfunction()

set(version_base "0.0.0")
if(EXISTS "${VERSION_FILE}")
    file(STRINGS "${VERSION_FILE}" version_lines LIMIT_COUNT 1)
    if(version_lines)
        string(STRIP "${version_lines}" version_base)
    endif()
endif()

_cs2kit_git(repo_commit "" -C "${REPO_DIR}" rev-parse --short HEAD)
if(repo_commit STREQUAL "" AND DEFINED ENV{GITHUB_SHA})
    string(SUBSTRING "$ENV{GITHUB_SHA}" 0 7 repo_commit)
endif()
if(repo_commit STREQUAL "")
    set(repo_commit "unknown")
endif()

_cs2kit_git(branch "" -C "${REPO_DIR}" rev-parse --abbrev-ref HEAD)
if(branch STREQUAL "" AND DEFINED ENV{GITHUB_REF_NAME})
    set(branch "$ENV{GITHUB_REF_NAME}")
endif()
if(branch STREQUAL "")
    set(branch "unknown")
endif()

_cs2kit_git(kit_commit "unknown" -C "${KIT_DIR}" rev-parse --short HEAD)
_cs2kit_git(commit_date "unknown" -C "${REPO_DIR}" log -1 --format=%cI)

set(dirty "false")
_cs2kit_git(status_output "" -C "${REPO_DIR}" status --porcelain --untracked-files=no)
if(NOT status_output STREQUAL "")
    set(dirty "true")
endif()

set(CS2KIT_BI_VERSION "${version_base}")
if(NOT repo_commit STREQUAL "unknown")
    string(APPEND CS2KIT_BI_VERSION "+${repo_commit}")
    if(dirty STREQUAL "true")
        string(APPEND CS2KIT_BI_VERSION "-dirty")
    endif()
endif()
set(CS2KIT_BI_REPO_COMMIT "${repo_commit}")
set(CS2KIT_BI_KIT_COMMIT "${kit_commit}")
set(CS2KIT_BI_BRANCH "${branch}")
set(CS2KIT_BI_DATE "${commit_date}")
set(CS2KIT_BI_DIRTY "${dirty}")

configure_file("${TEMPLATE_FILE}" "${OUTPUT_FILE}" @ONLY)
