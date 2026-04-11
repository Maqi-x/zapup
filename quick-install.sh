#!/bin/sh
set -e

TMP_DIR=$(mktemp -d)

git clone https://github.com/thezaplang/zapup.git "$TMP_DIR/zapup"

cd "$TMP_DIR/zapup"

make submodules
make -j$(($(nproc)-1))
sudo make install

cd /
rm -rf "$TMP_DIR"
