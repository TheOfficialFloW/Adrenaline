#!/bin/bash

# http://redsymbol.net/articles/unofficial-bash-strict-mode/
set -euo pipefail
IFS=$'\n\t'

script_root="$(dirname "$(readlink -f $0)")"
parent_dir="$(dirname "${script_root}")"
. "${parent_dir}/utils.sh"
WORKDIR="$(pwd)"
cd "${script_root}"

git clone --branch master https://bitbucket.org/DaveeFTW/psp-packer.git
cd psp-packer
store_git_rev "${WORKDIR}"
cmake .
make
cp psp-packer "${PSPDEV}/bin/"