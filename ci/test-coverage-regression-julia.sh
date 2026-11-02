#!/bin/bash
set -x

WD=$(pwd)
build_root=build

cd ${build_root}
ctest -R "julia" --output-on-failure
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

cd ${WD}

mkdir code_coverage_files || true
lcov --capture --directory $build_root/CMakeFiles/spu-obj.dir/src --output-file code_coverage_files/streampu_julia.info
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp code_coverage_files/streampu_julia.info code_coverage_files/streampu_julia2.info
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
sed -i -e "s#${WD}/#\./#g" code_coverage_files/streampu_julia2.info
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
lcov --remove code_coverage_files/streampu_julia2.info "*/usr*" "*lib/*" --output-file ${WD}/code_coverage_files/streampu_julia_clean.info

