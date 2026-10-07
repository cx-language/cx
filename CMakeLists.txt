cmake_minimum_required(VERSION 3.16)
# FetchContent dependencies declaring cmake_minimum_required < 3.5 (zlib)
# are a hard error under CMake 4+. This floor maps those declarations to
# 3.5; it does not affect this project (3.16) and is inert on older CMake.
if(CMAKE_VERSION VERSION_GREATER_EQUAL "4.0")
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
endif()
project(cx)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_UNITY_BUILD ON CACHE BOOL "Enable unity build")

if(MSVC)
    # Use the static MSVC runtime (/MT even in Debug) to match the prebuilt
    # LLVM distribution. Set the modern variable so FetchContent dependencies
    # (zlib, zstd) get it too, and keep the flag rewrite below for the flags
    # themselves. C flags are included: zlib/zstd are C.
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

    # Enable standards-conforming preprocessor. https://learn.microsoft.com/en-us/cpp/build/reference/zc-preprocessor
    string(APPEND CMAKE_CXX_FLAGS " /Zc:preprocessor")

    set(CMAKE_CXX_FLAGS "-w14062 -wd4244 -D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING ${CMAKE_CXX_FLAGS}")

    # Use static release version of Runtime Library to match prebuilt LLVM.
    # https://gitlab.kitware.com/cmake/community/wikis/FAQ#how-can-i-build-my-msvc-application-with-a-static-runtime
    foreach(flag_var CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        string(REGEX REPLACE "/MDd?" "/MT" ${flag_var} "${${flag_var}}")
        string(REGEX REPLACE "/DNDEBUG" "" ${flag_var} "${${flag_var}}")
    endforeach(flag_var)

    # Disable run-time error checks to keep Debug test runs usable: /RTC,
    # RTTI, security cookies, and checked iterators slow the unoptimized
    # compiler binary several-fold, and every lit/snippet/LSP test pays that
    # cost once per compiler invocation (thousands of invocations per run).
    # Assertions stay enabled in all configurations (NDEBUG is stripped
    # above), so Debug still catches logic errors.
    string(REGEX REPLACE "/RTC[^ ]*" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
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

# Instrumented build for the LSan arena gate (test/lsp/check_lsan.py), which
# runs cx-lsp --leak-check. Needs a Clang with -fsanitize=leak (LLVM Clang on
# macOS/Linux); Apple Clang lacks it.
option(CX_SANITIZE_LEAK "Build with -fsanitize=leak for the LSan arena gate" OFF)
if(CX_SANITIZE_LEAK)
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(FATAL_ERROR "CX_SANITIZE_LEAK needs LLVM Clang with -fsanitize=leak")
    endif()
    add_compile_options(-fsanitize=leak)
    add_link_options(-fsanitize=leak)
endif()

include(FetchContent)

# LLVM's CMake config includes LLVMExports.cmake, which hard-errors if any of
# its link-interface targets are missing: ZLIB::ZLIB and zstd::libzstd_static
# (via LLVMSupport), LibXml2::LibXml2 (via LLVMWindowsManifest), and
# DIASDK::Diaguids (via LLVMDebugInfoPDB). LLVMConfig.cmake itself calls
# find_package() for each, but on a bare Windows machine none are findable.
# Provide them automatically so contributors need no manual vcpkg installs.
# System/vcpkg libraries are preferred when findable (see CMAKE_PREFIX_PATH);
# the FetchContent fallbacks below only run when they are absent.
#
# ZLIB and zstd are linked into every binary via LLVMSupport, so they must be
# real libraries and are built from source when missing. LibXml2 and the DIA
# SDK back LLVM components cx never links (our llvm_map_components set is
# core/native/linker/orcjit/passes/support plus the Clang libraries, which
# contains neither LLVMWindowsManifest nor LLVMDebugInfoPDB), so dummy
# INTERFACE targets suffice for those when they are absent. If you add an
# LLVM component that needs either, install the real library instead.
set(CX_LIBXML2_DUMMY FALSE)
set(CX_DIASDK_DUMMY FALSE)

# --- ZLIB (real, required via LLVMSupport) ---
find_package(ZLIB QUIET)
if(NOT TARGET ZLIB::ZLIB)
    message(STATUS "ZLIB not found, fetching zlib from source")
    FetchContent_Declare(zlib
        GIT_REPOSITORY https://github.com/madler/zlib.git
        GIT_TAG v1.3.1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        UPDATE_DISCONNECTED TRUE)
    set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "Build zlib examples" FORCE)
    set(SKIP_INSTALL_ALL ON CACHE BOOL "Skip zlib install targets" FORCE)
    FetchContent_MakeAvailable(zlib)
    if(NOT TARGET ZLIB::ZLIB)
        add_library(ZLIB::ZLIB ALIAS zlibstatic)
    endif()
    # zlib's sources don't compile in unity (jumbo) mode; our top-level
    # CMAKE_UNITY_BUILD=ON would otherwise leak into this subbuild.
    set_target_properties(zlib zlibstatic PROPERTIES UNITY_BUILD OFF)
    # Our alias satisfies LLVMExports; skip LLVMConfig's own (noisy) search.
    set(CMAKE_DISABLE_FIND_PACKAGE_ZLIB TRUE)
endif()

# --- zstd (real, required via LLVMSupport) ---
find_package(zstd QUIET)
if(NOT TARGET zstd::libzstd_static)
    message(STATUS "zstd not found, fetching zstd from source")
    FetchContent_Declare(zstd
        GIT_REPOSITORY https://github.com/facebook/zstd.git
        GIT_TAG v1.5.7
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        SOURCE_SUBDIR build/cmake
        UPDATE_DISCONNECTED TRUE)
    set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
    set(ZSTD_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(ZSTD_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
    set(ZSTD_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    set(ZSTD_BUILD_STATIC ON CACHE BOOL "" FORCE)
    if(MSVC)
        set(ZSTD_USE_STATIC_RUNTIME ON CACHE BOOL "" FORCE)
    endif()
    # zstd's CMake force-sets CMAKE_BUILD_TYPE=Release in the cache when it
    # is unset; preserve ours across the sub-configure.
    set(CX_SAVED_BUILD_TYPE "${CMAKE_BUILD_TYPE}")
    FetchContent_MakeAvailable(zstd)
    set(CMAKE_BUILD_TYPE "${CX_SAVED_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
    if(NOT TARGET zstd::libzstd_static)
        add_library(zstd::libzstd_static ALIAS libzstd_static)
    endif()
    # As above: keep our unity-build default out of the zstd subbuild.
    set_target_properties(libzstd_static PROPERTIES UNITY_BUILD OFF)
    set(CMAKE_DISABLE_FIND_PACKAGE_zstd TRUE)
endif()

# --- LibXml2 (dummy suffices; LLVMWindowsManifest is not linked) ---
find_package(LibXml2 QUIET)
if(NOT TARGET LibXml2::LibXml2)
    message(STATUS "LibXml2 not found, defining dummy target (LLVMWindowsManifest is not linked by cx)")
    add_library(LibXml2::LibXml2 INTERFACE IMPORTED)
    set(CX_LIBXML2_DUMMY TRUE)
    set(CMAKE_DISABLE_FIND_PACKAGE_LibXml2 TRUE)
endif()

# --- DIA SDK (dummy suffices; LLVMDebugInfoPDB is not linked) ---
# FindDIASDK.cmake ships inside the LLVM distribution, so it is not on
# CMAKE_MODULE_PATH until find_package(LLVM) runs; there is nothing to
# find beforehand. The dummy satisfies LLVMExports.
if(WIN32 AND NOT TARGET DIASDK::Diaguids)
    message(STATUS "Defining dummy DIASDK::Diaguids target (LLVMDebugInfoPDB is not linked by cx)")
    add_library(DIASDK::Diaguids INTERFACE IMPORTED)
    set(CX_DIASDK_DUMMY TRUE)
    # Skip LLVMConfig's/ClangConfig's own noisy searches; the dummy suffices.
    set(CMAKE_DISABLE_FIND_PACKAGE_DIASDK TRUE)
endif()

find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)
set(LLVM_PACKAGE_VERSION "${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}.${LLVM_VERSION_PATCH}")
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
add_definitions(-DCX_ROOT_DIR="${PROJECT_SOURCE_DIR}")
find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE CX_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)
endif()
if(NOT CX_COMMIT_HASH)
    set(CX_COMMIT_HASH "unknown")
endif()
add_definitions(-DCX_COMMIT_HASH="${CX_COMMIT_HASH}")

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
if(LLVM_TOOLS_BINARY_DIR)
    # Lets the embedded Clang find its subprograms (lld-link) on Windows
    # machines where LLVM's bin directory is not on PATH.
    target_compile_definitions(libcx PRIVATE CX_LLVM_TOOLS_DIR="${LLVM_TOOLS_BINARY_DIR}")
endif()
target_precompile_headers(libcx PRIVATE src/pch.h)
# Clang's headers (notably clang/AST/Stmt.h) befriend AST node classes that
# are only declared later in the same header, without a prior forward
# declaration. In a unity build an earlier TU's `using namespace cx;` is still
# active when these headers are parsed, so the unqualified friend names bind
# to cx's same-named AST classes (cx::IfStmt, ...) instead of declaring
# clang's, and MSVC rejects the resulting private-member accesses (C2248).
# Keep the Clang-consuming TUs standalone.
set_source_files_properties(src/sema/c-import.cpp src/driver/clang.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

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
# The LibXml2/DIASDK dummy targets above only satisfy LLVMExports.cmake; they
# carry no library. Fail fast if a linked component ever needs the real one.
if(CX_LIBXML2_DUMMY AND "LLVMWindowsManifest" IN_LIST LLVM_LIBS)
    message(FATAL_ERROR "LLVMWindowsManifest requires real LibXml2, but the dummy target was used. Install libxml2 and add it to CMAKE_PREFIX_PATH.")
endif()
if(CX_DIASDK_DUMMY AND "LLVMDebugInfoPDB" IN_LIST LLVM_LIBS)
    message(FATAL_ERROR "LLVMDebugInfoPDB requires the real DIA SDK, but the dummy target was used. Install the DIA SDK (Visual Studio) or point MSVC_DIA_SDK_DIR at it.")
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
add_test(NAME website-serve COMMAND python3 "${PROJECT_SOURCE_DIR}/website/test_serve.py")
add_test(NAME bench-corpus-sloc COMMAND python3 "${PROJECT_SOURCE_DIR}/scripts/test_bench_corpus.py")
add_test(NAME lit-config COMMAND python3 "${PROJECT_SOURCE_DIR}/test/test_lit_cfg.py")
add_test(NAME prose COMMAND python3 "${PROJECT_SOURCE_DIR}/website/check_prose.py")
add_test(NAME jit-pins COMMAND python3 "${PROJECT_SOURCE_DIR}/website/check_jit_pins.py")
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
# The C-backend snippet suite shells out to a C compiler per snippet and
# dominates check wall time. CI runs it separately (see build.yml); local
# check runs skip it.
set_tests_properties(docs-snippets-c-backend PROPERTIES LABELS slow)
# Run one test per processor. The suites are CPU-bound, so extra CTest slots
# oversubscribe slower Debug runners and trigger test timeouts. The snippet
# suites cap their inner parallelism above so they don't multiply the slot
# count; examples and LSP are short enough to leave uncapped.
add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --parallel ${CHECK_PARALLEL_JOBS} -LE slow -C $<CONFIG>
    USES_TERMINAL)
add_custom_target(update_snapshots ${CMAKE_COMMAND} -E env UPDATE_SNAPSHOTS=1 cmake --build "${CMAKE_BINARY_DIR}" --target check)
add_dependencies(check cx cx-lsp FileCheck example_embedding test_compile_api test_build_config test_rename test_msvc_link_args test_output_file_name
                     test_string_interner test_mangle test_lexer test_ast_vector)

add_executable(test_string_interner test/ast/test-string-interner.cpp)
target_include_directories(test_string_interner PRIVATE src)
target_link_libraries(test_string_interner libcx)
add_test(NAME string-interner COMMAND test_string_interner)
add_executable(test_mangle test/ast/test-mangle.cpp)
target_include_directories(test_mangle PRIVATE src)
target_link_libraries(test_mangle libcx)
add_test(NAME mangle COMMAND test_mangle)
add_executable(test_ast_vector test/ast/test-ast-vector.cpp)
target_include_directories(test_ast_vector PRIVATE src)
target_link_libraries(test_ast_vector libcx)
add_test(NAME ast-vector COMMAND test_ast_vector)
add_executable(test_lexer test/parser/test-lexer.cpp)
target_include_directories(test_lexer PRIVATE src)
target_link_libraries(test_lexer libcx)
add_test(NAME lexer COMMAND test_lexer)

# Tests for the in-memory compilation API (also used by the WebAssembly build).
add_executable(test_compile_api test/compile-api/test-compile-api.cpp src/wasm/api.cpp)
target_include_directories(test_compile_api PRIVATE src)
target_compile_definitions(test_compile_api PRIVATE CX_TEST_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
target_link_libraries(test_compile_api libcx)
foreach(test_case hello error struct warning dispatch json nomain)
    add_test(NAME compile-api-${test_case} COMMAND test_compile_api ${test_case})
endforeach()

# Tests for build.cx dependency parsing.
add_executable(test_build_config test/build-config/test-build-config.cpp)
target_include_directories(test_build_config PRIVATE src)
target_link_libraries(test_build_config libcx)
add_test(NAME build-config-git-urls COMMAND test_build_config git-urls)
add_test(NAME build-config-missing-url COMMAND test_build_config missing-url)
set_tests_properties(build-config-missing-url PROPERTIES PASS_REGULAR_EXPRESSION "missing required 'url'")
add_test(NAME build-config-duplicate-key COMMAND test_build_config duplicate-key)
set_tests_properties(build-config-duplicate-key PROPERTIES PASS_REGULAR_EXPRESSION "duplicate 'name' key")
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

# Tests for cx::msvcLinkArgs (-L/-l translation for MSVC).
add_executable(test_msvc_link_args test/driver/test-msvc-link-args.cpp)
target_include_directories(test_msvc_link_args PRIVATE src)
target_link_libraries(test_msvc_link_args libcx)
add_test(NAME driver-msvc-link-args COMMAND test_msvc_link_args)

# Tests for cx::withExecutableExtension (directory-build output names on Windows).
add_executable(test_output_file_name test/driver/test-output-file-name.cpp)
target_include_directories(test_output_file_name PRIVATE src)
target_link_libraries(test_output_file_name libcx)
add_test(NAME driver-output-file-name COMMAND test_output_file_name)

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
        -Dcc_path=${CMAKE_C_COMPILER}
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
# Offline builds: point FILECHECK_SOURCE at a local FileCheck.cpp to skip the download below.
set(FILECHECK_SOURCE "" CACHE FILEPATH "Local FileCheck.cpp for your LLVM version, used instead of downloading it")
if(NOT TARGET FileCheck)
    set(FILECHECK_SRC_PATH "llvmorg-${LLVM_PACKAGE_VERSION}/llvm/utils/FileCheck/FileCheck.cpp")
    set(FILECHECK_SRC_URL "https://raw.githubusercontent.com/llvm/llvm-project/${FILECHECK_SRC_PATH}")
    set(FILECHECK_SRC_FILE "${CMAKE_CURRENT_BINARY_DIR}/${FILECHECK_SRC_PATH}")
    if(FILECHECK_SOURCE)
        if(NOT EXISTS "${FILECHECK_SOURCE}" OR IS_DIRECTORY "${FILECHECK_SOURCE}")
            message(FATAL_ERROR "FILECHECK_SOURCE must be an existing FileCheck.cpp file, got '${FILECHECK_SOURCE}'.")
        endif()
        set(FILECHECK_SRC_FILE "${FILECHECK_SOURCE}")
    elseif(NOT EXISTS "${FILECHECK_SRC_FILE}")
        message(STATUS "Downloading LLVM FileCheck source from ${FILECHECK_SRC_URL}")
        file(DOWNLOAD "${FILECHECK_SRC_URL}" "${FILECHECK_SRC_FILE}" TIMEOUT 120 STATUS downloadStatus)
        list(GET downloadStatus 0 downloadCode)
        if(NOT downloadCode EQUAL 0)
            list(GET downloadStatus 1 downloadMessage)
            # Remove partial downloads so re-configuring retries instead of compiling a corrupt file.
            file(REMOVE "${FILECHECK_SRC_FILE}")
            message(FATAL_ERROR "Could not download LLVM FileCheck source from ${FILECHECK_SRC_URL} (${downloadMessage}). Install an LLVM distribution that ships FileCheck, or set -DFILECHECK_SOURCE=/path/to/FileCheck.cpp to use a local copy.")
        endif()
    endif()
    string(REGEX REPLACE "/RTC[^ ]*" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}") # Remove /RTC flags, they conflict with /O2.
    add_executable(FileCheck ${FILECHECK_SRC_FILE})
    llvm_map_components_to_libnames(FILECHECK_NEEDED_LIBS filecheck support)
    target_link_libraries(FileCheck ${FILECHECK_NEEDED_LIBS})
    # Optimize test utilities by default. No /GL: the LTCG object it emits
    # conflicts with the incremental FASTLINK Debug link (LNK2001 on main).
    if(MSVC)
        set_target_properties(FileCheck PROPERTIES COMPILE_FLAGS "${LLVM_DEFINITIONS} /w /O2 /MT")
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
install(DIRECTORY vendor/ DESTINATION "${CMAKE_INSTALL_DATADIR}/cx/vendor")
