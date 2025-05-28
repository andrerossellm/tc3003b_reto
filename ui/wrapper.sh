#!/bin/bash

ARCH=$(uname -m)

if [[ "$ARCH" == "x86_64" ]]; then
    exec .x86
elif [[ "$ARCH" == "aarch64" ]]; then
    exec ./arm.out
else
    echo "Unsupported architecture: $ARCH"
    exit 1
fi

