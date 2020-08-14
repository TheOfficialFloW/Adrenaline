#!/bin/bash

# http://redsymbol.net/articles/unofficial-bash-strict-mode/
set -euo pipefail
IFS=$'\n\t'

script_root="$(dirname "$(readlink -f $0)")"
WORKDIR="$(pwd)"
cd "${script_root}"

latest_release_url="$(curl -L "https://api.github.com/repos/frangarcj/vita-shader-collection/releases?per_page=100" \
    | jq -r '[.[] | select(.prerelease | not)][].assets[].browser_download_url' \
    | grep 'master' \
    | head -n 1)"

echo "$(basename "$(dirname "${latest_release_url}")")" > "${WORKDIR}/vita-shader-collection_rev.txt"

curl -L "${latest_release_url}" | tar xvz --show-transformed --transform='s|includes|include|' -C "${VITASDK}/arm-vita-eabi"