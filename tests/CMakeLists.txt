# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2019-2021, Intel Corporation

#
# tests/CMakeLists.txt - prepares all tests; it specifies which tests are enabled based on options
#	and available programs. Build tests with 'make tests' command, execute them using 'ctest'.
#
include(cmake/functions.cmake)

# ----------------------------------------------------------------- #
## Setup
# ----------------------------------------------------------------- #
add_custom_target(tests)

set(CXX_STANDARD 17 CACHE STRING "C++ language standard")

set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD ${CXX_STANDARD})

# XXX: valgrind required only for tests? or in top-level CMake?
# # Find valgrind
# if(PKG_CONFIG_FOUND)
#	pkg_check_modules(VALGRIND QUIET valgrind)
# else()
#	find_package(VALGRIND QUIET)
# endif()

if(NOT VALGRIND_FOUND AND TESTS_USE_VALGRIND)
	message(FATAL_ERROR "Valgrind not found, but flag TESTS_USE_VALGRIND was set.")
elseif(NOT VALGRIND_FOUND)
	message(WARNING "Valgrind not found. Valgrind tests will not be performed.")
elseif(VALGRIND_FOUND)
	message(STATUS "Found Valgrind in '${VALGRIND_LIBRARY_DIRS}' (version: ${VALGRIND_VERSION})")
endif()

# find_pmemcheck()
find_libunwind()
# find_pmreorder()
# find_pmempool()
# find_gdb()

if(TESTS_RAPIDCHECK)
	find_package(rapidcheck REQUIRED)
endif()

# Add checks when DEVELOPER_MODE is ON
add_cppstyle(tests ${TESTS_ROOT_DIR}/*/*.[ch]
		   ${TESTS_ROOT_DIR}/*/*.[ch]pp)
add_check_whitespace(tests ${TESTS_ROOT_DIR}/*.*
		${CMAKE_CURRENT_SOURCE_DIR}/*/*.*)

add_library(test_backtrace STATIC common/test_backtrace.c)
if(LIBUNWIND_FOUND)
	target_compile_definitions(test_backtrace PUBLIC USE_LIBUNWIND=1)
endif()

# XXX: add libpmemset dependency and fix 'check_is_pmem.cpp'
#		or use libpmem library? (fix findLIBPMEM2)
# add_executable(check_is_pmem common/check_is_pmem.cpp)
# target_link_libraries(check_is_pmem ${LIBPMEM_LIBRARIES})

if(COVERAGE AND TESTS_USE_VALGRIND)
	message(STATUS "This is the Coverage build, skipping Valgrind tests")
endif()

# ----------------------------------------------------------------- #
## Common tests
# ----------------------------------------------------------------- #
build_test(stream_from_map api_c/stream_from_map.c)
add_test_generic(NAME stream_from_map TRACERS none)

# XXX: remove or re-write this test, when real TC comes up
build_test(cpp_linkage api_c/stream_from_map.cpp)
add_test_generic(NAME cpp_linkage TRACERS none)

if(TESTS_RAPIDCHECK)
	build_test_ext(NAME util_popcount SRC_FILES unittest/popcount.cpp LIBS rapidcheck)
	add_test_generic(NAME util_popcount TRACERS none)
endif()
