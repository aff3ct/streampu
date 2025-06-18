# - Try to find HELGRIND

## Check for pkg-config
#find_package(PkgConfig)

# Check for HELGRIND
message(STATUS "Checking for HELGRIND")

pkg_check_modules(HELGRIND valgrind)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HELGRIND REQUIRED_VARS HELGRIND_INCLUDE_DIRS)

