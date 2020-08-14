#!/bin/bash

# http://redsymbol.net/articles/unofficial-bash-strict-mode/
set -euo pipefail
IFS=$'\n\t'

script_root="$(dirname "$(readlink -f $0)")"
cd "${script_root}"

docker/build.sh $@

cd ..
docker run -it -v"$(pwd)":/root/Adrenaline --rm io.github.theflow.adrenaline_build:latest Adrenaline/buildscripts/clean.sh