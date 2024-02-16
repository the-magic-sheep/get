#!/bin/bash

if [ -d ~/.get ]; then
    echo "Cannot install 'get': ~/.get directory already exists." >&2
    exit 1
fi

# Create the .get directory
mkdir $HOME/.get

# Create the file cache directory
mkdir $HOME/.get/.cache

mkdir $HOME/.get/bin

curl "https://raw.githubusercontent.com/the-magic-sheep/get/master/get.c" \
    | gcc -o ~/.get/bin/get -xc -


