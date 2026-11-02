#!/bin/bash
set -x

JULIA_VERSION="1.13.3"
JULIA_MAJOR_MINOR="1.13"
JULIA_INSTALL_DIR="/usr/local/julia-${JULIA_VERSION}"
JULIA_TARBALL="julia-${JULIA_VERSION}-linux-x86_64.tar.gz"
JULIA_URL="https://julialang-s3.julialang.org/bin/linux/x64/${JULIA_MAJOR_MINOR}/${JULIA_TARBALL}"

if [ ! -d "${JULIA_INSTALL_DIR}" ]; then
	if ! command -v wget &> /dev/null && ! command -v curl &> /dev/null; then
		apt-get update
		apt-get -y install wget ca-certificates
	fi

	if command -v wget &> /dev/null; then
		wget -q "${JULIA_URL}" -O "/tmp/${JULIA_TARBALL}"
	else
		curl -fsSL "${JULIA_URL}" -o "/tmp/${JULIA_TARBALL}"
	fi
	rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

	tar -xzf "/tmp/${JULIA_TARBALL}" -C /usr/local
	rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
	rm -f "/tmp/${JULIA_TARBALL}"
fi

export JULIA_BINDIR="${JULIA_INSTALL_DIR}/bin"
export PATH="${JULIA_BINDIR}:${PATH}"
export LD_LIBRARY_PATH="${JULIA_INSTALL_DIR}/lib:${JULIA_INSTALL_DIR}/lib/julia:${LD_LIBRARY_PATH}"

