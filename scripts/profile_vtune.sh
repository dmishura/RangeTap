#!/usr/bin/env sh
set -eu

build_dir=${1:-build}
result_dir=${2:-rangetap-vtune}
vtune_bin=${VTUNE_BIN:-}
sampling_mode=${VTUNE_SAMPLING_MODE:-sw}

if [ -z "${vtune_bin}" ]; then
    vtune_bin=$(command -v vtune 2>/dev/null || true)
fi
if [ -z "${vtune_bin}" ]; then
    for candidate in /opt/intel/oneapi/vtune/*/bin64/vtune; do
        if [ -x "${candidate}" ]; then
            vtune_bin=${candidate}
        fi
    done
fi
if [ -z "${vtune_bin}" ]; then
    echo "VTune executable not found" >&2
    exit 1
fi

"${vtune_bin}" -collect hotspots -knob sampling-mode="${sampling_mode}" \
    -result-dir "${result_dir}" -- "${build_dir}/rntp_runtime_itt"
