@PACKAGE_INIT@
set(_bindir @PACKAGE_CMAKE_INSTALL_BINDIR@)

include(${CMAKE_CURRENT_LIST_DIR}/tjson-targets.cmake)
check_required_components(tjson)

