cmake_minimum_required(VERSION 3.20...3.22)

# Read version from version.hpp
file(READ "${CMAKE_CURRENT_LIST_DIR}/include/tao/pegtl/version.hpp" version_hpp_data)
string(REGEX MATCH "#define TAO_PEGTL_VERSION \"([^\"]+)\"" _ ${version_hpp_data})
set(PEGTL_VERSION "${CMAKE_MATCH_1}")

project(pegtl VERSION ${PEGTL_VERSION} DESCRIPTION "The Parsing Expression Grammar Template Library" HOMEPAGE_URL https://github.com/taocpp/PEGTL LANGUAGES CXX)

set(PEGTL_IS_MAIN_PROJECT OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(PEGTL_IS_MAIN_PROJECT ON)
endif()

if(PEGTL_HAS_PARENT)
  # Multiple versions of PEGTL can't co-exist
  if(NOT ${PROJECT_NAME}_VERSION STREQUAL ${PROJECT_VERSION})
    message(FATAL_ERROR "Multiple mismatched PEGTL versions")
  endif()

  # Only include if this is the first include
  if(NOT ${PROJECT_NAME}_DIR STREQUAL "${PROJECT_BINARY_DIR}")
    return()
  endif()
endif()

# Keep track of pegtl version
set(${PROJECT_NAME}_FOUND TRUE CACHE BOOL "" FORCE)
set(${PROJECT_NAME}_VERSION "${PROJECT_VERSION}" CACHE STRING "" FORCE)
set(${PROJECT_NAME}_DIR "${PROJECT_BINARY_DIR}" CACHE PATH "" FORCE)

mark_as_advanced(${PROJECT_NAME}_FOUND)
mark_as_advanced(${PROJECT_NAME}_VERSION)
mark_as_advanced(${PROJECT_NAME}_DIR)

# installation directories
set(PEGTL_INSTALL_INCLUDE_DIR "include" CACHE STRING "The installation include directory")
set(PEGTL_INSTALL_DOC_DIR "share/doc/tao/pegtl" CACHE STRING "The installation doc directory")
set(PEGTL_INSTALL_CMAKE_DIR "share/pegtl/cmake" CACHE STRING "The installation cmake directory")
set(PEGTL_INSTALL_PKGCONFIG_DIR "share/pkgconfig" CACHE STRING "The installation pkgconfig directory")

# define a header-only library
add_library(pegtl INTERFACE)
add_library(taocpp::pegtl ALIAS pegtl)
target_include_directories(pegtl INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${PEGTL_INSTALL_INCLUDE_DIR}>
)

# require C++23
target_compile_features(pegtl INTERFACE cxx_std_23)

# testing
option(PEGTL_BUILD_TESTS "Build test programs" ${PEGTL_IS_MAIN_PROJECT})
if(PEGTL_BUILD_TESTS)
  enable_testing()
  add_subdirectory(src/test)
  option(PEGTL_BUILD_COMPILE_TESTS "Enable compile accept/reject tests" ON)
  if(PEGTL_BUILD_COMPILE_TESTS)
    add_subdirectory(src/compile)
  endif()
endif()

# examples
option(PEGTL_BUILD_EXAMPLES "Build example programs" ${PEGTL_IS_MAIN_PROJECT})
if(PEGTL_BUILD_EXAMPLES)
  add_subdirectory(src/example)
endif()

# linting
option(PEGTL_BUILD_LINT "Build lint programs" OFF)
if(PEGTL_BUILD_LINT)
  add_subdirectory(src/lint)
endif()

# Make package findable by CMake
configure_file(.cmake/pegtl-config.cmake.in pegtl-config.cmake @ONLY)

# Make package findable by pkg-config
configure_file(.pkg-config/pegtl.pc.in pegtl.pc @ONLY)

# Ignore pointer width differences since this is a header-only library
unset(CMAKE_SIZEOF_VOID_P)

# Enable version checks in find_package
include(CMakePackageConfigHelpers)
write_basic_package_version_file(pegtl-config-version.cmake COMPATIBILITY SameMajorVersion)

# install and export target
install(TARGETS pegtl EXPORT pegtl-targets)

install(EXPORT pegtl-targets
  FILE pegtl-config.cmake
  NAMESPACE taocpp::
  DESTINATION ${PEGTL_INSTALL_CMAKE_DIR}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pegtl-config-version.cmake DESTINATION ${PEGTL_INSTALL_CMAKE_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pegtl.pc DESTINATION ${PEGTL_INSTALL_PKGCONFIG_DIR})
install(DIRECTORY include/ DESTINATION ${PEGTL_INSTALL_INCLUDE_DIR})
install(FILES LICENSE_1_0.txt DESTINATION ${PEGTL_INSTALL_DOC_DIR})

export(EXPORT pegtl-targets
  FILE ${pegtl_BINARY_DIR}/pegtl-targets.cmake
  NAMESPACE taocpp::
)
