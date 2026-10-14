# Regenerates the cx version header at build time so cx --version tracks
# branch switches without a cmake rerun. Rewrites the file only when the hash
# changes, so dependents don't rebuild on every build. Expects SRC_DIR, OUT,
# and GIT_EXECUTABLE (empty when git is unavailable).
if(GIT_EXECUTABLE)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
        WORKING_DIRECTORY ${SRC_DIR}
        OUTPUT_VARIABLE hash
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)
endif()
if(NOT hash)
    set(hash "unknown")
endif()
set(content "#pragma once\n#define CX_COMMIT_HASH \"${hash}\"\n")
get_filename_component(dir ${OUT} DIRECTORY)
file(MAKE_DIRECTORY ${dir})
set(write TRUE)
if(EXISTS ${OUT})
    file(READ ${OUT} old)
    if(old STREQUAL content)
        set(write FALSE)
    endif()
endif()
if(write)
    file(WRITE ${OUT} "${content}")
endif()
