#!/bin/bash
set -x

build_root=build
cd ${build_root}

ctest -LE "^skip-memcheck$" -T memcheck 2>&1 | tee memcheck.txt
cat memcheck.txt | grep "Memory Leak";
rc=$?; if [[ $rc == 0 ]]; then exit 1; fi
