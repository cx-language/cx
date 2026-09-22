cmake_minimum_required(VERSION 3.16)
project(cx)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_UNITY_BUILD ON CACHE BOOL "Enable unity build")

if(MSVC)
    # Enable standards-conforming preprocessor. https://learn.microsoft.com/en-us/cpp/build/reference/zc-preprocessor
    string(APPEND CMAKE_CXX_FLAGS " /Zc:preprocessor")

    set(CMAKE_CXX_FLAGS "-w14062 -wd4244 -D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING ${CMAKE_CXX_FLAGS}")

    # Use static release version of Runtime Library to match prebuilt LLVM.
    # https://gitlab.kitware.com/cmake/community/wikis/FAQ#how-can-i-build-my-msvc-application-with-a-static-runtime
    foreach(flag_var CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        string(REGEX REPLACE "/MDd" "/MT" ${flag_var} "${${flag_var}}")
        string(REGEX REPLACE "/DNDEBUG" "" ${flag_var} "${${flag_var}}")
    endforeach(flag_var)

    if(NOT DEFINED ENV{CI})
        # Disable run-time error checks locally to speed up debug build.
        string(REGEX REPLACE "/RTC[^ ]*" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
        string(REGEX REPLACE "/GR" "/GR-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GS- /MP /D_ITERATOR_DEBUG_LEVEL=0 /D_SCL_SECURE=0")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /incremental /DEBUG:FASTLINK")
    endif()
else()
    set(CMAKE_CXX_FLAGS "-fno-rtti -fvisibility-inlines-hidden -Wall -Wextra -pedantic-errors -Wmissing-field-initializers -Wno-unknown-pragmas -Wno-missing-braces -Wno-sign-compare ${CMAKE_CXX_FLAGS}")

    foreach(flag_var CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        string(REGEX REPLACE "-DNDEBUG" "" ${flag_var} "${${flag_var}}")
    endforeach(flag_var)

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(CMAKE_CXX_FLAGS "-Wsuggest-override ${CMAKE_CXX_FLAGS}")
    endif()

    if(NOT MINGW)
        set(CMAKE_EXE_LINKER_FLAGS "-rdynamic ${CMAKE_EXE_LINKER_FLAGS}")
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "--system-header-prefix=llvm --system-header-prefix=clang ${CMAKE_CXX_FLAGS}")
endif()

find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)
set(LLVM_PACKAGE_VERSION "${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}.${LLVM_VERSION_PATCH}")
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
add_definitions(-DCX_ROOT_DIR="${PROJECT_SOURCE_DIR}")

# Define the include path of the Clang builtin headers, to be used by the compiler to avoid errors
# about missing C headers when importing C headers from cx code. For more information, see e.g.
# http://clang.llvm.org/docs/FAQ.html#i-get-errors-about-some-headers-being-missing-stddef-h-stdarg-h
# CLANG_CMAKE_DIR is defined by ClangConfig.cmake during the above find_package(Clang) call.
add_definitions(-DCLANG_BUILTIN_INCLUDE_PATH="${CLANG_CMAKE_DIR}/../../clang/${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}.${LLVM_VERSION_PATCH}/include")

enable_testing()

file(GLOB_RECURSE LIBCX_SOURCES CONFIGURE_DEPENDS
    src/ast/*.h src/ast/*.cpp
    src/backend/*.h src/backend/*.cpp
    src/driver/*.h src/driver/*.cpp
    src/build/*.h src/build/*.cpp
    src/parser/*.h src/parser/*.cpp
    src/sema/*.h src/sema/*.cpp
    src/support/*.h src/support/*.cpp
    src/lsp/*.h src/lsp/*.cpp
    src/api.cpp
)
# The WebAssembly API in src/wasm/ is linked into the Emscripten module (and
# test binaries) separately, not into libcx. It must be excluded explicitly:
# GLOB_RECURSE also matches the src/api.cpp pattern against src/wasm/api.cpp,
# which would otherwise put a duplicate cxCompileToC definition into libcx.
list(FILTER LIBCX_SOURCES EXCLUDE REGEX "src/wasm/")
# The language server entry point has its own main() and lives in the cx-lsp
# binary, not in libcx.
list(FILTER LIBCX_SOURCES EXCLUDE REGEX "src/lsp/main.cpp")
add_library(libcx ${LIBCX_SOURCES})
set_target_properties(libcx PROPERTIES PREFIX "")
target_precompile_headers(libcx PRIVATE src/pch.h)

add_executable(cx src/cli/main.cpp)
target_link_libraries(cx libcx)

# The language server: a thin stdio process that answers every operation by
# spawning a fresh `cx-lsp --query` subprocess (one compilation per process),
# honoring the compiler's start-fast/compile-fast/exit design.
add_executable(cx-lsp src/lsp/main.cpp)
target_link_libraries(cx-lsp libcx)

if(LLVM_LINK_LLVM_DYLIB)
    set(LLVM_LIBS LLVM)
else()
    llvm_map_components_to_libnames(LLVM_LIBS core native linker passes support)
endif()
list(APPEND LLVM_LIBS clangAST clangBasic clangFrontend clangLex clangParse clangSema)
if(LLVM_LINK_LLVM_DYLIB AND TARGET clang-cpp)
    # Shared LLVM build: use the shared libclang-cpp for the Clang C++ API
    # instead of the static archives. (Homebrew's static libclangDriver.a
    # ships an LLVM-bitcode object file that the system linker rejects.)
    list(APPEND LLVM_LIBS clang-cpp)
else()
    list(APPEND LLVM_LIBS clangDriver)
endif()
target_link_libraries(libcx ${LLVM_LIBS})

# Tests. Note: tests are run with CTest rather than custom build targets so
# that expected compiler error output isn't misinterpreted as build errors
# (MSBuild fails custom build steps whose output matches its error pattern).
add_executable(example_embedding examples/embedding/embedding.cpp)
target_include_directories(example_embedding PRIVATE src)
target_link_libraries(example_embedding libcx)
add_test(NAME examples COMMAND python3 "${PROJECT_SOURCE_DIR}/examples/build_examples.py" "--cx=$<TARGET_FILE:cx>")
add_test(NAME examples-c-backend COMMAND python3 "${PROJECT_SOURCE_DIR}/examples/build_examples.py" "--cx=$<TARGET_FILE:cx>" "--backend=c")
add_test(NAME docs-snippets COMMAND python3 "${PROJECT_SOURCE_DIR}/docs/check_snippets.py" "--cx=$<TARGET_FILE:cx>")
add_test(NAME docs-snippets-c-backend COMMAND python3 "${PROJECT_SOURCE_DIR}/docs/check_snippets.py" "--cx=$<TARGET_FILE:cx>" "--backend=c")
add_test(NAME docs-stdlib-unit COMMAND python3 "${PROJECT_SOURCE_DIR}/docs/test_generate_std_docs.py")
# Language server end-to-end tests (query subprocess + stdio server session).
add_test(NAME lsp COMMAND python3 "${PROJECT_SOURCE_DIR}/test/lsp/test_lsp.py" "--cx-lsp=$<TARGET_FILE:cx-lsp>" "--cx=$<TARGET_FILE:cx>")
# Bound the stdio session: a framing regression would otherwise block on a
# pipe read forever instead of failing (takes ~2s locally; 600s is ample
# even for Windows Debug).
set_tests_properties(lsp PROPERTIES TIMEOUT 600)
# The two example suites write the same output files into examples/, so they
# must not run concurrently with each other now that check runs tests in
# parallel.
set_tests_properties(examples examples-c-backend PROPERTIES RESOURCE_LOCK examples-workdir)
include(ProcessorCount)
ProcessorCount(CHECK_PARALLEL_JOBS)
if(CHECK_PARALLEL_JOBS EQUAL 0)
    set(CHECK_PARALLEL_JOBS 4)
endif()
# Two extra slots so the serial long-pole suites (docs, examples, lsp)
# don't starve the lit shards of workers.
math(EXPR CHECK_PARALLEL_JOBS "${CHECK_PARALLEL_JOBS} + 2")
add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --parallel ${CHECK_PARALLEL_JOBS} -C $<CONFIG>
    USES_TERMINAL)
add_custom_target(update_snapshots ${CMAKE_COMMAND} -E env UPDATE_SNAPSHOTS=1 cmake --build "${CMAKE_BINARY_DIR}" --target check)
add_dependencies(check cx cx-lsp FileCheck example_embedding test_compile_api test_build_config test_rename)

# Tests for the in-memory compilation API (also used by the WebAssembly build).
add_executable(test_compile_api test/compile-api/test-compile-api.cpp src/wasm/api.cpp)
target_include_directories(test_compile_api PRIVATE src)
target_compile_definitions(test_compile_api PRIVATE CX_TEST_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
target_link_libraries(test_compile_api libcx)
foreach(test_case hello error struct warning dispatch json)
    add_test(NAME compile-api-${test_case} COMMAND test_compile_api ${test_case})
endforeach()

# Tests for build.cx dependency parsing.
add_executable(test_build_config test/build-config/test-build-config.cpp)
target_include_directories(test_build_config PRIVATE src)
target_link_libraries(test_build_config libcx)
add_test(NAME build-config-git-urls COMMAND test_build_config git-urls)
add_test(NAME build-config-missing-url COMMAND test_build_config missing-url)
set_tests_properties(build-config-missing-url PROPERTIES PASS_REGULAR_EXPRESSION "missing required 'url'")
add_test(NAME build-config-missing-build-file COMMAND test_build_config missing-build-file)
add_test(NAME build-config-search-paths COMMAND test_build_config search-paths)

# Tests for cx::renameFile (output file replacement).
add_executable(test_rename test/support/test-rename.cpp)
target_include_directories(test_rename PRIVATE src)
target_link_libraries(test_rename libcx)
add_test(NAME support-rename COMMAND test_rename)

# Compiler tests, sharded over one CTest test per few files so CTest
# reports progress as tests complete. A single aggregate lit invocation
# only reports when the whole suite finishes, since CTest captures each
# test's output until it completes (which also hides lit's own progress
# bar). Shards amortize lit's startup cost over several files; each shard
# runs single-threaded (-j1) so CTest remains the only scheduler. Files
# are dealt round-robin for balance. Declared after the slower suites so
# those start first on fresh build directories without recorded costs.
file(GLOB_RECURSE LIT_TEST_FILES CONFIGURE_DEPENDS
    RELATIVE ${PROJECT_SOURCE_DIR}/test ${PROJECT_SOURCE_DIR}/test/*.cx)
list(SORT LIT_TEST_FILES)
# Mirror lit's `excludes`: helper sources under inputs/ aren't tests.
list(FILTER LIT_TEST_FILES EXCLUDE REGEX "(^|/)inputs/")
set(LIT_SHARD_COUNT 128)
set(shard_index 0)
foreach(lit_test IN LISTS LIT_TEST_FILES)
    math(EXPR shard "${shard_index} % ${LIT_SHARD_COUNT}")
    list(APPEND LIT_SHARD_${shard} "${PROJECT_SOURCE_DIR}/test/${lit_test}")
    math(EXPR shard_index "${shard_index} + 1")
endforeach()
math(EXPR LIT_SHARD_MAX "${LIT_SHARD_COUNT} - 1")
foreach(shard RANGE ${LIT_SHARD_MAX})
    if(NOT DEFINED LIT_SHARD_${shard})
        continue()
    endif()
    set(shard_files ${LIT_SHARD_${shard}})
    add_test(NAME lit/shard-${shard}
        COMMAND lit --verbose --succinct -j1 ${EXTRA_LIT_FLAGS} ${shard_files}
        -Dcx_path=$<TARGET_FILE:cx>
        -Dfilecheck_path=$<TARGET_FILE:FileCheck>
        -Dtest_dir=${PROJECT_SOURCE_DIR}/test
        -Dllvm_version=${LLVM_VERSION_MAJOR})
endforeach()

# Download the LLVM FileCheck utility for tests.
set(FILECHECK_SRC_PATH "llvmorg-${LLVM_PACKAGE_VERSION}/llvm/utils/FileCheck/FileCheck.cpp")
set(FILECHECK_SRC_URL "https://raw.githubusercontent.com/llvm/llvm-project/${FILECHECK_SRC_PATH}")
set(FILECHECK_SRC_FILE "${CMAKE_CURRENT_BINARY_DIR}/${FILECHECK_SRC_PATH}")
if(NOT EXISTS "${FILECHECK_SRC_FILE}")
    message(STATUS "Downloading LLVM FileCheck source from ${FILECHECK_SRC_URL}")
    file(DOWNLOAD "${FILECHECK_SRC_URL}" "${FILECHECK_SRC_FILE}")
endif()
if(NOT TARGET FileCheck)
    string(REGEX REPLACE "/RTC[^ ]*" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}") # Remove /RTC flags, they conflict with /O2.
    add_executable(FileCheck ${FILECHECK_SRC_FILE})
    llvm_map_components_to_libnames(FILECHECK_NEEDED_LIBS filecheck support)
    target_link_libraries(FileCheck ${FILECHECK_NEEDED_LIBS})
endif()

# Optimize test utilities by default.
if(MSVC)
    set_target_properties(FileCheck PROPERTIES COMPILE_FLAGS "${LLVM_DEFINITIONS} /w /O2 /GL /MT")
    set_target_properties(libcx PROPERTIES COMPILE_FLAGS "/MT")
    set_target_properties(cx PROPERTIES COMPILE_FLAGS "/MT")
else()
    set_target_properties(FileCheck PROPERTIES COMPILE_FLAGS "${LLVM_DEFINITIONS} -w -O3")
endif()

add_custom_target(lint COMMAND "${PROJECT_SOURCE_DIR}/scripts/format.sh" --check)
add_custom_target(format COMMAND "${PROJECT_SOURCE_DIR}/scripts/format.sh")

# Relocatable install layout: binaries in <prefix>/bin locate std/ at
# <prefix>/share/cx/std at runtime (see getCxRootDir), so an installed
# tree works wherever it is unpacked.
include(GNUInstallDirs)
install(TARGETS cx cx-lsp RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(DIRECTORY std/ DESTINATION "${CMAKE_INSTALL_DATADIR}/cx/std")
