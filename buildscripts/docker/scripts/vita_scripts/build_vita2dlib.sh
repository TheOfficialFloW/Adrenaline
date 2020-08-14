#!/bin/bash

# http://redsymbol.net/articles/unofficial-bash-strict-mode/
set -euo pipefail
IFS=$'\n\t'

script_root="$(dirname "$(readlink -f $0)")"
parent_dir="$(dirname "${script_root}")"
. "${parent_dir}/utils.sh"
WORKDIR="$(pwd)"
cd "${script_root}"

git clone --branch fbo https://github.com/frangarcj/vita2dlib.git
cd vita2dlib
store_git_rev "${WORKDIR}"
cd libvita2d
make
make install