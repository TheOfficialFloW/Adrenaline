#!/bin/bash

# http://redsymbol.net/articles/unofficial-bash-strict-mode/
set -euo pipefail
IFS=$'\n\t'

script_root="$(dirname "$(readlink -f $0)")"
cd "${script_root}"

docker/build.sh $@

cd ..

#Git Bash for Windows requires special handling
if [[ "$OSTYPE" == "msys" ]]; then
  winpty docker run -it -v"/$PWD":/root/Adrenaline --rm io.github.theflow.adrenaline_build:latest Adrenaline/buildscripts/build.sh
else
  docker run -it -v"$(pwd)":/root/Adrenaline --user "$(id -u):$(id -g)" --rm io.github.theflow.adrenaline_build:latest Adrenaline/buildscripts/build.sh
fi
