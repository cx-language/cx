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

    # Disable run-time error checks to keep Debug test runs usable: /RTC,
    # RTTI, security cookies, and checked iterators slow the unoptimized
    # compiler binary several-fold, and every lit/snippet/LSP test pays that
    # cost once per compiler invocation (thousands of invocations per run).
    # Assertions stay enabled in all configurations (NDEBUG is stripped
    # above), so Debug still catches logic errors.
    string(REGEX REPLACE "/RTC[^ ]*" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REGEX REPLACE "/GR" "/GR-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GS- /MP /D_ITERATOR_DEBUG_LEVEL=0 /D_SCL_SECURE=0")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /incremental /DEBUG:FASTLINK")
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
if(MSVC)
    # JITed `cx run` programs execute on this thread; reserve the same 8MB stack the link-and-exec path gives run children.
    target_link_options(cx PRIVATE "LINKER:/STACK:8388608")
endif()

# The language server: a thin stdio process that answers every operation by
# spawning a fresh `cx-lsp --query` subprocess (one compilation per process),
# honoring the compiler's start-fast/compile-fast/exit design.
add_executable(cx-lsp src/lsp/main.cpp)
target_link_libraries(cx-lsp libcx)

if(LLVM_LINK_LLVM_DYLIB)
    set(LLVM_LIBS LLVM)
else()
    llvm_map_components_to_libnames(LLVM_LIBS core native linker orcjit passes support)
endif()
if(LLVM_LINK_LLVM_DYLIB AND TARGET clang-cpp)
    # Shared LLVM build: use only the shared libclang-cpp for the Clang C++
    # API. It contains everything the static archives below have, so linking
    # both only slows the link and bloats the binary. (Homebrew's static
    # libclangDriver.a also ships an LLVM-bitcode object file that the
    # system linker rejects.)
    list(APPEND LLVM_LIBS clang-cpp)
else()
    list(APPEND LLVM_LIBS clangAST clangBasic clangFrontend clangLex clangParse clangSema clangDriver)
endif()
target_link_libraries(libcx ${LLVM_LIBS})

# Tests. Note: tests are run with CTest rather than custom build targets so
# that expected compiler error output isn't misinterpreted as build errors
# (MSBuild fails custom build steps whose output matches its error pattern).
add_executable(example_embedding examples/embedding/embedding.cpp)
target_include_directories(example_embedding PRIVATE src)
target_link_libraries(example_embedding libcx)
include(ProcessorCount)
ProcessorCount(CHECK_PARALLEL_JOBS)
if(CHECK_PARALLEL_JOBS EQUAL 0)
    set(CHECK_PARALLEL_JOBS 4)
endif()
# Inner parallelism for the snippet suites: CTest already parallelizes across
# tests, so a full core count here oversubscribes (two suites times N threads
# plus lit shards). Cap at 4, fewer on small machines. Direct script runs keep
# the higher default for fast iteration.
set(SNIPPET_JOBS 4)
if(CHECK_PARALLEL_JOBS LESS 4)
    set(SNIPPET_JOBS ${CHECK_PARALLEL_JOBS})
