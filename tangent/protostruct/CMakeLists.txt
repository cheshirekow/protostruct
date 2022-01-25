find_tangent_package(argue CONFIG REQUIRED)
find_tangent_package(libtangent-util CONFIG REQUIRED)

if(NOT Clang_FOUND)
  message(FATAL_ERROR "libclang not found")
endif()

# ========================================
# Generate protobuf descriptor extensions
# ========================================

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/descriptor_extensions.pb.h
         ${CMAKE_CURRENT_BINARY_DIR}/descriptor_extensions.pb.cc
         # TODO(josh): figure out how to deal with python path so that this can
         # go in the binary tree
         ${CMAKE_CURRENT_BINARY_DIR}/descriptor_extensions_pb2.py
  DEPENDS descriptor_extensions.proto
  COMMAND
    protoc --proto_path=${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/descriptor_extensions.proto
    --cpp_out=${CMAKE_BINARY_DIR} --python_out=${CMAKE_BINARY_DIR})

# =================================
# zip up protostruct python package
# =================================

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pyzip_exe_body.zip
         ${CMAKE_CURRENT_BINARY_DIR}/empty.cc
  DEPENDS gen.py
          make_pyzip.py
          templates/XXX-recon.h.jinja2
          templates/XXX-simple.h.jinja2
          templates/XXX-simple.cc.jinja2
          templates/XXX.cereal.h.jinja2
          templates/XXX.pbwire.c.jinja2
          templates/XXX.pbwire.h.jinja2
          templates/XXX.pb2c.cc.jinja2
          templates/XXX.pb2c.h.jinja2
          templates/XXX.proto.jinja2
          ${CMAKE_CURRENT_BINARY_DIR}/descriptor_extensions_pb2.py
  COMMAND
    $<TARGET_FILE:Python::Interpreter> -Bm tangent.protostruct.make_pyzip -o
    ${CMAKE_CURRENT_BINARY_DIR}/pyzip_exe_body.zip -m
    ${CMAKE_CURRENT_SOURCE_DIR}/gen.py -b ${CMAKE_BINARY_DIR}
  # NOTE(josh): cmake really sucks! There is no way to add a file-level
  # dependency between the protostruct `add_executable` and this .zip file... so
  # we have to generate a stub .cc file just so we can depend on it in the
  # protostruct rule.
  COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/empty.cc
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_custom_target(protostruct-pyzip_exe_body
                  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/pyzip_exe_body.zip)

# ======================
# Protostruct elf-header
# ======================

get_version_from_header(protostruct-main.cc TANGENT_PROTOSTRUCT_VERSION)

cc_binary(
  protostruct
  SRCS protostruct-main.cc
       ${CMAKE_CURRENT_BINARY_DIR}/descriptor_extensions.pb.cc
       ${CMAKE_CURRENT_BINARY_DIR}/empty.cc
  DEPS argue tangent::util libclang Python::Embed
  PKGDEPS libglog protobuf
  PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(protostruct PUBLIC ${CLANG_INCLUDE_DIRS}
                                              ${CMAKE_BINARY_DIR})

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/empty.cc
                            PROPERTIES GENERATED TRUE)

# Strip symbols from release binary and concat the data trailer to create the
# final binary
add_custom_command(
  TARGET protostruct
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:protostruct>
          ${CMAKE_BINARY_DIR}/protostruct.debug
  COMMAND ${CMAKE_STRIP} -g $<TARGET_FILE:protostruct>
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:protostruct>
          $<TARGET_FILE:protostruct>.hdr
  COMMAND
    cat $<TARGET_FILE:protostruct>.hdr
    ${CMAKE_CURRENT_BINARY_DIR}/pyzip_exe_body.zip > $<TARGET_FILE:protostruct>
  COMMAND chmod +x $<TARGET_FILE:protostruct>)

# ========================
# Protostruct installation
# ========================

# NOTE(josh): If CMAKE_LIBRARY_ARCHITECTURE is defined (it is on debian) then
# ${CMAKE_INSTALL_LIBDIR} will end with it's value. We don't want to install
# there because protostruct is a cmake utility package, not a package of
# software to be built into the project.
set(_package_location lib/cmake/protostruct)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/protostruct-config-version.cmake"
  VERSION "${TANGENT_PROTOSTRUCT_VERSION}"
  COMPATIBILITY AnyNewerVersion)

configure_package_config_file(
  protostruct-config.cmake ${CMAKE_CURRENT_BINARY_DIR}/protostruct-config.cmake
  PATH_VARS CMAKE_INSTALL_BINDIR
  INSTALL_DESTINATION ${_package_location})

install(TARGETS protostruct RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/protostruct-config.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/protostruct-config-version.cmake"
        DESTINATION ${_package_location})

# cmake-format: off
set(_code
    "execute_process("
    "  COMMAND python -B ${TANGENT_TOOLING}/install_debug_symbols.py "
    "    --bin-path ${CMAKE_BINARY_DIR}/protostruct-exe.debug"
    "    --lib-dir ${CMAKE_INSTALL_PREFIX}/lib"
    "  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}"
    ")")
# cmake-format: on
string(REPLACE ";" "\n" _code "${_code}")
install(CODE ${_code})

# =========
# libpbwire
# =========

set(_headers pbwire.h)
set(_sources pbwire.cc)
get_version_from_header(pbwire.h TANGENT_PBWIRE_VERSION)

