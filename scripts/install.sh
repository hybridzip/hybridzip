#!/usr/bin/bash
set -e

rm -rf /usr/lib/hzip
mkdir /usr/lib/hzip

cp -rp "$(dirname "$0")/bin/"* /usr/bin
cp -rp "$(dirname "$0")/lib/"* /usr/lib/hzip