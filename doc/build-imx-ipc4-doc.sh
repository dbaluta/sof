#!/bin/sh
# Render doc/imx-ipc4-support.rst to a self-contained HTML page for reading.
#
# Usage:
#   ./doc/build-imx-ipc4-doc.sh [output.html]
#
# Default output is ../imx-ipc4-support.html (outside the repo, so the build
# artifact is not tracked). Requires docutils (provides rst2html5).

set -e

here=$(dirname "$0")
src="$here/imx-ipc4-support.rst"
out="${1:-$here/../../imx-ipc4-support.html}"

rst2html5 "$src" "$out"
echo "Wrote $out"
