# A simple replacement for the official FindPython.cmake distributed with
# later cmakes.
# NOTE(josh): I tried to backboard FindPython.cmake from version 3.22 back to
# version 3.10 (bonic) but the stupid findscript can't find python even when
# `python` is available in the environment. You can't get much dumber than
# that.

find_program(Python_INTERPRETER python3 python REQUIRED)
add_executable(Python::Interpreter IMPORTED)
set_property(TARGET Python::Interpreter
  PROPERTY IMPORTED_LOCATION ${Python_INTERPRETER})

check_call(COMMAND ${Python_INTERPRETER} --version OUTPUT_VARIABLE _python_version_raw OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX MATCH "([0-9]+\\.[0-9]+\\.[0-9]+)" Python_VERSION ${_python_version_raw})

find_program(Python_CONFIG NAMES python-config python3-config)

check_call(
  COMMAND ${Python_CONFIG} --cflags
  OUTPUT_VARIABLE _pydev_cflags_raw
  OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " +" ";" _pydev_cflags_raw "${_pydev_cflags_raw}")
unset(_pydev_cflags)
foreach(cflag ${_pydev_cflags_raw})
  if("${cflag}" MATCHES "-(I|(isystem)|(iquote)).*")
    # pass
  elseif("${cflag}" MATCHES "-isystem.*")
    # pass
  else()
    if("${cflag}" STREQUAL "-Wstrict-prototypes")
      # This flag is only for C so
      list(APPEND _pydev_cflags "$<$<COMPILE_LANGUAGE:C>:${cflag}>")
    elseif(("${cflag}" STREQUAL "-fuse-linker-plugin"
        OR "${cflag}" STREQUAL "-ffat-lto-objects"
        OR "${cflag}" STREQUAL "-flto"
        OR "${cflag}" MATCHES "-specs=.*")
        AND CMAKE_C_COMPILER_ID STREQUAL "Clang")
      # These flags are not supported on clang. -flto is supported but it
      # must be used both at compile and link time and I'm sure the semantics
      # are different because we are mixing GCC and clang built objects.
    else()
      list(APPEND _pydev_cflags ${cflag})
    endif()

  endif()
endforeach()

check_call(
  COMMAND ${Python_CONFIG} --includes
  OUTPUT_VARIABLE _pydev_includes
  OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " +" ";" _pydev_includes "${_pydev_includes}")
string(REGEX REPLACE "-I" "" _pydev_includes "${_pydev_includes}")

check_call(
  COMMAND ${Python_CONFIG} --ldflags
  OUTPUT_VARIABLE _pydev_ldflags_raw
  OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " +" ";" _pydev_ldflags_raw "${_pydev_ldflags_raw}")
unset(_pydev_libdirs)
unset(_pydev_ldflags)
foreach(ldflag ${_pydev_ldflags_raw})
  if("${ldflag}" MATCHES "-L.*")
    list(APPEND _pydev_libdirs "${ldflag}")
  elseif("${ldflag}" MATCHES "-l.*")
    # pass
  else()
    list(APPEND _pydev_ldflags "${ldflag}")
  endif()
endforeach()

check_call(
  COMMAND ${Python_CONFIG} --libs
  OUTPUT_VARIABLE _pydev_libs
  OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " +" ";" _pydev_libs "${_pydev_libs}")

if(Python_VERSION VERSION_GREATER 3.7.999)
  check_call(
    COMMAND ${Python_CONFIG} --libs --embed
    OUTPUT_VARIABLE _pydev_embed_raw
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  unset(_pydev_embed)
  foreach(flag ${_pydev_embed_raw})
    if("${flag}" IN_LIST _pydev_libs)
      # pass
    else()
      list(APPEND _pydev_embed ${flag})
    endif()
  endforeach()
else()
endif()

add_library(Python.Module INTERFACE)
target_compile_options(Python.Module INTERFACE ${_pydev_cflags})
target_include_directories(Python.Module INTERFACE ${_pydev_includes})
if(CMAKE_VERSION VERSION_GREATER 3.12.999)
target_link_options(Python.Module INTERFACE ${_pydev_ldflags})
else()
target_link_libraries(Python.Module INTERFACE ${_pydev_ldflags})
endif()
target_link_libraries(Python.Module INTERFACE ${_pydev_libs})
add_library(Python::Module ALIAS Python.Module)

add_library(Python.Embed INTERFACE)
target_link_libraries(Python.Embed INTERFACE Python.Module ${_pydev_embed})
add_library(Python::Embed ALIAS Python.Embed)

check_call(
  COMMAND ${Python_CONFIG} --extension-suffix
  OUTPUT_VARIABLE Python_EXTENSION_SUFFIX
  OUTPUT_STRIP_TRAILING_WHITESPACE)
