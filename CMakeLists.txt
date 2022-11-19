cmake_minimum_required(VERSION 3.18)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project("libPSI" VERSION 1.0.0)

# sets some variables
include(cmake/preamble.cmake)

# sets build option
include(cmake/buildOptions.cmake)

# find all the dependancies.
include(cmake/findDependancies.cmake)

add_definitions(-DSOLUTION_DIR=\"${CMAKE_SOURCE_DIR}\")

# ######################################################################################################################
# Build libPSI                 #
# ######################################################################################################################

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
  if(ENABLE_BUILD_WITH_TIME_TRACE)
    target_compile_options(project_options INTERFACE -ftime-trace)
  endif()
endif()

add_library(project_warnings INTERFACE)

# enable cache system
include(cmake/cache.cmake)

# Add linker configuration include(cmake/Linker.cmake) configure_linker(project_options)

# standard compiler warnings
include(cmake/compileWarnings.cmake)
set_project_warnings(project_warnings)

# sanitizer options if supported by compiler
include(cmake/sanitizers.cmake)
enable_sanitizers(project_options)

# allow for static analysis options
include(cmake/staticAnalyzers.cmake)

add_subdirectory(libPSI)
add_subdirectory(libPSI_Tests)
add_subdirectory(frontend)

# setup the install
include(cmake/install.cmake)
