#!/usr/bin/env python

""" Build the except shared libraries and their test programs, reproducing the
Makefile: compile lib/except.c and lib/exceptcc.cc into libexcept.so and
libexceptcc.so (-shared -fpic -ldl), then build the four test binaries, two of
which link the libraries. The object files go to a temporary directory, so
only the declared outputs land in the tree. File arguments are ignored -- the
layout below defines the build. """

import os
import subprocess
import sys
import tempfile

BASE_FLAGS = ["-O2", "-fpic", "-Wall", "-Wextra", "-Werror", "-pedantic", "-g2", "-Ilib", "-Itest"]
LD_BASE = ["-lpthread"]


def run(cmd):
    """ Run a command, exiting the process on the first failure. """
    ret = subprocess.call(cmd)
    if ret != 0:
        sys.exit(ret)


def build_library(compiler, source, obj, lib):
    """ Compile one source and link it into a shared library. """
    run([compiler, *BASE_FLAGS, "-c", source, "-o", obj])
    run([compiler, "-shared", "-fpic", "-o", lib, obj, "-ldl"])


def main():
    """ main entry point """
    with tempfile.TemporaryDirectory() as objdir:
        build_library("gcc", "lib/except.c", os.path.join(objdir, "except.o"), "libexcept.so")
        build_library("g++", "lib/exceptcc.cc", os.path.join(objdir, "exceptcc.o"), "libexceptcc.so")
    # test binaries: (compiler, source, output, extra link args)
    tests = [
        ("gcc", "test/test_link.c", "test/test_link.elf",
         [*LD_BASE, "-L.", "-lexcept"]),
        ("g++", "test/test_linkcc.cc", "test/test_linkcc.elf",
         [*LD_BASE, "-L.", "-lexceptcc"]),
        ("gcc", "test/test_nolink.c", "test/test_nolink.elf", LD_BASE),
        ("g++", "test/test_nolinkcc.cc", "test/test_nolinkcc.elf", LD_BASE),
    ]
    for compiler, source, output, ldflags in tests:
        run([compiler, *BASE_FLAGS, source, "-o", output, *ldflags])


if __name__ == "__main__":
    main()
