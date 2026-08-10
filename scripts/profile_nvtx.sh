#!/usr/bin/env sh
set -eu

build_dir=${1:-build}
result_file=${2:-rangetap-nvtx}

nsys profile --trace=nvtx,osrt --force-overwrite=true --output="${result_file}" \
    "${build_dir}/rntp_runtime_nvtx"
