set(_stagefiles)
set(_stage ${CMAKE_CURRENT_BINARY_DIR}/stage)

configure_file(conf.py ${_stage}/conf.py @ONLY)
list(APPEND _stagefiles ${_stage}/conf.py)

stage_files(
  LIST _stagefiles
  STAGE ${_stage}
  SOURCEDIR ${CMAKE_CURRENT_SOURCE_DIR}
  FILES index.rst changelog.rst design.rst ../README.rst:README.rst)

stage_files(
  LIST _stagefiles
  STAGE ${_stage}
  SOURCEDIR ${CMAKE_SOURCE_DIR}/doc
  FILES conf.py:conf_common.py sphinx-static/css/cheshire_theme.css)

add_custom_target(stagefiles-protostruct-doc DEPENDS ${_stagefiles})

set(_sphinx_manifest ${_stagefiles})
list(SORT _sphinx_manifest)

# NOTE(josh): cmake 3.10 does not have list(JOIN)
string(REPLACE ";" "\n" _manifest_text "${_sphinx_manifest}")

# NOTE(josh): don't use file(WRITE) because that wont create a dependency on the
# configure step to rerun cmake if the file is missing
configure_file(MANIFEST.in ${CMAKE_CURRENT_BINARY_DIR}/MANIFEST
               NEWLINE_STYLE UNIX)

add_custom_target(
  clean-protostruct-sphinx-stage
  DEPENDS ${_sphinx_manifest} ${CMAKE_CURRENT_BINARY_DIR}/MANIFEST
  COMMAND python -B ${TANGENT_TOOLING}/clean_stage.py
          ${CMAKE_CURRENT_BINARY_DIR}/MANIFEST ${_stage}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Cleaning protostruct/doc/stage")

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
  COMMAND env PYTHONPATH=${CMAKE_SOURCE_DIR} sphinx-build -M html ${_stage}
          ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND touch ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
  DEPENDS clean-protostruct-sphinx-stage ${_sphinx_manifest}
          ${CMAKE_CURRENT_BINARY_DIR}/MANIFEST ${CMAKE_SOURCE_DIR}/doc/conf.py
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Building sphinx documentation for protostruct")

add_custom_target(doc-protostruct
                  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html/index.html)

add_custom_target(
  showdoc-protostruct
  COMMAND xdg-open ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html/index.html)

add_dependencies(doc doc-protostruct)
set_property(GLOBAL APPEND PROPERTY global_doc_files ${_sphinx_manifest})

set_property(
  DIRECTORY
  APPEND
  PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${_stagefiles})
