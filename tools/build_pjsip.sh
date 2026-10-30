#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
# SPDX-License-Identifier: GPL-3.0-or-later
# Builds PJSIP as static, audio-only libraries and installs them into a prefix
# that the app CMake build picks up via -DPj_DIR=<prefix>/lib/cmake/Pj.
#
#   tools/build_pjsip.sh <install-prefix> [version]
#
# Runs on macOS (bash) and Windows (Git Bash + Visual Studio).
#
# TLS is off for now: PJSIP's CMake build (experimental in 2.17) stubs out the
# native Schannel/Apple backends, so TLS will come with an OpenSSL build (stage 2).
set -euo pipefail

PREFIX="${1:?usage: build_pjsip.sh <install-prefix> [version]}"
VERSION="${2:-2.17}"
WORK="$(mktemp -d)"

echo "Building PJSIP ${VERSION} into ${PREFIX}"
curl -fsSL "https://github.com/pjsip/pjproject/archive/refs/tags/${VERSION}.tar.gz" | tar -xz -C "$WORK"

# On Windows use Ninja (run from an MSVC developer environment): the Visual Studio
# generator can't build the EXCLUDE_FROM_ALL third-party targets by name.
GENERATOR=()
case "$(uname -s)" in
    MINGW*|MSYS*|CYGWIN*) GENERATOR=(-G Ninja -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl) ;;
esac

# ${arr[@]+...}: macOS bash 3.2 treats an empty array as unbound under `set -u`.
cmake ${GENERATOR[@]+"${GENERATOR[@]}"} -S "$WORK/pjproject-${VERSION}" -B "$WORK/build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
    -DBUILD_SHARED_LIBS=OFF \
    -DBUILD_TESTING=OFF \
    -DPJ_SKIP_EXPERIMENTAL_NOTICE=ON \
    -DPJLIB_WITH_SSL="" \
    -DCMAKE_DISABLE_FIND_PACKAGE_OpenSSL=TRUE \
    -DPJNATH_WITH_UPNP=OFF \
    -DPJMEDIA_WITH_VIDEO=OFF \
    -DPJMEDIA_WITH_AUDIODEV_WASAPI=OFF \
    -DPJMEDIA_WITH_OPUS_CODEC=OFF \
    -DPJMEDIA_WITH_G7221_CODEC=OFF \
    -DPJMEDIA_WITH_ILBC_CODEC=OFF \
    -DPJMEDIA_WITH_SILK_CODEC=OFF \
    -DPJMEDIA_WITH_BCG729_CODEC=OFF \
    -DPJMEDIA_WITH_LYRA_CODEC=OFF \
    -DPJMEDIA_WITH_OPENCORE_AMRNB_CODEC=OFF \
    -DPJMEDIA_WITH_OPENCORE_AMRWB_CODEC=OFF

cmake --build "$WORK/build" --config Release --parallel

# Bundled third-party libs are EXCLUDE_FROM_ALL, yet PjTargets.cmake exports all of
# them; build the ones nothing links (e.g. libyuv without video) so install finds them.
# G.722.1 and iLBC are built only for that reason: their codecs are disabled above
# (unclear licenses, see NOTICE.md), so nothing in the app links them.
# Targets missing on a platform are skipped; a lib that is exported but still
# missing makes the app configure step fail loudly.
for dep in g7221 gsm ilbc resample speex srtp yuv webrtc webrtc_aec3; do
    cmake --build "$WORK/build" --config Release --parallel --target "$dep" \
        || echo "note: bundled target '$dep' not available, skipped"
done

cmake --install "$WORK/build" --config Release

rm -rf "$WORK"
echo "PJSIP installed to ${PREFIX}"
