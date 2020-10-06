@PACKAGE_INIT@
set(protostruct_BINDIR @PACKAGE_CMAKE_INSTALL_BINDIR@)

include(${CMAKE_CURRENT_LIST_DIR}/protostruct-targets.cmake)
check_required_components(protostruct)

function(_protostruct_error)
  message(FATAL_protostruct_error " protostruct_gen(): " ${ARGN})
endfunction()

function(protostruct_gen)
  set(_args_STEP "both")
  set(onevalue_args STEP OUTFILE PROTO_OUT CPP_OUT)
  set(multivalue_args PROTO_PATH CLANG_FLAGS)
  cmake_parse_arguments(_args "" "${onevalue_args}" "${multivalue_args}"
                        ${ARGN})

  set(_protostruct_path "${protostruct_BINDIR}/protostruct")
  if(NOT EXISTS ${_protostruct_path})
    _protostruct_error(
      "protostruct not found at expected location: ${protostruct_BINDIR}")
  endif()

  if(_args_UNPARSED_ARGUMENTS)
    _protostruct_error("too-many arguments to protostruct_gen():"
                       " ${_args_UNPARSED_ARGUMENTS}")
  endif()

  if(NOT _args_OUTFILE)
    set(_args_OUTFILE ${CMAKE_CURRENT_BINARY_DIR}/foo.pb3)
  endif()

  add_custom_command(
    OUTPUT foo.pbwire.h
           foo.pbwire.c
           foo.pb2c.h
           foo.pb2c.cc
           foo.proto
    COMMAND "${_protostruct_path}"
      --step ${_args_STEP}
      --outfile ${_args_OUTFILE}
      --proto-out ${_args_PROTO_OUT}
      --cpp-out ${_args_CPP_OUT})
endfunction()
