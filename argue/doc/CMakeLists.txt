#
# Generated source bits for sphinx documentation
#
set(_copyfiles)
set(_genbits)
set(_stage ${CMAKE_CURRENT_BINARY_DIR}/stage)

add_custom_command(
  OUTPUT ${_stage}/bits
  COMMAND ${CMAKE_COMMAND} -E make_directory ${_stage}/bits
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

configure_file(conf.py ${_stage}/conf.py @ONLY)
list(APPEND _copyfiles ${_stage}/conf.py)

# copy sourcefiles from the sourcetree into the build tree so that they're all
# accessible from one tree
set(_docsources
    .readthedocs.yml
    changelog.rst
    features.rst
    index.rst
    usage.rst
    README.rst
    release_notes.rst)
foreach(docsource ${_docsources})
  add_custom_command(
    OUTPUT ${_stage}/${docsource}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${docsource}
            ${_stage}/${docsource}
    DEPENDS ${docsource} ${_stage}/bits)
  list(APPEND _copyfiles ${_stage}/${docsource})
endforeach()

# For the subproject build, also copy some of the common files into the build
# tree.
add_custom_command(
  OUTPUT ${_stage}/conf_common.py
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/doc/conf.py
          ${_stage}/conf_common.py)
list(APPEND _copyfiles ${_stage}/conf_common.py)

add_custom_command(
  OUTPUT ${_stage}/sphinx-static/css/cheshire_theme.css
  COMMAND
    ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/doc/sphinx-static/css/cheshire_theme.css
    ${_stage}/sphinx-static/css/cheshire_theme.css
  DEPENDS ${_stage}/bits
          ${CMAKE_SOURCE_DIR}/doc/sphinx-static/css/cheshire_theme.css)
list(APPEND _copyfiles ${_stage}/sphinx-static/css/cheshire_theme.css)

# Listings and generated bits
add_custom_command(
  OUTPUT ${_stage}/bits/demo.cc
  COMMAND
    ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/argue/examples/argparse_example.cc
    ${_stage}/bits/demo.cc
  DEPENDS ${_stage}/bits ${CMAKE_SOURCE_DIR}/argue/examples/argparse_example.cc
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
list(APPEND _copyfiles ${_stage}/bits/demo.cc)

add_custom_command(
  OUTPUT ${_stage}/bits/simple_example.cc
  COMMAND
    ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/../examples/simple_example.cc
    ${_stage}/bits/simple_example.cc
  DEPENDS ${_stage}/bits
          ${CMAKE_CURRENT_SOURCE_DIR}/../examples/simple_example.cc
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
list(APPEND _copyfiles ${_stage}/bits/simple_example.cc)

add_custom_command(
  OUTPUT ${_stage}/bits/demo-usage.txt
  COMMAND $<TARGET_FILE:argue-argparse-example> --help >
          ${_stage}/bits/demo-usage.txt 2>&1
  DEPENDS argue-argparse-example ${_stage}/bits
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
list(APPEND _genbits ${_stage}/bits/demo-usage.txt)

add_custom_command(
  OUTPUT ${_stage}/bits/demo-usage.json
  COMMAND
    ${CMAKE_COMMAND} -E env ARGUE_HELP_FORMAT=json
    $<TARGET_FILE:argue-argparse-example> --help >
    ${_stage}/bits/demo-usage.json 2>&1
  DEPENDS argue-argparse-example ${_stage}/bits
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
list(APPEND _genbits ${_stage}/bits/demo-usage.json)

add_custom_command(
  OUTPUT ${_stage}/bits/subparser-example-usage.txt
  COMMAND $<TARGET_FILE:argue-subparser-example> --help >
          ${_stage}/bits/subparser-example-usage.txt 2>&1
  DEPENDS argue-subparser-example ${_stage}/bits
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
list(APPEND _genbits ${_stage}/bits/subparser-example-usage.txt)

set(_sphinx_manifest ${_genbits} ${_copyfiles})
list(SORT _sphinx_manifest)

# NOTE(josh): cmake 3.10 does not have list(JOIN)
string(REPLACE ";" "\n" _manifest_text "${_sphinx_manifest}")

# NOTE(josh): don't use file(WRITE) because that wont create a dependency on the
# configure step to rerun cmake if the file is missing
configure_file(MANIFEST.in ${CMAKE_CURRENT_BINARY_DIR}/MANIFEST
               NEWLINE_STYLE UNIX)

add_custom_target(
  clean-argue-sphinx-stage
  DEPENDS ${_sphinx_manifest} ${CMAKE_CURRENT_BINARY_DIR}/MANIFEST
  COMMAND python -B ${TANGENT_TOOLING}/clean_stage.py
          ${CMAKE_CURRENT_BINARY_DIR}/MANIFEST ${_stage}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Cleaning argue/doc/stage")

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
  COMMAND env PYTHONPATH=${CMAKE_SOURCE_DIR} sphinx-build -M html ${_stage}
          ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND touch ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
  DEPENDS clean-argue-sphinx-stage ${_sphinx_manifest}
          ${CMAKE_CURRENT_BINARY_DIR}/MANIFEST ${CMAKE_SOURCE_DIR}/doc/conf.py
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Building sphinx documentation for argue")

add_custom_target(doc-argue DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html/index.html)

add_custom_target(
  showdoc-argue
  COMMAND xdg-open ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html/index.html)

add_dependencies(doc doc-argue)
set_property(GLOBAL APPEND PROPERTY global_doc_files ${_sphinx_manifest})

set_property(
  DIRECTORY
  APPEND
  PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${_copyfiles} ${_genbits})

set(_deploykey)
if(IS_TRAVIS_CI)
  set(_keystore ${CMAKE_SOURCE_DIR}/tangent/tooling/deploy_keys)
  set(_deploykey ${CMAKE_CURRENT_BINARY_DIR}/argue-rtd.deploy_key)
  add_custom_command(
    OUTPUT "${_deploykey}"
    COMMAND
      # cmake-format: off
      openssl aes-256-cbc
      -K $ENV{encrypted_4bf912706780_key}
      -iv $ENV{encrypted_4bf912706780_iv}
      -in ${_keystore}/argue-rtd.enc
      -out "${_deploykey}"
      -d
      # cmake-format: on
    COMMAND chmod 0600 "${_deploykey}"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Decrypting argue-rtd.deploy_key")
endif()

add_custom_target(
  argue-push-rtd-repo
  COMMAND
    # cmake-format: off
    python -B ${TANGENT_TOOLING}/github.py sync-doc-artifacts
      --doc-repo git@github.com:cheshirekow/argue-rtd.git
      --repo-dir ${CMAKE_CURRENT_BINARY_DIR}/argue-rtd.git
      --scratch-tree ${CMAKE_CURRENT_BINARY_DIR}/scratch-tree
      --stage ${_stage}
      --deploy-key "${_deploykey}"
      --tag \$\${TRAVIS_TAG}
    # cmake-format: on
  DEPENDS clean-argue-sphinx-stage ${_sphinx_manifest} ${_deploykey}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Pushing doc artifacts to the RTD repository")
