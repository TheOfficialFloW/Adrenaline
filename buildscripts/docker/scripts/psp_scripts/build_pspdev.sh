#!/bin/bash

# http://redsymbol.net/articles/unofficial-bash-strict-mode/
set -euo pipefail
IFS=$'\n\t'

script_root="$(dirname "$(readlink -f $0)")"
WORKDIR="$(pwd)"
cd "${script_root}"

latest_release_url="$(curl -L "https://api.github.com/repos/pspdev/pspdev/releases?per_page=100" \
    | jq -r 'first(.[] | select(.prerelease | not).assets[].browser_download_url | contains("linux-x64"))')"

echo "$(basename "$(dirname "${latest_release_url}")")" > "${WORKDIR}/pspdev_rev.txt"

curl -L "${latest_release_url}" | tar xvz -C "$(dirname "${PSPDEV}")"
