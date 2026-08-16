#!/bin/sh
# cleanup.sh — remove makepkg build artifacts from this directory.
set -eu
cd "$(dirname "$0")"

# makepkg-managed dirs and metadata
#makepkg -c 2>/dev/null || true

# top-level sources + packages
rm -rf src pkg
rm -rf opons-voxd

#rm -f .SRCINFO *.pkg.tar.zst *.sig *.log

# makepkg's top-level source files (renamed tarball, header, git clone)
rm  opons-voxd whisper.cpp portaudio.h

echo "cleanup done"