cc_library(
  pbwire STATIC
  SRCS ${_headers} ${_sources}
  PROPERTIES ARCHIVE_OUTPUT_NAME tangent-pbwire
             EXPORT_NAME static
             INTERFACE_INCLUDE_DIRECTORIES "$<INSTALL_INTERFACE:include>")
add_library(pbwire::static ALIAS pbwire)

cc_library(
  pbwire-shared SHARED
  SRCS ${_headers} ${_sources}
  PROPERTIES EXPORT_NAME shared
             LIBRARY_OUTPUT_NAME tangent-pbwire
             VERSION "${TANGENT_PBWIRE_API_VERSION}"
             SOVERSION "${TANGENT_PBWIRE_SO_VERSION}"
             INTERFACE_INCLUDE_DIRECTORIES "$<INSTALL_INTERFACE:include>")
add_library(pbwire::shared ALIAS pbwire-shared)

cc_test(
  pbwire-test
  SRCS pbwire-test.cc
  DEPS pbwire gtest gtest_main)

# ======================
# libpbwire installation
# ======================

configure_file(libpbwire.pc ${CMAKE_CURRENT_BINARY_DIR}/libpbwire.pc @ONLY)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/pbwire-config-version.cmake"
  VERSION "${TANGENT_PBWIRE_VERSION}"
  COMPATIBILITY AnyNewerVersion)

set(_package_location ${CMAKE_INSTALL_LIBDIR}/cmake/pbwire)

configure_package_config_file(
  pbwire-config.cmake ${CMAKE_CURRENT_BINARY_DIR}/pbwire-config.cmake
  PATH_VARS CMAKE_INSTALL_BINDIR
  INSTALL_DESTINATION ${_package_location})

install(
  TARGETS pbwire pbwire-shared
  EXPORT pbwire-targets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")

install(FILES ${_headers}
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/tangent/pbwire")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libpbwire.pc
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

export(
  EXPORT pbwire-targets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/pbwire-targets.cmake"
  NAMESPACE "pbwire::")

install(
  EXPORT pbwire-targets
  FILE pbwire-targets.cmake
  NAMESPACE "pbwire::"
  DESTINATION ${_package_location})

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/pbwire-config.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/pbwire-config-version.cmake"
        DESTINATION ${_package_location})

# =================
# Protostruct tests
# =================

# compile .h/.proto -> .pb3
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_messages.pb3
         ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages.proto
  DEPENDS protostruct ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages.h
          ${CMAKE_CURRENT_SOURCE_DIR}/test/test_in.proto
          ${CMAKE_CURRENT_BINARY_DIR}/descriptor_extensions_pb2.py
  COMMAND
    # cmake-format: off
    $<TARGET_FILE:protostruct>
    --proto-path ${CMAKE_CURRENT_SOURCE_DIR}/test
    compile tangent/protostruct/test/test_messages.h
    --proto-in test_in.proto
    --proto-out ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages.proto
    --pb3-out ${CMAKE_CURRENT_BINARY_DIR}/test_messages.pb3
    -- -I ${CMAKE_SOURCE_DIR}
    # cmake-format: on
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# generate code from .pb3
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages-recon.h
         ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages-simple.h
         ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages-simple.cc
         ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages.cereal.h
         ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages.pbwire.h
         ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages.pbwire.c
         ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages.pb2c.h
         ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages.pb2c.cc
  DEPENDS protostruct ${CMAKE_CURRENT_BINARY_DIR}/test_messages.pb3
          ${CMAKE_CURRENT_BINARY_DIR}/descriptor_extensions_pb2.py
  COMMAND
    # cmake-format: off
    $<TARGET_FILE:protostruct> generate
    ${CMAKE_CURRENT_BINARY_DIR}/test_messages.pb3
    --cpp-root ${CMAKE_SOURCE_DIR}
    cpp-simple cereal pbwire pb2c recon
    # cmake-format: on
  COMMAND
    ${_clang_format} -i -style=File
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages-recon.h
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages-simple.h
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages-simple.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages.cereal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages.pbwire.h
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages.pbwire.c
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages.pb2c.h
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages.pb2c.cc
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# generate C/C++ bindings from .proto
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test/test_messages.pb.h
         ${CMAKE_CURRENT_BINARY_DIR}/test/test_messages.pb.cc
  DEPENDS test/test_messages.proto
  COMMAND
    protoc --proto_path=${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages.proto
    --cpp_out=${CMAKE_BINARY_DIR})

cc_library(
  test-messages
  SRCS ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages.cereal.h
       ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages.pbwire.h
       ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages.pbwire.c
       ${CMAKE_CURRENT_BINARY_DIR}/test/test_messages.pb.h
       ${CMAKE_CURRENT_BINARY_DIR}/test/test_messages.pb.cc
       ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages.pb2c.h
       ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages.pb2c.cc
       ${CMAKE_CURRENT_BINARY_DIR}/descriptor_extensions.pb.h
       ${CMAKE_CURRENT_BINARY_DIR}/descriptor_extensions.pb.cc
  DEPS cereal pbwire)
target_include_directories(test-messages PUBLIC ${CMAKE_BINARY_DIR})

cc_library(
  test-messages-simple
  SRCS ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages-simple.h
       ${CMAKE_CURRENT_SOURCE_DIR}/test/test_messages-simple.cc
  DEPS tjson tjson-cpp)
target_include_directories(test-messages PUBLIC ${CMAKE_BINARY_DIR})

cc_test(
  protostruct-devtest3
  SRCS devtest.cc
  DEPS cereal gtest gtest_main test-messages
  PKGDEPS protobuf)
