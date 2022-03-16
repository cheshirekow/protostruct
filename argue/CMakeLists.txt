find_tangent_package(libtangent-util CONFIG REQUIRED)

set(_headers
    argue.h
    action.h
    action.tcc
    exception.h
    glog.h
    json_dump.h
    keywords.h
    keywords.tcc
    kwargs.h
    kwargs.tcc
    parse.h
    parse.tcc
    parser.h
    parser.tcc
    storage_model.h
    storage_model.tcc
    util.h)

set(_sources
    action.cc
    exception.cc
    json_dump.cc
    kwargs.cc
    parse.cc
    parser.cc
    glog.cc)

get_version_from_header(argue.h ARGUE_VERSION)

cc_library(
  argue STATIC
  SRCS ${_sources}
  DEPS fmt::fmt tangent::util
  PKGDEPS libglog
  PROPERTIES EXPORT_NAME static
             INTERFACE_INCLUDE_DIRECTORIES "$<INSTALL_INTERFACE:include>")
add_library(argue::static ALIAS argue)

cc_library(
  argue-shared SHARED
  SRCS ${_sources}
  DEPS fmt::fmt tangent::util-shared
  PKGDEPS libglog
  PROPERTIES LIBRARY_OUTPUT_NAME argue
             VERSION "${ARGUE_API_VERSION}"
             SOVERSION "${ARGUE_SO_VERSION}"
             EXPORT_NAME shared
             INTERFACE_INCLUDE_DIRECTORIES "$<INSTALL_INTERFACE:include>")
add_library(argue::shared ALIAS argue-shared)

cc_binary(
  argue-can-complete
  SRCS can_complete.cc
  DEPS argue loki
  PKGDEPS libelf
  PROPERTIES EXPORT_NAME can_complete)
add_executable(argue::can-complete ALIAS argue-can-complete)

configure_file(libargue.pc ${CMAKE_CURRENT_BINARY_DIR}/libargue.pc @ONLY)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/argue-config-version.cmake"
  VERSION "${ARGUE_VERSION}"
  COMPATIBILITY AnyNewerVersion)

set(_package_location ${CMAKE_INSTALL_LIBDIR}/cmake/argue)

configure_package_config_file(
  argue-config.cmake ${CMAKE_CURRENT_BINARY_DIR}/argue-config.cmake
  PATH_VARS CMAKE_INSTALL_BINDIR
  INSTALL_DESTINATION ${_package_location})

add_subdirectory(doc)
add_subdirectory(examples)
add_subdirectory(test)

install(
  TARGETS argue argue-shared argue-can-complete
  EXPORT argue-targets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")

install(FILES ${_headers} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/argue")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libargue.pc
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

install(FILES bash_completion.d/argue-argcomplete
        DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/bash_completion.d")

export(
  EXPORT argue-targets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/argue-targets.cmake"
  NAMESPACE "argue::")

install(
  EXPORT argue-targets
  FILE argue-targets.cmake
  NAMESPACE "argue::"
  DESTINATION ${_package_location})

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/argue-config.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/argue-config-version.cmake"
        DESTINATION ${_package_location})
