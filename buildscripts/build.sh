#!/bin/bash

# http://redsymbol.net/articles/unofficial-bash-strict-mode/
set -euo pipefail
IFS=$'\n\t'

if [ -z "${PSPDEV+x}" ]; then
    export PSPDEV="/usr/local/pspdev"
    export PATH="${PATH}:${PSPDEV}/bin"
fi

if [ -z "${VITASDK+x}" ]; then
    export VITASDK="/usr/local/vitasdk"
    export PATH="${VITASDK}/bin:${PATH}"
fi

script_root="$(dirname "$(readlink -f $0)")"
cd "${script_root}"

./clean.sh

modules/cef.sh
modules/kernel.sh
modules/vsh.sh
modules/user.sh
modules/updater.sh
modules/bubble.sh
