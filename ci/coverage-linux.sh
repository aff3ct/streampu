#!/bin/bash
set -x

if [ -f code_coverage_files/streampu_julia_clean.info ]; then
	lcov -a code_coverage_files/streampu_clean.info \
	     -a code_coverage_files/streampu_julia_clean.info \
	     -o code_coverage_files/streampu_merged.info
	rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
	mv code_coverage_files/streampu_merged.info code_coverage_files/streampu_clean.info
fi

mkdir code_coverage_report || true
genhtml code_coverage_files/streampu_clean.info --output-directory ./code_coverage_report/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

# for sonarqube
lcov_cobertura.py code_coverage_files/streampu_clean.info --output code_coverage_report/streampu.xml
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
