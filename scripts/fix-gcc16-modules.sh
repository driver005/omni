#!/usr/bin/env bash
# Work around GCC libstdc++/119266: the generated libstdc++.modules.json points
# at bits/std.cc one directory too shallow on the standard Linux layout where
# headers live in /usr/include/c++/<version>/ but the manifest lands in
# /usr/lib/gcc/<target>/<version>/.
#
# Run once after installing GCC 16 (and again after a GCC major-version bump):
#   scripts/fix-gcc16-modules.sh
set -euo pipefail

CXX="${CXX:-g++-16}"
if ! command -v "$CXX" >/dev/null 2>&1; then
    echo "error: $CXX not found; install GCC 16 (g++-16) first" >&2
    exit 1
fi

TARGET="$("$CXX" -dumpmachine)"
VERSION_MAJOR="$("$CXX" -dumpversion | cut -d. -f1)"
LIBDIR="/usr/lib/gcc/${TARGET}"
LINK_PARENT="${LIBDIR}/include"
LINK="${LINK_PARENT}/c++"
HEADERS="/usr/include/c++"

SUDO=""
if [[ "$(id -u)" -ne 0 ]]; then
    SUDO="sudo"
fi

echo "GCC ${VERSION_MAJOR} target: ${TARGET}"
echo "Linking ${LINK} -> ${HEADERS}"
${SUDO} mkdir -p "${LINK_PARENT}"
${SUDO} ln -sfn "${HEADERS}" "${LINK}"

if [[ ! -f "${LINK}/${VERSION_MAJOR}/bits/std.cc" ]]; then
    echo "error: std.cc still not reachable at ${LINK}/${VERSION_MAJOR}/bits/std.cc" >&2
    exit 1
fi
echo "OK: ${LINK}/${VERSION_MAJOR}/bits/std.cc is reachable"
