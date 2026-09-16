cmake_minimum_required(VERSION 3.19)

project(i18n_keyval VERSION 0.8.0 LANGUAGES CXX)

option(I18N_KEYVAL_BUILD_TESTS "Build tests" OFF)
option(I18N_KEYVAL_EXCEPTIONS "If off, compile with -fno-exceptions and print i18n exception messages" ON)

SET(
    I18N_KEYVAL_SOURCES
    src/i18n_keyval/core/api.cpp
    src/i18n_keyval/core/registry.cpp
    src/i18n_keyval/core/common.cpp
    src/i18n_keyval/util/file.cpp
    src/i18n_keyval/util/locale.cpp
)

add_library(i18n_keyval ${I18N_KEYVAL_SOURCES})
add_library(i18n_keyval::i18n_keyval ALIAS i18n_keyval)

target_compile_features(i18n_keyval PUBLIC cxx_std_20)

# PRIVATE: these are this library's own build settings (warning level,
# optimization level, and -- in the no-exceptions build mode --
# -fno-exceptions), not part of its interface. A consumer's own targets
# have their own opinions about warnings and optimization, and forcing
# -fno-exceptions PUBLIC would disable exception support in the
# *consumer's* code too, for every target that merely links this
# library, regardless of whether that consumer's own code uses
# exceptions at all. Nothing in this library's public headers requires
# the consumer to match these settings: I18N_KEYVAL_EXCEPTIONS only
# changes whether throw_i18n_exception() (compiled into this library)
# throws or prints, and the bundled translators already avoid throwing
# across the library/consumer boundary in ways that would require the
# consumer to be compiled the same way (see e.g. the nlohmann_json
# translator's non-throwing parse()).
if (MSVC)
    target_compile_options(i18n_keyval PRIVATE /W4)
else()
    target_compile_options(i18n_keyval PRIVATE -Wall -Wextra -O3)
endif()

if (I18N_KEYVAL_EXCEPTIONS)
    # PUBLIC: I18N_TRY/I18N_CATCH/I18N_THROW in core/common.hpp are
    # part of the public API (a custom translator could use them), and
    # their expansion depends on whether I18N_EXCEPTIONS is defined at
    # the *consumer's* include site, not just inside this library's own
    # compiled sources.
    target_compile_definitions(i18n_keyval PUBLIC I18N_EXCEPTIONS)
endif()

if (NOT I18N_KEYVAL_EXCEPTIONS)
    target_compile_options(i18n_keyval PRIVATE -fno-exceptions)
endif()

target_include_directories(i18n_keyval PUBLIC "src")

if (I18N_KEYVAL_BUILD_TESTS)
    include(CTest)
    enable_testing()
    message(STATUS "Building tests")
    enable_testing()
    add_subdirectory("test")
    add_test(NAME i18n_keyval_tests COMMAND i18n_keyval_tests)
endif()
