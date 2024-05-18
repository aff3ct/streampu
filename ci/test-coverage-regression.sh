#!/bin/bash
set -x

WD=$(pwd)
build_root=build

cd ${build_root}
make test
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

cd ${WD}

mkdir code_coverage_files || true
lcov --capture --directory $build_root/CMakeFiles/spu-obj.dir/src --output-file code_coverage_files/streampu.info
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp code_coverage_files/streampu.info code_coverage_files/streampu2.info
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
sed -i -e "s#${WD}/#\./#g" code_coverage_files/streampu2.info
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
lcov --remove code_coverage_files/streampu2.info "*/usr*" "*lib/*" --output-file ${WD}/code_coverage_files/streampu_clean.info
