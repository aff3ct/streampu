# - Try to find HELGRIND (Valgrind annotations)
# Only works on Unix-like systems

if(NOT UNIX)
  message(STATUS "HELGRIND not supported on non-Unix platforms.")
  return()
endif()

# Try to use pkg-config
find_package(PkgConfig QUIET)

if(PKG_CONFIG_FOUND)
  pkg_check_modules(HELGRIND valgrind)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HELGRIND REQUIRED_VARS HELGRIND_INCLUDE_DIRS)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HELGRIND REQUIRED_VARS HELGRIND_INCLUDE_DIRS)