endif()
add_test(NAME examples COMMAND python3 "${PROJECT_SOURCE_DIR}/examples/build_examples.py" "--cx=$<TARGET_FILE:cx>")
add_test(NAME examples-c-backend COMMAND python3 "${PROJECT_SOURCE_DIR}/examples/build_examples.py" "--cx=$<TARGET_FILE:cx>" "--backend=c")
add_test(NAME docs-snippets COMMAND python3 "${PROJECT_SOURCE_DIR}/website/check_snippets.py" "--cx=$<TARGET_FILE:cx>" "--jobs=${SNIPPET_JOBS}")
add_test(NAME docs-snippets-c-backend COMMAND python3 "${PROJECT_SOURCE_DIR}/website/check_snippets.py" "--cx=$<TARGET_FILE:cx>" "--backend=c" "--jobs=${SNIPPET_JOBS}")
add_test(NAME docs-stdlib-unit COMMAND python3 "${PROJECT_SOURCE_DIR}/website/test_generate_std_docs.py")
add_test(NAME docs-search-index COMMAND python3 "${PROJECT_SOURCE_DIR}/website/test_search_index.py")
add_test(NAME prose COMMAND python3 "${PROJECT_SOURCE_DIR}/website/check_prose.py")
# Language server end-to-end tests (query subprocess + stdio server session).
add_test(NAME lsp COMMAND python3 "${PROJECT_SOURCE_DIR}/test/lsp/test_lsp.py" "--cx-lsp=$<TARGET_FILE:cx-lsp>" "--cx=$<TARGET_FILE:cx>")
# Bound the stdio session: a framing regression would otherwise block on a
# pipe read forever instead of failing. Windows Debug runners need far more
# than 600s (each query spawns a fresh unoptimized compiler build), so allow
# 1800s; a hung session still fails instead of blocking forever.
set_tests_properties(lsp PROPERTIES TIMEOUT 1800)
# The two example suites write the same output files into examples/, so they
# must not run concurrently with each other now that check runs tests in
# parallel.
set_tests_properties(examples examples-c-backend PROPERTIES RESOURCE_LOCK examples-workdir)
# Bound every long suite: a hung compiler must fail loudly instead of
# blocking CTest forever. Generous enough for slow Debug runners.
set_tests_properties(examples examples-c-backend docs-snippets docs-snippets-c-backend PROPERTIES TIMEOUT 1200)
# Run one test per processor. The suites are CPU-bound, so extra CTest slots
# oversubscribe slower Debug runners and trigger test timeouts. The snippet
# suites cap their inner parallelism above so they don't multiply the slot
# count; examples and LSP are short enough to leave uncapped.
add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --parallel ${CHECK_PARALLEL_JOBS} -C $<CONFIG>
    USES_TERMINAL)
add_custom_target(update_snapshots ${CMAKE_COMMAND} -E env UPDATE_SNAPSHOTS=1 cmake --build "${CMAKE_BINARY_DIR}" --target check)
add_dependencies(check cx cx-lsp FileCheck example_embedding test_compile_api test_build_config test_rename test_string_interner test_mangle)

add_executable(test_string_interner test/ast/test-string-interner.cpp)
target_include_directories(test_string_interner PRIVATE src)
target_link_libraries(test_string_interner libcx)
add_test(NAME string-interner COMMAND test_string_interner)
add_executable(test_mangle test/ast/test-mangle.cpp)
target_include_directories(test_mangle PRIVATE src)
target_link_libraries(test_mangle libcx)
add_test(NAME mangle COMMAND test_mangle)

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
add_test(NAME build-config-closure-transitive COMMAND test_build_config closure-transitive)
add_test(NAME build-config-closure-cycle COMMAND test_build_config closure-cycle)
add_test(NAME build-config-closure-vendored COMMAND test_build_config closure-vendored)
add_test(NAME build-config-closure-missing-skipped COMMAND test_build_config closure-missing-skipped)
add_test(NAME build-config-resolve-dependency COMMAND test_build_config resolve-dependency)

# Tests for cx::renameFile (output file replacement).
add_executable(test_rename test/support/test-rename.cpp)
target_include_directories(test_rename PRIVATE src)
target_link_libraries(test_rename libcx)
add_test(NAME support-rename COMMAND test_rename)

