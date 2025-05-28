#!/bin/bash

ARCH=$(uname -m)
FOLDER_PATH="$1"

if [[ "$ARCH" == "x86_64" ]]; then
    exec .x86 "$FOLDER_PATH"
elif [[ "$ARCH" == "aarch64" ]]; then
    exec ./arm.out "$FOLDER_PATH"
else
    echo "Unsupported architecture: $ARCH"
    exit 1
fi

