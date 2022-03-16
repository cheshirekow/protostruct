=====
argue
=====

.. image:: https://travis-ci.com/cheshirekow/argue.svg?branch=master
    :target: https://travis-ci.com/cheshirekow/argue

.. image:: https://readthedocs.org/projects/argue/badge/?version=latest
    :target: https://argue.readthedocs.io

``argue`` is an argument parsing library for C++, largely inspired by
python's ``argparse`` package.

::

    $ ./argue-demo -h
    ./argue_demo
    --------------------
      version: 0.0.1
      author :Josh Bialkowski <josh.bialkowski@gmail.com>
      copyright: (C) 2018

    usage: ./argue_demo [-s/--sum] [-h/--help] [-v/--version]

    Flags:
    --------------------
    -s     --sum                  sum the integers (default: find the max)
    -h     --help                 print this help message
    -v     --version              print version information and exit

    Positionals:
    --------------------
    integer                       an integer for the accumulator

    $ argue-demo 1 2 3 4
    max(1, 2, 3, 4) = 4

    $ argue-demo --sum 1 2 3 4
    sum(1, 2, 3, 4) = 10