# Compiler tests, sharded over one CTest test per few files so CTest
# reports progress as tests complete and the suite runs in parallel. A
# single aggregate lit invocation only reports when the whole suite
# finishes, since CTest captures each test's output until it completes
# (which also hides lit's own progress bar). Shards amortize lit's startup
# cost over several files; each shard runs single-threaded (-j1) so CTest
# remains the only scheduler. Files are dealt round-robin for balance.
# Declared after the slower suites so those start first on fresh build
# directories without recorded costs.
# CX_TEST_SHARDING=OFF recovers a single serial lit run for debugging.
option(CX_TEST_SHARDING "Split the lit suite into parallel CTest shards (disable for a single serial lit run)" ON)
file(GLOB_RECURSE LIT_TEST_FILES CONFIGURE_DEPENDS
    RELATIVE ${PROJECT_SOURCE_DIR}/test ${PROJECT_SOURCE_DIR}/test/*.cx)
list(SORT LIT_TEST_FILES)
# Mirror lit's `excludes`: helper sources under inputs/ aren't tests.
list(FILTER LIT_TEST_FILES EXCLUDE REGEX "(^|/)inputs/")
if(CX_TEST_SHARDING)
    set(LIT_SHARD_COUNT 128)
else()
    set(LIT_SHARD_COUNT 1)
endif()
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
    if(CX_TEST_SHARDING)
        set(lit_test_name lit/shard-${shard})
        set(shard_files ${LIT_SHARD_${shard}})
    else()
        # Single serial run: let lit discover the suite from the test
        # directory via test/lit.cfg. Passing every file explicitly would
        # exceed the Windows command line length limit.
        set(lit_test_name lit)
        set(shard_files ${PROJECT_SOURCE_DIR}/test)
    endif()
    add_test(NAME ${lit_test_name}
        COMMAND lit --verbose --succinct -j1 ${EXTRA_LIT_FLAGS} ${shard_files}
        -Dcx_path=$<TARGET_FILE:cx>
        -Dfilecheck_path=$<TARGET_FILE:FileCheck>
        -Dtest_dir=${PROJECT_SOURCE_DIR}/test
        -Dllvm_version=${LLVM_VERSION_MAJOR})
    # Lit already caps each file at 120s; this bounds the shard as a whole.
    set_tests_properties(${lit_test_name} PROPERTIES TIMEOUT 900)
endforeach()

# FileCheck for tests: the LLVM distribution's CMake config already exports
# an imported FileCheck target when it ships the binary; otherwise import
# the binary from the distribution, or download the source and build it.
if(NOT TARGET FileCheck)
    find_program(LLVM_FILECHECK_EXE FileCheck PATHS "${LLVM_TOOLS_BINARY_DIR}" NO_DEFAULT_PATH)
    if(LLVM_FILECHECK_EXE)
        message(STATUS "Using FileCheck from the LLVM distribution: ${LLVM_FILECHECK_EXE}")
        add_executable(FileCheck IMPORTED GLOBAL)
        set_property(TARGET FileCheck PROPERTY IMPORTED_LOCATION "${LLVM_FILECHECK_EXE}")
    endif()
endif()
if(NOT TARGET FileCheck)
    set(FILECHECK_SRC_PATH "llvmorg-${LLVM_PACKAGE_VERSION}/llvm/utils/FileCheck/FileCheck.cpp")
    set(FILECHECK_SRC_URL "https://raw.githubusercontent.com/llvm/llvm-project/${FILECHECK_SRC_PATH}")
    set(FILECHECK_SRC_FILE "${CMAKE_CURRENT_BINARY_DIR}/${FILECHECK_SRC_PATH}")
    if(NOT EXISTS "${FILECHECK_SRC_FILE}")
        message(STATUS "Downloading LLVM FileCheck source from ${FILECHECK_SRC_URL}")
        file(DOWNLOAD "${FILECHECK_SRC_URL}" "${FILECHECK_SRC_FILE}")
    endif()
    string(REGEX REPLACE "/RTC[^ ]*" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}") # Remove /RTC flags, they conflict with /O2.
    add_executable(FileCheck ${FILECHECK_SRC_FILE})
    llvm_map_components_to_libnames(FILECHECK_NEEDED_LIBS filecheck support)
    target_link_libraries(FileCheck ${FILECHECK_NEEDED_LIBS})
    # Optimize test utilities by default.
    if(MSVC)
        set_target_properties(FileCheck PROPERTIES COMPILE_FLAGS "${LLVM_DEFINITIONS} /w /O2 /GL /MT")
    else()
        set_target_properties(FileCheck PROPERTIES COMPILE_FLAGS "${LLVM_DEFINITIONS} -w -O3")
    endif()
endif()

# Optimize test utilities by default (FileCheck flags already set above
# when it is built from source; nothing to compile for the imported one).
if(MSVC)
    set_target_properties(libcx PROPERTIES COMPILE_FLAGS "/MT")
    set_target_properties(cx PROPERTIES COMPILE_FLAGS "/MT")
endif()

add_custom_target(lint COMMAND "${PROJECT_SOURCE_DIR}/scripts/format.sh" --check
    COMMAND python3 "${PROJECT_SOURCE_DIR}/website/check_prose.py")
add_custom_target(format COMMAND "${PROJECT_SOURCE_DIR}/scripts/format.sh")

# Relocatable install layout: binaries in <prefix>/bin locate std/ at
# <prefix>/share/cx/std at runtime (see getCxRootDir), so an installed
# tree works wherever it is unpacked.
include(GNUInstallDirs)
install(TARGETS cx cx-lsp RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(DIRECTORY std/ DESTINATION "${CMAKE_INSTALL_DATADIR}/cx/std")
