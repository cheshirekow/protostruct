with section("lint"):
  disabled_codes = [
    # A custom command with one output doesn't really need a comment because
    # the default "generating XXX" is a good message already.
    "C0113",
  ]


with section("parse"):
  additional_commands = {
    # Wrappers.cmake
    "cc_binary": {
      "pargs": "1+",
      "flags": ["ADD_RUNTARGET"],
      "kwargs": {
          "INC": {
            "pargs": 0,
            "kwargs": {
                "PUBLIC": "*",
                "PRIVATE": "*",
                "INTERFACE": "*",
            }
          },
          "LIBDIRS": {
            "pargs": "*",
            "kwargs": {
                "PUBLIC": "*",
                "PRIVATE": "*",
                "INTERFACE": "*",
            }
          },
          "SRCS": "*",
          "DEPS": "*",
          "PKGDEPS": "*",
          "PROPERTIES": {
            "kwargs": {
              "OUTPUT_NAME": 1,
              "EXPORT_NAME": 1,
            }
          }
      }
    },

    "cc_library": {
      "pargs": "1+",
      "flags": ["STATIC", "SHARED"],
      "kwargs": {
          "INC": {
            "pargs": 0,
            "kwargs": {
                "PUBLIC": "*",
                "PRIVATE": "*",
                "INTERFACE": "*",
            }
          },
          "LIBDIRS": {
            "pargs": "*",
            "kwargs": {
                "PUBLIC": "*",
                "PRIVATE": "*",
                "INTERFACE": "*",
            }
          },
          "SRCS": "*",
          "DEPS": {
            "pargs": "*",
            "kwargs": {
                "PUBLIC": "*",
                "PRIVATE": "*",
                "INTERFACE": "*",
            }
          },
          "PKGDEPS": "*",
          "PROPERTIES": {
            "kwargs": {
              "ARCHIVE_OUTPUT_NAME": 1,
              "EXPORT_NAME": 1,
              "INTERFACE_INCLUDE_DIRECTORIES": 1,
              "LIBRARY_OUTPUT_NAME": 1,
              "OUTPUT_NAME": 1,
              "SOVERSION": 1,
              "SUFFIX": 1,
              "VERSION": 1,
            }
          }
      }
    },

    "cc_test": {
      "pargs": 1,
      "kwargs": {
          "SRCS": "*",
          "DEPS": "*",
          "PKGDEPS": "*",
          "ARGV": "*",
          "TEST_DEPS": "*",
          "WORKING_DIRECTORY": "*",
          "LABELS": "*"
      }
    },

    "check_call": {
      "kwargs": {
        "COMMAND": "*",
        "WORKING_DIRECTORY": "1",
        "TIMEOUT": "1",
        "RESULT_VARIABLE": "1",
        "RESULTS_VARIABLE": "1",
        "OUTPUT_VARIABLE": "1",
        "ERROR_VARIABLE": "1",
        "INPUT_FILE": "1",
        "OUTPUT_FILE": "1",
        "ERROR_FILE": "1",
        "ENCODING": "1",
      },
      "flags": [
        "OUTPUT_QUIET",
        "ERROR_QUIET",
        "OUTPUT_STRIP_TRAILING_WHITESPACE",
        "ERROR_STRIP_TRAILING_WHITESPACE",
      ]
    },

    "gresource" : {
      "pargs": 2,
      "kwargs": {
        "SRCDIR": 1,
        "DEPENDS": "+",
      }
    },

    "join": {
      "pargs": [1, "+"],
      "kwargs": {
          "GLUE": 1,
      }
    },

    "check_pyoneline": {
      "pargs": "+",
      "kwargs": {
        "OUTPUT_VARIABLE": 1,
        "ERROR_VARIABLE": 1
      }
    },

    "tangent_addtest": {
      "kwargs": {
        "NAME": 1,
        "WORKING_DIRECTORY": 1,
        "COMMAND": "+",
        "CONFIGURATIONS": "+",
        "DEPENDS": "+",
        "LABELS": "+",
      }
    },

    "travis_decrypt": {
      "pargs": [3],
      "kwargs": {}
    },

    # debian.cmake
    "create_debian_binary_packages": {
      "pargs": [3, "+"],
      "kwargs": {
        "OUTPUTS": "*",
        "DEPS": "*"
      }
    },
    "create_debian_depsrepo": {
      "pargs": [3, "+"]
    },
    "create_debian_packages": {
      "pargs": [
        {"nargs": "+", "flags": ["FORCE_PBUILDER"]},
      ],
      "kwargs": {
        "OUTPUTS": "*",
        "DEPS": "*"
      }
    },
    "debhelp": {
      "pargs": ["1+", ],
      "spelling": "DEBHELP"
    },
    "get_debs": {
      "pargs": [3, "*"],
    },

    # pkgconfig.cmake
    "pkg_find": {
      "kwargs": {
        "PKG": "*"
      }
    },

    # codestyle.cmake
    "format_and_lint": {
      "kwargs": {
        "CC": "*",
        "CMAKE": "*",
        "PY": "*",
        "JS": "*",
        "SHELL": "*"
      }
    },

    # other:
    "importvars": {
      "pargs": "1+",
      "kwargs": {
        "VARS": "+"
      },
      "spelling": "IMPORTVARS"
    },

    "exportvars": {
      "pargs": "1+",
      "kwargs": {
        "VARS": "+"
      },
      "spelling": "EXPORTVARS"
    },

    "stage_files": {
      "kwargs": {
        "LIST": 1,
        "STAGE": 1,
        "SOURCEDIR": 1,
        "FILES": "*"
      },
    },

    "tangent_extract_svg": {
      "kwargs": {
        "OUTPUT": 1,
        "SRC": 1,
        "EXPORT": 1,
      }
    },

    "tangent_fetchobj": {
      "pargs": 2,
      "kwargs": {
        "OUTDIR": 1,
      }
    },

    "tangent_rmark_render": {
      "pargs": 1,
      "kwargs": {
        "UUID": 1,
        "OUTPUT": 1,
        "FORMAT": 1,
        "PAGENO": 1,
        "DEPENDS": 1
      }
    },

    "tangent_unzip": {
      "pargs": "1+",
      "kwargs": {
        "OUTPUT": "1+",
        "WORKING_DIRECTORY": 1,
      }
    },


    # GtkDocConfig.cmake
    "gtk_doc_add_module": {
      "pargs": 1,
      "kwargs": {
        "FIXREFOPTS": "*",
        "IGNOREHEADERS": "*",
        "LIBRARIES": "*",
        "LIBRARY_DIRS": "*",
        "SOURCE": "*",
        "SUFFIXES": "*",
        "XML": 1,
      },
    },

  }
