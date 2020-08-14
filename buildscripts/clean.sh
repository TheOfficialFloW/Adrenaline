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

declare -a files=(
    "../bubble/build"
    "../bubble/pkg/sce_module/adrenaline_kernel.skprx"
    "../bubble/pkg/sce_module/adrenaline_user.suprx"
    "../bubble/pkg/sce_module/adrenaline_vsh.suprx"
    "../cef/updater/EBOOT.PBP"
    "../kernel/build"
    "../user/build"
    "../vsh/build"
    "../user/flash0/kd/inferno.prx"
    "../user/flash0/kd/popcorn.prx"
    "../user/flash0/kd/systemctrl.prx"
    "../user/flash0/kd/vshctrl.prx"
    "../user/flash0/payloadex.bin"
    "../user/flash0/vsh"
)

for i in ${files[@]}; do
    ([ -d "$i" ] || [ -f "$i" ]) && rm -rf "$i"
done

make -C ../cef clean
make -C ../cef/updater clean