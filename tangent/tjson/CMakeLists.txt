get_version_from_header(tjson.h TJSON_VERSION)

set(_headers emit.h parse.h tjson.h)
set(_sources emit.c parse.c tjson.c)

cc_library(
  tjson STATIC
  SRCS ${_sources}
  PROPERTIES ARCHIVE_OUTPUT_NAME tjson
             EXPORT_NAME static
             INTERFACE_INCLUDE_DIRECTORIES "$<INSTALL_INTERFACE:include>")
add_library(tjson::static ALIAS tjson)

cc_library(
  tjson-shared SHARED
  SRCS ${_sources}
  PROPERTIES LIBRARY_OUTPUT_NAME tjson
             EXPORT_NAME shared
             VERSION "${TANGENT_JSON_API_VERSION}"
             SOVERSION "${TANGENT_JSON_SO_VERSION}"
             INTERFACE_INCLUDE_DIRECTORIES "$<INSTALL_INTERFACE:include>")
add_library(tjson::shared ALIAS tjson-shared)

set(_headers cpputil.h)
set(_sources cpputil.cc)

cc_library(
  tjson-cpp STATIC
  DEPS util tjson::static
  SRCS ${_sources}
  PROPERTIES ARCHIVE_OUTPUT_NAME tjson-cpp
             EXPORT_NAME static
             INTERFACE_INCLUDE_DIRECTORIES "$<INSTALL_INTERFACE:include>")
add_library(tjson::cpp-static ALIAS tjson-cpp)

cc_library(
  tjson-cpp-shared SHARED
  DEPS util-shared tjson::shared
  SRCS ${_sources}
  PROPERTIES LIBRARY_OUTPUT_NAME tjson-cpp
             EXPORT_NAME shared
             VERSION "${TANGENT_JSON_API_VERSION}"
             SOVERSION "${TANGENT_JSON_SO_VERSION}"
             INTERFACE_INCLUDE_DIRECTORIES "$<INSTALL_INTERFACE:include>")
add_library(tjson::cpp-shared ALIAS tjson-cpp-shared)

cc_binary(
  tjson-exe
  SRCS main.cc
  DEPS argue tjson
  PROPERTIES OUTPUT_NAME tjson)

glob_subdirs()

configure_file(libtjson.pc ${CMAKE_CURRENT_BINARY_DIR}/libtjson.pc @ONLY)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/tjson-config-version.cmake"
  VERSION "${TJSON_VERSION}"
  COMPATIBILITY AnyNewerVersion)

set(_package_location ${CMAKE_INSTALL_LIBDIR}/cmake/tjson)

configure_package_config_file(
  tjson-config.cmake ${CMAKE_CURRENT_BINARY_DIR}/tjson-config.cmake
  PATH_VARS CMAKE_INSTALL_BINDIR
  INSTALL_DESTINATION ${_package_location})

install(
  TARGETS tjson tjson-shared
  EXPORT tjson-targets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")

install(FILES ${_headers} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/tjson")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libtjson.pc
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

export(
  EXPORT tjson-targets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/tjson-targets.cmake"
  NAMESPACE "tjson::")

install(
  EXPORT tjson-targets
  FILE tjson-targets.cmake
  NAMESPACE "tjson::"
  DESTINATION ${_package_location})

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/tjson-config.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/tjson-config-version.cmake"
        DESTINATION ${_package_location})
