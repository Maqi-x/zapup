#!/bin/sh
set -e

TMP_DIR=$(mktemp -d)

git clone https://github.com/Maqi-x/zapup.git "$TMP_DIR/zapup"

cd "$TMP_DIR/zapup"

make submodules
make
sudo make install

cd /
rm -rf "$TMP_DIR"
