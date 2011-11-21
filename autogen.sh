#!/bin/sh

mkdir -p build-aux || exit 1
autoreconf -vfi || exit 2
echo "Build system ready, type './configure' to configure it."
exit 0
