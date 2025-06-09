#!/bin/bash
# Usage: ./add_binary.sh /path/to/binary
# E.g.: ./add_binary.sh /usr/bin/acpidump
if [ -z "$1" ]; then
echo "Usage: $0 /path/to/binary"
exit 1
fi
# This script assumes that the project root is the parent directory of the script.
# You may change the path definition as needed.

script_dir=$(dirname "$(realpath "$0")")
BINARY="$1"
BINARY_NAME=$(basename "$BINARY")
INITRAM_DIR="$script_dir/../_install"
# Make sure we're not missing any directory
mkdir -p "$INITRAM_DIR/bin"
mkdir -p "$INITRAM_DIR/usr/bin"
mkdir -p "$INITRAM_DIR/lib"
mkdir -p "$INITRAM_DIR/lib64"
mkdir -p "$INITRAM_DIR/lib/x86_64-linux-gnu"
# Copy the binary
if [[ "$BINARY" == /usr/bin/* ]]; then
cp "$BINARY" "$INITRAM_DIR/usr/bin/"
chmod +x "$INITRAM_DIR/usr/bin/$BINARY_NAME"
else
cp "$BINARY" "$INITRAM_DIR/bin/"
chmod +x "$INITRAM_DIR/bin/$BINARY_NAME"
fi
# Get and copy all dependencies
ldd "$BINARY" | grep "=>" | awk '{print $3}' | while read -r LIB; do
if [ -n "$LIB" ]; then
# Create target directory
TARGET_DIR=$(dirname "$LIB")
mkdir -p "$INITRAM_DIR$TARGET_DIR"
# Copy library
cp "$LIB" "$INITRAM_DIR$TARGET_DIR/"
fi
done
# Don't forget the dynamic linker (ld-linux)
LINKER=$(ldd "$BINARY" | grep "ld-linux" | awk '{print $1}')
if [ -n "$LINKER" ]; then
LINKER_DIR=$(dirname "$LINKER")
mkdir -p "$INITRAM_DIR$LINKER_DIR"
cp "$LINKER" "$INITRAM_DIR$LINKER_DIR/"
fi
echo "Added $BINARY_NAME and its dependencies to initramfs"
