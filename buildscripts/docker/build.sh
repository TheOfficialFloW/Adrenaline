#!/bin/bash

# http://redsymbol.net/articles/unofficial-bash-strict-mode/
set -euo pipefail
IFS=$'\n\t'

script_root="$(dirname "$(readlink -f $0)")"
cd "${script_root}"

docker build . \
    -t "io.github.theflow.adrenaline_build:$(git rev-parse --short HEAD)" \
    -t "io.github.theflow.adrenaline_build:latest" \
    $@